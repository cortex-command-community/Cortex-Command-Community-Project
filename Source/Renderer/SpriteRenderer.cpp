#include "SpriteRenderer.h"
#include "glad/gl.h"
#include "GLResourceMan.h"
#include "Shader.h"
#include "GLCheck.h"
#include "glm/gtc/matrix_transform.hpp"

using namespace RTE;

SpriteRenderer::SpriteRenderer(const Shader* shader, const FloatRect& size) {
	m_Size = size;
	m_Shader = shader;
	InitGLPointers();
}

SpriteRenderer::~SpriteRenderer() {
}
void SpriteRenderer::InitGLPointers() {
	GLuint VBO;
	float vertices[] = {
	    // pos      // tex
	    0.0f, 1.0f, 0.0f, 1.0f,
	    1.0f, 0.0f, 1.0f, 0.0f,
	    0.0f, 0.0f, 0.0f, 0.0f,

	    0.0f, 1.0f, 0.0f, 1.0f,
	    1.0f, 1.0f, 1.0f, 1.0f,
	    1.0f, 0.0f, 1.0f, 0.0f};

	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &VBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindVertexArray(m_VAO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	GL_CHECK(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float))));
	GL_CHECK(glEnableVertexAttribArray(1));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void SpriteRenderer::Draw(BITMAP* image, glm::vec2 pos, float angle, glm::vec2 scale) {
	glDisable(GL_DEPTH_TEST);
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(pos, 0.0f));
	model = glm::translate(model, glm::vec3(0.5f, 0.5f, 0.0f));
	model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::translate(model, glm::vec3(-0.5f, -0.5f, 0.0f));
	model = glm::scale(model, glm::vec3(image->w * scale.x, image->h * scale.y, 1.0f));
	glm::mat4 proj = glm::ortho(m_Size.x, m_Size.x + m_Size.w, m_Size.y, m_Size.y+m_Size.h, -1.0f, 1.0f);
	std::cout << pos.x << " " << pos.y << "\n" << std::endl;
	std::cout << image->w << " " << image->h << " " << m_Size.w << " " << m_Size.h << " " << m_Size.x << " " << m_Size.y << "\n---" << std::endl;

	m_Shader->Use();
	glBindVertexArray(m_VAO);
	glActiveTexture(GL_TEXTURE0);
	GL_CHECK(glBindTexture(GL_TEXTURE_2D, g_GLResourceMan.GetStaticTextureFromBitmap(image)));
	m_Shader->SetMatrix4f(m_Shader->GetTransformUniform(), model);
	m_Shader->SetMatrix4f(m_Shader->GetProjectionUniform(), proj);
	m_Shader->SetMatrix4f(m_Shader->GetUVTransformUniform(), glm::mat4(1.0f));
	GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, 6));
}
void SpriteRenderer::DrawPivot(BITMAP* image, glm::vec2 pos, float angle, glm::vec2 scale, glm::vec2 pivot) {
}
void SpriteRenderer::Draw(BITMAP* image, FloatRect fromRegion, glm::vec2 pos) {
	Draw(image, fromRegion, {fromRegion.w, fromRegion.h}, pos);
}
void SpriteRenderer::Draw(BITMAP* image, FloatRect fromRegion, glm::vec2 toSize, glm::vec2 pos, float angle, glm::vec2 pivot, glm::vec2 scale) {
	glDisable(GL_DEPTH_TEST);
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(pos, 0.0f));
	model = glm::translate(model, glm::vec3(0.5f * fromRegion.w, 0.5f * fromRegion.h, 0.0f) + glm::vec3(pivot, 0.0f));
	model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::translate(model, glm::vec3(-0.5f * fromRegion.w, -0.5f * fromRegion.h, 0.0f) - glm::vec3(pivot, 0.0f));
	model = glm::scale(model, glm::vec3(toSize, 1.0f));

	glm::mat4 uvRegion = glm::mat4(1.0f);
	uvRegion = glm::translate(uvRegion, glm::vec3(fromRegion.x / static_cast<float>(image->w), fromRegion.y / static_cast<float>(image->h), 0.0f));
	uvRegion = glm::scale(uvRegion, glm::vec3(fromRegion.w / static_cast<float>(image->w), fromRegion.h / static_cast<float>(image->h), 1.0f));

	glm::mat4 proj = glm::ortho(m_Size.x, m_Size.x + m_Size.w, m_Size.y, m_Size.y+m_Size.h, -1.0f, 1.0f);

	m_Shader->Use();
	glBindVertexArray(m_VAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_GLResourceMan.GetStaticTextureFromBitmap(image));
	m_Shader->SetMatrix4f(m_Shader->GetTransformUniform(), model);
	m_Shader->SetMatrix4f(m_Shader->GetProjectionUniform(), proj);
	m_Shader->SetMatrix4f(m_Shader->GetUVTransformUniform(), uvRegion);
	GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, 6));
}
void SpriteRenderer::BeginScissor(FloatRect region) {
	glEnable(GL_SCISSOR_TEST);
	glScissor(region.x, region.y, region.w, region.h);
}
void SpriteRenderer::EndScissor() {
	glDisable(GL_SCISSOR_TEST);
}