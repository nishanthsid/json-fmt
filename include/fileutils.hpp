#pragma once

#include<string>
#include<fstream>
#include<iostream>
#include<vector>

namespace fileutils{

    class OutputFileWriter{
        private:
            static const std::size_t BUFFER_SIZE = 1 << 20;
            std::vector<char> outPutBuffer = std::vector<char>(BUFFER_SIZE);
            size_t bytesPushed = 0;
            std::ofstream file;

            void writeToFile(){
                if(bytesPushed > 0){
                    file.write(outPutBuffer.data(),bytesPushed);
                    bytesPushed = 0;
                }
            }

        public:
            OutputFileWriter(std::string fileName){
                file.open(fileName,std::ios::binary);
                if(!file.is_open()){
                    throw std::runtime_error("Failed to open file: " + fileName);
                }
            }
            void pushChar(char nextChar){
                outPutBuffer[bytesPushed++] = nextChar;
                if(bytesPushed == BUFFER_SIZE){
                    writeToFile();
                    bytesPushed = 0;
                } 
            }

            void flush(){
                writeToFile();
            }

            ~OutputFileWriter(){
                flush();
                if(file.is_open()){
                    file.close();
                }
            }
    };

    class InputFileReader{
        private:
            static const std::size_t BUFFER_SIZE = 1 << 20;
            std::vector<char> textChunk = std::vector<char>(BUFFER_SIZE);;
            std::ifstream file;
            size_t bytesReadFromBuffer = 0;
            size_t bytesReadFromFile = 0;
            int eof = 0;

            void updateBytesLeft(){
                std::streamsize bytesRead = file.gcount();
                if(bytesRead != 0){
                    bytesReadFromBuffer = bytesRead;
                }
                else{
                    eof = 1;
                }
            }

            void readNextChunk(){
                file.read(textChunk.data(),BUFFER_SIZE);
                updateBytesLeft();
            }

        public:
            InputFileReader(std::string fileName){
                file.open(fileName, std::ios::binary);
                if(!file.is_open()){
                    throw std::runtime_error("Failed to open file: " + fileName);
                }
                readNextChunk();
            }

            bool isEof(){
                return eof == 1;
            }

            char readNextChar(){
                if(bytesReadFromFile >= bytesReadFromBuffer){
                    readNextChunk();
                    bytesReadFromFile = 0;   
                }
                return textChunk[bytesReadFromFile++];
            }
            ~InputFileReader() {
                if (file.is_open()) {
                    file.close();
                }
            }
    };

};

