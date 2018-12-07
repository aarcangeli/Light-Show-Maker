#include "Player.h"
#include <GLFW/glfw3.h>
#include <imgui.h>

using namespace sm;

void Player::update() {
    double currentFrame = glfwGetTime();
    double delta = currentFrame - lastTime;
    lastTime = currentFrame;

    if (isPlaying) {
        position += delta;
    }
}

void Player::play() {
    setPlaying(true);

}

void Player::pause() {
    setPlaying(false);
}

void Player::stop() {
    setPlaying(false);
    position = 0;
}

time_unit Player::playerPosition() {
    return (time_unit) (position * TIME_UNITS);
}

void Player::seek(sm::time_unit time) {
    position = (float) time / TIME_UNITS;
    if (position < min_time) {
        position = min_time;
    }
    if (position > max_time) {
        position = max_time;
        isPlaying = false;
    }
}

bool Player::playing() { return isPlaying; }

void Player::togglePlay() {
    setPlaying(!isPlaying);
}

void Player::goBack() {
    position = backPosition;
}

void Player::setPlaying(bool playing) {
    if (playing && !isPlaying) {
        backPosition = position;
    }
    isPlaying = playing;
}
