#ifndef STR_UTILS_H
#define STR_UTILS_H

#include <string>
#include <algorithm>
#include <codecvt>
#include <string>
#include <locale>
#include <path.hpp>

namespace sm {

// https://stackoverflow.com/a/42844629/1119279
static bool endsWith(const std::string &str, const std::string &suffix) {
    return str.size() >= suffix.size() && 0 == str.compare(str.size() - suffix.size(), suffix.size(), suffix);
}

static bool startsWith(const std::string &str, const std::string &prefix) {
    return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
}

static void dirName(std::string &path) {
    std::replace(path.begin(), path.end(), '\\', '/');
    size_t posSlash = path.rfind('/');
    path.resize(posSlash);
}

static void removeExtension(std::string &path) {
    std::replace(path.begin(), path.end(), '\\', '/');
    size_t posDot = path.rfind('.');
    size_t posSlash = path.rfind('/');
    if (posSlash > 0 && posDot > posSlash) path.resize(posDot);
}

// convert UTF-8 string to wstring
static std::wstring utf8_to_wstring(const std::string &str) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv1;
    return conv1.from_bytes(str);
}

// convert wstring to UTF-8 string
static std::string wstring_to_utf8(const std::wstring &str) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv1;
    return conv1.to_bytes(str);
}

static Pathie::Path makeRelative(Pathie::Path filename, Pathie::Path baseDir) {
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

    // skip common pathes
    while (pos < countFilename && pos < countBaseDir && filename[pos] == baseDir[pos]) {
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

}

#endif //STR_UTILS_H
