#pragma once

/// BuyMenuGUI class
/// @author Daniel Tabar
/// dtabar@datarealms.com
/// http://www.datarealms.com
/// Inclusions of header files
// #include "FrameMan.h"
#include "Timer.h"
#include "Controller.h"
#include "Loadout.h"

#include <map>

struct BITMAP;

namespace RTE {

	class GUIScreen;
	class GUIInput;
	class GUIControlManager;
	class GUICollectionBox;
	class GUITab;
	class GUIListBox;
	class GUITextBox;
	class GUIButton;
	class GUIBitmap;
	class GUILabel;
	class SceneObject;
	class MovableObject;
	class ACraft;

	/// A full menu system that represents a purchasing GUI for Cortex Command
	class BuyMenuGUI {

		/// Public member variable, method and friend function declarations
	public:
		/// Constructor method used to instantiate a BuyMenuGUI object in system
		/// memory. Create() should be called before using the object.
		BuyMenuGUI();

		/// Destructor method used to clean up a BuyMenuGUI object before deletion
		/// from system memory.
		~BuyMenuGUI();

		/// Makes the BuyMenuGUI object ready for use.
		/// @param pController A poitner to a Controller which will control this Menu. Ownership is
		/// NOT TRANSFERRED!
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create(Controller* pController);

		/// Resets the entire BuyMenuGUI, including its inherited members, to
		/// their default settings or values.
		void Reset() { Clear(); }

		/// Destroys and resets (through Clear()) the BuyMenuGUI object.
		void Destroy();

		/// Loads or re-loads all the loadout presets from the appropriate files
		/// on disk. This will first clear out all current loadout presets!
		/// @return Success or not.
		bool LoadAllLoadoutsFromFile();

		/// Saves all the loadouts to appropriate file on disk. Does NOT save
		/// any named presets which will be loaded from the standard preset
		/// loadouts first anyway.
		/// @return Success or not.
		bool SaveAllLoadoutsToFile();

		/// Sets the controller used by this. The ownership of the controller is
		/// NOT transferred!
		/// @param pController The new controller for this menu. Ownership is NOT transferred
		void SetController(Controller* pController) { m_pController = pController; }

		/// Enables or disables the menu. This will animate it in and out of view.
		/// @param enable Whether to enable or disable the menu. (default: true)
		void SetEnabled(bool enable = true);

		/// Reports whether the menu is enabled or not.
		bool IsEnabled() const { return m_MenuEnabled == ENABLED || m_MenuEnabled == ENABLING; }

		/// Reports whether the menu is at all visible or not.
		bool IsVisible() const { return m_MenuEnabled == ENABLED || m_MenuEnabled == ENABLING || m_MenuEnabled == DISABLING; }

		/// Sets where on the screen that this GUI is being drawn to. If upper
		/// left corner, then 0, 0. This will affect the way the mouse is positioned
		/// etc.
		/// @param newPosX The new screen position of this entire GUI.
		void SetPosOnScreen(int newPosX, int newPosY);

		/// Sets which MetaPlayer uses this menu, if any.
		/// @param metaPlayer The index of the MetaPlayer that uses this menu.
		void SetMetaPlayer(int metaPlayer);

		/// Gets which MetaPlayer uses this menu, if any.
		/// @return Metaplayer who owns this buy menu
		int GetMetaPlayer() const { return m_MetaPlayer; }

		/// Sets which DataModule ID should be treated as the native tech of the user of this menu.
		/// This will also apply the DataModule's faction BuyMenu theme, if applicable.
		/// @param whichModule The module ID to set as the native one. 0 means everything is native.
		void SetNativeTechModule(int whichModule);

		/// Sets the multiplier of the cost of any foreign Tech items.
		/// @param newMultiplier The scalar multiplier of the costs of foreign Tech items.
		void SetForeignCostMultiplier(float newMultiplier) { m_ForeignCostMult = newMultiplier; }

		/// Sets whether a data module shown in the item menu should be expanded
		/// or not.
		/// @param whichModule The module ID to set as expanded.
		/// @param expanded Whether should be expanded or not. (default: true)
		void SetModuleExpanded(int whichModule, bool expanded = true);

		/// Reports whether a purchase was made during the last Update.
		/// @return Wheter the BUY button was pressed or not during the last update.
		bool PurchaseMade() const { return m_PurchaseMade; }

		/// Gets the width of the current delivery craft.
		/// @return The width of the delivery craft, in pixels.
		int GetDeliveryWidth() const { return m_DeliveryWidth; }

