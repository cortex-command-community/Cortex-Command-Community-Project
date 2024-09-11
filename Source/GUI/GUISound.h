#pragma once

#include "SoundContainer.h"
#include "Singleton.h"

#define g_GUISound GUISound::Instance()

namespace RTE {

	/// The singleton loader for all GUI sound effects.
	class GUISound : public Singleton<GUISound> {

	public:
#pragma region Creation
		/// Constructor method used to instantiate a GUISound object in system memory. Create() should be called before using the object.
		GUISound();

		/// Creates all the GUI sound effects with Sound::Create and their additional samples with Sound::AddSample and makes the GUISound object ready for use.
		void Initialize();
#pragma endregion

#pragma region Destruction
		/// Destroys and resets (through Clear()) the GUISound object.
		void Destroy() { Clear(); }
#pragma endregion

#pragma region Getters
		/// Gets juicy logo signature jingle Sound.
		/// @return Juicy logo signature jingle Sound.
		SoundContainer* SplashSound() { return &m_SplashSound; }

		/// Gets SoundContainer for enabling menu.
		/// @return SoundContainer for enabling menu.
		SoundContainer* EnterMenuSound() { return &m_EnterMenuSound; }

		/// Gets SoundContainer for disabling menu.
		/// @return SoundContainer for disabling menu.
		SoundContainer* ExitMenuSound() { return &m_ExitMenuSound; }

		/// Gets SoundContainer for changing focus.
		/// @return SoundContainer for changing focus.
		SoundContainer* FocusChangeSound() { return &m_FocusChangeSound; }

		/// Gets SoundContainer for selecting items in list, etc.
		/// @return SoundContainer for selecting items in list, etc.
		SoundContainer* SelectionChangeSound() { return &m_SelectionChangeSound; }

		/// Gets SoundContainer for adding or deleting items in list.
		/// @return SoundContainer for adding or deleting items in list.
		SoundContainer* ItemChangeSound() { return &m_ItemChangeSound; }

		/// Gets SoundContainer for button press.
		/// @return SoundContainer for button press.
		SoundContainer* ButtonPressSound() { return &m_ButtonPressSound; }

		/// Gets SoundContainer for button press of going back button.
		/// @return SoundContainer for button press of going back button.
		SoundContainer* BackButtonPressSound() { return &m_BackButtonPressSound; }

		/// Gets SoundContainer for confirming a selection.
		/// @return SoundContainer for confirming a selection.
		SoundContainer* ConfirmSound() { return &m_ConfirmSound; }

		/// Gets SoundContainer for erroneous input.
		/// @return SoundContainer for erroneous input.
		SoundContainer* UserErrorSound() { return &m_UserErrorSound; }

		/// Gets SoundContainer for testing volume when adjusting volume sliders.
		/// @return SoundContainer for testing volume when adjusting volume sliders.
		SoundContainer* TestSound() { return &m_TestSound; }

		/// Gets SoundContainer for opening pie menu.
		/// @return SoundContainer for opening pie menu.
		SoundContainer* PieMenuEnterSound() { return &m_PieMenuEnterSound; }

		/// Gets SoundContainer for closing pie menu.
		/// @return SoundContainer for closing pie menu.
		SoundContainer* PieMenuExitSound() { return &m_PieMenuExitSound; }

		/// Gets SoundContainer for when PieMenu hover arrow appears or changes slice.
		/// @return SoundContainer for when PieMenu hover arrow appears or changes slice.
		SoundContainer* HoverChangeSound() { return &m_HoverChangeSound; }

		/// Gets SoundContainer for when PieMenu hover arrow appears or changes to a disabled slice.
		/// @return SoundContainer for when PieMenu hover arrow appears or changes to a disabled slice.
		SoundContainer* HoverDisabledSound() { return &m_HoverDisabledSound; }

		/// Gets SoundContainer for picking a valid PieMenu slice.
		/// @return SoundContainer for picking a valid PieMenu slice.
		SoundContainer* SlicePickedSound() { return &m_SlicePickedSound; }

		/// Gets SoundContainer for erroneous input in PieMenu.
		/// @return SoundContainer for erroneous input in PieMenu.
		SoundContainer* DisabledPickedSound() { return &m_DisabledPickedSound; }

		/// Gets SoundContainer for when the funds of a team changes.
		/// @return SoundContainer for when the funds of a team changes.
		SoundContainer* FundsChangedSound() { return &m_FundsChangedSound; }

