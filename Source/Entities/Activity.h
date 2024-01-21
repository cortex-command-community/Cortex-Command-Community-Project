#pragma once

#include "Icon.h"
#include "Controller.h"
#include "GenericSavedData.h"

namespace RTE {

	class Scene;
	class ACraft;

	/// Base class for all Activities, including game modes and editors.
	class Activity : public Entity {

	public:
		SerializableOverrideMethods;
		ClassInfoGetters;

		/// Enumeration for the different states the Activity can be in.
		enum ActivityState {
			NoActivity = -1,
			NotStarted = 0,
			Starting,
			Editing,
			PreGame,
			Running,
			HasError,
			Over
		};

		/// Enumeration for the different teams in an activity.
		enum Teams {
			NoTeam = -1,
			TeamOne = 0,
			TeamTwo,
			TeamThree,
			TeamFour,
			MaxTeamCount
		};

		/// Enumeration for the different observations states (camera viewpoint).
		enum ViewState {
			Normal = 0,
			Observe,
			DeathWatch,
			ActorSelect,
			AISentryPoint,
			AIPatrolPoints,
			AIGoldDigPoint,
			AIGoToPoint,
			LandingZoneSelect,
			UnitSelectCircle,
		};

		/// Enumeration for the different difficulty settings.
		enum DifficultySetting {
			MinDifficulty = 0,
			CakeDifficulty = 20,
			EasyDifficulty = 40,
			MediumDifficulty = 60,
			HardDifficulty = 80,
			NutsDifficulty = 95,
			MaxDifficulty = 100,
		};

		/// Enumeration for the different AI skill settings.
		enum AISkillSetting {
			MinSkill = 1,
			InferiorSkill = 35,
			DefaultSkill = 50,
			AverageSkill = 70,
			GoodSkill = 99,
			UnfairSkill = 100
		};

#pragma region Creation
		/// Constructor method used to instantiate an Activity object in system memory. Create() should be called before using the object.
		Activity();

		/// Makes the Activity object ready for use.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create() override;

		/// Creates an Activity to be identical to another, by deep copy.
		/// @param reference A reference to the Activity to deep copy.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(const Activity& reference);
#pragma endregion

#pragma region Destruction
		/// Destructor method used to clean up an Activity object before deletion from system memory.
		~Activity() override;

		/// Destroys and resets (through Clear()) the Activity object.
		/// @param notInherited Whether to only destroy the members defined in this derived class, or to destroy all inherited members also.
		void Destroy(bool notInherited = false) override {
			if (!notInherited) {
				Entity::Destroy();
			}
			Clear();
		}
#pragma endregion

#pragma region Getters and Setters
		/// Gets the current Activity state code. See the ActivityState enumeration for values.
		/// @return The current state of this Activity. See ActivityState enumeration.
		ActivityState GetActivityState() const { return m_ActivityState; }

		/// Overrides the current Activity state. Should not be used much, use dedicated state setting functions instead.
		/// @param newState The new state to set.
		void SetActivityState(ActivityState newState) { m_ActivityState = newState; }

		/// Indicates whether the Activity is currently running or not (not editing, over or paused)
		/// @return Whether the Activity is running or not.
		bool IsRunning() const { return (m_ActivityState == ActivityState::Running || m_ActivityState == ActivityState::Editing) && !m_Paused; }

		/// Indicates whether the Activity is currently paused or not.
		/// @return Whether the Activity is paused or not.
		bool IsPaused() const { return m_Paused; }

		/// Pauses and unpauses the Activity.
		/// @param pause Whether to pause the Activity or not.
		virtual void SetPaused(bool pause = true) { m_Paused = pause; }

		/// Indicates whether the Activity is over or not.
		/// @return Whether the Activity is over or not.
		bool IsOver() const { return m_ActivityState == ActivityState::Over; }

