#pragma once
#include <unordered_map>
#include <string>
#include "Rectangles.h"
#include "glad/gl.h"
namespace RTE {
	class Shader;
	class RenderTarget {
	public:
		RenderTarget(const FloatRect& size, const FloatRect& defaultViewport, GLuint colorTexture = 0, bool defaultFB0 = false);
		virtual ~RenderTarget();
		void Begin(bool clear = true);
		void End(bool blit = false, RenderTarget* target = nullptr);
		GLuint GetFramebuffer() { return m_FBO; }
		GLuint GetColorTexture() { return m_ColorTexture; }

		const FloatRect& GetSize() { return m_Size; }

	protected:
		FloatRect m_Viewport{};
		FloatRect m_Size{};
		GLuint m_FBO{0};

	private:
		GLuint m_ColorTexture{0};
		bool m_ColorTextureOwned{true};
	};

	class DepthTarget : public RenderTarget {
	public:
		DepthTarget(FloatRect size, FloatRect defaultViewport, GLuint colorTexture = 0, GLuint depthTexture = 0, bool defaultFB0 = false);
		virtual ~DepthTarget();
		void Begin(bool clear = true);
		void End(bool blit = false, DepthTarget* target = nullptr);
		GLuint GetDepthTexture() { return m_DepthTexture; }

	private:
		GLuint m_DepthTexture;
		GLuint m_DepthTextureOwned{true};
	};
} // namespace RTE