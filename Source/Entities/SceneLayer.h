#pragma once

#include "Entity.h"
#include "ContentFile.h"
#include "Box.h"

#include <future>

namespace RTE {

	/// A scrolling layer of the Scene.
	template <bool TRACK_DRAWINGS>
	class SceneLayerImpl : public Entity {
		friend class NetworkServer;

	public:
		EntityAllocation(SceneLayerImpl);
		SerializableOverrideMethods;

#pragma region Creation
		/// Constructor method used to instantiate a SceneLayerImpl object in system memory. Create() should be called before using the object.
		SceneLayerImpl();

		/// Makes the SceneLayer object ready for use.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create() override { return Entity::Create(); }

		/// Makes the SceneLayer object ready for use.
		/// @param bitmapFile The ContentFile to load as this SceneLayer's graphical representation.
		/// @param drawMasked Whether to draw masked (transparent) or not.
		/// @param offset The initial scroll offset.
		/// @param wrapX Whether the layer should wrap around or stop when scrolling beyond its bitmap's boundaries on the X axis.
		/// @param wrapY Whether the layer should wrap around or stop when scrolling beyond its bitmap's boundaries on the Y axis.
		/// @param scrollInfo
		/// A vector whose components define two different things, depending on wrap arguments.
		/// If a wrap argument is set to false, the corresponding component here will be interpreted as the width (X) or height (Y) (in pixels) of the total bitmap area that this layer is allowed to scroll across before stopping at an edge.
		/// If wrapping is set to true, the value in scrollInfo is simply the ratio of offset at which any scroll operations will be done in.
		/// A special command is if wrap is false and the corresponding component is -1.0, that signals that the own width or height should be used as scrollInfo input.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(const ContentFile& bitmapFile, bool drawMasked, const Vector& offset, bool wrapX, bool wrapY, const Vector& scrollInfo);

		/// Makes the SceneLayer object ready for use.
		/// @param bitmap The BITMAP to use for this SceneLayer. Ownership IS transferred!
		/// @param drawMasked Whether to draw masked (transparent) or not.
		/// @param offset The initial scroll offset.
		/// @param wrapX Whether the layer should wrap around or stop when scrolling beyond its bitmap's boundaries on the X axis.
		/// @param wrapY Whether the layer should wrap around or stop when scrolling beyond its bitmap's boundaries on the Y axis.
		/// @param scrollInfo
		/// A vector whose components define two different things, depending on wrap arguments.
		/// If a wrap argument is set to false, the corresponding component here will be interpreted as the width (X) or height (Y) (in pixels) of the total bitmap area that this layer is allowed to scroll across before stopping at an edge.
		/// If wrapping is set to true, the value in scrollInfo is simply the ratio of offset at which any scroll operations will be done in.
		/// A special command is if wrap is false and the corresponding component is -1.0, that signals that the own width or height should be used as scrollInfo input.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(BITMAP* bitmap, bool drawMasked, const Vector& offset, bool wrapX, bool wrapY, const Vector& scrollInfo);

		/// Creates a SceneLayer to be identical to another, by deep copy.
		/// @param reference A reference to the SceneLayer to deep copy.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(const SceneLayerImpl& reference);
#pragma endregion

#pragma region Destruction
		/// Destructor method used to clean up a SceneLayer object before deletion from system memory.
		~SceneLayerImpl() override;

		/// Destroys and resets (through Clear()) the SceneLayer object.
		/// @param notInherited Whether to only destroy the members defined in this derived class, or to destroy all inherited members also.
		void Destroy(bool notInherited = false) override;
#pragma endregion

#pragma region Data Handling
		/// Whether this SceneLayer's bitmap data is loaded from a file or was generated at runtime.
		/// @return Whether this SceneLayer's bitmap data was loaded from a file or not.
		virtual bool IsLoadedFromDisk() const { return !m_BitmapFile.GetDataPath().empty(); }

		/// Loads previously specified/created data into memory. Has to be done before using this SceneLayer if the bitmap was not generated at runtime.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		virtual int LoadData();

