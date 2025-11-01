#pragma once
#include "fileutils.hpp"

namespace jsonfmt{
    class JsonFormat{
        private:
            enum class context{
                NORMAL,
                STRING
            };
            context cntx = context::NORMAL;
            fileutils::InputFileReader inputJson;
            fileutils::OutputFileWriter outPutJson;

        public:
            JsonFormat(std::string inputFile, std::string outPutFile) \
            : inputJson(inputFile),
              outPutJson(outPutFile){
            }

            void formatJson(int indent = 4){
                long level = 0;
                char nextChar;
                int isNewLine = 0;
                int isEscapedChar = 0;
                while(true){
                    nextChar = inputJson.readNextChar();
                    if(inputJson.isEof()){
                        break;
                    }
                    switch(cntx){
                        case context::NORMAL:{
                            switch(nextChar){
                                case '{':
                                    if(isNewLine){
                                        isNewLine = 0;
                                        for(int i = 0; i < level; i++){
                                            for(int j = 0;j < indent; j++){
                                                outPutJson.pushChar(' ');
                                            }
                                        }
                                    }
                                    outPutJson.pushChar('{');
                                    outPutJson.pushChar('\n');
                                    isNewLine = 1;
                                    level += 1;
                                    break;
                                case '[':
                                    if(isNewLine){
                                        isNewLine = 0;
                                        for(int i = 0; i < level; i++){
                                            for(int j = 0;j < indent; j++){
                                            outPutJson.pushChar(' ');
                                            }
                                        }
                                    }
                                    outPutJson.pushChar('[');
                                    outPutJson.pushChar('\n');
                                    isNewLine = 1;
                                    level += 1;
                                    break;
                                case ':':
                                    outPutJson.pushChar(':');
                                    outPutJson.pushChar(' ');
                                    break;
                                case ' ':
                                case '\n':
                                case '\t':
                                    break;
                                case '}':
                                    outPutJson.pushChar('\n');
                                    level -= 1;
                                    for(int i = 0;i < level;i++){
                                        for(int j = 0;j < indent; j++){
                                                outPutJson.pushChar(' ');
                                                }
                                    }
                                    outPutJson.pushChar('}');
                                    break;
                                case ']':
                                    outPutJson.pushChar('\n');
                                    level -= 1;
                                    for(int i = 0;i < level;i++){
                                        for(int j = 0;j < indent; j++){
                                            outPutJson.pushChar(' ');
                                            }
                                    }
                                    outPutJson.pushChar(']');
                                    break;
                                case ',':
                                    outPutJson.pushChar(',');
                                    outPutJson.pushChar('\n');
                                    isNewLine = 1;
                                    break;
                                case '\"':
                                    cntx = context::STRING;
                                default:
                                    if(isNewLine){
                                        isNewLine = 0;
                                        for(int i = 0; i < level; i++){
                                            for(int j = 0;j < indent; j++){
                                                outPutJson.pushChar(' ');
                                            }
                                        }
                                    }
                                    outPutJson.pushChar(nextChar);
                                    break;
                                    
                            }
                            break;
                        }
                        case context::STRING:{
                            switch(nextChar){
                                case '\\':
                                    isEscapedChar = 1;
                                    outPutJson.pushChar(nextChar);
                                    break;
                                case '\"':
                                    outPutJson.pushChar('\"');
                                    if(isEscapedChar == 0){
                                        cntx = context::NORMAL;
                                    }
                                    else{
                                        isEscapedChar = 0;
                                    }
                                    break;
                                default:
                                    if(isEscapedChar){
                                        isEscapedChar = 0;
                                    }
                                    outPutJson.pushChar(nextChar);
                                    break;
                                
                            }
                            break;

                        }
                    }
                }
            }

            void minifyJson(){
                int isEscapedChar = 0;
                while(true){
                    char nextChar = inputJson.readNextChar();
                    if(inputJson.isEof()){
                        break;
                    }
                    switch(cntx){
                        case context::NORMAL: {
                            switch(nextChar){
                                case '\n':
                                case '\t':
                                case ' ' :
                                    break;
                                case '\"':
                                    cntx = context::STRING;
                                default:
                                    outPutJson.pushChar(nextChar);
                                    break;
                            }
                            break;
                        }
                        case context::STRING: {
                            switch(nextChar){
                                case '\\':
                                    isEscapedChar = 1;
                                    outPutJson.pushChar(nextChar);
                                    break;
                                case '\"':
                                    outPutJson.pushChar('\"');
                                    if(isEscapedChar == 0){
                                        cntx = context::NORMAL;
                                    }
                                    else{
                                        isEscapedChar = 0;
                                    }
                                    break;
                                default:
                                    if(isEscapedChar){
                                        isEscapedChar = 0;
                                    }
                                    outPutJson.pushChar(nextChar);
                                    break;
                                
                            }
                            break;
                        }

                    }
                }
            }
    };
};
