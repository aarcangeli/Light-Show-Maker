#ifndef EXTERNALRESOURCE_H
#define EXTERNALRESOURCE_H

#include "string"
#include "vector"

namespace sm {
namespace model {

// a reference to an external file (an image, sound ecc...)
// a resource will never be saved by LSM
class ExternalResource {
public:
    std::string absoluteFilename;

    std::vector<uint8_t> load();
};

}
}

#endif //EXTERNALRESOURCE_H
