#include "GLResourceMan.h"

#include "ContentFile.h"
#include "RTEError.h"

#include "GLCheck.h"
#include "allegro.h"
#include <algorithm>

#include "tracy/Tracy.hpp"
#include "tracy/TracyOpenGL.hpp"

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
GLuint GLResourceMan::GetStaticTextureFromFile(const std::string& filename) {
	BITMAP* bitmap = ContentFile(filename.c_str()).GetAsBitmap();
	return GetStaticTextureFromBitmap(bitmap);
}

GLuint GLResourceMan::GetStaticTextureFromBitmap(BITMAP* bitmap) {
	if (!bitmap->extra) {
		m_StaticTextures.emplace_back(new GLBitmapInfo);
		GL_CHECK(glGenTextures(1, &m_StaticTextures.back()->m_Texture));
		bitmap->extra = reinterpret_cast<void*>(m_StaticTextures.back().get());
		GL_CHECK(glPixelStorei(GL_UNPACK_ALIGNMENT, bitmap_color_depth(bitmap) == 8 ? 1 : 4));
		GL_CHECK(glActiveTexture(GL_TEXTURE0));
		GL_CHECK(glBindTexture(GL_TEXTURE_2D, reinterpret_cast<GLBitmapInfo*>(bitmap->extra)->m_Texture));
		GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, bitmap_color_depth(bitmap) == 8 ? GL_R8 : GL_RGBA, bitmap->w, bitmap->h, 0, bitmap_color_depth(bitmap) == 8 ? GL_RED : GL_RGBA, GL_UNSIGNED_BYTE, bitmap->line[0]));
		GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
		//GL_CHECK(glGenerateMipmap(GL_TEXTURE_2D));
		return m_StaticTextures.back()->m_Texture;
	} else {
		return reinterpret_cast<GLBitmapInfo*>(bitmap->extra)->m_Texture;
	}
	return 0;
}

GLuint GLResourceMan::GetDynamicTextureFromBitmap(BITMAP* bitmap, bool updated, const std::vector<IntRect>& updateRegions) {
	ZoneScopedN("Bitmap Upload");
	GLuint texture = GetStaticTextureFromBitmap(bitmap);
	if (updated) {
		TracyGpuZone("Atrocities (Bitmap Upload)");
		glBindTexture(GL_TEXTURE_2D, texture);
		glPixelStorei(GL_UNPACK_ALIGNMENT, bitmap_color_depth(bitmap) == 8 ? 1 : 4);
		if (updateRegions.size() == 0) {
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, bitmap->w, bitmap->h, bitmap_color_depth(bitmap) == 8 ? GL_RED: GL_RGBA, GL_UNSIGNED_BYTE, bitmap->line[0]);
		} else {
			for (auto& region: updateRegions) {
				glTexSubImage2D(GL_TEXTURE_2D, 0, region.m_Left, region.m_Top, region.m_Right - region.m_Left, region.m_Bottom - region.m_Top, bitmap_color_depth(bitmap) == 8 ? GL_RED : GL_RGBA, GL_UNSIGNED_BYTE, bitmap->line[region.m_Top] + (region.m_Left));
			}
		}
	}
	return texture;
}