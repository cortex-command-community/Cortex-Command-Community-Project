#pragma once

/// Header file for the ThreadMan class.
/// Author(s):
#include "Singleton.h"

#include "Activity.h"
#include "SLTerrain.h"

#include <atomic>

#define g_ThreadMan ThreadMan::Instance()

#include "BS_thread_pool.hpp"

namespace RTE {
	struct RenderableGameState {
		RenderableGameState();
		~RenderableGameState();

		SLTerrain* m_Terrain = nullptr;
		// std::unique_ptr<SLTerrain> m_Terrain = nullptr;
		Activity* m_Activity = nullptr;
		// std::unique_ptr<Activity> m_Activity = std::make_unique<Activity>();
		std::vector<std::function<void(float)>> m_RenderQueue;
	};

	/// The manager for any long-running background threads (i.e the simulation thread), and managing communication between threads.
	class ThreadMan : public Singleton<ThreadMan> {
	public:
		/// Constructor method used to instantiate a ThreadMan object in system
		/// memory. Create() should be called before using the object.
		ThreadMan();

		/// Destructor method used to clean up a ThreadMan object before deletion
		/// from system memory.
		virtual ~ThreadMan();

		/// Makes the TimerMan object ready for use.
		virtual int Initialize();

		void Destroy();

		/// Updates the state of this ThreadMan. Supposed to be done every render frame.
		void Update();

		/// Notifies us that the next sim update will be drawn, and transfers the
		/// required information from the current simulation update to a drawable format.
		void TransferSimStateToRenderer();

		/// Tells us whether or not there's a new sim frame waiting to be drawn.
		bool NewSimFrameIsReady() { return m_NewSimFrame; }

		/// Get a game state we can safely read from the render thread.
		const RenderableGameState& GetDrawableGameState() const { return *m_GameStateDrawable; }

		std::vector<std::function<void(float)>>& GetSimRenderQueue() { return m_SimRenderQueue; }

		/// Queue a function that will be ran in the sim thread
		void QueueInSimulationThread(std::function<void(void)> funcToRun);

		/// Run all queued functions for the sim thread
		void RunSimulationThreadFunctions();

		BITMAP* GetRenderTarget() const { return m_RenderTarget; }
		void SetRenderTarget(BITMAP* newRenderTarget) { m_RenderTarget = newRenderTarget; }

		Vector GetRenderOffset() const { return m_RenderOffset; }
		void SetRenderOffset(Vector newRenderOffset) { m_RenderOffset = newRenderOffset; }

		virtual const std::string& GetClassName() const { return m_ClassName; }

		BS::thread_pool& GetPriorityThreadPool() { return m_PriorityThreadPool; }

		BS::thread_pool& GetBackgroundThreadPool() { return m_BackgroundThreadPool; }

	protected:
		// Member variables
		static const std::string m_ClassName;

	private:
		// Because of the hacky way we've converted a single-threaded immediate-mode CPU rendered game into a multi-threaded queued renderer
		// (and hopefully one day soon, GPU rendered!)
		// We need to store a render target and offset that we apply to the draws here.
		BITMAP* m_RenderTarget;
		Vector m_RenderOffset;

		std::unique_ptr<RenderableGameState> m_GameStateModifiable; //!< Current game state game state that sim can update (owned)
		std::unique_ptr<RenderableGameState> m_GameStateDrawable; //!< Stable game state that we are drawing (owned)
		std::mutex m_GameStateCopyMutex; //!< Mutex to ensure we can't swap our rendering game state while it's being copied to.

		// We store this here, as well as in the game state copy mutex.
		// The sim thread uses this queue, and clears it at the start of each new sim thread
		// It then copies it into the RenderableGameState once it's finished with it.
		std::vector<std::function<void(float)>> m_SimRenderQueue;

		std::atomic<bool> m_NewSimFrame; //!< Whether we have a new sim frame ready to draw.

		std::vector<std::function<void(void)>> m_SimFunctions;
		std::mutex m_SimFunctionMutex; //!< Mutex to ensure that functions are added to the sim thread safely

		void Clear();

		// Disallow the use of some implicit methods.
		ThreadMan(const ThreadMan& reference);
		ThreadMan& operator=(const ThreadMan& rhs);

		// For tasks that we want to be performed ASAP, i.e needs to be complete this frame at some point
		BS::thread_pool m_PriorityThreadPool;

		// For background tasks that we can just let happen whenever over multiple frames
		BS::thread_pool m_BackgroundThreadPool;
	};

} // namespace RTE
