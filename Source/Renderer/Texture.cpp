#include "Texture.h"

using namespace RTE;

std::shared_ptr<DrawCall> Texture::Draw(Box source, Box dest, glm::vec2 origin, float angle, Color color) {
	bool flipX = false;
	bool flipY = false;

	if (source.IsEmpty() || dest.IsEmpty()) {
		return nullptr;
	}
}
