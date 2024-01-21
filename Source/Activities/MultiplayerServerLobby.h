#pragma once

/// Description:
/// Author(s):
/// Inclusions of header files
#include "RTETools.h"
#include "ActivityMan.h"
#include "GameActivity.h"

namespace RTE {
	class GUIScreen;
	class GUIInput;
	class GUIControlManager;
	class GUICollectionBox;
	class GUIComboBox;
	class GUICheckbox;
	class GUITab;
	class GUIListBox;
	class GUITextBox;
	class GUIButton;
	class GUILabel;
	class GUISlider;
	class Entity;
	class Scene;
	class Activity;

	/// Activity for editing scenes.
	/// 9/17/2007 Spliced out and made to derive from EditorActivty
	class MultiplayerServerLobby : public GameActivity {

		/// Public member variable, method and friend function declarations
	public:
		// Concrete allocation and cloning definitions
		EntityAllocation(MultiplayerServerLobby);
		SerializableOverrideMethods;
		ClassInfoGetters;

		/// Constructor method used to instantiate a MultiplayerServerLobby object in system
		/// memory. Create() should be called before using the object.
		MultiplayerServerLobby();

		/// Destructor method used to clean up a MultiplayerServerLobby object before deletion
		/// from system memory.
		~MultiplayerServerLobby() override;

		/// Makes the MultiplayerServerLobby object ready for use.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create() override;

		/// Creates a MultiplayerServerLobby to be identical to another, by deep copy.
		/// @param reference A reference to the MultiplayerServerLobby to deep copy.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create(const MultiplayerServerLobby& reference);

		/// Resets the entire MultiplayerServerLobby, including its inherited members, to their
		/// default settings or values.
		void Reset() override {
			Clear();
			Activity::Reset();
		}

		/// Destroys and resets (through Clear()) the MultiplayerServerLobby object.
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

		void UpdateInput();

		void UpdateActivityBox();

		// void UpdateScenesBox();

		void UpdatePlayersBox(bool newActivity);

		void UpdateGoldSlider(const GameActivity* pSelectedGA);

		void UpdateDifficultySlider();

		void UpdateSkillSlider();

		void GetAllScenesAndActivities();

		bool StartGame();

		int PlayerCount();

		/// Protected member variable and method declarations
	protected:
		// These add on the player and team max counts
		enum PlayerColumns {
			PLAYER_CPU = Players::MaxPlayerCount,
			PLAYERCOLUMNCOUNT
		};

		enum TeamRows {
			TEAM_DISABLED = Teams::MaxTeamCount,
			TEAMROWCOUNT
		};

		// Member variables
		static Entity::ClassInfo m_sClass;

		// The editor GUI
		// MultiplayerServerLobbyGUI *m_pEditorGUI;

		// GUI Screen for use by the GUI dialog boxes. Owned
		GUIScreen* m_pGUIScreen;
		// Input controller for he dialog box gui.  Owned
		GUIInput* m_pGUIInput;
		// The control manager which holds all the gui elements for the dialog boxes.  Owned
		GUIControlManager* m_pGUIController;

		GUICollectionBox* m_pRootBox;
		GUICollectionBox* m_pPlayerSetupBox;

		// Activity selection screen controls
		GUIComboBox* m_pActivitySelect;
		GUIComboBox* m_pSceneSelect;
		GUILabel* m_pDifficultyLabel;
		GUISlider* m_pDifficultySlider;

		GUICollectionBox* m_aapPlayerBoxes[PLAYERCOLUMNCOUNT][TEAMROWCOUNT];
		GUICollectionBox* m_apTeamBoxes[TEAMROWCOUNT];
		GUILabel* m_apTeamNameLabels[TEAMROWCOUNT];
		GUILabel* m_pStartErrorLabel;
		GUILabel* m_pCPULockLabel;
		// Which team the CPU is locked to, if any
		int m_LockedCPUTeam;

		// Tech selection combos
		GUIComboBox* m_apTeamTechSelect[Teams::MaxTeamCount];

		// AI skill selection
		GUISlider* m_apTeamAISkillSlider[Teams::MaxTeamCount];
		GUILabel* m_apTeamAISkillLabel[Teams::MaxTeamCount];

		GUILabel* m_pGoldLabel;
		GUISlider* m_pGoldSlider;
		GUICheckbox* m_pFogOfWarCheckbox;
		GUICheckbox* m_pRequireClearPathToOrbitCheckbox;
		GUICheckbox* m_pDeployUnitsCheckbox;

		const Icon* m_apPlayerIcons[c_MaxClients];

		GUILabel* m_apPlayerNameLabel[c_MaxClients];

		BITMAP* m_pUIDrawBitmap;

		BITMAP* m_pCursor;

		BITMAP* m_pScenePreviewBitmap;
		BITMAP* m_pDefaultPreviewBitmap;

		GUIButton* m_pStartScenarioButton;

		// The scene preset currently selected, NOT OWNED
		const Scene* m_pSelectedScene;

		// The current set of Scenes being displayed - not owned, nor are the scenes
		std::list<Scene*>* m_pScenes;
		// The map of Activity:ies, and the Scene:s compatible with each, neither of which are owned here
		std::map<Activity*, std::list<Scene*>> m_Activities;

		/// Private member variable and method declarations
	private:
		/// Clears all the member variables of this Activity, effectively
		/// resetting the members of this abstraction level only.
		void Clear();
	};

} // namespace RTE
