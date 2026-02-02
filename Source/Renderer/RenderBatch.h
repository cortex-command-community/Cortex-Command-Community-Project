#pragma once
#include "Shader.h"
#include "BlendMode.h"
#include "Constants.h"
#include "DrawCall.h"

#include "glad/gl.h"
#include "glm/fwd.hpp"
#include <optional>
#include <vector>


namespace RTE {
	class Shader;



	struct VertexBuffer {
		std::vector<Vertex> m_Vertices{};
		std::vector<unsigned int> m_Indices{};
		GLuint m_VertexArray{0};
		GLuint m_VertexBuffer{0};
		GLuint m_IndexBuffer{0};
		int m_BufferElements{c_DefaultBatchVAOElements};
		~VertexBuffer();
		VertexBuffer();
		VertexBuffer(int bufferSize);
		VertexBuffer(VertexBuffer&& vertexBuffer) = default;

		void UpdateBuffers();

		VertexBuffer& operator=(VertexBuffer&& rhs) = default;
	private:
		void InitializeBuffers();
		VertexBuffer(const VertexBuffer&) =delete;
		VertexBuffer& operator=(const VertexBuffer&) = delete;
	};

	/// Render batch based on raysan5's raylib
	struct RenderBatch {
	public:
		constexpr static float c_DrawDepthIncrement = 1.0f/20000.0f;
		VertexBuffer m_VertexBuffers{};
		std::vector<std::shared_ptr<DrawCall>> m_DrawCalls{};
		float m_CurrentDepth{0.0f};
		float m_CurrentZ{0.0f};
		const Shader* m_DefaultShader{nullptr};
		glm::mat4 m_CurrentView{1.0f};
		glm::mat4 m_CurrentProjection{1.0f};

		RenderBatch();

		void BeginFrame();
		void EndFrame();
		void Render();

		/// Collect draw calls and render.
		void Flush() { EndFrame(); Render(); }

	private:
		void ApplyDrawCalls();
	};
}
