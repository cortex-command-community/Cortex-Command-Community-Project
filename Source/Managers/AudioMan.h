#pragma once

#include "Constants.h"
#include "Entity.h"
#include "Timer.h"
#include "Vector.h"
#include "Singleton.h"

#include "fmod/fmod.hpp"
#include "fmod/fmod_errors.h"

#define g_AudioMan AudioMan::Instance()

namespace RTE {

	class SoundContainer;

	/// The singleton manager of sound effect and music playback.
	class AudioMan : public Singleton<AudioMan> {
		friend class SettingsMan;
		friend class SoundContainer;

	public:
		/// Hardcoded playback priorities for sounds. Note that sounds don't have to use these specifically; their priority can be anywhere between high and low.
		enum PlaybackPriority {
			PRIORITY_HIGH = 0,
			PRIORITY_NORMAL = 128,
			PRIORITY_LOW = 256
		};

		/// Music event states for sending music data from the server to clients during multiplayer games.
		enum NetworkMusicState {
			MUSIC_PLAY = 0,
			MUSIC_STOP,
			MUSIC_SILENCE,
			MUSIC_SET_PITCH
		};

		/// The data struct used to send music data from the server to clients during multiplayer games.
		struct NetworkMusicData {
			unsigned char State;
			char Path[256];
			int LoopsOrSilence;
			float Position;
			float Pitch;
		};

		/// Sound event states for sending sound data from the server to clients during multiplayer games.
		enum NetworkSoundState {
			SOUND_SET_GLOBAL_PITCH = 0,
			SOUND_PLAY,
			SOUND_STOP,
			SOUND_SET_POSITION,
			SOUND_SET_VOLUME,
			SOUND_SET_CUSTOMPANVALUE,
			SOUND_SET_PANNINGSTRENGTHMULTIPLIER,
			SOUND_SET_PITCH,
			SOUND_FADE_OUT
		};

		/// The data struct used to send sound data from the server to clients during multiplayer games.
		struct NetworkSoundData {
			unsigned char State;
			std::size_t SoundFileHash;
			int Channel;
			bool Immobile;
			float AttenuationStartDistance;
			float CustomPanValue;
			float PanningStrengthMultiplier;
			int Loops;
			int Priority;
			bool AffectedByGlobalPitch;
			float Position[2];
			float Volume;
			float Pitch;
			int FadeOutTime;
		};

#pragma region Creation
		/// Constructor method used to instantiate a AudioMan object in system memory.
		/// Create() should be called before using the object.
		AudioMan();

		/// Makes the AudioMan object ready for use.
		/// @return Whether the audio system was initialized successfully. If not, no audio will be available.
		bool Initialize();
#pragma endregion

#pragma region Destruction
		/// Destructor method used to clean up a AudioMan object before deletion from system memory.
		~AudioMan();

		/// Destroys and resets (through Clear()) the AudioMan object.
		void Destroy();
#pragma endregion

#pragma region Concrete Methods
		/// Updates the state of this AudioMan. Supposed to be done every frame before drawing.
		void Update();
#pragma endregion

#pragma region General Getters and Setters
		/// Gets the audio management system object used for playing all audio.
		/// @return The audio management system object used by AudioMan for playing audio.
		FMOD::System* GetAudioSystem() const { return m_AudioSystem; }

		/// Reports whether audio is enabled.
		/// @return Whether audio is enabled.
		bool IsAudioEnabled() const { return m_AudioEnabled; }

		/// Gets the virtual and real playing channel counts, filling in the passed-in out-parameters.
		/// @param outVirtualChannelCount The out-parameter that will hold the virtual channel count.
		/// @param outRealChannelCount The out-parameter that will hold the real channel count.
		/// @return Whether or not the playing channel count was succesfully gotten.
		bool GetPlayingChannelCount(int* outVirtualChannelCount, int* outRealChannelCount) const { return m_AudioSystem->getChannelsPlaying(outVirtualChannelCount, outRealChannelCount) == FMOD_OK; }