		/// Saves data currently in memory to disk.
		/// @param bitmapPath The filepath to the where to save the bitmap data.
		/// @param doAsyncSaves Whether or not to save asynchronously.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		virtual int SaveData(const std::string& bitmapPath, bool doAsyncSaves = true);

		/// Clears out any previously loaded bitmap data from memory.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		virtual int ClearData();
#pragma endregion

#pragma region Getters and Setters
		/// Gets the scroll offset of this SceneLayer.
		/// @return A Vector with the scroll offset.
		Vector GetOffset() const { return m_Offset; }

		/// Sets the scroll offset of this SceneLayer. Observe that this offset will be modified by the scroll ratio before applied.
		/// @param newOffset The new offset Vector.
		void SetOffset(const Vector& newOffset) { m_Offset = newOffset; }

		/// Gets the scroll ratio that modifies the offset.
		/// @return A copy of the ratio.
		Vector GetScrollRatio() const { return m_ScrollRatio; }

		/// Sets the scroll ratio of this SceneLayer. This modifies the offset before any actual scrolling occurs.
		/// @param newRatio The new scroll ratio vector.
		void SetScrollRatio(const Vector& newRatio) { m_ScrollRatio = newRatio; }

		/// Gets the scale factor that this is drawn in.
		/// @return The scale factor of this to the target it is drawn to. (2x if this is half the res, etc.)
		Vector GetScaleFactor() const { return m_ScaleFactor; }

		/// Sets the scale that this should be drawn at when using DrawScaled.
		/// @param newScale The new scale factor vector.
		void SetScaleFactor(const Vector& newScale);

		/// Indicates whether the layer is set to wrap around the X axis when scrolled out of bounds.
		/// @return Whether this SceneLayer wraps on the X axis or not.
		bool WrapsX() const { return m_WrapX; }

		/// Indicates whether the layer is set to wrap around the Y axis when scrolled out of bounds.
		/// @return Whether this SceneLayer wraps on the Y axis or not.
		bool WrapsY() const { return m_WrapY; }

		/// Gets a specific pixel from the bitmap of this SceneLayer.
		/// @param pixelX The X coordinate of the pixel to get.
		/// @param pixelY The Y coordinate of the pixel to get.
		/// @return An int specifying the requested pixel's color index.
		int GetPixel(int pixelX, int pixelY) const;

		/// Sets a specific pixel on the bitmap of this SceneLayer to a specific value.
		/// @param pixelX The X coordinate of the pixel to set.
		/// @param pixelY The Y coordinate of the pixel to set.
		/// @param materialID The color index to set the pixel to.
		void SetPixel(int pixelX, int pixelY, int materialID);

		/// Returns whether the integer coordinates passed in are within the bounds of this SceneLayer.
		/// @param pixelX The X coordinates of the pixel.
		/// @param pixelY The Y coordinates of the pixel.
		/// @param margin
		/// @return Whether within bounds or not.
		bool IsWithinBounds(int pixelX, int pixelY, int margin = 0) const;
#pragma endregion

#pragma region Concrete Methods
		/// Clears our BITMAP.
		/// @param clearTo What color to clear the bitmap to.
		void ClearBitmap(ColorKeys clearTo);

		/// Wraps the given position coordinate if it is out of bounds of this SceneLayer and wrapping is enabled on the appropriate axes.
		/// Does not force the position coordinate within bounds if wrapping is not enabled.
		/// @param posX The X coordinates of the position to wrap.
		/// @param posY The Y coordinates of the position to wrap.
		/// @return Whether wrapping was performed or not.
		bool WrapPosition(int& posX, int& posY) const;

		/// Wraps the given position Vector if it is out of bounds of this SceneLayer and wrapping is enabled on the appropriate axes.
		/// Does not force the position Vector within bounds if wrapping is not enabled.
		/// @param pos The vector coordinates of the position to wrap.
		/// @return Whether wrapping was performed or not.
		bool WrapPosition(Vector& pos) const { return ForceBoundsOrWrapPosition(pos, false); }

