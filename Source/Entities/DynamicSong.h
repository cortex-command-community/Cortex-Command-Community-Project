#pragma once

#include "Entity.h"

namespace RTE {
	/// A typed SongSection containing one or more SoundContainers to play.
	class DynamicSongSection : public Entity {

	public:
		EntityAllocation(DynamicSongSection);
		SerializableOverrideMethods;
		ClassInfoGetters;

#pragma region Creation
		/// Constructor method used to instantiate a DynamicSongSection object in system memory. Create() should be called before using the object.
		DynamicSongSection();

		/// Copy constructor method used to instantiate a DynamicSongSection object identical to an already existing one.
		/// @param reference A reference to the DynamicSongSection to deep copy.
		DynamicSongSection(const DynamicSongSection& reference);

		/// Creates a DynamicSongSection to be identical to another, by deep copy.
		/// @param reference A reference to the DynamicSongSection to deep copy.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(const DynamicSongSection& reference);
#pragma endregion

#pragma region Destruction
		/// Destructor method used to clean up a DynamicSongSection object before deletion from system memory.
		~DynamicSongSection() override;

		/// Destroys and resets (through Clear()) the DynamicSongSection object.
		/// @param notInherited Whether to only destroy the members defined in this derived class, or to destroy all inherited members also.
		void Destroy(bool notInherited = false) override {
			if (!notInherited) {
				Entity::Destroy();
			}
			Clear();
		}

		/// Resets the entire DynamicSongSection, including its inherited members, to their default settings or values.
		void Reset() override {
			Clear();
			Entity::Reset();
		}
#pragma endregion

#pragma region SoundContainer Addition
	/// Adds a new TransitionSoundContainer to this DynamicSongSection.
	/// @param soundContainerToAdd The new SoundContainer to add.
	void AddTransitionSoundContainer(const SoundContainer& soundContainerToAdd) { m_TransitionSoundContainers.push_back(soundContainerToAdd); }

	/// Adds a new SoundContainer to this DynamicSongSection.
	/// @param soundContainerToAdd The new SoundContainer to add.
	void AddSoundContainer(const SoundContainer& soundContainerToAdd) { m_SoundContainers.push_back(soundContainerToAdd); }
#pragma endregion 

#pragma region Getters and Setters
	/// Gets the vector of TransitionSoundContainers for this DynamicSongSection.
	/// @return The vector of TransitionSoundContainers for this DynamicSongSection.
	std::vector<SoundContainer>& GetTransitionSoundContainers() { return m_TransitionSoundContainers; }
		
	/// Gets the vector of SoundContainers for this DynamicSongSection.
	/// @return The vector of SoundContainers for this DynamicSongSection.
	std::vector<SoundContainer>& GetSoundContainers() { return m_SoundContainers; }

	/// Gets the SectionType of this DynamicSongSection.
	/// @return The SectionType of this DynamicSongSection.
	std::string& GetSectionType() { return m_SectionType; }

	/// Sets the SectionType of this DynamicSongSection.
	/// @param newSectionType The new SectionType for this DynamicSongSection.
	void SetSectionType(const std::string& newSectionType) { m_SectionType = newSectionType; }

	/// Selects a random transitional SoundContainer with no repeats.
	/// @return The selected transitional SoundContainer.
	SoundContainer& SelectTransitionSoundContainer();

	/// Selects a random SoundContainer with no repeats.
	/// @return The selected SoundContainer.
	SoundContainer& SelectSoundContainer();
#pragma endregion 

	private:
		static Entity::ClassInfo m_sClass; //!< ClassInfo for this class.

		std::vector<SoundContainer> m_TransitionSoundContainers; //!< The SoundContainers that will play when first switching to this DynamicSongSection.
		unsigned int m_LastTransitionSoundContainerIndex; //!< The last index used to select a TransitionSoundContainer.
		std::vector<SoundContainer> m_SoundContainers; //!< The SoundContainers making up this DynamicSongSection.
		unsigned int m_LastSoundContainerIndex; //!< The last index used to select a SoundContainer.
		std::string m_SectionType; //!< The name of the type of dynamic music this is.
		
		/// Clears all the member variables of this DynamicSongSection, effectively resetting the members of this abstraction level only.
		void Clear();
	};

	/// A container for coherent DynamicSongSections forming a full music track.
	class DynamicSong : public Entity {

	public:
		EntityAllocation(DynamicSong);
		SerializableOverrideMethods;
		ClassInfoGetters;

#pragma region Creation
		/// Constructor method used to instantiate a DynamicSong object in system memory. Create() should be called before using the object.
		DynamicSong();

		/// Copy constructor method used to instantiate a DynamicSong object identical to an already existing one.
		/// @param reference A reference to the DynamicSong to deep copy.
		DynamicSong(const DynamicSong& reference);

		/// Creates a DynamicSong to be identical to another, by deep copy.
		/// @param reference A reference to the DynamicSong to deep copy.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(const DynamicSong& reference);
#pragma endregion

#pragma region Destruction
		/// Destructor method used to clean up a DynamicSong object before deletion from system memory.
		~DynamicSong() override;

		/// Destroys and resets (through Clear()) the DynamicSong object. It doesn't delete the DynamicSongSections.
		/// @param notInherited Whether to only destroy the members defined in this derived class, or to destroy all inherited members also.
		void Destroy(bool notInherited = false) override {
			if (!notInherited) {
				Entity::Destroy();
			}
			Clear();
		}

		/// Resets the entire DynamicSong, including its inherited members, to their default settings or values.
		void Reset() override {
			Clear();
			Entity::Reset();
		}
#pragma endregion
		
#pragma region SongSection Addition
		/// Adds a new DynamicSongSection to this DynamicSong. Will assert if this DynamicSong already has a song section with the given SectionType.
		/// @param songSectionToAdd The new DynamicSongSection to add.
		void AddSongSection(DynamicSongSection& songSectionToAdd) {
			for (DynamicSongSection& songSection: GetSongSections()) {
				if (songSection.GetSectionType() == songSectionToAdd.GetSectionType()) {
					RTEAssert(false, "Tried to add a SongSection with SectionType " + songSection.GetSectionType() + ", which the DynamicSong already had in another SongSection!")
					break;
				}
			}
			m_SongSections.push_back(songSectionToAdd);
		}
#pragma endregion
		
#pragma region Getters and Setters
		/// Gets the DefaultSongSection for this DynamicSong.
		/// @return The default DynamicSongSection for this DynamicSong.
		DynamicSongSection& GetDefaultSongSection() { return m_DefaultSongSection; }

		/// Sets the DefaultSongSection for this DynamicSong.
		/// @param newDefaultSongSection The new default DynamicSongSection for this DynamicSong.
		void SetDefaultSongSection(const DynamicSongSection& newDefaultSongSection) { m_DefaultSongSection = newDefaultSongSection; }

		/// Gets the vector of DynamicSongSections for this DynamicSong.
		/// @return The vector of DynamicSongSections for this DynamicSong.
		std::vector<DynamicSongSection>& GetSongSections() { return m_SongSections; }
#pragma endregion 

	private:
		static Entity::ClassInfo m_sClass; //!< ClassInfo for this class.

		DynamicSongSection m_DefaultSongSection; //!< The fallback DynamicSongSection if one with the desired Type can't be found.
		std::vector<DynamicSongSection> m_SongSections; //!< The DynamicSongSections making up this DynamicSong.
		
		/// Clears all the member variables of this DynamicSong, effectively resetting the members of this abstraction level only.
		void Clear();
	};
} // namespace RTE
