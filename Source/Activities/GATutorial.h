#pragma once

/// Header file for the ActivityMan class.
/// @author Daniel Tabar
/// data@datarealms.com
/// http://www.datarealms.com
/// Inclusions of header files
#include "GameActivity.h"
#include "Box.h"

namespace RTE {

	class Actor;

	/// Tutorial mission with lots of special triggering logic.
	class GATutorial : public GameActivity {

		/// Public member variable, method and friend function declarations
	public:
		// Concrete allocation and cloning definitions
		EntityAllocation(GATutorial);
		SerializableOverrideMethods;
		ClassInfoGetters;

		/// Constructor method used to instantiate a GATutorial object in system
		/// memory. Create() should be called before using the object.
		GATutorial();

		/// Destructor method used to clean up a GATutorial object before deletion
		/// from system memory.
		~GATutorial() override;

		/// Makes the GATutorial object ready for use.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create() override;

		/// Creates a GATutorial to be identical to another, by deep copy.
		/// @param reference A reference to the GATutorial to deep copy.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create(const GATutorial& reference);

		/// Resets the entire GATutorial, including its inherited members, to their
		/// default settings or values.
		void Reset() override {
			Clear();
			Activity::Reset();
		}

		/// Destroys and resets (through Clear()) the GATutorial object.
		/// @param notInherited Whether to only destroy the members defined in this derived class, or (default: false)
		/// to destroy all inherited members also.
		void Destroy(bool notInherited = false) override;

		/// Tells if a particular Scene supports this specific Activity on it.
		/// Usually that means certain Area:s need to be defined in the Scene.
		/// @param pScene The Scene to check if it supports this Activiy. Ownership IS NOT TRANSFERRED!
		/// @param teams How many teams we're checking for. Some scenes may support and activity (default: -1)
		/// but only for a limited number of teams. If -1, not applicable.
		/// @return Whether the Scene has the right stuff.
		bool SceneIsCompatible(Scene* pScene, int teams = -1) override;

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
		/// @param pTargetBitmap A pointer to a BITMAP to draw on.
		/// @param targetPos The absolute position of the target bitmap's upper left corner in the scene. (default: Vector())
		void Draw(BITMAP* pTargetBitmap, const Vector& targetPos = Vector()) override;

		/// Protected member variable and method declarations
	protected:
		/// Goes through all Actor:s currently in the MovableMan and sets each
		/// one not controlled by a player to be AI controlled and AIMode setting
		/// based on team and CPU team.
		void InitAIs() override;

		/// Sets up or resets the Tutorial Areas to show the current control
		/// mappings etc.
		void SetupAreas();

		enum TutorialArea {
			BRAINCHAMBER = 0,
			BODYSTORAGE,
			SHAFT,
			OBSTACLECOURSE,
			FIRINGRANGE,
			ROOFTOP,
			ROOFEAST,
			AREACOUNT
		};

		enum TutorialRoom {
			ROOM0 = 0,
			ROOM1,
			ROOM2,
			ROOM3,
			ROOMCOUNT
		};

		enum LitState {
			UNLIT = 0,
			LIT,
			LITSTATECOUNT
		};

		enum ScreenState {
			SCREENOFF = 0,
			STATICLITTLE,
			STATICLARGE,
			SHOWINGSTEP,
			SREENSTATECOUNT
		};

		enum FightStage {
			NOFIGHT = 0,
			DEFENDING,
			ATTACK,
			FIGHTSTAGECOUNT
		};

		struct TutStep {
			// Text of this step
			std::string m_Text;
			// Duration of the whole step
			int m_Duration;
			// BITMAPs not owned here
			std::vector<BITMAP*> m_pScreens;
			// The duration of one frame
			int m_FrameDuration;

			TutStep(std::string text, int stepDuration, std::string screensPath = "", int frameCount = 1, int frameDuration = 250);
		};

		// Member variables
		static Entity::ClassInfo m_sClass;

		// The player which is actually playing the tut
		int m_TutorialPlayer;
		// The areas that trigger specific sets of steps to be shown
		Box m_TriggerBoxes[AREACOUNT];
		// Positions of the screens for each area
		Vector m_ScreenPositions[AREACOUNT];
		// The current state of the all the different areas' screens
		ScreenState m_ScreenStates[AREACOUNT];
		// Offsets of the center of the text line from the screen position
		Vector m_TextOffsets[AREACOUNT];
		// Screen bitmaps common to all areas.. off, static etc
		BITMAP* m_apCommonScreens[STATICLARGE + 1];
		// The steps themselves; cycles through for each area
		std::vector<TutStep> m_TutAreaSteps[AREACOUNT];
		// Positions of the numbered room signs
		Vector m_RoomSignPositions[ROOMCOUNT];
		// Room sign bitmaps, unlit and lit
		BITMAP* m_aapRoomSigns[ROOMCOUNT][LITSTATECOUNT];
		// The timer which keeps track of how long each area has been showing
		Timer m_AreaTimer;
		// The timer which keeps track of how long each step should be shown
		Timer m_StepTimer;
		// Which are the player-controlled actor is within
		TutorialArea m_CurrentArea;
		// Which are the player-controlled actor was in last
		TutorialArea m_PrevArea;
		// If teh screen has just changed and needs to be redrawn
		bool m_ScreenChange;
		// Which tutorial step of the current area currently being played back
		unsigned int m_CurrentStep;
		// Which frame of the current step's animation are we on?
		int m_CurrentFrame;
		// Current room
		TutorialRoom m_CurrentRoom;
		// Trigger box for the subsequent fight
		Box m_FightTriggers[FIGHTSTAGECOUNT];
		unsigned int m_EnemyCount; //!< The amount of enemy actors at the start of the activity.
		// The current fight stage
		FightStage m_CurrentFightStage;
		// The CPU opponent brain; not owned!
		Actor* m_pCPUBrain;

		/// Private member variable and method declarations
	private:
		/// Clears all the member variables of this Activity, effectively
		/// resetting the members of this abstraction level only.
		void Clear();
	};

} // namespace RTE
