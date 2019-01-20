#ifndef DECORATION_H
#define DECORATION_H

#include "Serialization.h"
#include "SelectableItem.h"
#include "fstream"
#include "memory"

namespace sm {
namespace model {

enum DecorationType {
    LIGHT,
    IMAGE,
};

class Decoration : public SelectableItem<Decoration> {
public:
    Decoration();

    DecorationType type;

    // box
    float posX, posY;

    // image
    Pathie::Path image;
    float width, height;
    float ratio = -1;

    // light
    uint32_t color;
    float size;

    SERIALIZATION_START {
        SelectableItem::serializeBase(ser);
        ser.serializeEnum("type", type);
        ser.serialize("posX", posX);
        ser.serialize("posY", posY);
        ser.serialize("width", width);
        ser.serialize("height", height);
        ser.serialize("ratio", ratio);

        // legacy image serializer
        if (type == IMAGE && ser.DESERIALIZING && ser.hasKey("png")) {
            std::vector<uint8_t> bytes;
            ser.serializeBinary("png", bytes);
//            std::shared_ptr<media::Image> image = media::decodeImage(bytes);
//            if (!image) {
//                throw "Cannot decode image";
//            }
            // save on disk
            Pathie::Path assetDir = ser.getBasePath().join("assets");
            Pathie::Path outputPath;
            int i = 0;
            do {
                outputPath = assetDir.join("decoration-" + std::to_string(i) + ".png");
                i++;
            } while (outputPath.exists());
            image = outputPath;
            outputPath.parent().mktree();
            std::ofstream file;
            file.open(outputPath.str(), std::ios_base::binary);
            file.write((char *) bytes.data(), bytes.size());
            file.close();
        }

        ser.serialize("image", image);

        if (type == LIGHT) {
            ser.serialize("color", color);
            ser.serialize("size", size);
        }
    }
};

}
}

#endif //DECORATION_H
