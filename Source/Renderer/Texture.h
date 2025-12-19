#pragma once
#include "glm/fwd.hpp"
#include "Box.h"
#include "Color.h"

namespace RTE {
	class Texture {
	public:
		void Draw(Box source, Box dest, glm::vec2 origin, float angle, Color tint);
	};
}
