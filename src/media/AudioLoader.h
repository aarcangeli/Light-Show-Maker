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
    sm::time_unit duration;
};

class AudioLoader {
public:
    AudioLoader();
    ~AudioLoader();

    void open(std::string filename);
    void close();
    bool isOpen() { return mIsOpen; }
    void process();

    std::vector<StreamInfo> streams;
    int preferredStream;

private:
    // ffmpeg data
    bool mIsOpen = true;
    AVFormatContext *fmt_ctx = nullptr;
    AVCodec *dec;
};

}
}

#endif //AUDIOLOADER_H