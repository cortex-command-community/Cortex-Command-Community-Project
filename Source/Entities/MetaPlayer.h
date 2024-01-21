#pragma once

#include "Entity.h"

namespace RTE {

	/// Holds data for a Metagame player aka "Tech" or "House".
	class MetaPlayer : public Entity {
		friend class MetagameGUI;

	public:
		EntityAllocation(MetaPlayer);
		SerializableOverrideMethods;
		ClassInfoGetters;

#pragma region Creation
		/// Constructor method used to instantiate a MetaPlayer object in system memory. Create() should be called before using the object.
		MetaPlayer();

		/// Copy constructor method used to instantiate a MetaPlayer object identical to an already existing one.
		/// @param reference A MetaPlayer object which is passed in by reference.
		MetaPlayer(const MetaPlayer& reference);

		/// Makes the MetaPlayer object ready for use.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create() override;

		/// Creates a MetaPlayer to be identical to another, by deep copy.
		/// @param reference A reference to the MetaPlayer to deep copy.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(const MetaPlayer& reference);
#pragma endregion

#pragma region Destruction
		/// Destructor method used to clean up a MetaPlayer object before deletion from system memory.
		~MetaPlayer() override;

		/// Destroys and resets (through Clear()) the MetaPlayer object.
		/// @param notInherited Whether to only destroy the members defined in this derived class, or to destroy all inherited members also.
		void Destroy(bool notInherited = false) override {
			if (!notInherited) {
				Entity::Destroy();
			}
			Clear();
		}

		/// Resets the entire MetaPlayer, including its inherited members, to their default settings or values.
		void Reset() override {
			Clear();
			Entity::Reset();
		}
#pragma endregion

#pragma region Getters and Setters
		/// Gets the name of the MetaPlayer.
		/// @return The name of the player.
		std::string GetName() const { return m_Name; }

		/// Sets the name of the MetaPlayer.
		/// @param newName The new name to set.
		void SetName(std::string newName) { m_Name = newName; }

		/// Gets the Team of this MetaPlayer.
		/// @return The Team of this player.
		int GetTeam() const { return m_Team; }

		/// Sets the Team of the MetaPlayer.
		/// @param newTeam The new team to set.
		void SetTeam(int newTeam) { m_Team = newTeam; }

		/// Indicates whether this MetaPlayer is human controlled or not (AI).
		/// @return Whether this player is human controlled.
		bool IsHuman() const { return m_Human; }

		/// Sets whether this MetaPlayer is human controlled or not (AI).
		/// @param human Whether this MetaPlayer is human controlled or not.
		void SetHuman(bool human) { m_Human = human; }

		/// Shows which in-game player controls this MetaPlayer is mapped to.
		/// @return The in-game player number this is mapped to.
		int GetInGamePlayer() const { return m_InGamePlayer; }

		/// Gets ID of the DataModule that this MetaPlayer is native to.
		/// @return The ID of the DataModule this is native to.
		int GetNativeTechModule() const { return m_NativeTechModule; }

		/// Gets the normalized aggressiveness scalar of this player if an AI.
		/// @return The current aggressiveness scalar, 0 min to 1.0 max.
		float GetAggressiveness() const { return m_Aggressiveness; }

		/// Sets the normalized aggressiveness scalar of this player if an AI.
		/// @param aggressiveness The new aggressiveness scalar, 0 min to 1.0 max.
		void SetAggressiveness(float aggressiveness) { m_Aggressiveness = aggressiveness; }

		/// Indicates which round this MetaPlayer made it to. If negative, he is still in the game.
		/// @return Which round the MetaPlayer made it to.
		int GetGameOverRound() const { return m_GameOverRound; }

		/// Sets which round this MetaPlayer lost out on. If set to negative, it means he's still in the game.
		/// @param gameOverRound The round the MetaPlayer lost out on.
		void SetGameOverRound(int gameOverRound) { m_GameOverRound = gameOverRound; }

		/// Tells whether this MetaPlayer is out of the game on or before a particular round of the current metagame.
		/// @param whichRound Which round to check against.
		/// @return Whether the MetaPlayer was flagged as being out of the game on that or any earlier round.
		bool IsGameOverByRound(int whichRound) const { return m_GameOverRound >= 0 && m_GameOverRound <= whichRound; }

		/// Gets the name of the scene this MetaPlayer is targeting for offensive.
		/// @return The name of the Scene this MetaPlayer is targeting.
		std::string GetOffensiveTargetName() const { return m_OffensiveTarget; }

		/// Sets the name of the scene this MetaPlayer is targeting for offensive.
		/// @param targetName The name of the Scene this MetaPlayer is targeting.
		void SetOffensiveTargetName(std::string targetName) { m_OffensiveTarget = targetName; }
#pragma endregion

#pragma region Funds and Costs
		/// Gets the amount of funds this MetaPlayer currently has in the game.
		/// @return A float with the funds tally for the requested MetaPlayer.
		float GetFunds() const { return m_Funds; }

		/// Sets the amount of funds this MetaPlayer currently has in the game.
		/// @param newFunds The new funds value for this MetaPlayer.
		void SetFunds(float newFunds) { m_Funds = newFunds; }

