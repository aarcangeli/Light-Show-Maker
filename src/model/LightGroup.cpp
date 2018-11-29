#include <cassert>
#include <algorithm>
#include "LightGroup.h"

using namespace sm;
using namespace sm::project;

LightGroup::LightGroup() = default;

LightGroup::~LightGroup() {
    int t = 0;
}

std::shared_ptr<KeyPoint> LightGroup::addKey(sm::time_unit start, sm::time_unit duration) {
    assert(sanityCheck());
    auto ret = std::make_shared<KeyPoint>();
    ret->start = start;
    ret->duration = duration;
    addKey(ret);
    return ret;
}

void LightGroup::addKey(const std::shared_ptr<sm::project::KeyPoint> &key) {
    assert(!hasKey(key));
    keys.insert(keys.begin() + findIndex(key->start), key);
    assert(sanityCheck());
}

void LightGroup::removeKey(const std::shared_ptr<KeyPoint> &key) {
    int32_t idx = findIndex(key);
    assert(idx >= 0);
    keys.erase(keys.begin() + idx);
}

bool LightGroup::hasKey(const std::shared_ptr<sm::project::KeyPoint> &key) {
    assert(sanityCheck());
    return findIndex(key) >= 0;
}

bool LightGroup::sanityCheck() {
    if (!keys.empty()) {
        time_unit last = keys[0]->start;
        for (auto &it : keys) {
            if (last > it->start) {
                return false;
            }
            last = it->start;
        }
    }
    return true;
}

size_t LightGroup::findIndex(time_unit time) {
    assert(sanityCheck());
    return findIndexIt(time, 0, keys.size());
}

size_t LightGroup::findIndexIt(time_unit time, size_t min, size_t max) {
    if (min >= max) return min;
    size_t midIdx = min + (max - min) / 2;
    auto &mid = keys[midIdx];
    if (time > mid->start) return findIndexIt(time, midIdx + 1, max);
    else return findIndexIt(time, min, midIdx);
}

void LightGroup::sortKeys() {
    std::stable_sort(keys.begin(), keys.end(), [](std::shared_ptr<KeyPoint> a, std::shared_ptr<KeyPoint> b) -> bool {
        return a->start < b->start;
    });
    assert(sanityCheck());
}

int32_t LightGroup::findIndex(const std::shared_ptr<KeyPoint> &key) {
    size_t ret = findIndex(key->start);
    if (ret < keys.size() && keys[ret] == key && (int32_t) ret == ret) {
        return (int32_t) ret;
    }
    return -1;
}
