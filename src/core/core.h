#ifndef CORE_H
#define CORE_H

#include "stdint.h"

namespace sm {

/// unit inside a second
const int TIME_UNITS = 1000;

/// a time in units
using time_unit = uint64_t;

const time_unit max_time = 100 * 60 * 60 * TIME_UNITS;

struct time_unwrapped {
    uint64_t mills;
    uint64_t seconds;
    uint64_t minutes;
    uint64_t hours;
};

time_unwrapped time_unwrap(time_unit in);
time_unit time_wrap(const time_unwrapped &in);

static const char *APPL_NAME = "Christmas Light Show Maker";

}

#endif //CORE_H
