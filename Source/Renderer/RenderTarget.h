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

		/// @brief RenderTarget constructor
		/// @param size Size of render target in pixels
		/// @param defaultViewport The default viewport to set up at Begin().
		/// @param bitDepth The color depth of this target, either 8 or 32 (default 32)
		/// @param colorTexture Optional color texture, if specified, bitDepth will be ignored.
		/// @param defaultFB0 Make this target for the window backbuffer.
		RenderTarget(const FloatRect& size, const FloatRect& defaultViewport, int bitDepth = 32, Texture2D colorTexture = {0, 0, 0, 0, -1}, bool defaultFB0 = false);
		/// Destructor.
		virtual ~RenderTarget();
		
		/// Enables this RenderTarget for drawing and sets up projection matrix. Draws current batch, resets draw depth and model matrix.
		/// @param clear Whether to clear the target.
		void Begin(bool clear = true, float zoom = 1.0f);
		
		/// Disables this RenderTarget and flushes the active batch.
		void End();

		/// @brief Getter for the FBO
		/// @return The FBO.
		GLuint GetFramebuffer() { return m_FBO; }
		/// Getter for the color buffer
		const Texture2D& GetColorTexture() { return m_Texture; }
		/// Getter for the depth buffer.
		const Texture2D& GetDepthTexture() { return m_Depth; }

		/// Getter for the size of this target.
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
