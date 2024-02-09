#pragma once

/// Header file for the ActivityMan class.
/// @author Daniel Tabar
/// data@datarealms.com
/// http://www.datarealms.com
/// Inclusions of header files
#include "GUISound.h"
#include "RTETools.h"
#include "ActivityMan.h"
#include "Scene.h"
#include "Actor.h"

#include <array>
#include <memory>
namespace RTE {

#define OBJARROWFRAMECOUNT 4
#define LZCURSORFRAMECOUNT 4

	class Actor;
	class ACraft;
	class PieMenu;
	class InventoryMenuGUI;
	class BuyMenuGUI;
	class SceneEditorGUI;
	class GUIBanner;
	class Loadout;

	/// Base class for all GameActivity:s, including game modes and editors.
	class GameActivity : public Activity {

		friend struct ActivityLuaBindings;

		// Keeps track of everything about a delivery in transit after purchase has been made with the menu
		struct Delivery {
			// OWNED by this until the delivery is made!
			ACraft* pCraft;
			// Which player ordered this delivery
			int orderedByPlayer;
			// Where to land
			Vector landingZone;
			// How much this delivery was offset upwards for multi-ordering, stored to help with delivery icons. If 0, this was presumably not a multi-order.
			float multiOrderYOffset;
			// How long left until entry, in ms
			long delay;
			// Times how long we've been in transit
			Timer timer;
		};

		/// Public member variable, method and friend function declarations
	public:
		SerializableOverrideMethods;
		ClassInfoGetters;

		enum ObjectiveArrowDir {
			ARROWDOWN = 0,
			ARROWLEFT,
			ARROWRIGHT,
			ARROWUP
		};

		enum BannerColor {
			RED = 0,
			YELLOW
		};

		/// Constructor method used to instantiate a GameActivity object in system
		/// memory. Create() should be called before using the object.
		GameActivity();

		/// Destructor method used to clean up a GameActivity object before deletion
		/// from system memory.
		~GameActivity() override;

		/// Makes the GameActivity object ready for use.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create() override;

		/// Creates a GameActivity to be identical to another, by deep copy.
		/// @param reference A reference to the GameActivity to deep copy.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create(const GameActivity& reference);

		/// Resets the entire GameActivity, including its inherited members, to their
		/// default settings or values.
		void Reset() override {
			Clear();
			Activity::Reset();
		}

		/// Destroys and resets (through Clear()) the GameActivity object.
		/// @param notInherited Whether to only destroy the members defined in this derived class, or (default: false)
		/// to destroy all inherited members also.
		void Destroy(bool notInherited = false) override;

		/// Gets the current CPU-assisted team, if any (NoTeam) - LEGACY function
		/// @return The current setting. NoTeam is no team is assisted.
		int GetCPUTeam() const { return m_CPUTeam; }

		/// Sets the current CPU-assisted team, if any (NoTeam) - LEGACY function
		/// @param team The new setting. NoTeam is no team is assisted. (default: Activity::NoTeam)
		void SetCPUTeam(int team = Activity::NoTeam);

		/// Sets the observation sceneman scroll targets, for when the game is
		/// over or a player is in observation mode
		/// @param newTarget The new absolute position to observe.
		/// @param player Which player to set it for. (default: 0)
		void SetObservationTarget(const Vector& newTarget, int player = 0) {
			if (player >= Players::PlayerOne && player < Players::MaxPlayerCount)
				m_ObservationTarget[player] = newTarget;
		}

		/// Sets the player death sceneman scroll targets, for when a player-
		/// controlled actor dies and the view should go to his last position
		/// @param newTarget The new absolute position to set as death view.
		/// @param player Which player to set it for. (default: 0)
		void SetDeathViewTarget(const Vector& newTarget, int player = 0) {
			if (player >= Players::PlayerOne && player < Players::MaxPlayerCount)
				m_DeathViewTarget[player] = newTarget;
		}

		/// Sets the he last selected landing zone.
		/// @param newZone The new absolute position to set as the last selected landing zone.
		/// @param player Which player to set it for. (default: 0)
		void SetLandingZone(const Vector& newZone, int player = 0) {
			if (player >= Players::PlayerOne && player < Players::MaxPlayerCount)
				m_LandingZone[player] = newZone;
		}

