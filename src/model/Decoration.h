#ifndef DECORATION_H
#define DECORATION_H

#include "Serialization.h"
#include "ImageLoader.h"
#include "memory"

namespace sm {
namespace model {

enum DecorationType {
    LIGHT,
    IMAGE,
};

class Decoration {
public:
    Decoration();

    DecorationType type;

    // box
    float posX, posY;

    // image
    std::shared_ptr<media::Image> image;
    float width, height;
    float ratio = -1;

    // light
    uint32_t color;
    float size;

    // volatile - hold texture
    void* textureId = nullptr;

    // volatile
    bool isSelected = false;

    SERIALIZATION_START {
        ser.serializeEnum("type", type);
        ser.serialize("posX", posX);
        ser.serialize("posY", posY);
        ser.serialize("width", width);
        ser.serialize("height", height);
        ser.serialize("ratio", ratio);

        if (type == IMAGE) {
            std::vector<uint8_t> bytes;
            if (ser.SERIALIZING) {
                if (!image) image = std::make_shared<media::Image>();
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
        if (type == LIGHT) {
            ser.serialize("color", color);
            ser.serialize("size", size);
        }
    }
};

}
}

#endif //DECORATION_H