		/// Gets the user-friendly description of this Activity.
		/// @return A string with the user-friendly description of this Activity.
		std::string GetDescription() const { return m_Description; }

		/// Gets the max number of players supported by this Activity.
		/// @return The max number of players supported by this Activity.
		int GetMaxPlayerSupport() const { return m_MaxPlayerSupport; }

		/// Gets the minimum number of teams with players that this Activity requires.
		/// @return The minimum number of Teams this Activity requires to run.
		int GetMinTeamsRequired() const { return m_MinTeamsRequired; }

		/// Tells if a particular Scene supports this specific Activity on it. Usually that means certain Areas need to be defined in the Scene.
		/// @param scene The Scene to check if it supports this Activity. Ownership is NOT transferred!
		/// @param teams How many teams we're checking for. Some scenes may support and Activity but only for a limited number of teams. If -1, not applicable.
		/// @return Whether the Scene has the right stuff.
		virtual bool SceneIsCompatible(Scene* scene, int teams = -1) { return scene && teams <= m_MinTeamsRequired; }

		/// Shows in which stage of the Campaign this appears.
		/// @return The stage number in the campaign. -1 means it's not in the campaign.
		int GetInCampaignStage() const { return m_InCampaignStage; }

		///
		/// Sets in which stage of the Campaign this appears.
		/// @param newStage The new stage to set. -1 means it doesn't appear in the campaign.
		void SetInCampaignStage(int newStage) { m_InCampaignStage = newStage; }

		/// Gets the name of the current scene.
		/// @return A string with the instance name of the scene.
		std::string GetSceneName() const { return m_SceneName; }

		/// Sets the name of the scene this is associated with.
		/// @param sceneName The new name of the scene to load next game.
		void SetSceneName(const std::string sceneName) { m_SceneName = sceneName; }

		/// Gets whether craft must be considered orbited if they reach the map border on non-wrapped maps.
		/// @return Whether craft are considered orbited when at the border of a non-wrapping map.
		bool GetCraftOrbitAtTheEdge() const { return m_CraftOrbitAtTheEdge; }

		/// Sets whether craft must be considered orbited if they reach the map border on non-wrapped maps.
		/// @param value Whether to consider orbited or not.
		void SetCraftOrbitAtTheEdge(bool value) { m_CraftOrbitAtTheEdge = value; }
#pragma endregion

#pragma region Virtual Override Methods
		/// Officially starts this Activity. Creates all the data necessary to start the Activity.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		virtual int Start();

		/// Forces this Activity to end.
		virtual void End();

		/// Updates the state of this Activity. Supposed to be done every frame before drawing.
		virtual void Update();

		/// Draws the currently active GUI of a screen to a BITMAP of choice.
		/// @param targetBitmap A pointer to a screen-sized BITMAP to draw on.
		/// @param targetPos The absolute position of the target bitmap's upper left corner in the scene.
		/// @param whichScreen Which screen's GUI to draw onto the bitmap.
		virtual void DrawGUI(BITMAP* targetBitmap, const Vector& targetPos = Vector(), int whichScreen = 0) {}

		/// Draws this Activity's current graphical representation to a BITMAP of choice. This includes all game-related graphics.
		/// @param targetBitmap A pointer to a BITMAP to draw on.
		/// @param targetPos The absolute position of the target bitmap's upper left corner in the scene.
		virtual void Draw(BITMAP* targetBitmap, const Vector& targetPos = Vector()) {}
#pragma endregion

#pragma region Player Handling
		/// Pre-calculates the player-to-screen index map, counts the number of active players etc.
		virtual void SetupPlayers();

		/// Indicates whether a specific player is active in the current game.
		/// @param player Which player index to check.
		/// @return Whether the player is active in the current Activity.
		bool PlayerActive(int player) const { return m_IsActive[player]; }

