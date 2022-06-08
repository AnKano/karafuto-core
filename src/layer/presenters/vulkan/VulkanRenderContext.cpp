#include "VulkanRenderContext.hpp"

#include "../../Layer.hpp"

namespace KCore {
    namespace Vulkan {
        VulkanRenderContext::VulkanRenderContext(Layer *layer) : IRenderContext(layer) {}

        void VulkanRenderContext::initialize() {
            core.run();
        }

        void VulkanRenderContext::prepareTransform(const std::string &rootQuadcode,
                                                   const std::string &childQuadcode,
                                                   glm::mat4 &scaleMatrix,
                                                   glm::mat4 &translationMatrix) {
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
            auto metas = getCurrentTileState();

            for (const auto &meta: metas) {
                auto items = meta.getRelatedQuadcodes();
                for (const auto item: items) {
                    if (!mCachedTextures.contains(item)) continue;

                    glm::mat4 scaleMatrix, translationMatrix;
                    auto rootQuadcode = meta.getQuadcode();
                    prepareTransform(rootQuadcode, item, scaleMatrix, translationMatrix);

                    auto &data = mCachedTextures[item];

                    int width = -1, height = -1, channels = -1;
                    auto image = STBImageUtils::decodeImageBuffer(data.data(), data.size(), width,
                                                                  height, channels);

                    if (channels == 3) {
                        std::vector <uint8_t> decodedImage(width * height * 4);
                        int pixel = 0;
                        for (auto j = 0u; j < width; ++j) {
                            for (auto i = 0u; i < height; ++i) {
                                decodedImage[pixel * 4] = image[pixel * 3];
                                decodedImage[pixel * 4 + 1] = image[pixel * 3 + 1];
                                decodedImage[pixel * 4 + 2] = image[pixel * 3 + 2];
                                decodedImage[pixel * 4 + 3] = 0xFF;

                                pixel++;
                            }
                        }
                        core.declareTile(decodedImage, scaleMatrix, translationMatrix);
                    } else if (channels == 4)
                        core.declareTile(image, scaleMatrix, translationMatrix);
                }

                core.drawFrame();

                auto results = core.readFrame();

                std::thread([this, results, meta]() {
                    auto image = new ImageResult{};
                    image->width = 1024;
                    image->height = 1024;
                    image->size = image->width * image->height * 2;
                    image->format = RGB565;

                    image->data = new uint8_t[image->size];
                    std::copy(results.begin(), results.end(), image->data);

                    auto rootQuadcode = meta.getQuadcode();
                    mWorld->pushToImageEvents(LayerEvent::MakeImageEvent(rootQuadcode, image));
                }).detach();

                std::this_thread::sleep_for(10ms);
            }
        }

        void VulkanRenderContext::dispose() {

        }
    }
}