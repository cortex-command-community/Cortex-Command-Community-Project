#pragma once

/// Header file for the ThreadMan class.
/// Author(s):
/// Inclusions of header files
#include "Singleton.h"
#define g_ThreadMan ThreadMan::Instance()

#include "BS_thread_pool.hpp"

namespace RTE {

	/// The centralized singleton manager of all threads.
	class ThreadMan :
	    public Singleton<ThreadMan> {

		/// Public member variable, method and friend function declarations
	public:
		/// Constructor method used to instantiate a ThreadMan object in system
		/// memory. Create() should be called before using the object.
		ThreadMan();

		/// Makes the TimerMan object ready for use.
		void Initialize(){};

		/// Destructor method used to clean up a ThreadMan object before deletion
		/// from system memory.
		virtual ~ThreadMan();

		/// Makes the ThreadMan object ready for use.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		virtual int Create();

		/// Resets the entire ThreadMan, including its inherited members, to
		/// their default settings or values.
		virtual void Reset() { Clear(); }

		/// Destroys and resets (through Clear()) the ThreadMan object.
		void Destroy();

		BS::thread_pool& GetPriorityThreadPool() { return m_PriorityThreadPool; }

		BS::thread_pool& GetBackgroundThreadPool() { return m_BackgroundThreadPool; }

		/// Protected member variable and method declarations
	protected:
		/// Private member variable and method declarations
	private:
		/// Clears all the member variables of this ThreadMan, effectively
		/// resetting the members of this abstraction level only.
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
