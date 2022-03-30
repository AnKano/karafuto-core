#include "FallbackRenderContext.hpp"

#include <iostream>

namespace KCore::Fallback {
    void FallbackRenderContext::initialize() {
        std::cout << "Fallback Render Thread ID: " << std::this_thread::get_id() << std::endl;
    }

    void FallbackRenderContext::performLoopStep() {
        std::cout << "Fallback ping" << std::endl;
        std::this_thread::sleep_for(1s);
    }

    void FallbackRenderContext::dispose() {
        std::cout << "Fallback disposed" << std::endl;
    }
}