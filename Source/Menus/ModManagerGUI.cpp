#include "ModManagerGUI.h"

#include "SettingsMan.h"
#include "PresetMan.h"
#include "WindowMan.h"
#include "DataModule.h"
#include "GlobalScript.h"

#include "GUI.h"
#include "AllegroScreen.h"
#include "GUIInputWrapper.h"
#include "GUICollectionBox.h"
#include "GUILabel.h"
#include "GUIButton.h"
#include "GUIListBox.h"

using namespace RTE;

ModManagerGUI::ModManagerGUI(AllegroScreen* guiScreen, GUIInputWrapper* guiInput, bool createForPauseMenu) {
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

	if (createForPauseMenu) {
		m_BackToMainButton->SetSize(120, 20);
		m_BackToMainButton->SetText("Back to Pause Menu");
	}
	m_BackToMainButton->SetPositionAbs((rootBox->GetWidth() - m_BackToMainButton->GetWidth()) / 2, modManagerMenuBox->GetYPos() + modManagerMenuBox->GetHeight() + 10);

	m_ModsListBox = dynamic_cast<GUIListBox*>(m_GUIControlManager->GetControl("ListBoxMods"));
	m_ModsListBox->SetMouseScrolling(true);
	m_ModsListBox->SetScrollBarThickness(15);
	m_ModsListBox->SetScrollBarPadding(2);

	m_ScriptsListBox = dynamic_cast<GUIListBox*>(m_GUIControlManager->GetControl("ListBoxScripts"));
	m_ScriptsListBox->SetMouseScrolling(true);
	m_ScriptsListBox->SetScrollBarThickness(15);
	m_ScriptsListBox->SetScrollBarPadding(2);

	m_ToggleModButton = dynamic_cast<GUIButton*>(m_GUIControlManager->GetControl("ButtonToggleMod"));
	m_ToggleScriptButton = dynamic_cast<GUIButton*>(m_GUIControlManager->GetControl("ButtonToggleScript"));
	m_ModOrScriptDescriptionLabel = dynamic_cast<GUILabel*>(m_GUIControlManager->GetControl("LabelDescription"));

	m_ModsListFetched = false;
	m_ScriptsListFetched = false;
}

