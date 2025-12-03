#pragma once
#include "fileutils.hpp"
#include <string>
#include <vector>

namespace jsontok{
    enum class TokenType{
        OPEN_BRACE,
        CLOSE_BRACE,
        OPEN_BRACK,
        CLOSE_BRACK,
        STRING,
        NUMBER,
        NULL_VAL,
        BOOL,
        COMMA,
        COLON,
        END_OF_FILE
    };

    class NumberParser{
        private:
            enum class NumberParserContext{
                START,
                SIGN,
                ESIGN,
                A,
                B,
                C,
                D,
                E,
                F
                
            };
        public:
            static bool isDigit(char ch){
                return ch >= '0' && ch <= '9';
            }
            static bool isRealNum(std::string& num){
                int index = 0;
                char ch;
                NumberParserContext cntx = NumberParserContext::START;
                int len = num.length();
                while(index <= len){
                    if(index == len){
                        ch = '$';
                    }
                    else{
                        ch = num[index];
                    }

                    switch(cntx){
                        case NumberParserContext::START:{
                            if(ch == '+' || ch == '-'){
                                cntx = NumberParserContext::SIGN;
                                index += 1;
                            }
                            else if(ch == '.'){
                                index += 1;
                                cntx = NumberParserContext::A;
                            }
                            else if(isDigit(ch)){
                                index += 1;
                                cntx = NumberParserContext::C;
                            }
                            else{
                                return false;
                            }
                            break;
                        }
                        case NumberParserContext::SIGN:{
                            if(isDigit(ch)){
                                index += 1;
                                cntx = NumberParserContext::C;
                            }
                            else if(ch == '.'){
                                index += 1;
                                cntx = NumberParserContext::A;
                            }
                            else{
                                return false;
                            }
                            break;
                        }
                        case NumberParserContext::A:{
                            if(isDigit(ch)){
                                index += 1;
                                cntx = NumberParserContext::B;
                            }
                            else{
                                return false;
                            }
                            break;
                        }
                        case NumberParserContext::C:{
                            if(isDigit(ch)){
                                index += 1;
                            }
                            else if(ch == '.'){
                                index += 1;
                                cntx = NumberParserContext::D;
                            }
                            else if(ch == '$'){
                                return true;
                            }
                            else if(ch == 'e' || ch == 'E'){
                                index += 1;
                                cntx = NumberParserContext::E;
                            }
                            else{
                                return false;
                            }
                            break;
                        }

                        case NumberParserContext::B:{
                            if(ch == '$'){
                                return true;
                            }
                            else if(isDigit(ch)){
                                index += 1;
                            }
                            else if(ch == 'e' || ch == 'E'){
                                index += 1;
                                cntx = NumberParserContext::E;
                            }
                            else{
                                return false;
                            }
                            break;
                        }
                        case NumberParserContext::D:{
                            if(ch == '$'){
                                return true;
                            }
                            else if(isDigit(ch)){
                                index += 1;
                            }
                            else if(ch == 'E' || ch == 'e'){
                                index += 1;
                                cntx = NumberParserContext::E;
                            }
                            else{
                                return false;
                            }
                            break;
                        }
                        case NumberParserContext::E:{
                            if(ch == '+' || ch == '-'){
                                index += 1;
                                cntx = NumberParserContext::ESIGN;
                            }
                            else if(isDigit(ch)){
                                index += 1;
                                cntx = NumberParserContext::F;
                            }
                            else{
                                return false;
                            }
                            break;
                        }
                        case NumberParserContext::ESIGN:{
                            if(isDigit(ch)){
                                index += 1;
                                cntx = NumberParserContext::F;
                            }
                            else{
                                return false;
                            }
                            break;
                        }
                        case NumberParserContext::F:{
                            if(isDigit(ch)){
                                index += 1;
                            }
                            else if(ch == '$'){
                                return true;
                            }
                            else{
                                return false;
                            }
                            break;
                        }
                    }
                }
                return false;
            }
    };

    class Token{
        private:
            TokenType tokenType;
            std::string rawTokenValue;
        public:
            Token(std::string tokenValue, TokenType type){
                rawTokenValue = tokenValue;
                tokenType = type;
            }
            
            Token(){}

            TokenType getTokenType() const{
                return tokenType;
            }

            std::string getRawTokenValue() const{
                return rawTokenValue;
            }
    };

    

    class JsonStreamTokenizer{
        private:
            enum class TokenizerContext{
                NORMAL,
                NUMBER,
                STRING
            };
            fileutils::InputFileReader reader;
            std::vector<Token> tokenStream;

        public:
            JsonStreamTokenizer(std::string fileName) : reader(fileName){}

            std::vector<Token> getTokenStream(){
                return tokenStream;
            }