		/// Gets the he last selected landing zone.
		/// @param player Which player to get it for. (default: 0)
		/// @return The new absolute position to set as the last selected landing zone.
		Vector GetLandingZone(int player = 0) {
			if (player >= Players::PlayerOne && player < Players::MaxPlayerCount)
				return m_LandingZone[player];
			else
				return Vector();
		}

		/// Sets the actor selection cursor position.
		/// @param newPos The new absolute position to put the cursor at.
		/// @param player Which player to set it for. (default: 0)
		void SetActorSelectCursor(const Vector& newPos, int player = 0) {
			if (player >= Players::PlayerOne && player < Players::MaxPlayerCount)
				m_ActorCursor[player] = newPos;
		}

		/// Gets the an in-game GUI Object for a specific player.
		/// @param which Which player to get the GUI for. (default: 0)
		/// @return A pointer to a BuyMenuGUI. Ownership is NOT transferred!
		BuyMenuGUI* GetBuyGUI(unsigned int which = 0) const { return m_pBuyGUI[which]; }

		/// Checks if the in-game GUI Object is visible for a specific player.
		/// @param which Which player to check the GUI for. -1 will check all players.
		/// @return Whether or not the BuyMenuGUI is visible for input player(s).
		bool IsBuyGUIVisible(int which = 0) const;

		/// Gets the an in-game editor GUI Object for a specific player.
		/// @param which Which player to get the GUI for. (default: 0)
		/// @return A pointer to a SceneEditorGUI. Ownership is NOT transferred!
		SceneEditorGUI* GetEditorGUI(unsigned int which = 0) const { return m_pEditorGUI[which]; }

		/// Locks a player controlled actor to a specific controller mode.
		/// Locking the actor will disable player input, including switching actors.
		/// @param player Which player to lock the actor for.
		/// @param lock Whether to lock or unlock the actor. (Default: true)
		/// @param lockToMode Which controller mode to lock the actor to. (Default: `CIM_AI`)
		/// @return Whether the (un)lock was performed.
		bool LockControlledActor(Players player, bool lock = true, Controller::InputMode lockToMode = Controller::InputMode::CIM_AI);

		/// Forces the this to focus player control to a specific Actor for a
		/// specific team. OWNERSHIP IS NOT TRANSFERRED!
		/// @param pActor Which Actor to switch focus to. The team of this Actor will be set
		/// once it is passed in. Ownership IS NOT TRANSFERRED! The Actor should
		/// be added to MovableMan already.
		/// @return Whether the focus switch was successful or not.
		bool SwitchToActor(Actor* pActor, int player = 0, int team = 0) override;

		/// Forces the this to focus player control to the next Actor of a
		/// specific team, other than the current one focused on.
		/// @param player Which team to switch to next actor on.
		/// @param team An actor pointer to skip in the sequence.
		void SwitchToNextActor(int player, int team, Actor* pSkip = 0) override;

		/// Forces this to focus player control to the previous Actor of a
		/// specific team, other than the current one focused on.
		/// @param player Which team to switch to next actor on.
		/// @param team An actor pointer to skip in the sequence.
		void SwitchToPrevActor(int player, int team, Actor* pSkip = 0) override;

		/// Sets which team is the winner, when the game is over.
		/// @param winnerTeam The team number of the winning team. 0 is team #1. Negative number
		/// means the game isn't over yet.
		void SetWinnerTeam(int winnerTeam) { m_WinnerTeam = winnerTeam; }

		/// Indicates which team is the winner, when the game is over.
		/// @return The team number of the winning team. 0 is team #1. Negative number
		/// means the game isn't over yet.
		int GetWinnerTeam() const { return m_WinnerTeam; }

		/// Gets access to the huge banner of any player that can display
		/// messages which can not be missed or ignored.
		/// @param whichColor Which color banner to get - see the GameActivity::BannerColor enum. (default: YELLOW)
		/// @param player Which player's banner to get. (default: Players::PlayerOne)
		/// @return A pointer to the GUIBanner object that we can
		GUIBanner* GetBanner(int whichColor = YELLOW, int player = Players::PlayerOne) { return whichColor == YELLOW ? m_pBannerYellow[player] : m_pBannerRed[player]; }

