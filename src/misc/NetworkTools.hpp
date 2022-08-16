#pragma once

#include <functional>
#include <string>

namespace KCore {
    class NetworkTools {
    public:
        static void performGETRequestAsync
                (const std::string &url, const std::function<void(const std::vector<uint8_t> &)> &callback);

        static std::vector<uint8_t> performGETRequestSync
        (const std::string &url);
    };
}