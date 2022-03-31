////
//// Created by Anton Shubin on 1/18/2021.
////


#include <chrono>
#include <glm/glm.hpp>
#include <utility>

#include "core/MapCore.hpp"
#include "core/worlds/PlainWorld.hpp"
#include "core/meshes/PolylineMesh.hpp"

#ifdef __EMSCRIPTEN__

#include <emscripten/bind.h>

using namespace emscripten;

uintptr_t createMapCore() {
    return reinterpret_cast<uintptr_t>(new KCore::MapCore);
}

uintptr_t createTerrainedWorld(float lat, float lon) {
    return reinterpret_cast<uintptr_t>(new KCore::TerrainedWorld(lat, lon));
}

void addTerrainSource(uintptr_t worldPtr, std::string globPrefix, std::string globPostfix) {
    auto *world = reinterpret_cast<KCore::TerrainedWorld *>(worldPtr);

    auto terrainSrcPtr = new KCore::SRTMLocalSource();
    terrainSrcPtr->addSourcePart(globPrefix, globPostfix);

    world->registerSource(terrainSrcPtr, "terrain");
}

void addImageSource(uintptr_t worldPtr, std::string url) {
    auto *world = reinterpret_cast<KCore::TerrainedWorld *>(worldPtr);

    auto imageBaseSrcPtr = new KCore::RemoteSource(std::move(url));
    world->registerSource(imageBaseSrcPtr, "base");
}

void addGeoJSONSource(uintptr_t worldPtr, std::string filePath) {
    auto *world = reinterpret_cast<KCore::TerrainedWorld *>(worldPtr);

    auto jsonSrcPtr = new KCore::GeoJSONLocalSource;
    jsonSrcPtr->addSourcePart(std::move(filePath));

    world->registerSource(jsonSrcPtr, "json");
}

void attachWorldToMap(uintptr_t worldPtr, uintptr_t mapPtr) {
    auto *map = reinterpret_cast<KCore::MapCore *>(mapPtr);
    auto *world = reinterpret_cast<KCore::TerrainedWorld *>(worldPtr);

    world->commitWorldSetup();
    map->setWorldAdapter((KCore::BaseWorld *) world);
}

void update(uintptr_t instancePtr,
            float camPosX, float camPosY, float camPosZ,
            float targetPosX, float targetPosY, float targetPosZ,
            float upX, float upY, float upZ,
            float sceneWidth, float sceneHeight) {
    auto mapInstance = reinterpret_cast<KCore::MapCore *>(instancePtr);

    const float aspectRatio{sceneWidth / sceneHeight};

    // create camera that describe point of view and matrix
    glm::mat4 cameraProjectionMatrix;
    glm::mat4 cameraViewMatrix;

    glm::vec3 cameraOpenGlSpacePosition{camPosX, camPosY, camPosZ};
    glm::vec3 cameraOpenGlSpaceTarget{targetPosX, targetPosY, targetPosZ};
    glm::vec3 cameraOpenGlSpaceUp{upX, upY, upZ};

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

    mapInstance->update(cameraProjectionMatrix, cameraViewMatrix, cameraOpenGlSpacePosition);
}

uintptr_t getSyncEvents(uintptr_t instancePtr, uintptr_t lengthPtr) {
    auto mapInstance = reinterpret_cast<KCore::MapCore *>(instancePtr);

    auto *syncs = new std::vector<KCore::MapEvent>(mapInstance->getSyncEvents());
    auto *length = reinterpret_cast<int *>(lengthPtr);
    *length = syncs->size();
    return reinterpret_cast<uintptr_t>(syncs->data());
}

uintptr_t getAsyncEvents(uintptr_t instancePtr, uintptr_t lengthPtr) {
    auto mapInstance = reinterpret_cast<KCore::MapCore *>(instancePtr);

    auto *asyncs = new std::vector<KCore::MapEvent>(mapInstance->getAsyncEvents());
    auto *length = reinterpret_cast<int *>(lengthPtr);
    *length = asyncs->size();
    return reinterpret_cast<uintptr_t>(asyncs->data());
}

uintptr_t getMeshVertices(uintptr_t meshPtr, uintptr_t lengthPtr) {
    auto *mesh = reinterpret_cast<KCore::BaseMesh *>(meshPtr);
    auto *length = reinterpret_cast<int *>(lengthPtr);

    auto &vertices = mesh->getVertices();
    *length = (int) vertices.size() * 3;
    return reinterpret_cast<uintptr_t>(glm::value_ptr(vertices[0]));
}