		/// Returns the total number of virtual audio channels available.
		/// @return The number of virtual audio channels available.
		int GetTotalVirtualChannelCount() const { return c_MaxVirtualChannels; }

		/// Returns the total number of real audio channels available.
		/// @return The number of real audio channels available.
		int GetTotalRealChannelCount() const {
			int channelCount;
			return m_AudioSystem->getSoftwareChannels(&channelCount) == FMOD_OK ? channelCount : 0;
		}

		/// Gets whether all audio is muted or not.
		/// @return Whether all the audio is muted or not.
		bool GetMasterMuted() const { return m_MuteMaster; }

		/// Mutes or unmutes all audio.
		/// @param muteOrUnmute Whether to mute or unmute all the audio.
		void SetMasterMuted(bool muteOrUnmute = true) {
			m_MuteMaster = muteOrUnmute;
			if (m_AudioEnabled) {
				m_MasterChannelGroup->setMute(m_MuteMaster);
			}
		}

		/// Gets the volume of all audio. Does not get music or sounds individual volumes.
		/// @return Current volume scalar value. 0.0-1.0.
		float GetMasterVolume() const { return m_MasterVolume; }

		/// Sets all the audio to a specific volume. Does not affect music or sounds individual volumes.
		/// @param volume The desired volume scalar. 0.0-1.0.
		void SetMasterVolume(float volume = 1.0F) {
			m_MasterVolume = std::clamp(volume, 0.0F, 1.0F);
			if (m_AudioEnabled) {
				m_MasterChannelGroup->setVolume(m_MasterVolume);
			}
		}

		/// Gets the global pitch scalar value for all sounds and music.
		/// @return The current pitch scalar. Will be > 0.
		float GetGlobalPitch() const { return m_GlobalPitch; }

		/// Sets the global pitch multiplier for mobile sounds, optionally setting it for immobile sounds and music.
		/// @param pitch New global pitch, limited to 8 octaves up or down (i.e. 0.125 - 8). Defaults to 1.
		/// @param includeImmobileSounds Whether to include immobile sounds (normally used for GUI and so on) in global pitch modification. Defaults to false.
		/// @param includeMusic Whether to include the music in global pitch modification. Defaults to false.
		void SetGlobalPitch(float pitch = 1.0F, bool includeImmobileSounds = false, bool includeMusic = false);

		/// The strength of the sound panning effect.
		/// @return 0 - 1, where 0 is no panning and 1 is fully panned.
		float GetSoundPanningEffectStrength() const { return m_SoundPanningEffectStrength; }
#pragma endregion

#pragma region Music Getters and Setters
		/// Reports whether any music stream is currently playing.
		/// @return Whether any music stream is currently playing.
		bool IsMusicPlaying() const {
			bool isPlayingMusic;
			return m_AudioEnabled && m_MusicChannelGroup->isPlaying(&isPlayingMusic) == FMOD_OK ? isPlayingMusic : false;
		}

		/// Gets whether the music channel is muted or not.
		/// @return Whether the music channel is muted or not.
		bool GetMusicMuted() const { return m_MuteMusic; }

		/// Mutes or unmutes the music channel.
		/// @param muteOrUnmute Whether to mute or unmute the music channel.
		void SetMusicMuted(bool muteOrUnmute = true) {
			m_MuteMusic = muteOrUnmute;
			if (m_AudioEnabled) {
				m_MusicChannelGroup->setMute(m_MuteMusic);
			}
		}

		/// Gets the volume of music. Does not get volume of sounds.
		/// @return Current volume scalar value. 0.0-1.0.
		float GetMusicVolume() const { return m_MusicVolume; }

		/// Sets the music to a specific volume. Does not affect sounds.
		/// @param volume The desired volume scalar. 0.0-1.0.
		void SetMusicVolume(float volume = 1.0F) {
			m_MusicVolume = std::clamp(volume, 0.0F, 1.0F);
			if (m_AudioEnabled) {
				m_MusicChannelGroup->setVolume(m_MusicVolume);
			}
		}

		/// Sets the music to a specific volume, but it will only last until a new song is played. Useful for fading etc.
		/// @param volume The desired volume scalar. 0.0-1.0.
		void SetTempMusicVolume(float volume = 1.0F);

