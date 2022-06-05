#include "VulkanRenderContext.hpp"

#include "../../../worlds/TerrainedWorld.hpp"

namespace KCore::Vulkan {
    VulkanRenderContext::VulkanRenderContext(TerrainedWorld *world) : IRenderContext(world) {}

    void VulkanRenderContext::initialize() {
        core.run();
    }

    void VulkanRenderContext::prepareTransform(const std::string &rootQuadcode, const std::string &childQuadcode,
                                               glm::mat4 &scaleMatrix, glm::mat4 &translationMatrix) {
        auto formulae = childQuadcode.substr(rootQuadcode.length());

        auto difference = (float) formulae.size();
        float scale = 1.0f / powf(2.0f, difference);

        float step = 0.5f;
        glm::vec3 position{0.0f};
        for (const auto &in: formulae) {
            if (in == '0') {
                position.x += step;
                position.y -= step;
            } else if (in == '1') {
                position.x -= step;
                position.y -= step;
            } else if (in == '2') {
                position.x += step;
                position.y += step;
            } else if (in == '3') {
                position.x -= step;
                position.y += step;
            }
            step /= 2.0f;
        }

        scaleMatrix = glm::scale(glm::vec3{scale});
        translationMatrix = glm::translate(glm::vec3{position.x, position.y, 0.0f});
    }

    void VulkanRenderContext::performLoopStep() {
        if (mWorldAdapter->getAsyncEventsLength() != 0) {
            std::this_thread::sleep_for(350ms);
            return;
        };

        auto metas = getCurrentTileState();

        for (const auto &meta: metas) {
//            auto start_time = std::chrono::high_resolution_clock::now();

            auto items = meta->getChildQuadcodes();
            for (const auto item: items) {
                if (mInRAMNotConvertedTextures.count(item) == 0) continue;

                glm::mat4 scaleMatrix, translationMatrix;
                auto rootQuadcode = meta->getTileDescription().getQuadcode();
                prepareTransform(rootQuadcode, item, scaleMatrix, translationMatrix);

                auto &data = mInRAMNotConvertedTextures[item];
                auto image = STBImageUtils::decodeImageBuffer(
                        reinterpret_cast<const uint8_t *>(data.data()),
                        data.size(),
                        STBI_rgb
                );

                std::vector<uint8_t> decodedImage(256*256*4);
                int pixel = 0;
                for (auto j = 0u; j < 256; ++j) {
                    for (auto i = 0u; i < 256; ++i) {
                        decodedImage[pixel * 4]     = image[pixel * 3 ];
                        decodedImage[pixel * 4 + 1] = image[pixel * 3 + 1];
                        decodedImage[pixel * 4 + 2] = image[pixel * 3 + 2];
                        decodedImage[pixel * 4 + 3] = 0xFF;

                        pixel++;
                    }
                }

// declare
                core.declareTile(decodedImage, scaleMatrix, translationMatrix);
            }

// render
            core.drawFrame();

            auto results = core.readFrame();

            std::thread([results, this, meta]() {
                auto t0 = std::chrono::high_resolution_clock::now();

                std::string compressed_data = gzip::compress(
                        reinterpret_cast<const char *>(results.data()), results.size(), Z_BEST_SPEED
                );

                auto rawBuffer = new std::vector<uint8_t>{};
                rawBuffer->resize(compressed_data.size());
                std::copy(compressed_data.begin(), compressed_data.end(), rawBuffer->data());

                auto rootQuadcode = meta->getTileDescription().getQuadcode();
                mWorldAdapter->pushToAsyncEvents(MapEvent::MakeRenderLoadedEvent(rootQuadcode, rawBuffer));
                auto t1 = std::chrono::high_resolution_clock::now();
                auto duration = t1 - t0;

                auto d = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
            }).detach();

//            auto end_time = std::chrono::high_resolution_clock::now();
//            auto time = end_time - start_time;
//
//            std::cout << time/std::chrono::milliseconds(1) << "ms to run.\n";

            std::this_thread::sleep_for(10ms);
        }
    }

    void VulkanRenderContext::dispose() {

    }
}