#include "FallbackNetworkContext.hpp"

namespace KCore {
    void FallbackNetworkContext::performLoopStep() {
        std::this_thread::sleep_for(100ms);
        while (!mRequestQueue.empty()) {
            // get next task or nullptr
            auto task = mRequestQueue.back();
            mRequestQueue.pop_back();
            delete task;
        }
    }

    void FallbackNetworkContext::initialize() {
        std::cout << "Network Thread ID: " << std::this_thread::get_id() << std::endl;
    }

    void FallbackNetworkContext::dispose() {}
}