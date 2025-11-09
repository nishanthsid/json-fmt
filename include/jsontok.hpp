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

    class Token{
        private:
            TokenType tokenType;
            std::string rawTokenValue;
        public:
            Token(std::string tokenValue, TokenType type){
                rawTokenValue = tokenValue;
                tokenType = type;
            }
            
            TokenType getTokenType() const{
                return tokenType;
            }

            std::string getRawTokenValue() const{
                return rawTokenValue;
            }
    };

    class JsonTokenizer{
        private:
            enum class TokenizerContext{
                NORMAL,
                NUMBER,
                STRING
            };
            fileutils::InputFileReader reader;
            std::vector<Token> tokenStream;

        public:
            JsonTokenizer(std::string fileName) : reader(fileName){}

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

                    std::cout<<nextChar<<" : "<<buffer<<std::endl;

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
                                    try{
                                        double num = std::stod(buffer.data());
                                    }
                                    catch(std::exception& e){
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
}
