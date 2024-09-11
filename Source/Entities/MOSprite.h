#pragma once

/// Header file for the MOSprite class.
/// @author Daniel Tabar
/// data@datarealms.com
/// http://www.datarealms.com
/// Inclusions of header files
#include "MovableObject.h"
#include "Box.h"

namespace RTE {

	class AEmitter;

	/// A movable object with mass that is graphically represented by a
	/// BITMAP.
	class MOSprite : public MovableObject {

		/// Public member variable, method and friend function declarations
	public:
		SerializableOverrideMethods;
		ClassInfoGetters;

		/// Constructor method used to instantiate a MOSprite object in system
		/// memory. Create() should be called before using the object.
		MOSprite();

		/// Destructor method used to clean up a MOSprite object before deletion
		/// from system memory.
		~MOSprite() override;

		/// Makes the MOSprite object ready for use.
		/// @param spriteFile A pointer to ContentFile that represents the bitmap file that will be
		/// used to create the Sprite.
		/// @param frameCount The number of frames in the Sprite's animation. (default: 1)
		/// @param mass A float specifying the object's mass in Kilograms (kg). (default: 1)
		/// @param position A Vector specifying the initial position. (default: Vector(0)
		/// @param 0) A Vector specifying the initial velocity.
		/// @param velocity The amount of time in ms this MovableObject will exist. 0 means unlim. (default: Vector(0)
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create(ContentFile spriteFile, const int frameCount = 1, const float mass = 1, const Vector& position = Vector(0, 0), const Vector& velocity = Vector(0, 0), const unsigned long lifetime = 0);

		/// Creates a MOSprite to be identical to another, by deep copy.
		/// @param reference A reference to the MOSprite to deep copy.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create(const MOSprite& reference);

		/// Makes the MOSprite object ready for use.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create() override;

		/// Resets the entire MOSprite, including its inherited members, to their
		/// default settings or values.
		void Reset() override {
			Clear();
			MovableObject::Reset();
		}

		/// Destroys and resets (through Clear()) the MOSprite object.
		/// @param notInherited Whether to only destroy the members defined in this derived class, or (default: false)
		/// to destroy all inherited members also.
		void Destroy(bool notInherited = false) override;

		/// Gets the largest radius of this in pixels.
		/// @return The radius from its center to the edge of its graphical representation.
		float GetRadius() const override { return m_SpriteRadius; }

		/// Gets the largest diameter of this in pixels.
		/// @return The largest diameter across its graphical representation.
		float GetDiameter() const override { return m_SpriteDiameter; }

		/// Gets the absoltue position of the top of this' HUD stack.
		/// @return A Vector with the absolute position of this' HUD stack top point.
		Vector GetAboveHUDPos() const override { return m_Pos + Vector(0, -GetRadius()); }

		// TODO: Improve this one! Really crappy fit
		/// Gets the oriented bounding box which is guaranteed to contain this,
		/// taking rotation etc into account. It's not guaranteed to be fit
		/// perfectly though. TODO MAKE FIT BETTER
		/// @return A Box which is guaranteed to contain this. Does nto take wrapping into
		/// account, and parts of this box may be out of bounds!
		Box GetBoundingBox() const { return Box(m_Pos + Vector(-GetRadius(), -GetRadius()), GetDiameter(), GetDiameter()); }

		/// Gets a frame of this MOSprite's BITMAP array.
		/// @param whichFrame Which frame to get. (default: 0)
		/// @return A pointer to the requested frame of this MOSprite's BITMAP array.
		/// Ownership is NOT transferred!
		BITMAP* GetSpriteFrame(unsigned int whichFrame = 0) const { return (whichFrame < m_FrameCount) ? m_aSprite[whichFrame] : 0; }

		/// Gets the width of the bitmap of this MOSprite
		/// @return Sprite width if loaded.
		int GetSpriteWidth() const { return m_aSprite[0] ? m_aSprite[0]->w : 0; }

		/// Gets the height of the bitmap of this MOSprite
		/// @return Sprite height if loaded.
		int GetSpriteHeight() const { return m_aSprite[0] ? m_aSprite[0]->h : 0; }

		/// Gets the number of frames in this MOSprite's animation.
		/// @return The frame count.
		int GetFrameCount() const { return m_FrameCount; }

		/// Gets the offset that the BITMAP has from the position of this
		/// MOSprite.
		/// @return A vector with the offset.
		Vector GetSpriteOffset() const { return m_SpriteOffset; }

		/// Returns whether this MOSprite is being drawn flipped horizontally
		/// (along the vertical axis), or not.
		/// @return Whether flipped or not.
		bool IsHFlipped() const override { return m_HFlipped; }

		/// Gets the current rotational Matrix of of this.
		/// @return The rotational Matrix of this MovableObject.
		Matrix GetRotMatrix() const override { return m_Rotation; }

		/// Gets the current rotational angle of of this, in radians.
		/// @return The rotational angle of this, in radians.
		float GetRotAngle() const override { return m_Rotation.GetRadAngle(); }

