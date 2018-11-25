#ifndef AUDIODEVICE_H
#define AUDIODEVICE_H

#include "string"
#include "ffmpeg.h"
#include "core.h"
#include "vector"

namespace sm {
namespace media {

class AudioDevice {
public:
    const int sampleRate = 44100;

    AudioDevice();
    ~AudioDevice();

    void open();

private:
};

}
}

#endif //AUDIODEVICE_H
