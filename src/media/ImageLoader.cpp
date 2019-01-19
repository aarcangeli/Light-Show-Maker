#include <utility>
#include <fstream>
#include "ImageLoader.h"
#include "ffmpeg.h"
#include "cassert"

using namespace sm::media;
using namespace std;

ImageLoader::ImageLoader() {
    frame = av_frame_alloc();
}

ImageLoader::~ImageLoader() {
    av_frame_free(&frame);
    close();
}

shared_ptr<Image> ImageLoader::loadImage(const std::string &filename_) {
    filename = filename_;
    close();

    if (avformat_open_input(&fmt_ctx, filename.c_str(), nullptr, nullptr) < 0) {
        printf("Cannot open file '%s'\n", filename.c_str());
        return nullptr;
    }

    return readFromCtx();
}

std::vector<uint8_t> ImageLoader::encodeImage(const shared_ptr<Image> &image) {
    buffer.resize(0);
    close();

    codec = avcodec_find_encoder(AV_CODEC_ID_PNG);
    if (!codec) {
        printf("ERROR: Cannot find png codec\n");
        return {};
    }

    uint32_t w = image->width;
    uint32_t h = image->height;
    frame->width = w;
    frame->height = h;
    frame->format = AV_PIX_FMT_RGBA;
    frame->data[0] = (uint8_t *) image->pixels.data();
    frame->linesize[0] = w * 4;
    frame->key_frame = 1;
    frame->pict_type = AV_PICTURE_TYPE_NONE;
    frame->pts = 1;

    codecCtx = avcodec_alloc_context3(codec);
    if (!codecCtx) {
        printf("ERROR: Cannot allocate AVCodecContext\n");
        return {};
    }

    codecCtx->width = w;
    codecCtx->height = h;
    codecCtx->pix_fmt = AV_PIX_FMT_RGBA;
    codecCtx->time_base = {1, 25};
    codecCtx->framerate = {25, 1};

    int ret = avcodec_open2(codecCtx, codec, nullptr);
    if (ret < 0) {
        printf("ERROR: Cannot open codec\n");
        return {};
    }

    if (!encodeFrame(frame)) return {};
    //if (!encodeFrame(frame)) return {};
    if (!encodeFrame(nullptr)) return {};

//    std::ofstream file("test.png", ios::out | ios::binary);
//    file.write((char *) buffer.data(), buffer.size());
//    file.close();

    return buffer;
}

int readFunction(void *opaque, uint8_t *buf, int buf_size) {
    auto me = (ImageLoader *) (opaque);
    uint32_t &pos = me->position;
    const vector<uint8_t> *myVec = me->myVec;
    if (pos + buf_size > myVec->size()) {
        buf_size = myVec->size() - pos;
    }
    memcpy(buf, myVec->data() + pos, buf_size);
    pos += buf_size;
    if (!buf_size) buf_size = -1;
    return buf_size;
}

shared_ptr<Image> ImageLoader::decodeImage(const vector<uint8_t> &vector) {
    close();
    this->myVec = &vector;
    this->position = 0;

    void *buffer = av_malloc(8192);

    AVIOContext *btc = avio_alloc_context((unsigned char *) buffer,
                                          8192, 0, (void *) this,
                                          &readFunction, nullptr, nullptr);

    fmt_ctx = avformat_alloc_context();
    fmt_ctx->pb = btc;
    if (avformat_open_input(&fmt_ctx, "dummyFilename", nullptr, nullptr) < 0) {
        printf("Warning: Cannot open format a codec for stream '%s'\n", filename.c_str());
        return nullptr;
    }

    shared_ptr<Image> ptr = readFromCtx();

    //av_free(btc);
    //av_free(buffer);

    return ptr;
}

