#pragma once

#include <thread>
#include <iostream>

#include "HTTPRequest.hpp"

namespace KCore {
    static void performGETRequestAsync(const std::string &url,
                                       const std::function<void(const std::vector<uint8_t> &)>& callback) {
        std::thread{[url, callback]() {
            try {
                http::Request request{url};
                const auto response = request.send("GET", "", {
                        {"Content-Type", "application/x-www-form-urlencoded"},
                        {"User-Agent",   "KarafutoMapCore/0.1"},
                });

                callback({response.body.begin(), response.body.end()});
            } catch (const std::exception &e) {
                std::cerr << "Request failed, error: " << e.what() << '\n';
            }
        }}.detach();
    }

    static std::vector<uint8_t> performGETRequestSync(const std::string &url) {
        try {
            http::Request request{url};
            const auto response = request.send("GET", "", {
                    {"Content-Type", "application/x-www-form-urlencoded"},
                    {"User-Agent",   "KarafutoMapCore/0.1"},
            });

            return response.body;
        } catch (const std::exception &e) {
            std::cerr << "Request failed, error: " << e.what() << '\n';
            return {};
        }
    }
}