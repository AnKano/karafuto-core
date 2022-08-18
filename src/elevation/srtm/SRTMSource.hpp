#pragma once

#include "../ISource.hpp"

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