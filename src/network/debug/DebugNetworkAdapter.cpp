#include "DebugNetworkAdapter.hpp"

#include "misc/DebugNewtorkResources.inl"

namespace KCore {
    std::vector<uint8_t> DebugNetworkAdapter::SyncRequest
            (const std::string &url, const std::string &method) {
        return KCore::Network::Debug::Resource::image;
    }
}
