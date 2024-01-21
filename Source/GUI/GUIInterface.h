#pragma once

// Header file for abstract classes used by the GUI library.

struct BITMAP;

namespace RTE {

#pragma region GUIBitmap
	/// An interface class inherited by the different types of bitmap methods.
	class GUIBitmap {

	public:
#pragma region Creation
		/// Constructor method used to instantiate a GUIBitmap object in system memory.
		GUIBitmap() = default;
#pragma endregion

#pragma region Destruction
		/// Destructor method used to clean up a GUIBitmap object before deletion from system memory.
		virtual ~GUIBitmap() = default;

		/// Destroys and resets the GUIBitmap object.
		virtual void Destroy() = 0;
#pragma endregion

#pragma region Getters and Setters
		/// Gets the path to the data file this GUIBitmap uses.
		/// @return >Path to the data file this GUIBitmap uses.
		virtual std::string GetDataPath() const = 0;

		/// Gets the underlying BITMAP of this GUIBitmap.
		/// @return The underlying BITMAP of this GUIBitmap.
		virtual BITMAP* GetBitmap() const = 0;

		/// Sets the underlying BITMAP for this GUIBitmap.
		/// @param newBitmap A pointer to the new BITMAP for this GUIBitmap.
		virtual void SetBitmap(BITMAP* newBitmap) = 0;

		/// Gets the width of the bitmap.
		/// @return The width of the bitmap.
		virtual int GetWidth() const = 0;

		/// Gets the height of the bitmap.
		/// @return The height of the bitmap.
		virtual int GetHeight() const = 0;

		/// Gets the number of bits per pixel color depth of the bitmap.
		/// @return The color depth of the bitmap.
		virtual int GetColorDepth() const = 0;

		/// Gets the color of a pixel at a specific point on the bitmap.
		/// @param posX X position on bitmap.
		/// @param posY Y position on bitmap.
		/// @return The color of the pixel at the specified point.
		virtual unsigned long GetPixel(int posX, int posY) const = 0;

		/// Sets the color of a pixel at a specific point on the bitmap.
		/// @param posX X position on bitmap.
		/// @param posY Y position on bitmap.
		/// @param pixelColor The color to set the pixel to.
		virtual void SetPixel(int posX, int posY, unsigned long pixelColor) = 0;

		/// Sets the color key (mask color) of the bitmap to the color of the pixel in the upper right corner of the bitmap.
		virtual void SetColorKey() {}

		/// Sets the color key (mask color) of the bitmap.
		/// @param colorKey Color key (mask color).
		virtual void SetColorKey(unsigned long colorKey) {}
#pragma endregion

#pragma region Clipping
		/// Gets the clipping rectangle of the bitmap.
		/// @param clippingRect Pointer to a GUIRect to fill out.
		virtual void GetClipRect(GUIRect* clippingRect) const = 0;

		/// Sets the clipping rectangle of the bitmap.
		/// @param clippingRect Pointer to a GUIRect to use as the clipping rectangle, or nullptr for no clipping.
		virtual void SetClipRect(GUIRect* clippingRect) = 0;

		/// Sets the clipping rectangle of the specified bitmap as the intersection of its current clipping rectangle and the rectangle described by the passed-in GUIRect.
		/// @param rect Rectangle pointer.
		virtual void AddClipRect(GUIRect* rect) = 0;
#pragma endregion

#pragma region Drawing
		/// Draw a section of this bitmap onto another bitmap.
		/// @param destBitmap Bitmap to draw onto.
		/// @param destX Destination X position.
		/// @param destY Destination Y position.
		/// @param srcPosAndSizeRect Source bitmap position and size rectangle.
		virtual void Draw(GUIBitmap* destBitmap, int destX, int destY, GUIRect* srcPosAndSizeRect) = 0;

		/// Draw a section of this bitmap onto another bitmap ignoring color-keyed pixels.
		/// @param destBitmap Bitmap to draw onto.
		/// @param destX Destination X position.
		/// @param destY Destination Y position.
		/// @param srcPosAndSizeRect Source bitmap position and size rectangle.
		virtual void DrawTrans(GUIBitmap* destBitmap, int destX, int destY, GUIRect* srcPosAndSizeRect) = 0;

