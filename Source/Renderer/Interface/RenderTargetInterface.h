#ifndef _RTERENDERTARGET_
#define _RTERENDERTARGET_
#include "Renderer.h"
#include "Texture2D.h"

namespace RTE {
	class RenderTarget : public Renderer {
	public:
		RenderTarget() = default;
		virtual ~RenderTarget() = default;

		virtual void SetTexture(Texture2D* backbuffer) = 0;

		virtual Texture2D* GetTexture() = 0;

		virtual void SetBlendMode(BlendMode* blendMode) = 0;

		virtual void SetWrapped(bool wrapped) = 0;
		virtual bool GetWrapped() = 0;

		virtual void SetViewport(int x, int y, int w, int h) = 0;
		virtual void ResetViewport() = 0;

		virtual void SetClearColor(glm::vec3 color) = 0;
	};
}

#endif