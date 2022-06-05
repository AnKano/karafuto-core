#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include <iostream>
#include <filesystem>

#include "BaseSourcePart.hpp"

namespace fs = std::filesystem;

namespace KCore {
    class BaseSource {
    protected:
        std::vector<std::shared_ptr<BaseSourcePart>> mPieces;

    public:
        BaseSource() = default;

        virtual uint8_t *getDataForTile(uint8_t zoom, uint16_t x, uint16_t y,
                                        uint16_t slicesX = 0, uint16_t slicesY = 0) = 0;

        void addSourcePart(BaseSourcePart *part) {
            auto sharedPiece = std::shared_ptr<BaseSourcePart>(part);
            mPieces.push_back(sharedPiece);
        }

        void addSourcePart(const std::string& filePath) {
            std::cout << filePath << std::endl;
            createPartFile(filePath);
        }

        void addSourcePart(const std::string &directoryPath, const std::string &postfix) {
            for (const auto &entry: fs::directory_iterator(directoryPath)) {
                std::cout << entry.path() << std::endl;

                auto filePath = entry.path();
                auto ext = entry.path().extension().string();

                if (ext != postfix) continue;

                createPartFile(filePath.string());
            }
        }

    protected:
        virtual std::vector<std::shared_ptr<BaseSourcePart>> getRelatedPieces(
                uint8_t zoom, uint16_t x, uint16_t y
        ) = 0;

        virtual void createPartFile(const std::string &path) = 0;
    };
}