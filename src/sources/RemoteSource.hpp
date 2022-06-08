#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <fstream>
#include <vector>

#include "BaseSource.hpp"
#include "../geography/TileDescription.hpp"
#include "../misc/Bindings.hpp"

namespace KCore {
    class RemoteSource {
    private:

        std::string mRawUrl;
        std::string mURLPrefix, mURLSuffix;

    public:
        /** example: https://10.0.0.1:8080/{z}/{x}/{y}.png
         * mURLPrefix - "https://10.0.0.1:8080/"
         * mURLSuffix - ".png"
         **/
        RemoteSource(std::string rawUrl);

        /** example prefix: "https://10.0.0.1:8080/"
         * example affix: ".png"
         * mRawUrl - "https://10.0.0.1:8080/{z}/{x}/{y}.png"
         **/
        RemoteSource(std::string prefix, std::string affix);

        std::string bakeUrl(const TileDescription &desc);

    private:
        void restoreRawUrl();

        void restoreAffixes();
    };
}