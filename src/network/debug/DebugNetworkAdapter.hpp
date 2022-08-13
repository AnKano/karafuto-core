#pragma once

#include "../INetworkAdapter.hpp"

namespace KCore {
    class DebugNetworkAdapter : public INetworkAdapter {
    public:
        std::vector<uint8_t> SyncRequest(const std::string &url, const std::string &method) override;
    };
}
