#include "ThreadMan.h"

using namespace RTE;

ThreadMan::ThreadMan() {
	Clear();
	Create();
}

ThreadMan::~ThreadMan() {
	Destroy();
}

void ThreadMan::Clear() {
#ifdef __EMSCRIPTEN__
	// Emscripten: std::thread requires -pthread + SharedArrayBuffer (COOP/COEP headers).
	// For Phase 1, run fully single-threaded with 0-thread pools (tasks run inline).
	m_PriorityThreadPool.reset(0);
	m_BackgroundThreadPool.reset(0);
#else
	m_PriorityThreadPool.reset();
	m_BackgroundThreadPool.reset(std::thread::hardware_concurrency() / 2);
#endif
}

int ThreadMan::Create() {
	return 0;
}

void ThreadMan::Destroy() {
	Clear();
}
