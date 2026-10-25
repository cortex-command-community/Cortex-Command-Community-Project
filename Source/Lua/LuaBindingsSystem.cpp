// Make sure that binding definition files are always set to NOT use pre-compiled headers and conformance mode (/permissive) otherwise everything will be on fire!

#include "LuaBindingRegisterDefinitions.h"

using namespace RTE;

LuaBindingRegisterFunctionDefinitionForType(SystemLuaBindings, Box) {
	registry.Class<Box>("Box")

	    .addConstructor<>()
	    .addConstructor<const Vector&, const Vector&>()
	    .addConstructor<float, float, float, float>()
	    .addConstructor<const Vector&, float, float>()
	    .addConstructor<const Box&>()
	    .addEqualityOperator<const Box&>()

	    .addProperty("ClassName", &Box::GetClassName)
	    .addProperty("Corner", &Box::GetCorner, &Box::SetCorner)
	    .addProperty("Width", &Box::GetWidth, &Box::SetWidth)
	    .addProperty("Height", &Box::GetHeight, &Box::SetHeight)
	    .addProperty("Center", &Box::GetCenter, &Box::SetCenter)
	    .addProperty("Area", &Box::GetArea)

	    .addFunction("GetRandomPoint", &Box::GetRandomPoint)
	    .addFunction("Unflip", &Box::Unflip)
	    .addFunction("IsWithinBox", &Box::IsWithinBox)
	    .addFunction("IsWithinBoxX", &Box::IsWithinBoxX)
	    .addFunction("IsWithinBoxY", &Box::IsWithinBoxY)
	    .addFunction("GetWithinBoxX", &Box::GetWithinBoxX)
	    .addFunction("GetWithinBoxY", &Box::GetWithinBoxY)
	    .addFunction("GetWithinBox", &Box::GetWithinBox)
	    .addFunction("IntersectsBox", &Box::IntersectsBox);
}

