#include "INetworkContext.hpp"

namespace KCore {
    INetworkContext::INetworkContext() {
        mThread = std::make_unique<std::thread>([this]() {
            initialize();
            runLoop();
        });
        mThread->detach();
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
}