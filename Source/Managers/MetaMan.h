#pragma once

/// Header file for the MetaMan class.
/// @author Daniel Tabar
/// data@datarealms.com
/// http://www.datarealms.com
/// Inclusions of header files
#include "RTETools.h"
#include "Singleton.h"
#define g_MetaMan MetaMan::Instance()

// #include "Serializable.h"
#include "SceneObject.h"
#include "Controller.h"
// #include "FrameMan.h"
// #include "SceneMan.h"
#include "ActivityMan.h"
#include "MetaPlayer.h"
#include "GAScripted.h"
#include "Icon.h"
#include "GUIBanner.h"

namespace RTE {

	class System;

#define DEFAULTGAMENAME "NewGame"
#define AUTOSAVENAME "AutoSave"
#define METASAVEPATH System::GetUserdataDirectory() + c_UserConquestSavesModuleName + "/"
#define METASAVEMODULENAME c_UserConquestSavesModuleName

	class MetagameGUI;
	class MetaSave;
	class Scene;

	/// The singleton manager of the Metagame of Cortex Command, ie the
	/// games played out in the campaign screen.
	class MetaMan : public Singleton<MetaMan>, public Serializable {
		friend struct ManagerLuaBindings;

		/// Public member variable, method and friend function declarations
	public:
		friend class MetagameGUI;
		friend class MetaSave;

		SerializableClassNameGetter;
		SerializableOverrideMethods;

		enum MetagameState {
			NOGAME = -1,
			GAMEINTRO = 0,
			NEWROUND,
			REVEALSCENES,
			COUNTINCOME,
			PLAYER1TURN,
			PLAYER2TURN,
			PLAYER3TURN,
			PLAYER4TURN,
			BUILDBASES,
			RUNACTIVITIES,
			ENDROUND,
			GAMEOVER
		};

		/// Constructor method used to instantiate a MetaMan object in system
		/// memory. Create() should be called before using the object.
		MetaMan();

		/// Destructor method used to clean up a MetaMan object before deletion
		/// from system memory.
		~MetaMan();

		/// Makes the MetaMan object ready for use.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Initialize();

		/// Wipes any current and sets up a new game based on a size parameter.
		/// @param gameSize The size of the new Metagame, which will affect how (default: 3)
		/// many Scenes/Sites will ultimately be used.
		/// @return An error return value signaling success or any particular failure.
		/// Anything below 0 is an error signal.
		int NewGame(int gameSize = 3);

		/// Wipes any current metagame and sets things back to as if program start.
		/// @return An error return value signaling success or any particular failure.
		/// Anything below 0 is an error signal.
		int EndGame();

		/// Load a Metagame from disk out of the special Metagame.rte data module
		/// @param pSave The MetaSave object to load from - Ownership Is Not Transferred!
		/// @return An error return value signaling success or any particular failure.
		/// Anything below 0 is an error signal.
		int Load(const MetaSave* pSave);

		/// Saves the bitmap data of all Scenes of this Metagame that are currently
		/// loaded.
		/// @param pathBase The filepath base to the where to save the Bitmap data. This means
		/// everything up to and including the unique name of the game.
		/// @return An error return value signaling success or any particular failure.
		/// Anything below 0 is an error signal.
		int SaveSceneData(std::string pathBase);

		/// Loads the bitmap data of all Scenes of this Metagame that have once
		/// been saved to files.
		/// @return An error return value signaling success or any particular failure.
		/// Anything below 0 is an error signal.
		int LoadSceneData();

		/// Clears the bitmap data of all Scenes of this Metagame that have once
		/// been saved to files.
		/// @return An error return value signaling success or any particular failure.
		/// Anything below 0 is an error signal.
		int ClearSceneData();

		/// Resets the entire MetaMan, including its inherited members, to
		/// their default settings or values.
		void Reset() override { Clear(); }

		/// Destroys and resets (through Clear()) the MetaMan object.
		void Destroy();

		/// Sets the name of the currently played Metagame. It's what's used when
		/// saving to disk.
		/// @param newName The Metagame's name.
		void SetGameName(std::string newName) { m_GameName = newName; }

		/// Gets the name of the currently played Metagame. It's what's used when
		/// saving to disk.
		/// @return The name of the current metagame.
		std::string GetGameName() const { return m_GameName; }

		/// Gets the GUI controller of this Metagame.
		/// @return The GUI controller of the metagame.
		MetagameGUI* GetGUI() { return m_pMetaGUI; }

		/// Shows which player's turn is now or coming up.
		/// @return The player who is currently doing his turn, or coming up next in an
		/// intermediate phase.
		int GetPlayerTurn() const;

		/// Gets current number of MetaPlayers
		/// @return The number of meta players in the current game.
		int GetPlayerCount() const { return m_Players.size(); }

		/// Gets the designated team of a specific player
		/// @param metaPlayer Which player.
		/// @return The team of that player.
		int GetTeamOfPlayer(int metaPlayer) const { return metaPlayer >= Players::PlayerOne && metaPlayer < static_cast<int>(m_Players.size()) ? m_Players[metaPlayer].GetTeam() : Activity::NoTeam; }

