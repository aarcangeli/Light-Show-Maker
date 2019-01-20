#include "core.h"

using namespace sm;

time_unwrapped sm::time_unwrap(time_unit time) {
    static_assert(TIME_UNITS == 1000, "edit this code");
    time_unwrapped ret{};
    ret.mills = static_cast<int32_t>(time % 1000);
    time /= 1000;
    ret.seconds = static_cast<int32_t>(time % 60);
    time /= 60;
    ret.minutes = static_cast<int32_t>(time % 60);
    time /= 60;
    ret.hours = static_cast<int32_t>(time % 60);
    return ret;
}

time_unit sm::time_wrap(const time_unwrapped &in) {
    static_assert(TIME_UNITS == 1000, "edit this code");
    time_unit ret = in.hours;
    ret *= 60;
    ret += in.minutes;
    ret *= 60;
    ret += in.seconds;
    ret *= 1000;
    ret += in.mills;
    return ret;
}

int profiler::indent = 0;