		/// Sets the Area within which a team can land things.
		/// @param team The number of the team we're setting for.
		/// @param newArea The Area we're setting to limit their landings within.
		void SetLZArea(int team, const Scene::Area& newArea) {
			m_LandingZoneArea[team].Reset();
			m_LandingZoneArea[team].Create(newArea);
		}

		/// Gets the Area within which a team can land things. OWNERSHIP IS NOT TRANSFERRED!
		/// @param team The number of the team we're setting for.
		/// @return The Area we're using to limit their landings within. OWNERSHIP IS NOT TRANSFERRED!
		const Scene::Area& GetLZArea(int team) const { return m_LandingZoneArea[team]; }

		/// Sets the width of the landing zone box that follows around a player's
		/// brain.
		/// @param player The number of the in-game player we're setting for.
		/// @param width The width of the box, in pixels. 0 means disabled.
		void SetBrainLZWidth(int player, int width) { m_BrainLZWidth[player] = width; }

		/// Gets the width of the landing zone box that follows around a player's
		/// brain.
		/// @param player The number of the player we're getting for.
		/// @return The width in pixels of the landing zone.
		int GetBrainLZWidth(int player) const { return m_BrainLZWidth[player]; }

		/// Created an objective point for one of the teams to show until cleared.
		/// @param description The team number of the team to give objective. 0 is team #1.
		/// @param objPos The very short description of what the objective is (three short words max)
		/// @param whichTeam The absolute scene coordiante position of the objective. (default: Teams::TeamOne)
		/// @param arrowDir The desired direction of the arrow when the point is on screen. (default: ARROWDOWN)
		void AddObjectivePoint(std::string description, Vector objPos, int whichTeam = Teams::TeamOne, ObjectiveArrowDir arrowDir = ARROWDOWN);

		/// Sorts all objective points according to their positions on the Y axis.
		void YSortObjectivePoints();

		/// Clears all objective points previously added, for both teams.
		void ClearObjectivePoints() { m_Objectives.clear(); }

		/// Adds somehting to the purchase list that will override what is set
		/// in the buy guy next time CreateDelivery is called.
		/// @param pPurchase The SceneObject preset to add to the override purchase list. OWNERSHIP IS NOT TRANSFERRED!
		/// @param player Which player's list to add an override purchase item to.
		/// @return The new total value of what's in the override purchase list.
		int AddOverridePurchase(const SceneObject* pPurchase, int player);

		/// First clears and then adds all the stuff in a Loadout to the override
		/// purchase list.
		/// @param pLoadout The Loadout preset to set the override purchase list to reflect. OWNERSHIP IS NOT TRANSFERRED!
		/// @param player The player we're talking about.
		/// @return The new total value of what's in the override purchase list.
		int SetOverridePurchaseList(const Loadout* pLoadout, int player);

		/// First clears and then adds all the stuff in a Loadout to the override
		/// purchase list.
		/// @param loadoutName The name of the Loadout preset to set the override purchase list to
		/// represent.
		/// @return The new total value of what's in the override purchase list.
		int SetOverridePurchaseList(std::string loadoutName, int player);

		/// Clears all items from a specific player's override purchase list.
		/// @param m_PurchaseOverride[player].clear( Which player's override purchase list to clear.
		void ClearOverridePurchase(int player) { m_PurchaseOverride[player].clear(); }

		/// Takes the current order out of a player's buy GUI, creates a Delivery
		/// based off it, and stuffs it into that player's delivery queue.
		/// @param player Which player to create the delivery for. Cargo AI mode and waypoint.
		/// @return Success or not.
		bool CreateDelivery(int player, int mode, Vector& waypoint) { return CreateDelivery(player, mode, waypoint, NULL); };