LuaBindingRegisterFunctionDefinitionForType(SystemLuaBindings, Controller) {
	registry.Class<Controller>("Controller")

	    .addConstructor<>()

	    .addProperty("InputMode", &Controller::GetInputMode, &Controller::SetInputMode)
	    .addProperty("ControlledActor", &Controller::GetControlledActor, &Controller::SetControlledActor)
	    .addProperty("Team", &Controller::GetTeam, &Controller::SetTeam)
	    .addProperty("AnalogMove", &Controller::GetAnalogMove, &Controller::SetAnalogMove)
	    .addProperty("AnalogAim", &Controller::GetAnalogAim, &Controller::SetAnalogAim)
	    .addProperty("AnalogCursor", &Controller::GetAnalogCursor, &Controller::SetAnalogCursor)
	    .addProperty("Player", &Controller::GetPlayer, &Controller::SetPlayer)
	    .addProperty("MouseMovement", &Controller::GetMouseMovement)
	    .addProperty("Disabled", &Controller::IsDisabled, &Controller::SetDisabled)

	    .addFunction("IsPlayerControlled", &Controller::IsPlayerControlled)
	    .addFunction("RelativeCursorMovement", &Controller::RelativeCursorMovement)
	    .addFunction("IsMouseControlled", &Controller::IsMouseControlled)
	    .addFunction("IsKeyboardOnlyControlled", &Controller::IsKeyboardOnlyControlled)
	    .addFunction("IsGamepadControlled", &Controller::IsGamepadControlled)
	    .addFunction("SetState", &Controller::SetState)
	    .addFunction("IsState", &Controller::IsState)
	    .addFunction("Override", &Controller::Override)

	    .addEnum({{"PRIMARY_ACTION", ControlState::PRIMARY_ACTION},
	              {"SECONDARY_ACTION", ControlState::SECONDARY_ACTION},
	              {"MOVE_IDLE", ControlState::MOVE_IDLE},
	              {"MOVE_RIGHT", ControlState::MOVE_RIGHT},
	              {"MOVE_LEFT", ControlState::MOVE_LEFT},
	              {"MOVE_UP", ControlState::MOVE_UP},
	              {"MOVE_DOWN", ControlState::MOVE_DOWN},
	              {"MOVE_FAST", ControlState::MOVE_FAST},
	              {"BODY_JUMPSTART", ControlState::BODY_JUMPSTART},
	              {"BODY_JUMP", ControlState::BODY_JUMP},
	              {"BODY_CROUCH", ControlState::BODY_PRONE}, // awful, but script compat
	              {"BODY_PRONE", ControlState::BODY_PRONE},
	              {"BODY_WALKCROUCH", ControlState::BODY_CROUCH},
	              {"AIM_UP", ControlState::AIM_UP},
	              {"AIM_DOWN", ControlState::AIM_DOWN},
	              {"AIM_SHARP", ControlState::AIM_SHARP},
	              {"WEAPON_FIRE", ControlState::WEAPON_FIRE},
	              {"WEAPON_RELOAD", ControlState::WEAPON_RELOAD},
	              {"WEAPON_RELOADHELD", ControlState::WEAPON_RELOADHELD},
	              {"PIE_MENU_OPENED", ControlState::PIE_MENU_OPENED},
	              {"PIE_MENU_ACTIVE", ControlState::PIE_MENU_ACTIVE},
	              {"WEAPON_CHANGE_NEXT", ControlState::WEAPON_CHANGE_NEXT},
	              {"WEAPON_CHANGE_PREV", ControlState::WEAPON_CHANGE_PREV},
	              {"WEAPON_PICKUP", ControlState::WEAPON_PICKUP},
	              {"WEAPON_DROP", ControlState::WEAPON_DROP},
	              {"WEAPON_PRIMARY_HOTKEY", ControlState::WEAPON_PRIMARY_HOTKEY},
	              {"WEAPON_AUXILIARY_HOTKEY", ControlState::WEAPON_AUXILIARY_HOTKEY},
	              {"ACTOR_PRIMARY_HOTKEY", ControlState::ACTOR_PRIMARY_HOTKEY},
	              {"ACTOR_AUXILIARY_HOTKEY", ControlState::ACTOR_AUXILIARY_HOTKEY},
	              {"WEAPON_PRIMARY_HOTKEYSTART", ControlState::WEAPON_PRIMARY_HOTKEYSTART},
	              {"WEAPON_AUXILIARY_HOTKEYSTART", ControlState::WEAPON_AUXILIARY_HOTKEYSTART},
	              {"ACTOR_PRIMARY_HOTKEYSTART", ControlState::ACTOR_PRIMARY_HOTKEYSTART},
	              {"ACTOR_AUXILIARY_HOTKEYSTART", ControlState::ACTOR_AUXILIARY_HOTKEYSTART},
	              {"ACTOR_NEXT", ControlState::ACTOR_NEXT},
	              {"ACTOR_PREV", ControlState::ACTOR_PREV},
	              {"ACTOR_BRAIN", ControlState::ACTOR_BRAIN},
	              {"ACTOR_NEXT_PREP", ControlState::ACTOR_NEXT_PREP},
	              {"ACTOR_PREV_PREP", ControlState::ACTOR_PREV_PREP},
	              {"HOLD_RIGHT", ControlState::HOLD_RIGHT},
	              {"HOLD_LEFT", ControlState::HOLD_LEFT},
	              {"HOLD_UP", ControlState::HOLD_UP},
	              {"HOLD_DOWN", ControlState::HOLD_DOWN},
	              {"PRESS_PRIMARY", ControlState::PRESS_PRIMARY},
	              {"PRESS_SECONDARY", ControlState::PRESS_SECONDARY},
	              {"PRESS_RIGHT", ControlState::PRESS_RIGHT},
	              {"PRESS_LEFT", ControlState::PRESS_LEFT},
	              {"PRESS_UP", ControlState::PRESS_UP},
	              {"PRESS_DOWN", ControlState::PRESS_DOWN},
	              {"RELEASE_PRIMARY", ControlState::RELEASE_PRIMARY},
	              {"RELEASE_SECONDARY", ControlState::RELEASE_SECONDARY},
	              {"PRESS_FACEBUTTON", ControlState::PRESS_FACEBUTTON},
	              {"RELEASE_FACEBUTTON", ControlState::RELEASE_FACEBUTTON},
	              {"SCROLL_UP", ControlState::SCROLL_UP},
	              {"SCROLL_DOWN", ControlState::SCROLL_DOWN},
	              {"DEBUG_ONE", ControlState::DEBUG_ONE},
	              {"CONTROLSTATECOUNT", ControlState::CONTROLSTATECOUNT}})
	    .addEnum({{"CIM_DISABLED", Controller::InputMode::CIM_DISABLED},
	              {"CIM_PLAYER", Controller::InputMode::CIM_PLAYER},
	              {"CIM_AI", Controller::InputMode::CIM_AI},
	              {"CIM_NETWORK", Controller::InputMode::CIM_NETWORK},
	              {"CIM_INPUTMODECOUNT", Controller::InputMode::CIM_INPUTMODECOUNT}});
}

