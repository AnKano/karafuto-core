#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <fstream>
#include <vector>

namespace KCore {
    class BaseFileSource {
    protected:
        std::string mFilename;
        std::vector<uint8_t> mData{};

    public:
        BaseFileSource(std::string filename) : mFilename(std::move(filename)) {
            loadFile();
        }

        virtual std::vector<uint8_t> getRawData() = 0;

        virtual uint8_t *getTileData(uint8_t zoom, uint16_t x, uint16_t y) = 0;

    private:
        void loadFile() {
            std::streampos fileSize;
            std::ifstream file(mFilename, std::ios::binary);

            file.seekg(0, std::ios::end);
            fileSize = file.tellg();
            file.seekg(0, std::ios::beg);

            mData.resize(fileSize);
            file.read((char *) &mData[0], fileSize);
        }
    };
}