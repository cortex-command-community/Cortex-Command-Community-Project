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
#include "GLState.h"

#include "raylib/raylib.h"
#define g_GLStateMan GLStateMan::Instance()
namespace RTE {
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

	class GLStateMan : public Singleton<GLStateMan> {
	public:
		GLStateMan();
		~GLStateMan();

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

		const GLState* GetState() { return m_State.get(); }

	private:
		std::vector<std::unique_ptr<GLBitmapInfo>> m_StaticTextures{};
		std::vector<GLuint> m_DynamicBitmapUploadBuffers{};
		std::unordered_map<std::string, std::pair<ShaderType, GLuint>> m_ShaderCache{};
		std::vector<GLuint> m_Shaders{};
		std::unique_ptr<GLState> m_State{nullptr};
	};

} // namespace RTE