		/// Gets the path of the last played music stream.
		/// @return The file path of the last played music stream.
		std::string GetMusicPath() const { return m_MusicPath; }

		/// Sets/updates the frequency/pitch for the music channel.
		/// @param pitch New pitch, a multiplier of the original normal frequency. Keep it > 0.
		/// @return Whether the music channel's pitch was successfully updated.
		bool SetMusicPitch(float pitch);

		/// Gets the position of playback of the current music stream, in seconds.
		/// @return The current position of the current stream playing, in seconds.
		float GetMusicPosition() const;

		/// Sets the music to a specific position in the song.
		/// @param position The desired position from the start, in seconds.
		void SetMusicPosition(float position);
#pragma endregion

#pragma region Overall Sound Getters and Setters
		/// Gets whether all the sound effects channels are muted or not.
		/// @return Whether all the sound effects channels are muted or not.
		bool GetSoundsMuted() const { return m_MuteSounds; }

		/// Mutes or unmutes all the sound effects channels.
		/// @param muteOrUnmute Whether to mute or unmute all the sound effects channels.
		void SetSoundsMuted(bool muteOrUnmute = true) {
			m_MuteSounds = muteOrUnmute;
			if (m_AudioEnabled) {
				// TODO: We may or may not wanna separate this out and add a UI sound slider
				m_SFXChannelGroup->setMute(m_MuteSounds);
				m_UIChannelGroup->setMute(m_MuteSounds);
			}
		}

		/// Gets the volume of all sounds. Does not get volume of music.
		/// @return Current volume scalar value. 0.0-1.0.
		float GetSoundsVolume() const { return m_SoundsVolume; }

		/// Sets the volume of all sounds to a specific volume. Does not affect music.
		/// @param volume The desired volume scalar. 0.0-1.0.
		void SetSoundsVolume(float volume = 1.0F) {
			m_SoundsVolume = volume;
			if (m_AudioEnabled) {
				m_SFXChannelGroup->setVolume(m_SoundsVolume);
				m_UIChannelGroup->setVolume(m_SoundsVolume);
			}
		}
#pragma endregion

#pragma region Global Playback and Handling
		/// Stops all playback and clears the music playlist.
		void StopAll() {
			if (m_AudioEnabled) {
				m_MasterChannelGroup->stop();
			}
			m_MusicPlayList.clear();
		}

		/// Makes all sounds that are looping stop looping, allowing them to play once more then be finished.
		void FinishIngameLoopingSounds();

		/// Pauses all ingame sounds.
		/// @param pause Whether to pause sounds or resume them.
		void PauseIngameSounds(bool pause = true) {
			if (m_AudioEnabled) {
				m_SFXChannelGroup->setPaused(pause);
			}
		}
#pragma endregion

#pragma region Music Playback and Handling
		/// Starts playing a certain WAVE, MOD, MIDI, OGG, MP3 file in the music channel.
		/// @param filePath The path to the music file to play.
		/// @param loops The number of times to loop the song. 0 means play once. -1 means play infinitely until stopped.
		/// @param volumeOverrideIfNotMuted The volume override for music for this song only, if volume is not muted. < 0 means no override.
		void PlayMusic(const char* filePath, int loops = -1, float volumeOverrideIfNotMuted = -1.0F);

		/// Plays the next music stream in the queue, if any is queued.
		void PlayNextStream();

		/// Stops playing a the music channel.
		void StopMusic();

		/// Queues up another path to a stream that will be played after the current one is done.
		/// Can be done several times to queue up many tracks. The last track in the queue will be looped infinitely.
		/// @param filePath The path to the music file to play after the current one.
		void QueueMusicStream(const char* filePath);

		/// Queues up a period of silence in the music stream playlist.
		/// @param seconds The number of secs to wait before going to the next stream.
		void QueueSilence(int seconds) {
			if (m_AudioEnabled && seconds > 0) {
				m_MusicPlayList.push_back("@" + std::to_string(seconds));
			}
		}

