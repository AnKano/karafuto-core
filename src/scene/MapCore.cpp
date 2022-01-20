//
// Created by ash on 25.09.2021.
//

#include "MapCore.hpp"
#include "glm/gtc/type_ptr.hpp"

namespace KCore {
    MapCore::MapCore(float latitude, float longitude) {
        glm::vec2 origin_lat_lon{latitude, longitude};
        glm::vec2 origin_point{KCore::GeographyConverter::latLonToPoint(origin_lat_lon)};

        World = std::make_shared<DataWorld>(origin_lat_lon, origin_point);
        World->updateFrustum(this->CameraProjectionMatrix, this->CameraViewMatrix);
        World->setPosition(this->CameraPosition);

        mesh = new GridMesh(1.0, 1.0, 32);

        // instantiate TileRenderer with queue
        RendererThread = std::make_shared<std::thread>([this]() {
            Renderer = std::make_unique<KCore::TileRenderer>();
            Renderer->load_to_texture();
            Renderer->read_from_texture();
        });
        RendererThread->detach();
    }

    void MapCore::update(const float *cameraProjectionMatrix_ptr,
                         const float *cameraViewMatrix_ptr,
                         const float *cameraPosition_ptr) {
        this->CameraViewMatrix = glm::make_mat4x4(cameraViewMatrix_ptr);
        this->CameraProjectionMatrix = glm::make_mat4x4(cameraProjectionMatrix_ptr);
        this->CameraPosition = glm::make_vec3(cameraPosition_ptr);

        performUpdate();
    }

    void MapCore::update(const glm::mat4 &cameraProjectionMatrix,
                         const glm::mat4 &cameraViewMatrix,
                         const glm::vec3 &cameraPosition) {
        this->CameraViewMatrix = cameraViewMatrix;
        this->CameraProjectionMatrix = cameraProjectionMatrix;
        this->CameraPosition = cameraPosition;

        performUpdate();
    }

    void MapCore::performUpdate() {
        World->updateFrustum(this->CameraProjectionMatrix, this->CameraViewMatrix);
        World->setPosition(this->CameraPosition);

        World->calculateTiles();
        World->calculateMetaTiles();
    }

    const std::list<DataTile> &MapCore::getTiles() {
        return World->getTiles();
    }

    const std::list<DataTile> &MapCore::getMetaTiles() {
        return World->getMetaTiles();
    }

#ifdef __EMSCRIPTEN__
    void map_core::update(intptr_t camera_projection_matrix_addr,
        intptr_t camera_view_matrix_addr,
        intptr_t camera_position_addr) {
        return update(reinterpret_cast<float*>(camera_projection_matrix_addr),
            reinterpret_cast<float*>(camera_view_matrix_addr),
            reinterpret_cast<float*>(camera_position_addr));
    }


    const std::vector<DataTile>& map_core::emscripten_get_tiles() {
        Tiles.clear();

        for (const auto& item : get_tiles())
            Tiles.push_back(*item);

        return std::move(Tiles);
    }
    const std::vector<DataTile>& map_core::emscripten_get_meta_tiles() {
        MetaTiles.clear();

        for (const auto& item : get_height_tiles())
            MetaTiles.push_back(*item);

        return std::move(MetaTiles);
    }
#endif
}