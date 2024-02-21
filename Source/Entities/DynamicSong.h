#pragma once

#include "Entity.h"

namespace RTE {
	// A type of SongSection for dynamic music. 
	class DynamicSongSectionType : public Entity {

	public:
		EntityAllocation(DynamicSongSectionType);
		SerializableOverrideMethods;
		ClassInfoGetters;

#pragma region Creation
		/// Constructor method used to instantiate a DynamicSongSectionType object in system memory. Create() should be called before using the object.
		DynamicSongSectionType();

		/// Copy constructor method used to instantiate a DynamicSongSectionType object identical to an already existing one.
		/// @param reference A reference to the DynamicSongSectionType to deep copy.
		DynamicSongSectionType(const DynamicSongSectionType& reference);

		/// Creates a DynamicSongSectionType to be identical to another, by deep copy.
		/// @param reference A reference to the DynamicSongSectionType to deep copy.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(const DynamicSongSectionType& reference);

#pragma endregion

#pragma region Destruction
		/// Destructor method used to clean up a DynamicSongSectionType object before deletion from system memory.
		~DynamicSongSectionType() override;

		/// Destroys and resets (through Clear()) the DynamicSongSectionType object. It doesn't delete the DynamicSongSectionTypeSections.
		/// @param notInherited Whether to only destroy the members defined in this derived class, or to destroy all inherited members also.
		void Destroy(bool notInherited = false) override {
			if (!notInherited) {
				Entity::Destroy();
			}
			Clear();
		}

		/// Resets the entire DynamicSongSectionType, including its inherited members, to their default settings or values.
		void Reset() override {
			Clear();
			Entity::Reset();
		}
#pragma endregion

	private:
		static Entity::ClassInfo m_sClass; //!< ClassInfo for this class.
		
		/// Clears all the member variables of this DynamicSongSectionType, effectively resetting the members of this abstraction level only.
		void Clear();
	};
	
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

		/// Destroys and resets (through Clear()) the DynamicSongSection object. It doesn't delete the DynamicSongSectionSections.
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

	private:
		static Entity::ClassInfo m_sClass; //!< ClassInfo for this class.
		
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

	private:
		static Entity::ClassInfo m_sClass; //!< ClassInfo for this class.
		
		/// Clears all the member variables of this DynamicSong, effectively resetting the members of this abstraction level only.
		void Clear();
	};
} // namespace RTE
