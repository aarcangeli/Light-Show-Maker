#include "ResourceManager.h"
#include <GLFW/glfw3.h>

using namespace sm;

void ResourceManager::recheckAllResources() {
    recheckResourcesBefore = glfwGetTime();
}

bool ResourceManager::needToBeUpdated(sm::model::ExternalResource &res) {
    if (res.lastCheck < recheckResourcesBefore) {
        res.lastCheck = recheckResourcesBefore;
        return res.needToBeUpdated();
    }
    return false;
}
