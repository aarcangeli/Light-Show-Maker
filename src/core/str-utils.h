#ifndef STR_UTILS_H
#define STR_UTILS_H

#include <string>

// https://stackoverflow.com/a/42844629/1119279
static bool endsWith(const std::string &str, const std::string &suffix) {
    return str.size() >= suffix.size() && 0 == str.compare(str.size() - suffix.size(), suffix.size(), suffix);
}

static bool startsWith(const std::string &str, const std::string &prefix) {
    return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
}

#endif //STR_UTILS_H
