#include "PieSlice.h"

#include "PieMenu.h"
#include "PresetMan.h"
#include "LuaMan.h"

using namespace RTE;

ConcreteClassInfo(PieSlice, Entity, 80);

PieSlice::PieSlice() {
		Clear();
}

PieSlice::~PieSlice() {
		Destroy(true);
}

void PieSlice::Clear() {
		m_Type = PieSliceType::NoType;
		m_Direction = Directions::Any;
		m_CanBeMiddleSlice = true;
		m_OriginalSource = nullptr;

		m_Enabled = true;
		m_Icon = nullptr;

		m_LuabindFunctionObject.reset();
		m_FunctionName.clear();

		m_SubPieMenu.reset();

		m_StartAngle = 0;
		m_SlotCount = 0;
		m_MidAngle = 0;

		m_DrawFlippedToMatchAbsoluteAngle = false;
}

int PieSlice::Create() {
		if (Entity::Create() < 0) {
			return -1;
		}
		if (!HasIcon()) {
			m_Icon = std::unique_ptr<Icon>(dynamic_cast<Icon*>(g_PresetMan.GetEntityPreset("Icon", "Blank")->Clone()));
		}

		return 0;
}

int PieSlice::Create(const PieSlice& reference) {
		Entity::Create(reference);

		m_Type = reference.m_Type;
		m_Direction = reference.m_Direction;
		m_CanBeMiddleSlice = reference.m_CanBeMiddleSlice;

		m_Enabled = reference.m_Enabled;
		m_Icon = std::unique_ptr<Icon>(dynamic_cast<Icon*>(reference.m_Icon->Clone()));

		m_FunctionName = reference.m_FunctionName;
		if (reference.m_LuabindFunctionObject) {
			m_LuabindFunctionObject = std::make_unique<SolObjectWrapper>(nullptr, reference.m_LuabindFunctionObject->GetFilePath());
			ReloadScripts();
		}

		if (reference.m_SubPieMenu) {
			SetSubPieMenu(dynamic_cast<PieMenu*>(reference.m_SubPieMenu->Clone()));
		}

		m_StartAngle = reference.m_StartAngle;
		m_SlotCount = reference.m_SlotCount;
		m_MidAngle = reference.m_MidAngle;

		m_DrawFlippedToMatchAbsoluteAngle = reference.m_DrawFlippedToMatchAbsoluteAngle;

		return 0;
}

int PieSlice::ReadProperty(const std::string_view& propName, Reader& reader) {
		StartPropertyList(return Entity::ReadProperty(propName, reader));

		MatchProperty("Type", { m_Type = static_cast<PieSliceType>(std::stoi(reader.ReadPropValue())); });
		MatchProperty("Direction", {
			if (std::string directionString = reader.ReadPropValue(); c_DirectionNameToDirectionsMap.find(directionString) != c_DirectionNameToDirectionsMap.end()) {
				m_Direction = c_DirectionNameToDirectionsMap.find(directionString)->second;
			} else {
				try {
					int direction = std::stoi(directionString);
					if (direction < Directions::None || direction > Directions::Any) {
						reader.ReportError("Direction " + directionString + " is invalid (Out of Bounds).");
					}
					m_Direction = static_cast<Directions>(direction);
				} catch (const std::invalid_argument&) {
					reader.ReportError("Direction " + directionString + " is invalid.");
				}
			}
			if (m_Direction == Directions::None) {
				reader.ReportError("Pie Slices cannot have direction None.");
			}
		});
		MatchProperty("CanBeMiddleSlice", { reader >> m_CanBeMiddleSlice; });
		MatchProperty("Enabled", { reader >> m_Enabled; });
		MatchProperty("Icon", { SetIcon(dynamic_cast<Icon*>(g_PresetMan.ReadReflectedPreset(reader))); });
		MatchProperty("ScriptPath", {
			std::string scriptPath;
			reader >> scriptPath;
			m_LuabindFunctionObject = std::make_unique<SolObjectWrapper>(nullptr, scriptPath);
			if (!m_FunctionName.empty()) {
				ReloadScripts();
			}
		});
		MatchProperty("FunctionName", {
			reader >> m_FunctionName;
			if (m_LuabindFunctionObject) {
				ReloadScripts();
			}
		});
		MatchProperty("SubPieMenu", { SetSubPieMenu(dynamic_cast<PieMenu*>(g_PresetMan.ReadReflectedPreset(reader))); });
		MatchProperty("DrawFlippedToMatchAbsoluteAngle", { reader >> m_DrawFlippedToMatchAbsoluteAngle; });

		EndPropertyList;
}

int PieSlice::Save(Writer& writer) const {
		Entity::Save(writer);

		if (m_Type != PieSliceType::NoType) {
			writer.NewPropertyWithValue("Type", static_cast<int>(m_Type));
		}
		if (m_Direction != Directions::Any) {
			writer.NewPropertyWithValue("Direction", static_cast<int>(m_Direction));
		}
		if (!m_Enabled) {
			writer.NewPropertyWithValue("Enabled", m_Enabled);
		}
		writer.NewPropertyWithValue("Icon", m_Icon.get());
		if (m_LuabindFunctionObject && !m_FunctionName.empty()) {
			writer.NewPropertyWithValue("ScriptPath", m_LuabindFunctionObject->GetFilePath());
			writer.NewPropertyWithValue("FunctionName", m_FunctionName);
		}
		if (m_SubPieMenu) {
			writer.NewPropertyWithValue("SubPieMenu", m_SubPieMenu.get());
		}

		return 0;
}

BITMAP* PieSlice::GetAppropriateIcon(bool sliceIsSelected) const {
		if (int iconFrameCount = m_Icon->GetFrameCount(); iconFrameCount > 0) {
			if (!IsEnabled() && iconFrameCount > 2) {
				return m_Icon->GetBitmaps8()[2];
			} else if (sliceIsSelected && iconFrameCount > 1) {
				return m_Icon->GetBitmaps8()[1];
			} else {
				return m_Icon->GetBitmaps8()[0];
			}
		}
		return nullptr;
}

PieMenu* PieSlice::GetSubPieMenu() const {
		return m_SubPieMenu.get();
}

void PieSlice::SetSubPieMenu(PieMenu* newSubPieMenu) {
		m_SubPieMenu = std::unique_ptr<PieMenu, PieMenuCustomDeleter>(newSubPieMenu);
}

int PieSlice::ReloadScripts() {
		int status = 0;

		if (m_LuabindFunctionObject) {
			std::string filePath = m_LuabindFunctionObject->GetFilePath();
			std::unordered_map<std::string, SolObjectWrapper*> scriptFileFunctions;

			status = g_LuaMan.GetMasterScriptState().RunScriptFileAndRetrieveFunctions(filePath, {m_FunctionName}, scriptFileFunctions, true);
			if (scriptFileFunctions.find(m_FunctionName) != scriptFileFunctions.end()) {
				m_LuabindFunctionObject = std::unique_ptr<SolObjectWrapper>(scriptFileFunctions.at(m_FunctionName));
			}
		}

		return status;
}

void PieSlice::RecalculateMidAngle() {
		m_MidAngle = m_StartAngle + (static_cast<float>(m_SlotCount) * PieQuadrant::c_PieSliceSlotSize / 2.0F);
}

void PieSlice::PieMenuCustomDeleter::operator()(PieMenu* pieMenu) const {
		pieMenu->Destroy(true);
}
