#pragma once

#include "Constants.h"
#include "LuabindObjectWrapper.h"
#include "Icon.h"

namespace RTE {

	class PieMenu;

	/// Enumeration for the types of PieSlices.
	enum class PieSliceType : int {
		NoType,
		// Inventory management
		Pickup,
		Drop,
		NextItem,
		PreviousItem,
		Reload,
		// Menu and GUI activation
		BuyMenu,
		FullInventory,
		Stats,
		Map,
		Ceasefire,
		// Squad
		FormSquad,
		// AI Modes
		AIModes,
		Sentry,
		Patrol,
		BrainHunt,
		GoldDig,
		GoTo,
		Return,
		Stay,
		Deliver,
		Scuttle,
		// Editor stuff
		EditorDone,
		EditorLoad,
		EditorSave,
		EditorNew,
		EditorPick,
		EditorMove,
		EditorRemove,
		EditorInFront,
		EditorBehind,
		EditorZoomIn,
		EditorZoomOut,
		EditorTeam1,
		EditorTeam2,
		EditorTeam3,
		EditorTeam4
	};

	/// An individual PieSlice in a PieMenu.
	class PieSlice : public Entity {

	public:
		EntityAllocation(PieSlice);
		SerializableOverrideMethods;
		ClassInfoGetters;

#pragma region Creation
		/// Constructor method used to instantiate a PieSlice object in system memory. Create() should be called before using the object.
		PieSlice();

		/// Makes the PieSlice object ready for use.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create() override;

		/// Creates a PieSlice to be identical to another, by deep copy.
		/// @param reference A reference to the PieSlice to deep copy.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(const PieSlice& reference);
#pragma endregion

#pragma region Destruction
		/// Destructor method used to clean up a PieSlice object before deletion from system memory.
		~PieSlice() override;

		/// Resets the entire Serializable, including its inherited members, to their default settings or values.
		void Reset() override { Clear(); }
#pragma endregion

#pragma region Getters and Setters
		/// Gets the SliceType of this PieSlice.
		/// @return The SliceType of this PieSlice.
		PieSliceType GetType() const { return m_Type; }

		/// Sets the SliceType of this PieSlice.
		/// @param newType The new SliceType of this PieSlice.
		void SetType(PieSliceType newType) { m_Type = newType; }

		/// Gets the Direction of this PieSlice.
		/// @return The Direction of this PieSlice.
		Directions GetDirection() const { return m_Direction; }

		/// Sets the Direction of this PieSlice.
		/// @param newDirection The new Direction of this PieSlice.
		void SetDirection(Directions newDirection) {
			if (newDirection != Directions::None) {
				m_Direction = newDirection;
			}
		}

		/// Gets whether or not this PieSlice can be the middle PieSlice of a PieQuadrant.
		/// @return Whether or not this PieSlice can be the middle PieSlice of a PieQuadrant.
		bool GetCanBeMiddleSlice() const { return m_CanBeMiddleSlice; }

		/// Sets whether or not this PieSlice can be the middle PieSlice of a PieQuadrant.
		/// @param newCanBeMiddleSlice Whether or not this PieSlice can be the middle PieSlice of a PieQuadrant.
		void SetCanBeMiddleSlice(bool newCanBeMiddleSlice) { m_CanBeMiddleSlice = newCanBeMiddleSlice; }

		/// Gets the original Entity source of this PieSlice, if there is one.
		/// @return A pointer to the original Entity source of this PieSlice, if there is one.
		const Entity* GetOriginalSource() const { return m_OriginalSource; }

		/// Sets the original Entity source of this PieSlice.
		/// @param originalSource A pointer to the original Entity source of this PieSlice.
		void SetOriginalSource(const Entity* originalSource) { m_OriginalSource = originalSource; }

		/// Gets whether or not this PieSlice is enabled.
		/// @return Whether or not this PieSlice is enabled.
		bool IsEnabled() const { return m_Enabled; }

		/// Sets whether or not this PieSlice should be enabled.
		/// @param enabled Whether or not this PieSlice should be enabled.
		void SetEnabled(bool enabled) { m_Enabled = enabled; }

		/// Gets whether or not this PieSlice has a valid Icon.
		/// @return Whether or not this PieSlice has a valid Icon.
		bool HasIcon() const { return m_Icon && m_Icon->GetFrameCount() > 0; }

		/// Gets the appropriate icon frame for this PieSlice.
		/// @param sliceIsSelected Whether or not this PieSlice is selected, which may affect which icon is appropriate.
		/// @return The icon for this PieSlice.
		BITMAP* GetAppropriateIcon(bool sliceIsSelected = false) const;

		/// Sets the new Icon for this PieSlice. Ownership IS transferred.
		/// @param newIcon The new Icon for this PieSlice.
		void SetIcon(Icon* newIcon) { m_Icon = std::unique_ptr<Icon>(newIcon); }

		/// Gets the LuabindObjectWrapper for the function this PieSlice should run when activated.
		/// @return The LuabindObjectWrapper this PieSlice should run when activated.
		const LuabindObjectWrapper* GetLuabindFunctionObjectWrapper() const { return m_LuabindFunctionObject.get(); }

		/// Gets the file path of the Lua file this PieSlice should run when activated, if any.
		/// @return The file path to the script file this PieSlice should load when activated.
		std::string GetScriptPath() const { return m_LuabindFunctionObject ? m_LuabindFunctionObject->GetFilePath() : ""; }

		/// Sets the file path of the scripted file this PieSlice should run when activated.
		/// @param newScriptPath The file path of the Lua file this PieSlice should run when activated.
		void SetScriptPath(const std::string& newScriptPath) {
			m_LuabindFunctionObject = std::make_unique<LuabindObjectWrapper>(nullptr, newScriptPath);
			ReloadScripts();
		}