		/// Turns off a player if they were active. Should only be done if brain etc are already taken care of and disposed of properly.
		/// Will also deactivate the team this player is on, if there's no other active players still on it.
		/// @param playerToDeactivate Which player index to deactivate.
		/// @return Whether the player was active before trying to deactivate.
		bool DeactivatePlayer(int playerToDeactivate);

		/// Sets up a specific player for this Activity, AI or Human.
		/// @param playerToAdd Which player slot to set up - PlayerOne to PlayerFour.
		/// @param isHuman Whether this player is Human.
		/// @param team Which Team this player belongs to.
		/// @param funds How many funds this player contributes to its Team's total funds.
		/// @param teamIcon The team flag icon of this player. Ownership is NOT transferred!
		/// @return The new total number of active players in the current game.
		int AddPlayer(int playerToAdd, bool isHuman, int team, float funds, const Icon* teamIcon = 0);

		/// Sets all players as not active in the current Activity.
		/// @param resetFunds Whether to reset the team funds as well.
		void ClearPlayers(bool resetFunds = true);

		/// Gets the total number of active players in the current Activity, AI or Human.
		/// @return The total number of players in the current Activity.
		int GetPlayerCount() const { return m_PlayerCount; }

		/// Gets the total number of human players in the current Activity.
		/// @return The total number of players in the current Activity.
		int GetHumanCount() const;

		/// Indicates whether a specific player is human in the current game, ie not an AI player and has a screen etc.
		/// @param player Which player index to check.
		/// @return Whether the player is active as a Human in the current Activity.
		bool PlayerHuman(int player) const { return m_IsHuman[player]; }

		/// Gets the current team a specific player belongs to.
		/// @param player The player to get the team info on.
		/// @return The team number of the specified player.
		int GetTeamOfPlayer(int player) const { return m_Team[player]; }

		/// Sets the current team a specific player belongs to.
		/// @param player The player to set the team for.
		/// @param team The team number to set the player to.
		void SetTeamOfPlayer(int player, int team);

		/// Converts a player index into a screen index, and only if that player is human.
		/// @param player Which player index to convert.
		/// @return The screen index, or -1 if non-human player or no players.
		int ScreenOfPlayer(int player) const { return (player >= Players::PlayerOne && player < Players::MaxPlayerCount) ? m_PlayerScreen[player] : -1; }

		/// Converts a screen index into a player index, if that screen exists.
		/// @param screen Which screen index to convert.
		/// @return The player index, or -1 if that screen is not in use.
		int PlayerOfScreen(int screen) const;

		/// Gets the current viewing state for a specific player. See the ViewState enumeration for values.
		/// @param whichPlayer Which player to get the view state for.
		/// @return The current viewing state of the player.
		ViewState GetViewState(int whichPlayer = 0) const { return m_ViewState[whichPlayer]; }

		/// Sets the current viewing state for a specific player. See the ViewState enumeration for values.
		/// @param whichViewState The state to set to.
		/// @param whichPlayer Which player to set the view state for.
		void SetViewState(ViewState whichViewState, int whichPlayer = 0) { m_ViewState[whichPlayer] = whichViewState; }

		/// Resets the message timer for one player.
		/// @param player The player to reset the message timer for.
		void ResetMessageTimer(int player = 0) {
			if (player >= Players::PlayerOne && player < Players::MaxPlayerCount) {
				m_MessageTimer[player].Reset();
			}
		}

		/// Gets a pointer to the GUI controller of the specified player.
		/// @param player Which player to get the Controller of.
		/// @return A pointer to the player's Controller. Ownership is NOT transferred!
		Controller* GetPlayerController(int player = 0) { return (player >= Players::PlayerOne && player < Players::MaxPlayerCount) ? &m_PlayerController[player] : nullptr; }
#pragma endregion

#pragma region Team Handling
		/// Gets the total number of teams in the current Activity.
		/// @return The total number of teams in the current Activity.
		int GetTeamCount() const { return m_TeamCount; }

