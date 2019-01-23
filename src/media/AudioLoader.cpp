#include "AudioLoader.h"

using namespace sm::media;

static char *errtext(int err) {
    static char errbuff[512];
    av_strerror(err, errbuff, sizeof(errbuff));
    return errbuff;
}

AudioLoader::AudioLoader() {
    frame = av_frame_alloc();
};

AudioLoader::~AudioLoader() {
    close();
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
        sampleRate = stream.sampleRate;
        this->choosedStream = stream.number;
        this->dec_ctx = dec_ctx;
    }

//    // Set resampler options
//    resample_context = swr_alloc_set_opts(nullptr,
//                                          av_get_default_channel_layout(OUTPUT_CHANNELS),
//                                          OUTPUT_FMT,
//                                          OUTPUT_RATE,
//                                          dec_ctx->channel_layout,
//                                          dec_ctx->sample_fmt,
//                                          dec_ctx->sample_rate,
//                                          0, nullptr);
//    if (!resample_context) {
//        fprintf(stderr, "Unable to allocate resampler context\n");
//        close();
//        return;
//    }
//
//    // Open the resampler
//    if ((ret = swr_init(resample_context)) < 0) {
//        fprintf(stderr, "Unable to open resampler context: %s\n", errtext(ret));
//        swr_free(&resample_context);
//        close();
//        return;
//    }

    mIsOpen = true;
    eof = false;
}

void AudioLoader::close() {
    avformat_close_input(&fmt_ctx);
    streams.resize(0);
    mIsOpen = false;
}

void AudioLoader::read(double timeout) {
    loop();
}

void AudioLoader::readAll() {
    while (mIsOpen && !eof) {
        loop();
    }
}

void AudioLoader::loop() {
    if (eof) return;
    int ret;
    AVPacket packet;

    if (av_read_frame(fmt_ctx, &packet) < 0) {
        eof = true;
        return;
    }

    if (packet.stream_index != choosedStream)
        return;

    ret = avcodec_send_packet(dec_ctx, &packet);
    if (ret < 0) {
        printf("ERROR: Error while sending a packet to the decoder of stream %i\n", packet.stream_index);
        close();
        return;
    }

    while (ret >= 0) {
        ret = avcodec_receive_frame(dec_ctx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
        }
        if (ret < 0) {
            printf("ERROR: Error while receiving a frame from the decoder of stream %i\n", packet.stream_index);
            close();
            return;
        }

        size_t pos = samples.size();
        samples.resize(pos + frame->nb_samples);
        float *inputSamples = (float *) frame->data[1];

        for (int i = 0; i < frame->nb_samples; i++) {
            samples[pos + i] = (short) (inputSamples[i] * 0x7fff);
        }

        av_frame_unref(frame);
    }

    av_packet_unref(&packet);
}

bool AudioLoader::isOpen() { return mIsOpen; }
