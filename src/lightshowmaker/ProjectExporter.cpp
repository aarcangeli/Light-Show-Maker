#include <utility>

#include "ProjectExporter.h"
#include "stdint.h"
#include "set"

using namespace sm;
using namespace sm::model;

void ProjectExporter::exportIno(std::shared_ptr<sm::model::Project> _proj, std::string filename) {
    proj = std::move(_proj);
    exportCommons(filename);

    std::set<model::arduino_number> tabooList;
    for (auto &layer : proj->canvas.groups) {
        auto it = tabooList.find(layer->number);
        if (it == tabooList.end()) {
            tabooList.insert(layer->number);
            exportChunk(filename, layer->number);
        }
    }

    proj = nullptr;
}

void ProjectExporter::exportChunk(std::string filename, model::arduino_number number) const {
    dirName(filename);
    std::string numberStr = std::to_string(number);
    filename += "/lsm_" + numberStr + ".h";
    std::ofstream outputFile(filename);

    outputFile << std::string("// Auto-generated with ") + APPL_NAME + "\n";
    outputFile << "\n";
    outputFile << "#ifndef __LSM_" + numberStr + "_H\n";
    outputFile << "#define __LSM_" + numberStr + "_H\n\n";

    int count = 0;
    for (auto &layer : proj->canvas.groups) {
        if (layer->number == number) {
            count++;
        }
    }

    outputFile << "#include \"lsm_common.h\"\n\n";

    size_t maxLetters = 0;
    for (auto &layer : proj->canvas.groups) {
        if (layer->number != number) continue;
        maxLetters = std::max(maxLetters, layer->identifier.size());
    }
    size_t i = 0;
    for (auto &layer : proj->canvas.groups) {
        if (layer->number != number) continue;
        if (!layer->identifier.empty()) {
            size_t letterCount = maxLetters - layer->identifier.size();
            outputFile << "#define LAYER_" << layer->identifier << " ";
            for (size_t j = 0; j < letterCount; j++) {
                outputFile << " ";
            }
            outputFile << std::to_string(i) << "\n";
        }
        i++;
    }
    outputFile << "\n";

    outputFile << "namespace lsm {\n\n";
    outputFile << "const unsigned long LIGHT_COUNT = " << count << ";\n\n";

    i = 0;
    outputFile << "const unsigned int ALL_LAYERS[" << count << "] = {\n";
    for (auto &layer : proj->canvas.groups) {
        if (layer->number != number) continue;
        if (!layer->identifier.empty()) {
            outputFile << "        LAYER_" << layer->identifier << ",";
            outputFile << "\n";
        }
        i++;
    }
    outputFile << "};\n\n";

    i = 0;
    for (auto &g : proj->canvas.groups) {
        if (g->number != number) continue;
        outputFile << "// " << g->name << "\n";
        outputFile << "const PROGMEM Key_Compact __keys__" << i << "[" << g->keys.size() << "] = {\n";
        for (auto &k : g->keys) {
            outputFile << "    {\n";
            outputFile << "        " << k->start << ",\n";
            outputFile << "        " << k->duration << ",\n";
            outputFile << "        " << uint32_t(k->maxWeight * 255) << ",\n";
            outputFile << "        " << k->fadeStart.type << ", " << uint32_t(k->fadeStart.exponent * 255) << ",\n";
            outputFile << "        " << k->fadeStart.duration << ",\n";
            outputFile << "        " << k->fadeStart.duration << ",\n";
            outputFile << "        " << k->fadeEnd.type << ", " << uint32_t(k->fadeEnd.exponent * 255) << ",\n";
            outputFile << "        " << std::max(0, k->duration - k->fadeEnd.duration) << ",\n";
            outputFile << "        " << k->fadeEnd.duration << ",\n";
            outputFile << "    },\n";
        }
        outputFile << "};\n\n";
        i++;
    }

    i = 0;
    outputFile << "const LightGroup groups[LIGHT_COUNT] = {\n";
    for (auto &g : proj->canvas.groups) {
        if (g->number != number) continue;
        outputFile << "    {" << g->keys.size() << ", __keys__" << i << "}, // " << g->name << "\n";
        i++;
    }
    outputFile << "};";

    outputFile << R"(

LightContext ctxs[LIGHT_COUNT];

void updateAlpha(time_unit position) {
    for (int i = 0; i < LIGHT_COUNT; i++) {
        const LightGroup &group = groups[i];
        LightContext &ctx = ctxs[i];
        ctx.alpha = 0;
        int &index = ctx.index;
        while (index < 0 || (index < group.keysCount && position > ctx.current.end)) {
            index++;
            Key_Compact destKey;
            const char *source = (char*) &group.keys[index];
            char *dest = (char*) &destKey;
            for (int j = 0; j < sizeof(Key_Compact); j++) {
                dest[j] = pgm_read_byte_near(&source[j]);
            }
            time_unit start = destKey.start;
            ctx.current.start = start;
            ctx.current.end = start + destKey.duration;
            ctx.current.maxWeight = destKey.maxWeight / 255.f;
            ctx.current.fade1.type = destKey.fade1_type;
            ctx.current.fade1.exponent = destKey.fade1_exponent / 255.f;
            ctx.current.fade1_end = start + destKey.fade1_end;
            ctx.current.fade1_duration = destKey.fade1_duration;
            ctx.current.fade2.type = destKey.fade2_type;
            ctx.current.fade2.exponent = destKey.fade2_exponent / 255.f;
            ctx.current.fade2_start = start + destKey.fade2_start;
            ctx.current.fade2_duration = destKey.fade2_duration;
        }
        if (index >= group.keysCount) continue;
        Key &key = ctx.current;
        ctx.alpha = computeEasing(key, position);
    }
}

)";

    outputFile << "} // namespace lsm\n\n";

    outputFile << "#endif //__LSM_" + numberStr + "_H\n";
}

