#pragma once
#include "Singleton.h"
#include "RenderBatch.h"
#include "GLState.h"
#include <memory>

namespace RTE {
	class RenderMan: public Singleton<RenderMan> {
	public:

	private:
		std::unique_ptr<RenderBatch> m_RenderBatch;
		std::unique_ptr<GLState> m_GLState;

	};
}
