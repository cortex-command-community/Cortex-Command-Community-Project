#include "RenderBatch.h"
#include "Constants.h"
#include "GLStateMan.h"
#include "RenderMan.h"
#include "Shader.h"
#include "glad/gl.h"
#include "glm/glm.hpp"
#include <cstddef>
#include <algorithm>
#include "tracy/Tracy.hpp"
#include "tracy/TracyOpenGL.hpp"
#include "GLCheck.h"

using namespace RTE;

VertexBuffer::VertexBuffer() {
	m_Vertices.reserve(4 * c_DefaultBatchVAOElements);
	m_Indices.reserve(6 * c_DefaultBatchVAOElements);
	InitializeBuffers();
};

VertexBuffer::~VertexBuffer() {
	glDeleteBuffers(1, &m_VertexBuffer);
	glDeleteBuffers(1, &m_IndexBuffer);
	glDeleteVertexArrays(1, &m_VertexArray);
	m_VertexBuffer = 0;
	m_IndexBuffer = 0;
	m_VertexArray = 0;
};

VertexBuffer::VertexBuffer(int bufferSize) :
    m_BufferElements(bufferSize) {
	m_Vertices.reserve(4 * bufferSize);
	m_Indices.reserve(6 * bufferSize);
	InitializeBuffers();
}

void VertexBuffer::InitializeBuffers() {

	GL_CHECK(glGenBuffers(1, &m_VertexBuffer));
	GL_CHECK(glGenBuffers(1, &m_IndexBuffer));
	GL_CHECK(glGenVertexArrays(1, &m_VertexArray));

	GL_CHECK(glBindVertexArray(m_VertexArray));
	GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer));
	GL_CHECK(glBufferData(GL_ARRAY_BUFFER, m_BufferElements * 4 * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW));

	GL_CHECK(glEnableVertexAttribArray(VertexAttribLocation::VERTEX));
	GL_CHECK(glVertexAttribPointer(VertexAttribLocation::VERTEX, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0));

	GL_CHECK(glEnableVertexAttribArray(VertexAttribLocation::TEXTURECOORDINATE));
	GL_CHECK(glVertexAttribPointer(VertexAttribLocation::TEXTURECOORDINATE, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, m_TextureUV)));

	GL_CHECK(glEnableVertexAttribArray(VertexAttribLocation::COLOR));
	GL_CHECK(glVertexAttribPointer(VertexAttribLocation::COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, m_Color)));

	GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexBuffer));
	GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_BufferElements * 6 * sizeof(decltype(m_Indices)::value_type), nullptr, GL_DYNAMIC_DRAW));
	glBindVertexArray(0);
}

RenderBatch::RenderBatch() = default;

void RenderBatch::BeginFrame() {
	m_CurrentDepth = 0;
	m_CurrentZ = c_DefaultDrawDepth;
	m_VertexBuffers.m_Vertices.clear();
	m_VertexBuffers.m_Indices.clear();
	m_DrawCalls.clear();
}

void RenderBatch::EndFrame() {
	ZoneScoped;
	ApplyDrawCalls();
	m_VertexBuffers.UpdateBuffers();
}

