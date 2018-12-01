#ifndef DECORATION_H
#define DECORATION_H

#include "Serialization.h"
#include "ImageLoader.h"
#include "memory"

namespace sm {
namespace project {

class Decoration {
public:
    Decoration();

    std::shared_ptr<media::Image> image;

    SERIALIZATION_START {
        std::vector<uint8_t> bytes;
        if (ser.SERIALIZING) {
            if (!image) image = std::make_shared<media::Image>();
            ser.serialize("width", image->width);
            ser.serialize("height", image->height);
            bytes = media::encodeImage(image);
            if (bytes.empty()) {
                throw "Cannot encode image";
            }
        }
        ser.serializeBinary("png", bytes);
        if (ser.DESERIALIZING) {
            image = media::decodeImage(bytes);
            if (!image) {
                throw "Cannot decode image";
            }
        }
    }
};

}
}

#endif //DECORATION_H
