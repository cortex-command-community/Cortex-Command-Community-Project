#pragma once
#include <vector>
#include "glm/vec2.hpp"
#include "glm/vec4.hpp"
#include "glad/gl.h"
namespace RTE {
	struct Vertex {
		glm::vec2 pos;
		glm::vec2 uv;
	};
	class VertexArray {
		VertexArray(GLuint drawMode);
		~VertexArray();
		void PushVertices(const std::vector<Vertex>& vertices);

		void Draw();
		GLuint GetVertexArray() { return m_VertexArray; }

	private:
		GLuint m_VertexArray;
		GLuint m_VertexBuffer;
		std::vector<Vertex> m_Vertices;
		GLuint m_Mode;
	};
} // namespace RTE