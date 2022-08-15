#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <fstream>

#include "../misc/FileTools.hpp"
#include "../misc/NetworkTools.hpp"

namespace KCore {
    enum SourceType {
        SourceFile = 0,
        SourceUrl = 1
    };

    class ISource {
    protected:
        std::string mPathOrUrl;
        std::string mFileName;
        std::string mFileNameBase;

    public:
        std::vector<uint8_t> mData;

    public:
        ISource(const char *path, SourceType type) : mPathOrUrl(path) {
            parseFilename();

            switch (type) {
                case SourceFile:
                    loadDataFromFile();
                    break;
                case SourceUrl:
                    loadDataFromUrl();
                    break;
                default:
                    std::runtime_error("wrong file format");
            }
        }

    private:
        void parseFilename() {
            auto lastSlashPos = mPathOrUrl.find_last_of("/\\") + 1;
            mFileName = mPathOrUrl.substr(lastSlashPos);

            auto dotPos = mFileName.find('.');
            mFileNameBase = mFileName.substr(0, dotPos);
        }

        void loadDataFromFile() {
            mData = readFile(mPathOrUrl);
        }

        void loadDataFromUrl() {
            mData = NetworkTools::performGETRequestSync(mPathOrUrl);
        }
    };

}