		/// Gets the previous rotational angle of this MOSprite, prior to this frame.
		/// @return The previous rotational angle in radians.
		float GetPrevRotAngle() const { return m_PrevRotation.GetRadAngle(); }

		/// Whether a set of X, Y coordinates overlap us (in world space).
		/// @param pixelX The given X coordinate, in world space.
		/// @param pixelY The given Y coordinate, in world space.
		/// @return Whether the given coordinate overlap us.
		bool HitTestAtPixel(int pixelX, int pixelY) const override;

		/// Gets the current angular velocity of this MovableObject. Positive is
		/// a counter-clockwise rotation.
		/// @return The angular velocity in radians per second.
		float GetAngularVel() const override { return m_AngularVel; }

		/* Can't do this since sprite is owned by ContentMan.
		/// Replaces this MOSprite's BITMAP and deletes the old one.
		/// @param pSprite A pointer to a BITMAP.
		    void SetSprite(BITMAP *pSprite) { delete m_aSprite; m_aSprite = pSprite; }
		*/

		/// Sets the offset that the BITMAP has from the position of this
		/// MOSprite.
		/// @param newOffset A vector with the new offset.
		void SetSpriteOffset(const Vector& newOffset) { m_SpriteOffset = newOffset; }

		/// Hard-sets the frame this sprite is supposed to show.
		/// @param newFrame An unsigned int pecifiying the new frame.
		void SetFrame(unsigned int newFrame);

		/// Hard-sets the frame this sprite is supposed to show, to the
		/// consecutive one after the current one. If currently the last fame is
		/// this will set it to the be the first, looping the animation.
		/// @return Whether the animation looped or not with this setting.
		bool SetNextFrame();

		/// Tells which frame is currently set to show.
		/// @return An unsigned int describing the current frame.
		unsigned int GetFrame() const { return m_Frame; }

		/// Sets the animation mode.
		/// @param animMode The animation mode we want to set. (default: NOANIM)
		void SetSpriteAnimMode(int animMode = NOANIM) { m_SpriteAnimMode = (SpriteAnimMode)animMode; }

		/// Gets the animation mode.
		/// @return The animation mode currently in effect.
		int GetSpriteAnimMode() const { return m_SpriteAnimMode; }

		/// Sets whether this MOSprite should be drawn flipped horizontally
		/// (along the vertical axis).
		/// @param flipped A bool with the new value.
		void SetHFlipped(const bool flipped) override { m_HFlipped = flipped; }

		/// Sets the current absolute angle of rotation of this MovableObject.
		/// @param m_Rotation.SetRadAngle(newAngle The new absolute angle in radians.
		void SetRotAngle(float newAngle) override { m_Rotation.SetRadAngle(newAngle); }

		/// Sets the current angular velocity of this MovableObject. Positive is
		/// a counter clockwise rotation.
		/// @param newRotVel The new angular velocity in radians per second.
		void SetAngularVel(float newRotVel) override { m_AngularVel = newRotVel; }

		/// Gets the GUI representation of this MOSprite, either based on the first frame of its sprite or separately defined icon file.
		/// @return The graphical representation of this MOSprite as a BITMAP.
		BITMAP* GetGraphicalIcon() const override { return m_GraphicalIcon != nullptr ? m_GraphicalIcon : m_aSprite[0]; }

		/// Gets the width of this MOSprite's GUI icon.
		/// @return The width of the GUI icon bitmap.
		int GetIconWidth() const { return GetGraphicalIcon()->w; }

		/// Gets the height of this MOSprite's GUI icon.
		/// @return The height of the GUI icon bitmap.
		int GetIconHeight() const { return GetGraphicalIcon()->h; }

		/// Forces this MOSprite out of resting conditions.
		void NotResting() override {
			MovableObject::NotResting();
			m_AngOscillations = 0;
		}

		/// Indicates whether this MO is moving or rotating stupidly fast in a way
		/// that will screw up the simulation.
		/// @return Whether this is either moving or rotating too fast.
		bool IsTooFast() const override { return m_Vel.MagnitudeIsGreaterThan(500.0F) || std::fabs(m_AngularVel) > (2000.0F / (GetRadius() + 1.0F)); }

		/// Slows the speed of anything that is deemed to be too fast to within
		/// acceptable rates.
		void FixTooFast() override {
			while (IsTooFast()) {
				m_Vel *= 0.5F;
				m_AngularVel *= 0.5F;
			}
		}

		/// Indicates whether this' current graphical representation overlaps
		/// a point in absolute scene coordinates.
		/// @param scenePoint The point in absolute scene coordinates.
		/// @return Whether this' graphical rep overlaps the scene point.
		bool IsOnScenePoint(Vector& scenePoint) const override;

		/// Takes a vector which is offset from the center of this when not rotated
		/// or flipped, and then rotates and/or flips it to transform it into this'
		/// 'local space', if applicable.
		/// @param offset A vector which is supposed to be offset from this' center when upright.
		/// @return The resulting vector whihch has been flipped and rotated as appropriate.
		Vector RotateOffset(const Vector& offset) const override;

