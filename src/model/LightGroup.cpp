#include <cassert>
#include "LightGroup.h"

using namespace sm;
using namespace sm::project;

LightGroup::LightGroup(Canvas *canvas) : canvas(canvas) {
}

LightGroup::~LightGroup() {
    int t = 0;
}

std::shared_ptr<KeyPoint> LightGroup::addKey(sm::time_unit start, sm::time_unit duration) {
    assert(sanityCheck());
    auto ret = std::make_shared<KeyPoint>(this);
    ret->start = start;
    ret->duration = duration;
    keys.insert(keys.begin() + findIndex(start), ret);
    assert(sanityCheck());
    return ret;
}

bool LightGroup::sanityCheck() {
    time_unit last = 0;
    for (auto &it : keys) {
        assert(last <= it->start);
        assert(it->duration > 0);
        last = it->start;
    }
    return true;
}

size_t LightGroup::findIndex(time_unit time) {
    return findIndexIt(time, 0, keys.size());
}

size_t LightGroup::findIndexIt(time_unit time, size_t min, size_t max) {
    if (min >= max) return min;
    size_t midIdx = min + (max - min) / 2;
    auto &mid = keys[midIdx];
    if (time > mid->start) return findIndexIt(time, midIdx + 1, max);
    else return findIndexIt(time, min, midIdx);
}
