#include "Image.h"
#include "glm/glm.hpp"
namespace RTE {
	extern void Draw(Image image, glm::vec2 pos, float angle = 0.0f, glm::vec2 scale = {0.0f, 0.0f});
	extern void Draw(Image image, float x, float y) { Draw(image, {x, y}); }
	extern void Draw(Image image, glm::vec2 pos, float angle, glm::vec2 scale = {0.0f, 0.0f}, glm::vec2 pivot = {0.0f, 0.0f});
	extern void Draw(Image image, glm::vec2 pos, float angel, glm::vec2 rotationPivot, glm::vec2 scale, glm::vec2 scaleCenter);
} // namespace RTE