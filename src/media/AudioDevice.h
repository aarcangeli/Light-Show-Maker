#ifndef AUDIODEVICE_H
#define AUDIODEVICE_H

#include "ao/ao.h"
#include "string"
#include "ffmpeg.h"
#include "core.h"
#include "vector"
#include "mutex"

namespace sm {
namespace media {

class AudioDevice {
public:
    AudioDevice();
    ~AudioDevice();

    void open(int sampleRate, int channels, int bits);

    void play(void *data, int size);

private:
    static int numberOfInstances;
    ao_device *adevice;
    ao_sample_format sformat;
    bool msIsOpen = false;
};

}
}

#endif //AUDIODEVICE_H
