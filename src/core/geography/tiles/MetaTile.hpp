#pragma once

#include "../TileDescription.hpp"

namespace KCore {
    struct MetaTile {
        TileDescription description;
        std::vector<std::string> childQuadcodes;
        std::vector<std::string> parentQuadcodes;
    };
}