		/// Gets the specified MetaPlayer
		/// @param metaPlayer Which player.
		/// @return The requested MetaPlayer
		MetaPlayer* GetPlayer(int metaPlayer) { return (metaPlayer >= Players::PlayerOne && metaPlayer < static_cast<int>(m_Players.size())) ? &(m_Players[metaPlayer]) : nullptr; }

		/// Gets the MetaPlayer playing a specific in-game player, if any.
		/// @param inGamePlayer Which in-game player to translate into a metaplayer.
		/// @return The requested MetaPlayer, if any is playing that in-game player. If not
		/// 0 is returned.
		MetaPlayer* GetMetaPlayerOfInGamePlayer(int inGamePlayer);

		/// Gets the flag Icon of a specific team
		/// @param team The team to get the Team icon of.
		/// @return A reference to the Icon.
		Icon& GetTeamIcon(int team) { return m_TeamIcons[team]; }

		/// Gets the next Scene in play that is owned by a specific player.
		/// @param metaPlayer The player to get the next owned Scene of.
		/// @param pScene The Scene to start searching from in the current roster of Scenes, OWNERSHIP IS NOT TRANSFERRED! (default: 0)
		/// @return A pointer to the next Scene found in the sequence. OWNERSHIP IS NOT TRANSFERRED!
		const Scene* GetNextSceneOfPlayer(int metaPlayer, const Scene* pScene = 0) const;

		/// Gets the total number of brains that a player has, including ones of
		/// his that are resident down on sites.
		/// @param metaPlayer The metagame player to get the total brain count from.
		/// @param countPoolsOnly Whether to only count the brains in the pools, or to also include all (default: false)
		/// resident brains as well.
		/// @return The total number of brains that belong to the metagame player.
		int GetTotalBrainCountOfPlayer(int metaPlayer, bool countPoolsOnly = false) const;

		/// Gets the total gold funds of all the players of a specific team combined.
		/// @param team The metagame team to get the total gold funds of.
		/// @return The total amount of ounces of gold this team has.
		int GetGoldCountOfTeam(int team) const;

		/// Gets the total number of bases that any specific team owns.
		/// @param team The team to get the scene/site ownership count of.
		/// @return The count of scenes owned by this team.
		int GetSceneCountOfTeam(int team) const;

		/// Gets the total number of brains that a team has, including ones that
		/// are resident down on sites.
		/// @param team The metagame team to get the total brain count from.
		/// @param countPoolsOnly Whether to only count the brains in the pools, or to also include all (default: false)
		/// resident brains as well.
		/// @return The total number of brains that belong to the metagame team.
		int GetTotalBrainCountOfTeam(int team, bool countPoolsOnly = false) const;

		/// Indicates which team, if any, is the only one left with brains in its
		/// pool.
		/// @return Which team, if any, is the sole remaining with any brains left in its
		/// players' brain pools.
		int OnlyTeamWithAnyBrainPoolLeft();

		/// Indicates whether there are no brains left in any active player's pool
		/// at all. This does NOT count deployed brain in bases.
		/// @return Whether there are no brains left in any player's brain pool.
		bool NoBrainsLeftInAnyPool();

		/// Indicates which single team has the most owned bases, and if there's a
		/// tie between two teams, total owned gold funds is used as a tiebreaker.
		/// @return Which team is currently in the lead.
		int WhichTeamIsLeading();

		/// Gets the total income from all scenes owned by a specific metaPlayer.
		/// @param metaPlayer The metagame player to get the total scene income from.
		/// @return The amount of income, in oz, the player made this round from its scenes.
		float GetSceneIncomeOfPlayer(int metaPlayer) const;

		/// Gets the ratio of funds already allocated to budgets of this player.
		/// @param metaPlayer The metagame player to get the budget ratio of.
		/// @param pException A scene to exclude from the tally, if any. (default: 0)
		/// @param includeOffensive Whether to count the money allocated for offensive action. (default: true)
		/// @param includeDefensive Whether to count the money allocated for defensive actions. (default: true)
		/// @return The amount, in ratio, that this player already has allocated.
		float GetBudgetedRatioOfPlayer(int metaPlayer, const Scene* pException = 0, bool includeOffensive = true, bool includeDefensive = true) const;

		/// Gets the count of funds still unbudgeted and available of a player.
		/// @param metaPlayer The metagame player to get the unallocated funds of.
		/// @param pException A scene to exclude from the tally, if any. (default: 0)
		/// @param deductOffensive Whether to count the money allocated for offensive action as remaining. (default: false)
		/// @param deductDefensive Whether to count the money allocated for defensive action as remaining. (default: false) const { return m_Players[metaPlayer].GetFunds() - m_Players[metaPlayer].GetFunds() * GetBudgetedRatioOfPlayer(metaPlayer)
		/// @return The amount, in oz, that this player unallocated and unused this turn.
		float GetRemainingFundsOfPlayer(int metaPlayer, const Scene* pException = 0, bool deductOffensive = false, bool deductDefensive = false) const { return m_Players[metaPlayer].GetFunds() - m_Players[metaPlayer].GetFunds() * GetBudgetedRatioOfPlayer(metaPlayer, pException, !deductOffensive, !deductDefensive); }

