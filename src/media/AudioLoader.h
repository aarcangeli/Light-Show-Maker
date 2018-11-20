#ifndef AUDIOLOADER_H
#define AUDIOLOADER_H

#include "string"

namespace sm {
namespace media {

class AudioLoader {
public:
    AudioLoader();
    ~AudioLoader();

    void startDecoding(std::string filename);
};

}
}

#endif //AUDIOLOADER_H