LuaBindingRegisterFunctionDefinitionForType(SystemLuaBindings, DataModule) {
	registry.Class<DataModule>("DataModule")

	    .addProperty("FileName", &DataModule::GetFileName)
	    .addProperty("FriendlyName", &DataModule::GetFriendlyName)
	    .addProperty("Author", &DataModule::GetAuthor)
	    .addProperty("Description", &DataModule::GetDescription)
	    .addProperty("Version", &DataModule::GetVersionNumber)
	    .addProperty("IsFaction", &DataModule::IsFaction)
	    .addProperty("IsMerchant", &DataModule::IsMerchant)

	    .addProperty("Presets", &DataModule::m_EntityList, LuaBinding::ReturnStlIterator);
}

LuaBindingRegisterFunctionDefinitionForType(SystemLuaBindings, Timer) {
	registry.Class<Timer>("Timer")

	    .addConstructor<>()
	    .addConstructor<double>()
	    .addConstructor<double, double>()

	    .addProperty("StartRealTimeMS", &Timer::GetStartRealTimeMS, &Timer::SetStartRealTimeMS)
	    .addProperty("ElapsedRealTimeS", &Timer::GetElapsedRealTimeS, &Timer::SetElapsedRealTimeS)
	    .addProperty("ElapsedRealTimeMS", &Timer::GetElapsedRealTimeMS, &Timer::SetElapsedRealTimeMS)
	    .addProperty("StartSimTimeMS", &Timer::GetStartSimTimeMS, &Timer::SetStartSimTimeMS)
	    .addProperty("ElapsedSimTimeS", &Timer::GetElapsedSimTimeS, &Timer::SetElapsedSimTimeS)
	    .addProperty("ElapsedSimTimeMS", &Timer::GetElapsedSimTimeMS, &Timer::SetElapsedSimTimeMS)
	    .addProperty("RealTimeLimitProgress", &Timer::GetRealTimeLimitProgress, &Timer::SetRealTimeLimitProgress)
	    .addProperty("SimTimeLimitProgress", &Timer::GetSimTimeLimitProgress, &Timer::SetSimTimeLimitProgress)

	    .addFunction("Reset", &Timer::Reset)
	    .addFunction("SetRealTimeLimitMS", &Timer::SetRealTimeLimitMS)
	    .addFunction("SetRealTimeLimitS", &Timer::SetRealTimeLimitS)
	    .addFunction("IsPastRealTimeLimit", &Timer::IsPastRealTimeLimit)
	    .addFunction("LeftTillRealTimeLimitMS", &Timer::LeftTillRealTimeLimitMS)
	    .addFunction("LeftTillRealTimeLimitS", &Timer::LeftTillRealTimeLimitS)
	    .addFunction("LeftTillRealMS", &Timer::LeftTillRealMS)
	    .addFunction("IsPastRealMS", &Timer::IsPastRealMS)
	    .addFunction("AlternateReal", &Timer::AlternateReal)
	    .addFunction("GetSimTimeLimitMS", &Timer::GetSimTimeLimitMS)
	    .addFunction("SetSimTimeLimitMS", &Timer::SetSimTimeLimitMS)
	    .addFunction("GetSimTimeLimitS", &Timer::GetSimTimeLimitS)
	    .addFunction("SetSimTimeLimitS", &Timer::SetSimTimeLimitS)
	    .addFunction("IsPastSimTimeLimit", &Timer::IsPastSimTimeLimit)
	    .addFunction("LeftTillSimTimeLimitMS", &Timer::LeftTillSimTimeLimitMS)
	    .addFunction("LeftTillSimTimeLimitS", &Timer::LeftTillSimTimeLimitS)
	    .addFunction("LeftTillSimMS", &Timer::LeftTillSimMS)
	    .addFunction("IsPastSimMS", &Timer::IsPastSimMS)
	    .addFunction("AlternateSim", &Timer::AlternateSim);
}

