#include "VertexArray.h"
#include "GLCheck.h"

using namespace RTE;

VertexArray::VertexArray(GLuint drawMode): m_Mode(drawMode) {
	glGenVertexArrays(1, &m_VertexArray);
	glGenBuffers(1, &m_VertexBuffer);
	GL_CHECK(glBindVertexArray(m_VertexArray));
	GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer));
	GL_CHECK(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr));
	GL_CHECK(glEnableVertexAttribArray(0));
	GL_CHECK(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, uv))));
	GL_CHECK(glEnableVertexAttribArray(1));
	GL_CHECK(glBindVertexArray(0));
}

VertexArray::~VertexArray() {
	glDeleteBuffers(1, &m_VertexBuffer);
	glDeleteVertexArrays(1, &m_VertexArray);
}

void VertexArray::Draw() {
	glBindVertexArray(m_VertexArray);
	glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, m_Vertices.size(), m_Vertices.data(), GL_STREAM_DRAW);
	glDrawArrays(m_Mode, 0, m_Vertices.size());
	m_Vertices.clear();
}