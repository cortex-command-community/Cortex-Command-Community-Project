#pragma once
#include "Singleton.h"
#include "RenderBatch.h"
#include "GLState.h"
#include "DrawCall.h"
#include "Texture.h"
#include <memory>

#define g_RenderMan RenderMan::Instance()

namespace RTE {
	class DrawCall;
	class RenderMan: public Singleton<RenderMan> {
	public:
		RenderMan() = default;
		~RenderMan() { Destroy(); }
		void Initialize();
		void Destroy();
		const RenderBatch* GetActiveBatch() { return m_RenderBatch.get(); }
		float GetCurrentDepth() const { return m_RenderBatch->m_CurrentDepth; }
		void SetCurrentDepth(float depth) { m_RenderBatch->m_CurrentDepth = depth; }
		void SetCurrentShader(const Shader* shader) { m_RenderBatch->m_DefaultShader = shader; }
		std::shared_ptr<DrawCall> BeginDraw();
		GLuint GetShapeTexture() { return m_ShapesTexture->GetTextureId(); }
		GLuint GetPaletteTexture() { return m_PaletteTexture->GetTextureId(); }
		Shader* GetDefaultShader() { return m_DefaultShader.get(); }
		Camera* GetActiveCamera() { return m_CurrentCamera; }
		void BeginFrame(Camera* camera);
		void DrawActiveBatch();
	private:
		std::unique_ptr<RenderBatch> m_RenderBatch{nullptr};
		std::unique_ptr<GLState> m_GLState{nullptr};
		std::shared_ptr<BitmapTexture> m_ShapesTexture{nullptr};
		std::shared_ptr<BitmapTexture> m_PaletteTexture{nullptr};
		std::shared_ptr<Shader> m_DefaultShader{nullptr};
		Camera* m_CurrentCamera{nullptr};
	};
}
