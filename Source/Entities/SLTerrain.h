#pragma once

#include "SceneLayer.h"
#include "Matrix.h"

namespace RTE {

	class MOPixel;
	class TerrainFrosting;
	class TerrainObject;
	class TerrainDebris;

	/// Collection of scrolling layers that compose the terrain of the Scene.
	class SLTerrain : public SceneLayer {

	public:
		EntityAllocation(SLTerrain);
		SerializableOverrideMethods;
		ClassInfoGetters;

		/// Enumeration for the different type of layers in the SLTerrain.
		enum class LayerType {
			ForegroundLayer,
			BackgroundLayer,
			MaterialLayer
		};

#pragma region Creation
		/// Constructor method used to instantiate a SLTerrain object in system memory. Create() should be called before using the object.
		SLTerrain();

		/// Makes the SLTerrain object ready for use.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create() override;

		/// Creates a SLTerrain to be identical to another, by deep copy.
		/// @param reference A reference to the SLTerrain to deep copy.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(const SLTerrain& reference);
#pragma endregion

#pragma region Destruction
		/// Destructor method used to clean up a SLTerrain object before deletion from system memory.
		~SLTerrain() override;

		/// Destroys and resets (through Clear()) the SLTerrain object.
		/// @param notInherited Whether to only destroy the members defined in this derived class, or to destroy all inherited members also.
		void Destroy(bool notInherited = false) override {
			if (!notInherited) {
				SceneLayer::Destroy();
			}
			Clear();
		}
#pragma endregion

#pragma region Data Handling
		/// Whether this SLTerrain's bitmap data is loaded from a file or was generated at runtime.
		/// @return Whether this SLTerrain's bitmap data was loaded from a file or was generated at runtime.
		bool IsLoadedFromDisk() const override { return (m_FGColorLayer && m_FGColorLayer->IsLoadedFromDisk()) && (m_BGColorLayer && m_BGColorLayer->IsLoadedFromDisk()); }

		/// Loads previously specified/created bitmap data into memory. Has to be done before using this SLTerrain if the bitmap was not generated at runtime.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int LoadData() override;

		/// Saves bitmap data currently in memory to disk.
		/// @param pathBase The filepath base to the where to save the Bitmap data. This means everything up to the extension. "FG" and "Mat" etc will be added.
		/// @param doAsyncSaves Whether or not to save asynchronously.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int SaveData(const std::string& pathBase, bool doAsyncSaves = true) override;

		/// Clears out any previously loaded bitmap data from memory.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int ClearData() override;
#pragma endregion

#pragma region Getters and Setters
		/// Gets the width of this SLTerrain as determined by the main (material) bitmap.
		/// @return The width of this SLTerrain, in pixels.
		int GetWidth() const { return m_Width; }

		/// Gets the height of this SLTerrain as determined by the main (material) bitmap.
		/// @return The height of this SLTerrain, in pixels.
		int GetHeight() const { return m_Height; }

		/// Sets the layer of this SLTerrain that should be drawn to the screen when Draw() is called.
		/// @param layerToDraw The layer that should be drawn. See LayerType enumeration.
		void SetLayerToDraw(LayerType layerToDraw) { m_LayerToDraw = layerToDraw; }

		/// Gets the foreground color bitmap of this SLTerrain.
		/// @return A pointer to the foreground color bitmap.
		BITMAP* GetFGColorBitmap() { return m_FGColorLayer->GetBitmap(); }

		/// Gets the background color bitmap of this SLTerrain.
		/// @return A pointer to the background color bitmap.
		BITMAP* GetBGColorBitmap() { return m_BGColorLayer->GetBitmap(); }

		/// Gets the material bitmap of this SLTerrain.
		/// @return A pointer to the material bitmap.
		BITMAP* GetMaterialBitmap() { return m_MainBitmap; }

		/// Gets a specific pixel from the foreground color bitmap of this.
		/// @param pixelX The X coordinate of the pixel to get.
		/// @param pixelY The Y coordinate of the pixel to get.
		/// @return An int specifying the requested pixel's foreground color index.
		int GetFGColorPixel(int pixelX, int pixelY) const { return m_FGColorLayer->GetPixel(pixelX, pixelY); }

