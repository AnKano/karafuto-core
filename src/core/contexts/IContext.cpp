#include "IContext.hpp"

namespace KCore {
    IContext::~IContext() {
        setShouldClose(true);
        // await to thread stop working
        while (getWorkingStatus()) std::this_thread::sleep_for(10ms);
    }

    void IContext::setCheckInterval(const uint64_t &value) {
        mCheckInterval = std::chrono::milliseconds(value);
    }

    void IContext::setCheckInterval(const std::chrono::milliseconds &value) {
        mCheckInterval = value;
    }

    void IContext::setShouldClose(const bool &value) {
        mShouldClose = value;
    }

    bool IContext::getWorkingStatus() const {
        return mReadyToBeDead;
    }
}