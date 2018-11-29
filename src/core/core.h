#ifndef CORE_H
#define CORE_H

#include "stdint.h"

namespace sm {

class Application;

extern Application *gApp;

/// unit inside a second
const int TIME_UNITS = 1000;

/// a time in units
using time_unit = int64_t;

const time_unit min_time = 0;

const time_unit max_time = 100 * 60 * 60 * TIME_UNITS;

struct time_unwrapped {
    int32_t mills;
    int32_t seconds;
    int32_t minutes;
    int32_t hours;
};

// PushFont(GetIO().Fonts->Fonts[FONT_BIG]);
// PopFont();
#define FONT_NORMAL 0
#define FONT_BIG 1

time_unwrapped time_unwrap(time_unit in);
time_unit time_wrap(const time_unwrapped &in);

static const char *APPL_NAME = "Christmas Light Show Maker";

}

#endif //CORE_H