		/// Wraps or bounds a position coordinate if it is out of bounds of the SceneLayer, depending on the wrap settings of this SceneLayer.
		/// @param posX The X coordinates of the position to wrap.
		/// @param posY The Y coordinates of the position to wrap.
		/// @return Whether wrapping was performed or not. Does not report on bounding.
		bool ForceBounds(int& posX, int& posY) const;

		/// Wraps or bounds a position coordinate if it is out of bounds of the SceneLayer, depending on the wrap settings of this SceneLayer.
		/// @param pos The Vector coordinates of the position to wrap.
		/// @return Whether wrapping was performed or not. Does not report on bounding.
		bool ForceBounds(Vector& pos) const { return ForceBoundsOrWrapPosition(pos, true); }
#pragma endregion

#pragma region Drawing Tracking
		/// Registers an area of the SceneLayer to be drawn upon. These areas will be cleared when ClearBitmap is called.
		/// @param left The position of the left side of the area to be drawn upon.
		/// @param top The position of the top of the area to be drawn upon.
		/// @param right The position of the right side of the area to be drawn upon.
		/// @param bottom"The position of the bottom of the area to be drawn upon
		void RegisterDrawing(int left, int top, int right, int bottom);

		/// Registers an area of the SceneLayer to be drawn upon. These areas will be cleared when ClearBitmap is called.
		/// @param center The position of the center of the area to be drawn upon.
		/// @param radius The radius of the area to be drawn upon.
		void RegisterDrawing(const Vector& center, float radius);
#pragma endregion

#pragma region Virtual Methods
		/// Updates the state of this SceneLayer.
		virtual void Update() {}

		/// Draws this SceneLayer's current scrolled position to a bitmap.
		/// @param targetBitmap The bitmap to draw to.
		/// @param targetBox The box on the target bitmap to limit drawing to, with the corner of box being where the scroll position lines up.
		/// @param offsetNeedsScrollRatioAdjustment Whether the offset of this SceneLayer or the passed in offset override need to be adjusted to scroll ratio.
		virtual void Draw(BITMAP* targetBitmap, Box& targetBox, bool offsetNeedsScrollRatioAdjustment = false);
#pragma endregion

	protected:
		ContentFile m_BitmapFile; //!< ContentFile containing the path to this SceneLayer's sprite file.

		BITMAP* m_MainBitmap; //!< The main BITMAP of this SceneLayer.
		BITMAP* m_BackBitmap; //!< The backbuffer BITMAP of this SceneLayer.

		// We use two bitmaps, as a backbuffer. While the main bitmap is being used, the secondary bitmap will be cleared on a separate thread. This is because we tend to want to clear some scene layers every frame and that is costly.
		std::future<void> m_BitmapClearTask; //!< Task for clearing BITMAP async in background.
		ColorKeys m_LastClearColor; //!< The last color we cleared this SceneLayer to.
		std::vector<IntRect> m_Drawings; //!< All the areas drawn within on this SceneLayer since the last clear.

		bool m_MainBitmapOwned; //!< Whether the main bitmap is owned by this.
		bool m_DrawMasked; //!< Whether pixels marked as transparent (index 0, magenta) are skipped when drawing or not (masked drawing).

		bool m_WrapX; //!< Whether wrapping is enabled on the X axis.
		bool m_WrapY; //!< Whether wrapping is enable on the Y axis.

		Vector m_OriginOffset; //!< Offset of this SceneLayer off the top left edge of the screen.
		Vector m_Offset; //!< The current scrolled offset of this SceneLayer, before being adjusted with the origin offset.

		Vector m_ScrollInfo; //!< The initial scrolling ratio of this SceneLayer as set in INI. Used to calculate the actual scrolling ratios.
		Vector m_ScrollRatio; //!< The scrolling ratios of this SceneLayer, adjusted to the Scene, player screen dimensions and scaling factor as necessary.
		Vector m_ScaleFactor; //!< The scaling factor of this SceneLayer. Used for scaled drawing and adjusting scrolling ratios.
		Vector m_ScaledDimensions; //!< The dimensions of this SceneLayer adjusted to the scaling factor.

