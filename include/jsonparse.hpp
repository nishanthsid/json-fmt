#pragma once
#include "jsontok.hpp"
#include <memory>
#include <string>
#include <vector>

namespace jsonparse {

    enum class JsonObjectType {
        OBJECT,
        ARRAY,
        LITERAL
    };

    enum class LiteralType {
        STRING,
        NUMBER,
        BOOL,
        NULL_VAL
    };

    class JsonEntity {
    public:
        virtual ~JsonEntity() = default;
        virtual JsonObjectType getObjType() const = 0;
    };

    using JPtr = std::shared_ptr<JsonEntity>;

    class JsonObject : public JsonEntity {
    private:
        std::vector<std::pair<std::string, JPtr>> keyPairs;

    public:
        JsonObjectType getObjType() const override {
            return JsonObjectType::OBJECT;
        }

        void addKeyPair(const std::string& key, JPtr value) {
            keyPairs.emplace_back(key, value);
        }

        JPtr getValue(const std::string& key) {
            for (auto& kv : keyPairs) {
                if (kv.first == key)
                    return kv.second;
            }
            throw std::runtime_error("Key not found: " + key);
        }

        const std::vector<std::pair<std::string, JPtr>>& getKeyPairs() const {
            return keyPairs;
        }
    };

    class JsonArray : public JsonEntity {
    private:
        std::vector<JPtr> arrayVals;

    public:
        JsonObjectType getObjType() const override {
            return JsonObjectType::ARRAY;
        }

        const std::vector<JPtr>& getArrayVals() const {
            return arrayVals;
        }

        void addArrayVal(JPtr val) {
            arrayVals.push_back(val);
        }
    };

    class JsonLiteral : public JsonEntity {
    public:
        JsonObjectType getObjType() const override {
            return JsonObjectType::LITERAL;
        }
        virtual LiteralType getLiteralType() const = 0;
    };

    class JsonString : public JsonLiteral {
    private:
        std::string value;

    public:
        JsonString(const std::string& v) : value(v) {}
        LiteralType getLiteralType() const override { return LiteralType::STRING; }
        const std::string& getValue() const { return value; }
    };

    class JsonNumber : public JsonLiteral {
    private:
        float value;

    public:
        JsonNumber(float v) : value(v) {}
        LiteralType getLiteralType() const override { return LiteralType::NUMBER; }
        float getValue() const { return value; }
    };

    class JsonBool : public JsonLiteral {
    private:
        bool value;

    public:
        JsonBool(bool v) : value(v) {}
        LiteralType getLiteralType() const override { return LiteralType::BOOL; }
        bool getValue() const { return value; }
    };

    class JsonNull : public JsonLiteral {
    public:
        LiteralType getLiteralType() const override { return LiteralType::NULL_VAL; }
    };

    class JsonParser {
    private:
        static void throwError(const std::string& where,
                               const std::string& expected,
                               const jsontok::Token& found)
        {
            std::string msg =
                "\n[JSON Parse Error]\n"
                "Location: " + where + "\n"
                "Expected: " + expected + "\n"
                "Found Token: '" + found.getRawTokenValue() + "'\n"
                "TokenType: " + std::to_string(static_cast<int>(found.getTokenType())) + "\n";

            throw std::runtime_error(msg);
        }

    public:

        static JPtr startParsing(jsontok::JsonOnDemandTokenizer& tokenizer) {
            jsontok::Token peek = tokenizer.peekNextToken();
            jsontok::TokenType type = peek.getTokenType();

            if (type == jsontok::TokenType::OPEN_BRACE) {
                return parseObject(tokenizer);
            }
            else if (type == jsontok::TokenType::OPEN_BRACK) {
                return parseArray(tokenizer);
            }
            else {
                throwError("startParsing()", "{ or [", peek);
            }
        }

