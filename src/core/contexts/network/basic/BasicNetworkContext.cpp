//
// Created by anshu on 5/24/2022.
//

#include "BasicNetworkContext.hpp"

#include "HTTPRequest.hpp"

namespace KCore {
    void BasicNetworkContext::initialize() {}

    void BasicNetworkContext::performLoopStep() {
        while (!mRequestQueue.empty()) {
            auto task = mRequestQueue.back();
            mRequestQueue.pop_back();

            std::thread{[task]() {
                try {
                    http::Request request{task->getUrl()};
                    const auto response = request.send("GET", "", {
                            {"Content-Type", "application/x-www-form-urlencoded"},
                            {"User-Agent",   "KarafutoMapCore/0.1"},
                    });
                    auto &buffer = task->getBuffer();
                    buffer.insert(buffer.end(), response.body.begin(), response.body.end());
                    task->Finalize();
                } catch (const std::exception &e) {
                    std::cerr << "Request failed, error: " << e.what() << '\n';
                }
            }}.detach();
        }
    }

    void BasicNetworkContext::dispose() {}
}