		/// Takes the current order out of a player's buy GUI, creates a Delivery
		/// based off it, and stuffs it into that player's delivery queue.
		/// @param player Which player to create the delivery for. Cargo AI mode and TargetMO.
		/// @return Success or not.
		bool CreateDelivery(int player, int mode, Actor* pTargetMO) {
			Vector point(-1, -1);
			return CreateDelivery(player, mode, point, pTargetMO);
		};

		/// Takes the current order out of a player's buy GUI, creates a Delivery
		/// based off it, and stuffs it into that player's delivery queue.
		/// @param player Which player to create the delivery for and Cargo AI mode.
		/// @return Success or not.
		bool CreateDelivery(int player, int mode) {
			Vector point(-1, -1);
			return CreateDelivery(player, mode, point, NULL);
		};

		/// Takes the current order out of a player's buy GUI, creates a Delivery
		/// based off it, and stuffs it into that player's delivery queue.
		/// @param player Which player to create the delivery for.
		/// @return Success or not.
		bool CreateDelivery(int player) {
			Vector point(-1, -1);
			return CreateDelivery(player, Actor::AIMODE_SENTRY, point, NULL);
		};

		/// Shows how many deliveries this team has pending.
		/// @param m_Deliveries[team].size( Which team to check the delivery count for.
		/// @return The number of deliveries this team has coming.
		int GetDeliveryCount(int team) { return m_Deliveries[team].size(); }

		/// Precalculates the player-to-screen index map, counts the number of
		/// active players etc.
		void SetupPlayers() override;

		/// Officially starts the game accroding to parameters previously set.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Start() override;

		/// Pauses and unpauses the game.
		/// @param pause Whether to pause the game or not. (default: true)
		void SetPaused(bool pause = true) override;

		/// Forces the current game's end.
		void End() override;

		/// This is a special update step for when any player is still editing the
		/// scene.
		void UpdateEditing();

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

		/// Returns the name of the tech module selected for this team during scenario setup
		/// @param team Team to return tech module for
		/// @return Tech module name, for example Dummy.rte, or empty string if there is no team
		std::string GetTeamTech(int team) const { return (team >= Teams::TeamOne && team < Teams::MaxTeamCount) ? m_TeamTech[team] : ""; }

		/// Sets tech module name for specified team. Module must set must be loaded.
		/// @param team Team to set module, module name, for example Dummy.rte
		void SetTeamTech(int team, std::string tech);

		/// Indicates whether a specific team is assigned a CPU player in the current game.
		/// @param team Which team index to check.
		/// @return Whether the team is assigned a CPU player in the current activity.
		bool TeamIsCPU(int team) const { return (team >= Teams::TeamOne && team < Teams::MaxTeamCount) ? m_TeamIsCPU[team] : false; }

		/// Returns active CPU team count.
		/// @return Returns active CPU team count.
		int GetActiveCPUTeamCount() const;

		/// Returns active human team count.
		/// @return Returns active human team count.
		int GetActiveHumanTeamCount() const;

		/// Changes how much starting gold was selected in scenario setup dialog. 20000 - infinite amount.
		/// @param amount Starting gold amount
		void SetStartingGold(int amount) { m_StartingGold = amount; }

		/// Returns how much starting gold was selected in scenario setup dialog. 20000 - infinite amount.
		/// @return How much starting gold must be given to human players.
		int GetStartingGold() { return m_StartingGold; }

		/// Changes whether fog of war must be enabled for this activity or not.
		/// Never hides or reveals anything, just changes internal flag.
		/// @param enable New fog of war state. true = enabled.
		/// Return value:	None.
		void SetFogOfWarEnabled(bool enable) { m_FogOfWarEnabled = enable; }

		/// Returns whether fog of war must be enabled for this activity or not.
		/// Call it to determine whether you should call MakeAllUnseen or not at the start of activity.
		/// @return Whether Fog of war flag was checked during scenario setup dialog.
		bool GetFogOfWarEnabled() { return m_FogOfWarEnabled; }

		/// Tells whether player activity requires a cleat path to orbit to place brain
		/// Return value:	Whether we need a clear path to orbit to place brains.
		bool GetRequireClearPathToOrbit() const { return m_RequireClearPathToOrbit; }

