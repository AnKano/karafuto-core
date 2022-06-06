#include "FallbackNetworkContext.hpp"

#include "misc/FallbackResource.inl"

namespace KCore {
    void FallbackNetworkContext::performLoopStep() {
        while (!mRequestQueue.empty()) {
            auto task = mRequestQueue.back();
            mRequestQueue.pop_back();

            std::thread{[task]() {
                try {
                    auto &buffer = task->getBuffer();
                    buffer.insert(
                            buffer.end(),
                            Network::Fallback::BuiltIn::image.begin(),
                            Network::Fallback::BuiltIn::image.end()
                    );
                    task->emitFinal();
                } catch (const std::exception &e) {
                    std::cerr << "Request failed, error: " << e.what() << '\n';
                }
            }}.detach();
        }
    }

    void FallbackNetworkContext::initialize() {
        std::cout << "Network Thread ID: " << std::this_thread::get_id() << std::endl;
    }

    void FallbackNetworkContext::dispose() {}
}