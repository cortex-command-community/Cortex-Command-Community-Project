#include "BigTexture.h"
#include "glad/gl.h"
#include <algorithm>
#include <cmath>
#include "Draw.h"
#include "GLResourceMan.h"
#include "tracy/Tracy.hpp"
#include "tracy/TracyOpenGL.hpp"

using namespace RTE;
int BigTexture::s_MaxGLTextureSize{0};

BigTexture::BigTexture(BITMAP* bitmap) {
	if (!s_MaxGLTextureSize) {
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &s_MaxGLTextureSize);
		s_MaxGLTextureSize /= 2;
	}
	int bitsPerPixel = bitmap_color_depth(bitmap);
	int bytesPerPixel = bitsPerPixel / 8;
	m_Bitmap = bitmap;
	GLBitmapInfo* bitmapExtra = g_GLResourceMan.MakeBitmapInfo();
	bitmap->extra = reinterpret_cast<void*>(bitmapExtra);
	PixelFormat format = bitsPerPixel == 8 ? PIXELFORMAT_UNCOMPRESSED_GRAYSCALE : PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
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
			m_Textures.emplace_back(
			    rlLoadTexture(nullptr, regionWidth, regionHeight, PIXELFORMAT_UNCOMPRESSED_GRAYSCALE, 1),
			    regionWidth,
			    regionHeight,
			    1,
			    format);
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

BigTexture::~BigTexture() {
	for (Texture2D& texture: m_Textures) {
		glDeleteTextures(1, &texture.id);
	}
}

void BigTexture::Draw(Rectangle source, Rectangle dest) {
	ZoneScoped;
	TracyGpuZone("BigTexture::Draw");
	float scaleX = dest.width / source.width;
	float scaleY = dest.height / source.height;
	Box sourceBox(Vector(source.x, source.y), source.width, source.height);
	for (int i = 0; i < m_Regions.size(); ++i){
		Box sourceIntersect = sourceBox.GetIntersection(m_Regions[i]);
		if (!sourceIntersect.IsEmpty()) {
#ifdef DEBUG_BUILD
			DrawRectangleLines(dest.x + sourceIntersect.m_Corner.m_X * scaleX,
				dest.y + sourceIntersect.m_Corner.m_Y * scaleY,
				sourceIntersect.m_Width * scaleX,
				sourceIntersect.m_Height * scaleY,
				{5, 0, 0, 255}
			);
			DrawRectangleLines(
				dest.x + sourceIntersect.m_Corner.m_X * scaleX + 10,
				dest.y + sourceIntersect.m_Corner.m_Y * scaleY + 10,
				sourceIntersect.m_Width * scaleX - 20,
				sourceIntersect.m_Height * scaleY - 20,
				{5, 0, 0, 255}
			);
#endif
			DrawTexturePro(
			    m_Textures[i],
			    sourceIntersect,
			    {dest.x + sourceIntersect.m_Corner.m_X * scaleX,
			     dest.y + sourceIntersect.m_Corner.m_Y * scaleY,
			     sourceIntersect.m_Width * scaleX,
			     sourceIntersect.m_Height * scaleY},
			    {0.0f, 0.0f},
			    0.0f,
			    {255, 255, 255, 255}
			);
		}
	}
}

void BigTexture::Update(const Box& updateRegion) {
	ZoneScoped;
	TracyGpuZone("BigTexture Upload");
	if (!m_Bitmap->extra) {
		m_Bitmap->extra = reinterpret_cast<void*>(g_GLResourceMan.MakeBitmapInfo());
	}
	int bytesPerPixel = bitmap_color_depth(m_Bitmap) / 8;
	glPixelStorei(GL_UNPACK_ALIGNMENT, bitmap_color_depth(m_Bitmap) == 8 ? 1 : 4);
	for (int i = 0; i < m_Regions.size(); ++i) {
		Box intersect = updateRegion.GetIntersection(m_Regions[i]);
		if (!intersect.IsEmpty()) {
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_UploadBuffers[i]);
			size_t pixelsSize = std::ceil(intersect.m_Width) * std::ceil(intersect.m_Height) * bytesPerPixel;
			unsigned char* pixels = (unsigned char*)glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, (intersect.m_Corner.GetFloorIntY() %s_MaxGLTextureSize) * m_Textures[i].width + intersect.m_Corner.GetFloorIntX() % s_MaxGLTextureSize, pixelsSize, GL_MAP_WRITE_BIT|GL_MAP_INVALIDATE_BUFFER_BIT);
			
			for (size_t y = 0; y < static_cast<int>(std::ceil(intersect.m_Height)); y++) {
				memcpy(
					pixels + y * static_cast<int>(std::ceil(intersect.m_Width)) * bytesPerPixel,
					m_Bitmap->line[y + intersect.m_Corner.GetFloorIntY()] + intersect.m_Corner.GetFloorIntX(),
					std::ceil(intersect.m_Width) * bytesPerPixel
				);
			}
			glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);


			glBindTexture(GL_TEXTURE_2D, m_Textures[i].id);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			glTexSubImage2D(
				GL_TEXTURE_2D,
				0, 
				intersect.m_Corner.GetFloorIntX() % s_MaxGLTextureSize, 
				intersect.m_Corner.GetFloorIntY() % s_MaxGLTextureSize, 
				std::ceil(intersect.m_Width), 
				std::ceil(intersect.m_Height), 
				bytesPerPixel == 1 ? GL_RED : GL_RGBA, 
				GL_UNSIGNED_BYTE,
				reinterpret_cast<void*>((intersect.m_Corner.GetFloorIntY() % s_MaxGLTextureSize) * m_Textures[i].width + intersect.m_Corner.GetFloorIntX() % s_MaxGLTextureSize)
			);

			glBindTexture(GL_TEXTURE_2D, 0);
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
		}
	}
}
