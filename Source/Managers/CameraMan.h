#pragma once

#include "Singleton.h"
#include "Timer.h"
#include "Vector.h"

#include <array>

#define g_CameraMan CameraMan::Instance()

namespace RTE {

	/// The singleton manager of the camera for each player.
	class CameraMan : public Singleton<CameraMan> {
		friend class SettingsMan;

	public:
#pragma region Creation
		/// Constructor method used to instantiate a CameraMan object in system memory. Create() should be called before using the object.
		CameraMan();

		/// Makes the CameraMan object ready for use.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Initialize() const { return 0; }
#pragma endregion

#pragma region Destruction
		/// Destructor method used to clean up a CameraMan object before deletion from system memory.
		~CameraMan();

		/// Resets the entire CameraMan to their default settings or values.
		void Reset() { Clear(); }

		/// Destroys and resets (through Clear()) the CameraMan object.
		void Destroy() { Clear(); }
#pragma endregion

#pragma region Screen Handling
		/// Gets the offset (scroll position) of the terrain.
		/// @param screenId Which screen you want to get the offset of.
		/// @return The offset for the given screen.
		Vector GetOffset(int screenId = 0) const { return m_Screens[screenId].Offset; }

		/// Sets the offset (scroll position) of the terrain.
		/// @param offset The new offset value.
		/// @param screenId Which screen you want to set the offset of.
		void SetOffset(const Vector& offset, int screenId = 0);

		/// Gets the difference in current offset and that of the Update() before.
		/// @return The delta offset in pixels.
		Vector GetDeltaOffset(int screenId = 0) const { return m_Screens[screenId].DeltaOffset; }

		/// Gets the offset (scroll position) of the terrain, without taking wrapping into account.
		/// @param screenId Which screen you want to get the offset of.
		/// @return The offset for the given screen.
		Vector GetUnwrappedOffset(int screenId = 0) const;

		/// Sets the offset (scroll position) of the terrain to center on specific world coordinates.
		/// If the coordinate to center on is close to the terrain border edges, the view will not scroll outside the borders.
		/// @param center The coordinates to center the terrain scroll on.
		/// @param screenId Which screen you want to set the offset of.
		void SetScroll(const Vector& center, int screenId = 0);

		/// Gets the team associated with a specific screen.
		/// @param screenId Which screen you want to get the team of.
		/// @return The team associated with the screen.
		int GetScreenTeam(int screenId = 0) const { return m_Screens[screenId].ScreenTeam; }

		/// Sets the team associated with a specific screen.
		/// @param team The team to set the screen to.
		/// @param screenId Which screen you want to set the team of.
		void SetScreenTeam(int team, int screenId = 0) { m_Screens[screenId].ScreenTeam = team; }

		/// Gets the amount that a specific screen is occluded by a GUI panel or something of the sort.
		/// This will affect how the scroll target translates into the offset of the screen, in order to keep the target centered on the screen.
		/// @param screenId Which screen you want to get the team of.
		/// @return A vector indicating the screen occlusion amount.
		Vector& GetScreenOcclusion(int screenId = 0) { return m_Screens[screenId].ScreenOcclusion; }

		/// Sets the amount that a specific screen is occluded by a GUI panel or something of the sort.
		/// This will affect how the scroll target translates into the offset of the screen, in order to keep the target centered on the screen.
		/// @param occlusion The amount of occlusion of the screen.
		/// @param screenId Which screen you want to set the occlusion of.
		void SetScreenOcclusion(const Vector& occlusion, int screenId = 0) { m_Screens[screenId].ScreenOcclusion = occlusion; }

		/// Gets the currently set scroll target, i.e. where the center of the specific screen is trying to line up with.
		/// @param screenId Which screen to get the target for.
		/// @return Current target vector in Scene coordinates.
		Vector GetScrollTarget(int screenId = 0) const;

		/// Interpolates a smooth scroll of the view from wherever it is now, towards centering on a new scroll target over time.
		/// @param targetCenter The new target vector in Scene coordinates.
		/// @param speed The normalized speed at screen the view scrolls. 0 being no movement, and 1.0 being instant movement to the target in one frame.
		/// @param screenId Which screen you want to set the scroll offset of.
		void SetScrollTarget(const Vector& targetCenter, float speed = 0.1F, int screenId = 0);

		/// Calculates a scalar of how distant a certain point in the world is from the currently closest scroll target of all active screens.
		/// @param point The world coordinate point to check distance to/from.
		/// @return
		/// A normalized scalar representing the distance between the closest scroll target of all active screens, to the passed in point.
		/// 0 means it's the point is within half a screen's width of the target, and 1.0 means it's on the clear opposite side of the scene.
		float TargetDistanceScalar(const Vector& point) const;

		/// Makes sure the current offset won't create a view of outside the scene.
		/// If that is found to be the case, the offset is corrected so that the view rectangle
		/// is as close to the old offset as possible, but still entirely within the scene world.
		/// @param screenId Which screen you want to check the offset of.
		void CheckOffset(int screenId = 0);

		/// Gets the frame width/height for a given screen.
		/// @param screenId Which screen you want to get frame width/height of.
		/// @return The frame width (x) and height (y).
		Vector GetFrameSize(int screenId = 0);
#pragma endregion

#pragma region Screen Shake Getters and Setters
		/// Gets the screen shake strength multiplier.
		/// @return The screen shake strength multiplier.
		float GetScreenShakeStrength() const { return m_ScreenShakeStrength; }

		/// Sets the screen shake strength multiplier.
		/// @param newValue New value for the screen shake strength multiplier.
		void SetScreenShakeStrength(float newValue) { m_ScreenShakeStrength = newValue; }

