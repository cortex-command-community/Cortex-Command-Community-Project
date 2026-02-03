#pragma once
#include "BlendMode.h"
#include "Shader.h"
#include "Vertex.h"
#include "Camera.h"

#include "glad/gl.h"
#include <optional>
#include <vector>

namespace RTE {
	class DrawCall {
	public:
		friend class RenderMan;
		std::vector<Vertex> m_Vertices;
		std::vector<int> m_Indices;
		bool m_Indexed{true};
		GLuint m_TextureId{0};
		BlendMode m_BlendMode{};
		const Shader* m_Shader{nullptr};
		std::vector<std::shared_ptr<UniformValueType>> m_UniformValues;
		Camera* m_Camera;
		std::optional<FloatRect> m_Scissor{std::nullopt};
		GLenum m_DrawMode{GL_TRIANGLES};

	private:
		DrawCall(int id, Camera* camera) :
			m_Camera(camera), m_Id(id) {}
		DrawCall(DrawCall&& drawCall) = default;
		int m_Id{0};
		DrawCall(DrawCall&) = delete;
		DrawCall operator=(DrawCall&) = delete;
	};
} // namespace RTE
