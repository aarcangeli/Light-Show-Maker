#ifndef AUDIODEVICE_H
#define AUDIODEVICE_H

#include "ao/ao.h"
#include "string"
#include "ffmpeg.h"
#include "core.h"
#include "vector"
#include "thread"
#include "mutex"

namespace sm {
namespace media {

class AudioDevice {
public:
    AudioDevice();
    ~AudioDevice();

    void open(int sampleRate, int channels);

private:
    static int numberOfInstances;
    ao_device *adevice;
    std::thread myThread;
};

}
}

#endif //AUDIODEVICE_H
