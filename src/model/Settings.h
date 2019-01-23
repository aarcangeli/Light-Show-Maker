#ifndef SETTINGS_H
#define SETTINGS_H

#include "Serialization.h"

namespace sm {
namespace model {

class Settings {
public:
    explicit Settings();

    int width = 1920;
    int height = 1080;

    SERIALIZATION_START {
        ser.serialize("width", width);
        ser.serialize("height", height);
    }
};

}
}

#endif //SETTINGS_H