		/// Clears the music queue.
		void ClearMusicQueue() { m_MusicPlayList.clear(); }

		/// Sets the music muffled state.
		/// @param musicMuffledState The new music muffled state to set.
		/// @return Whether the new music muffled state was successfully set.
		FMOD_RESULT SetMusicMuffledState(bool musicMuffledState);
#pragma endregion

#pragma region Lua Sound File Playing
		/// Starts playing a certain sound file.
		/// @param filePath The path to the sound file to play.
		/// @return The new SoundContainer being played. OWNERSHIP IS TRANSFERRED!
		SoundContainer* PlaySound(const std::string& filePath) { return PlaySound(filePath, Vector(), -1); }

		/// Starts playing a certain sound file at a certain position for all players.
		/// @param filePath The path to the sound file to play.
		/// @return The new SoundContainer being played. OWNERSHIP IS TRANSFERRED!
		SoundContainer* PlaySound(const std::string& filePath, const Vector& position) { return PlaySound(filePath, position, -1); }

		/// Starts playing a certain sound file at a certain position for a certain player.
		/// @param filePath The path to the sound file to play.
		/// @param position The position at which to play the SoundContainer's sounds.
		/// @param player Which player to play the SoundContainer's sounds for, -1 means all players.
		/// @return The new SoundContainer being played. OWNERSHIP IS TRANSFERRED!
		SoundContainer* PlaySound(const std::string& filePath, const Vector& position, int player);
#pragma endregion

#pragma region Network Audio Handling
		/// Returns true if manager is in multiplayer mode.
		/// @return True if in multiplayer mode.
		bool IsInMultiplayerMode() const { return m_IsInMultiplayerMode; }

		/// Sets the multiplayer mode flag.
		/// @param value Whether this manager should operate in multiplayer mode.
		void SetMultiplayerMode(bool value) { m_IsInMultiplayerMode = value; }

		/// Fills the list with music events happened for the specified network player.
		/// @param player Player to get events for.
		/// @param list List with events for this player.
		void GetMusicEvents(int player, std::list<NetworkMusicData>& list);

		/// Adds the music event to internal list of music events for the specified player.
		/// @param player Player(s) for which the event happened.
		/// @param state NetworkMusicState for the event.
		/// @param filepath Music file path to transmit to client.
		/// @param loops LoopsOrSilence counter or, if state is silence, the length of the silence.
		/// @param position Music playback position.
		/// @param pitch Pitch value.
		void RegisterMusicEvent(int player, NetworkMusicState state, const char* filepath, int loopsOrSilence = 0, float position = 0, float pitch = 1.0F);

		/// Clears the list of current Music events for the target player.
		/// @param player Player to clear music events for. -1 clears for all players
		void ClearMusicEvents(int player);

		/// Fills the list with sound events happened for the specified network player.
		/// @param player Player to get events for.
		/// @param list List with events for this player.
		void GetSoundEvents(int player, std::list<NetworkSoundData>& list);

		/// Adds the sound event to the internal list of sound events for the specified player.
		/// @param player Player(s) for which the event happened.
		/// @param state NetworkSoundState for the event.
		/// @param soundContainer A pointer to the SoundContainer this event is happening to, or a null pointer for global events.
		/// @param fadeOutTime THe amount of time, in MS, to fade out over. This data isn't contained in SoundContainer, so it needs to be passed in separately.
		void RegisterSoundEvent(int player, NetworkSoundState state, const SoundContainer* soundContainer, int fadeOutTime = 0);

		/// Clears the list of current Sound events for the target player.
		/// @param player Player to clear sound events for. -1 clears for all players.
		void ClearSoundEvents(int player);
#pragma endregion

	protected:
		const FMOD_VECTOR c_FMODForward = FMOD_VECTOR{0, 0, 1}; //!< An FMOD_VECTOR defining the Forwards direction. Necessary for 3D Sounds.
		const FMOD_VECTOR c_FMODUp = FMOD_VECTOR{0, 1, 0}; //!< An FMOD_VECTOR defining the Up direction. Necessary for 3D Sounds.