		/// Takes a vector which is offset from the center of this when not rotated
		/// or flipped, and then rotates and/or flips it to transform it into this'
		/// 'local space', but in REVERSE.
		/// @param offset A vector which is supposed to be offset from this' center when upright.
		/// @return The resulting vector whihch has been flipped and rotated as appropriate.
		Vector UnRotateOffset(const Vector& offset) const;

		/// Adjusts an absolute angle based on wether this MOSprite is flipped.
		/// @param angle The input angle in radians.
		/// @return The output angle in radians, which will be unaltered if this MOSprite is not flipped.
		float FacingAngle(float angle) const { return (m_HFlipped ? c_PI : 0) + (angle * GetFlipFactor()); }

		/// Sets entry wound emitter for this MOSprite
		/// @param presetName Emitter preset name and module name
		void SetEntryWound(std::string presetName, std::string moduleName);

		/// Sets exit wound emitter for this MOSprite
		/// @param presetName Emitter preset name and module name
		void SetExitWound(std::string presetName, std::string moduleName);

		/// Returns entry wound emitter preset name for this MOSprite
		/// @return Wound emitter preset name
		std::string GetEntryWoundPresetName() const;

		/// Returns exit wound emitter preset name for this MOSprite
		/// @return Wound emitter preset name
		std::string GetExitWoundPresetName() const;

		/// Returns animation duration in ms
		/// @return Animation duration in ms
		int GetSpriteAnimDuration() const { return m_SpriteAnimDuration; }

		/// Sets animation duration in ms
		/// @param newDuration Animation duration in ms
		void SetSpriteAnimDuration(int newDuration) { m_SpriteAnimDuration = newDuration; }

		/// Updates this MovableObject. Supposed to be done every frame.
		void Update() override;

		/// Draws this MOSprite's current graphical representation to a
		/// BITMAP of choice.
		/// @param pTargetBitmap A pointer to a BITMAP to draw on.
		/// @param targetPos The absolute position of the target bitmap's upper left corner in the Scene. (default: Vector())
		/// @param mode In which mode to draw in. See the DrawMode enumeration for the modes. (default: g_DrawColor)
		/// @param onlyPhysical Whether to not draw any extra 'ghost' items of this MovableObject, (default: false)
		/// indicator arrows or hovering HUD text and so on.
		void Draw(BITMAP* pTargetBitmap, const Vector& targetPos = Vector(), DrawMode mode = g_DrawColor, bool onlyPhysical = false) const override;

		/// Returns a positive or negative number value to multiply with for external calculations.
		/// @return 1 for not flipped, -1 for flipped.
		float GetFlipFactor() const { return m_HFlipped ? -1.0F : 1.0F; }

		/// Protected member variable and method declarations
	protected:
		// Member variables
		static Entity::ClassInfo m_sClass;

		Matrix m_Rotation; // Rotational matrix of this MovableObject.
		Matrix m_PrevRotation; // Rotational matrix of this MovableObject, last frame.
		float m_AngularVel; // The angular velocity by which this MovableObject rotates, in radians per second (r/s).
		float m_PrevAngVel; // Previous frame's angular velocity.
		ContentFile m_SpriteFile;
		// Vector of pointers to BITMAPs representing the multiple frames of this sprite.
		std::vector<BITMAP*> m_aSprite;
		ContentFile m_IconFile; //!< The file containing the GUI icon.
		BITMAP* m_GraphicalIcon; //!< The GUI representation of this MOSprite as a BITMAP.
		// Number of frames, or elements in the m_aSprite array.
		unsigned int m_FrameCount;
		Vector m_SpriteOffset;
		// State, which frame of sprite to be drawn.
		unsigned int m_Frame;
		// Which animation to use for the body
		SpriteAnimMode m_SpriteAnimMode;
		// Body animation duration, how long in ms that each full body animation cycle takes.
		int m_SpriteAnimDuration;
		// The timer to keep track of the body animation
		Timer m_SpriteAnimTimer;
		// Keep track of animation direction (mainly for ALWAYSPINGPONG), true is decreasing frame, false is increasing frame
		bool m_SpriteAnimIsReversingFrames;
		// Whether flipped horizontally or not.
		bool m_HFlipped;
		// The precalculated maximum possible radius and diameter of this, in pixels
		float m_SpriteRadius;
		float m_SpriteDiameter;
		int m_AngOscillations; //!< A counter for oscillations in rotation, in order to detect settling.
		// Whether to disable the settle material ID when this gets drawn as material
		bool m_SettleMaterialDisabled;
		// Entry wound template
		const AEmitter* m_pEntryWound;
		// Exit wound template
		const AEmitter* m_pExitWound;

		/// Private member variable and method declarations
	private:
		/// Clears all the member variables of this MOSprite, effectively
		/// resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		MOSprite(const MOSprite& reference) = delete;
		MOSprite& operator=(const MOSprite& rhs) = delete;
	};

} // namespace RTE