		/// Gets the name of the Lua function to run when this PieSlice is activated.
		/// @return The name of the Lua function this PieSlice should execute when activated.
		const std::string& GetFunctionName() const { return m_FunctionName; }

		/// Sets the name of the Lua function to run when this PieSlice is activated as a scripted pie menu option.
		/// @param newFunctionName The name of the Lua function to run when this PieSlice is activated.
		void SetFunctionName(const std::string& newFunctionName) {
			m_FunctionName = newFunctionName;
			ReloadScripts();
		}

		// TODO Ideally this would be done with a weak_ptr but I'm not sure how it'll go with LuaMan. Try it out and see
		/// Gets the sub-PieMenu for this PieSlice if there is one. Ownership is NOT transferred.
		/// @return The sub-PieMenu for this PieSlice if there is one. Ownership is NOT transferred.
		PieMenu* GetSubPieMenu() const;

		/// Sets the sub-PieMenu for this PieSlice. Ownership IS transferred.
		/// @param newSubPieMenu The new sub-PieMenu for this PieSlice. Ownership IS transferred.
		void SetSubPieMenu(PieMenu* newSubPieMenu);
#pragma endregion

#pragma region Angle Getter and Setters
		/// Gets the start angle this PieSlice's area is set to be at in its pie menu.
		/// @return The start angle of this PieSlice's area.
		float GetStartAngle() const { return m_StartAngle; }

		/// Sets the start angle this PieSlice's area should be at in its pie menu.
		/// @param startAngle The start angle to set for the PieSlice's area.
		void SetStartAngle(float startAngle) {
			m_StartAngle = startAngle;
			RecalculateMidAngle();
		}

		/// Gets the number of slots this PieSlice takes up.
		/// @return The number of slots this PieSlice takes up.
		int GetSlotCount() const { return m_SlotCount; }

		/// Sets the number of slots this PieSlice takes up.
		/// @param slotCount The number of slots this PieSlice should take up.
		void SetSlotCount(int slotCount) {
			m_SlotCount = std::max(1, slotCount);
			RecalculateMidAngle();
		}

		/// Gets the mid angle this PieSlice's area is set to be at in its pie menu.
		/// @return The mid angle of this PieSlice's area.
		float GetMidAngle() const { return m_MidAngle; }

		/// Sets the mid angle this PieSlice's area should be at in its pie menu.
		/// @param midAngle The mid angle to set for the PieSlice's area.
		void SetMidAngle(float midAngle) { m_MidAngle = midAngle; }

		/// Gets whether or not this PieSlice should draw itself flipped to match its absolute angle (i.e. its angle accounting for its PieMenu's rotation).
		/// @return Whether or not this PieSlice should draw itself flipped to match its absolute angle.
		bool GetDrawFlippedToMatchAbsoluteAngle() const { return m_DrawFlippedToMatchAbsoluteAngle; }

		/// Sets whether or not this PieSlice should draw itself flipped to match its absolute angle (i.e. its angle accounting for its PieMenu's rotation).
		/// @param shouldDrawFlippedToMatchAbsoluteAngle Whether or not this PieSlice should draw itself flipped to match its absolute angle.
		void SetDrawFlippedToMatchAbsoluteAngle(bool shouldDrawFlippedToMatchAbsoluteAngle) { m_DrawFlippedToMatchAbsoluteAngle = shouldDrawFlippedToMatchAbsoluteAngle; }
#pragma endregion

		/// Reloads the the script on this PieSlice.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int ReloadScripts() final;

	private:
		/// Custom deleter for PieMenu to avoid include problems with unique_ptr.
		struct PieMenuCustomDeleter {
			void operator()(PieMenu* pieMenu) const;
		};

		static Entity::ClassInfo m_sClass; //!< ClassInfo for this class.

		PieSliceType m_Type; //!< The slice type, also used to determine the icon.
		Directions m_Direction; //!< The desired direction/location of this on the PieMenu.
		bool m_CanBeMiddleSlice; //!< Whether or not this PieSlice is allowed to be the middle slice. Defaults to true and should usually stay that way.
		const Entity* m_OriginalSource; //!< A pointer to the original source of this PieSlice, normally filled in when PieSlices are added to PieMenus by objects other than the PieMenu's owner, and nullptr otherwise.

		bool m_Enabled; //!< Whether this PieSlice is enabled or disabled and grayed out.
		std::unique_ptr<Icon> m_Icon; //!< The icon of this PieSlice.

		std::unique_ptr<LuabindObjectWrapper> m_LuabindFunctionObject; //!< The LuabindObjectWrapper holding the function this PieSlice runs when activated.
		std::string m_FunctionName; //!< Name of the function in the script this PieSlice runs. Used for safely reloading scripts.

		std::unique_ptr<PieMenu, PieMenuCustomDeleter> m_SubPieMenu; //!< Unique pointer to the sub-PieMenu this should open when activated.

		float m_StartAngle; //!< The start angle of this PieSlice's area on the PieMenu, counted in radians from straight out right and going counter clockwise.
		int m_SlotCount; //!< The arc length of the PieSlice area, so that the icon should be drawn at the AreaStart + halfway of this.
		float m_MidAngle; //!< The mid angle of this PieSlice.

		bool m_DrawFlippedToMatchAbsoluteAngle; //!< Whether or not this PieSlice should draw flipped based on its absolute angle (i.e. its angle accounting for its pie menu's rotation).

		/// Recalculates this PieSlice's mid angle based on its start angle and slot count.
		void RecalculateMidAngle();

		/// Clears all the member variables of this PieSlice, effectively resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		PieSlice(const PieSlice& reference) = delete;
		PieSlice& operator=(const PieSlice& rhs) = delete;
	};
} // namespace RTE
