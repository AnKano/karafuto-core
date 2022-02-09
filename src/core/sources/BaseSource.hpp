#pragma once

#include <cstdint>
#include <vector>
#include <memory>

#include "BaseSourcePiece.hpp"

namespace KCore {
    class BaseSource {
    protected:
        std::vector<std::shared_ptr<BaseSourcePiece>> mPieces;

    public:
        BaseSource() = default;

        virtual uint8_t *getDataForTile(uint8_t zoom, uint16_t x, uint16_t y,
                                        uint16_t slicesX = 0, uint16_t slicesY = 0) = 0;

        void addSourcePiece(BaseSourcePiece *piece) {
            auto sharedPiece = std::shared_ptr<BaseSourcePiece>(piece);
            mPieces.push_back(sharedPiece);
        }

    protected:
        virtual std::vector<std::shared_ptr<BaseSourcePiece>>
            getRelatedPieces(uint8_t zoom, uint16_t x, uint16_t y) = 0;
    };
}