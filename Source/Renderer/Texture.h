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
		ClampToBorder,
		ClampToEdge,
		Repeat
	};
	class BitmapTexture : public Texture {
	public:
		/// Constructs a texture from an allegro BITMAP.
		/// @param bitmap Unique pointer to the BITMAP representing this texture's pixels.
		/// @param filtering A Filter mode which is applied to the GL texture. Always set to Nearest for indexed images.
		/// @param clamp The texture wrapping mode.
		BitmapTexture(std::unique_ptr<BITMAP, BitmapDeleter> bitmap, Filter filtering = Filter::Linear, WrapType clamp = WrapType::ClampToEdge);

		/// Destructor.
		~BitmapTexture() = default;

		/// Returns the pixels of this BitmapTexture.
		/// If the bitmap is modified, call Update to update the texture on the GPU as well.
		/// @return (Non owning) Bitmap to the pixels.
		BITMAP* GetBitmap() const { return m_Pixels.get(); }

		/// Update the pixels on GPU.
		void Update(const FloatRect& region);

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
