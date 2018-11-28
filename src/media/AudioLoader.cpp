#include "AudioLoader.h"

using namespace sm::media;

AudioLoader::AudioLoader() {
    frame = av_frame_alloc();
};

AudioLoader::~AudioLoader() {
    av_frame_free(&frame);
};

void AudioLoader::open(std::string filename) {
    close();

    if (avformat_open_input(&fmt_ctx, filename.c_str(), nullptr, nullptr) < 0) {
        printf("Cannot open file '%s'\n", filename.c_str());
        return;
    }

    if (avformat_find_stream_info(fmt_ctx, nullptr) < 0) {
        printf("Cannot find stream information from file '%s'\n", filename.c_str());
        return;
    }

    preferredStream = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, &dec, 0);
    if (preferredStream < 0) {
        printf("Cannot find an audio stream in file '%s'\n", filename.c_str());
        return;
    }

    for (int i = 0; i < fmt_ctx->nb_streams; i++) {
        AVStream *stream = fmt_ctx->streams[i];
        StreamInfo info{};
        info.number = i;
        info.type = (StreamType) stream->codecpar->codec_type;
        float duration = (float) stream->duration * stream->time_base.num / stream->time_base.den * sm::TIME_UNITS;
        info.duration = static_cast<time_unit>(duration);
        info.sampleRate = stream->codecpar->sample_rate;
        streams.push_back(info);
    }


    for (auto &stream : streams) {
        if (stream.type != AUDIO) continue;
        AVCodecContext *dec_ctx;
        dec_ctx = avcodec_alloc_context3(dec);
        if (!dec_ctx) {
            printf("Cannot allocate AVCodecContext\n");
            return;
        }
        avcodec_parameters_to_context(dec_ctx, fmt_ctx->streams[stream.number]->codecpar);
        if (avcodec_open2(dec_ctx, dec, nullptr) < 0) {
            printf("Cannot open decoder for stream %i in '%s'\n", stream.number, filename.c_str());
            avcodec_free_context(&dec_ctx);
            continue;
        }
        stream.dec_ctx = dec_ctx;
        break;
    }

    mIsOpen = true;
}

void AudioLoader::close() {
    avformat_close_input(&fmt_ctx);
    streams.resize(0);
    mIsOpen = false;
}

void AudioLoader::process() {
    if (!mIsOpen) return;

    // todo: time based process
    while (mIsOpen) loop();
}

void AudioLoader::loop() {
    int ret;
    AVPacket packet;

    if (av_read_frame(fmt_ctx, &packet) < 0) {
        close();
        return;
    }

    StreamInfo stream = streams[packet.stream_index];
    if (!stream.dec_ctx) {
        return;
    }

    ret = avcodec_send_packet(stream.dec_ctx, &packet);
    if (ret < 0) {
        printf("ERROR: Error while sending a packet to the decoder of stream %i\n", stream.number);
        close();
        return;
    }

    while (ret >= 0) {
        ret = avcodec_receive_frame(stream.dec_ctx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
        }
        if (ret < 0) {
            printf("ERROR: Error while receiving a frame from the decoder of stream %i\n", stream.number);
            close();
            return;
        }

        av_frame_unref(frame);
    }

    av_packet_unref(&packet);
}

bool AudioLoader::isOpen() { return mIsOpen; }