		/// Gets the name of a specific team.
		/// @param whichTeam Which team to get the name of. 0 = first team.
		/// @return The current name of that team.
		std::string GetTeamName(int whichTeam = 0) const;

		/// Sets the name of a specific team.
		/// @param whichTeam Which team to set the name of. 0 = first team.
		/// @param newName The name to set it to.
		void SetTeamName(int whichTeam, const std::string& newName) {
			if (whichTeam >= Teams::TeamOne && whichTeam < Teams::MaxTeamCount) {
				m_TeamNames[whichTeam] = newName;
			}
		}

		/// Gets the Icon of a specific team.
		/// @param whichTeam Which team to get the Icon of. 0 = first team.
		/// @return The current Icon of that team.
		const Icon* GetTeamIcon(int whichTeam = 0) const { return (whichTeam >= Teams::TeamOne && whichTeam < Teams::MaxTeamCount) ? &m_TeamIcons[whichTeam] : nullptr; }

		/// Sets the Icon of a specific team.
		/// @param whichTeam Which team to set the Icon of. 0 = first team.
		/// @param newIcon The Icon to set it to.
		void SetTeamIcon(int whichTeam, const Icon& newIcon) {
			if (whichTeam >= Teams::TeamOne && whichTeam < Teams::MaxTeamCount) {
				m_TeamIcons[whichTeam] = newIcon;
			}
		}

		/// Indicates whether a specific team is active in the current game.
		/// @param team Which team index to check.
		/// @return Whether the team is active in the current Activity.
		bool TeamActive(int team) const { return (team >= Teams::TeamOne && team < Teams::MaxTeamCount) ? m_TeamActive[team] : false; }

		/// Sets the given team as active, even if it shouldn't be considered as such normally. Useful for Activities that don't want to define/show all used teams.
		/// @param team The team to force as active.
		void ForceSetTeamAsActive(int team) {
			if (team >= Teams::TeamOne && team < Teams::MaxTeamCount) {
				m_TeamActive[team] = true;
			}
		}

		/// Indicates whether a team is player controlled or not.
		/// @param team The team number to check.
		/// @return Whether  team is player controlled or not.
		bool IsHumanTeam(int team) const;

		/// Gets the current number of players in a specific team.
		/// @param team Which team to get the player count for.
		/// @return The player count in the specified team.
		int PlayersInTeamCount(int team) const;

		/// Gets the number of deaths on a specific team so far on the current Activity.
		/// @param whichTeam Which team to get the death tally of. 0 = first team.
		/// @return The current death count.
		int GetTeamDeathCount(int whichTeam = 0) const { return (whichTeam >= Teams::TeamOne && whichTeam < Teams::MaxTeamCount) ? m_TeamDeaths[whichTeam] : 0; }

		/// Increments the tally of a death of an actor on a specific team.
		/// @param whichTeam Which team to increase the death count of. 0 = first team.
		/// @param howMany The new death count.
		/// @return The updated death count of the team.
		int ReportDeath(int whichTeam = 0, int howMany = 1) { return (whichTeam >= Teams::TeamOne && whichTeam < Teams::MaxTeamCount) ? m_TeamDeaths[whichTeam] += howMany : 0; }
#pragma endregion

#pragma region Funds Handling
		/// Gets the amount of funds a specific team currently has in the Activity.
		/// @param whichTeam Which team to get the fund count from. 0 = first team.
		/// @return A float with the funds tally for the requested team.
		float GetTeamFunds(int whichTeam = 0) const { return (whichTeam >= Teams::TeamOne && whichTeam < Teams::MaxTeamCount) ? m_TeamFunds[whichTeam] : 0; }

		/// Sets the amount of funds a specific team currently has in the Activity.
		/// @param newFunds Which team to set the fund count for. 0 = first team.
		/// @param which A float with the funds tally for the requested team.
		void SetTeamFunds(float newFunds, int whichTeam = 0) {
			if (whichTeam >= Teams::TeamOne && whichTeam < Teams::MaxTeamCount) {
				m_TeamFunds[whichTeam] = newFunds;
			}
		}