		/// Sets a specific pixel on the foreground color bitmap of this SLTerrain to a specific color.
		/// @param pixelX The X coordinate of the pixel to set.
		/// @param pixelY The Y coordinate of the pixel to set.
		/// @param materialID The color index to set the pixel to.
		void SetFGColorPixel(int pixelX, int pixelY, const int materialID) const { m_FGColorLayer->SetPixel(pixelX, pixelY, materialID); }

		/// Gets a specific pixel from the background color bitmap of this.
		/// @param pixelX The X coordinate of the pixel to get.
		/// @param pixelY The Y coordinate of the pixel to get.
		/// @return An int specifying the requested pixel's background color index.
		int GetBGColorPixel(int pixelX, int pixelY) const { return m_BGColorLayer->GetPixel(pixelX, pixelY); }

		/// Sets a specific pixel on the background color bitmap of this SLTerrain to a specific color.
		/// @param pixelX The X coordinate of the pixel to set.
		/// @param pixelY The Y coordinate of the pixel to set.
		/// @param materialID The color index to set the pixel to.
		void SetBGColorPixel(int pixelX, int pixelY, int materialID) const { m_BGColorLayer->SetPixel(pixelX, pixelY, materialID); }

		/// Gets a specific pixel from the material bitmap of this SceneLayer.
		/// @param pixelX The X coordinate of the pixel to get.
		/// @param pixelY The Y coordinate of the pixel to get.
		/// @return An int specifying the requested pixel's material index.
		int GetMaterialPixel(int pixelX, int pixelY) const { return GetPixel(pixelX, pixelY); }

		/// Sets a specific pixel on the material bitmap of this SLTerrain to a specific material.
		/// @param pixelX The X coordinate of the pixel to set.
		/// @param pixelY The Y coordinate of the pixel to set.
		/// @param materialID The material index to set the pixel to.
		void SetMaterialPixel(int pixelX, int pixelY, int materialID) { SetPixel(pixelX, pixelY, materialID); }

		/// Indicates whether a terrain pixel is of Air or Cavity material.
		/// @param pixelX The X coordinate of the pixel to check.
		/// @param pixelY The Y coordinate of the pixel to check.
		/// @return Whether the terrain pixel is of Air or Cavity material.
		bool IsAirPixel(int pixelX, int pixelY) const;

		/// Checks whether a bounding box is completely buried in the terrain.
		/// @param checkBox The box to check.
		/// @return Whether the box is completely buried, i.e. no corner sticks out in the Air or Cavity.
		bool IsBoxBuried(const Box& checkBox) const;
#pragma endregion

#pragma region Concrete Methods
		/// Gets a deque of unwrapped boxes which show the areas where the material layer has had objects applied to it since last call to ClearUpdatedMaterialAreas().
		/// @return Reference to the deque that has been filled with Boxes which are unwrapped and may be out of bounds of the scene!
		std::deque<Box>& GetUpdatedMaterialAreas() { return m_UpdatedMaterialAreas; }

		/// Adds a notification that an area of the material terrain has been updated.
		/// @param newArea The Box defining the newly updated material area that can be unwrapped and may be out of bounds of the scene.
		void AddUpdatedMaterialArea(const Box& newArea) { m_UpdatedMaterialAreas.emplace_back(newArea); }

		/// Removes any color pixel in the color layer of this SLTerrain wherever there is an air material pixel in the material layer.
		void CleanAir();

		/// Removes any color pixel in the color layer of this SLTerrain wherever there is an air material pixel in the material layer inside the specified box.
		/// @param box Box to clean.
		/// @param wrapsX Whether the scene is X-wrapped.
		/// @param wrapsY Whether the scene is Y-wrapped.
		void CleanAirBox(const Box& box, bool wrapsX, bool wrapsY);

