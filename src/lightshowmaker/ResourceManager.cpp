#include "ResourceManager.h"
#include "use_glad.h"
#include <GLFW/glfw3.h>
#include <memory>
#include <fstream>

using namespace sm;

void ResourceManager::recheckAllResources() {
    recheckResourcesBefore = glfwGetTime();
}

bool ResourceManager::needToBeUpdated(Pathie::Path &filename) {
    return needToBeUpdated(filename, ctxs[filename]);
}

void ResourceManager::markAsLoaded(Pathie::Path &filename) {
    ResCtx &ctx = ctxs[filename];
    ctx.modificationTime = filename.mtime();
    ctx.lastLoadedFilename = filename;
}

size_t ResourceManager::loadTexture(Pathie::Path &filename) {
    ResCtx &ctx = ctxs[filename];
    auto &id = ctx.textureId;
    if (!id || needToBeUpdated(filename, ctx)) {
        if (!id) {
            GLuint texture = 0;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            id = (size_t) texture;
        }
        std::shared_ptr<media::Image> im = loadAsImage(filename, ctx);
        if (im) {
            glBindTexture(GL_TEXTURE_2D, (GLuint) id);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, im->width, im->height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                         im->pixels.data());
        }
    }
    return id;
}

bool ResourceManager::needToBeUpdated(Pathie::Path &filename, ResCtx &ctx) {
    if (ctx.lastCheck < recheckResourcesBefore) {
        ctx.lastCheck = recheckResourcesBefore;
        if (filename.mtime() != ctx.modificationTime) {
            return true;
        }
    }
    return false;
}

std::vector<uint8_t> ResourceManager::loadAsBinary(Pathie::Path filename, ResCtx &ctx) {
    ctx.modificationTime = filename.mtime();
    ctx.lastLoadedFilename = filename;

    std::ifstream file;
    file.open(filename.str(), std::ios_base::binary);
    std::vector<uint8_t> buffer(std::istreambuf_iterator<char>(file), {});
    return buffer;
}

std::shared_ptr<media::Image> ResourceManager::loadAsImage(Pathie::Path filename, ResCtx &ctx) {
    return media::decodeImage(loadAsBinary(filename, ctx));
}
