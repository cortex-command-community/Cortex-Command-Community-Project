#pragma once

#include "Timer.h"
#include "Vector.h"

#include "GUI.h"
#include "GUIControlManager.h"
#include "AllegroScreen.h"
#include "GUIInputWrapper.h"

#include <array>
#include <deque>
#include <memory>
#include <vector>
#include <utility>

namespace RTE {

	class Controller;
	class MovableObject;
	class Actor;
	class Icon;
	class AllegroBitmap;
	class GUIFont;
	class GUICollectionBox;
	class GUILabel;
	class GUIButton;
	class GUIScrollbar;

	/// A GUI menu for managing inventories.
	class InventoryMenuGUI {

	public:
		/// Enumeration for the modes an InventoryMenuGUI can have.
		enum class MenuMode {
			Carousel,
			Full,
			Transfer
		};

#pragma region Creation
		/// Constructor method used to instantiate an InventoryMenuGUI object in system memory. Create() should be called before using the object.
		InventoryMenuGUI();

		/// Makes the InventoryMenuGUI object ready for use.
		/// @param activityPlayerController A pointer to a Controller which will control this Menu. Ownership is NOT transferred!
		/// @param inventoryActor The Actor whose inventory this GUI will display. Ownership is NOT transferred!
		/// @param menuMode The mode this menu should use when it's enabled. Defaults to Carousel.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(Controller* activityPlayerController, Actor* inventoryActor = nullptr, MenuMode menuMode = MenuMode::Carousel);
#pragma endregion

#pragma region Destruction
		/// Destructor method used to clean up an InventoryMenuGUI object before deletion from system memory.
		~InventoryMenuGUI();

		/// Destroys and resets the InventoryMenuGUI object.
		void Destroy();
#pragma endregion

#pragma region Getters and Setters
		/// Sets the controller used by this. The ownership of the controller is NOT transferred!
		/// @param controller The new controller for this menu. Ownership is NOT transferred!
		void SetController(Controller* controller) { m_MenuController = controller; }

		/// Gets the Actor whose inventory this GUI will display. The ownership of the Actor is NOT transferred!
		/// @return The Actor whose inventory this GUI will display. Ownership is NOT transferred!
		const Actor* GetInventoryActor() const { return m_InventoryActor; }

		/// Sets the Actor whose inventory this GUI will display. The ownership of the Actor is NOT transferred!
		/// @param actor The new Actor whose inventory this GUI will display. Ownership is NOT transferred!
		void SetInventoryActor(Actor* newInventoryActor);

		/// Gets the MenuMode this InventoryMenuGUI is currently in.
		/// @return The current MenuMode of this InventoryMenuGUI.
		MenuMode GetMenuMode() const { return m_MenuMode; }

		/// Sets the MenuMode for this InventoryMenuGUI to be in.
		/// @param newMenuMode The new MenuMode of this InventoryMenuGUI.
		void SetMenuMode(MenuMode newMenuMode) { m_MenuMode = newMenuMode; }

		/// Gets whether the menu is enabled or not.
		/// @return Whether the menu is enabled.
		bool IsEnabled() const { return m_EnabledState == EnabledState::Enabled || m_EnabledState == EnabledState::Enabling; }

		/// Gets whether the menu is enabled and not in carousel mode or not.
		/// @return Whether the menu is enabled and not in carousel mode
		bool IsEnabledAndNotCarousel() const { return IsEnabled() && m_MenuMode != MenuMode::Carousel; }

		/// Gets whether the menu is in the process of enabling or disabling.
		/// @return
		bool IsEnablingOrDisabling() const { return m_EnabledState == EnabledState::Enabling || m_EnabledState == EnabledState::Disabling; }

		/// Gets whether the menu is at all visible or not.
		/// @return Whether the menu is visible.
		bool IsVisible() const { return m_EnabledState != EnabledState::Disabled; }

		/// Enables or disables the menu and animates it in and out of view.
		/// @param enable Whether to enable or disable the menu.
		void SetEnabled(bool enable);

