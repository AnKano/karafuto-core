#pragma once

#include "../../ISource.hpp"

#include <sstream>
#include <regex>
#include <exception>

namespace KCore {

    class SRTMSource : public ISource {
    public:
        double mXOrigin{}, mYOrigin{};
        double mXOpposite{}, mYOpposite{};
        double mPixelWidth{}, mPixelHeight{};

    public:
        SRTMSource(const char *path, SourceType type);
    };
}