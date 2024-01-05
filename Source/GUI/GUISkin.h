#ifndef _GUISKIN_
#define _GUISKIN_

namespace RTE {

/// <summary>
/// Skin class used for the controls to get skin details.
/// </summary>
class GUISkin {

public:


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     GUISkin
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a GUISkin object in system
//                  memory.
// Arguments:       GUIScreen Interface.

    explicit GUISkin(GUIScreen *Screen);


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:        GUISkin
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to free a GUISkin object in system
//                  memory.
// Arguments:       None.

    ~GUISkin();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Load
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Loads a skin for a directory
// Arguments:       Skin directory and the file within to use

	bool Load(const std::string &directory, const std::string &fileName = "skin.ini");


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the data
// Arguments:       None.

    void Clear();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Frees the allocated data.
// Arguments:       None.

    void Destroy();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetValue
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a string value
// Arguments:       Section, Variable, String pointer

    bool GetValue(const std::string &Section, const std::string &Variable, std::string *Value);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetValue
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets an integer array of values
// Arguments:       Section, Variable, Integer array, max size of array
// Returns:         Number of elements read

    int GetValue(const std::string &Section, const std::string &Variable, int *Array, int MaxArraySize);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetValue
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a single integer value.
// Arguments:       Section, Variable, Integer pointer

    bool GetValue(const std::string &Section, const std::string &Variable, int *Value);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetValue
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a single unsigned integer value.
// Arguments:       Section, Variable, Unsigned Integer pointer

    bool GetValue(const std::string &Section, const std::string &Variable, unsigned long *Value);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CreateBitmap
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a blank bitmap.
// Arguments:       Width, Height.

    GUIBitmap * CreateBitmap(int Width, int Height);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CreateBitmap
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a bitmap from a filename.
// Arguments:       Filename.

    GUIBitmap * CreateBitmap(const std::string &Filename);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetFont
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a cached copy of a font. Ownership is NOT transferred!
// Arguments:       None.

    GUIFont * GetFont(const std::string &Name);


    /// <summary>
    /// Draws the mouse onto the screen.
    /// </summary>
    /// <param name="Image">Mouse image ID.</param>
    /// <param name="X">Horizontal position on the screen.</param>
    /// <param name="Y">Vertical position on the screen.</param>
	/// <param name="pScreen">The GUIScreen to draw to, overriding the one passed in on construction.</param>
    void DrawMouse(int Image, int X, int Y, GUIScreen *guiScreenOverride = nullptr);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          BuildStandardRect
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Builds a bitmap from a standard skin property section.
// Arguments:       Destination bitmap, Section name, Position, Size. Whether to draw the
//                  background and frame, a GUIRect to be filled in with the border sizes of the four sides of the built standard rect.

    void BuildStandardRect(GUIBitmap *Dest, const std::string &Section, int X, int Y, int Width, int Height, bool buildBG = true, bool buildFrame = true, GUIRect *borderSizes = nullptr);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ConvertColor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Converts an 8bit palette index to a valid pixel format.
//                  Primarily used for development in windowed mode.
// Arguments:       Color value in any bit depth. Will be converted to the format specified.
//                  An optional target color depth that will determine what format the color
//                  should be converted to. If this is 0, then the current video color depth
//                  will be used as target.

    unsigned long ConvertColor(unsigned long color, int targetDepth = 0);


private:

    std::string m_Directory;
    GUIScreen *m_Screen;
    GUIBitmap *m_MousePointers[3];

    std::vector<GUIProperties *> m_PropList;
    std::vector<GUIBitmap *> m_ImageCache;
    std::vector<GUIFont *> m_FontCache;

	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          LoadMousePointer
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Loads a mouse pointer image & details
	// Arguments:       Section name.

	GUIBitmap * LoadMousePointer(const std::string &Section);
};
};
#endif