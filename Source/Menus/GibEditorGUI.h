#pragma once

/// GibEditorGUI class
/// @author Daniel Tabar
/// dtabar@datarealms.com
/// http://www.datarealms.com
/// Inclusions of header files
// #include "FrameMan.h"
#include "Timer.h"
#include "Vector.h"
#include "Controller.h"
#include "PieSlice.h"

#include <list>
#include <memory>

struct BITMAP;

namespace RTE {

	class MovableObject;
	class MOSRotating;
	class ObjectPickerGUI;
	class PieMenu;

	/// A placement part of the gib editor which manages the pie menu and picker.
	class GibEditorGUI {

		/// Public member variable, method and friend function declarations
	public:
		// Different modes of this editor
		enum EditorGUIMode {
			INACTIVE = 0,
			PICKOBJECTTOLOAD,
			PICKINGGIB,
			ADDINGGIB,
			PLACINGGIB,
			MOVINGGIB,
			DELETINGGIB,
			PLACEINFRONT,
			PLACEBEHIND,
			DONEEDITING,
			EDITORGUIMODECOUNT
		};

		/// Constructor method used to instantiate a GibEditorGUI object in system
		/// memory. Create() should be called before using the object.
		GibEditorGUI();

		/// Destructor method used to clean up a GibEditorGUI object before deletion
		/// from system memory.
		~GibEditorGUI();

		/// Makes the GibEditorGUI object ready for use.
		/// @param pController A poitner to a Controller which will control this Menu. Ownership is
		/// @param whichModuleSpace NOT TRANSFERRED! (default: -1)
		/// Which module space that this eidtor will be able to pick objects from.
		/// -1 means all modules.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create(Controller* pController, int whichModuleSpace = -1);

		/// Resets the entire GibEditorGUI, including its inherited members, to
		/// their default settings or values.
		void Reset() { Clear(); }

		/// Destroys and resets (through Clear()) the GibEditorGUI object.
		void Destroy();

		/// Sets the controller used by this. The ownership of the controller is
		/// NOT transferred!
		/// @param pController The new controller for this menu. Ownership is NOT transferred
		void SetController(Controller* pController);

		/// Sets where on the screen that this GUI is being drawn to. If upper
		/// left corner, then 0, 0. This will affect the way the mouse is positioned
		/// etc.
		/// @param newPosX The new screen position of this entire GUI.
		void SetPosOnScreen(int newPosX, int newPosY);

		/// Sets the absolute scene coordinates of the cursor of this Editor.
		/// @param newCursorPos The new cursor position in absolute scene units.
		void SetCursorPos(const Vector& newCursorPos) { m_CursorPos = newCursorPos; }

		/// Sets the new Object to be held at the gib cursor of this Editor. Ownership
		/// IS transferred!
		/// @param pNewGibObject The new Object to be held by the cursor. Ownership IS transferred!
		void SetCurrentGib(MovableObject* pNewGibObject) { m_pCurrentGib = pNewGibObject; }

		/// Gets any Pie menu slice command activated last update.
		/// @return The enum'd int of any slice activated. See the PieSliceType enum.
		PieSliceType GetActivatedPieSlice() const;

		/// Gets the list of already placed gibs of the currently edited object.
		/// Ownership of neither list not objects IS NOT transferred!
		/// @return The current list of placed gibs. OWNERSHIP IS NOT TRANSFERRED!
		std::list<MovableObject*>* GetPlacedGibs() { return &m_PlacedGibs; }

		/// Gets the currently held Object in the cursor of this Editor. Ownership
		/// IS NOT transferred!
		/// @return The currently held object, if any. OWNERSHIP IS NOT TRANSFERRED!
		const MovableObject* GetCurrentGib() { return m_pCurrentGib; }

		/// Returns a refernece instance of an object if it has been picked to be
		/// loaded into the editor. Ownership is NOT transferred.
		/// @return Reference instance of the picked object to be loaded. 0 if nothing. OWNERSHIP IS NOT TRANSFERRED!
		/// was picked since last update.
		const MOSRotating* GetObjectToLoad() { return m_pObjectToLoad; }

		/// Sets the current mode of this editor.
		/// @param newMode The new mode to set to, see the EditorGUIMode enum.
		void SetEditorGUIMode(EditorGUIMode newMode) { m_EditorGUIMode = newMode; }

		/// Gets the current mode of this editor.
		/// @return The current mode this is set to; see the EditorGUIMode enum.
		EditorGUIMode GetEditorGUIMode() const { return m_EditorGUIMode; }

		/// Sets which DataModule space to be picking objects from. If -1, then
		/// let the player pick from all loaded modules.
		/// @param moduleSpaceID The ID of the module to let the player pick objects from. All official (default: -1)
		/// modules' objects will alwayws be presented, in addition to the one
		/// passed in here.
		void SetModuleSpace(int moduleSpaceID = -1);

