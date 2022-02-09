#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <fstream>
#include <vector>

#include "BaseSourcePiece.hpp"

namespace KCore {
    class FileSource : public BaseSourcePiece {
    protected:
        std::string mPath;

        std::string mFolderPath;

        std::string mFileName;
        std::string mFileNameBase;
        std::string mFileNameExtension;

        bool mFileCorrupted{false};
    public:
        std::vector<uint8_t> mData{};

    public:
        FileSource(std::string path) : mPath(std::move(path)) {
            auto lastSlashPos = mPath.find_last_of("/\\") + 1;
            mFolderPath = mPath.substr(0, lastSlashPos);
            mFileName = mPath.substr(lastSlashPos);

            auto dotPos = mFileName.find('.');
            mFileNameBase = mFileName.substr(0, dotPos);
            mFileNameExtension = mFileName.substr(dotPos + 1);

            loadFile();
        }

        const std::vector<uint8_t> &getRawData() {
            return mData;
        }

        uint8_t *getTileData(uint8_t zoom, uint16_t x, uint16_t y) override {
            return nullptr;
        }

    private:
        void loadFile() {
            std::streampos fileSize;
            std::ifstream file(mPath, std::ios::binary);

            file.seekg(0, std::ios::end);
            fileSize = file.tellg();
            file.seekg(0, std::ios::beg);

            mData.resize(fileSize);
            file.read((char *) &mData[0], fileSize);
        }
    };
}