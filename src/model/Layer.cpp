#include <cassert>
#include <algorithm>
#include "Layer.h"

using namespace sm;
using namespace sm::project;

Layer::Layer() = default;

Layer::~Layer() = default;

std::shared_ptr<KeyPoint> Layer::addKey(sm::time_unit start, sm::time_unit duration) {
    assert(sanityCheck());
    auto ret = std::make_shared<KeyPoint>();
    ret->start = start;
    ret->duration = duration;
    addKey(ret);
    return ret;
}

void Layer::addKey(const std::shared_ptr<sm::project::KeyPoint> &key) {
    assert(!hasKey(key));
    keys.insert(keys.begin() + findIndex(key->start), key);
    assert(sanityCheck());
}

void Layer::removeKey(const std::shared_ptr<KeyPoint> &key) {
    int32_t idx = findIndex(key);
    if (idx < 0) {
        int t = 0;
    }
    assert(idx >= 0);
    keys.erase(keys.begin() + idx);
}

bool Layer::hasKey(const std::shared_ptr<sm::project::KeyPoint> &key) {
    assert(sanityCheck());
    return findIndex(key) >= 0;
}

bool Layer::sanityCheck() {
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

size_t Layer::findIndex(time_unit time) {
    assert(sanityCheck());
    return findIndexIt(time, 0, keys.size());
}

size_t Layer::findBefore(time_unit time) {
    assert(sanityCheck());
    size_t it = findIndex(time);
    while (it > 0 && keys[it - 1]->start + keys[it - 1]->duration > time) it--;
    return it;
}

float Layer::computeEasing(sm::time_unit time) {
    size_t index = findIndex(time) - 1;
    if (index < 0 || index >= keys.size()) return 0;
    auto &key = keys[index];
    return key->computeEasing(time - key->start);
}

size_t Layer::findIndexIt(time_unit time, size_t min, size_t max) {
    if (min >= max) return min;
    size_t midIdx = min + (max - min) / 2;
    auto &mid = keys[midIdx];
    if (time > mid->start) return findIndexIt(time, midIdx + 1, max);
    else return findIndexIt(time, min, midIdx);
}

void Layer::sortKeys() {
    std::stable_sort(keys.begin(), keys.end(), [](std::shared_ptr<KeyPoint> a, std::shared_ptr<KeyPoint> b) -> bool {
        return a->start < b->start;
    });
    assert(sanityCheck());
}

int32_t Layer::findIndex(const std::shared_ptr<KeyPoint> &key) {
    int32_t ret = static_cast<int>(findIndex(key->start));
    while (ret > 0 && ret < keys.size() && keys[ret]->start == keys[ret - 1]->start) {
        ret--;
    }
    while (ret >= 0 && ret < keys.size() - 1 && keys[ret] != key && keys[ret]->start == keys[ret + 1]->start) {
        ret++;
    }
    if (ret < keys.size() && keys[ret] == key) {
        return (int32_t) ret;
    }
    return -1;
}

void Layer::changeKeyStart(std::shared_ptr<KeyPoint> &key, time_unit newStart) {
    assert(sanityCheck());
    int32_t idx = findIndex(key);
    assert(idx >= 0);
    // remove
    keys.erase(keys.begin() + idx);
    // change start
    key->start = newStart;
    // readd
    keys.insert(keys.begin() + findIndex(key->start), key);
    assert(sanityCheck());
}
