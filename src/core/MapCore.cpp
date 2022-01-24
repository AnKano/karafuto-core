#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "MapCore.hpp"

#include "sources/local/SRTMFileSource.hpp"
#include "sources/remote/RasterRemoteSource.hpp"

#include "worlds/PlainWorld.hpp"
#include "queue/tasks/CallbackTask.hpp"
#include "queue/tasks/NetworkTask.hpp"

#include "meshes/PolygonMesh.hpp"

namespace KCore {
    MapCore::MapCore(float latitude, float longitude) {
        glm::vec2 origin_lat_lon{latitude, longitude};
        glm::vec2 origin_point{GeographyConverter::latLonToPoint(origin_lat_lon)};

        KCore::WorldConfig config{};
        config.GenerateMeta = true;

        mWorld = new TerrainedWorld(origin_lat_lon, origin_point, config);
        mWorld->updateFrustum(this->mCameraProjectionMatrix, this->mCameraViewMatrix);
        mWorld->setPosition(this->mCameraPosition);

        PolygonMesh mesh;
        mesh.createMesh();

        RasterRemoteSource source("https://tile.openstreetmap.org/", ".png");
        SRTMFileSource hgt("assets/sources/N48E142.hgt");

        mTilesData.setStayAliveInterval(25000);
    }

    MapCore::~MapCore() {
        dispose();
    }

    void MapCore::update(const float *cameraProjectionMatrix_ptr,
                         const float *cameraViewMatrix_ptr,
                         const float *cameraPosition_ptr) {
        this->mCameraViewMatrix = glm::make_mat4x4(cameraViewMatrix_ptr);
        this->mCameraProjectionMatrix = glm::make_mat4x4(cameraProjectionMatrix_ptr);
        this->mCameraPosition = glm::make_vec3(cameraPosition_ptr);

        performUpdate();
    }

    void MapCore::update(const glm::mat4 &cameraProjectionMatrix,
                         const glm::mat4 &cameraViewMatrix,
                         const glm::vec3 &cameraPosition) {
        this->mCameraViewMatrix = cameraViewMatrix;
        this->mCameraProjectionMatrix = cameraProjectionMatrix;
        this->mCameraPosition = cameraPosition;

        performUpdate();
    }

    void MapCore::performUpdate() {
        if (!mWorld) return;

        mWorld->updateFrustum(this->mCameraProjectionMatrix, this->mCameraViewMatrix);
        mWorld->setPosition(this->mCameraPosition);
        mWorld->update();

        auto tiles = mWorld->getTiles();
        for (const auto &item: tiles) {
            if (item.getVisibility() != Visible) continue;

            auto quadcode = item.getQuadcode();
            auto url = item.tileURL();

            auto inCache = mTilesData.keyInCache(quadcode, true);
            if (!inCache) {
                mTilesData.setOrReplace(quadcode, {});
                mRenderingContext.pushTaskToQueue(new NetworkTask(&mTilesData, quadcode, url));
            }

//            auto tilecode = item.getTilecode();
//            if (tilecode.x == 0 && tilecode.y == 0 && tilecode.z == 1) {
//                auto task = new CallbackTask([item]() {
//                    std::cout << "invoked by " << glm::to_string(item.getTilecode()) << std::endl;
//                });
//                mRenderingContext.pushTaskToQueue(task);
//
//            }
        }
    }

    const std::vector<TileDescription> &MapCore::getTiles() {
        if (!mWorld) throw std::runtime_error("world not initialized");
        return mWorld->getTiles();
    }

    const std::vector<TileDescription> &MapCore::getMetaTiles() {
        if (!mWorld) throw std::runtime_error("world not initialized");
        return ((TerrainedWorld *) mWorld)->getMetaTiles();
    }

    void MapCore::dispose() {
        // dispose rendering context thread
        mRenderingContext.setShouldClose(true);
        while (mRenderingContext.getWorkingStatus());
    }

#ifdef __EMSCRIPTEN__
    void map_core::update(intptr_t camera_projection_matrix_addr,
        intptr_t camera_view_matrix_addr,
        intptr_t camera_position_addr) {
        return update(reinterpret_cast<float*>(camera_projection_matrix_addr),
            reinterpret_cast<float*>(camera_view_matrix_addr),
            reinterpret_cast<float*>(camera_position_addr));
    }


    const std::vector<TileDescription>& map_core::emscripten_get_tiles() {
        mCommonTiles.clear();

        for (const auto& item : get_tiles())
            mCommonTiles.push_back(*item);

        return std::move(mCommonTiles);
    }
    const std::vector<TileDescription>& map_core::emscripten_get_meta_tiles() {
        mMetaTiles.clear();

        for (const auto& item : get_height_tiles())
            mMetaTiles.push_back(*item);

        return std::move(mMetaTiles);
    }
#endif
}