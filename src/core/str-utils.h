#ifndef STR_UTILS_H
#define STR_UTILS_H

#include <string>
#include <algorithm>

namespace sm {

// https://stackoverflow.com/a/42844629/1119279
static bool endsWith(const std::string &str, const std::string &suffix) {
    return str.size() >= suffix.size() && 0 == str.compare(str.size() - suffix.size(), suffix.size(), suffix);
}

static bool startsWith(const std::string &str, const std::string &prefix) {
    return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
}

static void removeExtension(std::string &path) {
    std::replace(path.begin(), path.end(), '\\', '/');
    size_t posDot = path.rfind('.');
    size_t posSlash = path.rfind('/');
    if (posSlash > 0 && posDot > posSlash) path.resize(posDot);
}

}

#endif //STR_UTILS_H
