#pragma once

#include <string>

namespace KCore {
    class BaseTile {
    private:
        std::string mQuadcode;

    public:
        BaseTile(std::string quadcode) : mQuadcode(std::move(quadcode)) {}
    };
}