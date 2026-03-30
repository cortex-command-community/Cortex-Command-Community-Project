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
	// Emscripten without pthreads: std::thread is unavailable (Asyncify and
	// pthreads are mutually exclusive).  BS::thread_pool::reset(0) still
	// creates 1+ thread via determine_thread_count(), which would fail.
	// Leave pools at default-constructed state (no threads, no task queue).
	// All submit()+wait() call sites must be guarded to run tasks inline.
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
