#include "Player.h"
#include "Application.h"
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <unistd.h>
#include <AudioLoader.h>

using namespace sm;
using namespace std;

Player::Player() : playerThread(&Player::worker, this) {}

void Player::update(model::Canvas &canvas) {

    if (gApp->getResourceManager().needToBeUpdated(canvas.audioFile)) {
        std::unique_lock<std::mutex> lock(workMutex);
        media::AudioLoader &audio = gApp->getAudio();
        audio.close();
        audio.open(canvas.audioFile.str());
        gApp->getResourceManager().markAsLoaded(canvas.audioFile);
        audio.readAll();
        reopenDevice = true;
    }

    if (isPlaying) {
//        position += delta;
//        sampleTarget = (size_t) (position * audio.sampleRate);
//        int total = audio.sampleRate / 60;
//        if (afterSeek) {
//            afterSeek = false;
//            samplePosition = sampleTarget;
//        }
//        if (samplePosition < 0) samplePosition = 0;
//        while (samplePosition < sampleTarget) {
//            device.play(&audio.samples[samplePosition], total * 2);
//            samplePosition += total;
//        }
    }
}

void Player::worker() {
    using namespace std::chrono;
    using time_point = std::chrono::high_resolution_clock::time_point;
    using duration = std::chrono::high_resolution_clock::duration;

    // separated thread
    bool isPlaying = false;
    duration currentPosition = duration::zero();
    time_point lastTime = high_resolution_clock::now();
    size_t currentSample;
    size_t devicePosition = 0;
    float position;
    bool reopenDevice;

    media::AudioLoader &audio = gApp->getAudio();

    while (true) {
        // sync with master
        {
            std::unique_lock<std::mutex> lock(workMutex);
            if (Player::isPlaying && !isPlaying) {
                lastTime = high_resolution_clock::now();
            }
            isPlaying = Player::isPlaying && !Player::isSeeking;
            if (Player::afterSeek) {
                std::chrono::duration<float> pos(Player::position);
                currentPosition = duration_cast<duration>(pos);
            }
            if (isPlaying) {
                time_point now = high_resolution_clock::now();
                currentPosition += now - lastTime;
                lastTime = now;
            }
            Player::position = duration_cast<std::chrono::duration<float>>(currentPosition).count();
            if (Player::position < 0) Player::position = 0;
            position = Player::position;

            currentSample = (size_t) (position * audio.sampleRate);

            if (Player::afterSeek) {
                devicePosition = currentSample;
            }

            reopenDevice = Player::reopenDevice;
            Player::afterSeek = false;
            Player::reopenDevice = false;
        }

        if (reopenDevice) {
            device.open(audio.sampleRate, 1, 16);
            printf("Reopening device\n");
        }

        if (isPlaying) {
            int samplesToSend = audio.sampleRate / 50;
            while (devicePosition < currentSample + samplesToSend / 2) {
                device.play(&audio.samples[devicePosition], samplesToSend * 2);
                devicePosition += samplesToSend;
            }
        }

        this_thread::sleep_for(milliseconds(3));
    }
}

void Player::play() {
    setPlaying(true);
}

void Player::pause() {
    setPlaying(false);
}

void Player::stop() {
    std::unique_lock<std::mutex> lock(workMutex);
    setPlaying(false);
    position = 0;
}

time_unit Player::playerPosition() {
    return (time_unit) (position * TIME_UNITS);
}

void Player::seek(sm::time_unit time) {
    std::unique_lock<std::mutex> lock(workMutex);
    position = (float) time / TIME_UNITS;
    if (position < min_time) {
        position = min_time;
    }
    if (position > max_time) {
        position = max_time;
        isPlaying = false;
    }
    backPosition = position;
    afterSeek = true;
}

bool Player::playing() {
    return isPlaying;
}

void Player::togglePlay() {
    setPlaying(!isPlaying);
}

void Player::goBack() {
    std::unique_lock<std::mutex> lock(workMutex);
    position = backPosition;
    afterSeek = true;
}

void Player::setPlaying(bool playing) {
    std::unique_lock<std::mutex> lock(workMutex);
    if (playing && !isPlaying) {
        afterSeek = true;
    }
    isPlaying = playing;
}

void Player::setSeeking(bool seeking) {
    isSeeking = seeking;
}

void Player::reloadMedia() {
//    gApp->getAudio().close();
}
