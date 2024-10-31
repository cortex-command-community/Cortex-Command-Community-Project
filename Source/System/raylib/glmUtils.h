#pragma once
#include "glm/mat4x4.hpp"
#include "raylib.h"

extern Matrix toRlMatrix(glm::mat4 matrix) { return reinterpret_cast<Matrix>(matrix); }