		/// Tells whether player activity requires a cleat path to orbit to place brain
		/// @param newvalue Whether we need a clear path to orbit to place brains.
		/// Return value:	None.
		void SetRequireClearPathToOrbit(bool newvalue) { m_RequireClearPathToOrbit = newvalue; }

		int GetDefaultFogOfWar() const { return m_DefaultFogOfWar; }

		int GetDefaultRequireClearPathToOrbit() const { return m_DefaultRequireClearPathToOrbit; }

		int GetDefaultDeployUnits() const { return m_DefaultDeployUnits; }

		int GetDefaultGoldCakeDifficulty() const { return m_DefaultGoldCakeDifficulty; }

		int GetDefaultGoldEasyDifficulty() const { return m_DefaultGoldEasyDifficulty; }

		int GetDefaultGoldMediumDifficulty() const { return m_DefaultGoldMediumDifficulty; }

		int GetDefaultGoldHardDifficulty() const { return m_DefaultGoldHardDifficulty; }

		int GetDefaultGoldNutsDifficulty() const { return m_DefaultGoldNutsDifficulty; }

		/// Gets the default gold for max difficulty.
		/// @return The default gold for max difficulty.
		int GetDefaultGoldMaxDifficulty() const { return m_DefaultGoldMaxDifficulty; }

		bool GetFogOfWarSwitchEnabled() const { return m_FogOfWarSwitchEnabled; }

		bool GetDeployUnitsSwitchEnabled() const { return m_DeployUnitsSwitchEnabled; }

		bool GetGoldSwitchEnabled() const { return m_GoldSwitchEnabled; }

		bool GetRequireClearPathToOrbitSwitchEnabled() const { return m_RequireClearPathToOrbitSwitchEnabled; }

		/// Returns CrabToHumanSpawnRatio for specified module
		/// @return Crab-To-Human spawn ratio value set for specified module, 0.25 is default.
		float GetCrabToHumanSpawnRatio(int moduleid);

		/// Returns current delivery delay
		/// @return Returns current delivery delay
		long GetDeliveryDelay() const { return m_DeliveryDelay; }

		/// Sets delivery delay
		/// @param newDeliveryDelay New delivery delay value in ms
		void SetDeliveryDelay(long newDeliveryDelay) { m_DeliveryDelay = newDeliveryDelay > 1 ? newDeliveryDelay : 1; }

		/// Returns whether buy menu is enabled in this activity.
		/// @param True if buy menu enabled false otherwise
		bool GetBuyMenuEnabled() const { return m_BuyMenuEnabled; }

		/// Sets whether buy menu is enabled in this activity
		/// @param newValue True to enable buy menu, false otherwise
		void SetBuyMenuEnabled(bool newValue) { m_BuyMenuEnabled = newValue; }

		/// Returns network player name
		/// @param player Player
		/// @return Network player name
		std::string& GetNetworkPlayerName(int player);

		/// Sets network player name
		/// @param player Player number, player name
		void SetNetworkPlayerName(int player, std::string name);

		/// Protected member variable and method declarations
	protected:
		/// Takes the current order out of a player's buy GUI, creates a Delivery
		/// based off it, and stuffs it into that player's delivery queue.
		/// @param player Which player to create the delivery for. Cargo AI mode waypoint or TargetMO.
		/// @return Success or not.
		bool CreateDelivery(int player, int mode, Vector& waypoint, Actor* pTargetMO);

		/// A struct to keep all data about a mission objective.
		struct ObjectivePoint {
			ObjectivePoint() {
				m_Description.clear();
				m_ScenePos.Reset();
				m_Team = Teams::NoTeam;
				m_ArrowDir = ARROWDOWN;
			}
			ObjectivePoint(const std::string& desc, const Vector& pos, int team = -1, ObjectiveArrowDir arrowDir = ARROWDOWN) {
				m_Description = desc;
				m_ScenePos = pos;
				m_Team = (Teams)team;
				m_ArrowDir = arrowDir;
			}

