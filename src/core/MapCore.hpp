#pragma once


#include <vector>
#include <thread>

#include "glm/glm.hpp"

#include "geography/TileDescription.hpp"
#include "worlds/TerrainedWorld.hpp"
#include "meshes/GridMesh.hpp"
#include "rendering/RenderContext.hpp"

#include "cache/LimitedCache.hpp"
#include "geography/tiles/CommonTile.hpp"
#include "geography/tiles/MetaTile.hpp"
#include "geography/plain/PlainCommonTile.hpp"

namespace KCore {
    class MapCore {
    private:
        glm::mat4 mCameraViewMatrix{}, mCameraProjectionMatrix{};
        glm::vec3 mCameraPosition{};

        BaseWorld *mWorld{};

        LimitedCache<std::map<std::string, std::shared_ptr<void>>> mTilesData;
        TimeoutCache<CommonTile> mCommonTiles;
//        TimeoutCache<MetaTile> mMetaTiles;

        RenderContext mRenderingContext;

    public:
        MapCore(float latitude, float longitude);

        ~MapCore();

        void update(const glm::mat4 &cameraProjectionMatrix,
                    const glm::mat4 &cameraViewMatrix,
                    const glm::vec3 &cameraPosition);

        void update(const float *cameraProjectionMatrix_ptr,
                    const float *cameraViewMatrix_ptr,
                    const float *cameraPosition_ptr);

        std::vector<PlainCommonTile> getTiles();

        const std::vector<TileDescription> &getMetaTiles();

#ifdef __EMSCRIPTEN__
        void update(intptr_t camera_projection_matrix_addr,
            intptr_t camera_view_matrix_addr,
            intptr_t camera_position_addr);

        const std::vector<TileDescription>& emscripten_get_tiles();

        const std::vector<TileDescription>& emscripten_get_meta_tiles();
#endif

    private:
        void dispose();

        void performUpdate();
    };

    extern "C" {
    DllExport KCore::MapCore *InstantiateMapCore(float latitude, float longitude);

    DllExport void UpdateMapCore(KCore::MapCore *mapCore,
                                 float *cameraProjectionMatrix,
                                 float *cameraViewMatrix,
                                 float *cameraPosition);

    DllExport KCore::PlainCommonTile *GetTiles(KCore::MapCore *mapCore, int &length);
    }

}