		/// Enables or disables the InventoryMenuGUI based on whether or not it's empty (i.e. its actor has equipped or inventory items).
		/// @return Whether it tried to enable InventoryMenuGUI (true) or disable it (false).
		bool EnableIfNotEmpty();
#pragma endregion

#pragma region Full And Transfer Mode Getters and Setters
		/// Clears the selected item for this InventoryMenuGUI.
		void ClearSelectedItem();

		/// Sets the selected item for this InventoryMenuGUI.
		/// If the button pointer or object pointer are null, or both indices are null, the selected item will instead be cleared.
		/// @param selectedItemButton A pointer to the GUIButton that is selected.
		/// @param selectedItemObject A pointer to the MovableObject that is selected.
		/// @param inventoryIndex The index of this selected item in the displayed inventory. -1 means the item is not in the inventory.
		/// @param equippedItemIndex The index of this selected item in the vector of equipped items. -1 means the item is not in the inventory.
		/// @param isBeingDragged Whether or not the selected item is being dragged.
		void SetSelectedItem(GUIButton* selectedItemButton, MovableObject* selectedItemObject, int inventoryIndex, int equippedItemIndex, bool isBeingDragged);
#pragma endregion

#pragma region Updating
		/// Updates the state of this GUI each frame.
		void Update();

		/// Draws the GUI.
		/// @param targetBitmap A pointer to a BITMAP to draw on. Generally a screen BITMAP.
		/// @param targetPos The absolute position of the target bitmap's upper left corner in the scene.
		void Draw(BITMAP* targetBitmap, const Vector& targetPos = Vector()) const;
#pragma endregion

	private:
		/// A struct containing all information required to drawn and animate a carousel item box in Carousel MenuMode.
		struct CarouselItemBox {
			MovableObject* Item; //!< A pointer to the item being displayed in the CarouselItemBox.
			bool IsForEquippedItems; //!< Whether or not this CarouselItemBox is for displaying equipped items.
			Vector FullSize; //!< The full size for this CarouselItemBox when it's not animating.
			Vector CurrentSize; //!< The current size of this CarouselItemBox.
			Vector Pos; //!< The position this CarouselItemBox should be drawn at.
			Vector IconCenterPosition; //!< The calculated position for the center of the icon this CarouselItemBox is displaying.
			std::pair<bool, bool> RoundedAndBorderedSides; //!< Whether the left and right sides, respectively, of this CarouselItemBox should have rounded corners and a border.

			/// Fills the passed in vector of Bitmaps and float with the appropriate graphical icons and mass for the Item of this CarouselItemBox.
			/// If this CarouselItemBox IsForEquippedItem, it will instead used the passed in pointer to the vector of equipped items.
			/// @param itemIcons A vector of Bitmaps to be filled in with the icon(s) for this CarouselItemBox's item(s).
			/// @param totalItemMass A float to be filled in with the total mass of this CarouselItemBox's item(s).
			/// @param equippedItems A pointer to the vector of sets of equipped items to be used if the CarouselItemBox IsForEquippedItem.
			void GetIconsAndMass(std::vector<BITMAP*>& itemIcons, float& totalItemMass, const std::vector<std::pair<MovableObject*, MovableObject*>>* equippedItems) const;
		};

		/// A struct containing all information required to describe a selected item in Full/Transfer MenuMode.
		struct GUISelectedItem {
			GUIButton* Button; //!< A pointer to the button for this GUISelectedItem.
			MovableObject* Object; //!< A pointer to the MovableObject for this GUISelectedItem. Should always match up with what the Button is displaying.
			int InventoryIndex; //!< The index in the InventoryItemsBox that this GUISelectedItem is for. Either this or the EquippedItemIndex must have a value.
			int EquippedItemIndex; //!< The index in the EquippedItemIndex that this GUISelectedItem is for. Either this or the InventoryIndex must have a value.
			bool IsBeingDragged; //!< Whether or not the GUISelectedItem is currently being dragged.
			int DragHoldCount; //!< How long dragging has been held for, used to determine if the GUISelectedItem was actually dragged or just clicked.

			/// Whether this selected item was being dragged for long enough that it matters. This helps make dragging not cause problems during instant clicks and releases.
			/// @return Whether this selected item was being dragged for long enough that it matters.
			bool DragWasHeldForLongEnough() const { return IsBeingDragged && DragHoldCount > 10; }
		};

