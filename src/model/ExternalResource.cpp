#include <fstream>
#include "ExternalResource.h"

using namespace sm;
using namespace sm::model;

Pathie::Path ExternalResource::makeRelative(Pathie::Path filename, Pathie::Path baseDir) {
    filename = filename.absolute();
    baseDir = baseDir.absolute();

#ifdef _WIN32
    {
        // different drive
        std::string filenameStr = filename.str();
        std::string baseDirStr = baseDir.str();
        if (filenameStr.length() && baseDirStr.length() && filenameStr[0] != baseDirStr[0]) {
            return filename;
        }
    }
#endif

    size_t pos = 0;
    size_t countFilename = filename.component_count();
    size_t countBaseDir = baseDir.component_count();

    Pathie::Path str1 = filename[pos];
    Pathie::Path str2 = baseDir[pos];
    while (pos < countFilename && pos < countBaseDir && filename[pos] == baseDir[pos]) {
        Pathie::Path str1 = filename[pos];
        Pathie::Path str2 = baseDir[pos];
        pos++;
    }

    Pathie::Path result;

    size_t countParent = countBaseDir - pos;
    for (size_t i = 0; i < countParent; ++i) {
        result = result.join("..");
    }

    while (pos < countFilename) {
        result = result.join(filename[pos]);
        pos++;
    }

    return result.prune();
}
