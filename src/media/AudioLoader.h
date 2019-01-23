#ifndef AUDIOLOADER_H
#define AUDIOLOADER_H

#include "string"
#include "ffmpeg.h"
#include "core.h"
#include "vector"

namespace sm {
namespace media {

// same as AVMediaType
enum StreamType {
    VIDEO,
    AUDIO,
    DATA,
    SUBTITLE,
    ATTACHMENT,
    NB,
};

struct StreamInfo {
    int number;
    StreamType type;
    int sampleRate;
    sm::time_unit duration;
    friend class AudioLoader;
};

class AudioLoader {
public:
    AudioLoader();
    ~AudioLoader();

    void open(std::string filename);
    void close();
    bool isOpen();
    void read(double timeout);
    void readAll();

    std::vector<StreamInfo> streams;

    int sampleRate;
    std::vector<int16_t> samples;

private:
    bool mIsOpen = false;
    bool eof = false;
    AVFormatContext *fmt_ctx = nullptr;
    AVCodec *dec;
    AVFrame *frame;
    void loop();
    int preferredStream;

    // selected stream
    int choosedStream;
    AVCodecContext *dec_ctx = nullptr;
};

}
}

#endif //AUDIOLOADER_H
