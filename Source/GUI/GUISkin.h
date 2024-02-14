#pragma once

namespace RTE {

	/// Skin class used for the controls to get skin details.
	class GUISkin {

	public:
		/// Constructor method used to instantiate a GUISkin object in system
		/// memory.
		/// @param Screen GUIScreen Interface.
		explicit GUISkin(GUIScreen* Screen);

		/// Loads a skin for a directory
		/// @param directory Skin directory and the file within to use
		bool Load(const std::string& directory, const std::string& fileName = "skin.ini");

		/// Clears all the data
		void Clear();

		/// Frees the allocated data.
		void Destroy();

		/// Gets a string value
		/// @param Section Section, Variable, String pointer
		bool GetValue(const std::string& Section, const std::string& Variable, std::string* Value);

		/// Gets an integer array of values
		/// @param Section Section, Variable, Integer array, max size of array
		/// @return Number of elements read
		int GetValue(const std::string& Section, const std::string& Variable, int* Array, int MaxArraySize);

		/// Gets a single integer value.
		/// @param Section Section, Variable, Integer pointer
		bool GetValue(const std::string& Section, const std::string& Variable, int* Value);

		/// Gets a single unsigned integer value.
		/// @param Section Section, Variable, Unsigned Integer pointer
		bool GetValue(const std::string& Section, const std::string& Variable, unsigned long* Value);

		/// Creates a blank bitmap.
		/// @param Width Width, Height.
		GUIBitmap* CreateBitmap(int Width, int Height);

		/// Creates a bitmap from a filename.
		/// @param Filename Filename.
		GUIBitmap* CreateBitmap(const std::string& Filename);

		/// Gets a cached copy of a font. Ownership is NOT transferred!
		GUIFont* GetFont(const std::string& Name);

		/// Draws the mouse onto the screen.
		/// @param Image Mouse image ID.
		/// @param X Horizontal position on the screen.
		/// @param Y Vertical position on the screen.
		/// @param pScreen The GUIScreen to draw to, overriding the one passed in on construction.
		void DrawMouse(int Image, int X, int Y, GUIScreen* guiScreenOverride = nullptr);

		/// Builds a bitmap from a standard skin property section.
		/// @param Dest Destination bitmap, Section name, Position, Size. Whether to draw the
		/// background and frame, a GUIRect to be filled in with the border sizes of the four sides of the built standard rect.
		void BuildStandardRect(GUIBitmap* Dest, const std::string& Section, int X, int Y, int Width, int Height, bool buildBG = true, bool buildFrame = true, GUIRect* borderSizes = nullptr);

		/// Converts an 8bit palette index to a valid pixel format.
		/// Primarily used for development in windowed mode.
		/// @param color Color value in any bit depth. Will be converted to the format specified.
		/// @param targetDepth An optional target color depth that will determine what format the color (default: 0)
		/// should be converted to. If this is 0, then the current video color depth
		/// will be used as target.
		unsigned long ConvertColor(unsigned long color, int targetDepth = 0);

	private:
		std::string m_Directory;
		GUIScreen* m_Screen;
		GUIBitmap* m_MousePointers[3];

		std::vector<GUIProperties*> m_PropList;
		std::vector<GUIBitmap*> m_ImageCache;
		std::vector<GUIFont*> m_FontCache;

		/// Loads a mouse pointer image & details
		/// @param Section Section name.
		GUIBitmap* LoadMousePointer(const std::string& Section);
	};
} // namespace RTE
