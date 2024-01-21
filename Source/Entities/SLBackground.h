#pragma once

#include "SceneLayer.h"
#include "Timer.h"

namespace RTE {

	/// A scrolling background layer of the Scene, placed behind the terrain.
	class SLBackground : public SceneLayer {
		friend class NetworkServer;

	public:
		EntityAllocation(SLBackground);
		SerializableOverrideMethods;
		ClassInfoGetters;

#pragma region Creation
		/// Constructor method used to instantiate a SLBackground object in system memory. Create() should be called before using the object.
		SLBackground();

		/// Makes the SLBackground object ready for use.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create() override;

		/// Creates a SLBackground to be identical to another, by deep copy.
		/// @param reference A reference to the SLBackground to deep copy.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(const SLBackground& reference);
#pragma endregion

#pragma region Destruction
		/// Destructor method used to clean up a SLBackground object before deletion from system memory.
		~SLBackground() override;

		/// Destroys and resets (through Clear()) the SLBackground object.
		/// @param notInherited Whether to only destroy the members defined in this derived class, or to destroy all inherited members also.
		void Destroy(bool notInherited = false) override {
			if (!notInherited) {
				SceneLayer::Destroy();
			}
			Clear();
		}
#pragma endregion

#pragma region Getters and Setters
		/// Gets whether this SLBackground's animation is handled manually/externally.
		/// @return Whether this SLBackground's animation is handled manually/externally. If true, animation will not be handled during Update().
		bool IsAnimatedManually() const { return m_IsAnimatedManually; }

		/// Sets whether this SLBackground is animated manually/externally.
		/// @param isAnimatedManually Whether this SLBackground is animated manually/externally and should skip animation handling during Update().
		void SetAnimatedManually(bool isAnimatedManually) { m_IsAnimatedManually = isAnimatedManually; }

		/// Gets the frame number of this SLBackground that is currently set to be drawn.
		/// @return The frame number that is currently set to be drawn.
		int GetFrame() const { return m_Frame; }

		/// Sets frame number that this SLBackground will draw.
		/// @param newFrame The frame number that is supposed to be drawn.
		void SetFrame(int newFrame) { m_Frame = std::clamp(newFrame, 0, m_FrameCount - 1); }

		/// Gets the animation mode of this SLBackground.
		/// @return The currently set animation mode. See SpriteAnimMode enumeration.
		int GetSpriteAnimMode() const { return m_SpriteAnimMode; }

		/// Sets the animation mode of this SLBackground.
		/// @param newAnimMode The new animation mode. See SpirteAnimMode enumeration.
		void SetSpriteAnimMode(SpriteAnimMode newAnimMode = SpriteAnimMode::NOANIM) { m_SpriteAnimMode = std::clamp(newAnimMode, SpriteAnimMode::NOANIM, SpriteAnimMode::ALWAYSPINGPONG); }

		/// Gets the time it takes to complete a full animation cycle of this SLBackground.
		/// @return The animation cycle duration, in milliseconds.
		int GetSpriteAnimDuration() const { return m_SpriteAnimDuration; }

		/// Sets the time it takes to complete a full animation cycle of this SLBackground.
		/// @param newDuration The new animation cycle duration, in milliseconds.
		void SetSpriteAnimDuration(int newDuration) { m_SpriteAnimDuration = newDuration; }

		/// Gets whether this SLBackground has auto-scrolling enabled and meets the requirements to actually auto-scroll.
		/// @return Whether this has auto-scrolling enabled and meets the requirements to actually auto-scroll.
		bool IsAutoScrolling() const { return (m_WrapX && m_CanAutoScrollX) || (m_WrapY && m_CanAutoScrollY); }

		/// Gets whether auto-scrolling is enabled on the X axis.
		/// @return Whether auto-scrolling is enabled on the X axis. This may be true even if auto-scrolling isn't actually happening due to not meeting requirements.
		bool GetAutoScrollX() const { return m_CanAutoScrollX; }

		/// Sets whether auto-scrolling is enabled on the X axis.
		/// @param autoScroll Whether auto-scrolling is enabled on the X axis or not. If requirements aren't met, this will not auto-scroll even if set to true.
		void SetAutoScrollX(bool autoScroll) { m_CanAutoScrollX = autoScroll; }

		/// Gets whether auto-scrolling is enabled on the Y axis.
		/// @return Whether auto-scrolling is enabled on the Y axis. This may be true even if auto-scrolling isn't actually happening due to not meeting requirements.
		bool GetAutoScrollY() const { return m_CanAutoScrollY; }

		/// Sets whether auto-scrolling is enabled on the Y axis.
		/// @param autoScroll Whether auto-scrolling is enabled on the Y axis or not. If requirements aren't met, this will not auto-scroll even if set to true.
		void SetAutoScrollY(bool autoScroll) { m_CanAutoScrollY = autoScroll; }

		/// Gets the duration between auto-scroll steps.
		/// @return The duration between auto-scroll steps, in milliseconds.
		int GetAutoScrollStepInterval() const { return m_AutoScrollStepInterval; }

		/// Sets the duration between auto-scroll steps.
		/// @param newStepInterval The new duration between auto-scroll steps, in milliseconds.
		void SetAutoScrollStepInterval(int newStepInterval) { m_AutoScrollStepInterval = newStepInterval; }

		/// Gets the auto-scroll step (pixels to advance per interval) values.
		/// @return A Vector with the auto-scroll step values.
		Vector GetAutoScrollStep() const { return m_AutoScrollStep; }

