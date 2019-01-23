#ifndef PLAYER_H
#define PLAYER_H

#include "core.h"
#include "AudioDevice.h"
#include "Canvas.h"
#include "AudioDevice.h"
#include <chrono>
#include <thread>

namespace sm {

class Player {
public:
    Player();
    void update(model::Canvas &canvas);

    void togglePlay();
    void play();
    void pause();
    void stop();
    void seek(time_unit time);
    void setSeeking(bool seeking);
    void setPlaying(bool playing);
    void goBack();
    void reloadMedia();

    time_unit playerPosition();
    bool playing();

private:
    bool isPlaying = false;
    float position = 0;
    float backPosition = 0;
    bool isSeeking = false;
    bool afterSeek = false;
    bool reopenDevice = false;
    media::AudioDevice device;

    std::thread playerThread;

    std::mutex workMutex;
    void worker();
};

}

#endif //PLAYER_H