		/// Takes a BITMAP and scans through the pixels on this terrain for pixels which overlap with it. Erases them from the terrain and can optionally generate MOPixels based on the erased or 'dislodged' terrain pixels.
		/// @param sprite A pointer to the source BITMAP whose silhouette will be used as a cookie-cutter on the terrain.
		/// @param pos The position coordinates of the sprite.
		/// @param pivot The pivot coordinate of the sprite.
		/// @param rotation The sprite's current rotation in radians.
		/// @param scale The sprite's current scale coefficient.
		/// @param makeMOPs Whether to generate any MOPixels from the erased terrain pixels.
		/// @param skipMOP How many pixels to skip making MOPixels from, between each that gets made. 0 means every pixel turns into an MOPixel.
		/// @param maxMOPs The max number of MOPixels to make, if they are to be made.
		/// @return A deque filled with the MOPixels of the terrain that are now dislodged. This will be empty if makeMOPs is false. Note that ownership of all the MOPixels in the deque IS transferred!
		std::deque<MOPixel*> EraseSilhouette(BITMAP* sprite, const Vector& pos, const Vector& pivot, const Matrix& rotation, float scale, bool makeMOPs = true, int skipMOP = 2, int maxMOPs = 150);

		/// Returns the direction of the out-of-bounds "orbit" for this scene, where the brain must path to and where dropships/rockets come from.
		/// @return The orbit direction, either Up, Down, Left or Right..
		Directions GetOrbitDirection() { return m_OrbitDirection; }
#pragma endregion

#pragma region Virtual Override Methods
		/// Updates the state of this SLTerrain.
		void Update() override;

		/// Draws this SLTerrain's current scrolled position to a bitmap.
		/// @param targetBitmap The bitmap to draw to.
		/// @param targetBox The box on the target bitmap to limit drawing to, with the corner of box being where the scroll position lines up.
		/// @param offsetNeedsScrollRatioAdjustment Whether the offset of this SceneLayer or the passed in offset override need to be adjusted to scroll ratio.
		void Draw(BITMAP* targetBitmap, Box& targetBox, bool offsetNeedsScrollRatioAdjustment = false) override;
#pragma endregion

	private:
		static Entity::ClassInfo m_sClass; //!< ClassInfo for this class.

		int m_Width; //!< The width of this SLTerrain as determined by the main (material) bitmap, in pixels.
		int m_Height; //!< The height of this SLTerrain as determined by the main (material) bitmap, in pixels.

		std::unique_ptr<SceneLayer> m_FGColorLayer; //!< The foreground color layer of this SLTerrain.
		std::unique_ptr<SceneLayer> m_BGColorLayer; //!< The background color layer of this SLTerrain.

		LayerType m_LayerToDraw; //!< The layer of this SLTerrain that should be drawn to the screen when Draw() is called. See LayerType enumeration.

		ContentFile m_DefaultBGTextureFile; //!< The background texture file that will be used to texturize Materials that have no defined background texture.

		std::vector<TerrainFrosting*> m_TerrainFrostings; //!< The TerrainFrostings that need to be placed on this SLTerrain.
		std::vector<TerrainDebris*> m_TerrainDebris; //!< The TerrainDebris that need to be  placed on this SLTerrain.
		std::vector<TerrainObject*> m_TerrainObjects; //!< The TerrainObjects that need to be placed on this SLTerrain.

		std::deque<Box> m_UpdatedMaterialAreas; //!< List of areas of the material layer (main bitmap) which have been affected by new objects copied to it. These boxes are NOT wrapped, and can be out of bounds!

		Directions m_OrbitDirection; //!< The direction of the out-of-bounds "orbit" for this scene, where the brain must path to and where dropships/rockets come from.

		/// Applies Material textures to the foreground and background color layers, based on the loaded material layer (main bitmap).
		void TexturizeTerrain();

		/// Clears all the member variables of this SLTerrain, effectively resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		SLTerrain(const SLTerrain& reference) = delete;
		SLTerrain& operator=(const SLTerrain& rhs) = delete;
	};
} // namespace RTE