		/// Return the list of things currently in the purchase order list box.
		/// @param listToFill A reference to a an empty list to fill with the Object:s ordered.
		/// Ownership of the Object:s is NOT TRANSFERRED!
		/// @return Whetehr any items were put in the list at all. false if there are no
		/// items in the order listbox.
		bool GetOrderList(std::list<const SceneObject*>& listToFill) const;

		/// Return the list of loadouts currently saved as presets.
		/// @return A reference to the list of loadout presets.
		std::vector<Loadout>& GetLoadoutPresets() { return m_Loadouts; }

		/// Saves the current loadout into a Set.
		void SaveCurrentLoadout();

		/// Return the intended delivery vehicle instance from the order.
		/// @return The poiner to the specified delivery craft instance. Note that this is
		/// just PresetMan's const pointer, so ownership is NOT transferred!
		const SceneObject* GetDeliveryCraftPreset() { return m_pSelectedCraft; }

		/// Return the total cost of everything listed in the order box.
		/// @param includeDelivery Whether or not to include delivery cost. (default: true)
		/// @return The total cost in ounces of gold.
		float GetTotalCost(bool includeDelivery = true) const;

		/// Return the total cost of everything listed in the order box, including delivery costs.
		/// @return The total cost in ounces of gold.
		float GetTotalOrderCost() const { return GetTotalCost(true); };

		/// Return the total cost of everything listed in the order box, excluding delivery costs.
		/// @return The total cost in ounces of gold.
		float GetTotalCartCost() const { return GetTotalCost(false); }

		/// Return the total mass of all items listed in the order box.
		/// @return The total mass (in kg) of the BuyMenu's cart.
		float GetTotalOrderMass() const;

		/// Return mass of craft used in the order box.
		/// @return The total mass in kg.
		float GetCraftMass();

		/// Return teh total number of passengers in the order box.
		/// @return The total number of passengers.
		int GetTotalOrderPassengers() const;

		/// Enable or disable the equipment selection mode for this BuyMenuGUI.
		/// @param enabled Whether or not equipment selection mode should be enabled.
		void EnableEquipmentSelection(bool enabled);

		/// Updates the nesting level for every item in the cart.
		void UpdateItemNestingLevels();

		/// Updates the state of this Menu each frame
		void Update();

		/// Draws the menu
		/// @param drawBitmap The bitmap to draw on.
		void Draw(BITMAP* drawBitmap) const;

		/// Tells whether passenger count constraints are enforced by this buy menu.
		/// @return True if passenger constraints are enforced by this menu, false otherwise
		bool EnforceMaxPassengersConstraint() const { return m_EnforceMaxPassengersConstraint; };

		/// Sets whether passenger count constraints are enforced by this buy menu.
		/// @param enforce True to enforce passenger constraints by this menu, false otherwise
		void SetEnforceMaxPassengersConstraint(bool enforce) { m_EnforceMaxPassengersConstraint = enforce; };

		/// Sets whether mass constraints are enforced by this buy menu.
		/// @param True if mass constraints are enforced by this menu, false otherwise
		bool EnforceMaxMassConstraint() const { return m_EnforceMaxMassConstraint; };

		/// Sets whether mass constraints are enforced by this buy menu.
		/// @param enforce True to enforce mass constraints by this menu, false otherwise
		void SetEnforceMaxMassConstraint(bool enforce) { m_EnforceMaxMassConstraint = enforce; };

		/// Adds an item to the list of allowed items.
		/// If the list is not empty then everything not in the list is removed from the buy menu
		/// Items will be removed from the buy menu when it's called, category changed or after a ForceRefresh().
		/// @param presetName Full preset name to add.
		void AddAllowedItem(std::string presetName) { m_AllowedItems[presetName] = true; };

		/// Removes an item from the list of allowed items.
		/// @param m_AllowedItems.erase(presetName Full preset name to remove.
		void RemoveAllowedItem(std::string presetName) { m_AllowedItems.erase(presetName); };

		/// Clears the list of allowed items
		void ClearAllowedItems() { m_AllowedItems.clear(); };

		/// Returns true if the item is in allowed list
		/// @param ! Full preset name. (default: m_AllowedItems.end()
		bool IsAllowedItem(std::string presetName) { return m_AllowedItems.find(presetName) != m_AllowedItems.end(); }

		/// Adds an item to the list of always allowed items. This list overrides all previous constraints.
		/// @param presetName Full preset name to add.
		void AddAlwaysAllowedItem(std::string presetName) { m_AlwaysAllowedItems[presetName] = true; };

