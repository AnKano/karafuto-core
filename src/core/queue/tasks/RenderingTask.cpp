//
// Created by anshu on 1/31/2022.
//

#include "RenderingTask.hpp"
#include "glad/glad.h"

#include "../../MapCore.hpp"

namespace KCore {
    void RenderingTask::onTaskComplete() {
        BaseTask::onTaskComplete();

        auto textureBytesCount = 2048 * 2048 * 3;
        auto buffer = std::make_shared<std::vector<uint8_t>>();
        buffer->resize(textureBytesCount);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, buffer->data());
        mStash_ptr->setOrReplace(mQuadcode + ".meta.image", std::static_pointer_cast<void>(buffer));

        MapEvent event{};
        {
            event.Type = ContentLoaded;
            strcpy_s(event.Quadcode, mQuadcode.c_str());
            event.OptionalPayload = nullptr;
        }

        mCore_ptr->pushEventToContentEvent(event);

        std::cout << mQuadcode << " loading from renderer complete" << std::endl;
    }

    void RenderingTask::performTask() {

    }
}