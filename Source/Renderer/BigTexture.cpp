#include "BigTexture.h"
#include "AllegroTools.h"
#include "Draw.h"
#include "GLStateMan.h"
#include "DebugMan.h"

#include "glad/gl.h"
#include <algorithm>
#include <cmath>
#include "tracy/Tracy.hpp"
#include "tracy/TracyOpenGL.hpp"

using namespace RTE;
int BigTexture::s_MaxGLTextureSize{0};

BigTexture::BigTexture(BITMAP* bitmap) {
	if (!s_MaxGLTextureSize) {
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &s_MaxGLTextureSize);
	}
	int bitsPerPixel = bitmap_color_depth(bitmap);
	int bytesPerPixel = bitsPerPixel / 8;
	m_Bitmap = bitmap;
	m_Width = bitmap->w;
	m_Height = bitmap->h;

	int height = bitmap->h;
	for (int y = 0; y < bitmap->h; y += s_MaxGLTextureSize) {
		int width = bitmap->w;
		for (int x = 0; x < bitmap->w; x += s_MaxGLTextureSize) {
			int regionWidth = std::min(width, s_MaxGLTextureSize);
			int regionHeight = std::min(height, s_MaxGLTextureSize);
			m_Regions.emplace_back(
				Vector(x, y),
				regionWidth,
				regionHeight);
			std::unique_ptr <BITMAP, BitmapDeleter> targetRegionPixels = std::unique_ptr<BITMAP, BitmapDeleter>(create_bitmap_ex(bitmap_color_depth(bitmap), regionWidth, regionHeight));
			m_Textures.emplace_back(std::make_shared<BitmapTexture>(std::move(targetRegionPixels), Filter::Nearest));
			GLuint uploadBuffer;
			glGenBuffers(1, &uploadBuffer);
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, uploadBuffer);
			glBufferData(GL_PIXEL_UNPACK_BUFFER, regionWidth * regionHeight * bytesPerPixel + 1, NULL, GL_STREAM_DRAW);
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
			m_UploadBuffers.emplace_back(uploadBuffer);
			width -= s_MaxGLTextureSize;
		}
		height -= s_MaxGLTextureSize;
	}
}


void BigTexture::Draw(const Box& source, const Box& dest) {
	ZoneScoped;
	TracyGpuZone("BigTexture::Draw");
	float scaleX = dest.m_Width / source.m_Width;
	float scaleY = dest.m_Height / source.m_Height;
	for (int i = 0; i < m_Regions.size(); ++i){
		Box sourceIntersect = source.GetIntersection(m_Regions[i]);
		if (!sourceIntersect.IsEmpty()) {
			Draw::DrawTexture(m_Textures[i].get(), sourceIntersect, Box(dest.m_Corner + Vector(sourceIntersect.m_Corner.m_X * scaleX, sourceIntersect.m_Corner.m_Y * scaleY), sourceIntersect.m_Width * scaleX, sourceIntersect.m_Height * scaleY));
		}
	}
}

void BigTexture::Update(const Box& updateRegion) {
	ZoneScoped;
	TracyGpuZone("BigTexture Upload");
	std::cout << updateRegion.m_Corner << " " << updateRegion.m_Width << " " << updateRegion.m_Height << std::endl;
	int bytesPerPixel = bitmap_color_depth(m_Bitmap) / 8;
	glPixelStorei(GL_UNPACK_ALIGNMENT, bitmap_color_depth(m_Bitmap) == 8 ? 1 : 4);
	for (int i = 0; i < m_Regions.size(); ++i) {
		Box intersect = updateRegion.GetIntersection(m_Regions[i]);
		if (!intersect.IsEmpty()) {
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_UploadBuffers[i]);
			size_t pixelsSize = std::ceil(intersect.m_Width) * std::ceil(intersect.m_Height) * bytesPerPixel;
			unsigned char* pixels = (unsigned char*)glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, (intersect.m_Corner.GetFloorIntY() %s_MaxGLTextureSize) * m_Textures[i]->GetDimensions().w + intersect.m_Corner.GetFloorIntX() % s_MaxGLTextureSize, pixelsSize, GL_MAP_WRITE_BIT|GL_MAP_INVALIDATE_BUFFER_BIT);

			for (size_t y = 0; y < static_cast<int>(std::ceil(intersect.m_Height)); y++) {
				memcpy(
					pixels + y * static_cast<int>(std::ceil(intersect.m_Width)) * bytesPerPixel,
					m_Bitmap->line[y + intersect.m_Corner.GetFloorIntY()] + intersect.m_Corner.GetFloorIntX(),
					std::ceil(intersect.m_Width) * bytesPerPixel
				);
			}
			glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);


			glBindTexture(GL_TEXTURE_2D, m_Textures[i]->GetTextureId());
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			assert(m_Textures[i]->GetDimensions().w >= intersect.m_Width);
			assert(m_Textures[i]->GetDimensions().h >= intersect.m_Height);
			glTexSubImage2D(
				GL_TEXTURE_2D,
				0,
				intersect.m_Corner.GetFloorIntX() % s_MaxGLTextureSize,
				intersect.m_Corner.GetFloorIntY() % s_MaxGLTextureSize,
				std::ceil(intersect.m_Width),
				std::ceil(intersect.m_Height),
				bytesPerPixel == 1 ? GL_RED : GL_RGBA,
				GL_UNSIGNED_BYTE,
				(GLvoid*)((intersect.m_Corner.GetFloorIntY() % s_MaxGLTextureSize) * static_cast<int>(m_Textures[i]->GetDimensions().w) + intersect.m_Corner.GetFloorIntX() % s_MaxGLTextureSize)
			);

			glBindTexture(GL_TEXTURE_2D, 0);
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
		}
	}
}