		/// Removes an item from the list of always allowed items.
		/// @param m_AlwaysAllowedItems.erase(presetName Full preset name to remove.
		void RemoveAlwaysAllowedItem(std::string presetName) { m_AlwaysAllowedItems.erase(presetName); };

		/// Clears the list of allowed items
		void ClearAlwaysAllowedItems() { m_AlwaysAllowedItems.clear(); };

		/// Returns true if the item is in always allowed list
		/// @param ! Full preset name. (default: m_AlwaysAllowedItems.end()
		bool IsAlwaysAllowedItem(std::string presetName) { return m_AlwaysAllowedItems.find(presetName) != m_AlwaysAllowedItems.end(); }

		/// Adds an item prohibited to buy from the buy menu.
		/// The item will be removed from the buy menu when it's called, category changed or after a ForceRefresh().
		/// @param presetName Full preset name to add.
		void AddProhibitedItem(std::string presetName) { m_ProhibitedItems[presetName] = true; };

		/// Removes item from the list of prohibited items
		/// @param m_ProhibitedItems.erase(presetName Full preset name to remove.
		void RemoveProhibitedItem(std::string presetName) { m_ProhibitedItems.erase(presetName); };

		/// Clears the list of prohibited items
		void ClearProhibitedItems() { m_ProhibitedItems.clear(); };

		/// Returns true if the item is in prohibited list
		/// @param ! Full preset name. (default: m_ProhibitedItems.end()
		bool IsProhibitedItem(std::string presetName) { return m_ProhibitedItems.find(presetName) != m_ProhibitedItems.end(); }

		/// Forces a refresh update of the list of buy menu items
		void ForceRefresh() { CategoryChange(); }

		/// Clear the cart out of items selected for purchase
		void ClearCartList();

		/// Adds an item to the cart.
		/// @param name The name shown for the item.
		/// @param rightText The text that is shown right-aligned on the item (typically the cost of the item).
		/// @param pBitmap The sprite image rendered for the item. This takes ownership!
		/// @param pEntity The entity that this item refers to and will create when bought.
		/// @param extraIndex Extra index for special indexing or reference that the item is associated with. Menu-specific.
		void AddCartItem(const std::string& name, const std::string& rightText = "", GUIBitmap* pBitmap = nullptr, const Entity* pEntity = 0, const int extraIndex = -1);

		/// Duplicates an item in the cart.
		/// @param itemIndex The index of the item to duplicate.
		void DuplicateCartItem(const int itemIndex);

		/// Loads the default loadout to the cart
		void LoadDefaultLoadoutToCart() { DeployLoadout(0); }

		/// If set to true only owned items will be shown in buy menu. Overriden by AlwaysAllowed list.
		/// @param value Value.
		void SetOnlyShowOwnedItems(bool value) { m_OnlyShowOwnedItems = value; }

		/// Returns whether only owned items will be shown in buy menu. Overriden by AlwaysAllowed list.
		/// @return Whether only owned items will be shown in buy menu.
		bool GetOnlyShowOwnedItems() const { return m_OnlyShowOwnedItems; }

		/// Sets the amount of specified items to be owned in this buy menu
		/// @param presetName Full preset name of item to own. Amount of owned items.
		void SetOwnedItemsAmount(std::string presetName, int amount) { m_OwnedItems[presetName] = amount; };

		/// Returns the amount of specified items owned in this buy menu
		/// @param presetName Full preset name of item.
		/// @return Amount of owned items.
		int GetOwnedItemsAmount(std::string presetName) {
			if (m_OwnedItems.find(presetName) != m_OwnedItems.end())
				return m_OwnedItems[presetName];
			else
				return 0;
		};

		/// Deducts 1 piece of owned item and return true if purchase can be made or false if the item is out of stock.
		/// @param presetName Full preset name of item.
		/// @return Whether the purchase can be conducted or the item is out of stock.
		bool CommitPurchase(std::string presetName);

#pragma region Faction Theme Handling
		/// Changes the banner image to the one specified. If none is specified, resets it to the default banner image.
		/// @param imagePath Path to image to set as banner.
		void SetBannerImage(const std::string& imagePath);

		/// Changes the logo image to the one specified. If none is specified, resets it to the default logo image.
		/// @param imagePath Path to image to set as logo.
		void SetLogoImage(const std::string& imagePath);
#pragma endregion

		/// Protected member variable and method declarations
	protected:
		/// Makes sure all things that to happen when category is changed, happens.
		/// @param focusOnCategoryTabs Wheter to change focus to the category tabs or not. (default: true)
		void CategoryChange(bool focusOnCategoryTabs = true);

