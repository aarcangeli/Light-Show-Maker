#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include "ExternalResource.h"

namespace sm {

class ResourceManager {
    double recheckResourcesBefore = 0;

public:
    void recheckAllResources();
    bool needToBeUpdated(sm::model::ExternalResource &res);
};

}

#endif //RESOURCEMANAGER_H
