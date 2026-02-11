#include "ModManagerGUI.h"

#include "SettingsMan.h"
#include "PresetMan.h"
#include "WindowMan.h"
#include "FrameMan.h"
#include "DataModule.h"
#include "GlobalScript.h"

#include "GUI.h"
#include "AllegroScreen.h"
#include "GUIInputWrapper.h"
#include "GUICollectionBox.h"
#include "GUILabel.h"
#include "GUIButton.h"
#include "GUIListBox.h"

// Includes for OpenModsFolder() on Windows
#if defined(_WIN32)
	#include <windows.h>
	#include <shellapi.h>
#endif

using namespace RTE;

ModManagerGUI::ModManagerGUI(AllegroScreen* guiScreen, GUIInputWrapper* guiInput) {
	m_GUIControlManager = std::make_unique<GUIControlManager>();
	RTEAssert(m_GUIControlManager->Create(guiScreen, guiInput, "Base.rte/GUIs/Skins/Menus", "MainMenuSubMenuSkin.ini"), "Failed to create GUI Control Manager and load it from Base.rte/GUIs/Skins/Menus/MainMenuSubMenuSkin.ini");
	m_GUIControlManager->Load("Base.rte/GUIs/ModManagerGUI.ini");

	int rootBoxMaxWidth = g_WindowMan.FullyCoversAllDisplays() ? g_WindowMan.GetPrimaryWindowDisplayWidth() / g_WindowMan.GetResMultiplier() : g_WindowMan.GetResX();

	GUICollectionBox* rootBox = dynamic_cast<GUICollectionBox*>(m_GUIControlManager->GetControl("root"));
	rootBox->Resize(rootBoxMaxWidth, g_WindowMan.GetResY());

	GUICollectionBox* modManagerMenuBox = dynamic_cast<GUICollectionBox*>(m_GUIControlManager->GetControl("CollectionBoxModManager"));
	modManagerMenuBox->CenterInParent(true, true);
	modManagerMenuBox->SetPositionAbs(modManagerMenuBox->GetXPos(), (rootBox->GetHeight() < 540) ? modManagerMenuBox->GetYPos() - 15 : 140);

	m_BackToMainButton = dynamic_cast<GUIButton*>(m_GUIControlManager->GetControl("ButtonBackToMainMenu"));
	m_OpenModsFolderButton = dynamic_cast<GUIButton*>(m_GUIControlManager->GetControl("ButtonOpenModsFolder"));

	m_ModsTabButton = dynamic_cast<GUIButton*>(m_GUIControlManager->GetControl("ButtonModsTab"));
	m_ScriptsTabButton = dynamic_cast<GUIButton*>(m_GUIControlManager->GetControl("ButtonScriptsTab"));

	m_ModuleHeaderLabel = dynamic_cast<GUILabel*>(m_GUIControlManager->GetControl("LabelModuleHeader"));
	m_ModuleHeaderLabel->SetFont(m_GUIControlManager->GetSkin()->GetFont("FontMainMenu.png"));

	m_ModsListBox = dynamic_cast<GUIListBox*>(m_GUIControlManager->GetControl("ListBoxMods"));
	m_ModsListBox->SetMouseScrolling(true);
	m_ModsListBox->SetScrollBarThickness(15);
	m_ModsListBox->SetScrollBarPadding(2);

	m_ScriptsListBox = dynamic_cast<GUIListBox*>(m_GUIControlManager->GetControl("ListBoxScripts"));
	m_ScriptsListBox->SetMouseScrolling(true);
	m_ScriptsListBox->SetScrollBarThickness(15);
	m_ScriptsListBox->SetScrollBarPadding(2);

	m_ToggleListEntryButton = dynamic_cast<GUIButton*>(m_GUIControlManager->GetControl("ButtonToggleListEntry"));
	m_ToggleListEntryButton->SetVisible(false);
	m_ToggleListEntryButton->SetEnabled(false);
	m_ModOrScriptDescriptionLabel = dynamic_cast<GUILabel*>(m_GUIControlManager->GetControl("LabelDescription"));

	m_ModuleIconCollectionBox = dynamic_cast<GUICollectionBox*>(m_GUIControlManager->GetControl("CollectionBoxModuleHeaderModuleIcon"));
	m_ModuleIconCollectionBox->SetVisible(false);

	m_ModsListFetched = false;
	m_ScriptsListFetched = false;
}

