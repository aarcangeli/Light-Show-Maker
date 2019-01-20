#ifndef EXTERNALRESOURCE_H
#define EXTERNALRESOURCE_H

#include <path.hpp>
#include "string"
#include "vector"
#include "Serialization.h"
#include "ImageLoader.h"

namespace sm {
namespace model {

// a reference to an external file (an image, sound ecc...)
// a resource will never be saved by LSM
class ExternalResource {
public:
    Pathie::Path filename;

    SERIALIZATION_START {
        Pathie::Path projectPath = ser.getBasePath();
        if (ser.DESERIALIZING) {
            std::string relativeFilename;
            ser.serialize("filename", relativeFilename);
            filename = projectPath.join(relativeFilename).prune();
        }
        if (ser.SERIALIZING && filename.str() != ".") {
            // make relative
            std::string relativeStr = makeRelative(filename, projectPath).str();
            ser.serialize("filename", relativeStr);
        }
    }

private:
    Pathie::Path makeRelative(Pathie::Path filename, Pathie::Path baseDir);
};

}
}

#endif //EXTERNALRESOURCE_H
