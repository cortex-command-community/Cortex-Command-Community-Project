#pragma once

#include "AllegroBitmap.h"

namespace RTE {

	/// Wrapper class to convert raw Allegro BITMAPs to GUI library backbuffer bitmaps.
	class AllegroScreen : public GUIScreen {

	public:
#pragma region Creation
		/// Constructor method used to instantiate an AllegroScreen object in system memory and make it ready for use.
		/// @param backBuffer A bitmap that represents the back buffer. Ownership is NOT transferred!
		explicit AllegroScreen(BITMAP* backBuffer);

		/// Creates a bitmap from a file.
		/// @param fileName File name to create bitmap from.
		/// @return Pointer to the created bitmap. Ownership IS transferred!
		GUIBitmap* CreateBitmap(const std::string& fileName) override;

		/// Creates an empty bitmap.
		/// @param width Bitmap width.
		/// @param height Bitmap height.
		/// @return Pointer to the created bitmap. Ownership IS transferred!
		GUIBitmap* CreateBitmap(int width, int height) override;
#pragma endregion

#pragma region Destruction
		/// Destructor method used to clean up a AllegroScreen object before deletion from system memory.
		~AllegroScreen() override;

		/// Destroys the AllegroScreen object.
		void Destroy() override { m_BackBufferBitmap.reset(); }
#pragma endregion

#pragma region Getters
		/// Gets the bitmap representing the screen.
		/// @return Pointer to the bitmap representing the screen. Ownership is NOT transferred!
		GUIBitmap* GetBitmap() const override { return m_BackBufferBitmap.get(); }
#pragma endregion

#pragma region Drawing
		/// Draws a bitmap onto the back buffer.
		/// @param guiBitmap The bitmap to draw to this AllegroScreen.
		/// @param destX Destination X position
		/// @param destY Destination Y position
		/// @param srcPosAndSizeRect Source bitmap position and size rectangle.
		void DrawBitmap(GUIBitmap* guiBitmap, int destX, int destY, GUIRect* srcPosAndSizeRect) override;

		/// Draws a bitmap onto the back buffer ignoring color-keyed pixels.
		/// @param guiBitmap The bitmap to draw to this AllegroScreen.
		/// @param destX Destination X position
		/// @param destY Destination Y position
		/// @param srcPosAndSizeRect Source bitmap position and size rectangle.
		void DrawBitmapTrans(GUIBitmap* guiBitmap, int destX, int destY, GUIRect* srcPosAndSizeRect) override;
#pragma endregion

#pragma region Virtual Override Methods
		/// Converts an 8bit palette index to a valid pixel format color.
		/// @param color Color value in any bit depth. Will be converted to the format specified.
		/// @param targetColorDepth An optional target color depth that will determine what format the color should be converted to. If this is 0, then the current video color depth will be used as target.
		/// @return The converted color.
		unsigned long ConvertColor(unsigned long color, int targetColorDepth = 0) override;
#pragma endregion

	private:
		std::unique_ptr<AllegroBitmap> m_BackBufferBitmap; //!< The AllegroBitmap that makes this AllegroScreen.

		// Disallow the use of some implicit methods.
		AllegroScreen& operator=(const AllegroScreen& rhs) = delete;
	};
} // namespace RTE