            void startTokenizing(){
                std::string buffer;
                TokenizerContext cntx = TokenizerContext::NORMAL;
                char nextChar;
                bool isEscape = false;

                while(true){
                    nextChar = reader.readNextChar();
                    if(reader.isEof()){
                        break;
                    }
                    switch(cntx){
                        case TokenizerContext::NORMAL:{
                            switch(nextChar){
                                case '{':{
                                    tokenStream.push_back(Token("{",TokenType::OPEN_BRACE));
                                    break;
                                }
                                case '[':{
                                    tokenStream.push_back(Token("[",TokenType::OPEN_BRACK));
                                    break;
                                }
                                case ']':{
                                    tokenStream.push_back(Token("]",TokenType::CLOSE_BRACK));
                                    break;
                                }
                                case '}':{
                                    tokenStream.push_back(Token("}",TokenType::CLOSE_BRACE));
                                    break;
                                }
                                case ':':{
                                    tokenStream.push_back(Token(":",TokenType::COLON));
                                    break;
                                }
                                case ',':{
                                    tokenStream.push_back(Token(",",TokenType::COMMA));
                                    break;
                                }
                                case ' ':
                                case '\n':
                                case '\r':
                                case '\t':{
                                    break;
                                }
                                case '\"':{
                                    cntx = TokenizerContext::STRING;
                                    buffer.clear();
                                    break;
                                }
                                case '-':
                                case '0':
                                case '1':
                                case '2':
                                case '3':
                                case '4':
                                case '5':
                                case '6':
                                case '7':
                                case '8':
                                case '9':{
                                    cntx = TokenizerContext::NUMBER;
                                    buffer.clear();
                                    buffer += nextChar;
                                    break;
                                }
                                default:{
                                    if(!isalpha(nextChar)){
                                        throw std::runtime_error(std::string("Invalid character in JSON context: ") + nextChar);
                                    }
                                    buffer += nextChar;
                                    if(buffer.size() == 4){
                                        if(buffer == "null"){
                                            tokenStream.push_back(Token("null",TokenType::NULL_VAL));
                                            buffer.clear();
                                        }
                                        else if(buffer == "true"){
                                            tokenStream.push_back(Token("true",TokenType::BOOL));
                                            buffer.clear();
                                        }
                                        else if(buffer != "fals"){
                                            throw std::runtime_error(std::string("Invalid literal found while parsing: ") + buffer);
                                        }

                                    }
                                    else if(buffer.size() == 5){
                                        if(buffer != "false"){
                                            throw std::runtime_error(std::string("Invalid literal found while parsing: ") + buffer);
                                        }
                                        else{
                                            tokenStream.push_back(Token("false",TokenType::BOOL));
                                            buffer.clear();
                                        }
                                    }
                                    
                                    break;
                                }
                            }
                            break;
                        }
                        case TokenizerContext::STRING:{
                            switch(nextChar){
                                case '\\':{
                                    isEscape = true;
                                    buffer += '\\';
                                    break;
                                }
                                case '\"':{
                                    if(!isEscape){
                                        tokenStream.push_back(Token(buffer.data(), TokenType::STRING));
                                        buffer.clear();
                                        cntx = TokenizerContext::NORMAL;
                                        break;
                                    }
                                    else{
                                        isEscape = false;
                                        buffer += '\"';
                                        break;
                                    }
                                }
                                default:{
                                    buffer += nextChar;
                                    break;
                                }
                            }
                            break;
                        }
                        case TokenizerContext::NUMBER:{
                            switch(nextChar){
                                case ' ':
                                case '\n':
                                case '\r':
                                case '\t':{
                                    break;
                                }
                                case ',':
                                case ']':
                                case '}':{
                                    if(!NumberParser::isRealNum(buffer)){
                                        throw std::runtime_error(std::string("Invalid numeric format encountered: ") + buffer);
                                    }
                                    tokenStream.push_back(Token(buffer.data(), TokenType::NUMBER));
                                    buffer.clear();
                                    cntx = TokenizerContext::NORMAL;
                                    switch(nextChar){
                                        case ']':{
                                            tokenStream.push_back(Token("]",TokenType::CLOSE_BRACK));
                                            break;
                                        }
                                        case '}':{
                                            tokenStream.push_back(Token("}",TokenType::CLOSE_BRACE));
                                            break;
                                        }
                                        case ',':{
                                            tokenStream.push_back(Token(",",TokenType::COMMA));
                                            break;
                                        }
                                    }
                                    break;
                                }
                                case '-':
                                case 'E':
                                case 'e':
                                case '.':
                                case '+':{
                                    buffer += nextChar;
                                    break;
                                }
                                default:{
                                    if(isdigit(nextChar)){
                                        buffer += nextChar;
                                        break;
                                    }
                                    else{
                                        throw std::runtime_error(std::string("Unexpected character inside number: ") + nextChar);
                                    }
                                    break;
                                }

                            }
                            break;
                        }
                    }
                }
                tokenStream.push_back(Token("$",TokenType::END_OF_FILE));
            }
        
    };

