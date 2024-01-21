#pragma once

#include "Vector.h"
#include "ContentFile.h"
#include "LuaMan.h"

namespace RTE {

	/// Self-contained struct defining an individual sound in a SoundSet.
	struct SoundData {
		ContentFile SoundFile;
		FMOD::Sound* SoundObject;
		Vector Offset = Vector();
		float MinimumAudibleDistance = 0.0F;
		float AttenuationStartDistance = -1.0F;
	};

	/// A set of sounds, and their selection data.
	class SoundSet : public Serializable {
		friend struct EntityLuaBindings;

	public:
		SerializableOverrideMethods;

		/// How the SoundSet should choose the next sound or SoundSet to play when SelectNextSound is called.
		enum SoundSelectionCycleMode {
			RANDOM = 0,
			FORWARDS,
			ALL
		};

#pragma region Creation
		/// Constructor method used to instantiate a SoundSet object in system memory. Create() should be called before using the object.
		SoundSet();

		/// Creates a SoundSet to be identical to another, by deep copy.
		/// @param reference A reference to the SoundSet to deep copy.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(const SoundSet& reference);
#pragma endregion

#pragma region Destruction
		/// Destructor method used to clean up a SoundSet object before deletion from system memory.
		~SoundSet();

		/// Destroys and resets (through Clear()) the SoundSet object.
		void Destroy() { Clear(); }
#pragma endregion

#pragma region INI Handling
		/// Handles reading a SoundData from INI, loading it in as a ContentFile and into FMOD, and reading any of its subproperties.
		/// Does not add the created SoundData to a this SoundSet.
		/// @param reader A Reader lined up to the value of the property to be read.
		/// @return SoundData for the newly read sound.
		static SoundData ReadAndGetSoundData(Reader& reader);

		/// Handles turning a SoundCelectionCycleMode from its user-friendly name in INI to its enum value, using the static SoundSelectionCycleMap.
		/// @param reader A Reader lined up to the value of the property to be read.
		/// @return The appropriate SoundSelectionCycleMode for the given INI value.
		static SoundSelectionCycleMode ReadSoundSelectionCycleMode(Reader& reader);

		/// Handles writing the given SoundSelectionCycleMode out to the given Writer, using the static SoundSelectionCycleMap.
		/// @param writer A Writer filled in with the property to write to.
		/// @param soundSelectionCycleMode The SoundSelectionCycleMode to write.
		static void SaveSoundSelectionCycleMode(Writer& writer, SoundSelectionCycleMode soundSelectionCycleMode);
#pragma endregion

#pragma region SoundData and SoundSet Addition
		/// Adds a new sound to this SoundSet, spitting out a Lua error if it fails. The sound will have default configuration.
		/// @param soundFilePath A path to the new sound to add. This will be handled through PresetMan.
		void AddSound(const std::string& soundFilePath) { AddSound(soundFilePath, false); }

		/// Adds a new sound to this SoundSet, either spitting out a Lua error or aborting if it fails. The sound will have default configuration.
		/// @param soundFilePath A path to the new sound to add. This will be handled through PresetMan.
		/// @param abortGameForInvalidSound Whether to abort the game if the sound couldn't be added, or just show a console error.
		void AddSound(const std::string& soundFilePath, bool abortGameForInvalidSound) { AddSound(soundFilePath, Vector(), 0, -1, abortGameForInvalidSound); }

		/// Adds a new sound to this SoundSet, spitting out a Lua error if it fails. The sound will be configured based on parameters.
		/// @param soundFilePath A path to the new sound to add. This will be handled through PresetMan.
		/// @param offset The offset position to play this sound at, where (0, 0) is no offset.
		/// @param minimumAudibleDistance The minimum distance at which this sound will be audible. 0 means there is none, which is normally the case.
		/// @param attenuationStartDistance The attenuation start distance for this sound, -1 sets it to default.
		void AddSound(const std::string& soundFilePath, const Vector& offset, float minimumAudibleDistance, float attenuationStartDistance) { AddSound(soundFilePath, offset, minimumAudibleDistance, attenuationStartDistance, false); }

		/// Adds a new sound to this SoundSet, either spitting out a Lua error or aborting if it fails. The sound will be configured based on parameters.
		/// @param soundFilePath A path to the new sound to add. This will be handled through PresetMan.
		/// @param offset The offset position to play this sound at, where (0, 0) is no offset.
		/// @param minimumAudibleDistance The minimum distance at which this sound will be audible. 0 means there is none, which is normally the case.
		/// @param attenuationStartDistance The attenuation start distance for this sound, -1 sets it to default.
		/// @param abortGameForInvalidSound Whether to abort the game if the sound couldn't be added, or just show a console error.
		void AddSound(const std::string& soundFilePath, const Vector& offset, float minimumAudibleDistance, float attenuationStartDistance, bool abortGameForInvalidSound);

		/// Removes all instances of the sound with the given filepath from this SoundSet. Does not remove it from any sub-SoundSets.
		/// @param soundFilePath The path to the sound to be removed from this SoundSet.
		/// @return Whether or not a sound with the given filepath was found in this SoundSet.
		bool RemoveSound(const std::string& soundFilePath) { return RemoveSound(soundFilePath, false); }

