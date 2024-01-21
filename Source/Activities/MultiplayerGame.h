#pragma once

/// Description:
/// Author(s):
/// Inclusions of header files
#include "RTETools.h"
#include "ActivityMan.h"

namespace RTE {

	class MultiplayerGameGUI;
	class GUIScreen;
	class GUIInput;
	class GUIControlManager;
	class GUICollectionBox;
	class GUITab;
	class GUIListBox;
	class GUITextBox;
	class GUIButton;
	class GUILabel;
	class GUIComboBox;

	/// Activity for editing scenes.
	/// 9/17/2007 Spliced out and made to derive from EditorActivty
	class MultiplayerGame : public Activity {

		/// Public member variable, method and friend function declarations
	public:
		// Concrete allocation and cloning definitions
		EntityAllocation(MultiplayerGame);
		SerializableOverrideMethods;
		ClassInfoGetters;

		enum MultiplayerGameMode {
			SETUP,
			CONNECTION,
			GAMEPLAY
		};

		/// Constructor method used to instantiate a MultiplayerGame object in system
		/// memory. Create() should be called before using the object.
		MultiplayerGame();

		/// Destructor method used to clean up a MultiplayerGame object before deletion
		/// from system memory.
		~MultiplayerGame() override;

		/// Makes the MultiplayerGame object ready for use.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create() override;

		/// Creates a MultiplayerGame to be identical to another, by deep copy.
		/// @param reference A reference to the MultiplayerGame to deep copy.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create(const MultiplayerGame& reference);

		/// Resets the entire MultiplayerGame, including its inherited members, to their
		/// default settings or values.
		void Reset() override {
			Clear();
			Activity::Reset();
		}

		/// Destroys and resets (through Clear()) the MultiplayerGame object.
		/// @param notInherited Whether to only destroy the members defined in this derived class, or (default: false)
		/// to destroy all inherited members also.
		void Destroy(bool notInherited = false) override;

		/// Officially starts the game accroding to parameters previously set.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Start() override;

		/// Pauses and unpauses the game.
		/// @param pause Whether to pause the game or not. (default: true)
		void SetPaused(bool pause = true) override;

		/// Forces the current game's end.
		void End() override;

		/// Updates the state of this ActivityMan. Supposed to be done every frame
		/// before drawing.
		void Update() override;

		/// Draws the currently active GUI of a screen to a BITMAP of choice.
		/// @param pTargetBitmap A pointer to a screen-sized BITMAP to draw on.
		/// @param targetPos The absolute position of the target bitmap's upper left corner in the scene. (default: Vector())
		/// @param which Which screen's GUI to draw onto the bitmap. (default: 0)
		void DrawGUI(BITMAP* pTargetBitmap, const Vector& targetPos = Vector(), int which = 0) override;

		/// Draws this ActivityMan's current graphical representation to a
		/// BITMAP of choice. This includes all game-related graphics.
		/// @param pTargetBitmap A pointer to a BITMAP to draw on. OWNERSHIP IS NOT TRANSFERRED!
		/// @param targetPos The absolute position of the target bitmap's upper left corner in the scene. (default: Vector())
		void Draw(BITMAP* pTargetBitmap, const Vector& targetPos = Vector()) override;

		/// Protected member variable and method declarations
	protected:
		// Member variables
		static Entity::ClassInfo m_sClass;

		// The editor GUI
		// MultiplayerGameGUI *m_pEditorGUI;

		GUIButton* m_BackToMainButton; //!< Button to return to main menu.

		//
		GUITextBox* m_pServerNameTextBox;

		GUITextBox* m_pPlayerNameTextBox;

		GUIButton* m_pConnectButton;

		GUITextBox* m_pNATServiceServerNameTextBox;

		GUITextBox* m_pNATServerNameTextBox;

		GUITextBox* m_pNATServerPasswordTextBox;

		GUIButton* m_pConnectNATButton;

		GUILabel* m_pStatusLabel;

		// GUI Screen for use by the GUI dialog boxes. Owned
		GUIScreen* m_pGUIScreen;
		// Input controller for he dialog box gui.  Owned
		GUIInput* m_pGUIInput;
		// The control manager which holds all the gui elements for the dialog boxes.  Owned
		GUIControlManager* m_pGUIController;
		// Current state of the activity
		MultiplayerGameMode m_Mode;

		// Timer to watchdog connection states and abort connection if nothing happened
		Timer m_ConnectionWaitTimer;

		// Last music file being player, used to recover playback state after pause
		std::string m_LastMusic;

		// Position of music being played, used to recover playback state after pause
		double m_LastMusicPos;

		/// Private member variable and method declarations
	private:
		/// Clears all the member variables of this Activity, effectively
		/// resetting the members of this abstraction level only.
		void Clear();
	};

} // namespace RTE