uintptr_t getMeshNormals(uintptr_t meshPtr, uintptr_t lengthPtr) {
    auto *mesh = reinterpret_cast<KCore::BaseMesh *>(meshPtr);
    auto *length = reinterpret_cast<int *>(lengthPtr);

    auto &normals = mesh->getNormals();
    *length = (int) normals.size() * 3;
    return reinterpret_cast<uintptr_t>(glm::value_ptr(normals[0]));
}

uintptr_t getMeshUVs(uintptr_t meshPtr, uintptr_t lengthPtr) {
    auto *mesh = reinterpret_cast<KCore::BaseMesh *>(meshPtr);
    auto *length = reinterpret_cast<int *>(lengthPtr);

    auto &uvs = mesh->getUVs();
    *length = (int) uvs.size() * 2;
    return reinterpret_cast<uintptr_t>(glm::value_ptr(uvs[0]));
}

uintptr_t getMeshIndices(uintptr_t meshPtr, uintptr_t lengthPtr) {
    auto *mesh = reinterpret_cast<KCore::BaseMesh *>(meshPtr);
    auto *length = reinterpret_cast<int *>(lengthPtr);

    auto &indices = mesh->getIndices();
    *length = (int) indices.size();
    return reinterpret_cast<uintptr_t>(indices.data());
}

uintptr_t getJSONTransObjects(uintptr_t vectorPtr, uintptr_t lengthPtr) {
    auto *vector = reinterpret_cast<std::vector<KCore::GeoJSONTransObject> *>(vectorPtr);
    auto *length = reinterpret_cast<int *>(lengthPtr);

    *length = (int) vector->size();
    return reinterpret_cast<uintptr_t>(vector->data());
}

uintptr_t getPointerToMeshFromTransObject(uintptr_t meshPtr) {
    auto *obj = reinterpret_cast<KCore::GeoJSONTransObject *>(meshPtr);
    return reinterpret_cast<uintptr_t>(obj->mesh);
}

void releaseEventsVector(uintptr_t vectorPtr){
    auto* vecPtr = reinterpret_cast<std::vector<KCore::MapEvent> *>(vectorPtr);

    for (const auto &item: *vecPtr) {
//        if (item.type == KCore::EventType::ContentLoadedRender) {
//            delete (std::vector<uint8_t> *) (item.payload);
//        }
    }

    delete vecPtr;
}
void releaseArray(uintptr_t arrayPtr) {
    auto* arrPtr = reinterpret_cast<uint8_t *>(arrayPtr);
    delete[] arrPtr;
}

EMSCRIPTEN_BINDINGS(KarafutoCore) {
    function("createMapCore", &createMapCore);
    function("createTerrainedWorld", &createTerrainedWorld);

    function("addTerrainSource", &addTerrainSource);
    function("addImageSource", &addImageSource);
    function("addGeoJSONSource", &addGeoJSONSource);
    function("attachWorldToMap", &attachWorldToMap);

    function("update", &update);

    // Event vector interaction
    function("getSyncEvents", &getSyncEvents, emscripten::allow_raw_pointers());
    function("getAsyncEvents", &getAsyncEvents, emscripten::allow_raw_pointers());
    function("releaseEventsVector", &releaseEventsVector);
    function("releaseArray", &releaseArray);

    // BaseMesh* interaction
    function("getMeshVertices", &getMeshVertices, emscripten::allow_raw_pointers());
    function("getMeshUVs", &getMeshUVs, emscripten::allow_raw_pointers());
    function("getMeshNormals", &getMeshNormals, emscripten::allow_raw_pointers());
    function("getMeshIndices", &getMeshIndices, emscripten::allow_raw_pointers());

    // GeoJSON interaction
    function("getJSONTransObjects", &getJSONTransObjects, emscripten::allow_raw_pointers());
    function("getPointerToMeshFromTransObject", &getPointerToMeshFromTransObject, emscripten::allow_raw_pointers());
}

#else

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

    const uint16_t iterations{100};

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

#ifdef __EMSCRIPTEN__
    TerrainSource_ptr->addSourcePart("/assets/sources", ".hgt");
#else
    TerrainSource_ptr->addSourcePart("../build/assets/sources", ".hgt");
#endif
//    auto ImageSource_ptr = KCore::CreateRemoteSource("http://tile.openstreetmap.org/{z}/{x}/{y}.png");

    world->registerSource(TerrainSource_ptr, "terrain");

    auto jsonSource = new KCore::GeoJSONLocalSource;
    jsonSource->addSourcePart("assets/sources/few.geojson");
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

#endif