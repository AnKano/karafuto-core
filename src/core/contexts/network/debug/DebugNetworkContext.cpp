#include "DebugNetworkContext.hpp"

#include "misc/DebugResource.inl"

namespace KCore {
    void DebugNetworkContext::performLoopStep() {
        while (!mRequestQueue.empty()) {
            auto task = mRequestQueue.back();
            mRequestQueue.pop_back();

            std::thread{[task]() {
                try {
                    task->getBuffer() = Network::Debug::BuiltIn::image;
                    task->emitFinal();
                } catch (const std::exception &e) {
                    std::cerr << "Request failed, error: " << e.what() << '\n';
                }
            }}.detach();
        }
    }

    void DebugNetworkContext::initialize() {
        std::cout << "Network Thread ID: " << std::this_thread::get_id() << std::endl;
    }

    void DebugNetworkContext::dispose() {}
}