		/// Changes a team's funds level by a certain amount.
		/// @param howMuch The amount with which to change the funds balance.
		/// @param whichTeam Which team to alter the funds of. 0 = first team.
		void ChangeTeamFunds(float howMuch, int whichTeam = 0);

		/// Checks whether the team funds changed since last time this was called. This also resets the state, so calling this again on the same team will yield false unless it's been changed again.
		/// @param whichTeam Which team's funds to check.
		/// @return Whether funds amount changed for this team since last time this was called.
		bool TeamFundsChanged(int whichTeam = 0);

		/// Gets the amount of funds a specific player originally added to his team's collective stash.
		/// @param player Which player to check for.
		/// @return A float with the funds originally deposited by this player.
		float GetPlayerFundsContribution(int player) const { return (player >= Players::PlayerOne && player < Players::MaxPlayerCount) ? m_FundsContribution[player] : 0; }

		/// Sets a new amount of starting funds for a player, after he has already been added.
		/// @param player Which player slot to update - has to already be active.
		/// @param newFunds Updated value of how many funds this player contributes to its Team's total funds.
		/// @return Whether the update was successful.
		bool UpdatePlayerFundsContribution(int player, float newFunds);

		/// Gets the share of funds a specific PLAYER currently has in the game, calculated from his original contribution to his team's collective funds.
		/// @param player Which player to get the fund count from.
		/// @return A float with the funds tally for the requested player.
		float GetPlayerFundsShare(int player = 0) const;
#pragma endregion

#pragma region Brain Handling
		/// Shows how many human controlled brains are left in this Activity.
		/// @return How many human controlled brains are left in this Activity.
		int HumanBrainCount() const { return GetBrainCount(true); }

		/// Shows how many AI controlled brains are left in this Activity.
		/// @return how many AI controlled brains are left in this Activity.
		int AIBrainCount() const { return GetBrainCount(false); }

		/// Gets the current Brain actor for a specific player.
		/// @param player Which player to get the brain actor for.
		/// @return A pointer to the Brain Actor. Ownership is NOT transferred!
		Actor* GetPlayerBrain(int player = 0) const { return (player >= Players::PlayerOne && player < Players::MaxPlayerCount) ? m_Brain[player] : nullptr; }

		/// Sets the current Brain actor for a specific player.
		/// @param newBrain A pointer to the new brain Actor. Ownership is NOT transferred!
		/// @param player Which team to set the brain actor for.
		void SetPlayerBrain(Actor* newBrain, int player = 0);

		/// Shows whether a specific player ever had a Brain yet.
		/// @param player Which player to check whether they ever had a Brain.
		/// @return Whether this player ever had a Brain.
		bool PlayerHadBrain(int player = 0) const { return (player >= Players::PlayerOne && player < Players::MaxPlayerCount) ? m_HadBrain[player] : false; }

		/// Sets to indicate that the player had a Brain at some point. This is to simulate that in automated battle cases.
		/// @param player Which player to set whether he had a Brain or not.
		/// @param hadBrain Whether he should be flagged as having had a Brain.
		void SetPlayerHadBrain(int player, bool hadBrain = true) {
			if (player >= Players::PlayerOne && player < Players::MaxPlayerCount)
				m_HadBrain[player] = hadBrain;
		}

		/// Shows whether a specific player's Brain was evacuated into orbit so far.
		/// @param player Which player to check whether their Brain was evacuated.
		/// @return Whether this player had a Brain that was evacuated.
		bool BrainWasEvacuated(int player = 0) const { return (player >= Players::PlayerOne && player < Players::MaxPlayerCount) ? m_BrainEvacuated[player] : false; }

