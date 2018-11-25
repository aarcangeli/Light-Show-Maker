#include "AudioLoader.h"

using namespace sm::media;

AudioLoader::AudioLoader() = default;

AudioLoader::~AudioLoader() = default;

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
        StreamInfo info;
        info.number = i;
        info.type = (StreamType) stream->codecpar->codec_type;
        float duration = (float) stream->duration * stream->time_base.num / stream->time_base.den * sm::TIME_UNITS;
        info.duration = static_cast<time_unit>(duration);
        streams.push_back(info);
    }

    mIsOpen = true;
}

void AudioLoader::close() {
    avformat_close_input(&fmt_ctx);
    streams.resize(0);
    mIsOpen = false;
}

void AudioLoader::process() {
}
