#include "Vertex.h"
#include "RenderMan.h"

#include <utility>

using namespace RTE;

Vertex::Vertex(glm::vec2 pos, glm::u8vec4 color) : Vertex(std::move(pos), glm::vec2(0.0f), g_RenderMan.GetActiveBatch()->m_CurrentZ, std::move(color)) {}

Vertex::Vertex(glm::vec2 pos, glm::vec2 uv, glm::u8vec4 color) : Vertex(std::move(pos), std::move(uv), g_RenderMan.GetActiveBatch()->m_CurrentZ, std::move(color)) {}

Vertex::Vertex(glm::vec2 pos, glm::vec2 uv, float layerDepth, glm::u8vec4 color) :
	m_Pos(std::move(pos), layerDepth + g_RenderMan.GetActiveBatch()->m_CurrentDepth), m_TextureUV(std::move(uv)), m_Color(std::move(color)) {}
