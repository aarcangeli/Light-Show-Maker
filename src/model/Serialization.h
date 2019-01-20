#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include <map>
#include <memory>
#include <vector>
#include <limits>
#include <cassert>
#include <sstream>
#include <path.hpp>
#include "utility"
#include "json/json.h"
#include "base64.h"

#define SERIALIZATION_START template<SERIALIZATION_TYPE __type> void serialize(sm::Serializer<__type> &ser)

namespace sm {

enum SERIALIZATION_TYPE {
    SER_JSON,
    DESER_JSON,
};

template<SERIALIZATION_TYPE type>
class Serializer;

class SerializationContext {
public:
    std::vector<void *> objects;
    std::map<void *, size_t> objToId;

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

    Pathie::Path basePath;
};

class SerializationCommons {
protected:
    bool ownedCtx = false;
    SerializationContext *ctx;

public:
    Json::Value jsonValue;

    ~SerializationCommons() {
        if (ownedCtx) {
            delete ctx;
        }
    }

    void setBasePath(const Pathie::Path &basePath) {
        ctx->basePath = basePath;
    }

    const Pathie::Path &getBasePath() {
        return ctx->basePath;
    }
};

template<>
class Serializer<SER_JSON> : public SerializationCommons {
    Serializer(SerializationContext *_ctx, Json::ValueType _jsonValue = Json::nullValue) {
        ctx = _ctx;
        jsonValue = _jsonValue;
    }

public:
    const bool SERIALIZING = true;
    const bool DESERIALIZING = false;

    Serializer() {
        ctx = new SerializationContext();
        ownedCtx = true;
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

    void serialize(const char *name, float &value) {
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

    void serializeBinary(const char *name, std::vector<uint8_t> &value) {
        std::string string = base64_encode(value.data(), value.size());
        serialize(name, string);
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

    bool hasKey(const char *name) {
        return jsonValue.isMember(name);
    }

};

template<>
class Serializer<DESER_JSON> : public SerializationCommons {

    Serializer(SerializationContext *_ctx, const Json::Value &_jsonValue) {
        ctx = _ctx;
        jsonValue = _jsonValue;
    }

public:
    const bool SERIALIZING = false;
    const bool DESERIALIZING = true;

    Serializer(const std::string &data) {
        ctx = new SerializationContext();
        ownedCtx = true;
        std::stringstream sstr(data);
        sstr >> jsonValue;
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
            if (itm) {
                value = *(std::shared_ptr<C> *) itm;
            } else {
                value = nullptr;
            }
            if (!value) {
                value = std::make_shared<C>();
                ctx->registerObj(id, (void *) &value);
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

    void serialize(const char *name, float &value) {
        if (!jsonValue.isMember(name)) return;
        value = jsonValue[name].asFloat();
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

    void serializeBinary(const char *name, std::vector<uint8_t> &value) {
        std::string string;
        serialize(name, string);
        std::string decoded = base64_decode(string);
        value.resize(decoded.size());
        memcpy(value.data(), decoded.data(), decoded.size());
    }

    // any other type
    template<typename C>
    void serialize(const char *name, C &value) {
        if (!jsonValue.isMember(name)) return;
        Serializer<DESER_JSON> sub(ctx, jsonValue[name]);
        sub.deserializeHere(value);
    }

    bool hasKey(const char *name) {
        return jsonValue.isMember(name);
    }
};

template<typename T>
std::string serializeObject(const T obj, const std::string &filename, bool undo) {
    Serializer<SER_JSON> serializer;
    if (!undo) {
        serializer.setBasePath(Pathie::Path(filename).parent());
    }
    obj->serialize(serializer);
    std::string result;
    Json::StreamWriterBuilder builder;
    if (!undo) builder.settings_["indentation"] = "  ";
    result = writeString(builder, serializer.jsonValue);
    return result;
}

template<typename T>
std::shared_ptr<T> deserializeObject(const std::string &str, const std::string &filename) {
    Serializer<DESER_JSON> serializer(str);
    serializer.setBasePath(Pathie::Path(filename).parent());
    std::shared_ptr<T> obj = std::make_shared<T>();
    obj->serialize(serializer);
    return obj;
}

}

#endif //SERIALIZATION_H
