#include "RemoteSource.hpp"

namespace KCore {
    RemoteSource::RemoteSource(std::string rawUrl) : mRawUrl(std::move(rawUrl)) {
        restoreAffixes();
    }

    RemoteSource::RemoteSource(std::string prefix, std::string affix)
            : mURLPrefix(std::move(prefix)), mURLSuffix(std::move(affix)) {
        restoreRawUrl();
    }

    std::string RemoteSource::bakeUrl(const TileDescription &desc) {
        return mURLPrefix + desc.tileURL() + mURLSuffix;
    }

    void RemoteSource::restoreRawUrl() {
        mRawUrl = mURLPrefix + "{z}/{x}/{y}" + mURLSuffix;
    }

    void RemoteSource::restoreAffixes() {
        auto payloadLength = std::string("{z}/{x}/{y}").length();
        auto position = mRawUrl.find("{z}/{x}/{y}");

        mURLPrefix = mRawUrl.substr(0, position);
        mURLSuffix = mRawUrl.substr(position + payloadLength);
    }
}