#include "RemoteSource.hpp"

namespace KCore {
    RemoteSource::RemoteSource(std::string rawUrl) : mRawUrl(std::move(rawUrl)) {
        restoreAffixes();
    }

    RemoteSource::RemoteSource(std::string prefix, std::string affix)
            : mURLPrefix(std::move(prefix)), mURLSuffix(std::move(affix)) {
        restoreRawUrl();
    }

    uint8_t *RemoteSource::getDataForTile(uint8_t zoom, uint16_t x, uint16_t y, uint16_t slicesX, uint16_t slicesY) {
        return nullptr;
    }

    std::string RemoteSource::bakeUrl(const TileDescription &desc) {
        return mURLPrefix + desc.tileURL() + mURLSuffix;
    }

    std::vector<std::shared_ptr<BaseSourcePart>> RemoteSource::getRelatedPieces(uint8_t zoom, uint16_t x, uint16_t y) {
        return {};
    }

    void RemoteSource::createPartFile(const std::string &path) {}

    void RemoteSource::restoreRawUrl() {
        mRawUrl = mURLPrefix + "{z}/{x}/{y}" + mURLSuffix;
    }

    void RemoteSource::restoreAffixes() {
        auto payloadLength = std::string("{z}/{x}/{y}").length();
        auto position = mRawUrl.find("{z}/{x}/{y}");

        mURLPrefix = mRawUrl.substr(0, position);
        mURLSuffix = mRawUrl.substr(position + payloadLength);
    }

    extern "C" {
    DllExport KCore::RemoteSource *CreateRemoteSource(const char *url) {
        return new RemoteSource(url);
    }
    }
}