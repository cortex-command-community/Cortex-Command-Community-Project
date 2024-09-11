#pragma once

#include "MovableObject.h"

namespace RTE {

	class Atom;

	/// A movable object with mass that is graphically represented by a single pixel.
	class MOPixel : public MovableObject {

	public:
		EntityAllocation(MOPixel);
		ClassInfoGetters;
		SerializableOverrideMethods;

#pragma region Creation
		/// Constructor method used to instantiate a MOPixel object in system memory. Create() should be called before using the object.
		MOPixel();

		/// Convenience constructor to both instantiate a MOPixel in memory and Create it at the same time.
		/// @param color A Color object specifying the color of this MOPixel.
		/// @param mass A float specifying the object's mass in Kilograms (kg).
		/// @param position A Vector specifying the initial position.
		/// @param velocity A Vector specifying the initial velocity.
		/// @param atom An Atom that will collide with the terrain. Ownership IS transferred!
		/// @param lifetime The amount of time in ms this MOPixel will exist. 0 means unlimited.
		MOPixel(Color color, const float mass, const Vector& position, const Vector& velocity, Atom* atom, const unsigned long lifetime = 0) {
			Clear();
			Create(color, mass, position, velocity, atom, lifetime);
		}

		/// Makes the MOPixel object ready for use.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create() override;

		/// Makes the MOPixel object ready for use.
		/// @param color A Color object specifying the color of this MOPixel.
		/// @param mass A float specifying the object's mass in Kilograms (kg).
		/// @param position A Vector specifying the initial position.
		/// @param velocity A Vector specifying the initial velocity.
		/// @param atom An Atom that will collide with the terrain.
		/// @param lifetime The amount of time in ms this MOPixel will exist. 0 means unlimited.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(Color color, const float mass, const Vector& position, const Vector& velocity, Atom* atom, const unsigned long lifetime = 0);

		/// Creates a MOPixel to be identical to another, by deep copy.
		/// @param reference A reference to the MOPixel to deep copy.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(const MOPixel& reference);
#pragma endregion

#pragma region Destruction
		/// Destructor method used to clean up a MOPixel object before deletion from system memory.
		~MOPixel() override;

		/// Destroys and resets (through Clear()) the MOPixel object.
		/// @param notInherited Whether to only destroy the members defined in this derived class, or to destroy all inherited members also.
		void Destroy(bool notInherited = false) override;

		/// Resets the entire MOPixel, including its inherited members, to their default settings or values.
		void Reset() override {
			Clear();
			MovableObject::Reset();
		}
#pragma endregion

#pragma region Getters and Setters
		/// Gets the drawing priority of this MOPixel, if two things were overlap when copying to the terrain, the higher priority MO would end up getting drawn.
		/// @return The drawing priority of this MOPixel.
		int GetDrawPriority() const override;

		/// Gets the main Material of this MOPixel.
		/// @return The Material of this MOPixel.
		const Material* GetMaterial() const override;

		/// Gets the current Atom of this MOPixel.
		/// @return A const reference to the current Atom.
		const Atom* GetAtom() const { return m_Atom; }

		/// Replaces the current Atom of this MOPixel with a new one.
		/// @param newAtom A reference to the new Atom. Ownership IS transferred!
		void SetAtom(Atom* newAtom);

		/// Gets the color of this MOPixel.
		/// @return A Color object describing the color.
		Color GetColor() const { return m_Color; }

		/// Gets the index of the color of this MOPixel.
		/// @return An int that is the index of the Color object.
		int GetColorIndex() const { return m_Color.GetIndex(); }

		/// Sets the color value of this MOPixel.
		/// @param newColor A Color object specifying the new color index value.
		void SetColor(Color newColor) { m_Color = newColor; }

		/// Sets the color value of this MOPixel via index.
		/// @param newColor An int specifying the new color index value.
		void SetColorIndex(int newColorIndex) { m_Color.SetRGBWithIndex(newColorIndex); }

		/// Travel distance until the bullet start to lose lethality.
		/// @return The factor that modifies the base value.
		float GetMaxLethalRangeFactor() const { return m_MaxLethalRange; }

		/// Travel distance until the bullet start to lose lethality.
		/// @param range The distance in pixels.
		void SetLethalRange(float range);

