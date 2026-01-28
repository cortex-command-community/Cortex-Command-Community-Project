#include "GUISound.h"

#include "SoundSet.h"

using namespace RTE;

GUISound::GUISound() {
	Clear();
}

void GUISound::Clear() {
	m_SplashSound = std::make_shared<SoundContainer>();
	m_EnterMenuSound = std::make_shared<SoundContainer>();
	m_ExitMenuSound = std::make_shared<SoundContainer>();
	m_FocusChangeSound = std::make_shared<SoundContainer>();
	m_SelectionChangeSound = std::make_shared<SoundContainer>();
	m_ItemChangeSound = std::make_shared<SoundContainer>();
	m_ButtonPressSound = std::make_shared<SoundContainer>();
	m_BackButtonPressSound = std::make_shared<SoundContainer>();
	m_ConfirmSound = std::make_shared<SoundContainer>();
	m_UserErrorSound = std::make_shared<SoundContainer>();
	m_TestSound = std::make_shared<SoundContainer>();
	m_PieMenuEnterSound = std::make_shared<SoundContainer>();
	m_PieMenuExitSound = std::make_shared<SoundContainer>();
	m_HoverChangeSound = std::make_shared<SoundContainer>();
	m_HoverDisabledSound = std::make_shared<SoundContainer>();
	m_SlicePickedSound = std::make_shared<SoundContainer>();
	m_DisabledPickedSound = std::make_shared<SoundContainer>();
	m_FundsChangedSound = std::make_shared<SoundContainer>();
	m_ActorSwitchSound = std::make_shared<SoundContainer>();
	m_BrainSwitchSound = std::make_shared<SoundContainer>();
	m_CameraTravelSound = std::make_shared<SoundContainer>();
	m_AreaPickedSound = std::make_shared<SoundContainer>();
	m_ObjectPickedSound = std::make_shared<SoundContainer>();
	m_PurchaseMadeSound = std::make_shared<SoundContainer>();
	m_PlacementBlip = std::make_shared<SoundContainer>();
	m_PlacementThud = std::make_shared<SoundContainer>();
	m_PlacementGravel = std::make_shared<SoundContainer>();
}

