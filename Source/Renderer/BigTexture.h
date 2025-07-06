#pragma once
#include <vector>
#include "raylib/raylib.h"
#include "Box.h"
struct BITMAP;
namespace RTE {
	/// BigTexture for big BITMAPS
	struct BigTexture {
		/// Constructs a BigTexture from a bitmap, generating extra textures as needed.
		BigTexture(BITMAP* bitmap);
		
		/// Destructs this BigTexture.
		~BigTexture();
		
		/// Update a region of this BigTexture.
		/// @param region the region to update, must be inside the texture.
		void Update(const Box& region);

		/// @brief Draw this texture (or part of it) to the screen
		/// @param source The source rectangle inside this texture.
		/// @param dest The destination rectangle on the screen.
		void Draw(Rectangle source, Rectangle dest);
		std::vector<Texture2D> m_Textures{}; //!< The tiles of this BigTexture.
		std::vector<unsigned int> m_UploadBuffers{}; //!< Upload buffers to reduce wait time on big uploads.
		std::vector<Box> m_Regions{}; //!< The offset regions of each tile.
		BITMAP* m_Bitmap{nullptr}; //!< The memory BITMAP, not owned.
		int m_Width{0}; //!< The total width of this texture.
		int m_Height{0}; //!< The total height of this texture.
		static int s_MaxGLTextureSize; //!< The maximum allowed size for gpu textures, tiles will be generated at half this to save some space since GL may expand textures to power of 2 squares.
	};
} // namespace RTE