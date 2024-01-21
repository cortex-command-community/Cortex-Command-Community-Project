#pragma once

#include "GUIInterface.h"
#include "ContentFile.h"

namespace RTE {

	/// Wrapper class to convert raw Allegro BITMAPs to GUI library bitmaps.
	class AllegroBitmap : public GUIBitmap {

	public:
#pragma region Creation
		/// Constructor method used to instantiate an AllegroBitmap object in system memory.
		AllegroBitmap();

		/// Constructor method used to instantiate an AllegroBitmap object in system memory and make it ready for use.
		/// @param bitmap The underlaying BITMAP of this AllegroBitmap. Ownership is NOT transferred!
		explicit AllegroBitmap(BITMAP* bitmap) {
			Clear();
			m_Bitmap = bitmap;
		}

		/// Creates an AllegroBitmap from a file.
		/// @param fileName File name to get the underlaying BITMAP from. Ownership is NOT transferred!
		void Create(const std::string& fileName);

		/// Creates an empty BITMAP that is owned by this AllegroBitmap.
		/// @param width Bitmap width.
		/// @param height Bitmap height.
		/// @param colorDepth Bitmap color depth (8 or 32).
		void Create(int width, int height, int colorDepth = 8);
#pragma endregion

#pragma region Destruction
		/// Destructor method used to clean up a AllegroBitmap object before deletion from system memory.
		~AllegroBitmap() override;

		/// Destroys and resets (through Clear()) the AllegroBitmap object.
		void Destroy() override;
#pragma endregion

#pragma region Getters and Setters
		/// Gets the path to the data file this AllegroBitmap uses.
		/// @return Path to the data file this AllegroBitmap uses.
		std::string GetDataPath() const override { return m_BitmapFile.GetDataPath(); }

		/// Gets the underlying BITMAP of this AllegroBitmap.
		/// @return The underlying BITMAP of this AllegroBitmap.
		BITMAP* GetBitmap() const override { return m_Bitmap; }

		/// Sets the underlying BITMAP for this AllegroBitmap. Ownership is NOT transferred.
		/// @param newBitmap A pointer to the new BITMAP for this AllegroBitmap.
		void SetBitmap(BITMAP* newBitmap) override {
			Destroy();
			m_Bitmap = newBitmap;
		}

		/// Gets the width of the bitmap.
		/// @return The width of the bitmap.
		int GetWidth() const override;

		/// Gets the height of the bitmap.
		/// @return The height of the bitmap.
		int GetHeight() const override;

		/// Gets the number of bits per pixel color depth of the bitmap.
		/// @return The color depth of the bitmap.
		int GetColorDepth() const override;

		/// Gets the color of a pixel at a specific point on the bitmap.
		/// @param posX X position on bitmap.
		/// @param posY Y position on bitmap.
		/// @return The color of the pixel at the specified point.
		unsigned long GetPixel(int posX, int posY) const override;

		/// Sets the color of a pixel at a specific point on the bitmap.
		/// @param posX X position on bitmap.
		/// @param posY Y position on bitmap.
		/// @param pixelColor The color to set the pixel to.
		void SetPixel(int posX, int posY, unsigned long pixelColor) override;
#pragma endregion

#pragma region Clipping
		/// Gets the clipping rectangle of the bitmap.
		/// @param clippingRect Pointer to a GUIRect to fill out.
		void GetClipRect(GUIRect* clippingRect) const override;

		/// Sets the clipping rectangle of the bitmap.
		/// @param clippingRect Pointer to a GUIRect to use as the clipping rectangle, or nullptr for no clipping.
		void SetClipRect(GUIRect* clippingRect) override;

		/// Sets the clipping rectangle of the bitmap as the intersection of its current clipping rectangle and the passed-in rectangle.
		/// @param clippingRect Pointer to a GUIRect to add to the existing clipping rectangle.
		void AddClipRect(GUIRect* clippingRect) override;
#pragma endregion

#pragma region Drawing
		/// Draw a section of this bitmap onto another bitmap
		/// @param destBitmap Bitmap to draw onto.
		/// @param destX Destination X position.
		/// @param destY Destination Y position.
		/// @param srcPosAndSizeRect Source bitmap position and size rectangle.
		void Draw(GUIBitmap* destBitmap, int destX, int destY, GUIRect* srcPosAndSizeRect) override;

		/// Draw a section of this bitmap onto another bitmap ignoring color-keyed pixels.
		/// @param destBitmap Bitmap to draw onto.
		/// @param destX Destination X position.
		/// @param destY Destination Y position.
		/// @param srcPosAndSizeRect Source bitmap position and size rectangle.
		void DrawTrans(GUIBitmap* destBitmap, int destX, int destY, GUIRect* srcPosAndSizeRect) override;

		/// Draw this bitmap scaled onto another bitmap ignoring color-keyed pixels.
		/// @param destBitmap Bitmap to draw onto.
		/// @param destX Destination X position.
		/// @param destY Destination Y position.
		/// @param width Target width of the bitmap.
		/// @param height Target height of the bitmap.
		void DrawTransScaled(GUIBitmap* destBitmap, int destX, int destY, int width, int height) override;
#pragma endregion

#pragma region Primitive Drawing
		/// Draws a line on this bitmap.
		/// @param x1 Start position on X axis.
		/// @param y1 Start position on Y axis.
		/// @param x2 End position on X axis.
		/// @param y2 End position on Y axis.
		/// @param color Color to draw this line with.
		void DrawLine(int x1, int y1, int x2, int y2, unsigned long color) override;

		/// Draws a rectangle on this bitmap.
		/// @param posX Position on X axis.
		/// @param posY Position on Y axis.
		/// @param width Width of rectangle.
		/// @param height Height of rectangle.
		/// @param color Color to draw this rectangle with.
		/// @param filled Whether to fill the rectangle with the set color or not.
		void DrawRectangle(int posX, int posY, int width, int height, unsigned long color, bool filled) override;
#pragma endregion

	private:
		BITMAP* m_Bitmap; //!< The underlaying BITMAP.
		ContentFile m_BitmapFile; //!< The ContentFile the underlaying BITMAP was created from, if created from a file.
		bool m_SelfCreated; //!< Whether the underlaying BITMAP was created by this and is owned.

		/// Clears all the member variables of this AllegroBitmap, effectively resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		AllegroBitmap& operator=(const AllegroBitmap& rhs) = delete;
	};
} // namespace RTE