void ProjectExporter::exportCommons(std::string filename) {
    dirName(filename);
    filename += "/lsm_common.h";
    std::ofstream outputFile(filename);

    outputFile << std::string("// Auto-generated with ") + APPL_NAME + "\n";
    outputFile << R"(
#ifndef __LSM_COMMON_H
#define __LSM_COMMON_H

namespace lsm {

// 32 bit unsigned
typedef unsigned long time_unit;
// 16 bit unsigned
typedef unsigned int time_unit_compact;

// FADE_TYPE
const unsigned char LINEAR = 0;
const unsigned char EXPONENTIAL = 1;
const unsigned char SIN = 2;
const unsigned char SIN_DOUBLE = 3;

struct FadeParams {
    unsigned char type;
    float exponent;
};

struct Key {
    // key
    time_unit start;
    time_unit end;
    float maxWeight;

    // fade start
    FadeParams fade1;
    time_unit fade1_end;
    float fade1_duration;

    // fade end
    FadeParams fade2;
    time_unit fade2_start;
    float fade2_duration;
};

struct Key_Compact {
    // key
    time_unit start;
    time_unit_compact duration;
    unsigned char maxWeight;

    // fade start
    unsigned char fade1_type;
    unsigned int fade1_exponent;
    time_unit_compact fade1_end; // relative to start
    time_unit_compact fade1_duration;

    // fade end
    unsigned char fade2_type;
    unsigned int fade2_exponent;
    time_unit_compact fade2_start; // relative to start
    time_unit_compact fade2_duration;
};

struct LightGroup {
    unsigned int keysCount;
    const Key_Compact *keys;
};

struct LightContext {
    Key current;
    int index = -1;
    long cycles;
    float alpha;
    bool on;
};

float computeEasing(FadeParams &params, float alpha) {
    switch (params.type) {
        case LINEAR:
            return alpha;
        case EXPONENTIAL:
            return pow(alpha, params.exponent);
        case SIN:
            return (float) sin(alpha * M_PI_2);
        case SIN_DOUBLE:
            return (float) ((1 + sin((alpha - 0.5) * M_PI)) / 2);
        default:
            return 1;
    }
}

float computeEasing(Key &key, time_unit pos) {
    if (pos < key.start || pos > key.end) return 0;
    time_unit diff1 = pos - key.fade1_end;
    if (pos < key.fade1_end) {
        float alpha = (pos - key.start) / (float)key.fade1_duration;
        return computeEasing(key.fade1, alpha) * key.maxWeight;
    }
    if (pos > key.fade2_start) {
        float alpha = (pos - key.fade2_start) / (float)key.fade2_duration;
        return computeEasing(key.fade2, 1 - alpha) * key.maxWeight;
    }
    return key.maxWeight;
}


void updateAlpha(time_unit position);

} // namespace lsm

#endif //__LSM_H
)";

}
