#pragma once
#include "glm/fwd.hpp"
#include "Rectangles.h"
#include "Color.h"
#include "glad/gl.h"
#include <memory>
#include "DrawCall.h"
#include "AllegroTools.h"

namespace RTE {
	/// Abstraction for rectangular textures.
	class Texture {
	public:
		Texture(std::unique_ptr<BITMAP, BitmapDeleter> bitmap);
		BITMAP* GetBitmap() const { return m_Pixels.get(); }
		GLuint GetTextureId() const { return m_TextureID; }
		const FloatRect& GetDimensions() const { return m_Dimensions; }

	private:
		GLuint m_TextureID{0};
		FloatRect m_Dimensions;
		std::unique_ptr<BITMAP, BitmapDeleter> m_Pixels;
		bool m_HaveAlpha;

	public:
		//explicit operator BITMAP*() { return m_Pixels.get(); }
	};
} // namespace RTE