    class JsonOnDemandTokenizer{
        private:
            enum class TokenizerContext{
                NORMAL,
                NUMBER,
                STRING
            };
            fileutils::InputFileReader reader;
            Token peek;
            bool shouldConsume = true;

            Token processNextToken() {
                static std::string buffer;
                static TokenizerContext cntx = TokenizerContext::NORMAL;
                static bool isEscape = false;
                static char unProcessed;
                static bool unProcessedCharPresent = false;

                while (true) {
                    char nextChar = 0;
                    if(!unProcessedCharPresent){
                        nextChar = reader.readNextChar();
                    }
                    else{
                        nextChar = unProcessed;
                        unProcessedCharPresent = false;
                    }
                    if (reader.isEof()) {
                        return Token("$", TokenType::END_OF_FILE);
                    }
                    switch (cntx) {
                        case TokenizerContext::NORMAL: {
                            switch (nextChar) {
                                case '{': return Token("{", TokenType::OPEN_BRACE);
                                case '[': return Token("[", TokenType::OPEN_BRACK);
                                case ']': return Token("]", TokenType::CLOSE_BRACK);
                                case '}': return Token("}", TokenType::CLOSE_BRACE);
                                case ':': return Token(":", TokenType::COLON);
                                case ',': return Token(",", TokenType::COMMA);

                                case ' ':
                                case '\n':
                                case '\r':
                                case '\t': {
                                    break;
                                }

                                case '\"': {
                                    cntx = TokenizerContext::STRING;
                                    buffer.clear();
                                    break;
                                }

                                case '-': case '0': case '1': case '2': case '3':
                                case '4': case '5': case '6': case '7': case '8': case '9': {
                                    cntx = TokenizerContext::NUMBER;
                                    buffer.clear();
                                    buffer += nextChar;
                                    break;
                                }

                                default: {
                                    if (!isalpha(nextChar))
                                        throw std::runtime_error(std::string("Invalid character in JSON: ") + nextChar);

                                    buffer += nextChar;
                                    if (buffer == "null") {
                                        buffer.clear();
                                        return Token("null", TokenType::NULL_VAL);
                                    }
                                    if (buffer == "true") {
                                        buffer.clear();
                                        return Token("true", TokenType::BOOL);
                                    }
                                    if (buffer == "false") {
                                        buffer.clear();
                                        return Token("false", TokenType::BOOL);
                                    }
                                    if (buffer.size() > 5)
                                        throw std::runtime_error(std::string("Invalid literal while parsing: ") + buffer);
                                    break;
                                }
                            }
                            break;
                        }

                        case TokenizerContext::STRING: {
                            switch (nextChar) {
                                case '\\': {
                                    isEscape = !isEscape;
                                    buffer += '\\';
                                    break;
                                }
                                case '\"': {
                                    if (!isEscape) {
                                        cntx = TokenizerContext::NORMAL;
                                        std::string val = buffer;
                                        buffer.clear();
                                        return Token(val, TokenType::STRING);
                                    } else {
                                        buffer += '\"';
                                        isEscape = false;
                                    }
                                    break;
                                }
                                default: {
                                    buffer += nextChar;
                                    isEscape = false;
                                    break;
                                }
                            }
                            break;
                        }

                        case TokenizerContext::NUMBER: {
                            if (isdigit(nextChar) || nextChar == '.' || nextChar == 'e' || nextChar == 'E' ||
                                nextChar == '+' || nextChar == '-') {
                                buffer += nextChar;
                            } else {
                                if(!NumberParser::isRealNum(buffer)){
                                    throw std::runtime_error(std::string("Invalid number format: ") + buffer);
                                }
                                std::string val = buffer;
                                buffer.clear();
                                cntx = TokenizerContext::NORMAL;
                                
                                unProcessed = nextChar;
                                unProcessedCharPresent = true;

                                return Token(val, TokenType::NUMBER);
                            }
                            break;
                        }
                    }
                }

                return Token("$", TokenType::END_OF_FILE);
            }

        public:
            
            JsonOnDemandTokenizer(std::string fileName) : reader(fileName) {}

            Token peekNextToken(){
                if(shouldConsume){
                    peek = processNextToken();
                    shouldConsume = false;
                }
                return peek;
            }

            Token getNextToken(){
                if(!shouldConsume){
                    shouldConsume = true;
                    return peek;   
                }
                return processNextToken();
                
            }
            
        
    };
}
