#include "RenderTarget.h"
#include "RTEError.h"
#include "glad/gl.h"
#include "GLCheck.h"
#include "WindowMan.h"
#include "RenderMan.h"
#include "Constants.h"
#include "Texture.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

using namespace RTE;

RenderTarget::RenderTarget(const FloatRect& size, const FloatRect& defaultViewport, int bitDepth, std::shared_ptr<BitmapTexture> colorTexture, bool defaultFB0) {
	m_Size = size;
	m_Viewport = defaultViewport;
	if (!defaultFB0) {
		if (colorTexture) {
			m_Texture = colorTexture;
		} else {
			m_Texture = std::make_shared<Texture>(size, Filter::Linear, WrapType::ClampToEdge, bitDepth);
		}
		m_Depth = std::make_shared<DepthTexture>(size);
		glGenFramebuffers(1, &m_FBO);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBO);

		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_Texture->GetTextureId(), 0);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_Depth->GetTextureId(), 0);

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	} else {
		m_Depth = nullptr;
		m_Texture = nullptr;
	}
}

RenderTarget::RenderTarget(bool defaultFB0) {
	if (!defaultFB0) {
		glGenFramebuffers(1, &m_FBO);
	}
}

RenderTarget::~RenderTarget() {
	if (m_FBO) {
		glDeleteFramebuffers(1, &m_FBO);
	}
}

void RenderTarget::Begin(bool clear, bool drawBatch) {

	if (drawBatch) {
		g_RenderMan.DrawActiveBatch();
	}
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBO);

	glViewport(m_Viewport.x, m_Viewport.y, m_Viewport.w, m_Viewport.h);

	if (clear) {
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	}
}

void RenderTarget::End(bool drawBatch) {
	if (drawBatch) {
		g_RenderMan.DrawActiveBatch();
	}
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void RenderTarget::Bind() {
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBO);
}
