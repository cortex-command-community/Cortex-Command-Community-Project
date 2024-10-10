#pragma once
#include "allegro.h"
#include "glm/glm.hpp"
#include "Vector.h"
#include "Shader.h"
#include "Rectangles.h"
namespace RTE {
	class SpriteRenderer {
	public:
		SpriteRenderer(const Shader* shader, const FloatRect& size);
		~SpriteRenderer();
		const FloatRect& GetSize() {return m_Size;};
		void SetSize(const FloatRect& size) { m_Size = size; }
		void Draw(BITMAP* image, glm::vec2 pos, float angle = 0.0f, glm::vec2 scale = {1.0f, 1.0f});

		void Draw(BITMAP* image, float x, float y) { Draw(image, {x, y}); }
		void DrawPivot(BITMAP* image, glm::vec2 pos, float angle, glm::vec2 scale = {1.0f, 1.0f}, glm::vec2 pivot = {0.0f, 0.0f});
		void Draw(BITMAP* image, FloatRect fromRegion, glm::vec2 pos);
		void Draw(BITMAP* image, FloatRect fromRegion, glm::vec2 toSize, glm::vec2 pos, float angle = 0.0f, glm::vec2 pivot = {0.0f, 0.0f}, glm::vec2 scale = {1.0f, 1.0f});
		void BeginScissor(FloatRect region);
		void EndScissor();
	private:
		const Shader* m_Shader;
		GLuint m_VAO;
		FloatRect m_Size;
		void InitGLPointers();
	};
} // namespace RTE