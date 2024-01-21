#pragma once

/// Header file for the Magazine class.
/// @author Daniel Tabar
/// data@datarealms.com
/// http://www.datarealms.com
/// Inclusions of header files
#include "Attachable.h"
#include "Round.h"

namespace RTE {

	/// An Attachable ammo magazine that can hold rounds that can be fired
	/// by HDFirearm:s.
	class Magazine : public Attachable {

		/// Public member variable, method and friend function declarations
	public:
		// Concrete allocation and cloning definitions
		EntityAllocation(Magazine);
		SerializableOverrideMethods;
		ClassInfoGetters;

		/// Constructor method used to instantiate a Magazine object in system
		/// memory. Create() should be called before using the object.
		Magazine();

		/// Destructor method used to clean up a Magazine object before deletion
		/// from system memory.
		~Magazine() override;

		/// Makes the Magazine object ready for use.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create() override;

		/// Creates a Magazine to be identical to another, by deep copy.
		/// @param reference A reference to the Magazine to deep copy.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create(const Magazine& reference);

		/// Resets the entire Magazine, including its inherited members, to their
		/// default settings or values.
		void Reset() override {
			Clear();
			Attachable::Reset();
			m_CollidesWithTerrainWhileAttached = false;
		}

		/// Destroys and resets (through Clear()) the SceneLayer object.
		/// @param notInherited Whether to only destroy the members defined in this derived class, or (default: false)
		/// to destroy all inherited members also.
		void Destroy(bool notInherited = false) override;

		/// Gets the next Round preset of ammo in this Magazine, without removing
		/// it. Ownership IS NOT transferred!
		/// @return A pointer to the next Round preset of ammo, or 0 if this Magazine is empty.
		const Round* GetNextRound() const;

		/// Gets the next Round of ammo in this Magazine, and removes it from the
		/// stack. Ownership IS transferred!
		/// @return A pointer to the next Round of ammo, or 0 if this Magazine is empty.
		Round* PopNextRound();

		/// Returns how many rounds are left in this Magazine.
		/// @return The number of rounds left. Negative value means infinite ammo left!
		int GetRoundCount() const { return m_RoundCount; }

		/// Sets how many rounds are left in this Magazine.
		/// @param newCount The new number of rounds left. Negative value means infinite ammo!
		void SetRoundCount(int newCount) { m_RoundCount = newCount; }

		/// Returns whether this Magazine is out of rounds.
		/// @return Whether this Magazine is out of rounds or not.
		bool IsEmpty() const { return m_FullCapacity >= 0 && m_RoundCount == 0; }

		/// Returns whether this Magazine has not used up any rounds yet.
		/// @return Whether this Magazine has not used any rounds yet.
		bool IsFull() const { return m_FullCapacity > 0 ? (m_RoundCount == m_FullCapacity || m_RoundCount < 0) : m_FullCapacity < 0; }

		/// Returns whether this Magazine has not used up half of the rounds yet.
		/// @return Whether this Magazine has not used half of its rounds yet.
		bool IsOverHalfFull() const { return m_FullCapacity > 0 ? ((m_RoundCount > (m_FullCapacity / 2)) || m_RoundCount < 0) : m_FullCapacity < 0; }

		/// Returns teh number of rounds this can hold when it's full.
		/// @return The number of rounds this can hold. Negative value means infinite ammo.
		int GetCapacity() const { return m_FullCapacity; }

		/// Whether this Magazine should be released into the scene when discarded
		/// or just deleted.
		/// @return Whether this Magazine should be relesed into scene or deleted when released.
		bool IsDiscardable() const { return m_Discardable; }

		/// Estimates what material strength the rounds in the magazine can destroy.
		/// @return The material strength.
		float EstimateDigStrength() const;

		/// Tells the AI what muzzle velocity to assume when aiming this weapon.
		/// @return Velocity in m/s.
		float GetAIAimVel() const { return m_AIAimVel; }

		/// Tells the AI what distance in pixels from the rounds in this mag round
		/// are mostly safe.
		/// @return Distance in pixels.
		int GetAIAimBlastRadius() const { return m_AIBlastRadius; }

		/// Tells the AI how much material this projectile can penetrate.
		/// @return The material strenght.
		float GetAIAimPenetration() const { return m_AIAimPenetration; }

		/// Gets the bullet acceleration scalar the AI use when aiming this weapon.
		/// @return A float with the scalar.
		float GetBulletAccScalar();

		/// Updates this MovableObject. Supposed to be done every frame.
		void Update() override;

		/// Draws this Magazine's current graphical representation to a
		/// BITMAP of choice.
		/// @param pTargetBitmap A pointer to a BITMAP to draw on.
		/// @param targetPos The absolute position of the target bitmap's upper left corner in the Scene. (default: Vector())
		/// @param mode In which mode to draw in. See the DrawMode enumeration for the modes. (default: g_DrawColor)
		/// @param onlyPhysical Whether to not draw any extra 'ghost' items of this MovableObject, (default: false)
		/// indicator arrows or hovering HUD text and so on.
		void Draw(BITMAP* pTargetBitmap, const Vector& targetPos = Vector(), DrawMode mode = g_DrawColor, bool onlyPhysical = false) const override;

		/// Protected member variable and method declarations
	protected:
		// Member variables
		static Entity::ClassInfo m_sClass;

		// How many rounds in mag. Negative value means infinite ammo
		int m_RoundCount;
		// The number of rounds that this mag holds. negative menas infinite ammo capacity
		int m_FullCapacity;
		// The ratio between regular and tracer rounds. 0 means no tracers.
		// e.g. 3 means every third round will be a tracer. ie Round To Tracer (RTT) ratio.
		int m_RTTRatio;
		// Round reference instances.
		const Round* m_pRegularRound;
		const Round* m_pTracerRound;
		// Whether this magazine should be released into the scene when discarded, or just be deleted instead
		bool m_Discardable;
		// The muzzle velocity the AI use when aiming this gun. calculated when the magazine is created
		float m_AIAimVel;
		// The estimated maximum distance in pixels the projectiles can hit from
		float m_AIAimMaxDistance;
		// The half of the theoretical upper limit for the amount of material strength this weapon can destroy with one projectile
		float m_AIAimPenetration;
		// Tells the AI what distance in pixels from this round is mostly safe.
		int m_AIBlastRadius;

		/// Private member variable and method declarations
	private:
		/// Clears all the member variables of this Magazine, effectively
		/// resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		Magazine(const Magazine& reference) = delete;
		Magazine& operator=(const Magazine& rhs) = delete;
	};

} // namespace RTE
