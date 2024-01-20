#ifndef _RTERENDERSTATE_
#define _RTERENDERSTATE_

#include "Texture2D.h"
#include "ShaderInterface.h"
#include "glm/glm.hpp"
#include "BlendModeInterface.h"

namespace RTE {
	struct RenderState {
		Texture2D* sprite{nullptr};
		Shader* shader{nullptr};
		glm::vec3 position{0};
		double angle{0};
		BlendMode blendMode;
		std::optional<Shape> shape = std::nullopt;
		std::optional<glm::vec4> color = std::nullopt;
	}
}

#endif