void ModManagerGUI::PopulateKnownModsList() {
	for (int i = 0; i < g_PresetMan.GetTotalModuleCount(); ++i) {
		if (i >= g_PresetMan.GetOfficialModuleCount() && i < g_PresetMan.GetTotalModuleCount()) {
			if (const DataModule* dataModule = g_PresetMan.GetDataModule(i); dataModule && !dataModule->IsUserdata()) {
				ModRecord modRecord(dataModule, i);
				m_KnownMods.emplace_back(modRecord);
			}
		}
	}
	// Add missing data from disabled mods settings
	for (const auto& [modPath, modDisabled]: g_SettingsMan.GetDisabledModsMap()) {
		bool found = false;
		for (const ModRecord& knowModListEntry: m_KnownMods) {
			if (modPath == knowModListEntry.GetDotRTEName()) {
				found = true;
				break;
			}
		}
		if (!found) {
			ModRecord disabledModRecord(modPath);
			m_KnownMods.emplace_back(disabledModRecord);
		}
	}
	std::sort(m_KnownMods.begin(), m_KnownMods.end());

	for (int i = 0; i < m_KnownMods.size(); i++) {
		m_ModsListBox->AddItem(m_KnownMods.at(i).GetDisplayString(), std::string(), nullptr, nullptr, i);
	}
	m_ModsListBox->ScrollToTop();
	m_ModsListFetched = true;
}

//todo to fetch
void ModManagerGUI::InitializeKnownScripts() {
	int totalModuleCount = g_PresetMan.GetTotalModuleCount();
	for (int currentModuleIndex = 0; currentModuleIndex < totalModuleCount; ++currentModuleIndex) {
		// todo: try GetFriendlyName()
		std::list<Entity*> globalScriptListForThisModule;
		g_PresetMan.GetAllOfType(globalScriptListForThisModule, "GlobalScript", currentModuleIndex);

		if (globalScriptListForThisModule.empty()) {
			continue;
		}


		std::vector<ScriptRecord>& currentModuleScripts =
		    m_KnownScriptsPerModule.emplace_back(
		        g_PresetMan.GetDataModule(currentModuleIndex)->GetFileName(),
		        g_PresetMan.GetDataModule(currentModuleIndex)->GetFriendlyName() 
					+ "\n" + g_PresetMan.GetDataModule(currentModuleIndex)->GetDescription(),
				currentModuleIndex
			).Records;

		for (Entity* globalScriptListEntry: globalScriptListForThisModule) {
			if (const GlobalScript* globalScript = dynamic_cast<GlobalScript*>(globalScriptListEntry)) {
				ScriptRecord scriptRecord =
				    {globalScript->GetPresetName(),
				     globalScript->GetModuleAndPresetName(),
				     globalScript->GetDescription(),
				     g_SettingsMan.IsGlobalScriptEnabled(globalScript->GetModuleAndPresetName())};
				currentModuleScripts.emplace_back(scriptRecord);
			}
		}
		std::sort(currentModuleScripts.begin(), currentModuleScripts.end());
	}
	m_ScriptsListBox->ScrollToTop();
	m_ScriptsListFetched = true;
}

int ModManagerGUI::ScriptListEntryEncodeExtraIndex(const int moduleNumber, const int scriptNumber) {
	return scriptNumber == -1 
		? (moduleNumber << 10) | EXTRA_INDEX_IS_A_MODULE_LABEL_MASK 
		: (moduleNumber << 10) + scriptNumber;
}

std::pair<int, int> ModManagerGUI::ScriptListEntryDecodeExtraIndex(const int extraIndex) {
	if (extraIndex & EXTRA_INDEX_IS_A_MODULE_LABEL_MASK) {
		return {(extraIndex ^ EXTRA_INDEX_IS_A_MODULE_LABEL_MASK) >> 10, -1};
	} else {
		return {extraIndex >> 10, extraIndex - ((extraIndex >> 10) << 10)};
	}
}

