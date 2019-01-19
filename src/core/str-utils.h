#ifndef STR_UTILS_H
#define STR_UTILS_H

#include <string>
#include <algorithm>
#include <codecvt>
#include <string>
#include <locale>

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

}

#endif //STR_UTILS_H