void ModManagerGUI::PopulateKnownModsList() {
	for (int i = 0; i < g_PresetMan.GetTotalModuleCount(); ++i) {
		if (i >= g_PresetMan.GetOfficialModuleCount() && i < g_PresetMan.GetTotalModuleCount()) {
			if (const DataModule* dataModule = g_PresetMan.GetDataModule(i); dataModule && !dataModule->IsUserdata()) {
				ModRecord modRecord = {dataModule->GetFileName(), dataModule->GetFriendlyName(), dataModule->GetDescription(), g_SettingsMan.IsModDisabled(dataModule->GetFileName())};
				m_KnownMods.emplace_back(modRecord);
			}
		}
	}
	// Add missing data from disabled mods settings
	for (const auto& [modPath, modDisabled]: g_SettingsMan.GetDisabledModsMap()) {
		bool found = false;
		for (const ModRecord& knowModListEntry: m_KnownMods) {
			if (modPath == knowModListEntry.ModulePath) {
				found = true;
				break;
			}
		}
		if (!found) {
			ModRecord disabledModRecord = {modPath, "N/A, Module not loaded", "N/A, Module not loaded", modDisabled};
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
				+ "\n" + g_PresetMan.GetDataModule(currentModuleIndex)->GetDescription() 
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

		modRecord.Disabled = !modRecord.Disabled;
		if (modRecord.Disabled) {
			m_ToggleModButton->SetText("Enable Mod");
			if (disabledModsList.find(modRecord.ModulePath) != disabledModsList.end()) {
				disabledModsList.at(modRecord.ModulePath) = true;
			} else {
				disabledModsList.try_emplace(modRecord.ModulePath, true);
			}
		} else {
			m_ToggleModButton->SetText("Disable Mod");
			disabledModsList.at(modRecord.ModulePath) = false;
		}
		selectedItem->m_Name = modRecord.GetDisplayString();
		m_ModsListBox->SetSelectedIndex(index);
		m_ModsListBox->Invalidate();
		g_GUISound.ItemChangeSound()->Play();
	}
}

ModManagerGUI::ScriptRecord* ModManagerGUI::ScriptListExtraIndexToScriptRecord(int extraIndex) {
	
	auto [moduleInd, scriptInd] = ScriptListEntryDecodeExtraIndex(extraIndex);
	// if the list item is a module label header - return nullptr
	if (scriptInd == -1) {
		return nullptr;
	}
	// else - a script, decode where to access it from the item's extra value
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
	//if the list item is a module label header then collapse/expand it
	if (scriptInd == -1) {
		m_KnownScriptsPerModule[moduleInd].Collapsed ^= 1; //toggle it
		PopulateKnownScriptsList(true);
	}
	// else - it's a script, toggle it:
	else {
		std::unordered_map<std::string, bool>& enabledScriptList = g_SettingsMan.GetEnabledGlobalScriptMap();

		ScriptRecord* scriptRecord = ScriptListExtraIndexToScriptRecord(extraIndex);
		scriptRecord->Enabled = !scriptRecord->Enabled;
		if (scriptRecord->Enabled) {
			m_ToggleScriptButton->SetText("Disable Script");
			if (enabledScriptList.find(scriptRecord->ModuleAndPresetName) != enabledScriptList.end()) {
				enabledScriptList.at(scriptRecord->ModuleAndPresetName) = true;
			} else {
				enabledScriptList.try_emplace(scriptRecord->ModuleAndPresetName, true);
			}
		} else {
			m_ToggleScriptButton->SetText("Enable Script");
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

	m_ModOrScriptDescriptionLabel->SetText(m_DisclaimerText);
}

bool ModManagerGUI::HandleInputEvents() {
	if (!ListsFetched()) {
		m_DisclaimerText = m_ModOrScriptDescriptionLabel->GetText();
		PopulateKnownModsList();
		InitializeKnownScripts();
		PopulateKnownScriptsList();
		ResetSelectionsAndGoToTop();
	}
	m_GUIControlManager->Update();

	GUIEvent guiEvent;
	while (m_GUIControlManager->GetEvent(&guiEvent)) {
		// buttons
		if (guiEvent.GetType() == GUIEvent::Command) {
			if (guiEvent.GetControl() == m_BackToMainButton) {
				ResetSelectionsAndGoToTop();
				return true;
			} else if (guiEvent.GetControl() == m_ToggleModButton) {
				ToggleMod();
			} else if (guiEvent.GetControl() == m_ToggleScriptButton) {
				ToggleInScriptList();
			}
		} 
		
		else if (guiEvent.GetType() == GUIEvent::Notification) {
			// button hover sound
			if (guiEvent.GetMsg() == GUIButton::Focused && dynamic_cast<GUIButton*>(guiEvent.GetControl())) {
				g_GUISound.SelectionChangeSound()->Play();
			}

			// list entries
			if (guiEvent.GetControl() == m_ModsListBox && m_ModsListBox->GetSelectedIndex() > -1) {
				switch (guiEvent.GetMsg()) {
					case GUIListBox::Select: {
						g_GUISound.SelectionChangeSound()->Play();
						const ModRecord& modRecord = m_KnownMods.at(m_ModsListBox->GetSelected()->m_ExtraIndex);
						m_ModOrScriptDescriptionLabel->SetText(modRecord.Description);
						m_ToggleModButton->SetText(modRecord.Disabled ? "Enable Mod" : "Disable Mod");
						break;
					}
					case GUIListBox::KeyDown:
						if (guiEvent.GetData() != 13) //enter key but doesnt work, todo
							break;
					case GUIListBox::DoubleClick:
						g_GUISound.SelectionChangeSound()->FadeOut(0);
						g_GUISound.ItemChangeSound()->Play();
						ToggleMod();
						break;
				}
			} else if (guiEvent.GetControl() == m_ScriptsListBox && m_ScriptsListBox->GetSelectedIndex() > -1) {
				switch (guiEvent.GetMsg()) {
					case GUIListBox::Select: {
						g_GUISound.SelectionChangeSound()->Play();
						int extraIndex = m_ScriptsListBox->GetSelected()->m_ExtraIndex;
						auto [moduleInd, scriptInd] = ScriptListEntryDecodeExtraIndex(extraIndex);
						// if we're on a script item
						if (scriptInd != -1) {
							const ScriptRecord* scriptRecord = ScriptListExtraIndexToScriptRecord(extraIndex);
							m_ModOrScriptDescriptionLabel->SetText(scriptRecord->Description.empty() ? "No description." : scriptRecord->Description);
							m_ToggleScriptButton->SetText(scriptRecord->Enabled ? "Disable Script" : "Enable Script");
						}
						// if we're on a module label
						else {
							ScriptRecordsInAModule* module =
							    &m_KnownScriptsPerModule[moduleInd];
							m_ModOrScriptDescriptionLabel->SetText(module->Description);
							m_ToggleScriptButton->SetText(module->Collapsed ? "Expand Category" : "Collapse Category");
						}
						break;
					}
					case GUIListBox::KeyDown:
						if (guiEvent.GetData() != 13) //todo, check above
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

void ModManagerGUI::Draw() const {
	m_GUIControlManager->Draw();
}
