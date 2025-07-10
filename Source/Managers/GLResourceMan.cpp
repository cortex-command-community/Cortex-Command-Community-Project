#include "GLResourceMan.h"

#include "ContentFile.h"
#include "RTEError.h"

#include "GLCheck.h"
#include "allegro.h"
#include <algorithm>

#include "tracy/Tracy.hpp"
#include "tracy/TracyOpenGL.hpp"

#include "raylib/raylib.h"
#include "raylib/rlgl.h"

using namespace RTE;

GLResourceMan::GLResourceMan() = default;

GLResourceMan::~GLResourceMan() = default;

void GLResourceMan::Clear() {
	for (auto prog: m_Shaders) {
		GL_CHECK(glDeleteProgram(prog));
	}
	for (auto& texture: m_StaticTextures) {
		glDeleteTextures(1, &texture->m_Texture);
	}
	for (auto& shaderData: m_ShaderCache) {
		glDeleteShader(shaderData.second.second);
	}
}

void GLResourceMan::Destroy() { Clear(); }
void GLResourceMan::Initialize() { Clear(); }

GLuint GLResourceMan::CompileShader(const std::string& filename, ShaderType type) {
	return 0;
}

GLuint GLResourceMan::MakeGLProgram() {
	return glCreateProgram();
}

// std::shared_ptr<Shader> GLResourceMan::MakeShaderProgram(const std::string& name, const std::string& vertexShaderPath, const std::string& fragmentShaderPath) {
// 	if (m_Shaders.find(name) != m_Shaders.end()) {
// 		RTEAbort("Attempted to reregister Shader: " + name);
// 	} else {
// 		m_Shaders[name] = std::make_shared<Shader>(vertexShaderPath, fragmentShaderPath);
// 		return m_Shaders[name];
// 	}
// 	return nullptr;
// }

GLBitmapInfo* GLResourceMan::GetBitmapInfo(BITMAP* bitmap) {
	return reinterpret_cast<GLBitmapInfo*>(bitmap->extra);
}

Texture2D GLResourceMan::GetStaticTextureFromFile(const std::string& filename) {
	BITMAP* bitmap = ContentFile(filename.c_str()).GetAsBitmap();
	return GetStaticTextureFromBitmap(bitmap);
}

GLBitmapInfo* GLResourceMan::MakeBitmapInfo() {
	m_StaticTextures.emplace_back(new GLBitmapInfo);
	m_StaticTextures.back()->m_ID = m_StaticTextures.size();
	return m_StaticTextures.back().get();
}

Texture2D GLResourceMan::GetStaticTextureFromBitmap(BITMAP* bitmap) {
	if (!bitmap->extra) {
		m_StaticTextures.emplace_back(new GLBitmapInfo);
		m_StaticTextures.back()->m_ID = m_StaticTextures.size();
		bitmap->extra = reinterpret_cast<void*>(m_StaticTextures.back().get());
		GL_CHECK(glPixelStorei(GL_UNPACK_ALIGNMENT, bitmap_color_depth(bitmap) == 8 ? 1 : 4));
		m_StaticTextures.back()->m_Texture = rlLoadTexture(bitmap->line[0], bitmap->w, bitmap->h, bitmap_color_depth(bitmap) == 8 ? PIXELFORMAT_UNCOMPRESSED_GRAYSCALE : PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, 1);

		return {
		    .id = m_StaticTextures.back()->m_Texture,
		    .width = bitmap->w,
		    .height = bitmap->h,
		    .mipmaps = 0,
		    .format = bitmap_color_depth(bitmap) == 8 ? PIXELFORMAT_UNCOMPRESSED_GRAYSCALE : PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
	} else {
		return {
		    .id = GetBitmapInfo(bitmap)->m_Texture,
		    .width = bitmap->w,
		    .height = bitmap->h,
		    .mipmaps = 0,
		    .format = 0};
	}
	return {0, 0, 0, 0, -1};
}


GLuint GLResourceMan::GetDynamicUploadBuffer(BITMAP* bitmap) {
	if (!bitmap->extra) {
		GetStaticTextureFromBitmap(bitmap);
	}
	GLBitmapInfo* info = GetBitmapInfo(bitmap);

	if (!info->m_UpdateBuffer) {
		GLuint updateBuffer;
		glGenBuffers(1, &updateBuffer);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, updateBuffer);
		glBufferData(GL_PIXEL_UNPACK_BUFFER, bitmap->w * bitmap->h * bitmap_color_depth(bitmap) / 8, nullptr, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
		info->m_UpdateBuffer = updateBuffer;
	}
	return info->m_UpdateBuffer;
}

GLuint GLResourceMan::UpdateDynamicBitmap(BITMAP* bitmap, bool updated, const std::vector<Box>& updateRegions) {
	ZoneScopedN("Bitmap Upload");
	GLuint texture = GetStaticTextureFromBitmap(bitmap).id;
	if (updated) {
		glBindTexture(GL_TEXTURE_2D, texture);
		glPixelStorei(GL_UNPACK_ALIGNMENT, bitmap_color_depth(bitmap) == 8 ? 1 : 4);
		if (updateRegions.size() == 0) {
			GL_CHECK(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, bitmap->w, bitmap->h, bitmap_color_depth(bitmap) == 8 ? GL_RED : GL_RGBA, GL_UNSIGNED_BYTE, bitmap->line[0]));
		} else {
			int bytesPerPixel = bitmap_color_depth(bitmap) / 8;
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, GetDynamicUploadBuffer(bitmap));
			std::vector<size_t> offsets = {0};
			for (size_t i = 0; i < updateRegions.size(); ++i) {
				std::vector<unsigned char> pixels(updateRegions[i].m_Width * updateRegions[i].m_Height * bytesPerPixel);
				for (size_t y = 0; y < updateRegions[i].m_Height; y++) {
					memcpy(
						pixels.data() + y * static_cast<int>(updateRegions[i].m_Width) * bytesPerPixel,
						bitmap->line[y + updateRegions[i].m_Corner.GetFloorIntY()] + updateRegions[i].m_Corner.GetFloorIntX(),
						updateRegions[i].m_Width * bytesPerPixel);
				}
				glBufferSubData(GL_PIXEL_UNPACK_BUFFER, offsets[i], updateRegions[i].m_Width * updateRegions[i].m_Height * bytesPerPixel, pixels.data());
				offsets.emplace_back(updateRegions[i].m_Width * updateRegions[i].m_Height * bytesPerPixel);
			}
			for (size_t i = 0; i < updateRegions.size(); ++i) {
				GL_CHECK(glTexSubImage2D(
					GL_TEXTURE_2D,
					0,
					updateRegions[i].m_Corner.GetFloorIntX(),
					updateRegions[i].m_Corner.GetFloorIntY(),
					updateRegions[i].m_Width,
					updateRegions[i].m_Height,
					bytesPerPixel == 1 ? GL_RED : GL_RGBA,
					GL_UNSIGNED_BYTE,
					(void*)offsets[i]
				));
			}
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
		}
	}
	return texture;
}

void GLResourceMan::DestroyBitmapInfo(BITMAP* bitmap) {
	GLBitmapInfo* info = GetBitmapInfo(bitmap);
	if (info) {
		rlUnloadTexture(info->m_Texture);
	}
}