void ModManagerGUI::PopulateKnownScriptsList(bool clearBeforehand = false) {
	if (clearBeforehand) {
		m_ScriptsListBox->ClearList();
	}

	for (int moduleIt = 0; moduleIt < m_KnownScriptsPerModule.size(); ++moduleIt) {
		auto& scriptRecordsInAModule = m_KnownScriptsPerModule[moduleIt];

		//display the module ".rte" thingy
		m_ScriptsListBox->AddItem(scriptRecordsInAModule.GetDisplayString(), 
			std::string(), nullptr, nullptr, ScriptListEntryEncodeExtraIndex(moduleIt, -1));

		if (scriptRecordsInAModule.Collapsed) {
			continue;
		}

		// then per each .rte display the scripts under:
		for (int scriptIt = 0; scriptIt < scriptRecordsInAModule.Records.size(); ++scriptIt) {
			auto& scriptRecord = scriptRecordsInAModule.Records[scriptIt];
			m_ScriptsListBox->AddItem(scriptRecord.GetDisplayString(), 
				std::string(), nullptr, nullptr, ScriptListEntryEncodeExtraIndex(moduleIt, scriptIt));
		}
	}	
}

void ModManagerGUI::ToggleMod() {
	int index = m_ModsListBox->GetSelectedIndex();
	if (index > -1) {
		std::unordered_map<std::string, bool>& disabledModsList = g_SettingsMan.GetDisabledModsMap();
		GUIListPanel::Item* selectedItem = m_ModsListBox->GetSelected();
		ModRecord& modRecord = m_KnownMods.at(selectedItem->m_ExtraIndex);

		modRecord.Enabled = !modRecord.Enabled;
		if (!modRecord.Enabled) {
			m_ToggleListEntryButton->SetText("Enable Mod");
			if (disabledModsList.find(modRecord.GetDotRTEName()) != disabledModsList.end()) {
				disabledModsList.at(modRecord.GetDotRTEName()) = true;
			} else {
				disabledModsList.try_emplace(modRecord.GetDotRTEName(), true);
			}
		} else {
			m_ToggleListEntryButton->SetText("Disable Mod");
			disabledModsList.at(modRecord.GetDotRTEName()) = false;
		}
		selectedItem->m_Name = modRecord.GetDisplayString();
		m_ModsListBox->SetSelectedIndex(index);
		m_ModsListBox->Invalidate();
		g_GUISound.ItemChangeSound()->Play();
	}
}

ModManagerGUI::ScriptRecord* ModManagerGUI::ScriptListExtraIndexToScriptRecord(int extraIndex) {
	auto [moduleInd, scriptInd] = ScriptListEntryDecodeExtraIndex(extraIndex);
	// If the list item is a module label header - return nullptr
	if (scriptInd == -1) {
		return nullptr;
	}
	// Else - a script, decode where to access it from the item's extra value
	return &m_KnownScriptsPerModule.at(moduleInd).Records.at(scriptInd);
}

void ModManagerGUI::ToggleInScriptList() {
	int index = m_ScriptsListBox->GetSelectedIndex();
	if (index <= -1) {
		return;
	}

	GUIListPanel::Item* selectedItem = m_ScriptsListBox->GetSelected();
	int extraIndex = selectedItem->m_ExtraIndex;
	auto [moduleInd, scriptInd] = ScriptListEntryDecodeExtraIndex(extraIndex);
	// If the list item is a module label header then collapse/expand it
	if (scriptInd == -1) {
		m_KnownScriptsPerModule[moduleInd].Collapsed ^= 1; //toggle it
		PopulateKnownScriptsList(true);
	}
	// Else - it's a script, toggle it:
	else {
		std::unordered_map<std::string, bool>& enabledScriptList = g_SettingsMan.GetEnabledGlobalScriptMap();

		ScriptRecord* scriptRecord = ScriptListExtraIndexToScriptRecord(extraIndex);
		scriptRecord->Enabled = !scriptRecord->Enabled;
		if (scriptRecord->Enabled) {
			m_ToggleListEntryButton->SetText("Disable Script");
			if (enabledScriptList.find(scriptRecord->ModuleAndPresetName) != enabledScriptList.end()) {
				enabledScriptList.at(scriptRecord->ModuleAndPresetName) = true;
			} else {
				enabledScriptList.try_emplace(scriptRecord->ModuleAndPresetName, true);
			}
		} else {
			m_ToggleListEntryButton->SetText("Enable Script");
			enabledScriptList.at(scriptRecord->ModuleAndPresetName) = false;
		}

		selectedItem->m_Name = scriptRecord->GetDisplayString();
	}
	m_ScriptsListBox->SetSelectedIndex(index);
	m_ScriptsListBox->Invalidate();
	g_GUISound.ItemChangeSound()->Play();
}