shared_ptr<Image> ImageLoader::readFromCtx() {
    if (avformat_find_stream_info(fmt_ctx, nullptr) < 0) {
        printf("Cannot find stream information from file '%s'\n", filename.c_str());
        return nullptr;
    }

    AVStream *stream = nullptr;
    int streamNumber = 0;
    for (int i = 0; i < fmt_ctx->nb_streams; i++) {
        AVStream *it = fmt_ctx->streams[i];
        if (it->codecpar->codec_type != AVMEDIA_TYPE_VIDEO) continue;
        codec = avcodec_find_decoder(it->codecpar->codec_id);
        if (!codec) {
            printf("Warning: Cannot find a codec for stream %i '%s'\n", streamNumber, filename.c_str());
            continue;
        }
        stream = it;
        streamNumber = i;
        break;
    }
    if (!stream) {
        printf("Cannot find a valid video stream from file '%s'\n", filename.c_str());
        return nullptr;
    }

    codecCtx = avcodec_alloc_context3(codec);
    if (!codecCtx) {
        printf("Cannot allocate AVCodecContext\n");
        return nullptr;
    }

    avcodec_parameters_to_context(codecCtx, stream->codecpar);
    if (avcodec_open2(codecCtx, codec, nullptr) < 0) {
        printf("Cannot open decoder for stream %i in '%s'\n", streamNumber, filename.c_str());
        avcodec_free_context(&codecCtx);
        return nullptr;
    }

    while (true) {
        AVPacket packet;

        if (av_read_frame(fmt_ctx, &packet) < 0) {
            printf("Cannot read from '%s'\n", filename.c_str());
            return nullptr;
        }

        if (avcodec_send_packet(codecCtx, &packet) < 0) {
            printf("ERROR: Error while sending a packet to the decoder in '%s'\n", filename.c_str());
            return nullptr;
        }

        while (true) {
            int ret = avcodec_receive_frame(codecCtx, frame);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                printf("EOF in '%s'\n", filename.c_str());
                return nullptr;
            }

            AVPixelFormat format = static_cast<AVPixelFormat>(frame->format);
            uint32_t width = static_cast<uint32_t>(frame->width);
            uint32_t height = static_cast<uint32_t>(frame->height);

            SwsContext *sws_ctx = sws_getContext(width, height, format,
                                                 width, height, AV_PIX_FMT_RGBA,
                                                 SWS_BILINEAR, nullptr, nullptr, nullptr);
            if (!sws_ctx) {
                printf("ERROR: Cannot create sws context\n");
                return nullptr;
            }

            // allocate output
            uint8_t *dst_data[4];
            int dst_linesize[4];
            if ((ret = av_image_alloc(dst_data, dst_linesize,
                                      width, height, AV_PIX_FMT_RGBA, 1)) < 0) {
                printf("ERROR: Could not allocate destination image\n");
                sws_freeContext(sws_ctx);
                return nullptr;
            }
            assert(ret == width * height * 4);
            assert(dst_linesize[0] == width * 4);

            int height2 = sws_scale(sws_ctx,
                                    frame->data, frame->linesize, 0, height,
                                    dst_data, dst_linesize);
            assert(height2 == height);

            shared_ptr<Image> result = make_shared<Image>();
            result->width = width;
            result->height = height;
            result->pixels.resize(width * height * 4);
            memcpy(result->pixels.data(), dst_data[0], width * height * 4);

            sws_freeContext(sws_ctx);
            av_frame_unref(frame);

            return result;
        }
    }
}

bool ImageLoader::encodeFrame(AVFrame *pFrame) {
    int ret = avcodec_send_frame(codecCtx, pFrame);
    if (ret < 0) {
        printf("ERROR: Error sending a frame for encoding\n");
        return false;
    }

    AVPacket *pkt = av_packet_alloc();

    while (true) {
        int ret = avcodec_receive_packet(codecCtx, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            break;
        if (ret < 0) {
            printf("ERROR: Error during encoding\n");
            av_packet_free(&pkt);
            return false;
        }

        uint64_t pos = buffer.size();
        buffer.resize(pos + pkt->size);
        memcpy(buffer.data() + pos, pkt->data, pkt->size);

        av_packet_unref(pkt);
    }

    av_packet_free(&pkt);
    return true;
}

void ImageLoader::close() {
    //avformat_close_input(&fmt_ctx);
    //avcodec_free_context(&codecCtx);
}

shared_ptr<Image> sm::media::loadImage(string filename) {
    return ImageLoader().loadImage(filename);
}

std::vector<uint8_t> sm::media::encodeImage(const std::shared_ptr<Image> &image) {
    return ImageLoader().encodeImage(image);
}

std::shared_ptr<Image> sm::media::decodeImage(const std::vector<uint8_t> &data) {
    return ImageLoader().decodeImage(data);
}
