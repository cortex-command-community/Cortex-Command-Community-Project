#pragma once

#include "Constants.h"
#include "Entity.h"
#include "Timer.h"
#include "Vector.h"
#include "Singleton.h"

#define g_MusicMan MusicMan::Instance()

namespace RTE {
	
	class MusicMan : public Singleton<MusicMan> {

#pragma region Creation
		/// Constructor method used to instantiate a MusicMan object in system memory.
		/// Create() should be called before using the object.
		MusicMan();

		/// Makes the MusicMan object ready for use.
		/// @return Whether the audio system was initialized successfully. If not, no audio will be available.
		bool Initialize();
#pragma endregion

#pragma region Destruction
		/// Destructor method used to clean up a MusicMan object before deletion from system memory.
		~MusicMan();

		/// Destroys and resets (through Clear()) the MusicMan object.
		void Destroy();
#pragma endregion

#pragma region Concrete Methods
		/// Updates the state of this MusicMan. Supposed to be done every frame before drawing.
		void Update();
#pragma endregion

		/// Clears all the member variables of this MusicMan, effectively resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		MusicMan(const MusicMan& reference) = delete;
		MusicMan& operator=(const MusicMan& rhs) = delete;
	};
} // namespace RTE