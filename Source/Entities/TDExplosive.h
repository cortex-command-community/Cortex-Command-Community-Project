#pragma once

#include "ThrownDevice.h"

namespace RTE {

	/// A thrown device that explodes after its trigger delay is completed after its activation.
	class TDExplosive : public ThrownDevice {

	public:
		EntityAllocation(TDExplosive);
		SerializableOverrideMethods;
		ClassInfoGetters;

#pragma region Creation
		/// Constructor method used to instantiate a TDExplosive object in system memory. Create() should be called before using the object.
		TDExplosive();

		/// Makes the TDExplosive object ready for use.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create() override;

		/// Creates a TDExplosive to be identical to another, by deep copy.
		/// @param reference A reference to the TDExplosive to deep copy.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(const TDExplosive& reference);
#pragma endregion

#pragma region Destruction
		/// Destructor method used to clean up a TDExplosive object before deletion from system memory.
		~TDExplosive() override;

		/// Destroys and resets (through Clear()) the SceneLayer object.
		/// @param notInherited Whether to only destroy the members defined in this derived class, or to destroy all inherited members also.
		void Destroy(bool notInherited = false) override {
			if (!notInherited) {
				ThrownDevice::Destroy();
			}
			Clear();
		}

		/// Resets the entire TDExplosive, including its inherited members, to their default settings or values.
		void Reset() override {
			Clear();
			ThrownDevice::Reset();
		}
#pragma endregion

#pragma region Getters and Setters
		/// If true, then the frame will not be changed automatically during update
		/// @return Whether or not the TDExplosive's Frame will change automatically during update.
		bool IsAnimatedManually() const { return m_IsAnimatedManually; }

		/// Sets whether this TDExplosive is animated manually.
		/// @param isAnimatedManually Whether or not to animate manually.
		void SetAnimatedManually(bool isAnimatedManually) { m_IsAnimatedManually = isAnimatedManually; }
#pragma endregion

#pragma region Virtual Override Methods
		/// Updates this MovableObject. Supposed to be done every frame.
		void Update() override;

		/// Draws this' current graphical HUD overlay representation to a BITMAP of choice.
		/// @param targetBitmap A pointer to a BITMAP to draw on.
		/// @param targetPos The absolute position of the target bitmap's upper left corner in the Scene.
		/// @param whichScreen Which player's screen this is being drawn to. May affect what HUD elements get drawn etc.
		/// @param playerControlled Whether or not this MovableObject is currently player controlled (not applicable for TDExplosive).
		void DrawHUD(BITMAP* targetBitmap, const Vector& targetPos = Vector(), int whichScreen = 0, bool playerControlled = false) override;
#pragma endregion

	protected:
		static Entity::ClassInfo m_sClass; //!< ClassInfo for this class.

		bool m_IsAnimatedManually; //!< If true m_Frame is not changed during an update hence the animation is done by external Lua code.

	private:
		/// Clears all the member variables of this TDExplosive, effectively resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		TDExplosive(const TDExplosive& reference) = delete;
		TDExplosive& operator=(const TDExplosive& rhs) = delete;
	};
} // namespace RTE
