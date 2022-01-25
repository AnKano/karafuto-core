#include <glm/gtc/type_ptr.hpp>

#include "MapCore.hpp"

#include "sources/remote/RasterRemoteSource.hpp"
#include "worlds/PlainWorld.hpp"
#include "queue/tasks/CallbackTask.hpp"
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

        mTilesData.setMaximalCount(5000);
        mTilesData.setCheckInterval(10);
        mTilesData.setStayAliveInterval(20);
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
        // request data for each tile not presented in cache
        for (const auto &item: tiles) {
            // process only visible tiles
            if (item.getVisibility() != Visible) continue;

            auto quadcode = item.getQuadcode();
            auto url = item.tileURL();

            auto inCache = mTilesData.keyInCache(quadcode, true);
            auto inSecondCache = mCommonTiles.keyInCache(quadcode, true);
            if (!inCache && !inSecondCache) {
                mTilesData.setOrReplace(quadcode, {});
                mCommonTiles.setOrReplace(quadcode, CommonTile{&mTilesData, &mRenderingContext, item});
            } else {
                mCommonTiles.getByKey(quadcode)->updateRelatedFields();
            }
        }
    }

    std::vector<PlainCommonTile> MapCore::getTiles() {
        if (!mWorld) throw std::runtime_error("world not initialized");

        std::vector<PlainCommonTile> commonTiles;

        auto tiles = mWorld->getTiles();
        for (const auto &item: tiles) {
            auto quadcode = item.getQuadcode();

            auto cachedTile = mCommonTiles.getByKey(quadcode);
            if (!cachedTile) continue;
            if (!cachedTile->isReady()) continue;

            commonTiles.emplace_back(mCommonTiles.getByKey(item.getQuadcode()));
        }

        return commonTiles;
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