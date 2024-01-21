#pragma once

#include "Entity.h"

namespace RTE {

	/// A light-weight Entity for storing only the necessary info about how to load an entire MetaMan state from disk.
	class MetaSave : public Entity {

	public:
		EntityAllocation(MetaSave);
		SerializableOverrideMethods;
		ClassInfoGetters;

#pragma region Creation
		/// Constructor method used to instantiate a MetaSave object in system memory. Create() should be called before using the object.
		MetaSave();

		/// Makes the MetaSave object ready for use from the currently loaded MetaMan state.
		/// @param savePath The path of the file to where the MetaMan state should be saved.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(std::string savePath);

		/// Creates a MetaSave to be identical to another, by deep copy.
		/// @param reference A reference to the MetaSave to deep copy.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(const MetaSave& reference);
#pragma endregion

#pragma region Destruction
		/// Destructor method used to clean up a MetaSave object before deletion from system memory.
		~MetaSave() override;

		/// Destroys and resets (through Clear()) the MetaSave object.
		/// @param notInherited Whether to only destroy the members defined in this derived class, or to destroy all inherited members also.
		void Destroy(bool notInherited = false) override {
			if (!notInherited) {
				Entity::Destroy();
			}
			Clear();
		}
#pragma endregion

#pragma region Getters
		/// Gets the full path to the ini file that stores the state of the MetaMan this is associated with.
		/// @return The path to the ini with the MetaMan state info.
		std::string GetSavePath() const { return m_SavePath; }

		/// Gets the total number of players this game has (including AIs).
		/// @return The player count.
		int GetPlayerCount() const { return m_PlayerCount; }

		/// Gets the difficulty for this game.
		/// @return Difficulty setting.
		int GetDifficulty() const { return m_Difficulty; }

		/// Gets the round number that this game is on.
		/// @return The round count.
		int GetRoundCount() const { return m_RoundCount; }

		/// Gets the total number of Scenes this game has.
		/// @return The number of Scenes.
		int GetSiteCount() const { return m_SiteCount; }
#pragma endregion

	protected:
		static Entity::ClassInfo m_sClass; //!< ClassInfo for this class.

		std::string m_SavePath; //!< The full path to the ini file which stores the stat of MetaMan this is associated with.
		int m_PlayerCount; //!< The number of players in this saved game.
		int m_Difficulty; //!< Game difficulty.
		int m_RoundCount; //!< The round this game is on.
		int m_SiteCount; //!< The site count of this game.

	private:
		/// Clears all the member variables of this MetaSave, effectively resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		MetaSave(const MetaSave& reference) = delete;
		MetaSave& operator=(const MetaSave& rhs) = delete;
	};
} // namespace RTE