void ModManagerGUI::ResetSelectionsAndGoToTop() {
	m_ModsListBox->ScrollToTop();
	m_ScriptsListBox->ScrollToTop();
	m_ModsListBox->SetSelectedIndex(-1);
	m_ScriptsListBox->SetSelectedIndex(-1);
	m_ModOrScriptDescriptionLabel->SetText(GetDisclaimerText());
}

void ModManagerGUI::OpenModsFolder() {
	static const char folder[] = ".\\Mods";
	#if defined(_WIN32)
	ShellExecuteA(nullptr, "open", folder, nullptr, nullptr, 10);
	// TODO: Confirm this works on Mac and Linux
	#elif defined(__APPLE__)
	system(("open \"" + folder + "\"").c_str());
	#elif defined(__linux__)
	system(("xdg-open \"" + folder + "\" &").c_str());
	#endif
}

bool ModManagerGUI::HandleInputEvents() {
	if (!ListsFetched()) {
		m_ModOrScriptDescriptionLabel->SetText(GetDisclaimerText());
		PopulateKnownModsList();
		InitializeKnownScripts();
		PopulateKnownScriptsList();
		ResetSelectionsAndGoToTop();
	}
	m_GUIControlManager->Update();

	GUIEvent guiEvent;
	while (m_GUIControlManager->GetEvent(&guiEvent)) {
		static auto toggleToggleEntryButton = [&](bool enable) {
			m_ToggleListEntryButton->SetVisible(enable);
			m_ToggleListEntryButton->SetEnabled(enable);
		};
		static auto resetRightHandSide = [&]() {
			UpdateModuleHeader(-1);
			m_ModOrScriptDescriptionLabel->SetText(GetDisclaimerText());
			m_ToggleListEntryButton->SetVisible(false);
			m_ToggleListEntryButton->SetEnabled(false);
			toggleToggleEntryButton(false);
		};
		// Buttons
		if (guiEvent.GetType() == GUIEvent::Command) {
			if (guiEvent.GetControl() == m_BackToMainButton) {
				ResetSelectionsAndGoToTop();
				toggleToggleEntryButton(false);
				return true;
			} 
			else if (guiEvent.GetControl() == m_OpenModsFolderButton) {
				OpenModsFolder();
			}
			else if (guiEvent.GetControl() == m_ToggleListEntryButton) {
				if (m_CurrentTab == ModManagerTab::Mods) {
					ToggleMod();
				}
				else if (m_CurrentTab == ModManagerTab::Scripts) {
					ToggleInScriptList();
				}
			}
			// Mods tab clicked
			else if (guiEvent.GetControl() == m_ModsTabButton) {
				g_GUISound.BackButtonPressSound()->Play();
				m_ModsListBox->SetEnabled(true);
				m_ScriptsListBox->SetEnabled(false);
				m_ModsListBox->SetVisible(true);
				m_ScriptsListBox->SetVisible(false);
				m_ScriptsTabButton->SetEnabled(true);
				m_ModsTabButton->SetEnabled(false);
				resetRightHandSide();
				ResetSelectionsAndGoToTop();
				toggleToggleEntryButton(false);
			}
			// Scripts tab clicked
			else if (guiEvent.GetControl() == m_ScriptsTabButton) {
				g_GUISound.BackButtonPressSound()->Play();
				m_ModsListBox->SetEnabled(false);
				m_ScriptsListBox->SetEnabled(true);
				m_ModsListBox->SetVisible(false);
				m_ScriptsListBox->SetVisible(true);
				m_ScriptsTabButton->SetEnabled(false);
				m_ModsTabButton->SetEnabled(true);
				resetRightHandSide();
				ResetSelectionsAndGoToTop();
				toggleToggleEntryButton(false);
			}
			m_GUIControlManager->GetManager()->SetFocus(nullptr);
		} 
		
		else if (guiEvent.GetType() == GUIEvent::Notification) {
			// Button hover sound
			if (guiEvent.GetMsg() == GUIButton::Focused && dynamic_cast<GUIButton*>(guiEvent.GetControl())) {
				g_GUISound.SelectionChangeSound()->Play();
			}

			// List entries:
			// 1. Mod list
			if (guiEvent.GetControl() == m_ModsListBox && m_ModsListBox->GetSelectedIndex() > -1) {
				switch (guiEvent.GetMsg()) {
					case GUIListBox::Select: {
						g_GUISound.SelectionChangeSound()->Play();
						toggleToggleEntryButton(true);
						const int modIt = m_ModsListBox->GetSelected()->m_ExtraIndex;
						const ModRecord& modRecord = m_KnownMods.at(modIt);
						m_ModOrScriptDescriptionLabel->SetText(modRecord.GetDescription());
						m_ToggleListEntryButton->SetText(modRecord.Enabled ? "Disable Mod" : "Enable Mod");
						UpdateModuleHeader(modRecord.InternalModuleIndex);

						break;
					}
					case GUIListBox::KeyDown:
						if (guiEvent.GetData() != 13) // TODO: Enter key but doesnt work, also below
							break;
					case GUIListBox::DoubleClick:
						g_GUISound.SelectionChangeSound()->FadeOut(0);
						g_GUISound.ItemChangeSound()->Play();
						ToggleMod();
						break;
				}
			} 
			// 2. Script list
			else if (guiEvent.GetControl() == m_ScriptsListBox && m_ScriptsListBox->GetSelectedIndex() > -1) {
				switch (guiEvent.GetMsg()) {
					case GUIListBox::Select: {
						toggleToggleEntryButton(true);
						g_GUISound.SelectionChangeSound()->Play();
						int extraIndex = m_ScriptsListBox->GetSelected()->m_ExtraIndex;
						auto [moduleInd, scriptInd] = ScriptListEntryDecodeExtraIndex(extraIndex);
						// if we're on a script item
						if (scriptInd != -1) {
							const ScriptRecord* scriptRecord = ScriptListExtraIndexToScriptRecord(extraIndex);
							const ScriptRecordsInAModule& module = m_KnownScriptsPerModule[moduleInd];
							m_ModOrScriptDescriptionLabel->SetText(
								scriptRecord->Description.empty() 
								? "No description." 
								: scriptRecord->DisplayName + "\n\n" + scriptRecord->Description);
							m_ToggleListEntryButton->SetText(scriptRecord->Enabled ? "Disable Script" : "Enable Script");
							UpdateModuleHeader(module.InternalModuleIndex);
						}
						// If we're on a module label
						else {
							const ScriptRecordsInAModule& module = m_KnownScriptsPerModule[moduleInd];
							m_ModOrScriptDescriptionLabel->SetText(module.Description);
							UpdateModuleHeader(module.InternalModuleIndex);
							m_ToggleListEntryButton->SetText(module.Collapsed ? "Expand Category" : "Collapse Category");
						}
						break;
					}
					case GUIListBox::KeyDown:
						if (guiEvent.GetData() != 13) // TODO, see above
							break;
					case GUIListBox::DoubleClick:
						g_GUISound.SelectionChangeSound()->FadeOut(0);
						g_GUISound.ItemChangeSound()->Play();
						ToggleInScriptList();
						break;
				}
			}
		}
	}
	return false;
}

