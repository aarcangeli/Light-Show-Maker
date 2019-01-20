#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include "ExternalResource.h"
#include "map"
#include "pathie.hpp"

namespace sm {

class ResourceManager {
    double recheckResourcesBefore = 0;

public:
    void recheckAllResources();
    size_t loadTexture(Pathie::Path &filename);

private:
    struct ResCtx {
        double lastCheck = -1;
        time_t modificationTime;
        Pathie::Path lastLoadedFilename;
        size_t textureId = 0;
    };
    std::map<Pathie::Path, ResCtx> ctxs;

    bool needToBeUpdated(Pathie::Path &filename, ResCtx &ctx);
    std::vector<uint8_t> loadAsBinary(Pathie::Path filename, ResCtx &ctx);
    std::shared_ptr<media::Image> loadAsImage(Pathie::Path filename, ResCtx &ctx);
};

}

#endif //RESOURCEMANAGER_H
