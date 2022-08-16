//
// Created by anshu on 8/13/2022.
//

#include "HTTPRequestNetworkAdapter.hpp"

#include <iostream>

namespace KCore {
    std::vector<uint8_t> HTTPRequestNetworkAdapter::SyncRequest
            (const std::string &url, const std::string &method) {
        auto result = getFromCache(url);
        if (result.has_value())
            return {result->begin(), result->end()};

        http::Request request{url};
        const auto response = request.send("GET", "", {
                {"Content-Type", "application/x-www-form-urlencoded"},
                {"User-Agent",   mUserAgent},
        });

        if (response.status.code == http::Status::Ok) {
            result = {response.body.begin(), response.body.end()};
            insertToCache(url, result.value());
            return result.value();
        }

        throw std::runtime_error(response.status.reason);
    }
}