void RenderBatch::Render() {
	ZoneScoped;
	TracyGpuZone("Render");
	glBindVertexArray(m_VertexBuffers.m_VertexArray);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VertexBuffers.m_IndexBuffer);

	const Shader* currentShader = g_RenderMan.GetDefaultShader();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_RenderMan.GetPaletteTexture());
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, g_RenderMan.GetShapeTexture());
	Camera* currentCamera = g_RenderMan.GetActiveCamera();
	currentShader->Enable();
	currentShader->SetInt(currentShader->GetTextureUniform(), 1);
	currentShader->SetInt(currentShader->GetPaletteUniform(), 0);
	if(!currentCamera) {
		currentShader->SetMatrix4f(currentShader->GetProjectionUniform(), glm::mat4(1.0f));
		currentShader->SetMatrix4f(currentShader->GetTransformUniform(), glm::mat4(1.0f));
		currentShader->SetMatrix4f(currentShader->GetUVTransformUniform(), glm::mat4(1.0f));
		currentShader->SetMatrix4f(currentShader->GetViewUniform(), glm::mat4(1.0f));
	} else {
		currentShader->SetMatrix4f(currentShader->GetProjectionUniform(), currentCamera->GetProjection());
		currentShader->SetMatrix4f(currentShader->GetTransformUniform(), glm::mat4(1.0f));
		currentShader->SetMatrix4f(currentShader->GetUVTransformUniform(), glm::mat4(1.0f));
		currentShader->SetMatrix4f(currentShader->GetViewUniform(), currentCamera->GetView());
	}

	GLuint indexOffset = 0;
	std::vector<GLuint> activeTextures = {g_RenderMan.GetPaletteTexture(), g_RenderMan.GetShapeTexture()};
	GLuint activeTexture = g_RenderMan.GetShapeTexture();
	GLint maxActiveTextures;
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxActiveTextures);
	BlendMode activeBlendMode(Blend::ALPHA);
	activeBlendMode.Enable();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glDisable(GL_MULTISAMPLE);
	for (auto& drawCall: m_DrawCalls) {
		if (drawCall->m_Shader && drawCall->m_Shader != currentShader) {
			currentShader = drawCall->m_Shader;
			currentShader->Enable();
			currentShader->SetInt(currentShader->GetTextureUniform(), 0);
			if (currentCamera) {
				currentShader->SetMatrix4f(currentShader->GetProjectionUniform(), currentCamera->GetProjection());
				currentShader->SetMatrix4f(currentShader->GetViewUniform(), currentCamera->GetView());
				currentShader->SetMatrix4f(currentShader->GetTransformUniform(), glm::mat4(1.0f));
				currentShader->SetMatrix4f(currentShader->GetUVTransformUniform(), glm::mat4(1.0f));
			}
		}

		currentShader->SetMatrix4f(currentShader->GetTransformUniform(), glm::mat4(1.0f));
		currentShader->SetMatrix4f(currentShader->GetUVTransformUniform(), glm::mat4(1.0f));

		if (drawCall->m_Camera && drawCall->m_Camera != currentCamera) {
			currentCamera = drawCall->m_Camera;
			currentShader->SetMatrix4f(currentShader->GetProjectionUniform(), currentCamera->GetProjection());
			currentShader->SetMatrix4f(currentShader->GetViewUniform(), currentCamera->GetView());
		}

		for (auto& uniform: drawCall->m_UniformValues) {
			uniform->Enable();
		}

		if (drawCall->m_Scissor != std::nullopt) {
			FloatRect& scissor = *drawCall->m_Scissor;
			glEnable(GL_SCISSOR_TEST);
			glScissor(scissor.x, scissor.y, scissor.w, scissor.h);
		} else {
			glDisable(GL_SCISSOR_TEST);
		}

		if (drawCall->m_TextureId != activeTexture) {
			glBindTexture(GL_TEXTURE_2D, drawCall->m_TextureId);
			activeTexture = drawCall->m_TextureId;
		}
		if (drawCall->m_BlendMode != activeBlendMode) {
			drawCall->m_BlendMode.Enable();
			activeBlendMode = drawCall->m_BlendMode;
		}

		currentShader->SetBool("rteIndexed", drawCall->m_Indexed);
		GL_CHECK(glDrawElements(GL_TRIANGLES, drawCall->m_Indices.size(), GL_UNSIGNED_INT, (GLvoid*)(indexOffset * sizeof(GLuint))));
		indexOffset += drawCall->m_Indices.size();
	}
	m_DrawCalls.clear();
	m_VertexBuffers.m_Vertices.clear();
	m_VertexBuffers.m_Indices.clear();
}

void RenderBatch::ApplyDrawCalls() {
	ZoneScoped;

	// TODO: Sort and batch DrawCalls by shader and transparency.
	//std::stable_sort(m_DrawCalls.begin(), m_DrawCalls.end(), [](auto r, auto l) { return r->m_TextureId < l->m_TextureId; });

	for (auto drawCall: m_DrawCalls) {
		RTEAssert(drawCall.use_count() == 2, "DrawCall still in use on EndFrame!");
		size_t vertexCount = m_VertexBuffers.m_Vertices.size();

		for (auto& index: drawCall->m_Indices) {
			index += vertexCount;
		}
		m_VertexBuffers.m_Vertices.insert(
		    m_VertexBuffers.m_Vertices.end(),
		    std::make_move_iterator(drawCall->m_Vertices.begin()),
		    std::make_move_iterator(drawCall->m_Vertices.end()));

		m_VertexBuffers.m_Indices.insert(
		    m_VertexBuffers.m_Indices.end(),
		    std::make_move_iterator(drawCall->m_Indices.begin()),
		    std::make_move_iterator(drawCall->m_Indices.end()));
	}
}

void VertexBuffer::UpdateBuffers() {
	ZoneScoped;
	TracyGpuZone("VertexBuffer::UpdateBuffers");
	glBindVertexArray(m_VertexArray);

	GLint bufferSize;
	GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer));
	GL_CHECK(glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufferSize));
	if (m_Vertices.size() * sizeof(decltype(m_Vertices)::value_type) < bufferSize) {
		GL_CHECK(glBufferSubData(GL_ARRAY_BUFFER, 0, m_Vertices.size() * sizeof(decltype(m_Vertices)::value_type), m_Vertices.data()));
	} else {
		GL_CHECK(glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(decltype(m_Vertices)::value_type), m_Vertices.data(), GL_DYNAMIC_DRAW));
	}

	GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexBuffer));
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufferSize);
	if (m_Indices.size() * sizeof(decltype(m_Indices)::value_type) < bufferSize) {
		GL_CHECK(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, m_Indices.size() * sizeof(decltype(m_Indices)::value_type), m_Indices.data()));
	} else {
		GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Indices.size() * sizeof(decltype(m_Indices)::value_type), m_Indices.data(), GL_DYNAMIC_DRAW));
	}
}
