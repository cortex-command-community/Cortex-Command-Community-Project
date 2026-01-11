#include "GlobalScript.h"

#include "LuaMan.h"
#include "MovableMan.h"
#include "PresetMan.h"

#include "ACraft.h"
#include "PieSlice.h"

#include "ConsoleMan.h"
#include "SettingsMan.h"

using namespace RTE;

ConcreteClassInfo(GlobalScript, Entity, 10);

GlobalScript::GlobalScript() {
	Clear();
}

GlobalScript::~GlobalScript() {
	Destroy(true);
}

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
	MatchProperty("_ClearPieSlices", {
		reader.ReadPropValue();
		m_PieSlicesToAdd.clear();
	});
	MatchForwards("AddPieSlice") MatchProperty("_AddPieSlice", { m_PieSlicesToAdd.emplace_back(std::unique_ptr<PieSlice>(dynamic_cast<PieSlice*>(g_PresetMan.ReadReflectedPreset(reader)))); });

	EndPropertyList;
}

int GlobalScript::Save(Writer& writer) const {
	Entity::Save(writer);

	if (!m_ScriptPath.empty())
		writer.NewPropertyWithValue("ScriptPath", m_ScriptPath);

	if (!m_LuaClassName.empty())
		writer.NewPropertyWithValue("LuaClassName", m_LuaClassName);

	writer.NewDistinctProperty("LateUpdate", m_LateUpdate, false);

	for (const std::unique_ptr<PieSlice>& pieSliceToAdd: m_PieSlicesToAdd) {
		writer.NewPropertyWithValue("_AddPieSlice", pieSliceToAdd.get());
	}

	return 0;
}

int GlobalScript::Write(Writer& writer, const Entity& entityReference, HashingData& hashData) const {
	Entity::Write(writer, entityReference, hashData);

	const GlobalScript& reference = static_cast<const GlobalScript&>(entityReference);

	writer.NewDistinctProperty("ScriptPath", m_ScriptPath, reference.m_ScriptPath);
	writer.NewDistinctProperty("LuaClassName", m_LuaClassName, reference.m_LuaClassName);
	writer.NewDistinctProperty("LateUpdate", m_LateUpdate, reference.m_LateUpdate);
	writer.NewPointerSequence("_ClearPieSlices", "_AddPieSlice", m_PieSlicesToAdd, hashData);

	return 0;
}

HashingData GlobalScript::Hash() const {
	HashingData hashData(Entity::Hash());
	uint64_t& hash = hashData.m_Hash;

	hash ^= static_cast<uint64_t>(m_LateUpdate) << 0;
	hash ^= RTE::Hash(m_ScriptPath) << 1;
	hash ^= RTE::Hash(m_LuaClassName) << 2;

	int i = 0;

	for (const auto& pieSlice: m_PieSlicesToAdd) {
		uint64_t sliceHash = pieSlice->Hash().m_Hash;
		hashData.m_Constituents.push_back(sliceHash);
		hash ^= sliceHash << (i++ % sizeof(uint64_t) * 8);
	}

	hashData.m_ParseValues.push_back(i);

	return hashData;
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
