#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include <map>
#include <memory>
#include <vector>
#include <limits>
#include <cassert>
#include <sstream>
#include "utility"
#include "json/json.h"

#define SERIALIZATION_START template<SERIALIZATION_TYPE __type> void serialize(sm::Serializer<__type> &ser)

namespace sm {

enum SERIALIZATION_TYPE {
    SER_JSON,
    DESER_JSON,
};

template<SERIALIZATION_TYPE type>
class Serializer;

class SerializationContext {
    std::vector<void*> objects;
    std::map<void*, size_t> objToId;

    bool findShared(void *obj, size_t &idx) {
        auto it = objToId.find(obj);
        if (it != objToId.end()) {
            idx = it->second;
            return true;
        }
        return false;
    }

    size_t generateId(void *obj) {
        size_t idx = objects.size();
        objToId[obj] = idx;
        objects.push_back(obj);
        return idx;
    }

    void *findObj(size_t idx) {
        return idx < objects.size() ? objects[idx] : nullptr;
    }

    void registerObj(size_t idx, void *obj) {
        if (idx >= objects.size()) {
            objects.resize(idx + 1);
        }
        objects[idx] = obj;
        objToId[obj] = idx;
    }

    template<SERIALIZATION_TYPE type>
    friend class Serializer;
};

template<>
class Serializer<SER_JSON> {
    bool ownedCtx = false;
    SerializationContext *ctx;
    Json::Value jsonValue;

    Serializer(SerializationContext *ctx, Json::ValueType type = Json::nullValue) :
            ctx(ctx),
            jsonValue(type) {}

public:
    Serializer() {
        ctx = new SerializationContext();
        ownedCtx = true;
    }
    ~Serializer() {
        if (ownedCtx) {
            delete ctx;
        }
    }

    std::string toString() {
        Json::StreamWriterBuilder builder;
        builder.settings_["indentation"] = "  ";
        return Json::writeString(builder, jsonValue);
    }

private:
    template<typename C>
    void serializeHere(C &value) {
        value.serialize(*this);
    }

    template<typename C>
    void serializeHere(std::shared_ptr<C> &value) {
        size_t idx;
        if (ctx->findShared(value.get(), idx)) {
            jsonValue["__shared"] = idx;
            return;
        }
        jsonValue["__shared"] = ctx->generateId(value.get());
        serializeHere(*value.get());
    }

public:
    void serialize(const char *name, std::string &value) {
        jsonValue[name] = value;
    }

    void serialize(const char *name, uint32_t &value) {
        jsonValue[name] = value;
    }

    void serialize(const char *name, uint64_t &value) {
        jsonValue[name] = value;
    }

    void serialize(const char *name, int32_t &value) {
        jsonValue[name] = value;
    }

    void serialize(const char *name, int64_t &value) {
        jsonValue[name] = value;
    }

    template<typename C>
    void serialize(const char *name, std::vector<C> &value) {
        Json::Value array(Json::arrayValue);
        uint64_t size = value.size();
        if (size) {
            if ((uint32_t) size != size) throw "BIG NUMBER";
            array.resize(static_cast<uint32_t>(size));
            uint32_t i = 0;
            for (auto &it : value) {
                Serializer<SER_JSON> sub(ctx, Json::objectValue);
                sub.serializeHere(it);
                if (sub.jsonValue.size()) {
                    array[i] = sub.jsonValue;
                }
                i++;
            }
            if (array.size()) {
                jsonValue[name] = array;
            }
        }
    }

    template<typename C>
    void serializeEnum(const char *name, C &value) {
        jsonValue[name] = value;
    }

    // any other type
    template<typename C>
    void serialize(const char *name, C &value) {
        Serializer<SER_JSON> sub(ctx, Json::objectValue);
        sub.serializeHere(value);
        if (sub.jsonValue.size()) {
            jsonValue[name] = sub.jsonValue;
        }
    }

};

template<>
class Serializer<DESER_JSON> {
    bool ownedCtx = false;
    SerializationContext *ctx;
    Json::Value jsonValue;

    Serializer(SerializationContext *ctx, const Json::Value &jsonValue) :
            ctx(ctx),
            jsonValue(jsonValue) {}

public:
    Serializer(const std::string &data) {
        ctx = new SerializationContext();
        ownedCtx = true;
        std::stringstream sstr(data);
        sstr >> jsonValue;
    }

    ~Serializer() {
        if (ownedCtx) {
            delete ctx;
        }
    }


private:
    template<typename C>
    void deserializeHere(C &value) {
        value.serialize(*this);
    }

    template<typename C>
    void deserializeHere(std::shared_ptr<C> &value) {
        if (jsonValue.isMember("__shared")) {
            Json::UInt id = jsonValue["__shared"].asUInt();
            void *itm = ctx->findObj(id);
            value = std::shared_ptr<C>((C *) itm);
            if (!value) {
                value = std::make_shared<C>();
                ctx->registerObj(id, value.get());
            }
        } else {
            value = std::make_shared<C>();
        }
        deserializeHere(*value.get());
    }

public:
    void serialize(const char *name, std::string &value) {
        if (!jsonValue.isMember(name)) return;
        value = jsonValue[name].asString();
    }

    void serialize(const char *name, uint32_t &value) {
        if (!jsonValue.isMember(name)) return;
        value = jsonValue[name].asUInt();
    }

    void serialize(const char *name, uint64_t &value) {
        if (!jsonValue.isMember(name)) return;
        value = jsonValue[name].asUInt64();
    }

    void serialize(const char *name, int32_t &value) {
        if (!jsonValue.isMember(name)) return;
        value = jsonValue[name].asInt();
    }

    void serialize(const char *name, int64_t &value) {
        if (!jsonValue.isMember(name)) return;
        value = jsonValue[name].asInt64();
    }

    template<typename C>
    void serialize(const char *name, std::vector<C> &value) {
        if (!jsonValue.isMember(name)) return;
        Json::Value &array = jsonValue[name];
        uint32_t size = array.size();
        if (size) {
            value.resize(size);
            uint32_t i = 0;
            for (auto &it : value) {
                Serializer<DESER_JSON> sub(ctx, array[i]);
                sub.deserializeHere(it);
                i++;
            }
        }
    }

    template<typename C>
    void serializeEnum(const char *name, C &value) {
        if (!jsonValue.isMember(name)) return;
        value = (C) jsonValue[name].asUInt();
    }

    // any other type
    template<typename C>
    void serialize(const char *name, C &value) {
        if (!jsonValue.isMember(name)) return;
        Serializer<DESER_JSON> sub(ctx, jsonValue[name]);
        sub.deserializeHere(value);
    }

};



}

#endif //SERIALIZATION_H
