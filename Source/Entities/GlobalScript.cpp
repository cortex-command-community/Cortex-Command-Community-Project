#include "GlobalScript.h"

#include "LuaMan.h"
#include "MovableMan.h"
#include "PresetMan.h"

#include "ACraft.h"
#include "PieSlice.h"

#include "ConsoleMan.h"
#include "SettingsMan.h"

namespace RTE {

	ConcreteClassInfo(GlobalScript, Entity, 10);

	void GlobalScript::Clear() {
		m_ScriptPath.clear();
		m_LuaClassName.clear();
		m_IsActive = true;
		m_HasStarted = false;
		m_LateUpdate = false;
		m_PieSlicesToAdd.clear();
	}

	int GlobalScript::Create(const GlobalScript& reference) {
		Entity::Create(reference);

		m_ScriptPath = reference.m_ScriptPath;
		m_LuaClassName = reference.m_LuaClassName;
		m_IsActive = reference.m_IsActive;
		m_HasStarted = reference.m_HasStarted;
		m_LateUpdate = reference.m_LateUpdate;

		for (const std::unique_ptr<PieSlice>& referencePieSliceToAdd: reference.m_PieSlicesToAdd) {
			m_PieSlicesToAdd.emplace_back(std::unique_ptr<PieSlice>(dynamic_cast<PieSlice*>(referencePieSliceToAdd->Clone())));
		}

		return 0;
	}

	int GlobalScript::ReadProperty(const std::string_view& propName, Reader& reader) {
		StartPropertyList(return Entity::ReadProperty(propName, reader));

		MatchProperty("ScriptPath", { m_ScriptPath = CorrectBackslashesInPath(reader.ReadPropValue()); });
		MatchProperty("LuaClassName", { reader >> m_LuaClassName; });
		MatchProperty("LateUpdate", { reader >> m_LateUpdate; });
		MatchProperty("AddPieSlice", { m_PieSlicesToAdd.emplace_back(std::unique_ptr<PieSlice>(dynamic_cast<PieSlice*>(g_PresetMan.ReadReflectedPreset(reader)))); });

		EndPropertyList;
	}

	int GlobalScript::Save(Writer& writer) const {
		Entity::Save(writer);

		writer.NewPropertyWithValue("ScriptPath", m_ScriptPath);
		writer.NewPropertyWithValue("LuaClassName", m_LuaClassName);
		writer.NewPropertyWithValue("LateUpdate", m_LateUpdate);

		for (const std::unique_ptr<PieSlice>& pieSliceToAdd: m_PieSlicesToAdd) {
			writer.NewPropertyWithValue("AddPieSlice", pieSliceToAdd.get());
		}

		return 0;
	}

	const std::vector<std::unique_ptr<PieSlice>>& GlobalScript::GetPieSlicesToAdd() const {
		static const std::vector<std::unique_ptr<PieSlice>> emptyVector;
		if (!m_HasStarted || !m_IsActive || !g_SettingsMan.IsGlobalScriptEnabled(GetModuleAndPresetName())) {
			return emptyVector;
		}

		return m_PieSlicesToAdd;
	}

	int GlobalScript::ReloadScripts() {
		int error = 0;

		if (!m_ScriptPath.empty()) {
			if (!g_LuaMan.GetMasterScriptState().GlobalIsDefined(m_LuaClassName)) {
				g_LuaMan.GetMasterScriptState().SetTempEntity(this);
				error = g_LuaMan.GetMasterScriptState().RunScriptString(m_LuaClassName + " = ToGlobalScript(LuaMan.TempEntity);");
			}
			if (error == 0) {
				g_LuaMan.GetMasterScriptState().RunScriptFile(m_ScriptPath);
			}
		}

		return error;
	}

	int GlobalScript::Start() {
		if (!g_SettingsMan.IsGlobalScriptEnabled(GetModuleAndPresetName())) {
			return 0;
		}

		if (g_SettingsMan.PrintDebugInfo()) {
			g_ConsoleMan.PrintString("DEBUG: Start Global Script: " + GetPresetName());
		}

		int error = ReloadScripts();
		if (error == 0) {
			error = g_LuaMan.GetMasterScriptState().RunScriptString("if " + m_LuaClassName + ".StartScript then " + m_LuaClassName + ":StartScript(); end");
			m_HasStarted = true;
		}

		m_IsActive = error == 0;
		return error;
	}

	int GlobalScript::Pause(bool pause) const {
		if (!m_IsActive || !m_HasStarted || !g_SettingsMan.IsGlobalScriptEnabled(GetModuleAndPresetName())) {
			return 0;
		}

		return g_LuaMan.GetMasterScriptState().RunScriptString("if " + m_LuaClassName + ".PauseScript then " + m_LuaClassName + ":PauseScript(" + (pause ? "true" : "false") + "); end");
	}

	int GlobalScript::End() const {
		if (!m_HasStarted) {
			return 0;
		}

		if (g_SettingsMan.PrintDebugInfo()) {
			g_ConsoleMan.PrintString("DEBUG: End Global Script: " + GetPresetName());
		}

		return g_LuaMan.GetMasterScriptState().RunScriptString("if " + m_LuaClassName + ".EndScript then " + m_LuaClassName + ":EndScript(); end");
	}

	void GlobalScript::HandleCraftEnteringOrbit(const ACraft* orbitedCraft) {
		if (!m_IsActive || !!m_HasStarted || orbitedCraft == nullptr || !g_MovableMan.IsActor(orbitedCraft) || !g_SettingsMan.IsGlobalScriptEnabled(GetModuleAndPresetName())) {
			return;
		}

		int error = g_LuaMan.GetMasterScriptState().RunScriptFunctionString(m_LuaClassName + ".CraftEnteredOrbit", m_LuaClassName, {m_LuaClassName, m_LuaClassName + ".CraftEnteredOrbit"}, {orbitedCraft});
		if (error) {
			m_IsActive = false;
		}
	}

	void GlobalScript::Update() {
		if (!m_IsActive) {
			return;
		}

		if (!g_SettingsMan.IsGlobalScriptEnabled(GetModuleAndPresetName())) {
			if (m_HasStarted) {
				End();
			}
			return;
		}

		if (!m_HasStarted) {
			Start();
		}

		int error = g_LuaMan.GetMasterScriptState().RunScriptString("if " + m_LuaClassName + ".UpdateScript then " + m_LuaClassName + ":UpdateScript(); end");
		if (error) {
			m_IsActive = false;
		}
	}
} // namespace RTE