			/// Simply draws this' arrow relative to a point on a bitmap.
			/// @param pTargetBitmap A pointer to the BITMAP to draw on.
			/// @param pArrowBitmap The arrow bitmap to draw, assuming it points downward.
			/// @param arrowPoint The absolute position on the bitmap to draw the point of the arrow at.
			/// @param arrowDir Which orientation to draw the arrow in, relative to the point. (default: ARROWDOWN)
			void Draw(BITMAP* pTargetBitmap, BITMAP* pArrowBitmap, const Vector& arrowPoint, ObjectiveArrowDir arrowDir = ARROWDOWN);

			// The description of this objective point
			std::string m_Description;
			// Absolute position in the scene where this is pointed
			Vector m_ScenePos;
			// The team this objective is relevant to
			Teams m_Team;
			// The positioning of the arrow that points at this objective
			ObjectiveArrowDir m_ArrowDir;
		};

		// Comparison functor for sorting objective points by their y pos using STL's sort
		struct ObjPointYPosComparison {
			bool operator()(ObjectivePoint& rhs, ObjectivePoint& lhs) { return rhs.m_ScenePos.m_Y < lhs.m_ScenePos.m_Y; }
		};

		/// Gets the next other team number from the one passed in, if any. If there
		/// are more than two teams in this game, then the next one in the series
		/// will be returned here.
		/// @param team The team not to get.
		/// @return The other team's number.
		int OtherTeam(int team);

		/// Indicates whether there is less than two teams left in this game with
		/// a brain in its ranks.
		/// @return Whether less than two teams have brains in them left.
		bool OneOrNoneTeamsLeft();

		/// Indicates which single team is left, if any.
		/// @return Which team stands alone with any brains in its ranks, if any. NoTeam
		/// is returned if there's either more than one team, OR there are no
		/// teams at all left with brains in em.
		int WhichTeamLeft();

		/// Indicates whether there are NO teams left with any brains at all!
		/// @return Whether any team has a brain in it at all.
		bool NoTeamLeft();

		/// Goes through all Actor:s currently in the MovableMan and sets each
		/// one not controlled by a player to be AI controlled and AIMode setting
		/// based on team and CPU team.
		virtual void InitAIs();

		/// Goes through all Actor:s currently in the MovableMan and disables or
		/// enables each one with a Controller set to AI input.
		/// @param disable Whether to disable or enable them; (default: true)
		/// @param whichTeam Which team to do this to. If all, then pass Teams::NoTeam (default: Teams::NoTeam)
		void DisableAIs(bool disable = true, int whichTeam = Teams::NoTeam);

		// Member variables
		static Entity::ClassInfo m_sClass;

		// Which team is CPU-managed, if any (-1) - LEGACY, now controlled by Activity::m_IsHuman
		int m_CPUTeam;
		// Team is active or not this game
		bool m_TeamIsCPU[Teams::MaxTeamCount];

		// The observation sceneman scroll targets, for when the game is over or a player is in observation mode
		Vector m_ObservationTarget[Players::MaxPlayerCount];
		// The player death sceneman scroll targets, for when a player-controlled actor dies and the view should go to his last position
		Vector m_DeathViewTarget[Players::MaxPlayerCount];
		// Times the delay between regular actor swtich, and going into manual siwtch mode
		Timer m_ActorSelectTimer[Players::MaxPlayerCount];
		// The cursor for selecting new Actors
		Vector m_ActorCursor[Players::MaxPlayerCount];
		// Highlighted actor while cursor switching; will be switched to if switch button is released now
		Actor* m_pLastMarkedActor[Players::MaxPlayerCount];
		// The last selected landing zone
		Vector m_LandingZone[Players::MaxPlayerCount];
		// Whether the last craft was set to return or not after delivering
		bool m_AIReturnCraft[Players::MaxPlayerCount];
		std::array<std::unique_ptr<PieMenu>, Players::MaxPlayerCount> m_StrategicModePieMenu; //!< The strategic mode PieMenus for each Player.
		// The inventory menu gui for each player
		InventoryMenuGUI* m_InventoryMenuGUI[Players::MaxPlayerCount];
		// The in-game buy GUIs for each player
		BuyMenuGUI* m_pBuyGUI[Players::MaxPlayerCount];
		// The in-game scene editor GUI for each player
		SceneEditorGUI* m_pEditorGUI[Players::MaxPlayerCount];
		bool m_LuaLockActor[Players::MaxPlayerCount]; //!< Whether or not to lock input for each player while lua has control.
		Controller::InputMode m_LuaLockActorMode[Players::MaxPlayerCount]; //!< The input mode to lock to while lua has control.
		// The in-game important message banners for each player
		GUIBanner* m_pBannerRed[Players::MaxPlayerCount];
		GUIBanner* m_pBannerYellow[Players::MaxPlayerCount];
		// How many times a banner has been repeated.. so we dont' annoy by repeating forever
		int m_BannerRepeats[Players::MaxPlayerCount];
		// Whether each player has marked himself as ready to start. Can still edit while this is set, but when all are set, the game starts
		bool m_ReadyToStart[Players::MaxPlayerCount];
		// An override purchase list that can be set by a script and will be used instead of what's in the buy menu. Object held in here are NOT OWNED
		// Once a delivery is made with anything in here, this list is automatically cleared out, and the next delivery will be what's set in the buy menu.
		std::list<const SceneObject*> m_PurchaseOverride[Players::MaxPlayerCount];