		/// Enumeration for enabled states when enabling/disabling the InventoryMenuGUI.
		enum class EnabledState {
			Enabling,
			Enabled,
			Disabling,
			Disabled
		};

		/// Enumeration for which direction an inventory is being swapped in.
		enum class CarouselAnimationDirection {
			Left = -1,
			None,
			Right
		};

		static constexpr int c_ItemsPerRow = 5; //!< The number of items per row of the inventory display. MUST be an odd number. Used in all MenuModes.
		static constexpr int c_MinimumItemPadding = 1; //!< The padding between item icons and their containing boxes. Items will have at least this much padding on all sides. Used in all MenuModes.

		static constexpr int c_CarouselMenuVerticalOffset = 85; //!< How high above its target the carousel will be. Used in Carousel MenuModes.
		static const Vector c_CarouselBoxMaxSize; //!< The size of the largest item box in the carousel, i.e. the one in the middle. Used in Carousel MenuMode.
		static const Vector c_CarouselBoxMinSize; //!< The size of the smallest item box in the carousel, i.e. the ones at the ends, excluding the exiting box, which is actually one size step smaller. Used in Carousel MenuMode.
		static const Vector c_CarouselBoxSizeStep; //!< The size step between each item box in the carousel. Used in Carousel MenuMode.
		static const int c_CarouselBoxCornerRadius; //!< The radius of the circles used to make rounded corners for carousel boxes. Calculated automatically based on max and min size. Used in Carousel MenuMode.
		static constexpr int c_MultipleItemInBoxOffset = 3; //!< How many pixels up and right items should be offset if there are more than one in a item box. Used in Carousel MenuMode.

		static constexpr int c_FullViewPageItemLimit = c_ItemsPerRow * 3; //!< The default number of rows in the full inventory display. Used in Full/Transfer MenuModes.
		static constexpr int c_FullMenuVerticalOffset = 50; //!< How high above its target the full GUI will be. Used in Full/Transfer MenuModes.

		static BITMAP* s_CursorBitmap; //!< The cursor image shared by all GUIs.

		GUIFont* m_SmallFont; //!< A pointer to the small font from FrameMan. Not owned here.
		GUIFont* m_LargeFont; //!< A pointer to the large font from FrameMan. Not owned here.

		Controller* m_MenuController; //!< The Controller which controls this menu.
		Actor* m_InventoryActor; //!< The Actor whose inventory this GUI will display.
		MenuMode m_MenuMode; //!< The mode this menu is in. See MenuMode enum for more details.
		Vector m_CenterPos; //!< The center position of this menu in the scene.

		EnabledState m_EnabledState; //!< The enabled state of the menu.
		Timer m_EnableDisableAnimationTimer; //!< Timer for progressing enabling/disabling animations.

		bool m_InventoryActorIsHuman; //!< Whether the Actor whose inventory this GUI will display is an AHuman.
		std::vector<std::pair<MovableObject*, MovableObject*>> m_InventoryActorEquippedItems; //!< A vector of pairs of pointers to the equipped item and equipped offhand item of the Actor whose inventory this GUI will display, if applicable. Pointers are NOT owned.

		bool m_CarouselDrawEmptyBoxes; //!< Whether or not the carousel should draw empty item boxes. Used in Carousel MenuMode.
		bool m_CarouselBackgroundTransparent; //!< Whether or not the carousel's background should be drawn transparently. Used in Carousel MenuMode.
		int m_CarouselBackgroundBoxColor; //!< The color used for the background box of the carousel. Used in Carousel MenuMode.
		Vector m_CarouselBackgroundBoxBorderSize; //!< The size of the border around the background box of the carousel. Used in Carousel MenuMode.
		int m_CarouselBackgroundBoxBorderColor; //!< The color used for the border of the background box of the carousel. Used in Carousel MenuMode.

