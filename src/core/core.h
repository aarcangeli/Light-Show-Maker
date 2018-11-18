#ifndef CORE_H
#define CORE_H

#include "stdint.h"

namespace sm {

/// unit inside a second
const int TIME_UNITS = 1000;

/// a time in units
using time_unit = uint64_t;

static const char *APPL_NAME = "Christmas Light Show Maker";

}

#endif //CORE_H
