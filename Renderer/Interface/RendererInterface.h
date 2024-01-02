#ifndef _RTERENDERER_H_
#define _RTERENDERER_H_
#include "RenderStateInterface.h"

namespace RTE {
	class Renderer {
	public:
		Renderer();
		virtual ~Renderer();

		virtual void Clear() = 0;
		virtual void Draw(RenderState &&state) = 0;
		virtual void Render() = 0;

	};
} // namespace RTE

#endif