#pragma once
#include "jsonparse.hpp"
#include "jsontok.hpp"

#include <stdexcept>
#include <string>

namespace json {

class Json {
private:
    jsonparse::JPtr root;

    static void require(bool cond, const char* msg) {
        if (!cond) throw std::runtime_error(msg);
    }

    jsonparse::JsonObject* asObjectPtr() const {
        require(root && root->getObjType() == jsonparse::JsonObjectType::OBJECT,
                "Not a JsonObject");
        return static_cast<jsonparse::JsonObject*>(root.get());
    }

    jsonparse::JsonArray* asArrayPtr() const {
        require(root && root->getObjType() == jsonparse::JsonObjectType::ARRAY,
                "Not a JsonArray");
        return static_cast<jsonparse::JsonArray*>(root.get());
    }

    jsonparse::JsonLiteral* asLiteralPtr() const {
        require(root && root->getObjType() == jsonparse::JsonObjectType::LITERAL,
                "Not a JsonLiteral");
        return static_cast<jsonparse::JsonLiteral*>(root.get());
    }

public:
    explicit Json(jsonparse::JPtr p) : root(p) {}

    explicit Json(const std::string& fileName) {
        jsontok::JsonOnDemandTokenizer tokenizer(fileName);
        root = jsonparse::JsonParser::startParsing(tokenizer);
        require(root != nullptr, "Parsing failed");
    }

    bool isObject() const {
        return root && root->getObjType() == jsonparse::JsonObjectType::OBJECT;
    }

    bool isArray() const {
        return root && root->getObjType() == jsonparse::JsonObjectType::ARRAY;
    }

    bool isLiteral() const {
        return root && root->getObjType() == jsonparse::JsonObjectType::LITERAL;
    }

    bool isString() const {
        return isLiteral() &&
               asLiteralPtr()->getLiteralType() == jsonparse::LiteralType::STRING;
    }

    bool isNumber() const {
        return isLiteral() &&
               asLiteralPtr()->getLiteralType() == jsonparse::LiteralType::NUMBER;
    }

    bool isBool() const {
        return isLiteral() &&
               asLiteralPtr()->getLiteralType() == jsonparse::LiteralType::BOOL;
    }

    bool isNull() const {
        return isLiteral() &&
               asLiteralPtr()->getLiteralType() == jsonparse::LiteralType::NULL_VAL;
    }

    Json operator[](const std::string& key) const {
        return Json(asObjectPtr()->getValue(key));
    }

    Json operator[](size_t index) const {
        auto arr = asArrayPtr();
        require(index < arr->getArrayVals().size(), "Index out of bounds");
        return Json(arr->getArrayVals()[index]);
    }

    bool hasKey(const std::string& key) const {
        auto obj = asObjectPtr();
        for (const auto& kv : obj->getKeyPairs())
            if (kv.first == key)
                return true;
        return false;
    }

    size_t objectSize() const {
        return asObjectPtr()->getKeyPairs().size();
    }

    size_t arraySize() const {
        return asArrayPtr()->getArrayVals().size();
    }

    std::string asString() const {
        require(isString(), "Not a string");
        return static_cast<jsonparse::JsonString*>(asLiteralPtr())->getValue();
    }

    float asNumber() const {
        require(isNumber(), "Not a number");
        return static_cast<jsonparse::JsonNumber*>(asLiteralPtr())->getValue();
    }

    bool asBool() const {
        require(isBool(), "Not a boolean");
        return static_cast<jsonparse::JsonBool*>(asLiteralPtr())->getValue();
    }

    jsonparse::JPtr raw() const {
        return root;
    }
};

}
