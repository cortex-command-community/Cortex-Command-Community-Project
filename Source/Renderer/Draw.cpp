#include "Draw.h"
#include "GLStateMan.h"
#include "RenderMan.h"
#include "glm/gtx/transform.hpp"

using namespace RTE;
void RTE::DrawTexture(BITMAP* bitmap, int posX, int posY, RLColor tint) {
	// DrawTexture(g_GLStateMan.GetStaticTextureFromBitmap(bitmap), posX, posY, tint);
}

void RTE::DrawTextureV(BITMAP* bitmap, Vector2 pos, RLColor tint) {
	// DrawTextureV(g_GLStateMan.GetStaticTextureFromBitmap(bitmap), pos, tint);
}

void RTE::DrawTextureEx(BITMAP* bitmap, Vector2 pos, float rotation, float scale, RLColor tint) {
	// DrawTextureEx(g_GLStateMan.GetStaticTextureFromBitmap(bitmap), pos, rotation, scale, tint);
}
void RTE::DrawTextureRec(BITMAP* bitmap, Rectangle source, Vector2 pos, RLColor tint) {
	// DrawTextureRec(g_GLStateMan.GetStaticTextureFromBitmap(bitmap), source, pos, tint);
}
void RTE::DrawTexturePro(BITMAP* bitmap, Rectangle source, Rectangle dest, Vector2 origin, float rotation, RLColor tint) {
	// DrawTexturePro(g_GLStateMan.GetStaticTextureFromBitmap(bitmap), source, dest, origin, rotation, tint);
}
namespace RTE {
	namespace Draw {
		std::shared_ptr<DrawCall> DrawTexture(Texture* texture, float posX, float posY, Color tint) {
			return DrawTexture(texture, glm::vec2(posX, posY), tint);
		}

		std::shared_ptr<DrawCall> DrawTexture(Texture* texture, glm::vec2 pos, Color tint) {
			std::shared_ptr<DrawCall> draw = g_RenderMan.BeginDraw();
			draw->m_TextureId = texture->GetTextureId();
			draw->m_Indexed = texture->GetBitDepth() == 8;
			Shape::Shape rect = Shape::Rectangle(FloatRect(pos.x, pos.y, texture->GetDimensions().w, texture->GetDimensions().h), tint);
			draw->m_Vertices = std::move(rect.m_Vertices);
			draw->m_Indices = std::move(rect.m_Indices);
			return draw;
		}

		std::shared_ptr<DrawCall> DrawTexture(Texture* texture, FloatRect dest, Color tint) {
			std::shared_ptr<DrawCall> draw = g_RenderMan.BeginDraw();
			draw->m_TextureId = texture->GetTextureId();
			draw->m_Indexed = texture->GetBitDepth() == 8;
			Shape::Shape rect = Shape::Rectangle(dest, tint);
			draw->m_Vertices = std::move(rect.m_Vertices);
			draw->m_Indices = std::move(rect.m_Indices);
			return draw;
		}

		std::shared_ptr<DrawCall> DrawTexture(Texture* texture, glm::vec2 pos, glm::vec2 origin, float angle, glm::vec2 scale, Color tint) {
			std::shared_ptr<DrawCall> draw = g_RenderMan.BeginDraw();
			draw->m_TextureId = texture->GetTextureId();
			draw->m_Indexed = texture->GetBitDepth() == 8;
			Shape::Shape rect = Shape::Rectangle(FloatRect(0, 0, texture->GetDimensions().w, texture->GetDimensions().h), tint);
			draw->m_Vertices = std::move(rect.m_Vertices);
			draw->m_Indices = std::move(rect.m_Indices);
			glm::mat4 transform = glm::translate(glm::vec3(pos, 0.0f));
			transform = glm::rotate(transform, angle, glm::vec3(0.0f, 0.0f, 1.0f));
			transform = glm::scale(transform, glm::vec3(scale, 1.0f));
			transform = glm::translate(transform, glm::vec3(origin, 0.0f));
			draw->m_UniformValues.emplace_back(std::make_unique<Matrix4fValue>(draw->m_Shader->GetTransformUniform(), std::move(transform)));

			return draw;
		}

		std::shared_ptr<DrawCall> DrawTexture(Texture* texture, const FloatRect& source, const FloatRect& dest, const Color& tint) {
			std::shared_ptr<DrawCall> draw = g_RenderMan.BeginDraw();
			draw->m_TextureId = texture->GetTextureId();
			draw->m_Indexed = texture->GetBitDepth() == 8;
			FloatRect uv = FloatRect(
			    source.x / texture->GetDimensions().w,
			    source.y / texture->GetDimensions().h,
			    source.w / texture->GetDimensions().w,
			    source.h / texture->GetDimensions().h);
			Shape::Shape rect = Shape::Rectangle(dest, uv, tint);
			draw->m_Vertices = std::move(rect.m_Vertices);
			draw->m_Indices = std::move(rect.m_Indices);
			return draw;
		}

	} // namespace Draw
} // namespace RTE
