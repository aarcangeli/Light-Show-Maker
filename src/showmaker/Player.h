#ifndef PLAYER_H
#define PLAYER_H

#include "core.h"

namespace sm {

class Player {
public:
    void update();

    void play();
    void pause();
    void stop();
    void seek(time_unit time);

    time_unit playerPosition();
    bool playing();

private:
    double lastTime = 0;
    bool isPlaying = false;
    float position = 0;
};

}

#endif //PLAYER_H
