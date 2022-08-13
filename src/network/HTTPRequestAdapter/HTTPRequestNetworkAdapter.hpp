#pragma once

#include <exception>

#include "../INetworkAdapter.hpp"
#include "HTTPRequest.hpp"

namespace KCore {
    class HTTPRequestNetworkAdapter : public INetworkAdapter {
    public:
        std::vector<uint8_t> SyncRequest(const std::string &url, const std::string &method) override;
    };
}