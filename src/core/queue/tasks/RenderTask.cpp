//
// Created by anshu on 1/31/2022.
//

#include "RenderTask.hpp"

//#include <GL/glew.h>

#include "../../MapCore.hpp"

namespace KCore {
    void RenderTask::onTaskComplete() {
        BaseTask::onTaskComplete();

//        auto textureBytesCount = 256 * 256 * 3;
//        auto buffer = std::make_shared<std::vector<uint8_t>>();
//        buffer->resize(textureBytesCount);
//        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, buffer->data());
//        if (!mCore_ptr->mDataStash.getByKey(mQuadcode + ".meta.image")) {
//            mCore_ptr->mDataStash.setOrReplace(mQuadcode + ".meta.image",
//                                               std::static_pointer_cast<void>(buffer));
//
//            MapEvent event{};
//            event.Type = ContentLoadedRender;
//            strcpy_s(event.Quadcode, mQuadcode.c_str());
//            event.OptionalPayload = nullptr;
//
//            mCore_ptr->pushEventToContentQueue(event);
//
//            std::cout << mQuadcode << " loading from renderer complete" << std::endl;
//        }
    }

    void RenderTask::performTask() {

    }
}