#pragma once

namespace RTE {

	/// A class to handle the drawing of text.
	class GUIFont {

	public:
		// Horizontal Text Alignment,
		enum {
			Left = 0,
			Centre,
			Right
		} HAlignment;

		// Vertical Text Alignment,
		enum {
			Top = 0,
			Middle,
			Bottom
		} VAlignment;

		// Character structure
		typedef struct {
			int m_Width;
			int m_Height;
			int m_Offset;
		} Character;

		// Font Color structure
		typedef struct {
			unsigned long m_Color;
			GUIBitmap* m_Bitmap;
		} FontColor;

		/// Constructor method used to instantiate a GUIFont object in system
		/// memory.
		explicit GUIFont(const std::string& Name);

		/// Loads the font from an image file.
		/// @param Screen Screen class, Filename of image.
		bool Load(GUIScreen* Screen, const std::string& Filename);

		/// Pre-Calculates the font using a specific color.
		/// @param Color Color.
		void CacheColor(unsigned long Color);

		/// Finds a font color structure from the cache.
		/// @param Color Color.
		FontColor* GetFontColor(unsigned long Color);

		/// Draws text to a bitmap.
		/// @param Bitmap Bitmap, Position, Text, Color, Drop-shadow, 0 = none.
		void Draw(GUIBitmap* Bitmap, int X, int Y, const std::string& Text, unsigned long Shadow = 0);

		/// Draws text to a bitmap aligned.
		/// @param Bitmap Bitmap, Position, Text.
		void DrawAligned(GUIBitmap* Bitmap, int X, int Y, const std::string& Text, int HAlign, int VAlign = Top, int maxWidth = 0, unsigned long Shadow = 0);

		/// Sets the current color.
		/// @param Color Color.
		void SetColor(unsigned long Color);

		/// Calculates the width of a piece of text.
		/// @param Text Text.
		int CalculateWidth(const std::string& Text);

		/// Calculates the width of a piece of text.
		/// @param Character Character.
		int CalculateWidth(const char Character);

		/// Calculates the height of a piece of text, if it's wrapped within a
		/// max width.
		/// @param Text Text, and the max width. If 0, no wrapping is done.
		int CalculateHeight(const std::string& Text, int MaxWidth = 0);

		/// Gets the font height.
		int GetFontHeight() const;

		/// Gets the name of the font
		std::string GetName() const;

		/// Destroys the font data
		void Destroy();

		/// Get the character kerning (spacing)
		int GetKerning() const;

		/// Set the character kerning (spacing), in pixels. 1 = one empty pixel
		/// between chars, 0 = chars are touching.
		void SetKerning(int newKerning = 1) { m_Kerning = newKerning; }

	private:
		GUIBitmap* m_Font;
		GUIScreen* m_Screen;
		std::vector<FontColor> m_ColorCache;

		int m_FontHeight;
		unsigned long m_MainColor;
		unsigned long m_CurrentColor;
		GUIBitmap* m_CurrentBitmap;
		std::string m_Name;
		Character m_Characters[256];

		int m_CharIndexCap; // The highest index of valid characters that was read in from the file

		int m_Kerning; // Spacing between characters
		int m_Leading; // Spacing between lines
	};
} // namespace RTE
