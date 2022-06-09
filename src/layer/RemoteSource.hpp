#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <fstream>
#include <vector>

#include "../geography/TileDescription.hpp"
#include "../misc/Bindings.hpp"

namespace KCore {
    class RemoteSource {
    private:
        std::string mRawUrl;
        std::string mURLPrefix, mURLSuffix;

    public:
        /** example: http://10.0.0.1:8080/{z}/{x}/{y}.png
         * mURLPrefix - "http://10.0.0.1:8080/"
         * mURLSuffix - ".png"
         **/
        explicit RemoteSource(std::string rawUrl);

        /** example prefix: "http://10.0.0.1:8080/"
         * example affix: ".png"
         * mRawUrl - "http://10.0.0.1:8080/{z}/{x}/{y}.png"
         **/
        RemoteSource(std::string prefix, std::string affix);

        std::string bakeUrl(const TileDescription &desc);

    private:
        void restoreRawUrl();

        void restoreAffixes();
    };
}