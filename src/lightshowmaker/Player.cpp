#include "Player.h"
#include "Application.h"
#include <GLFW/glfw3.h>
#include <imgui.h>

using namespace sm;

void Player::update(model::Canvas &canvas) {
    double currentFrame = glfwGetTime();
    double delta = currentFrame - lastTime;
    lastTime = currentFrame;
    media::AudioLoader &audio = gApp->getAudio();

    if (justPlayed) {
        if (!audio.isOpen()) {
            audio.open(canvas.audioFile.str());
            if (audio.isOpen()) {
                device.open(audio.sampleRate, 1, 16);
            }
            audio.readAllSamples();
        }
        justPlayed = false;
    }

    if (isPlaying) {
        position += delta;
        sampleTarget = (size_t) (position * audio.sampleRate);
        int total = audio.sampleRate / 60;
        if (afterSeek) {
            afterSeek = false;
            samplePosition = sampleTarget;
        }
        if (samplePosition < 0) samplePosition = 0;
        while (samplePosition < sampleTarget) {
            device.play(&audio.samples[samplePosition], total * 2);
            samplePosition += total;
        }
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
    afterSeek = true;
}

bool Player::playing() { return isPlaying; }

void Player::togglePlay() {
    setPlaying(!isPlaying);
}

void Player::goBack() {
    position = backPosition;
    afterSeek = true;
}

void Player::setPlaying(bool playing) {
    if (playing && !isPlaying) {
        backPosition = position;
        justPlayed = true;
    }
    isPlaying = playing;
}

void Player::reloadMedia() {
    gApp->getAudio().close();
}