		/// Sets whether a player's Brain was evacuated during the Activity.
		/// @param player Which player to check whether their Brain was evacuated.
		/// @param evacuated Whether it was evacuated yet.
		void SetBrainEvacuated(int player = 0, bool evacuated = true) {
			if (player >= Players::PlayerOne && player < Players::MaxPlayerCount) {
				m_BrainEvacuated[player] = evacuated;
			}
		}

		/// Shows whether ANY player evacuated their Brain.
		/// @return Whether any player evacuated their Brain yet.
		bool AnyBrainWasEvacuated() const;

		/// Shows whether the passed in actor is the Brain of any player.
		/// @param actor Which Actor to check for player braininess.
		/// @return Whether any player's Brain or not.
		bool IsAssignedBrain(Actor* actor) const;

		/// Shows which player has a specific actor as a Brain, if any.
		/// @param actor Which Actor to check for player braininess.
		/// @return Which player has this assigned as a Brain, if any.
		int IsBrainOfWhichPlayer(Actor* actor) const;

		/// Shows whether the passed in actor is the Brain of any other player.
		/// @param actor Which Actor to check for other player braininess.
		/// @param player From which player's perspective to check.
		/// @return Whether other player's Brain or not.
		bool IsOtherPlayerBrain(Actor* actor, int player) const;
#pragma endregion

#pragma region Difficulty Handling
		/// Returns string representation of a given difficulty value.
		/// @param difficulty Difficulty setting
		/// @return Corresponding difficulty string.
		static std::string GetDifficultyString(int difficulty);

		/// Gets the current difficulty setting.
		/// @return The current setting.
		int GetDifficulty() const { return m_Difficulty; }

		/// Sets the current difficulty setting.
		/// @param newDifficulty The new difficulty setting.
		void SetDifficulty(int newDifficulty) { m_Difficulty = Limit(newDifficulty, DifficultySetting::MaxDifficulty, DifficultySetting::MinDifficulty); }
#pragma endregion

#pragma region AI Handling
		/// Returns string representation of a given AI skill value.
		/// @param skill AI skill setting.
		/// @return Corresponding AI skill string.
		static std::string GetAISkillString(int skill);

		/// Returns skill level for specified team. If team is less than 0 or greater than 3 an average of all teams is returned.
		/// @param team Team to get skill level for.
		/// @return Team skill level.
		int GetTeamAISkill(int team) const;

		/// Sets AI skill level for specified team.
		/// @param team The team to set for.
		/// @param skill AI skill level, 1-100.
		void SetTeamAISkill(int team, int skill) {
			if (team >= Teams::TeamOne && team < Teams::MaxTeamCount) {
				m_TeamAISkillLevels[team] = Limit(skill, AISkillSetting::UnfairSkill, AISkillSetting::MinSkill);
			}
		}
#pragma endregion

#pragma region Actor Handling
		/// Gets the currently controlled actor of a specific player.
		/// @param player Which player to get the controlled actor of.
		/// @return A pointer to the controlled Actor. Ownership is NOT transferred! 0 If no actor is currently controlled by this player.
		Actor* GetControlledActor(int player = 0) { return (player >= Players::PlayerOne && player < Players::MaxPlayerCount) ? m_ControlledActor[player] : nullptr; }

		/// Makes the player's ControlledActor the leader of any squad it is a member of.
		/// @param player Player to reassign for.
		/// @param team Team of the player.
		void ReassignSquadLeader(const int player, const int team);

		/// Forces the ActivityMan to focus player control to a specific Actor for a specific team. Ownership is NOT transferred!
		/// @param actor Which Actor to switch focus to. The team of this Actor will be set once it is passed in. The actor should have been added to MovableMan already.
		/// @param player Player to force for.
		/// @param team Which team to switch to next actor on.
		/// @return Whether the focus switch was successful or not.
		virtual bool SwitchToActor(Actor* actor, int player = 0, int team = 0);

