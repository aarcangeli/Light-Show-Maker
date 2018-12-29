#ifndef PLAYER_H
#define PLAYER_H

#include "core.h"
#include "AudioDevice.h"
#include "Canvas.h"
#include "AudioDevice.h"

namespace sm {

class Player {
public:
    void update(model::Canvas &canvas);

    void togglePlay();
    void play();
    void pause();
    void stop();
    void seek(time_unit time);
    void setPlaying(bool playing);
    void goBack();
    void reloadMedia();

    time_unit playerPosition();
    bool playing();

private:
    double lastTime = 0;
    bool isPlaying = false;
    float position = 0;
    float backPosition = 0;
    bool justPlayed = false;
    bool afterSeek = false;
    size_t sampleTarget;
    size_t samplePosition;
    media::AudioDevice device;
};

}

#endif //PLAYER_H