		/// Shows whether a game is currently in progress
		/// @return Whether a game is going or not.
		bool GameInProgress() { return m_GameState >= GAMEINTRO && m_GameState <= ENDROUND; }

		/// Shows whether game is suspended or not.
		/// @return Whether suspended or not.
		bool IsSuspended() { return m_Suspended; }

		/// Suspends or unsuspends the game so exclusive GUIs and menus can be
		/// shown.
		/// @param suspend Whether to suspend or not.
		void SetSuspend(bool suspend);

		/// Checks wheter a certain player index is valid for the current game
		/// @return Whether the player index passed in is active for the current game.
		bool IsActivePlayer(int metaPlayer) { return metaPlayer >= Players::PlayerOne && metaPlayer < static_cast<int>(m_Players.size()); }

		/// Checks wheter a certain team index is valid for the current game
		/// @return Whether the team index passed in is active for the current game.
		bool IsActiveTeam(int team) { return team >= Activity::TeamOne && team < m_TeamCount; }

		/// Checks whether one team has ownership of all revealed sites.
		/// @return Which team has all sites, if any. If not NoTeam is returned.
		int WhichTeamOwnsAllSites();

		/// Checks for game over condition
		/// @return Whether the game over conditions have been met
		bool IsGameOver();

		/// Shows whether the game has been saved and no data loss will result
		/// if program is quit right now.
		/// @return Whether the game is saved.
		bool GameIsSaved() { return m_GameSaved || m_GameState <= NOGAME || m_GameState >= GAMEOVER; }

		/// Yields a set of ALL eligible Scene presets for a new game.
		/// @param pScenes The list to fill with all the eligible presets. If no list is passed (default: 0)
		/// it will be ignored. Presets returned in list are NOT OWNED there.
		/// @return The count of total number preset scenes eligible for gameplay.
		int TotalScenePresets(std::list<Scene*>* pScenes = 0);

		/// Yields a set of randomly selected Scene presets for a new game.
		/// @param gameSize The size of the set.
		/// @param pSelected The list to fill with the selected presets, depending on currently (default: 0)
		/// set player numbers and loaded eligible scenes. If no list is passed
		/// it will be ignored. Presets returned in list are NOT OWNED there.
		/// @return The count of selected preset scenes.
		int SelectScenePresets(int gameSize, std::list<Scene*>* pSelected = 0);

		/// Clears out all the lined-up activities for the current round.
		void ClearActivities();

		/// Does all the things an AI metaPlayer needs to do during his turn.
		/// @param metaPlayer Which AI metaPlayer we're going to process.
		void AIPlayerTurn(int metaPlayer);

		/// Updates the state of this and the current Metagame. Supposed to be
		/// done every frame before drawing.
		void Update();

		/// Draws this MetaMan's current graphical representation to a BITMAP of choice.
		/// @param pTargetBitmap A pointer to a BITMAP to draw on.
		/// @param targetPos The absolute position of the target bitmap's upper left corner in the scene. (default: Vector())
		void Draw(BITMAP* pTargetBitmap, const Vector& targetPos = Vector());

		/// Protected member variable and method declarations
	protected:
		// GUI controller, owned
		MetagameGUI* m_pMetaGUI;

		// Current Metagame state
		int m_GameState;
		// Whether the state just changed
		bool m_StateChanged;
		// Whether the game is currently suspended (e.g. in the menu)
		bool m_Suspended;
		// Whether game has been saved since the last suspension of it
		bool m_GameSaved;

		// The save name of the currently played metagame
		std::string m_GameName;
		// The players of the metagame
		std::vector<MetaPlayer> m_Players;
		// The number of Team:s in play this game
		int m_TeamCount;
		// The flag icons of all teams
		Icon m_TeamIcons[Activity::MaxTeamCount];
		// The current round the game is on, starting with count on 0
		int m_CurrentRound;
		// All Scenes of the current game, OWNED by this. Stored sequentially in order of revealing
		std::vector<Scene*> m_Scenes;
		// How many of the scenes have been revealed so far - the whole number. It's a float to increase it slower than once a round
		float m_RevealedScenes;
		// How many scenes to reveal each round.. can be a fractional that adds up over several days
		float m_RevealRate;
		// Any extra reveals to make next reveal phase
		float m_RevealExtra;

		// The Activities generated by the current round's offensive maneuvers
		std::vector<GAScripted*> m_RoundOffensives;
		// The current offensive action that we're about to play next
		unsigned int m_CurrentOffensive;
		// Game difficulty
		int m_Difficulty;
		// Teams AI Skill
		int m_TeamAISkill[Activity::MaxTeamCount];

		// Timer for measuring how long each phase has gone for
		Timer m_PhaseTimer;

		/// Private member variable and method declarations
	private:
		static const std::string c_ClassName; //!< A string with the friendly-formatted type name of this object.

		/// Clears all the member variables of this MetaMan, effectively
		/// resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		MetaMan(const MetaMan& reference) = delete;
		MetaMan& operator=(const MetaMan& rhs) = delete;
	};
} // namespace RTE