		/// Gets SoundContainer for switching between regular (non-brain) actors.
		/// @return SoundContainer for switching between regular (non-brain) actors.
		SoundContainer* ActorSwitchSound() { return &m_ActorSwitchSound; }

		/// Gets SoundContainer for switching to the brain shortcut.
		/// @return SoundContainer for switching to the brain shortcut.
		SoundContainer* BrainSwitchSound() { return &m_BrainSwitchSound; }

		/// Gets SoundContainer when camera is traveling between actors.
		/// @return SoundContainer when camera is traveling between actors.
		SoundContainer* CameraTravelSound() { return &m_CameraTravelSound; }

		/// Gets SoundContainer for making an area focus.
		/// @return SoundContainer for making an area focus.
		SoundContainer* AreaPickedSound() { return &m_AreaPickedSound; }

		/// Gets SoundContainer for making an object focus.
		/// @return SoundContainer for making an object focus.
		SoundContainer* ObjectPickedSound() { return &m_ObjectPickedSound; }

		/// Gets SoundContainer for making a purchase.
		/// @return SoundContainer for making a purchase.
		SoundContainer* PurchaseMadeSound() { return &m_PurchaseMadeSound; }

		/// Gets SoundContainer for placement of object to scene.
		/// @return SoundContainer for placement of object to scene.
		SoundContainer* PlacementBlip() { return &m_PlacementBlip; }

		/// Gets SoundContainer for placement of object to scene.
		/// @return SoundContainer for placement of object to scene.
		SoundContainer* PlacementThud() { return &m_PlacementThud; }

		/// Gets SoundContainer for gravely placement of object to scene.
		/// @return SoundContainer for gravely placement of object to scene.
		SoundContainer* PlacementGravel() { return &m_PlacementGravel; }
#pragma endregion

	protected:
		SoundContainer m_SplashSound; //! Juicy logo signature jingle sound.

		SoundContainer m_EnterMenuSound; //! SoundContainer for enabling menu.
		SoundContainer m_ExitMenuSound; //! SoundContainer for disabling menu.

		SoundContainer m_FocusChangeSound; //! SoundContainer for changing focus.

		SoundContainer m_SelectionChangeSound; //! SoundContainer for selecting items in list, etc.
		SoundContainer m_ItemChangeSound; //! SoundContainer for adding or deleting items in list.

		SoundContainer m_ButtonPressSound; //! SoundContainer for button press.
		SoundContainer m_BackButtonPressSound; //! SoundContainer for button press of going back button.
		SoundContainer m_ConfirmSound; //! SoundContainer for confirming a selection.
		SoundContainer m_UserErrorSound; //! SoundContainer for erroneous input.
		SoundContainer m_TestSound; //! SoundContainer for testing volume when adjusting volume sliders.

		SoundContainer m_PieMenuEnterSound; //! SoundContainer for opening pie menu.
		SoundContainer m_PieMenuExitSound; //! SoundContainer for closing pie menu.

		SoundContainer m_HoverChangeSound; //! SoundContainer for when PieMenu hover arrow appears or changes slice.
		SoundContainer m_HoverDisabledSound; //! SoundContainer for when PieMenu hover arrow appears or changes to a disabled slice.
		SoundContainer m_SlicePickedSound; //! SoundContainer for picking a valid PieMenu slice.
		SoundContainer m_DisabledPickedSound; //! SoundContainer for erroneous input in PieMenu.

		SoundContainer m_FundsChangedSound; //! SoundContainer for when the funds of a team changes.

		SoundContainer m_ActorSwitchSound; //! SoundContainer for switching between regular (non-brain) actors.
		SoundContainer m_BrainSwitchSound; //! SoundContainer for switching to the brain shortcut.

		SoundContainer m_CameraTravelSound; //! SoundContainer when camera is traveling between actors.

		SoundContainer m_AreaPickedSound; //! SoundContainer for making an area focus.
		SoundContainer m_ObjectPickedSound; //! SoundContainer for making an object focus.
		SoundContainer m_PurchaseMadeSound; //! SoundContainer for making a purchase.

		SoundContainer m_PlacementBlip; //! SoundContainer for placement of object to scene.
		SoundContainer m_PlacementThud; //! SoundContainer for placement of object to scene.
		SoundContainer m_PlacementGravel; //! SoundContainer for gravely placement of object to scene.

		/// Clears all the member variables of this GUISound, effectively resetting the members of this abstraction level only.
		void Clear();
	};
} // namespace RTE
