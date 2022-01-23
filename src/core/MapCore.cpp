#include <glm/gtc/type_ptr.hpp>

#include "MapCore.hpp"
#include "sources/local/TerrainFileSource.hpp"
#include "sources/remote/RasterRemoteSource.hpp"
#include "worlds/PlainWorld.hpp"
#include "queues/tasks/CallbackTask.hpp"

namespace KCore {
    MapCore::MapCore(float latitude, float longitude) {
        glm::vec2 origin_lat_lon{latitude, longitude};
        glm::vec2 origin_point{GeographyConverter::latLonToPoint(origin_lat_lon)};

        KCore::WorldConfig config{};
        config.GenerateMeta = true;

        mWorld = new TerrainedWorld(origin_lat_lon, origin_point, config);
        mWorld->updateFrustum(this->mCameraProjectionMatrix, this->mCameraViewMatrix);
        mWorld->setPosition(this->mCameraPosition);

        NetworkQueue networkQueue;
        networkQueue.perform_request();

        RasterRemoteSource source("https://tile.openstreetmap.org/", ".png");

        // !TODO: resolve this shit!
        std::this_thread::sleep_for(1000ms);

        auto queue = mRenderingContext.getQueue();
        for (int i = 0; i < 100; i++) {
            auto task = new CallbackTask([i]() {
                std::cout << "first quack " << i << std::endl;
            });

            queue->pushTask(task);
        }

        std::this_thread::sleep_for(1000ms);

        for (int i = 101; i < 201; i++) {
            auto task = new CallbackTask([i]() {
                std::cout << "second quack " << i << std::endl;
            });

            queue->pushTask(task);
        }
    }

    MapCore::~MapCore() {
        disposeThreads();
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
    }

    const std::vector<TileDescription> &MapCore::getTiles() {
        if (!mWorld) return mCommonTiles;

//        auto tiles = mWorld->getTiles();
//        for (const auto &item: tiles)
//            mCommonTiles.emplace_back(TileDescription(*item));

        return mCommonTiles;
    }

    const std::vector<TileDescription> &MapCore::getMetaTiles() {
        if (!mWorld) return mMetaTiles;

//        auto tiles = ((TerrainedWorld *) mWorld)->getMetaTiles();
//        for (const auto &item: tiles)
//            mMetaTiles.emplace_back(TileDescription(*item));

        return mMetaTiles;
    }

    void MapCore::disposeThreads() {
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