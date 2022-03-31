#include "INetworkContext.hpp"

namespace KCore {
    INetworkContext::INetworkContext() {
        mRenderThread = std::make_unique<std::thread>([this]() {
            initialize();
            runLoop();
        });
        mRenderThread->detach();
    }

    INetworkContext::~INetworkContext() {
        disposeContext();
    }

    void INetworkContext::setWaitInterval(const uint64_t &value) {
        mWaitInterval = std::chrono::milliseconds(value);
    }

    void INetworkContext::setWaitInterval(const std::chrono::milliseconds &value) {
        mWaitInterval = value;
    }

    void INetworkContext::setShouldClose(const bool &value) {
        mShouldClose = value;
    }

    bool INetworkContext::getWorkingStatus() const {
        return mReadyToBeDead;
    }

    void INetworkContext::runLoop() {
        while (!mShouldClose)
            performLoopStep();

        disposeContext();

        mReadyToBeDead = true;
    }

    void INetworkContext::disposeContext() {
        setShouldClose(true);
        // await to thread stop working
        while (getWorkingStatus()) std::this_thread::sleep_for(10ms);

        dispose();
    }

    void INetworkContext::pushRequestToQueue(NetworkRequest *task) {
        mRequestQueue.push_back(task);
    }

    void INetworkContext::synchronousStep() {
        // do nothing by default
    }
}