LuaBindingRegisterFunctionDefinitionForType(SystemLuaBindings, Vector) {
	registry.Class<Vector>("Vector")

	    .addConstructor<>()
	    .addConstructor<float, float>()
	    .addEqualityOperator<const Vector&>()
	    .addFunction("__add", +[](const Vector& self, const Vector& other) { return self + other; })
	    .addFunction("__sub", +[](const Vector& self, const Vector& other) { return self - other; })
	    .addFunction("__mul", +[](const Vector& self, float scalar) { return self * scalar; })
	    .addFunction("__div", +[](const Vector& self, float scalar) { return self / scalar; })
	    .addToString()

	    .addProperty("ClassName", &Vector::GetClassName)
	    .addProperty("RoundedX", &Vector::GetRoundIntX)
	    .addProperty("RoundedY", &Vector::GetRoundIntY)
	    .addProperty("Rounded", &Vector::GetRounded)
	    .addProperty("FlooredX", &Vector::GetFloorIntX)
	    .addProperty("FlooredY", &Vector::GetFloorIntY)
	    .addProperty("Floored", &Vector::GetFloored)
	    .addProperty("CeilingedX", &Vector::GetCeilingIntX)
	    .addProperty("CeilingedY", &Vector::GetCeilingIntY)
	    .addProperty("Ceilinged", &Vector::GetCeilinged)
	    .addProperty("Magnitude", &Vector::GetMagnitude)
	    .addProperty("SqrMagnitude", &Vector::GetSqrMagnitude)
	    .addProperty("Largest", &Vector::GetLargest)
	    .addProperty("Smallest", &Vector::GetSmallest)
	    .addProperty("Normalized", &Vector::GetNormalized)
	    .addProperty("Perpendicular", &Vector::GetPerpendicular)
	    .addProperty("AbsRadAngle", &Vector::GetAbsRadAngle, &Vector::SetAbsRadAngle)
	    .addProperty("AbsDegAngle", &Vector::GetAbsDegAngle, &Vector::SetAbsDegAngle)

	    .addReadWriteMember("X", &Vector::m_X)
	    .addReadWriteMember("Y", &Vector::m_Y)

	    .addFunction("MagnitudeIsGreaterThan", &Vector::MagnitudeIsGreaterThan)
	    .addFunction("MagnitudeIsLessThan", &Vector::MagnitudeIsLessThan)
	    .addFunction("SetMagnitude", &Vector::SetMagnitude, LuaBinding::ReturnSelf)
	    .addFunction("GetXFlipped", &Vector::GetXFlipped)
	    .addFunction("GetYFlipped", &Vector::GetYFlipped)
	    .addFunction("CapMagnitude", &Vector::CapMagnitude, LuaBinding::ReturnSelf)
	    .addFunction("ClampMagnitude", &Vector::ClampMagnitude, LuaBinding::ReturnSelf)
	    .addFunction("FlipX", &Vector::FlipX, LuaBinding::ReturnSelf)
	    .addFunction("FlipY", &Vector::FlipY, LuaBinding::ReturnSelf)
	    .addFunction("IsZero", &Vector::IsZero)
	    .addFunction("IsOpposedTo", &Vector::IsOpposedTo)
	    .addFunction("Dot", &Vector::Dot)
	    .addFunction("Cross", &Vector::Cross)
	    .addFunction("Round", &Vector::Round, LuaBinding::ReturnSelf)
	    .addFunction("ToHalf", &Vector::ToHalf, LuaBinding::ReturnSelf)
	    .addFunction("Floor", &Vector::Floor, LuaBinding::ReturnSelf)
	    .addFunction("Ceiling", &Vector::Ceiling, LuaBinding::ReturnSelf)
	    .addFunction("Normalize", &Vector::Normalize, LuaBinding::ReturnSelf)
	    .addFunction("Perpendicularize", &Vector::Perpendicularize, LuaBinding::ReturnSelf)
	    .addFunction("Reset", &Vector::Reset)
	    .addFunction("RadRotate", &Vector::RadRotate, LuaBinding::ReturnSelf)
	    .addFunction("DegRotate", &Vector::DegRotate, LuaBinding::ReturnSelf)
	    .addFunction("GetRadRotatedCopy", &Vector::GetRadRotatedCopy)
	    .addFunction("GetDegRotatedCopy", &Vector::GetDegRotatedCopy)
	    .addFunction("AbsRotateTo", &Vector::AbsRotateTo, LuaBinding::ReturnSelf)
	    .addFunction("SetXY", &Vector::SetXY, LuaBinding::ReturnSelf);
}

LuaBindingRegisterFunctionDefinitionForType(SystemLuaBindings, PathRequest) {
	using namespace micropather;
	registry.Class<PathRequest>("PathRequest")

	    .addProperty("Path", &PathRequest::path, LuaBinding::ReturnStlIterator)
	    .addProperty("PathLength", &PathRequest::pathLength)
	    .addProperty("Status", &PathRequest::status)
	    .addProperty("TotalCost", &PathRequest::totalCost)

	    .addEnum({{"Solved", micropather::MicroPather::SOLVED},
	              {"NoSolution", micropather::MicroPather::NO_SOLUTION},
	              {"StartEndSame", micropather::MicroPather::START_END_SAME}});
}
