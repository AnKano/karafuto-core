#include <chrono>
#include <glm/glm.hpp>
#include <utility>

#include "core/MapCore.hpp"
#include "core/meshes/PolylineMesh.hpp"

int main() {
    const uint16_t viewportWidth{1920}, viewportHeight{1080};
    const float aspectRatio{(float) viewportWidth / viewportHeight};

    // create camera that describe point of view and matrix
    glm::mat4 cameraProjectionMatrix;
    glm::mat4 cameraViewMatrix;

    glm::vec3 cameraOpenGlSpacePosition{1000.0f, 10000.0f, 10000.0f};
    glm::vec3 cameraOpenGlSpaceTarget{0.0f, 0.0f, 0.0f};
    glm::vec3 cameraOpenGlSpaceUp{0.0f, 1.0f, 0.0f};

    // setup matrices
    cameraViewMatrix = glm::lookAt(cameraOpenGlSpacePosition, cameraOpenGlSpaceTarget, cameraOpenGlSpaceUp);
    cameraProjectionMatrix = glm::perspective(glm::radians(60.0f), aspectRatio, 0.1f, 2500000.0f);

    const uint32_t iterations{10000};

    auto *core = KCore::CreateMapCore(46.9181f, 142.7189f);
    for (auto i = 0; i < iterations; i++) {
        cameraOpenGlSpacePosition.x += 150.0f;
        KCore::UpdateMapCore(
                core,
                reinterpret_cast<float *>(&cameraProjectionMatrix),
                reinterpret_cast<float *>(&cameraViewMatrix),
                reinterpret_cast<float *>(&cameraOpenGlSpacePosition)
        );
        auto a = KCore::GetEventsVector(core);
        for (const auto &item: *a) {
            switch (item.type) {
                case KCore::InFrustum: {
                    auto *body = static_cast<KCore::TileDescription *>(item.payload);
//                    std::cout << "In frustum " << item.quadcode << std::endl;
                    break;
                }
                case KCore::NotInFrustum:
//                    std::cout << "Not In frustum " << item.quadcode << std::endl;
                    break;
                default:
                    break;
            }
            std::cout << std::endl;
        }
    }

    return 0;
}