		/// Removes all instances of the sound with the given filepath from this SoundSet, optionally removing it from all sub-SoundSets as well.
		/// @param soundFilePath The path to the sound to be removed from this SoundSet.
		/// @param removeFromSubSoundSets Whether or not to remove the sound from any sub-SoundSets as well as this SoundSet.
		/// @return Whether or not a sound with the given filepath was found in this SoundSet or, if set to remove from sub-SoundSets, any of its sub-SoundSets.
		bool RemoveSound(const std::string& soundFilePath, bool removeFromSubSoundSets);

		/// Adds a copy of the given SoundData to this SoundSet.
		/// @param soundDataToAdd The SoundData to copy to this SoundSet.
		void AddSoundData(const SoundData& soundDataToAdd) { m_SoundData.push_back(soundDataToAdd); }

		/// Adds a copy of the passed in SoundSet as a sub SoundSet of this SoundSet. Ownership IS transferred!
		/// @param soundSetToAdd A reference to the SoundSet to be copied in as a sub SoundSet of this SoundSet. Ownership IS transferred!
		void AddSoundSet(const SoundSet& soundSetToAdd) { m_SubSoundSets.push_back(soundSetToAdd); }
#pragma endregion

#pragma region Getters and Setters
		/// Shows whether this SoundSet has any sounds in it or, optionally its SubSoundSets.
		/// @return Whether this SoundSet has any sounds, according to the conditions.
		bool HasAnySounds(bool includeSubSoundSets = true) const;

		/// Gets the current SoundSelectionCycleMode for this SoundSet, which is used to determine what SoundSet to select next time SelectNextSounds is called.
		/// @return The current sound selection cycle mode.
		SoundSelectionCycleMode GetSoundSelectionCycleMode() const { return m_SoundSelectionCycleMode; }

		/// Sets the SoundSelectionCycleMode for this SoundSet, which is used to determine what SoundSet to select next time SelectNextSounds is called.
		/// @param newSoundSelectionCycleMOde The new SoundSelectionCycleMode for this SoundSet.
		void SetSoundSelectionCycleMode(SoundSelectionCycleMode newSoundSelectionCycleMode) {
			m_SoundSelectionCycleMode = newSoundSelectionCycleMode;
			if (m_SoundSelectionCycleMode == SoundSelectionCycleMode::FORWARDS) {
				m_CurrentSelection.second = -1;
			}
		}

		/// Fills the passed in vector with the flattened SoundData in the SoundSet, optionally only getting currently selected SoundData.
		/// @param flattenedSoundData A reference vector of SoundData references to be filled with this SoundSet's flattened SoundData.
		/// @param onlyGetSelectedSoundData Whether to only get SoundData that is currently selected, or to get all SoundData in this SoundSet.
		void GetFlattenedSoundData(std::vector<SoundData*>& flattenedSoundData, bool onlyGetSelectedSoundData);

		/// Fills the passed in vector with the flattened SoundData in the SoundSet, optionally only getting currently selected SoundData.
		/// @param flattenedSoundData A reference vector of SoundData references to be filled with this SoundSet's flattened SoundData.
		/// @param onlyGetSelectedSoundData Whether to only get SoundData that is currently selected, or to get all SoundData in this SoundSet.
		void GetFlattenedSoundData(std::vector<const SoundData*>& flattenedSoundData, bool onlyGetSelectedSoundData) const;

		/// Gets the vector of SubSoundSets for this SoundSet.
		/// @return The vector of SubSoundSets for this SoundSet.
		std::vector<SoundSet>& GetSubSoundSets() { return m_SubSoundSets; }
#pragma endregion

#pragma region Miscellaneous
		/// Selects the next sounds of this SoundSet to be played, also selecting them for sub SoundSets as appropriate.
		/// @return False if this SoundSet or any of its sub SoundSets failed to select sounds, or true if everything worked.
		bool SelectNextSounds();
#pragma endregion

#pragma region Class Info
		/// Gets the class name of this Serializable.
		/// @return A string with the friendly-formatted type name of this object.
		const std::string& GetClassName() const override { return m_sClassName; }
#pragma endregion

	private:
		static const std::string m_sClassName; //!< A string with the friendly-formatted type name of this object.
		static const std::unordered_map<std::string, SoundSet::SoundSelectionCycleMode> c_SoundSelectionCycleModeMap; //!< A map of strings to SoundSelectionCycleModes to support string parsing for the SoundCycleMode enum. Populated in the implementing cpp file.

		SoundSelectionCycleMode m_SoundSelectionCycleMode; //!< The SoundSelectionCycleMode for this SoundSet.
		std::pair<bool, int> m_CurrentSelection; //!< Whether the current selection is in the SoundData (false) or SoundSet (true) vector, and its index in the appropriate vector.

		std::vector<SoundData> m_SoundData; //!< The SoundData available for selection in this SoundSet.
		std::vector<SoundSet> m_SubSoundSets; //!< The sub SoundSets available for selection in this SoundSet.

		/// Clears all the member variables of this SoundSet, effectively resetting the members of this abstraction level only.
		void Clear();
	};
} // namespace RTE
