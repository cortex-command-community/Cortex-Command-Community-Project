#pragma once

/// Description:
/// Author(s):
/// Inclusions of header files
// #include "FrameMan.h"
#include "Timer.h"
#include "Vector.h"
#include "Controller.h"

struct BITMAP;

namespace RTE {
	class SceneObject;
	class ObjectPickerGUI;
	class PieMenuGUI;

	/// Description:
	/// Class history:
	class MultiplayerGameGUI {

		/// Public member variable, method and friend function declarations
	public:
		// Different modes of this editor
		enum GUIMode {
			INACTIVE = 0,
			ACTIVE = 1
		};

		/// Constructor method used to instantiate a MultiplayerGameGUI object in system
		/// memory. Create() should be called before using the object.
		MultiplayerGameGUI();

		/// Destructor method used to clean up a MultiplayerGameGUI object before deletion
		/// from system memory.
		~MultiplayerGameGUI();

		/// Makes the MultiplayerGameGUI object ready for use.
		/// @param pController A poitner to a Controller which will control this Menu. Ownership is
		/// NOT TRANSFERRED!
		/// Whether the editor should have all the features enabled, like load/save
		/// and undo capabilities.
		/// Which module space that this eidtor will be able to pick objects from.
		/// -1 means all modules.
		/// Which Tech module that will be presented as the native one to the player.
		/// The multiplier of all foreign techs' costs.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create(Controller* pController);

		/// Resets the entire MultiplayerGameGUI, including its inherited members, to
		/// their default settings or values.
		void Reset() { Clear(); }

		/// Destroys and resets (through Clear()) the MultiplayerGameGUI object.
		void Destroy();

		/// Updates the state of this Menu each frame
		void Update();

		/// Draws the editor
		/// @param pTargetBitmap The bitmap to draw on.
		/// @param targetPos The absolute position of the target bitmap's upper left corner in the scene. (default: Vector())
		void Draw(BITMAP* pTargetBitmap, const Vector& targetPos = Vector()) const;

		/// Protected member variable and method declarations
	protected:
		// Controller which conrols this menu. Not owned
		Controller* m_pController;

		/// Private member variable and method declarations
	private:
		/// Clears all the member variables of this MultiplayerGameGUI, effectively
		/// resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		MultiplayerGameGUI(const MultiplayerGameGUI& reference);
		MultiplayerGameGUI& operator=(const MultiplayerGameGUI& rhs);
	};

} // namespace RTE
