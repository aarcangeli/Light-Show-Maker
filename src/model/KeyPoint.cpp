#include "KeyPoint.h"
#include "math.h"

using namespace sm::model;

KeyPoint::KeyPoint() = default;

float KeyPoint::computeEasing(sm::time_unit time) {
    if (time < 0 || time >= duration) return 0;
    if (time <= fadeStart.duration) {
        return computeFade(false, fadeStart, time) * maxWeight;
    }
    if (time >= duration - fadeEnd.duration) {
        return computeFade(true, fadeEnd, fadeEnd.duration - (time - duration + fadeEnd.duration)) * maxWeight;
    }
    return maxWeight;
}

float KeyPoint::computeFade(bool isRight, const Fade &fade, sm::time_unit time) const {
    time_unit dur = fade.duration;
    if (isRight && dur > duration - fadeStart.duration) {
        dur = duration - fadeStart.duration;
    }
    float alpha = (float) time / dur;
    switch (fade.type) {
        case LINEAR:
            return alpha;
        case EXPONENTIAL:
            return pow(alpha, fade.exponent);
        case SIN:
            return (float) sin(alpha * M_PI_2);
        case SIN_DOUBLE:
            return (float) ((1 + sin((alpha - 0.5) * M_PI)) / 2);
    }
    return 0;
}