		/// Gets the longest a trail can be drawn, in pixels.
		/// @return The new max length, in pixels. If 0, no trail is drawn.
		int GetTrailLength() const;

		/// Sets the longest a trail can be drawn, in pixels.
		/// @param trailLength The new max length, in pixels. If 0, no trail is drawn.
		void SetTrailLength(int trailLength);

		/// Gets this MOPixel's staininess, which defines how likely a pixel is to stain a surface when it collides with it.
		/// @return This MOPixel's current staininess value.
		float GetStaininess() const { return m_Staininess; }

		/// Sets this MOPixel's staininess, which defines how likely a pixel is to stain a surface when it collides with it.
		/// @param staininess The new staininess value.
		void SetStaininess(float staininess) { m_Staininess = staininess; }

		/// Whether a set of X, Y coordinates overlap us (in world space).
		/// @param pixelX The given X coordinate, in world space.
		/// @param pixelY The given Y coordinate, in world space.
		/// @return Whether the given coordinate overlap us.
		bool HitTestAtPixel(int pixelX, int pixelY) const override;
#pragma endregion

#pragma region Virtual Override Methods
		/// Travels this MOPixel, using its physical representation.
		void Travel() override;

		/// Calculates the collision response when another MO's Atom collides with this MO's physical representation.
		/// The effects will be applied directly to this MO, and also represented in the passed in HitData.
		/// @param hitData Reference to the HitData struct which describes the collision. This will be modified to represent the results of the collision.
		/// @return Whether the collision has been deemed valid. If false, then disregard any impulses in the HitData.
		bool CollideAtPoint(HitData& hitData) override;

		/// Does the calculations necessary to detect whether this MOPixel is at rest or not. IsAtRest() retrieves the answer.
		void RestDetection() override;

		/// Defines what should happen when this MOPixel hits and then bounces off of something. This is called by the owned Atom/AtomGroup of this MOPixel during travel.
		/// @param hd The HitData describing the collision in detail.
		/// @return Whether the MOPixel should immediately halt any travel going on after this bounce.
		bool OnBounce(HitData& hd) override { return false; }

		/// Defines what should happen when this MOPixel hits and then sink into something. This is called by the owned Atom/AtomGroup of this MOPixel during travel.
		/// @param hd The HitData describing the collision in detail.
		/// @return Whether the MOPixel should immediately halt any travel going on after this sinkage.
		bool OnSink(HitData& hd) override { return false; }

		/// Updates this MOPixel. Supposed to be done every frame.
		void Update() override;

		/// Draws this MOPixel's current graphical representation to a BITMAP of choice.
		/// @param targetBitmap A pointer to a BITMAP to draw on.
		/// @param targetPos The absolute position of the target bitmap's upper left corner in the Scene.
		/// @param mode In which mode to draw in. See the DrawMode enumeration for the modes.
		/// @param onlyPhysical Whether to not draw any extra 'ghost' items of this MOPixel, indicator arrows or hovering HUD text and so on.
		void Draw(BITMAP* targetBitmap, const Vector& targetPos = Vector(), DrawMode mode = g_DrawColor, bool onlyPhysical = false) const override;
#pragma endregion

	protected:
		static Entity::ClassInfo m_sClass; //!< ClassInfo for this class.

		Atom* m_Atom; //!< The single Atom that is responsible for collisions of this MOPixel.
		Color m_Color; //!< Color representation of this MOPixel.

		float m_LethalRange; //!< After this distance in meters, the MO has a chance to no longer hit MOs, and its Lifetime decreases. Defaults to the length of a player's screen.
		float m_MinLethalRange; //!< Lower bound multiplier for setting LethalRange at random. By default, 1.0 equals one screen.
		float m_MaxLethalRange; //!< Upper bound multiplier for setting LethalRange at random. By default, 1.0 equals one screen.
		float m_LethalSharpness; //!< When Sharpness has decreased below this threshold the MO becomes m_HitsMOs = false. Default is Sharpness * 0.5.
		float m_Staininess; //!< How likely a pixel is to stain a surface when it collides with it. Defaults to 0 (never stain).

	private:
		/// Clears all the member variables of this MOPixel, effectively resetting the members of this abstraction level only.
		void Clear();
	};
} // namespace RTE