		/// Shows whether an edit on the scene was made in the last Update.
		/// @return Whether any edit was made.
		bool EditMade() const { return m_EditMade; }

		/// Updates the state of this Menu each frame
		void Update();

		/// Draws the editor
		/// @param pTargetBitmap The bitmap to draw on.
		/// @param targetPos The absolute position of the target bitmap's upper left corner in the scene. (default: Vector())
		void Draw(BITMAP* pTargetBitmap, const Vector& targetPos = Vector()) const;

		/// Protected member variable and method declarations
	protected:
		/// Adds a MovableObject to be placed in the editor. Ownership IS transferred!
		/// @param pObjectToAdd The MovableOjbect instace to add, OIT!
		/// @param listOrder Where in the list the object should be inserted. -1 means at the end (default: -1)
		/// of the list.
		void AddPlacedObject(MovableObject* pObjectToAdd, int listOrder = -1);

		/// Removes a MovableObject placed in this editor.
		/// @param whichToRemove The list order number of the object to remove. If -1, the last one is removed. (default: -1)
		void RemovePlacedObject(int whichToRemove = -1);

		/// Returns the last placed object that graphically overlaps an absolute
		/// point in the scene. Note that the placed object don't really exist in
		/// the scene, but in the editor. Their own Pos's are used.
		/// @param scenePoint The point in absolute scene coordinates that will be used to pick the
		/// last placed MovableObject which overlaps it.
		/// @param pListOrderPlace An int which will be filled out with the order place of any found object (default: 0)
		/// in the list. if nothing is found, it will get a value of -1.
		/// @return The last hit MovableObject, if any. Ownership is NOT transferred!
		const MovableObject* PickPlacedObject(Vector& scenePoint, int* pListOrderPlace = 0) const;

		/// Updated the objects in the placed scene objects list of this. This is
		/// mostly for the editor to represent the items correctly.
		void UpdatePlacedObjects();

		enum BlinkMode {
			NOBLINK = 0,
			OBJECTBLINKON,
			OBJECTBLINKOFF,
			BLINKMODECOUNT
		};

		// Controller which conrols this menu. Not owned
		Controller* m_pController;
		// Whether an editor was made to the Scene in the last Update
		bool m_EditMade;
		// The current mode of the whole GUI. See EditorGUIMode enum.
		EditorGUIMode m_EditorGUIMode;
		// The previous mode of the whole GUI, to go back to when the current mode is done in some cases
		EditorGUIMode m_PreviousMode;
		// The ref instance picked to be loaded into the editor from the picker. 0 if none has been yet. Not owned.
		const MOSRotating* m_pObjectToLoad;
		// Notification blink timer
		Timer m_BlinkTimer;
		// What we're blinking
		int m_BlinkMode;
		// Measures the time to when to start repeating inputs when they're held down
		Timer m_RepeatStartTimer;
		// Measures the interval between input repeats
		Timer m_RepeatTimer;

		std::unique_ptr<PieMenu> m_PieMenu; //!< The PieMenu for this GibEditorGUI.
		// The object picker
		ObjectPickerGUI* m_pPicker;
		// Grid snapping enabled
		bool m_GridSnapping;
		// The zooming bitmaps owned by this; source gets the area from the screen, destination is the zoomed in view
		BITMAP* m_pZoomSource;
		// The zoom factor of the magnifying window. 0 means no zoom window
		int m_ZoomFactor;
		// Current cursor position, in absolute scene coordinates
		Vector m_CursorPos;
		// The offset from the current object's position to the cursor, if any
		Vector m_CursorOffset;
		// Cursor position in free air, or over something
		bool m_CursorInAir;
		// Gib facing left or not when placing
		bool m_FacingLeft;
		// List of all the gibs currently placed on the edited object.
		// They are proxies of the actual gib list of the edited object, and are OWNED by editor.
		std::list<MovableObject*> m_PlacedGibs;
		// Currently held object to place as a gib. This is what is attached to the cursor and will be placed when the fire button is pressed
		// OWNED by this.
		MovableObject* m_pCurrentGib;
		// Where in the scene's list order the next object should be placed. If -1, then place at the end of the list.
		int m_GibListOrder;
		// Whether to draw the currently held object
		bool m_DrawCurrentGib;
		// Currently placed scene object to make blink when drawing it. NOT OWNED.
		const MovableObject* m_pObjectToBlink;

		/// Private member variable and method declarations
	private:
		/// Clears all the member variables of this GibEditorGUI, effectively
		/// resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		GibEditorGUI(const GibEditorGUI& reference) = delete;
		GibEditorGUI& operator=(const GibEditorGUI& rhs) = delete;
	};

} // namespace RTE
