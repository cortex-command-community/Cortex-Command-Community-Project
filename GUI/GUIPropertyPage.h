#ifndef _GUIPROPERTYPAGE_
#define _GUIPROPERTYPAGE_

#include "GUITextPanel.h"
#include "GUIScrollPanel.h"

namespace RTE {

/// <summary>
/// A property page control class.
/// </summary>
class GUIPropertyPage : public GUIControl, public GUIPanel {

public:

    // PropertyPage Notifications
    enum {
        Changed = 0,        // Any text panel has changed. Property values are NOT updated
        Enter                // A text panel has lost focus or the enter key was hit
    } Notification;


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     GUIPropertyPage
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a GUIPropertyPage object in
//                  system memory.
// Arguments:       GUIManager, GUIControlManager.

    GUIPropertyPage(GUIManager *Manager, GUIControlManager *ControlManager);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the control has been created.
// Arguments:       Name, Position.

    void Create(const std::string &Name, int X, int Y, int Width = -1, int Height = -1) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the control has been destroyed.
// Arguments:       None.

    void Destroy() override;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the control has been created.
// Arguments:       Properties.

    void Create(GUIProperties *Props) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ChangeSkin
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the skin has been changed.
// Arguments:       New skin pointer.

    void ChangeSkin(GUISkin *Skin) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws the panel
// Arguments:       Screen class

    void Draw(GUIScreen *Screen) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  OnMouseDown
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the mouse goes down on the panel
// Arguments:       Mouse Position, Mouse Buttons, Modifier.

    void OnMouseDown(int X, int Y, int Buttons, int Modifier) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  OnMouseUp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the mouse goes up on the panel
// Arguments:       Mouse Position, Mouse Buttons, Modifier.

    void OnMouseUp(int X, int Y, int Buttons, int Modifier) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnMouseMove
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the mouse moves (over the panel, or when captured).
// Arguments:       Mouse Position, Mouse Buttons, Modifier.

    void OnMouseMove(int X, int Y, int Buttons, int Modifier) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnMouseEnter
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the mouse enters the panel.
// Arguments:       Mouse Position, Mouse Buttons, Modifier.

    void OnMouseEnter(int X, int Y, int Buttons, int Modifier) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnMouseLeave
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the mouse leaves the panel.
// Arguments:       Mouse Position, Mouse Buttons, Modifier.

    void OnMouseLeave(int X, int Y, int Buttons, int Modifier) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPanel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the panel of the control.
// Arguments:       None.
// Returns:         0 if the control does not have a panel, otherwise the topmost panel.

    GUIPanel * GetPanel() override;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetControlID
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns a string representing the control's ID
// Arguments:       None.

    static std::string GetControlID()    { return "PROPERTYPAGE"; };


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetControlRect
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the rectangle of the control.
// Arguments:       Position, Size.

    void GetControlRect(int *X, int *Y, int *Width, int *Height) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          StoreProperties
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the control to store the values into properties.
// Arguments:       None.

    void StoreProperties() override;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Move
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the control needs to be moved.
// Arguments:       New position.

    void Move(int X, int Y) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Resize
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the control needs to be resized.
// Arguments:       New size.

    void Resize(int Width, int Height) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetPropertyValues
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Refreshes the page with new variables & values.
// Arguments:       GUIProperties.

    void SetPropertyValues(GUIProperties *Props);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPropertyValues
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the properties in the page.
// Arguments:       None.

    GUIProperties * GetPropertyValues();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ReceiveSignal
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when receiving a signal.
// Arguments:       Signal source, Signal code, Signal data.

    void ReceiveSignal(GUIPanel *Source, int Code, int Data) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ClearValues
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears the property page values.
// Arguments:       None.

    void ClearValues();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          InvokeUpdate
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Invokes an explicit update on text panels to property page.
// Arguments:       None.
// Returns:         Boolean whether or not any values have changed.

    bool InvokeUpdate();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          HasTextFocus
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks if any of the visible text panels have focus.
// Arguments:       None.

    bool HasTextFocus();

private:

    GUIBitmap *m_DrawBitmap;
    unsigned long m_LineColor;

    GUIProperties m_PageValues;
    std::vector<GUITextPanel *> m_TextPanelList;
    GUIScrollPanel *m_VertScroll;

	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          BuildBitmap
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Create the property page bitmap to draw.
	// Arguments:       None.

	void BuildBitmap();
};
};
#endif