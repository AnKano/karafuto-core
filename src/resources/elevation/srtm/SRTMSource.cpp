
#include "SRTMSource.hpp"

#include <sstream>
#include <regex>

namespace KCore {
    KCore::SRTMSource::SRTMSource
            (const char *path, SourceType type) : ISource(path, type) {
        //!TODO: describe parsing
        std::regex rgx("([NS])([0-9]+)([EW])([0-9]+)");
        std::smatch matches;

        if (std::regex_search(mFileNameBase, matches, rgx)) {
            auto mYSign = (matches[1] == 'N') ? 1 : -1;
            auto mYVal = std::stoi(matches[2]);
            auto mY = mYSign * mYVal;

            auto mXSign = (matches[3] == 'E') ? 1 : -1;
            auto mXVal = std::stoi(matches[4]);
            auto mX = mXSign * mXVal;

            mYOrigin = mY + 1.0;
            mYOpposite = mY;

            mXOrigin = mX;
            mXOpposite = mX + 1.0;

            mPixelWidth = 1.0 / 3601.0;
            mPixelHeight = mPixelWidth;
        } else
            throw std::runtime_error("Wrong SRTM-file filename: unable to parse sensitive meta!");
    }
}