		/// Loads the loadout set into the cart, replacing whatever's there now.
		/// @param index The index of the loadout to load.
		/// @return Whether it was loaded successfully or not.
		bool DeployLoadout(int index);

		/// Adds all objects of a specific type already defined in PresetMan to the current shop/item list. They will be grouped into the different data modules they were read from.
		/// @param moduleList Reference to the data module vector of entity lists to add the items to.
		/// @param type The name of the class to add all objects of. "" or "All" looks for all.
		/// @param groups The name of the groups to add all objects of. An empty vector or "All" looks for all.
		/// @param excludeGroups Whether the specified groups should be excluded, meaning all objects NOT associated with the groups will be added.
		void AddObjectsToItemList(std::vector<std::list<Entity*>>& moduleList, const std::string& type = "", const std::vector<std::string>& groups = {}, bool excludeGroups = false);

		/// Adds all loadout presets' representations to the item GUI list.
		void AddPresetsToItemList();

		/// Updates the text of the total cost label to reflect the total cost of
		/// all the items in teh order box.
		/// @param whichTeam The team to display the total funds of. (default: 0)
		void UpdateTotalCostLabel(int whichTeam = 0);

		/// Updates the text of the specified label to reflect the total mass of
		/// all the items in the order box.
		/// @param pCraft Craft to read MaxMass from. Label to update.
		void UpdateTotalMassLabel(const ACraft* pCraft, GUILabel* pLabel) const;

		/// Updates the text of the specified label to reflect the total passenger count of
		/// all the items in teh order box.
		/// @param pCraft Craft to read MaxPassengers from. Label to update.
		void UpdateTotalPassengersLabel(const ACraft* pCraft, GUILabel* pLabel) const;

		/// Attempts to make a purchase with everything already set up.
		void TryPurchase();

		enum MenuEnabled {
			ENABLING = 0,
			ENABLED,
			DISABLING,
			DISABLED
		};

		enum MenuFocus {
			SETBUTTONS = 0,
			CATEGORIES,
			ITEMS,
			ORDER,
			OK,
			CLEARORDER,
			FOCUSCOUNT
		};

		enum MenuCategory {
			CRAFT = 0,
			BODIES,
			MECHA,
			TOOLS,
			GUNS,
			BOMBS,
			SHIELDS,
			SETS,
			CATEGORYCOUNT
		};

		enum BlinkMode {
			NOBLINK = 0,
			NOFUNDS,
			NOCRAFT,
			MAXPASSENGERS,
			MAXMASS,
			BLINKMODECOUNT
		};

		// Controls this Menu. Not owned
		Controller* m_pController;
		// GUI Screen for use by the in-game GUI
		GUIScreen* m_pGUIScreen;
		// Input controller
		GUIInput* m_pGUIInput;
		// The control manager which holds all the controls
		GUIControlManager* m_pGUIController;
		// Visibility state of each menu
		int m_MenuEnabled;
		// Focus state
		int m_MenuFocus;
		// Focus change direction - 0 is non,e negative is back, positive forward
		int m_FocusChange;
		// Category selection state
		int m_MenuCategory;
		// Speed at which the menus appear and disappear
		float m_MenuSpeed;
		// Which item in the currently focused list box we have selected
		int m_ListItemIndex;
		// Which item we're dragging
		int m_DraggedItemIndex;
		// Whether we're currently dragging
		bool m_IsDragging;
		// Which object was last hovered over by the mouse, to avoid repeatedly selecting hte same item over and over when mose only moves a pixel
		int m_LastHoveredMouseIndex;
		// Which item in each of the categories was last selected, so the scrolling doesn't have to be redone each time user flips back and forth
		int m_CategoryItemIndex[CATEGORYCOUNT];
		// Which metaplayer, if any, is using this menu
		int m_MetaPlayer;
		// The ID of the DataModule that contains the native Tech of the Player using this menu
		int m_NativeTechModule;
		// The multiplier of costs of any foreign tech items
		float m_ForeignCostMult;
		// Arry of bools showing which modules that have been expanded in the item list
		bool* m_aExpandedModules;
		// Notification blink timer
		Timer m_BlinkTimer;
		// What we're blinking
		int m_BlinkMode;
		// Measures real time to determine how fast the menu should animate when opening/closing to appear real time to the player
		Timer m_MenuTimer;
		// Measures the time to when to start repeating inputs when they're held down
		Timer m_RepeatStartTimer;
		// Measures the interval between input repeats
		Timer m_RepeatTimer;