void GUISound::Initialize() {
	// Interface sounds should not be pitched to reinforce the appearance of time decoupling between simulation and UI.

	m_SplashSound->Create("Base.rte/Sounds/GUIs/MetaStart.flac", true, false, SoundContainer::BusRouting::UI);

	m_EnterMenuSound->Create("Base.rte/Sounds/GUIs/MenuEnter.flac", true, false, SoundContainer::BusRouting::UI);

	m_ExitMenuSound->Create("Base.rte/Sounds/GUIs/MenuExit1.flac", true, false, SoundContainer::BusRouting::UI);
	m_ExitMenuSound->GetTopLevelSoundSet().AddSound("Base.rte/Sounds/GUIs/MenuExit2.flac", true);

	m_FocusChangeSound->Create("Base.rte/Sounds/GUIs/FocusChange.flac", true, false, SoundContainer::BusRouting::UI);

	m_SelectionChangeSound->Create("Base.rte/Sounds/GUIs/SelectionChange.flac", true, false, SoundContainer::BusRouting::UI);

	m_ItemChangeSound->Create("Base.rte/Sounds/GUIs/ItemChange.flac", true, false, SoundContainer::BusRouting::UI);

	m_ButtonPressSound->Create("Base.rte/Sounds/GUIs/ButtonPress.flac", true, false, SoundContainer::BusRouting::UI);

	m_BackButtonPressSound->Create("Base.rte/Sounds/GUIs/BackButtonPress.flac", true, false, SoundContainer::BusRouting::UI);

	m_ConfirmSound->Create("Base.rte/Sounds/GUIs/MenuExit1.flac", true, false, SoundContainer::BusRouting::UI);

	m_UserErrorSound->Create("Base.rte/Sounds/GUIs/UserError.flac", true, false, SoundContainer::BusRouting::UI);

	m_TestSound->Create("Base.rte/Sounds/GUIs/Test.flac", true, false, SoundContainer::BusRouting::UI);

	m_PieMenuEnterSound->Create("Base.rte/Sounds/GUIs/PieMenuEnter.flac", true, false, SoundContainer::BusRouting::UI);

	m_PieMenuExitSound->Create("Base.rte/Sounds/GUIs/PieMenuExit.flac", true, false, SoundContainer::BusRouting::UI);

	//		m_HoverChangeSound->Create("Base.rte/Sounds/GUIs/SelectionChange.flac", true, false, SoundContainer::BusRouting::UI);
	m_HoverChangeSound = m_SelectionChangeSound;

	m_HoverDisabledSound->Create("Base.rte/Sounds/GUIs/PlacementBlip.flac", true, false, SoundContainer::BusRouting::UI);

	m_SlicePickedSound->Create("Base.rte/Sounds/GUIs/SlicePicked.flac", true, false, SoundContainer::BusRouting::UI);

	//		m_DisabledPickedSound->Create("Base.rte/Sounds/GUIs/PieMenuExit.flac", true, false, SoundContainer::BusRouting::UI);
	m_DisabledPickedSound = m_PieMenuExitSound;

	m_FundsChangedSound->Create("Base.rte/Sounds/GUIs/FundsChanged1.flac", true, false, SoundContainer::BusRouting::UI);
	m_FundsChangedSound->GetTopLevelSoundSet().AddSound("Base.rte/Sounds/GUIs/FundsChanged2.flac", true);
	m_FundsChangedSound->GetTopLevelSoundSet().AddSound("Base.rte/Sounds/GUIs/FundsChanged3.flac", true);
	m_FundsChangedSound->GetTopLevelSoundSet().AddSound("Base.rte/Sounds/GUIs/FundsChanged4.flac", true);
	m_FundsChangedSound->GetTopLevelSoundSet().AddSound("Base.rte/Sounds/GUIs/FundsChanged5.flac", true);
	m_FundsChangedSound->GetTopLevelSoundSet().AddSound("Base.rte/Sounds/GUIs/FundsChanged6.flac", true);
	m_FundsChangedSound->SetSoundOverlapMode(SoundContainer::SoundOverlapMode::RESTART);

	m_ActorSwitchSound->Create("Base.rte/Sounds/GUIs/ActorSwitch.flac", true, false, SoundContainer::BusRouting::UI);

	m_BrainSwitchSound->Create("Base.rte/Sounds/GUIs/BrainSwitch.flac", true, false, SoundContainer::BusRouting::UI);

	m_CameraTravelSound->Create("Base.rte/Sounds/GUIs/CameraTravel1.flac", true, false, SoundContainer::BusRouting::UI);
	m_CameraTravelSound->GetTopLevelSoundSet().AddSound("Base.rte/Sounds/GUIs/CameraTravel2.flac", true);
	m_CameraTravelSound->GetTopLevelSoundSet().AddSound("Base.rte/Sounds/GUIs/CameraTravel3.flac", true);

	//		m_AreaPickedSound->Create("Base.rte/Sounds/GUIs/MenuEnter.flac", true, false, SoundContainer::BusRouting::UI);
	m_AreaPickedSound = m_ConfirmSound;

	//		m_ObjectPickedSound->Create("Base.rte/Sounds/GUIs/MenuEnter.flac", true, false, SoundContainer::BusRouting::UI);
	m_ObjectPickedSound = m_ConfirmSound;

	//		m_PurchaseMadeSound->Create("Base.rte/Sounds/GUIs/MenuEnter.flac", true, false, SoundContainer::BusRouting::UI);
	m_PurchaseMadeSound = m_ConfirmSound;

	m_PlacementBlip->Create("Base.rte/Sounds/GUIs/PlacementBlip.flac", true, false, SoundContainer::BusRouting::UI);

	m_PlacementThud->Create("Base.rte/Sounds/GUIs/PlacementThud1.flac", true, false, SoundContainer::BusRouting::UI);
	m_PlacementThud->GetTopLevelSoundSet().AddSound("Base.rte/Sounds/GUIs/PlacementThud2.flac", true);

	m_PlacementGravel->Create("Base.rte/Sounds/GUIs/PlacementGravel1.flac", true, false, SoundContainer::BusRouting::UI);
	m_PlacementGravel->GetTopLevelSoundSet().AddSound("Base.rte/Sounds/GUIs/PlacementGravel2.flac", true);
	m_PlacementGravel->GetTopLevelSoundSet().AddSound("Base.rte/Sounds/GUIs/PlacementGravel3.flac", true);
	m_PlacementGravel->GetTopLevelSoundSet().AddSound("Base.rte/Sounds/GUIs/PlacementGravel4.flac", true);
}