		CarouselAnimationDirection m_CarouselAnimationDirection; //!< Which direction the carousel is currently animating in, if any. Used in Carousel MenuMode animations.
		Timer m_CarouselAnimationTimer; //!< Timer for progressing carousel animations.
		std::array<std::unique_ptr<CarouselItemBox>, c_ItemsPerRow> m_CarouselItemBoxes; //!< An array of CarouselItemBoxes that make up the carousel. Used in Carousel MenuMode.
		std::unique_ptr<CarouselItemBox> m_CarouselExitingItemBox; //!< A special case CarouselItemBox used to display the item that is exiting during animations. Used in Carousel MenuMode.
		std::unique_ptr<BITMAP> m_CarouselBitmap; //!< The intermediary Bitmap onto which the carousel's items and mass indicators are drawn. It is then drawn onto the Bitmap the carousel is drawn to. Used in Carousel MenuMode.
		std::unique_ptr<BITMAP> m_CarouselBGBitmap; //!< The intermediary Bitmap onto which the carousel's background boxes are drawn. It is then drawn onto the Bitmap the carousel is drawn to. Used in Carousel MenuMode.

		bool m_GUIDisplayOnly; //!< Whether this GUI is display only, or can be interacted with, and thereby affect the inventory it's displaying. Used in Full/Transfer MenuModes.
		bool m_GUIShowEmptyRows; //!< Whether this GUI should show empty rows, up to the FullPageViewItemLimit.
		Vector m_GUICursorPos; //!< Screen position of the cursor. Used in Full/Transfer MenuModes.
		Vector m_PreviousGUICursorPos; //!< The previous screen position of the cursor. Used in Full/Transfer MenuModes.
		int m_GUIInventoryActorCurrentEquipmentSetIndex; //!< An int describing the index in InventoryActorEquippedItems which is currently being shown by the gui. Used in Full/Transfer MenuModes.
		std::unique_ptr<GUISelectedItem> m_GUISelectedItem; //!< The selected item for this InventoryMenuGUI. Used in Full/Transfer MenuModes.

		Timer m_GUIRepeatStartTimer; //!< Measures the time to when to start repeating inputs when they're held down. Used in Full/Transfer MenuModes.
		Timer m_GUIRepeatTimer; //!< Measures the interval between input repeats. Used in Full/Transfer MenuModes.
		GUIButton* m_NonMouseHighlightedButton; //!< A pointer to the GUIButton currently highlighted by the keyboard or gamepad.
		// TODO These previous buttons should be cleaned up when InventoryMenuGUI is refactored. Requirements were unclear so this is more complicated than it needs to be, these 3 can be consolidated into 2 or 1 variable, and the code handling them can be simplified a bit.
		GUIButton* m_NonMousePreviousEquippedItemsBoxButton; //!< A pointer to the last GUIButton in the EquippedItemsBox that was highlighted by the keyboard or gamepad.
		GUIButton* m_NonMousePreviousInventoryItemsBoxButton; //!< A pointer to the last GUIButton in the InventoryItemsBox that was highlighted by the keyboard or gamepad.
		GUIButton* m_NonMousePreviousReloadOrDropButton; //!< A pointer to whichever of the reload or drop GUIButtons was last highlighted by the keyboard or gamepad.

		Vector m_GUITopLevelBoxFullSize; //!< A Vector holding the full size of the top level box for enabling/disabling animations.
		bool m_GUIShowInformationText; //!< Whether or information text explaining how to use the menu should be showing.
		const Icon* m_GUIInformationToggleButtonIcon; //!< A pointer to the PresetMan pie icon for information, used here for the information toggle button. Not Owned here.
		const Icon* m_GUIReloadButtonIcon; //!< A pointer to the PresetMan pie icon for reloading, used here for the reload button. Not Owned here.
		const Icon* m_GUIDropButtonIcon; //!< A pointer to the PresetMan pie icon for dropping items, used here for the drop button. Not Owned here.
		std::vector<std::pair<MovableObject*, GUIButton*>> m_GUIInventoryItemButtons; //!< A vector of pairs of MovableObject pointers and GUIButton pointers, connecting inventory GUIButtons to their corresponding MovableObjects.