		/// Changes this MetaPlayer's funds level by a certain amount.
		/// @param howMuch The amount with which to change the funds balance.
		/// @return The new amount of funds of this MetaPlayer.
		float ChangeFunds(float howMuch) { return m_Funds += howMuch; }

		/// Decreases this player's funds level by a certain absolute amount, and returns that difference as a positive value.
		/// If the amount isn't available to spend in the funds, the rest of the funds are spent and only that amount is returned.
		/// @param howMuch The amount with which to change the funds balance. This should be a positive value to decrease the funds amount.
		/// @return The amount of funds that were spent.
		float SpendFunds(float howMuch) {
			howMuch = std::min(m_Funds, howMuch);
			m_Funds -= howMuch;
			return howMuch;
		}

		/// Gets the offensive budget of this MetaPlayer for this round, in oz.
		/// @return The offensive budget, in oz.
		float GetOffensiveBudget() const { return m_OffensiveBudget; }

		/// Sets the offensive budget of this MetaPlayer for this round, in oz.
		/// @param newBudget The new offensive budget, in oz.
		void SetOffensiveBudget(float newBudget) { m_OffensiveBudget = newBudget; }

		/// Gets the multiplier of costs of any Tech items foreign to this MetaPlayer.
		/// @return The scalar multiplier of all costs of foreign tech items.
		float GetForeignCostMultiplier() const { return m_ForeignCostMult; }

		/// Gets the multiplier of costs of any Tech items native to this MetaPlayer.
		/// @return The scalar multiplier of all costs of native tech items.
		float GetNativeCostMultiplier() const { return m_NativeCostMult; }

		/// Sets the multiplier of costs of any Tech items native to this MetaPlayer.
		/// @param newNativeCostMult The scalar multiplier of all costs of native tech items.
		void SetNativeCostMultiplier(float newNativeCostMult) { m_NativeCostMult = newNativeCostMult; }
#pragma endregion

#pragma region Brain Pool
		/// Gets the number of brains in this MetaPlayer's brain pool.
		/// @return The number of brains that are available for deployment.
		int GetBrainPoolCount() const { return m_BrainPool; }

		/// Sets the number of brains in this MetaPlayer's brain pool.
		/// @param brainCount The number of brains that should be available for deployment.
		void SetBrainPoolCount(int brainCount) { m_BrainPool = brainCount; }

		/// Alters the number of brains in this MetaPlayer's brain pool.
		/// @param change The number of brains to add or remove from the pool.
		/// @return The resulting count after the alteration.
		int ChangeBrainPoolCount(int change) { return m_BrainPool += change; }

		/// Gets the number of brains in this MetaPlayer that are out on the move between the pool and sites.
		/// @return The number of brains that are out in transit.
		int GetBrainsInTransit() const { return m_BrainsInTransit; }

		/// Sets the number of brains in this MetaPlayer that are out on the move between the pool and sites.
		/// @param transitCount The number of brains that are out in transit.
		void SetBrainsInTransit(int transitCount) { m_BrainsInTransit = transitCount; }

		/// Alters the number of brains of this MetaPlayer which are traveling.
		/// @param change The number of brains to add or remove from transit.
		/// @return The resulting count after the alteration.
		int ChangeBrainsInTransit(int change) { return m_BrainsInTransit += change; }
#pragma endregion

#pragma region Operator Overloads
		/// An assignment operator for setting one MetaPlayer equal to another.
		/// @param rhs A MetaPlayer reference.
		/// @return A reference to the changed MetaPlayer.
		MetaPlayer& operator=(const MetaPlayer& rhs) {
			if (this != &rhs) {
				Destroy();
				Create(rhs);
			}
			return *this;
		}
#pragma endregion

	protected:
		static Entity::ClassInfo m_sClass; //!< ClassInfo for this class.

		std::string m_Name; //!< The name of the player.

		int m_Team; //!< The team index of this player.
		bool m_Human; //!< Whether the player is human controlled.
		int m_InGamePlayer; //!< The in-game player controls that this MetaPlayer is mapped to.
		float m_Aggressiveness; //!< Normalized aggressiveness temperament of this if it's an AI player. 0 is min, 1.0 max.

		int m_GameOverRound; //!< Which round this player ended up getting to. If negative, he's still in the game.

		int m_NativeTechModule; //!< The tech module that will have native pricing for this player.
		float m_NativeCostMult; //!< The cost multiplier of native groups available to this player.
		float m_ForeignCostMult; //!< The cost multiplier of foreign groups available to this player.

		int m_BrainPool; //!< The current number of unused brains left available to this player in the pool.
		int m_BrainsInTransit; //!< The current number of brains that are traveling to or from a site; this is only for UI purposes to show the player what's happening, it's just a runtime var - don't save it.

		float m_Funds; //!< The current funds of this player, in oz.
		float m_PhaseStartFunds; //!< The previous round's funds of this player, in oz. No need to save this, a runtime var.
		float m_OffensiveBudget; //!< The money allocated by this player this round to attack or explore a specific site

		std::string m_OffensiveTarget; //!< Name of the Scene this player is targeting for its offensive this round.

	private:
		/// Clears all the member variables of this MetaPlayer, effectively resetting the members of this abstraction level only.
		void Clear();
	};
} // namespace RTE
