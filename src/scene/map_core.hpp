#pragma once

#include <vector>
#include "glm/glm.hpp"
#include "geography/data_tile.hpp"
#include "geography/data_world.hpp"

namespace kcore {
	class map_core {
	private:
		glm::mat4 m_camera_view_matrix{}, m_camera_projection_matrix{};
		glm::vec3 m_camera_position{};

		std::shared_ptr<data_world> m_world;
		std::vector<data_tile> m_tiles;
		std::vector<data_tile> m_meta_tiles;

	public:
		map_core() = default;

		map_core(float latitude, float longitude);

		void update(const glm::mat4& camera_projection_matrix,
			const glm::mat4& camera_view_matrix,
			const glm::vec3& camera_position);

		void update(const float* camera_projection_matrix_raw,
			const float* camera_view_matrix_raw,
			const float* camera_position_data);

		const std::list<data_tile>& get_tiles();

		const std::list<data_tile>& get_meta_tiles();

#ifdef __EMSCRIPTEN__
		void update(intptr_t camera_projection_matrix_addr,
			intptr_t camera_view_matrix_addr,
			intptr_t camera_position_addr);

		const std::vector<data_tile>& emscripten_get_tiles();

		const std::vector<data_tile>& emscripten_get_meta_tiles();
#endif

	private:
		void general_update();
	};
}