		/// GUI elements that make up the full mode InventoryMenuGUI.
		std::unique_ptr<GUIControlManager> m_GUIControlManager;
		std::unique_ptr<AllegroScreen> m_GUIScreen;
		std::unique_ptr<GUIInputWrapper> m_GUIInput;
		GUICollectionBox* m_GUITopLevelBox;
		GUILabel* m_GUIInformationText;
		GUIButton* m_GUIInformationToggleButton;
		GUICollectionBox* m_GUIEquippedItemsBox;
		GUIButton* m_GUISwapSetButton;
		GUIButton* m_GUIEquippedItemButton;
		GUIButton* m_GUIOffhandEquippedItemButton;
		GUIButton* m_GUIReloadButton;
		GUIButton* m_GUIDropButton;
		GUICollectionBox* m_GUIInventoryItemsBox;
		GUIScrollbar* m_GUIInventoryItemsScrollbar;

#pragma region Create Breakdown
		/// Gets whether the InventoryMenuGUI is ready to be activated in Carousel MenuMode. If it's not, that mode should be setup.
		/// @return Whether or not the InventoryMenuGUI is ready to be activated in carousel MenuMode.
		bool CarouselModeReadyForUse() const { return m_CarouselBitmap && m_CarouselBGBitmap; }

		/// Makes the InventoryMenuGUI object ready for Carousel MenuMode use.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int SetupCarouselMode();

		/// Gets whether the InventoryMenuGUI is ready to be activated in Full or Transfer MenuMode. If it's not, that mode should be setup.
		/// @return Whether or not the InventoryMenuGUI is ready to be activated in Full or Transfer MenuMode.
		bool FullOrTransferModeReadyForUse() const { return m_GUIControlManager && m_GUIScreen && m_GUIInput; }

		/// Makes the InventoryMenuGUI object ready for Full or Transfer MenuMode use.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int SetupFullOrTransferMode();
#pragma endregion

#pragma region Update Breakdown
		/// Handles MenuMode specific updating for when the InventoryMenuGUI is in Carousel MenuMode.
		void UpdateCarouselMode();

		/// Handles sorting out positions and sizes of CarouselItemBoxes when in Carousel MenuMode.
		void UpdateCarouselItemBoxSizesAndPositions();

		/// Handles MenuMode specific updating for when the InventoryMenuGUI is in Full MenuMode.
		void UpdateFullMode();

		/// Handles visibility and content for equipped item buttons in Full MenuMode. Does not deal with button input.
		void UpdateFullModeEquippedItemButtons();

		/// Handles hiding and showing the scrollbar, and calculating everything related to it in Full MenuMode.
		/// @param inventory A pointer to the inventory this InventoryMenuGUI is displaying.
		void UpdateFullModeScrollbar(const std::deque<MovableObject*>* inventory);

		/// Handles content and enabled status for inventory item buttons in Full MenuMode. Does not deal with button input.
		/// @param inventory A pointer to the inventory this InventoryMenuGUI is displaying.
		void UpdateFullModeInventoryItemButtons(const std::deque<MovableObject*>* inventory);

		/// Handles everything for displaying information text in Full MenuMode.
		/// @param inventory A pointer to the inventory this InventoryMenuGUI is displaying.
		void UpdateFullModeInformationText(const std::deque<MovableObject*>* inventory);

		/// Handles updating icons and widths to support higlighting, for non-item buttons in Full MenuMode.
		void UpdateFullModeNonItemButtonIconsAndHighlightWidths();

		/// Handles MenuMode specific updating for when the InventoryMenuGUI is in Transfer MenuMode.
		void UpdateTransferMode();
#pragma endregion

#pragma region GUI Input Handling
		/// Player input handling for all devices.
		void HandleInput();

		/// Player mouse input event handling of the GUIControls of this InventoryMenuGUI.
		/// @return Whether the mouse was released this frame after being held long enough to count, useful to prevent duplicate button handling when releasing the left click.
		bool HandleMouseInput();

		/// Player keyboard or gamepad input event handling of the GUIControls of this InventoryMenuGUI.
		void HandleNonMouseInput();

		/// Gets any keyboard or gamepad directional input.
		/// @return The direction of found input. Priority matches ordering of the Directions enumeration.
		Directions GetNonMouseButtonControllerMovement();

		/// Breakdown of HandleNonMouseInput for handling pressing/holding up.
		/// @return The next button to highlight, based on input handling.
		GUIButton* HandleNonMouseUpInput();

