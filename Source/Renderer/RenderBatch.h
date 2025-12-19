#pragma once
#include "Shader.h"
#include "glad/gl.h"
#include <optional>
#include <vector>
#include "BlendMode.h"
#include "Constants.h"

#include "glm/fwd.hpp"


namespace RTE {
	class Shader;


	struct DrawCall {
		int m_VertexCount{0};
		int m_VertexAlignment{0};
		GLuint textureId{0};
		BlendMode m_BlendMode{};
		Shader* m_Shader{nullptr};
		std::optional<std::vector<UniformValueType>> m_UniformValues{std::nullopt};
	};

	struct VertexBuffer {
		std::vector<glm::vec3> m_Vertices{c_DefaultBatchVAOElements * 4};
		std::vector<glm::vec2> m_TexCoords{c_DefaultBatchVAOElements * 4};
		std::vector<glm::vec3> m_Normals{c_DefaultBatchVAOElements * 4};
		std::vector<glm::vec<4, unsigned char>> m_Colors{c_DefaultBatchVAOElements * 4};
		std::vector<unsigned int> m_Indices{c_DefaultBatchVAOElements * 6};
		GLuint m_VertexArray{0};
		GLuint m_VertexBuffer{0};
		GLuint m_TexCoordBuffer{0};
		GLuint m_NormalsBuffer{0};
		GLuint m_ColorsBuffer{0};
		GLuint m_IndexBuffer{0};
		int m_BufferElements{c_DefaultBatchVAOElements};
		~VertexBuffer();
		VertexBuffer();
		VertexBuffer(int bufferSize);

	private:
		void InitializeBuffers();
	};

	/// Render batch based on raysan5's raylib
	struct RenderBatch {
	public:
		VertexBuffer m_VertexBuffers{};
		std::vector<DrawCall> m_DrawCalls{};
		int m_VertexCount{0};
		float m_CurrentDepth{0.0f};
		float m_CurrentZ{0.0f};

		RenderBatch();

		void BeginFrame();
		void Render();

	private:
		void SetupBuffers();
	};
}
