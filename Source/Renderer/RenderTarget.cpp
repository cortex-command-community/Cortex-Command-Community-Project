#include "RenderTarget.h"
#include "RTEError.h"
#include "glad/gl.h"
#include "GLCheck.h"
#include "raylib/rlgl.h"
#include "WindowMan.h"
#include "Constants.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

using namespace RTE;

RenderTarget::RenderTarget(const FloatRect& size, const FloatRect& defaultViewport, int bitDepth, Texture2D colorTexture, bool defaultFB0) {
	m_Size = size;
	m_Viewport = defaultViewport;
	if (!defaultFB0) {
		if (colorTexture.id != 0) {
			m_Texture = std::move(colorTexture);
			m_ColorTextureOwned = false;
		} else {
			m_Texture = {
			    .id = rlLoadTexture(nullptr, size.w, size.h, bitDepth == 8 ? PIXELFORMAT_UNCOMPRESSED_GRAYSCALE : PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, 1),
			    .width = static_cast<int>(size.w),
			    .height = static_cast<int>(size.h),
			    .mipmaps = 0,
			    .format = bitDepth == 8 ? PIXELFORMAT_UNCOMPRESSED_GRAYSCALE : PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
			};
		}
		m_Depth = {
		    .id = rlLoadTextureDepth(size.w, size.h, false),
		    .width = static_cast<int>(size.w),
		    .height = static_cast<int>(size.h),
		    .mipmaps = 0,
		    .format = GL_DEPTH_COMPONENT,
		};
		m_FBO = rlLoadFramebuffer();
		rlEnableFramebuffer(m_FBO);
		rlFramebufferAttach(m_FBO, m_Texture.id, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_TEXTURE2D, 0);
		rlFramebufferAttach(m_FBO, m_Depth.id, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_TEXTURE2D, 0);
		rlDisableFramebuffer();
	} else {
		m_ColorTextureOwned = false;
		m_Depth.id = 0;
		m_Texture.id = 0;
	}
}

RenderTarget::~RenderTarget() {
	if (m_FBO) {
		rlUnloadFramebuffer(m_FBO);
	}
	if (m_ColorTextureOwned) {
		rlUnloadTexture(m_Texture.id);
	}
	rlUnloadTexture(m_Depth.id);
}

void RenderTarget::Begin(bool clear, float zoom) {
	rlDrawRenderBatchActive();
	rlResetDrawDepth();
	rlEnableFramebuffer(m_FBO);

	glViewport(m_Viewport.x, m_Viewport.y, m_Viewport.w, m_Viewport.h);

	rlSetFramebufferWidth(m_Size.w);
	rlSetFramebufferHeight(m_Size.h);

	rlMatrixMode(RL_PROJECTION);
	rlLoadIdentity();
	rlOrtho(0.0f, m_Size.w, m_Size.h, 0.0f, c_NearDepth, c_FarDepth);
	rlMatrixMode(RL_MODELVIEW);
	rlLoadIdentity();
	rlTranslatef(m_Size.w / 2, m_Size.h / 2, 0.0f);
	rlScalef(zoom, zoom, 1.0f);
	rlTranslatef(-m_Size.w / 2, -m_Size.h / 2, 0.0f);

	if (clear) {
		rlClearScreenBuffers();
	}
}

void RenderTarget::End() {
	rlDrawRenderBatchActive();

	rlDisableFramebuffer();
	glViewport(0, 0, g_WindowMan.GetResX(), g_WindowMan.GetResY());
	rlResetDrawDepth();
}
