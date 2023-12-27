#ifndef _RTERENDERER_H_
#define _RTERENDERER_H_
#include "RenderStateInterface.h"

namespace RTE {
	class Renderer {
	public:
		Renderer();
		virtual ~Renderer();

		virtual void Clear() = 0;
		virtual void Draw(RenderState&& state) = 0;
		virtual void Render() = 0;
		virtual int Draw(Texture2D *texture, float x, float y) = 0;
		virtual int Draw(Texture2D *texture, float x, float y, glm::vec3 color) = 0;
		virtual int Draw(Texture2D *texture, float x, float y, float angle) = 0;
		virtual int Draw(Texture2D *texture, float x, float y, glm::vec2 flip) = 0;
		virtual int Draw(Texture2D *texture, float x, float y, float angle, glm::vec2 flip) = 0;
		virtual int Draw(Texture2D *texture, float x, float y, float w, float h) = 0;
		virtual int Draw(Texture2D *texture, float x, float y, float w, float h, float angle) = 0;
		virtual int Draw(Texture2D *texture, float x, float y, float w, float h, float angle, glm::vec2 flip) = 0;
		virtual int Draw(Texture2D *texture, float x, float y, float w, float h, float angle, glm::vec2 flip, glm::vec3 color) = 0;
		virtual int Draw(Shape *shape, float x, float y) = 0;
		virtual int Draw(Shape *shape, float x, float y, float angle) = 0;
		virtual int Draw(Shape *shape, float x, float y, float angle, glm::vec3 color) = 0;
	};
} // namespace RTE

#endif