        static JPtr parseObject(jsontok::JsonOnDemandTokenizer& tokenizer) {
            JPtr jsonEntity = std::make_shared<JsonObject>();
            auto obj = std::static_pointer_cast<JsonObject>(jsonEntity);

            jsontok::Token currentTok = tokenizer.getNextToken(); // consumes '{'
            std::string currKey = "";

            while (currentTok.getTokenType() != jsontok::TokenType::CLOSE_BRACE) {
                currentTok = tokenizer.getNextToken();

                if(currentTok.getTokenType() == jsontok::TokenType::CLOSE_BRACE){
                    return obj;
                }

                if (currentTok.getTokenType() != jsontok::TokenType::STRING) {
                    throwError("parseObject(): reading key", "STRING (object key)", currentTok);
                }

                currKey = currentTok.getRawTokenValue();

                currentTok = tokenizer.getNextToken();
                if (currentTok.getTokenType() != jsontok::TokenType::COLON) {
                    throwError("parseObject(): after key", "COLON ':'", currentTok);
                }

                currentTok = tokenizer.peekNextToken();

                switch (currentTok.getTokenType()) {

                    case jsontok::TokenType::BOOL: {
                        bool val = (currentTok.getRawTokenValue() == "true");
                        obj->addKeyPair(currKey, std::make_shared<JsonBool>(val));
                        tokenizer.getNextToken();
                        break;
                    }

                    case jsontok::TokenType::STRING: {
                        obj->addKeyPair(currKey,
                            std::make_shared<JsonString>(currentTok.getRawTokenValue()));
                        tokenizer.getNextToken();
                        break;
                    }

                    case jsontok::TokenType::NUMBER: {
                        obj->addKeyPair(currKey,
                            std::make_shared<JsonNumber>(std::stod(currentTok.getRawTokenValue())));
                        tokenizer.getNextToken();
                        break;
                    }

                    case jsontok::TokenType::NULL_VAL: {
                        obj->addKeyPair(currKey, std::make_shared<JsonNull>());
                        tokenizer.getNextToken();
                        break;
                    }

                    case jsontok::TokenType::OPEN_BRACK: {
                        obj->addKeyPair(currKey, parseArray(tokenizer));
                        break;
                    }

                    case jsontok::TokenType::OPEN_BRACE: {
                        obj->addKeyPair(currKey, parseObject(tokenizer));
                        break;
                    }

                    default:
                        throwError("parseObject(): value", "literal | array | object", currentTok);
                }

                currentTok = tokenizer.getNextToken();

                if (currentTok.getTokenType() == jsontok::TokenType::CLOSE_BRACE) {
                    return obj;
                }
                else if (currentTok.getTokenType() != jsontok::TokenType::COMMA) {
                    throwError("parseObject(): expecting comma between pairs",
                               "',' or '}'",
                               currentTok);
                }
            }
            return obj;
        }

        static JPtr parseArray(jsontok::JsonOnDemandTokenizer& tokenizer) {
            JPtr jsonEntity = std::make_shared<JsonArray>();
            auto arr = std::static_pointer_cast<JsonArray>(jsonEntity);

            jsontok::Token currentTok = tokenizer.getNextToken(); // consumes '['

            while (currentTok.getTokenType() != jsontok::TokenType::CLOSE_BRACK) {

                currentTok = tokenizer.peekNextToken();

                switch (currentTok.getTokenType()) {

                    case jsontok::TokenType::BOOL: {
                        arr->addArrayVal(std::make_shared<JsonBool>(
                            currentTok.getRawTokenValue() == "true"
                        ));
                        tokenizer.getNextToken();
                        break;
                    }

                    case jsontok::TokenType::STRING: {
                        arr->addArrayVal(std::make_shared<JsonString>(
                            currentTok.getRawTokenValue()
                        ));
                        tokenizer.getNextToken();
                        break;
                    }

                    case jsontok::TokenType::NUMBER: {
                        arr->addArrayVal(std::make_shared<JsonNumber>(
                            std::stod(currentTok.getRawTokenValue())
                        ));
                        tokenizer.getNextToken();
                        break;
                    }

                    case jsontok::TokenType::NULL_VAL: {
                        arr->addArrayVal(std::make_shared<JsonNull>());
                        tokenizer.getNextToken();
                        break;
                    }

                    case jsontok::TokenType::OPEN_BRACE: {
                        arr->addArrayVal(parseObject(tokenizer));
                        break;
                    }

                    case jsontok::TokenType::OPEN_BRACK: {
                        arr->addArrayVal(parseArray(tokenizer));
                        break;
                    }

                    case jsontok::TokenType::CLOSE_BRACK: {
                        tokenizer.getNextToken();
                        return arr;
                    }

                    default:
                        throwError("parseArray(): value", "literal | array | object", currentTok);
                }

                currentTok = tokenizer.getNextToken();

                if (currentTok.getTokenType() == jsontok::TokenType::CLOSE_BRACK) {
                    return arr;
                }
                else if (currentTok.getTokenType() != jsontok::TokenType::COMMA) {
                    throwError("parseArray(): expecting comma between values",
                               "',' or ']'",
                               currentTok);
                }
            }

            return arr;
        }
    };

}