		// The delivery queue which contains all the info about all the made orders currently in transit to delivery
		std::deque<Delivery> m_Deliveries[Teams::MaxTeamCount];
		// The box within where landing zones can be put
		Scene::Area m_LandingZoneArea[Teams::MaxTeamCount];
		// How wide around the brain the automatic LZ is following
		int m_BrainLZWidth[Players::MaxPlayerCount];
		// The objective points for each team
		std::list<ObjectivePoint> m_Objectives;

		// Tech of player
		std::string m_TeamTech[Teams::MaxTeamCount];

		// Initial gold amount selected by player in scenario setup dialog
		int m_StartingGold;
		// Whether fog of war was enabled or not in scenario setup dialog
		bool m_FogOfWarEnabled;
		// Whether we need a clear path to orbit to place brain
		bool m_RequireClearPathToOrbit;

		// Default fog of war switch state for this activity, default -1 (unspecified)
		int m_DefaultFogOfWar;
		// Default clear path to orbit switch value, default -1 (unspecified)
		int m_DefaultRequireClearPathToOrbit;
		// Default deploy units swutch value, default -1 (unspecified)
		int m_DefaultDeployUnits;
		// Default gold amount for different difficulties, defalt -1 (unspecified)
		int m_DefaultGoldCakeDifficulty;
		int m_DefaultGoldEasyDifficulty;
		int m_DefaultGoldMediumDifficulty;
		int m_DefaultGoldHardDifficulty;
		int m_DefaultGoldNutsDifficulty;
		int m_DefaultGoldMaxDifficulty;
		// Whether those switches are enabled or disabled in scenario setup dialog, true by default
		bool m_FogOfWarSwitchEnabled;
		bool m_DeployUnitsSwitchEnabled;
		bool m_GoldSwitchEnabled;
		bool m_RequireClearPathToOrbitSwitchEnabled;
		bool m_BuyMenuEnabled;

		// The cursor animations for the LZ indicators
		std::vector<BITMAP*> m_aLZCursor[4];
		std::array<int, Players::MaxPlayerCount> m_LZCursorWidth; //!< The width of each players' LZ cursor.
		// The cursor animations for the objective indications
		std::vector<BITMAP*> m_aObjCursor[4];

		// Time it takes for a delivery to be made, in ms
		long m_DeliveryDelay;
		// Cursor animation timer
		Timer m_CursorTimer;
		// Total gameplay timer, not including editing phases
		Timer m_GameTimer;
		// Game end timer
		Timer m_GameOverTimer;
		// Time between game over and reset
		long m_GameOverPeriod;
		// The winning team number, when the game is over
		int m_WinnerTeam;

		std::string m_NetworkPlayerNames[Players::MaxPlayerCount];

		/// Private member variable and method declarations
	private:
		/// Clears all the member variables of this Activity, effectively
		/// resetting the members of this abstraction level only.
		void Clear();
	};

} // namespace RTE
