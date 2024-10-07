#pragma once

#include "Singleton.h"

#include "glad/gl.h"
#include <unordered_map>
#include <vector>
#include <string>
#include <memory>
#include "Shader.h"

namespace RTE {
#define g_GLResourceMan GLResourceMan::Instance()
	enum class ShaderType {
		Fragment,
		Vertex
	};
	struct GLBitmapInfo {
		GLuint m_Texture;
		size_t m_ID;
	};
	class GLResourceMan : public Singleton<GLResourceMan> {
	public:
		GLResourceMan();
		~GLResourceMan();

		void Initialize();
		void Destroy();
		void Clear();

		GLuint CompileShader(const std::string& filename, ShaderType type);

		std::shared_ptr<Shader> MakeShaderProgram(const std::string& name, const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
		GLuint GetStaticTextureFromFile(const std::string& filename);
		GLuint GetStaticTextureFromBitmap(BITMAP* bitmap);

		std::shared_ptr<Shader> GetShaderByName(const std::string& name);

	private:
		std::vector<std::unique_ptr<GLBitmapInfo>> m_StaticTextures;
		std::unordered_map<std::string, std::pair<ShaderType, GLuint>> m_ShaderCache;
		std::unordered_map<std::string, std::shared_ptr<Shader>> m_Shaders;
	};

} // namespace RTE