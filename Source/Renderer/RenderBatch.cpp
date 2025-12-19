#include "RenderBatch.h"
#include "Constants.h"
#include "GLStateMan.h"
#include "Shader.h"
#include "glm/glm.hpp"

using namespace RTE;

VertexBuffer::VertexBuffer() {
	InitializeBuffers();
};

VertexBuffer::~VertexBuffer() {
	glDeleteBuffers(1, &m_VertexBuffer);
	glDeleteBuffers(1, &m_TexCoordBuffer);
	glDeleteBuffers(1, &m_NormalsBuffer);
	glDeleteBuffers(1, &m_ColorsBuffer);
	glDeleteBuffers(1, &m_IndexBuffer);
	glDeleteVertexArrays(1, &m_VertexArray);
};

VertexBuffer::VertexBuffer(int bufferSize) :
    m_Vertices(bufferSize * 4), m_TexCoords(bufferSize * 4), m_Normals(bufferSize * 4), m_Colors(bufferSize * 4), m_Indices(bufferSize * 6), m_BufferElements(bufferSize) {
	InitializeBuffers();
}

void VertexBuffer::InitializeBuffers() {

	// Setup index buffer for quad drawing
	for (int j,k = 0; j < (6 * m_BufferElements); j += 6) {
		m_Indices[j] = 4 * k;
		m_Indices[j + 1] = 4 * k + 1;
		m_Indices[j + 2] = 4 * k + 2;
		m_Indices[j + 3] = 4 * k;
		m_Indices[j + 4] = 4 * k + 2;
		m_Indices[j + 5] = 4 * k + 3;

		k++;
	}

	glGenBuffers(1, &m_VertexBuffer);
	glGenBuffers(1, &m_TexCoordBuffer);
	glGenBuffers(1, &m_NormalsBuffer);
	glGenBuffers(1, &m_ColorsBuffer);
	glGenBuffers(1, &m_IndexBuffer);
	glGenVertexArrays(1, &m_VertexArray);

	glBindVertexArray(m_VertexArray);
	glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(glm::vec3), m_Vertices.data(), GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(VertexAttribLocation::VERTEX);
	glVertexAttribPointer(VertexAttribLocation::VERTEX, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, m_TexCoordBuffer);
	glBufferData(GL_ARRAY_BUFFER, m_TexCoords.size() * sizeof(glm::vec2), m_TexCoords.data(), GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(VertexAttribLocation::TEXTURECOORDINATE);
	glVertexAttribPointer(VertexAttribLocation::TEXTURECOORDINATE, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, m_NormalsBuffer);
	glBufferData(GL_ARRAY_BUFFER, m_Normals.size() * sizeof(glm::vec3), m_Normals.data(), GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(VertexAttribLocation::NORMAL);
	glVertexAttribPointer(VertexAttribLocation::NORMAL, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, m_ColorsBuffer);
	glBufferData(GL_ARRAY_BUFFER, m_Colors.size() * sizeof(decltype(m_Colors)::value_type), m_Colors.data(), GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(VertexAttribLocation::COLOR);
	glVertexAttribPointer(VertexAttribLocation::COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Indices.size() * sizeof(decltype(m_Indices)::value_type), m_Indices.data(), GL_STATIC_DRAW);
}

RenderBatch::RenderBatch() {
	m_VertexBuffers = {VertexBuffer()};
}

void RenderBatch::BeginFrame() {
	m_CurrentDepth = 0;
	m_CurrentZ = c_DefaultDrawDepth;
}

void RenderBatch::Render() {
	if (m_VertexBuffers.m_Vertices.size() > 0) {
		SetupBuffers();
	}



}

void RenderBatch::SetupBuffers() {
	glBindVertexArray(m_VertexBuffers.m_VertexArray);

	glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffers.m_VertexBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, m_VertexBuffers.m_Vertices.size() * sizeof(decltype(m_VertexBuffers.m_Vertices)::value_type), m_VertexBuffers.m_Vertices.data());

	glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffers.m_TexCoordBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, m_VertexBuffers.m_Indices.size() * sizeof(decltype(m_VertexBuffers.m_Colors)::value_type), m_VertexBuffers.m_Colors.data());

	glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffers.m_NormalsBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, m_VertexBuffers.m_Indices.size() * sizeof(decltype(m_VertexBuffers.m_Normals)::value_type), m_VertexBuffers.m_Normals.data());

	glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffers.m_ColorsBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, m_VertexBuffers.m_Indices.size() * sizeof(decltype(m_VertexBuffers.m_Indices)::value_type), m_VertexBuffers.m_Indices.data());

	glBindVertexArray(0);
}
