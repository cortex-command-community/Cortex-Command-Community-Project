#pragma once
#include <unordered_map>
#include <string>
#include "Rectangles.h"
#include "glad/gl.h"
#include "raylib/raylib.h"
namespace RTE {
	class Shader;
	class RenderTarget {
	public:
		RenderTarget(const FloatRect& size, const FloatRect& defaultViewport, Texture2D colorTexture = {0, 0, 0, 0, -1}, bool defaultFB0 = false);
		virtual ~RenderTarget();
		void Begin(bool clear = true);
		void End();
		GLuint GetFramebuffer() { return m_FBO; }
		Texture2D GetColorTexture() { return m_Texture; }
		Texture2D GetDepthTexture() { return m_Depth; }

		const FloatRect& GetSize() { return m_Size; }

	protected:
		FloatRect m_Viewport{};
		FloatRect m_Size{};
		GLuint m_FBO{0};

	private:
		Texture2D m_Texture{};
		Texture2D m_Depth{};
		bool m_ColorTextureOwned{true};
	};
} // namespace RTE