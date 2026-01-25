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
		RenderMan();
		~RenderMan() { Destroy(); }
		void Initialize();
		void Destroy();
		const RenderBatch* GetActiveBatch() { return m_RenderBatch.get(); }
		std::shared_ptr<DrawCall> BeginDraw();
		GLint GetShapeTexture() { return m_ShapesTexture->GetTextureId(); }
	private:
		std::unique_ptr<RenderBatch> m_RenderBatch;
		std::unique_ptr<GLState> m_GLState;
		std::shared_ptr<Texture> m_ShapesTexture{};
	};
}
