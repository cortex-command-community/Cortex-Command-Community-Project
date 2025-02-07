#pragma once

#include "Singleton.h"

#include "glad/gl.h"
#include <unordered_map>
#include <vector>
#include <string>
#include <memory>
#include <optional>
#include "Shader.h"
#include "Box.h"

#include "raylib/raylib.h"
namespace RTE {
#define g_GLResourceMan GLResourceMan::Instance()
	enum class ShaderType {
		Fragment,
		Vertex
	};
	struct GLBitmapInfo {
		GLuint m_Texture{0};
		GLuint* m_Textures{nullptr};
		size_t m_ID{0};
		GLuint m_UpdateBuffer{0};
	};
	class GLResourceMan : public Singleton<GLResourceMan> {
	public:
		GLResourceMan();
		~GLResourceMan();

		void Initialize();
		void Destroy();
		void Clear();

		GLuint CompileShader(const std::string& filename, ShaderType type);
		GLuint MakeGLProgram();

		// std::shared_ptr<Shader> MakeShaderProgram(const std::string& name, const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
		// std::shared_ptr<Shader> GetShaderByName(const std::string& name);
		Texture2D GetStaticTextureFromFile(const std::string& filename);
		Texture2D GetStaticTextureFromBitmap(BITMAP* bitmap);

		GLBitmapInfo* GetBitmapInfo(BITMAP* bitmap);
		GLuint GetDynamicUploadBuffer(BITMAP* bitmap);
		GLuint UpdateDynamicBitmap(BITMAP* bitmap, bool updated, const std::vector<Box>& = {});

		void DestroyBitmapInfo(BITMAP* bitmap);

		GLBitmapInfo* MakeBitmapInfo();


	private:
		std::vector<std::unique_ptr<GLBitmapInfo>> m_StaticTextures;
		std::vector<GLuint> m_DynamicBitmapUploadBuffers;
		std::unordered_map<std::string, std::pair<ShaderType, GLuint>> m_ShaderCache;
		std::vector<GLuint> m_Shaders;
	};

} // namespace RTE