		/// Draw this bitmap scaled onto another bitmap ignoring color-keyed pixels.
		/// @param destBitmap Bitmap to draw onto.
		/// @param destX Destination X position.
		/// @param destY Destination Y position.
		/// @param width Target width of the bitmap.
		/// @param height Target height of the bitmap.
		virtual void DrawTransScaled(GUIBitmap* destBitmap, int destX, int destY, int width, int height) = 0;
#pragma endregion

#pragma region Primitive Drawing
		/// Draws a line on this bitmap.
		/// @param x1 Start position on X axis.
		/// @param y1 Start position on Y axis.
		/// @param x2 End position on X axis.
		/// @param y2 End position on Y axis.
		/// @param color Color to draw this line with.
		virtual void DrawLine(int x1, int y1, int x2, int y2, unsigned long color) = 0;

		/// Draws a rectangle on this bitmap.
		/// @param posX Position on X axis.
		/// @param posY Position on Y axis.
		/// @param width Width of rectangle.
		/// @param height Height of rectangle.
		/// @param color Color to draw this rectangle with.
		/// @param filled Whether to fill the rectangle with the set color or not.
		virtual void DrawRectangle(int posX, int posY, int width, int height, unsigned long color, bool filled) = 0;
#pragma endregion

		// Disallow the use of some implicit methods.
		GUIBitmap& operator=(const GUIBitmap& rhs) = delete;
	};
#pragma endregion

#pragma region GUIScreen
	/// An interface class inherited by the different types of graphics methods.
	class GUIScreen {

	public:
#pragma region Creation
		/// Constructor method used to instantiate a GUIScreen object in system memory.
		GUIScreen() = default;

		/// Creates a bitmap from a file.
		/// @param fileName File name to create bitmap from.
		/// @return Pointer to the created bitmap.
		virtual GUIBitmap* CreateBitmap(const std::string& fileName) = 0;

		/// Creates an empty bitmap.
		/// @param width Bitmap width.
		/// @param height Bitmap height.
		/// @return Pointer to the created bitmap.
		virtual GUIBitmap* CreateBitmap(int width, int height) = 0;
#pragma endregion

#pragma region Destruction
		/// Destructor method used to clean up a GUIScreen object before deletion from system memory.
		virtual ~GUIScreen() = default;

		/// Destroys and resets the GUIScreen object.
		virtual void Destroy() = 0;
#pragma endregion

#pragma region Getters
		/// Gets the bitmap representing the screen.
		/// @return Pointer to the bitmap representing the screen.
		virtual GUIBitmap* GetBitmap() const = 0;
#pragma endregion

#pragma region Pure Virtual Methods
		/// Draws a bitmap onto the back buffer.
		/// @param guiBitmap The bitmap to draw to this AllegroScreen.
		/// @param destX Destination X position
		/// @param destY Destination Y position
		/// @param srcPosAndSizeRect Source bitmap position and size rectangle.
		virtual void DrawBitmap(GUIBitmap* guiBitmap, int destX, int destY, GUIRect* srcPosAndSizeRect) = 0;

		/// Draws a bitmap onto the back buffer ignoring color-keyed pixels.
		/// @param guiBitmap The bitmap to draw to this AllegroScreen.
		/// @param destX Destination X position
		/// @param destY Destination Y position
		/// @param srcPosAndSizeRect Source bitmap position and size rectangle.
		virtual void DrawBitmapTrans(GUIBitmap* guiBitmap, int destX, int destY, GUIRect* srcPosAndSizeRect) = 0;

		/// Converts an 8bit palette index to a valid pixel format color.
		/// @param color Color value in any bit depth. Will be converted to the format specified.
		/// @param targetColorDepth An optional target color depth that will determine what format the color should be converted to. If this is 0, then the current video color depth will be used as target.
		/// @return The converted color.
		virtual unsigned long ConvertColor(unsigned long color, int targetColorDepth = 0) = 0;
#pragma endregion

		// Disallow the use of some implicit methods.
		GUIScreen& operator=(const GUIScreen& rhs) = delete;
	};
#pragma endregion
} // namespace RTE
