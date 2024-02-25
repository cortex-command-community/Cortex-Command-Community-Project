#pragma once

#include "Constants.h"
#include "DynamicSong.h"
#include "Entity.h"
#include "Timer.h"
#include "Vector.h"
#include "Singleton.h"

#define g_MusicMan MusicMan::Instance()

namespace RTE {

	class MusicMan : public Singleton<MusicMan> {

	public:
		/// Hardcoded music types for things like the main menu.
		enum HardcodedMusicTypes {
			IntroMusic = 0,
			MainMenuMusic = 1,
			ScenarioMenuMusic = 2
		};

#pragma region Creation
		/// Constructor method used to instantiate a MusicMan object in system memory.
		/// Create() should be called before using the object.
		MusicMan();

		/// Makes the MusicMan object ready for use.
		/// @return Whether the muic system was initialized successfully.
		bool Initialize();

		// Makes the hardcoded music ready for use.
		// @return Whether the SoundContainers were successfully found or not.
		bool InitializeHardcodedSoundContainers();
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

#pragma region Music Getters and Setters
		/// Gets the hardcoded intro music SoundContainer for the game.
		/// @return A pointer to the SoundContainer.
		SoundContainer* GetHardcodedIntroMusic() const { return m_IntroMusicSoundContainer; }

		/// Gets the hardcoded main menu music SoundContainer for the game.
		/// @return A pointer to the SoundContainer.
		SoundContainer* GetHardcodedMainMenuMusic() const { return m_MainMenuMusicSoundContainer; }

		/// Gets the hardcoded scenario menu music SoundContainer for the game.
		/// @return A pointer to the SoundContainer.
		SoundContainer* GetHardcodedScenarioMenuMusic() const { return m_ScenarioMenuMusicSoundContainer; }
#pragma endregion

#pragma region Music Handling
		/// Resets music state, stopping and clearing playing dynamic songs or interrupting music, etc. to make ready for new music.
		void ResetMusicState();

		/// Begins playing a new dynamic song, optionally from a specific section type.
		/// @param songName PresetName of the dynamic song to play.
		/// @param songSectionType Type of DynamicSongSection to play first.
		/// @param playImmediately Whether to immediately play this song or wait for the current music piece to end.
		/// @param playTransition Whether to play the TransitionSoundContainer of the upcoming section or not.
		/// @return Whether the song was successfully started or not.
		bool PlayDynamicSong(std::string songName, std::string songSectionType = "Default", bool playImmediately = false, bool playTransition);

		/// Switches the next SongSection queued to play.
		/// @param songSectionType Next SongSectionType to play.
		/// @param playImmediately Whether to immediately play the new SongSectionType or not.
		/// @param playTransition Whether to play the TransitionSoundContainer of the upcoming section or not.
		bool SetNextDynamicSongSection(std::string songSectionType, bool playImmediately, bool playTransition = true);

		/// Plays the next queued SoundContainer and prepares a new one.
		/// @param fadeOutCurrent Whether to fade out the current playing SoundContainer or let it play out.
		/// @return Whether the function completed successfully or not.
		bool CyclePlayingSoundContainers(bool fadeOutCurrent = true);

		/// Sets the current playing music to end, disabling further playback of new music.
		/// @param Whether to also fade out the current playing music or not.
		/// @return True if this was not set to end music previously, false if it already was.
		bool EndMusic(bool fadeOutCurrent = false);

		/// Function to interrupt other music and play a type of hardcoded music.
		/// @param hardcodedMusicType The type of hardcoded music to play.
		void PlayInterruptingMusic(SoundContainer* soundContainer);

		/// Signals the end of hardcoded music, resuming other music if needed.
		void EndInterruptingMusic();
#pragma endregion

	protected:
		bool m_IsPlayingDynamicMusic; //!< Whether this is actively playing dynamic music or not.
		bool m_HardcodedSoundContainersInitialized; //!< Whether we have initialized base hardcoded SoundContainers or not.

		SoundContainer* m_IntroMusicSoundContainer; //!< SoundContainer for hardcoded intro music.
		SoundContainer* m_MainMenuMusicSoundContainer; //!< SoundContainer for hardcoded main menu music.
		SoundContainer* m_ScenarioMenuMusicSoundContainer; //!< SoundContainer for hardcoded scenario menu music.

		SoundContainer* m_InterruptingMusicSoundContainer; //!< Current interrupting music being played.

		DynamicSong* m_CurrentSong; //!< The current DynamicSong being played.
		std::string m_CurrentSongSectionType; //!< The current type of DynamicSongSection we are trying to play.
		DynamicSongSection* m_CurrentSongSection; //!< The current DynamicSongSection we are actually playing.

		SoundContainer* m_OldSoundContainer; //!< The previous SoundContainer that was played as music.
		SoundContainer* m_CurrentSoundContainer; //!< The current selected SoundContainer playing as music.
		SoundContainer* m_NextSoundContainer; //!< The next selected SoundContainer to play as music.

		Timer m_MusicTimer; //!< Timer for musical horizontal sequencing.
		double m_MusicPausedTime; //!< Elapsed MusicTimer time when we last paused, used to resync on unpause.
		bool m_ReturnToDynamicMusic; //!< Signifier that this was playing dynamic music before being interrupted.

	private:
#pragma region Internal Music Handling
		/// Sets the current SoundContainer that should be playing.
		/// @param newSoundContainer New SoundContainer that should be playing.
		void SetCurrentSoundContainer(SoundContainer* newSoundContainer) { m_CurrentSoundContainer = newSoundContainer; }

		/// Sets the next SoundContainer that should play after the current one.
		/// @param newSoundContainer New SoundContainer that should play next.
		void SetNextSoundContainer(SoundContainer* newSoundContainer) { m_NextSoundContainer = newSoundContainer; }

		/// Sets the current SongSectionType this wants to play.
		/// @param newSongSectionType New SongSectionType for this to want to play.
		void SetCurrentSongSectionType(std::string newSongSectionType) { m_CurrentSongSectionType = newSongSectionType; }

		/// Selects and sets the next SongSection based on the current SongSectionType.
		void SelectNextSongSection();

		/// Selects and sets the next SoundContainer from the current SongSection.
		void SelectNextSoundContainer(bool selectTransition = false);
#pragma endregion

		/// Clears all the member variables of this MusicMan, effectively resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		MusicMan(const MusicMan& reference) = delete;
		MusicMan& operator=(const MusicMan& rhs) = delete;
	};
} // namespace RTE