#pragma once
#include "glm/fwd.hpp"
#include "Rectangles.h"
#include "Color.h"
#include "glad/gl.h"
#include <memory>
#include "DrawCall.h"
#include "AllegroTools.h"

extern "C" {
typedef struct BITMAP BITMAP;
}

namespace RTE {
	/// Abstraction for rectangular textures.
	class Texture {
	public:
		Texture();
		Texture(GLuint textureId);
		Texture(FloatRect dimensions, int bitDepth = 32);
		virtual ~Texture();
		void Bind();
		GLuint GetTextureId() const { return m_TextureID; }
		const FloatRect& GetDimensions() const { return m_Dimensions; }

	protected:
		GLuint m_TextureID{0};
		FloatRect m_Dimensions;
	};

	enum class Filter {
		Linear,
		LinearMipmap,
		Nearest
	};
	enum class WrapType {
		ClampToEdge,
		Repeat
	};
	class BitmapTexture : public Texture {
	public:
		BitmapTexture(std::unique_ptr<BITMAP, BitmapDeleter> bitmap, Filter filtering = Filter::Linear, WrapType clamp = WrapType::ClampToEdge);
		~BitmapTexture() = default;
		BITMAP* GetBitmap() const { return m_Pixels.get(); }

	private:
		std::unique_ptr<BITMAP, BitmapDeleter> m_Pixels;
		bool m_HaveAlpha;

	public:
		// explicit operator BITMAP*() { return m_Pixels.get(); }
	};

	class DepthTexture : public Texture {
	public:
		DepthTexture(FloatRect dimensions);
		~DepthTexture() = default;
	};
} // namespace RTE
