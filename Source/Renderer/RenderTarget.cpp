#include "RenderTarget.h"
#include "RTEError.h"
#include "glad/gl.h"
#include "GLCheck.h"

using namespace RTE;

RenderTarget::RenderTarget(const FloatRect& size, const FloatRect& defaultViewport, GLuint colorTexture, bool defaultFB0) {
	m_Size = size;
	m_Viewport = defaultViewport;
	if (colorTexture != 0) {
		m_ColorTexture = colorTexture;
		m_ColorTextureOwned = false;
	} else {
		glGenTextures(1, &m_ColorTexture);
		glBindTexture(GL_TEXTURE_2D, m_ColorTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_Size.w, m_Size.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	if (!defaultFB0) {
		glGenFramebuffers(1, &m_FBO);
		glDisable(GL_DEPTH_TEST);
		glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ColorTexture, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}

RenderTarget::~RenderTarget() {
	if (m_FBO) {
		glDeleteFramebuffers(1, &m_FBO);
	}
	if (m_ColorTextureOwned) {
		glDeleteTextures(1, &m_ColorTexture);
	}
}

void RenderTarget::Begin(bool clear) {
	glDisable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ColorTexture, 0);
	RTEAssert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer inclomplete with: " + std::to_string(glCheckFramebufferStatus(GL_FRAMEBUFFER)));
	glViewport(m_Viewport.x, m_Viewport.y, m_Viewport.w, m_Viewport.h);
	if (clear) {
		glClear(GL_COLOR_BUFFER_BIT);
	}
}

void RenderTarget::End(bool blit, RenderTarget* target) {
	GLint currentFBO;
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &currentFBO);
	RTEAssert(currentFBO == m_FBO, "Different Framebuffer encountered at end draw!");
	if (blit) {
		RTEAssert(target, "No target specified for blit at end draw!");
		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_FBO);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target->m_FBO);
		glBlitFramebuffer(m_Size.x, m_Size.y, m_Size.x + m_Size.w, m_Size.y + m_Size.h,
		                  target->m_Size.x, target->m_Size.y, target->m_Size.x + target->m_Size.w, target->m_Size.y + target->m_Size.h,
		                  GL_COLOR_BUFFER_BIT, GL_NEAREST);
	}
	if (target) {
		target->Begin(false);
	} else {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}

DepthTarget::DepthTarget(FloatRect size, FloatRect defaultViewport, GLuint colorTexture, GLuint depthTexture, bool defaultFB0) :
    RenderTarget(size, defaultViewport, colorTexture, defaultFB0) {
	if (depthTexture != 0) {
		m_DepthTexture = depthTexture;
		m_DepthTextureOwned = false;
	} else {
		glGenTextures(1, &m_DepthTexture);
		glBindTexture(GL_TEXTURE_2D, m_DepthTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, m_Size.w, m_Size.h, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, 0);
		GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

DepthTarget::~DepthTarget() {
	if (m_DepthTextureOwned) {
		glDeleteTextures(1, &m_DepthTexture);
	}
}

void DepthTarget::Begin(bool clear) {
	glEnable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, GetColorTexture(), 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_DepthTexture, 0);
	glViewport(m_Viewport.x, m_Viewport.y, m_Viewport.w, m_Viewport.h);

	if (clear) {
		glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
	}
}

void DepthTarget::End(bool blit, DepthTarget* target) {
	GLint currentFBO;
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &currentFBO);
	RTEAssert(currentFBO == m_FBO, "Different Framebuffer encountered at end draw!");
	if (blit) {
		RTEAssert(target, "No target specified for blit at end draw!");
		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_FBO);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target->m_FBO);
		glBlitFramebuffer(m_Size.x, m_Size.y, m_Size.x + m_Size.w, m_Size.y + m_Size.h,
		                  target->m_Size.x, target->m_Size.y, target->m_Size.x + target->m_Size.w, target->m_Size.y + target->m_Size.h,
		                  GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_LINEAR);
	}
	if (target) {
		target->Begin(false);
	} else {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}