		/// Sets the auto-scroll step (pixels to advance per interval) values.
		/// @param newStep A Vector with the new auto-scroll step values.
		void SetAutoScrollStep(const Vector& newStep) { m_AutoScrollStep = newStep; }

		/// Gets the auto-scroll step (pixels to advance per interval) value on the X axis.
		/// @return The auto-scroll step value on the X axis.
		float GetAutoScrollStepX() const { return m_AutoScrollStep.GetX(); }

		/// Sets the auto-scroll step (pixels to advance per interval) value on the X axis.
		/// @param newStepX The new auto-scroll step value on the X axis.
		void SetAutoScrollStepX(float newStepX) { m_AutoScrollStep.SetX(newStepX); }

		/// Gets the auto-scroll step (pixels to advance per interval) value on the Y axis.
		/// @return The auto-scroll step value on the Y axis.
		float GetAutoScrollStepY() const { return m_AutoScrollStep.GetY(); }

		/// Sets the auto-scroll step (pixels to advance per interval) value on the Y axis.
		/// @param newStepY The new auto-scroll step value on the Y axis.
		void SetAutoScrollStepY(float newStepY) { m_AutoScrollStep.SetY(newStepY); }
#pragma endregion

#pragma region Concrete Methods
		/// Initializes the scale factors for all auto-scaling modes for this SLBackground, then sets the appropriate factor according to the auto-scaling setting.
		/// Has to be done during Scene loading to correctly adjust the factors in cases the Scene does not vertically cover the player's whole screen.
		void InitScaleFactors();
#pragma endregion

#pragma region Virtual Override Methods
		/// Updates the state of this SLBackground.
		void Update() override;

		/// Draws this SLBackground's current scrolled position to a bitmap.
		/// @param targetBitmap The bitmap to draw to.
		/// @param targetBox The box on the target bitmap to limit drawing to, with the corner of box being where the scroll position lines up.
		/// @param offsetNeedsScrollRatioAdjustment Whether the offset of this SceneLayer or the passed in offset override need to be adjusted to scroll ratio.
		void Draw(BITMAP* targetBitmap, Box& targetBox, bool offsetNeedsScrollRatioAdjustment = false) override;
#pragma endregion

	private:
		/// Enumeration for the different modes of SLBackground auto-scaling.
		enum LayerAutoScaleMode {
			AutoScaleOff,
			FitScreen,
			AlwaysUpscaled,
			LayerAutoScaleModeCount
		};

		static Entity::ClassInfo m_sClass; //!< ClassInfo for this class.

		std::vector<BITMAP*> m_Bitmaps; //!< Vector containing all the BITMAPs of this SLBackground. Not owned.
		int m_FrameCount; //!< The total number of frames in this SLBackground's animation.
		int m_Frame; //!< The frame that is currently being shown/drawn.

		SpriteAnimMode m_SpriteAnimMode; //!< The mode in which this SLBackground is animating. See SpriteAnimMode enumeration.
		int m_SpriteAnimDuration; //!< The duration it takes to complete a full animation cycle, in milliseconds.
		bool m_SpriteAnimIsReversingFrames; //!< Keeps track of animation direction (mainly for ALWAYSPINGPONG). True is decreasing frame, false is increasing frame.
		Timer m_SpriteAnimTimer; //!< Timer to keep track of animation.

		bool m_IsAnimatedManually; //!< Whether this SLBackground's animation is handled manually/externally and should not be handled during Update().

		bool m_CanAutoScrollX; //!< Whether auto-scrolling is enabled on the X axis, but not whether auto-scrolling actually happens on this axis.
		bool m_CanAutoScrollY; //!< Whether auto-scrolling is enabled on the Y axis, but not whether auto-scrolling actually happens on this axis.
		Vector m_AutoScrollStep; //!< Vector with the number of pixels on each axis to advance per interval when auto-scrolling. Can be fractions of a pixel, but will not be visible until AutoScrollOffset adds up to a full pixel step.
		int m_AutoScrollStepInterval; //!< The duration between auto-scroll steps, in milliseconds.
		Timer m_AutoScrollStepTimer; //!< Timer to keep track of auto-scrolling steps.
		Vector m_AutoScrollOffset; //!< The offset to adjust the main offset with when auto-scrolling to actually get the auto-scrolling effect, adjusted for wrapping.

		int m_FillColorLeft; //!< Palette index to use for filling the gap between the left edge of the bitmap and the left edge of the screen/scene box in cases where the bitmap doesn't cover the whole target area.
		int m_FillColorRight; //!< Palette index to use for filling the gap between the right edge of the bitmap and the right edge of the screen/scene box in cases where the bitmap doesn't cover the whole target area.
		int m_FillColorUp; //!< Palette index to use for filling the gap between the upper edge of the bitmap and the upper edge of the screen/scene box in cases where the bitmap doesn't cover the whole target area.
		int m_FillColorDown; //!< Palette index to use for filling the gap between the lower edge of the bitmap and the lower edge of the screen/scene box in cases where the bitmap doesn't cover the whole target area.

		bool m_IgnoreAutoScale; //!< Whether auto-scaling settings are ignored and the read-in scale factor is used instead.

		/// Clears all the member variables of this SLBackground, effectively resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		SLBackground(const SLBackground& reference) = delete;
		SLBackground& operator=(const SLBackground& rhs) = delete;
	};
} // namespace RTE
