#include "AudioDevice.h"

using namespace sm::media;

int AudioDevice::numberOfInstances = 0;

AudioDevice::AudioDevice() {
    if (numberOfInstances == 0) ao_initialize();
    numberOfInstances++;
};

AudioDevice::~AudioDevice() {
    numberOfInstances--;
    if (numberOfInstances == 0) ao_shutdown();
};

void AudioDevice::open(int sampleRate, int channels) {
    ao_sample_format sformat;
    sformat.channels = channels;
    sformat.rate = sampleRate;
    sformat.byte_format = AO_FMT_NATIVE;
    sformat.matrix = nullptr;

    int driver = ao_default_driver_id();
    if (driver < 0) {
        printf("Cannot find a live driver\n");
        return;
    }

    ao_device *adevice = ao_open_live(driver, &sformat, nullptr);
    if (!adevice) {
        printf("Cannot open driver, errno=%i\n", errno);
        return;
    }


}