		/// Forces the Activity to focus player control to the previous Actor of a specific team, other than the current one focused on.
		/// @param player Player to force for.
		/// @param team Which team to switch to next Actor on.
		/// @param actorToSkip An Actor pointer to skip in the sequence.
		virtual void SwitchToPrevActor(int player, int team, Actor* actorToSkip = 0) { SwitchToPrevOrNextActor(false, player, team, actorToSkip); }

		/// Forces the Activity to focus player control to the next Actor of a specific team, other than the current one focused on.
		/// @param player Player to force for.
		/// @param team Which team to switch to next Actor on.
		/// @param actorToSkip An Actor pointer to skip in the sequence.
		virtual void SwitchToNextActor(int player, int team, Actor* actorToSkip = 0) { SwitchToPrevOrNextActor(true, player, team, actorToSkip); }

		/// Forces player to lose control of the currently selected Actor, as if it had died.
		/// @param player Which player to lose control of their selected Actor.
		virtual void LoseControlOfActor(int player);

		/// Handles when an ACraft has left the game scene and entered orbit, though does not delete it. Ownership is NOT transferred, as the ACraft's inventory is just 'unloaded'.
		/// @param orbitedCraft The ACraft instance that entered orbit. Ownership is NOT transferred!
		virtual void HandleCraftEnteringOrbit(ACraft* orbitedCraft);
#pragma endregion

#pragma region Save and Load Handling
		/// Gets whether or not this Activity can be saved by the user. For this to be true, the Scene must not be MetagameInternal, and the AllowsUserSaving flag must not be disabled.
		/// @return Whether or not this Activity can be saved.
		bool CanBeUserSaved() const;

		/// Gets whether or not this Activity allows the player to manually save.
		/// @return Whether or not this Activity allows the player to manually save.
		bool GetAllowsUserSaving() const { return m_AllowsUserSaving; }

		/// Sets whether or not this Activity can be manually saved be the player.
		/// @param allowsUserSaving Whether or not this Activity can be manually saved be the player.
		void SetAllowsUserSaving(bool allowsUserSaving) { m_AllowsUserSaving = allowsUserSaving; }

		/// Saves a string which will be stored in our ini.
		/// @param key The key of the saved string.
		/// @param value The string to save.
		void SaveString(const std::string& key, const std::string& value) { m_SavedValues.SaveString(key, value); };

		/// Loads and returns a previously saved string.
		/// @param key The key of the string to load.
		const std::string& LoadString(const std::string& key) { return m_SavedValues.LoadString(key); };

		/// Saves a number which will be stored in our ini.
		/// @param key The key of the saved number.
		/// @param value The number to save.
		void SaveNumber(const std::string& key, float value) { m_SavedValues.SaveNumber(key, value); };

		/// Loads and returns a previously saved number.
		/// @param key The key of the string to load.
		float LoadNumber(const std::string& key) { return m_SavedValues.LoadNumber(key); };
#pragma endregion

	protected:
		static Entity::ClassInfo m_sClass; //!< ClassInfo for this class.

		ActivityState m_ActivityState; //!< Current state of this Activity.
		bool m_Paused; //!< Whether this Activity is paused or not.

		bool m_AllowsUserSaving; //!< Whether or not the current Activity can be saved by the user.

		std::string m_Description; //!< User-friendly description of what this Activity is all about.
		std::string m_SceneName; //!< The name of the Scene in which this Activity takes place.

		int m_MaxPlayerSupport; //!< How many separate players this Activity can support at the same time.
		int m_MinTeamsRequired; //!< How many separate teams this Activity can support at the same time.

		int m_Difficulty; //!< Current difficulty setting of this Activity.
		bool m_CraftOrbitAtTheEdge; //!< If true then on non-wrapping maps craft beyond the edge of the map are considered orbited.
		int m_InCampaignStage; //!< Which stage of the campaign this Activity appears in, if any (-1 means it's not in the campaign).