		FMOD::System* m_AudioSystem; //!< The FMOD Sound management object.
		FMOD::ChannelGroup* m_MasterChannelGroup; //!< The top-level FMOD ChannelGroup that holds everything.
		FMOD::ChannelGroup* m_SFXChannelGroup; //!< The FMOD ChannelGroup for diegetic gameplay sounds.
		FMOD::ChannelGroup* m_UIChannelGroup; //!< The FMOD ChannelGroup for UI sounds.
		FMOD::ChannelGroup* m_MusicChannelGroup; //!< The FMOD ChannelGroup for music.

		bool m_AudioEnabled; //!< Bool to tell whether audio is enabled or not.
		std::vector<std::unique_ptr<const Vector>> m_CurrentActivityHumanPlayerPositions; //!< The stored positions of each human player in the current activity. Only filled when there's an activity running.
		std::unordered_map<int, float> m_SoundChannelMinimumAudibleDistances; //!<  An unordered map of sound channel indices to floats representing each Sound Channel's minimum audible distances. This is necessary to keep safe data in case the SoundContainer is destroyed while the sound is still playing, as happens often with TDExplosives.

		bool m_MuteMaster; //!< Whether all the audio is muted.
		bool m_MuteMusic; //!< Whether the music channel is muted.
		bool m_MuteSounds; //!< Whether all the sound effects channels are muted.
		float m_MasterVolume; //!< Global volume of all audio.
		float m_MusicVolume; //!< Global music volume.
		float m_SoundsVolume; //!< Global sounds effects volume.
		float m_GlobalPitch; //!< Global pitch multiplier.

		float m_SoundPanningEffectStrength; //!< The strength of the sound panning effect, 0 (no panning) - 1 (full panning).

		//////////////////////////////////////////////////
		// TODO These need to be removed when our soundscape is sorted out. They're only here temporarily to allow for easier tweaking by pawnis.
		float m_ListenerZOffset;
		float m_MinimumDistanceForPanning;
		//////////////////////////////////////////////////

		bool m_MusicMuffled; //!< Whether the music bus is muffled.
		std::string m_MusicPath; //!< The path to the last played music stream.
		std::list<std::string> m_MusicPlayList; //!< Playlist of paths to music to play after the current non looping one is done.
		Timer m_SilenceTimer; //!< Timer for measuring silences between songs.

		bool m_IsInMultiplayerMode; //!< If true then the server is in multiplayer mode and will register sound and music events into internal lists.
		std::list<NetworkSoundData> m_SoundEvents[c_MaxClients]; //!< Lists of per player sound events.
		std::list<NetworkMusicData> m_MusicEvents[c_MaxClients]; //!< Lists of per player music events.

		std::mutex g_SoundEventsListMutex[c_MaxClients]; //!< A list for locking sound events for multiplayer to avoid race conditions and other such problems.
		std::mutex m_SoundChannelMinimumAudibleDistancesMutex; //!, As above but for m_SoundChannelMinimumAudibleDistances

	private:
#pragma region Sound Container Actions and Modifications
		/// Starts playing the next SoundSet of the given SoundContainer for the give player.
		/// @param soundContainer Pointer to the SoundContainer to start playing. Ownership is NOT transferred!
		/// @param player Which player to play the SoundContainer's sounds for, -1 means all players. Defaults to -1.
		/// @return Whether or not playback of the Sound was successful.
		bool PlaySoundContainer(SoundContainer* soundContainer, int player = -1);

		/// Sets/updates the position of a SoundContainer's playing sounds.
		/// @param soundContainer A pointer to a SoundContainer object. Ownership IS NOT transferred!
		/// @return Whether the position was successfully set.
		bool ChangeSoundContainerPlayingChannelsPosition(const SoundContainer* soundContainer);

		/// Changes the volume of a SoundContainer's playing sounds.
		/// @param soundContainer A pointer to a SoundContainer object. Ownership IS NOT transferred!
		/// @param newVolume The new volume to play sounds at, between 0 and 1.
		/// @return Whether the volume was successfully updated.
		bool ChangeSoundContainerPlayingChannelsVolume(const SoundContainer* soundContainer, float newVolume);

