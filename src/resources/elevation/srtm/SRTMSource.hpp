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
        SRTMSource(const char *path, SourceType type) : ISource(path, type) {
            restoreFromFilename();
        }

    private:
        void restoreFromFilename() {
            std::regex rgx("([NS])([0-9]+)([EW])([0-9]+)");
            std::smatch matches;

            if (std::regex_search(mFileNameBase, matches, rgx)) {
                std::cout << "Match found\n";

                auto mYSign = (std::stoi(matches[0]) == 'N') ? 1 : -1;
                auto mYVal = std::stoi(matches[1]);
                auto mY = mYSign * mYVal;

                auto mXSign = (std::stoi(matches[2]) == 'E') ? 1 : -1;
                auto mXVal = std::stoi(matches[3]);
                auto mX = mXSign * mXVal;

                mYOrigin = mY + 1.0;
                mYOpposite = mY;

                mXOrigin = mX;
                mXOpposite = mX + 1.0;

                mPixelWidth = 1.0 / 3601.0;
                mPixelHeight = mPixelWidth;
            } else
                throw std::runtime_error("Wrong SRTM-file filename: unable to parse sensitive meta!");
        };

    };

}