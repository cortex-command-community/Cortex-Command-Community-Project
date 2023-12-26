#ifndef _RTEDRAW_H_
#define _RTEDRAW_H_

#include "Interface/ShapeInterface.h"
#include "Interface/RendererInterface.h"
#include "Interface/Texture2D.h"
#include "glm/glm.hpp"

namespace RTE {
	extern int Draw(Texture2D* texture, Renderer* renderer, float x, float y);
	extern int Draw(Texture2D* texture, Renderer* renderer, float x, float y, glm::vec3 color);
	extern int Draw(Texture2D* texture, Renderer* renderer, float x, float y, float angle);
	extern int Draw(Texture2D* texture, Renderer* renderer, float x, float y, glm::vec2 flip);
	extern int Draw(Texture2D* texture, Renderer* renderer, float x, float y, float angle, glm::vec2 flip);
	extern int Draw(Texture2D* texture, Renderer* renderer, float x, float y, float w, float h);
	extern int Draw(Texture2D* texture, Renderer* renderer, float x, float y, float w, float h, float angle);
	extern int Draw(Texture2D* texture, Renderer* renderer, float x, float y, float w, float h, float angle, glm::vec2 flip);
	extern int Draw(Texture2D* texture, Renderer* renderer, float x, float y, float w, float h, float angle, glm::vec2 flip, glm::vec3 color);

	extern int Draw(Shape* shape, Renderer* renderer, float x, float y);
	extern int Draw(Shape* shape, Renderer* renderer, float x, float y, float angle);
	extern int Draw(Shape* shape, Renderer* renderer, float x, float y, float angle, glm::vec3 color);
}

#endif