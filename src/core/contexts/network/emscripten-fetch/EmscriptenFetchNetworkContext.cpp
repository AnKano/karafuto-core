#include "EmscriptenFetchNetworkContext.hpp"

#include <emscripten/fetch.h>

void downloadSucceeded(emscripten_fetch_t *fetch) {
    auto *task = (KCore::NetworkRequest *) fetch->userData;
    auto *transmissionBuffer = (std::vector<uint8_t> *) &task->getBuffer();
    transmissionBuffer->insert(
            transmissionBuffer->end(),
            fetch->data,
            fetch->data + fetch->numBytes - 1
    );

    std::cout << transmissionBuffer->size() << std::endl;
    emscripten_fetch_close(fetch);
}

namespace KCore {
    void EmscriptenFetchNetworkContext::onEachStep() {
        std::this_thread::sleep_for(1000ms);
    }

    void EmscriptenFetchNetworkContext::synchronousStep() {
        while (!mRequestQueue.empty()) {
            // get next task or nullptr
            auto task = mRequestQueue.back();
            mRequestQueue.pop_back();

            performTask(task);
        }
    }

    void EmscriptenFetchNetworkContext::performTask(NetworkRequest *task) {
        emscripten_fetch_attr_t attr;
        emscripten_fetch_attr_init(&attr);
        strcpy(attr.requestMethod, "GET");
        attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
        attr.onsuccess = downloadSucceeded;
        attr.userData = task;

        emscripten_fetch(&attr, task->getUrl().c_str());
    }

    void EmscriptenFetchNetworkContext::init() {
        std::cout << "Network EFN Thread ID: " << std::this_thread::get_id() << std::endl;
    }

    void EmscriptenFetchNetworkContext::dispose() {
        std::cout << "Network EFN destroyed!" << std::endl;
    }
}