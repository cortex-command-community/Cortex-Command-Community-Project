#pragma once

#include "MOSprite.h"

namespace RTE {

	class Atom;

	/// A small animated sprite that plays its animation and changes the animation and playback speed when it collides with other things.
	class MOSParticle : public MOSprite {

	public:
		EntityAllocation(MOSParticle);
		ClassInfoGetters;
		SerializableOverrideMethods;

#pragma region Creation
		/// Constructor method used to instantiate a MOSParticle object in system memory. Create() should be called before using the object.
		MOSParticle();

		/// Makes the MOSParticle object ready for use.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create() override;

		/// Makes the MOSParticle object ready for use.
		/// @param spriteFile A pointer to ContentFile that represents the bitmap file that will be used to create the Sprite.
		/// @param frameCount The number of frames in the Sprite's animation.
		/// @param mass A float specifying the object's mass in Kilograms (kg).
		/// @param position A Vector specifying the initial position.
		/// @param velocity A Vector specifying the initial velocity.
		/// @param lifetime The amount of time in ms this MOSParticle will exist. 0 means unlimited.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(ContentFile spriteFile, const int frameCount = 1, const float mass = 1, const Vector& position = Vector(0, 0), const Vector& velocity = Vector(0, 0), const unsigned long lifetime = 0) {
			MOSprite::Create(spriteFile, frameCount, mass, position, velocity, lifetime);
			return 0;
		}

		/// Creates a MOSParticle to be identical to another, by deep copy.
		/// @param reference A reference to the MOSParticle to deep copy.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(const MOSParticle& reference);
#pragma endregion

#pragma region Destruction
		/// Destructor method used to clean up a MOSParticle object before deletion from system memory.
		~MOSParticle() override;

		/// Destroys and resets (through Clear()) the SceneLayer object.
		/// @param notInherited Whether to only destroy the members defined in this derived class, or to destroy all inherited members also.
		void Destroy(bool notInherited = false) override;

		/// Resets the entire MOSParticle, including its inherited members, to their default settings or values.
		void Reset() override {
			Clear();
			MOSprite::Reset();
		}
#pragma endregion

#pragma region Getters and Setters
		/// Gets the drawing priority of this MOSParticle. If two things are overlapping when copying to the terrain, the higher priority MO would end up getting drawn.
		/// @return The drawing priority of this MOSParticle.
		int GetDrawPriority() const override;

		/// Gets the main material of this MOSParticle.
		/// @return The material of this MOSParticle.
		const Material* GetMaterial() const override;

		/// Gets the current Atom of this MOSParticle.
		/// @return A const reference to the current Atom.
		const Atom* GetAtom() const { return m_Atom; }

		/// Replaces the current Atom of this MOSParticle with a new one.
		/// @param newAtom A reference to the new Atom.
		void SetAtom(Atom* newAtom);
#pragma endregion

#pragma region Virtual Override Methods
		/// Travels this MOSParticle, using its physical representation.
		void Travel() override;

		/// Calculates the collision response when another MO's Atom collides with this MO's physical representation.
		/// The effects will be applied directly to this MO, and also represented in the passed in HitData.
		/// @param hitData Reference to the HitData struct which describes the collision. This will be modified to represent the results of the collision.
		/// @return Whether the collision has been deemed valid. If false, then disregard any impulses in the HitData.
		bool CollideAtPoint(HitData& hitData) override { return true; }

		/// Does the calculations necessary to detect whether this MOSParticle is at rest or not. IsAtRest() retrieves the answer.
		void RestDetection() override;

		/// Defines what should happen when this MOSParticle hits and then bounces off of something. This is called by the owned Atom/AtomGroup of this MOSParticle during travel.
		/// @param hd The HitData describing the collision in detail.
		/// @return Whether the MOSParticle should immediately halt any travel going on after this bounce.
		bool OnBounce(HitData& hd) override { return false; }

		/// Defines what should happen when this MOSParticle hits and then sink into something. This is called by the owned Atom/AtomGroup of this MOSParticle during travel.
		/// @param hd The HitData describing the collision in detail.
		/// @return Whether the MOSParticle should immediately halt any travel going on after this sinkage.
		bool OnSink(HitData& hd) override { return false; }

		/// Updates this MOParticle. Supposed to be done every frame.
		void Update() override;

		/// Draws this MOSParticle's current graphical representation to a BITMAP of choice.
		/// @param targetBitmap A pointer to a BITMAP to draw on.
		/// @param targetPos The absolute position of the target bitmap's upper left corner in the Scene.
		/// @param mode In which mode to draw in. See the DrawMode enumeration for the modes.
		/// @param onlyPhysical Whether to not draw any extra 'ghost' items of this MOSParticle, indicator arrows or hovering HUD text and so on.
		void Draw(BITMAP* targetBitmap, const Vector& targetPos = Vector(), DrawMode mode = g_DrawColor, bool onlyPhysical = false) const override;
#pragma endregion

	protected:
		static Entity::ClassInfo m_sClass; //!< ClassInfo for this class.

		Atom* m_Atom; //!< The Atom that will be the physical representation of this MOSParticle.
		float m_TimeRest; //!< Accumulated time in seconds that did not cause a frame change.

	private:
		/// Clears all the member variables of this MOSParticle, effectively resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		MOSParticle(const MOSParticle& reference) = delete;
		MOSParticle& operator=(const MOSParticle& rhs) = delete;
	};
} // namespace RTE