		/// Initialize the scroll ratios from the scroll info. Must be done after the bitmap has been created.
		/// @param initForNetworkPlayer
		/// @param player
		void InitScrollRatios(bool initForNetworkPlayer = false, int player = Players::NoPlayer);

		/// Wraps or bounds a position coordinate if it is out of bounds of the SceneLayer, depending on the wrap settings of this SceneLayer.
		/// @param pos The Vector coordinates of the position to wrap.
		/// @param forceBounds Whether to attempt bounding or wrapping, or just wrapping.
		/// @return Whether wrapping was performed or not. Does not report on bounding.
		bool ForceBoundsOrWrapPosition(Vector& pos, bool forceBounds) const;

#pragma region Draw Breakdown
		/// Performs wrapped drawing of this SceneLayer's bitmap to the screen in cases where it is both wider and taller than the target bitmap.
		/// @param targetBitmap The bitmap to draw to.
		/// @param targetBox The box on the target bitmap to limit drawing to, with the corner of box being where the scroll position lines up.
		/// @param drawScaled Whether to use scaled drawing routines or not.
		void DrawWrapped(BITMAP* targetBitmap, const Box& targetBox, bool drawScaled) const;

		/// Performs tiled drawing of this SceneLayer's bitmap to the screen in cases where the target bitmap is larger in some dimension.
		/// @param targetBitmap The bitmap to draw to.
		/// @param targetBox The box on the target bitmap to limit drawing to, with the corner of box being where the scroll position lines up.
		/// @param drawScaled Whether to use scaled drawing routines or not.
		void DrawTiled(BITMAP* targetBitmap, const Box& targetBox, bool drawScaled) const;
#pragma endregion

	private:
		/// Clears any tracked and drawn-to areas.
		/// @param clearTo Color to clear to.
		void ClearDrawings(BITMAP* bitmap, const std::vector<IntRect>& drawings, ColorKeys clearTo) const;

		/// Clears all the member variables of this SceneLayer, effectively resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		SceneLayerImpl(const SceneLayerImpl& reference) = delete;
		void operator=(const SceneLayerImpl& rhs) = delete;
	};

	/// If we track drawings, then disallow getting non-const access to the underlying bitmap - we must draw through special functions on SceneLayer that'll track the drawings.
	class SceneLayerTracked : public SceneLayerImpl<true> {

	public:
		EntityAllocation(SceneLayerTracked);
		ClassInfoGetters;

		/// Constructor method used to instantiate a SceneLayerTracked object in system memory. Create() should be called before using the object.
		SceneLayerTracked() :
		    SceneLayerImpl<true>() {}

		// TODO: We shouldn't let external users access a non-const version of our bitmap. We should do all drawing to it internally, and track registering our MOID drawings internally too.
		// However, in the interest of time (and my own sanity), given that the old code already does this, we're not doing that yet.
		/// Gets the BITMAP that this SceneLayer uses.
		/// @return A pointer to the BITMAP of this SceneLayer. Ownership is NOT transferred!
		BITMAP* GetBitmap() const { return m_MainBitmap; }

	protected:
		static Entity::ClassInfo m_sClass; //!< ClassInfo for this class.
	};

	class SceneLayer : public SceneLayerImpl<false> {

	public:
		EntityAllocation(SceneLayer);
		ClassInfoGetters;

		/// Constructor method used to instantiate a SceneLayer object in system memory. Create() should be called before using the object.
		SceneLayer() :
		    SceneLayerImpl<false>() {}

		/// Gets the BITMAP that this SceneLayer uses.
		/// @return A pointer to the BITMAP of this SceneLayer. Ownership is NOT transferred!
		BITMAP* GetBitmap() const { return m_MainBitmap; }

	protected:
		static Entity::ClassInfo m_sClass; //!< ClassInfo for this class.
	};
} // namespace RTE