		/// Breakdown of HandleNonMouseInput for handling pressing/holding down.
		/// @return The next button to highlight, based on input handling.
		GUIButton* HandleNonMouseDownInput();

		/// Breakdown of HandleNonMouseInput for handling pressing/holding left.
		/// @return The next button to highlight, based on input handling.
		GUIButton* HandleNonMouseLeftInput();

		/// Breakdown of HandleNonMouseInput for handling pressing/holding right.
		/// @return The next button to highlight, based on input handling.
		GUIButton* HandleNonMouseRightInput();

		/// Handles item button press command events from the GUIControls of this InventoryMenuGUI by selecting/de-selecting the corresponding item.
		/// @param pressedButton A pointer to the GUIButton that was pressed.
		/// @param buttonObject A pointer to the MovableObject the GUIButton represents.
		/// @param buttonEquippedItemIndex The index of this button in the vector of equipped items if applicable. The default value of -1 means it's not an equipped item.
		/// @param buttonHeld Whether or not the button was held.
		void HandleItemButtonPressOrHold(GUIButton* pressedButton, MovableObject* buttonObject, int buttonEquippedItemIndex = -1, bool buttonHeld = false);
#pragma endregion

#pragma region GUI Button Actions
		// TODO implement swap mode
		/// Handles set swap button press command events from the GUIControls of this InventoryMenuGUI.
		// void SwapEquippedItemSet() {}

		/// Swaps the equipped item at the given equipped item index with one in the inventory Actor's inventory at the given inventory item index.
		/// Accounts for either index pointing to empty buttons and any other potential complications.
		/// @param equippedItemIndex The index of the equipped item being swapped out.
		/// @param inventoryItemIndex The index in the inventory of the item being swapped in.
		/// @return Whether the inventory item at the given inventory item index was successfully equipped to the given equipped item index.
		bool SwapEquippedItemAndInventoryItem(int equippedItemIndex, int inventoryItemIndex);

		/// Reloads the selected item if it is equipped, or swaps to it and then reloads it if it isn't.
		void ReloadSelectedItem();

		/// Drops the selected item, rotating its drop velocity in the direction of the passed in dropDirection Vector.
		/// @param dropDirection A pointer to a Vector containing the direction the selected item should be dropped in. Nullptr means standard dropping will be used.
		void DropSelectedItem(const Vector* dropDirection = nullptr);
#pragma endregion

#pragma region Draw Breakdown
		/// Draws the InventoryMenuGUI when it's in Carousel MenuMode.
		/// @param targetBitmap A pointer to a BITMAP to draw on. Generally a screen BITMAP.
		/// @param drawPos The position at which to draw the carousel.
		void DrawCarouselMode(BITMAP* targetBitmap, const Vector& drawPos) const;

		/// Draws the InventoryMenuGUI when it's in Full MenuMode.
		/// @param targetBitmap A pointer to a BITMAP to draw on. Generally a screen BITMAP.
		/// @param drawPos The position at which to draw the GUI.
		void DrawFullMode(BITMAP* targetBitmap, const Vector& drawPos) const;

		/// Draws the specified CarouselItemBox's background to the carousel background Bitmap.
		/// @param itemBoxToDraw The CarouselItemBox to draw.
		void DrawCarouselItemBoxBackground(const CarouselItemBox& itemBoxToDraw) const;

		/// Draws the specified CarouselItemBox's item(s) and mass text to the carousel Bitmap.
		/// @param itemBoxToDraw The CarouselItemBox to draw.
		/// @param carouselAllegroBitmap An AllegroBitmap of the bitmap the CarouselItemBox should draw its foreground to. Used for drawing mass strings, and predefined to avoid needless creation.
		void DrawCarouselItemBoxForeground(const CarouselItemBox& itemBoxToDraw, AllegroBitmap* carouselAllegroBitmap) const;
#pragma endregion

		/// Clears all the member variables of this InventoryMenuGUI, effectively resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		InventoryMenuGUI(const InventoryMenuGUI& reference) = delete;
		InventoryMenuGUI& operator=(const InventoryMenuGUI& rhs) = delete;
	};
} // namespace RTE