		/// Gets how quickly screen shake decays, per second.
		/// @return The screen shake decay.
		float GetScreenShakeDecay() const { return m_ScreenShakeDecay; }

		/// Gets the maximum amount of screen shake time, i.e. the number of seconds screen shake will happen until ScreenShakeDecay reduces it to zero.
		/// @return The maximum screen shake time, in seconds.
		float GetMaxScreenShakeTime() const { return m_MaxScreenShakeTime; }

		/// Gets how much the screen should shake per unit of energy from gibbing (i.e explosions), when screen shake amount is auto-calculated.
		/// @return The default shakiness per unit of gib energy.
		float GetDefaultShakePerUnitOfGibEnergy() const { return m_DefaultShakePerUnitOfGibEnergy; }

		/// Gets how much the screen should shake per unit of energy for recoil, when screen shake amount is auto-calculated.
		/// @return The default shakiness per unit of recoil energy.
		float GetDefaultShakePerUnitOfRecoilEnergy() const { return m_DefaultShakePerUnitOfRecoilEnergy; }

		/// The maximum amount of screen shake recoil can cause, when screen shake is auto-calculated. This is ignored by per-firearm shake settings.
		/// @return The maximum auto-calculated recoil shakiness.
		float GetDefaultShakeFromRecoilMaximum() const { return m_DefaultShakeFromRecoilMaximum; }
#pragma endregion

#pragma region Screen Shake Actions
		/// Resets all screen shake and the screen scroll timers that affect it.
		void ResetAllScreenShake();

		/// Increases the magnitude of screen shake.
		/// This is used for spatially located screen-shake, and will automatically determine which screens have shake applied
		/// If the screen-shake position is outside our view, it'll gradually weaken and fade away depending on distance.
		/// @param magnitude The amount of screen shake.
		/// @param position The spatial location of the screen-shake event.
		void AddScreenShake(float magnitude, const Vector& position);

		/// Increases the magnitude of screen shake.
		/// @param magnitude The amount of screen shake to add.
		/// @param screenId Which screen you want to add screen-shake to.
		void AddScreenShake(float magnitude, int screenId = 0) { m_Screens[screenId].ScreenShakeMagnitude += magnitude; }

		/// Sets the magnitude of screen shake.
		/// @param magnitude The amount of screen shake.
		/// @param screenId Which screen you want to set screen-shake for.
		void SetScreenShake(float magnitude, int screenId = 0) { m_Screens[screenId].ScreenShakeMagnitude = magnitude; }

		/// Applies screen shake to be at least magnitude.
		/// @param magnitude The amount of screen shake.
		/// @param screenId Which screen you want to set screen-shake for.
		void ApplyScreenShake(float magnitude, int screenId = 0) { m_Screens[screenId].ScreenShakeMagnitude = std::max(magnitude, m_Screens[screenId].ScreenShakeMagnitude); }
#pragma endregion

#pragma region Concrete Methods
		/// Updates the state of this CameraMan. Supposed to be done every frame before drawing.
		void Update(int screenId);
#pragma endregion

	private:
		/// A screen. Each player should have one of these.
		/// TODO: This is a struct right now, as it has been torn verbatim out of SceneMan. In future it should be a proper class with methods, instead of CameraMan handling everything.
		struct Screen {
			int ScreenTeam = 0; //!< The team associated with this Screen.

			Vector Offset; //!< The position of the upper left corner of the view.
			Vector DeltaOffset; //!< The difference in current offset and the Update() before.
			Vector ScrollTarget; //!< The final offset target of the current scroll interpolation, in scene coordinates.

			Timer ScrollTimer; //!< Scroll timer for making scrolling work framerate independently.
			float ScrollSpeed = 0; //!< The normalized speed the screen's view scrolls. 0 being no movement, and 1.0 being instant movement to the target in one frame.

			bool TargetXWrapped = false; //!< Whether the ScrollTarget got x wrapped around the world this frame or not.
			bool TargetYWrapped = false; //!< Whether the ScrollTarget got y wrapped around the world this frame or not.

			std::array<int, 2> SeamCrossCount = {0, 0}; //!< Keeps track of how many times and in screen directions the wrapping seam has been crossed. This is used for keeping the background layers' scroll from jumping when wrapping around. X and Y.

			Vector ScreenOcclusion; //!< The amount a screen is occluded or covered by GUI, etc.

			float ScreenShakeMagnitude = 0; //!< The magnitude of screen shake that is currently being applied.
		};

		float m_ScreenShakeStrength; //!< A global multiplier applied to screen shaking strength.
		float m_ScreenShakeDecay; //!< How quickly screen shake falls off.
		float m_MaxScreenShakeTime; //!< The maximum amount of screen shake time, i.e. the number of seconds screen shake will happen until ScreenShakeDecay reduces it to zero.
		float m_DefaultShakePerUnitOfGibEnergy; //!< How much the screen should shake per unit of energy from gibbing (i.e explosions), when screen shake amount is auto-calculated.
		float m_DefaultShakePerUnitOfRecoilEnergy; //!< How much the screen should shake per unit of energy for recoil, when screen shake amount is auto-calculated.
		float m_DefaultShakeFromRecoilMaximum; //!< The maximum amount of screen shake recoil can cause, when screen shake is auto-calculated. This is ignored by per-firearm shake settings.

		std::array<Screen, c_MaxScreenCount> m_Screens; //!< Array with the Screen of each player.

		/// Clears all the member variables of this CameraMan, effectively resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		CameraMan(const CameraMan& reference) = delete;
		CameraMan& operator=(const CameraMan& rhs) = delete;
	};
} // namespace RTE
