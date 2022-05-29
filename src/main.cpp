#include <chrono>
#include <glm/glm.hpp>
#include <utility>

#include "core/MapCore.hpp"
#include "core/worlds/PlainWorld.hpp"
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
    {
        cameraViewMatrix = glm::lookAt(
                cameraOpenGlSpacePosition,
                cameraOpenGlSpaceTarget,
                cameraOpenGlSpaceUp
        );

        cameraProjectionMatrix = glm::perspective(
                glm::radians(60.0f), aspectRatio,
                0.1f, 2500000.0f
        );
    }

    const uint32_t iterations{10000};

    // 46.9181f, 142.7189f is latitude and longitude of
    // the surroundings of the city of Yuzhno-Sakhalinsk
//    auto *world = new KCore::PlainWorld{46.9181f, 142.7189f};
//
//    auto jsonSource = new KCore::GeoJSONLocalSource;
//    jsonSource->addSourcePart("assets/sources/points.geojson");
//    world->registerSource(jsonSource, "json");
//
//    auto terrainSource = new KCore::SRTMLocalSource;
//    terrainSource->addSourcePart("assets/sources/N45E141.hgt");
//    terrainSource->addSourcePart("assets/sources", ".hgt");
//    world->registerSource(terrainSource, "terrain");
//
//    auto imageSource = new KCore::RemoteSource("http://tile.openstreetmap.org/{z}/{x}/{y}.png");
//    world->registerSource(imageSource, "base");
//
//    KCore::MapCore core;
//    core.setWorldAdapter(world);

//    auto *world = new KCore::TerrainedWorld{46.9181f, 142.7189f};
//
//    auto jsonSource = new KCore::GeoJSONLocalSource;
//    jsonSource->addSourcePart("assets/sources/points.geojson");
//    world->registerSource(jsonSource, "json");
//
//    auto terrainSource = new KCore::SRTMLocalSource;
//    terrainSource->addSourcePart("assets/sources", ".hgt");
//    world->registerSource(terrainSource, "terrain");
//
//    auto imageSource = new KCore::RemoteSource("http://tile.openstreetmap.org/{z}/{x}/{y}.png");
//    world->registerSource(imageSource, "base");
//
//    KCore::MapCore core;
//    core.setWorldAdapter(world);

    KCore::MapCore core;
    auto *world = new KCore::TerrainedWorld(46.7197f, 142.5233f);

    auto TerrainSource_ptr = new KCore::SRTMLocalSource();

    TerrainSource_ptr->addSourcePart("../build/assets/sources", ".hgt");
    auto ImageSource_ptr = KCore::CreateRemoteSource("http://tile.openstreetmap.org/{z}/{x}/{y}.png");

    world->registerSource(TerrainSource_ptr, "terrain");
    world->registerSource(ImageSource_ptr, "base");

    auto jsonSource = new KCore::GeoJSONLocalSource;
    jsonSource->addSourcePart("assets/sources/12.geojson");
    world->registerSource(jsonSource, "json");

    world->commitWorldSetup();

    core.setWorldAdapter((KCore::BaseWorld *) world);

    auto start = std::chrono::system_clock::now();
    for (auto i = 0; i < iterations; i++) {
        core.update(cameraProjectionMatrix, cameraViewMatrix, cameraOpenGlSpacePosition);

        auto a = core.getSyncEvents();
        auto b = core.getAsyncEvents();

        for (const auto &item: a) {
            switch (item.type) {
                case KCore::InFrustum:
                    std::cout << "In frustum " << item.quadcode << std::endl;
                    break;
                case KCore::NotInFrustum:
                    std::cout << "Not In frustum " << item.quadcode << std::endl;
                    break;
                default:
                    break;
            }
            std::cout << std::endl;
        }
    }
    auto elapsed = std::chrono::system_clock::now() - start;

    std::cout << (elapsed / std::chrono::microseconds(1)) / iterations
              << " microseconds per iterations" << std::endl;

    return 0;
}