void ModManagerGUI::UpdateModuleHeader(const int moduleIndex) {
	if (moduleIndex == -1) {
		m_ModuleHeaderLabel->SetText("");
		m_ModuleIconCollectionBox->SetVisible(false);
		m_ModuleIconCollectionBox->SetEnabled(false);

		m_ModuleHeaderLabel->SetSize(199, m_ModuleHeaderLabel->GetHeight());
		m_ModuleHeaderLabel->SetPositionRel(13, 2);
		m_ModuleHeaderLabel->SetHAlignment(GUIFont::Centre);

		return;
	}

	const DataModule* module = g_PresetMan.GetDataModule(moduleIndex);
	// Header text change
	std::string formattedStr = module->GetFriendlyName();
	if (formattedStr == "") {
		formattedStr = g_PresetMan.GetDataModuleName(moduleIndex);
		transform(formattedStr.begin(), formattedStr.end(), formattedStr.begin(), ::tolower);
		if (formattedStr.ends_with(".rte")) {
			formattedStr.erase(formattedStr.size() - 4);
		}
	} else {
		transform(formattedStr.begin(), formattedStr.end(), formattedStr.begin(), ::tolower);
	}
	
	m_ModuleHeaderLabel->SetText(std::move(formattedStr));

	// Header icon change
	BITMAP* selectedModuleIconBm = module->GetIcon();
	if (selectedModuleIconBm != 0) {
		constexpr int maxSize = 46;
		// We need to scale the module icons to 46-by-46
		// They're usually 23-by-23, but might not be
		// And so we preserve aspect ratio by getting scaling factors:
		if (selectedModuleIconBm->h == 0 || selectedModuleIconBm->w == 0) {
			return;
		}
		float aspect = float(selectedModuleIconBm->w) / float(selectedModuleIconBm->h);
		int scaledW, scaledH, centeredX, centeredY;
		if (aspect >= 1.0f) {
			// Width > Height
			scaledW = maxSize;
			scaledH = int(float(maxSize) / aspect + 0.5f);
			centeredX = 0;
			centeredY = (maxSize - scaledH) / 2;
		} else {
			// Height > Width
			scaledH = maxSize;
			scaledW = int(float(maxSize) * aspect + 0.5f);
			centeredX = (maxSize - scaledW) / 2;
			centeredY = 0;
		}

		// Unique_ptr so it gets GC'd
		std::unique_ptr<BITMAP> newIconBm8Streched(create_bitmap_ex(8, maxSize, maxSize));
		clear_to_color(newIconBm8Streched.get(), g_MaskColor);

		// We stretch in a separate step, because BMs need be of same color depth
		masked_stretch_blit(selectedModuleIconBm, newIconBm8Streched.get(), 0, 0, selectedModuleIconBm->w, selectedModuleIconBm->h, centeredX, centeredY, scaledW, scaledH);

		// Non-unique because GUI elements own their BM*'s and GC on change
		BITMAP* newIconBm32(create_bitmap_ex(32, 50, 50));

		// We blit the old one onto new to preserve borders
		blit(m_ModuleIconCollectionBox->GetDrawImage()->GetBitmap(), newIconBm32, 0, 0, 0, 0, 50, 50);

		// And then clear out middle between borders, so transparent module icons dont stack
		set_clip_rect(newIconBm32, 2, 2, 47, 47);
		clear_to_color(newIconBm32, g_MaskColor);
		set_clip_rect(newIconBm32, 0, 0, 50, 50);

		// And then draw the stretched module icon off-corner in the middle (to keep borders)
		draw_sprite(newIconBm32, newIconBm8Streched.get(), 2, 2);
		
		// Done, pass ownership
		m_ModuleIconCollectionBox->SetDrawImage(new AllegroBitmap(newIconBm32));

		m_ModuleIconCollectionBox->SetVisible(true);
		m_ModuleIconCollectionBox->SetEnabled(true);

		m_ModuleHeaderLabel->SetSize(149, m_ModuleHeaderLabel->GetHeight());
		m_ModuleHeaderLabel->SetPositionRel(63, 2);
		m_ModuleHeaderLabel->SetHAlignment(GUIFont::Left);
	} else {
		// Doesn't matter what the icon BM is - just hide it
		m_ModuleIconCollectionBox->SetVisible(false);
		m_ModuleIconCollectionBox->SetEnabled(false);

		m_ModuleHeaderLabel->SetSize(199, m_ModuleHeaderLabel->GetHeight());
		m_ModuleHeaderLabel->SetPositionRel(13, 2);
		m_ModuleHeaderLabel->SetHAlignment(GUIFont::Centre);
	}
}

void ModManagerGUI::Draw() const {
	m_GUIControlManager->Draw();
}