		bool m_SelectingEquipment; //!< Whether or not the menu is in equipment mode.
		MenuCategory m_LastVisitedEquipmentTab; //!< The last tab visited while in equipment mode.
		MenuCategory m_LastVisitedMainTab; //!< The last tab visited while not in equipment mode.
		int m_LastEquipmentScrollPosition; //!< The last scroll position while in equipment mode.
		int m_LastMainScrollPosition; //!< The last scroll position while not in equipment mode.
		MenuCategory m_FirstMainTab; //!< The first enabled tab when not in equipment mode.
		MenuCategory m_LastMainTab; //!< The last enabled tab when not in equipment mode.
		MenuCategory m_FirstEquipmentTab; //!< The first enabled tab when in equipment mode.
		MenuCategory m_LastEquipmentTab; //!< The last enabled tab when in equipment mode.

		// Collection box of the buy GUIs
		GUICollectionBox* m_pParentBox;
		// Collection box of the buy popups that contain information about items
		GUICollectionBox* m_pPopupBox;
		// Label displaying the item popup description
		GUILabel* m_pPopupText;
		// Top banner
		GUICollectionBox* m_Banner;
		// Logo label that disappears when the sets category is selected
		GUICollectionBox* m_Logo;
		// All the radio buttons for the different shop categories
		GUITab* m_pCategoryTabs[CATEGORYCOUNT];
		// The Listbox which lists all the shop's items in the currently selected category
		GUIListBox* m_pShopList;
		// The Listbox which lists all the items currently in the shopping cart or order
		GUIListBox* m_pCartList;
		// The single-line textbox which shows the selected delivery craft
		GUITextBox* m_pCraftBox;

		// Panel with craft parameters
		GUICollectionBox* m_pCraftCollectionBox;

		// Selected craft name
		GUILabel* m_pCraftNameLabel;
		// Selected craft price
		GUILabel* m_pCraftPriceLabel;
		// Selected craft passenger caption
		GUILabel* m_pCraftPassengersCaptionLabel;
		// Selected craft passenger count
		GUILabel* m_pCraftPassengersLabel;
		// Selected craft total mass caption
		GUILabel* m_pCraftMassCaptionLabel;
		// Selected craft total mass
		GUILabel* m_pCraftMassLabel;

		// Label displaying "Delivered On:"
		GUILabel* m_pCraftLabel;
		// The selected craft instance for delivery
		const SceneObject* m_pSelectedCraft;
		// Label displaying the total cost
		GUILabel* m_pCostLabel;
		// The purchasing button
		GUIButton* m_pBuyButton;

		GUIButton* m_ClearOrderButton; //!< Button for clearing the cart.
		// The save set button
		GUIButton* m_pSaveButton;
		// The clear set button
		GUIButton* m_pClearButton;
		// Sets of user-defined loadouts that can be selected quickly.
		std::vector<Loadout> m_Loadouts;
		// Purchase has been made
		bool m_PurchaseMade;
		int m_DeliveryWidth; //!< The width of the currently selected delivery craft, which will determine the width of the LZ marker.
		// The cursor image shared by all buy menus
		static BITMAP* s_pCursor;

		// If true UI won't afford to order a craft with more passengers than allowed by craft
		bool m_EnforceMaxPassengersConstraint;
		// If true UI won't afford to order a craft with more mass than allowed by craft
		bool m_EnforceMaxMassConstraint;

		// Only show items that owned
		bool m_OnlyShowOwnedItems;
		// If not empty then only shows items present in this list
		std::map<std::string, bool> m_AllowedItems;
		// If not empty then items from this list are always shown int he buy menu no matter what other constraints there are
		std::map<std::string, bool> m_AlwaysAllowedItems;
		// If not empty then removes items from ths list the buy menu
		std::map<std::string, bool> m_ProhibitedItems;
		// A map of owned items, for which the gold will not be deducted when bought
		std::map<std::string, int> m_OwnedItems;

		/// Private member variable and method declarations
	private:
		static const std::string c_DefaultBannerImagePath; //!< Path to the default banner image.
		static const std::string c_DefaultLogoImagePath; //!< Path to the default logo image.

		/// Refresh tab disabled states, so tabs get properly enabled/disabled based on whether or not equipment selection mode is enabled.
		void RefreshTabDisabledStates();

		/// Clears all the member variables of this BuyMenuGUI, effectively
		/// resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		BuyMenuGUI(const BuyMenuGUI& reference) = delete;
		BuyMenuGUI& operator=(const BuyMenuGUI& rhs) = delete;
	};

} // namespace RTE
