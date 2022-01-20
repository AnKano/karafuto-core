#pragma once

#include <vector>
#include <thread>

#include "glm/glm.hpp"

#include "geography/DataTile.hpp"
#include "geography/DataWorld.hpp"
#include "rendering/TileRenderer.hpp"
#include "meshes/GridMesh.hpp"

namespace KCore {
    class MapCore {
    private:
        glm::mat4 CameraViewMatrix{}, CameraProjectionMatrix{};
        glm::vec3 CameraPosition{};

        std::shared_ptr<DataWorld> World;
        std::vector<DataTile> Tiles;
        std::vector<DataTile> MetaTiles;

        std::shared_ptr<std::thread> RendererThread;

        std::unique_ptr<KCore::TileRenderer> Renderer;

        KCore::IMesh* mesh;

    public:
        MapCore() = default;

        MapCore(float latitude, float longitude);

        void update(const glm::mat4 &cameraProjectionMatrix,
                    const glm::mat4 &cameraViewMatrix,
                    const glm::vec3 &cameraPosition);

        void update(const float *cameraProjectionMatrix_ptr,
                    const float *cameraViewMatrix_ptr,
                    const float *cameraPosition_ptr);

        const std::list<DataTile> &getTiles();

        const std::list<DataTile> &getMetaTiles();

#ifdef __EMSCRIPTEN__
        void update(intptr_t camera_projection_matrix_addr,
            intptr_t camera_view_matrix_addr,
            intptr_t camera_position_addr);

        const std::vector<DataTile>& emscripten_get_tiles();

        const std::vector<DataTile>& emscripten_get_meta_tiles();
#endif

    private:
        void performUpdate();
    };
}
