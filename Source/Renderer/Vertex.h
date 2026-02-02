#pragma once
#include "glm/glm.hpp"

namespace RTE {
	struct Vertex {
		Vertex(glm::vec2 pos, glm::u8vec4 color = {255, 255, 255, 255});
		Vertex(glm::vec2 pos, glm::vec2 uv, glm::u8vec4 color = {255, 255, 255, 255});
		Vertex(glm::vec2 pos, glm::vec2 uv, float layerDepth, glm::u8vec4 color = {255, 255, 255, 255});

		glm::vec3 m_Pos{};
		glm::vec2 m_TextureUV{};
		glm::u8vec4 m_Color{};
	};
}
