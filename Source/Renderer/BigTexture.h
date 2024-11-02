#pragma once
#include <vector>
#include "raylib/raylib.h"
#include "Box.h"
struct BITMAP;
namespace RTE {
	struct BigTexture {
		BigTexture(BITMAP* bitmap);
		~BigTexture();
		void Update(const Box& region);

		void Draw(Rectangle source, Rectangle dest);
		std::vector<Texture2D> m_Textures{};
		std::vector<Box> m_Regions{};
		BITMAP* m_Bitmap{nullptr};
		int m_Width{0};
		int m_Height{0};
		static int s_MaxGLTextureSize;
	};
} // namespace RTE