		/// Changes the frequency/pitch of a SoundContainer's playing sounds.
		/// @param soundContainer A pointer to a SoundContainer object. Ownership IS NOT transferred!
		/// @return Whether the pitch was successfully updated.
		bool ChangeSoundContainerPlayingChannelsPitch(const SoundContainer* soundContainer);

		/// Updates the custom pan value of a SoundContainer's playing sounds.
		/// @param soundContainer A pointer to a SoundContainer object. Ownership IS NOT transferred!
		/// @return Whether the custom pan value was successfully updated.
		bool ChangeSoundContainerPlayingChannelsCustomPanValue(const SoundContainer* soundContainer);

		/// Stops playing a SoundContainer's playing sounds for a certain player.
		/// @param soundContainer A pointer to a SoundContainer object6. Ownership is NOT transferred!
		/// @param player Which player to stop playing the SoundContainer for.
		/// @return
		bool StopSoundContainerPlayingChannels(SoundContainer* soundContainer, int player);

		/// Fades out playback a SoundContainer.
		/// @param soundContainer A pointer to a SoundContainer object. Ownership is NOT transferred!
		/// @param fadeOutTime The amount of time, in ms, to fade out over.
		void FadeOutSoundContainerPlayingChannels(SoundContainer* soundContainer, int fadeOutTime);
#pragma endregion

#pragma region 3D Effect Handling
		/// Updates 3D effects calculations for all sound channels whose SoundContainers isn't immobile.
		void Update3DEffectsForSFXChannels();

		/// Sets or updates the position of the given sound channel so it handles scene wrapping correctly. Also handles volume attenuation and minimum audible distance.
		/// @param soundChannel The channel whose position should be set or updated.
		/// @param positionToUse An optional position to set for this sound channel. Done this way to save setting and resetting data in FMOD.
		/// @return Whether the channel's position was succesfully set.
		FMOD_RESULT UpdatePositionalEffectsForSoundChannel(FMOD::Channel* soundChannel, const FMOD_VECTOR* positionToUse = nullptr) const;
#pragma endregion

#pragma region FMOD Callbacks
		/// A static callback function for FMOD to invoke when the music channel finishes playing. See fmod docs - FMOD_CHANNELCONTROL_CALLBACK for details
		static FMOD_RESULT F_CALLBACK MusicChannelEndedCallback(FMOD_CHANNELCONTROL* channelControl, FMOD_CHANNELCONTROL_TYPE channelControlType, FMOD_CHANNELCONTROL_CALLBACK_TYPE callbackType, void* commandData1, void* commandData2);

		/// A static callback function for FMOD to invoke when a sound channel finished playing. See fmod docs - FMOD_CHANNELCONTROL_CALLBACK for details
		static FMOD_RESULT F_CALLBACK SoundChannelEndedCallback(FMOD_CHANNELCONTROL* channelControl, FMOD_CHANNELCONTROL_TYPE channelControlType, FMOD_CHANNELCONTROL_CALLBACK_TYPE callbackType, void* commandData1, void* commandData2);
#pragma endregion

#pragma region Utility Methods
		/// Gets the corresponding FMOD_VECTOR for a given RTE Vector.
		/// @param vector The RTE Vector to get as an FMOD_VECTOR.
		/// @return The FMOD_VECTOR that corresponds to the given RTE Vector.
		FMOD_VECTOR GetAsFMODVector(const Vector& vector, float zValue = 0) const;

		/// Gets the corresponding RTE Vector for a given FMOD_VECTOR.
		/// @param fmodVector The FMOD_VECTOR to get as an RTE Vector.
		/// @return The RTE Vector that corresponds to the given FMOD_VECTOR.
		Vector GetAsVector(FMOD_VECTOR fmodVector) const;
#pragma endregion

		/// Clears all the member variables of this AudioMan, effectively resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		AudioMan(const AudioMan& reference) = delete;
		AudioMan& operator=(const AudioMan& rhs) = delete;
	};
} // namespace RTE