		int m_PlayerCount; //!< The number of total players in the current Activity, AI and Human.
		bool m_IsActive[Players::MaxPlayerCount]; //!< Whether a specific player is at all active and playing this Activity.
		bool m_IsHuman[Players::MaxPlayerCount]; //!< Whether a specific player is Human or not, and needs a screen etc.

		int m_PlayerScreen[Players::MaxPlayerCount]; //!< The screen index of each player - only applicable to human players. -1 if AI or other.
		ViewState m_ViewState[Players::MaxPlayerCount]; //!< What to be viewing for each player.
		Timer m_DeathTimer[Players::MaxPlayerCount]; //!< Timers for measuring death view delays.

		std::string m_TeamNames[Teams::MaxTeamCount]; //!< Names for each team.
		Icon m_TeamIcons[Teams::MaxTeamCount]; //!< Icons for each team.

		int m_TeamCount; //!< The number of teams in the current Activity.
		bool m_TeamActive[Teams::MaxTeamCount]; //!< Whether a specific team is active or not in this Activity.
		int m_Team[Players::MaxPlayerCount]; //!< The designated team of each player.
		int m_TeamDeaths[Teams::MaxTeamCount]; //!< The count of how many actors have died on this team.
		int m_TeamAISkillLevels[Teams::MaxTeamCount]; //!< AI skill levels for teams.

		float m_TeamFunds[Teams::MaxTeamCount]; //!< Gold counter for each team.
		float m_TeamFundsShare[Players::MaxPlayerCount]; //!< The ratio of how much this player contributed to his team's funds at the start of the Activity.
		bool m_FundsChanged[Teams::MaxTeamCount]; //!< Whether the team funds have changed during the current frame.
		float m_FundsContribution[Players::MaxPlayerCount]; //!< How much this player contributed to his team's funds at the start of the Activity.

		Actor* m_Brain[Players::MaxPlayerCount]; //!< The Brain of each player. Not owned!
		bool m_HadBrain[Players::MaxPlayerCount]; //!< Whether each player has yet had a Brain. If not, then their Activity doesn't end if no brain is found.
		bool m_BrainEvacuated[Players::MaxPlayerCount]; //!< Whether a player has evacuated his Brain into orbit.

		Actor* m_ControlledActor[Players::MaxPlayerCount]; //!< Currently controlled actor, not owned.
		Controller m_PlayerController[Players::MaxPlayerCount]; //!< The Controllers of all the players for the GUIs.

		Timer m_MessageTimer[Players::MaxPlayerCount]; //!< Message timer for each player.

		/// Generic additional saved strings/numbers, which are used for scripts primarily.
		/// They live here in the base class because GAScripted doesn't have a lua interface although it's a little messy.
		/// On the bright side, this would allows other parts of the code to add some metadata to stamp extra information onto an activity if needed, that'll be ignored otherwise.
		GenericSavedData m_SavedValues;

	private:
		/// Shared method to get the amount of human or AI controlled brains that are left in this Activity.
		/// @param getForHuman Whether to get brain count for Human or for AI. True for Human.
		/// @return How many human or AI controlled brains are left in this Activity.
		int GetBrainCount(bool getForHuman) const;

		/// Shared method to force the Activity to focus player control to the previous or next Actor of a specific team, other than the current one focused on.
		/// @param nextActor Whether to switch to the previous or the next Actor. True for next Actor.
		/// @param player Player to force for.
		/// @param team Which team to switch to next Actor on.
		/// @param skip An Actor pointer to skip in the sequence.
		void SwitchToPrevOrNextActor(bool nextActor, int player, int team, const Actor* skip = 0);

		/// Clears all the member variables of this Activity, effectively resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		Activity(const Activity& reference) = delete;
		Activity& operator=(const Activity& rhs) = delete;
	};
} // namespace RTE
