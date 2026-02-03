#include "Texture.h"
#include "allegro.h"
#include "GLCheck.h"
#include "glad/gl.h"

using namespace RTE;

Texture::Texture(): m_Dimensions() {
	glGenTextures(1, &m_TextureID);
}

Texture::~Texture() {
	glDeleteTextures(1, &m_TextureID);
}

Texture::Texture(GLuint textureId): m_TextureID(textureId), m_Dimensions() {
	glBindTexture(GL_TEXTURE_2D, m_TextureID);
	glGetTexLevelParameterfv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &m_Dimensions.w);
	glGetTexLevelParameterfv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &m_Dimensions.h);
	glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::Texture(FloatRect dimensions, int bitDepth) : m_Dimensions(std::move(dimensions)) {
	glGenTextures(1, &m_TextureID);

	Bind();

	if (bitDepth == 8) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, m_Dimensions.w, m_Dimensions.h, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
		GLint swizzleMask[] = {GL_RED, GL_RED, GL_RED, GL_ONE};
		glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
	} else {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_Dimensions.w, m_Dimensions.h, 0, bitDepth == 32 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, nullptr);

	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::Bind() {
	glBindTexture(GL_TEXTURE_2D, m_TextureID);
}

BitmapTexture::BitmapTexture(std::unique_ptr<BITMAP, BitmapDeleter> bitmap, Filter filtering, WrapType clamp) : Texture(), m_Pixels(std::move(bitmap)), m_HaveAlpha(false) {
	m_Dimensions = FloatRect(0.0f, 0.0f, m_Pixels->w, m_Pixels->h);
	Bind();

	int bitDepth = bitmap_color_depth(m_Pixels.get());
	if (bitDepth == 8) {
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, m_Pixels->w, m_Pixels->h, 0, GL_RED, GL_UNSIGNED_BYTE, m_Pixels->dat);
		GLint swizzleMask[] = {GL_RED, GL_RED, GL_RED, GL_ONE};
		glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
		filtering = Filter::Nearest;
	} else {
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_Pixels->w, m_Pixels->h, 0, bitDepth == 32 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, m_Pixels->dat);
	}

	GLint wrap;
	switch (clamp) {
		case WrapType::ClampToBorder:
			wrap = GL_CLAMP_TO_BORDER;
			break;
		case WrapType::ClampToEdge:
			wrap = GL_CLAMP_TO_EDGE;
			break;
		case RTE::WrapType::Repeat:
			wrap = GL_REPEAT;
			break;
	}

	GLint textureMagFilter;
	GLint textureMinFilter;
	switch (filtering) {
		case Filter::Linear:
			textureMagFilter = GL_LINEAR;
			textureMinFilter = GL_LINEAR;
			break;
		case Filter::LinearMipmap:
			textureMagFilter = GL_LINEAR;
			textureMinFilter = GL_LINEAR_MIPMAP_LINEAR;
			break;
		case Filter::Nearest:
			textureMagFilter = GL_NEAREST;
			textureMinFilter = GL_NEAREST;
			break;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, textureMagFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, textureMinFilter);

	if (filtering == Filter::LinearMipmap) {
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	glBindTexture(GL_TEXTURE_2D, 0);
}

void BitmapTexture::Update(const FloatRect& region) {
	RTEAssert((region.x >= 0) && (region.y>= 0) && (region.x + region.w) <= m_Dimensions.w && (region.y + region.h) <= m_Dimensions.h, "Update area out of BITMAP bounds!");
	Bind();
	int bitDepth = bitmap_color_depth(m_Pixels.get());
	glPixelStorei(GL_UNPACK_ROW_LENGTH, m_Dimensions.w);
	if (bitDepth == 8) {
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexSubImage2D(GL_TEXTURE_2D, 0, region.x, region.y, region.w, region.h, GL_RED, GL_UNSIGNED_BYTE, m_Pixels->line[static_cast<int>(region.y)] + static_cast<int>(region.x));
	} else {
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		glTexSubImage2D(GL_TEXTURE_2D, 0, region.x, region.y, region.w, region.h, GL_RGBA, GL_UNSIGNED_BYTE, m_Pixels->line[static_cast<int>(region.y)] + (static_cast<int>(region.x) * 4));
	}
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

DepthTexture::DepthTexture(FloatRect dimensions) : Texture(std::move(dimensions)) {
	Bind();

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_Dimensions.w, m_Dimensions.h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}
