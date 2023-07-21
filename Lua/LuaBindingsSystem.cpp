// Make sure that binding definition files are always set to NOT use pre-compiled headers and conformance mode (/permissive) otherwise everything will be on fire!

#include "LuaBindingRegisterDefinitions.h"

#include "Actor.h"
#include "Box.h"
#include "Controller.h"
#include "DataModule.h"
#include "Timer.h"
#include "Vector.h"
#include "PathFinder.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(SystemLuaBindings, Box) {
		auto luaType = SimpleTypeLuaClassDefinition(Box);

		luaType.set(sol::call_constructor, sol::constructors<
			Box(),
			Box(const Vector&, const Vector&),
			Box(float, float, float, float&),
			Box(const Vector&, float, float),
			Box(const Box&)
		>());

		luaType[sol::meta_function::equal_to] = [](const Box& lhs, const Box& rhs) { return lhs == rhs; };

		luaType["ClassName"] = sol::property(&Box::GetClassName);
		luaType["Corner"] = sol::property(&Box::GetCorner, &Box::SetCorner);
		luaType["Width"] = sol::property(&Box::GetWidth, &Box::SetWidth);
		luaType["Height"] = sol::property(&Box::GetHeight, &Box::SetHeight);
		luaType["Center"] = sol::property(&Box::GetCenter, &Box::SetCenter);
		luaType["Area"] = sol::property(&Box::GetArea);

		luaType["GetRandomPoint"] = &Box::GetRandomPoint;
		luaType["Unflip"] = &Box::Unflip;
		luaType["IsWithinBox"] = &Box::IsWithinBox;
		luaType["IsWithinBoxX"] = &Box::IsWithinBoxX;
		luaType["IsWithinBoxY"] = &Box::IsWithinBoxY;
		luaType["GetWithinBoxX"] = &Box::GetWithinBoxX;
		luaType["GetWithinBoxY"] = &Box::GetWithinBoxY;
		luaType["GetWithinBox"] = &Box::GetWithinBox;
		luaType["IntersectsBox"] = &Box::IntersectsBox;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(SystemLuaBindings, Controller) {
		auto luaType = SimpleTypeLuaClassDefinition(Controller);

		luaType.set(sol::call_constructor, sol::constructors<
			Controller()
		>());

		luaType["InputMode"] = sol::property(&Controller::GetInputMode, &Controller::SetInputMode);
		luaType["ControlledActor"] = sol::property(&Controller::GetControlledActor, &Controller::SetControlledActor);
		luaType["Team"] = sol::property(&Controller::GetTeam, &Controller::SetTeam);
		luaType["AnalogMove"] = sol::property(&Controller::GetAnalogMove, &Controller::SetAnalogMove);
		luaType["AnalogAim"] = sol::property(&Controller::GetAnalogAim, &Controller::SetAnalogAim);
		luaType["AnalogCursor"] = sol::property(&Controller::GetAnalogCursor, &Controller::SetAnalogCursor);
		luaType["Player"] = sol::property(&Controller::GetPlayer, &Controller::SetPlayer);
		luaType["MouseMovement"] = sol::property(&Controller::GetMouseMovement);
		luaType["Disabled"] = sol::property(&Controller::IsDisabled, &Controller::SetDisabled);

		luaType["IsPlayerControlled"] = &Controller::IsPlayerControlled;
		luaType["RelativeCursorMovement"] = &Controller::RelativeCursorMovement;
		luaType["IsMouseControlled"] = &Controller::IsMouseControlled;
		luaType["IsKeyboardOnlyControlled"] = &Controller::IsKeyboardOnlyControlled;
		luaType["IsGamepadControlled"] = &Controller::IsGamepadControlled;
		luaType["SetState"] = &Controller::SetState;
		luaType["IsState"] = &Controller::IsState;

		{
			sol::table enumTable = LegacyEnumTypeTable("ControlState");
			enumTable["PRIMARY_ACTION"] = ControlState::PRIMARY_ACTION;
			enumTable["SECONDARY_ACTION"] = ControlState::SECONDARY_ACTION;
			enumTable["MOVE_IDLE"] = ControlState::MOVE_IDLE;
			enumTable["MOVE_RIGHT"] = ControlState::MOVE_RIGHT;
			enumTable["MOVE_LEFT"] = ControlState::MOVE_LEFT;
			enumTable["MOVE_UP"] = ControlState::MOVE_UP;
			enumTable["MOVE_DOWN"] = ControlState::MOVE_DOWN;
			enumTable["MOVE_FAST"] = ControlState::MOVE_FAST;
			enumTable["BODY_JUMPSTART"] = ControlState::BODY_JUMPSTART;
			enumTable["BODY_JUMP"] = ControlState::BODY_JUMP;
			enumTable["BODY_CROUCH"] = ControlState::BODY_CROUCH;
			enumTable["AIM_UP"] = ControlState::AIM_UP;
			enumTable["AIM_DOWN"] = ControlState::AIM_DOWN;
			enumTable["AIM_SHARP"] = ControlState::AIM_SHARP;
			enumTable["WEAPON_FIRE"] = ControlState::WEAPON_FIRE;
			enumTable["WEAPON_RELOAD"] = ControlState::WEAPON_RELOAD;
			enumTable["PIE_MENU_OPENED"] = ControlState::PIE_MENU_OPENED;
			enumTable["PIE_MENU_ACTIVE"] = ControlState::PIE_MENU_ACTIVE;
			enumTable["WEAPON_CHANGE_NEXT"] = ControlState::WEAPON_CHANGE_NEXT;
			enumTable["WEAPON_CHANGE_PREV"] = ControlState::WEAPON_CHANGE_PREV;
			enumTable["WEAPON_PICKUP"] = ControlState::WEAPON_PICKUP;
			enumTable["WEAPON_DROP"] = ControlState::WEAPON_DROP;
			enumTable["ACTOR_NEXT"] = ControlState::ACTOR_NEXT;
			enumTable["ACTOR_PREV"] = ControlState::ACTOR_PREV;
			enumTable["ACTOR_BRAIN"] = ControlState::ACTOR_BRAIN;
			enumTable["ACTOR_NEXT_PREP"] = ControlState::ACTOR_NEXT_PREP;
			enumTable["ACTOR_PREV_PREP"] = ControlState::ACTOR_PREV_PREP;
			enumTable["HOLD_RIGHT"] = ControlState::HOLD_RIGHT;
			enumTable["HOLD_LEFT"] = ControlState::HOLD_LEFT;
			enumTable["HOLD_UP"] = ControlState::HOLD_UP;
			enumTable["HOLD_DOWN"] = ControlState::HOLD_DOWN;
			enumTable["PRESS_PRIMARY"] = ControlState::PRESS_PRIMARY;
			enumTable["PRESS_SECONDARY"] = ControlState::PRESS_SECONDARY;
			enumTable["PRESS_RIGHT"] = ControlState::PRESS_RIGHT;
			enumTable["PRESS_LEFT"] = ControlState::PRESS_LEFT;
			enumTable["PRESS_UP"] = ControlState::PRESS_UP;
			enumTable["PRESS_DOWN"] = ControlState::PRESS_DOWN;
			enumTable["RELEASE_PRIMARY"] = ControlState::RELEASE_PRIMARY;
			enumTable["RELEASE_SECONDARY"] = ControlState::RELEASE_SECONDARY;
			enumTable["PRESS_FACEBUTTON"] = ControlState::PRESS_FACEBUTTON;
			enumTable["RELEASE_FACEBUTTON"] = ControlState::RELEASE_FACEBUTTON;
			enumTable["SCROLL_UP"] = ControlState::SCROLL_UP;
			enumTable["SCROLL_DOWN"] = ControlState::SCROLL_DOWN;
			enumTable["DEBUG_ONE"] = ControlState::DEBUG_ONE;
			enumTable["CONTROLSTATECOUNT"] = ControlState::CONTROLSTATECOUNT;
		}
		{
			sol::table enumTable = LegacyEnumTypeTable("InputMode");
			enumTable["CIM_DISABLED"] = Controller::InputMode::CIM_DISABLED;
			enumTable["CIM_PLAYER"] = Controller::InputMode::CIM_PLAYER;
			enumTable["CIM_AI"] = Controller::InputMode::CIM_AI;
			enumTable["CIM_NETWORK"] = Controller::InputMode::CIM_NETWORK;
			enumTable["CIM_INPUTMODECOUNT"] = Controller::InputMode::CIM_INPUTMODECOUNT;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(SystemLuaBindings, DataModule) {
		auto luaType = SimpleTypeLuaClassDefinition(DataModule);

		luaType["FileName"] = sol::property(&DataModule::GetFileName);
		luaType["FriendlyName"] = sol::property(&DataModule::GetFriendlyName);
		luaType["Author"] = sol::property(&DataModule::GetAuthor);
		luaType["Description"] = sol::property(&DataModule::GetDescription);
		luaType["Version"] = sol::property(&DataModule::GetVersionNumber);
		luaType["IsFaction"] = sol::property(&DataModule::IsFaction);
		luaType["IsMerchant"] = sol::property(&DataModule::IsMerchant);

		luaType["Presets"] = &DataModule::m_EntityList;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(SystemLuaBindings, Timer) {
		auto luaType = SimpleTypeLuaClassDefinition(Timer);

		luaType.set(sol::call_constructor, sol::constructors<
			Timer(),
			Timer(double),
			Timer(double, double)
		>());

		luaType["StartRealTimeMS"] = sol::property(&Timer::GetStartRealTimeMS, &Timer::SetStartRealTimeMS);
		luaType["ElapsedRealTimeS"] = sol::property(&Timer::GetElapsedRealTimeS, &Timer::SetElapsedRealTimeS);
		luaType["ElapsedRealTimeMS"] = sol::property(&Timer::GetElapsedRealTimeMS, &Timer::SetElapsedRealTimeMS);
		luaType["StartSimTimeMS"] = sol::property(&Timer::GetStartSimTimeMS, &Timer::SetStartSimTimeMS);
		luaType["ElapsedSimTimeS"] = sol::property(&Timer::GetElapsedSimTimeS, &Timer::SetElapsedSimTimeS);
		luaType["ElapsedSimTimeMS"] = sol::property(&Timer::GetElapsedSimTimeMS, &Timer::SetElapsedSimTimeMS);
		luaType["RealTimeLimitProgress"] = sol::property(&Timer::RealTimeLimitProgress);
		luaType["SimTimeLimitProgress"] = sol::property(&Timer::SimTimeLimitProgress);

		luaType["Reset"] = &Timer::Reset;
		luaType["SetRealTimeLimitMS"] = &Timer::SetRealTimeLimitMS;
		luaType["SetRealTimeLimitS"] = &Timer::SetRealTimeLimitS;
		luaType["IsPastRealTimeLimit"] = &Timer::IsPastRealTimeLimit;
		luaType["LeftTillRealTimeLimitMS"] = &Timer::LeftTillRealTimeLimitMS;
		luaType["LeftTillRealTimeLimitS"] = &Timer::LeftTillRealTimeLimitS;
		luaType["LeftTillRealMS"] = &Timer::LeftTillRealMS;
		luaType["IsPastRealMS"] = &Timer::IsPastRealMS;
		luaType["AlternateReal"] = &Timer::AlternateReal;
		luaType["GetSimTimeLimitMS"] = &Timer::GetSimTimeLimitMS;
		luaType["SetSimTimeLimitMS"] = &Timer::SetSimTimeLimitMS;
		luaType["GetSimTimeLimitS"] = &Timer::GetSimTimeLimitS;
		luaType["SetSimTimeLimitS"] = &Timer::SetSimTimeLimitS;
		luaType["IsPastSimTimeLimit"] = &Timer::IsPastSimTimeLimit;
		luaType["LeftTillSimTimeLimitMS"] = &Timer::LeftTillSimTimeLimitMS;
		luaType["LeftTillSimTimeLimitS"] = &Timer::LeftTillSimTimeLimitS;
		luaType["LeftTillSimMS"] = &Timer::LeftTillSimMS;
		luaType["IsPastSimMS"] = &Timer::IsPastSimMS;
		luaType["AlternateSim"] = &Timer::AlternateSim;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(SystemLuaBindings, Vector) {
		auto luaType = SimpleTypeLuaClassDefinition(Vector);

		luaType.set(sol::call_constructor, sol::constructors<
			Vector(),
			Vector(float, float)
		>());

		luaType[sol::meta_function::equal_to] = [](const Vector& lhs, const Vector& rhs) { return lhs == rhs; };
		luaType[sol::meta_function::addition] = [](const Vector& lhs, const Vector& rhs) { return lhs + rhs; };
		luaType[sol::meta_function::subtraction] = [](const Vector& lhs, const Vector& rhs) { return lhs - rhs; };
		luaType[sol::meta_function::multiplication] = [](const Vector& lhs, const Vector& rhs) { return lhs * rhs; };
		luaType[sol::meta_function::division] = [](const Vector& lhs, const Vector& rhs) { return lhs / rhs; };
		luaType[sol::meta_function::to_string] = [](const Vector& obj) { std::stringstream stream; stream << obj; return stream.str(); };

		luaType["ClassName"] = sol::property(&Vector::GetClassName);
		luaType["RoundedX"] = sol::property(&Vector::GetRoundIntX);
		luaType["RoundedY"] = sol::property(&Vector::GetRoundIntY);
		luaType["Rounded"] = sol::property(&Vector::GetRounded);
		luaType["FlooredX"] = sol::property(&Vector::GetFloorIntX);
		luaType["FlooredY"] = sol::property(&Vector::GetFloorIntY);
		luaType["Floored"] = sol::property(&Vector::GetFloored);
		luaType["CeilingedX"] = sol::property(&Vector::GetCeilingIntX);
		luaType["CeilingedY"] = sol::property(&Vector::GetCeilingIntY);
		luaType["Ceilinged"] = sol::property(&Vector::GetCeilinged);
		luaType["Magnitude"] = sol::property(&Vector::GetMagnitude);
		luaType["SqrMagnitude"] = sol::property(&Vector::GetSqrMagnitude);
		luaType["Largest"] = sol::property(&Vector::GetLargest);
		luaType["Smallest"] = sol::property(&Vector::GetSmallest);
		luaType["Normalized"] = sol::property(&Vector::GetNormalized);
		luaType["Perpendicular"] = sol::property(&Vector::GetPerpendicular);
		luaType["AbsRadAngle"] = sol::property(&Vector::GetAbsRadAngle, &Vector::SetAbsRadAngle);
		luaType["AbsDegAngle"] = sol::property(&Vector::GetAbsDegAngle, &Vector::SetAbsDegAngle);

		luaType["X"] = &Vector::m_X;
		luaType["Y"] = &Vector::m_Y;

		luaType["MagnitudeIsGreaterThan"] = &Vector::MagnitudeIsGreaterThan;
		luaType["MagnitudeIsLessThan"] = &Vector::MagnitudeIsLessThan;
		luaType["SetMagnitude"] = &Vector::SetMagnitude;
		luaType["GetXFlipped"] = &Vector::GetXFlipped;
		luaType["GetYFlipped"] = &Vector::GetYFlipped;
		luaType["CapMagnitude"] = &Vector::CapMagnitude;
		luaType["ClampMagnitude"] = &Vector::ClampMagnitude;
		luaType["FlipX"] = &Vector::FlipX;
		luaType["FlipY"] = &Vector::FlipY;
		luaType["IsZero"] = &Vector::IsZero;
		luaType["IsOpposedTo"] = &Vector::IsOpposedTo;
		luaType["Dot"] = &Vector::Dot;
		luaType["Cross"] = &Vector::Cross;
		luaType["Round"] = &Vector::Round;
		luaType["ToHalf"] = &Vector::ToHalf;
		luaType["Floor"] = &Vector::Floor;
		luaType["Ceiling"] = &Vector::Ceiling;
		luaType["Normalize"] = &Vector::Normalize;
		luaType["Perpendicularize"] = &Vector::Perpendicularize;
		luaType["Reset"] = &Vector::Reset;
		luaType["RadRotate"] = &Vector::RadRotate;
		luaType["DegRotate"] = &Vector::DegRotate;
		luaType["GetRadRotatedCopy"] = &Vector::GetRadRotatedCopy;
		luaType["GetDegRotatedCopy"] = &Vector::GetDegRotatedCopy;
		luaType["AbsRotateTo"] = &Vector::AbsRotateTo;
		luaType["SetXY"] = &Vector::SetXY;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(SystemLuaBindings, PathRequest) {
		auto luaType = SimpleTypeLuaClassDefinition(PathRequest);

		luaType["Path"] = sol::readonly(&PathRequest::path);
		luaType["PathLength"] = sol::readonly(&PathRequest::pathLength);
		luaType["Status"] = sol::readonly(&PathRequest::status);
		luaType["TotalCost"] = sol::readonly(&PathRequest::totalCost);

		luaType.new_enum<micropather::MicroPather::Status>("Status", {
			{ "Solved", micropather::MicroPather::Status::SOLVED },
			{ "NoSolution", micropather::MicroPather::Status::NO_SOLUTION },
			{ "StartEndSame", micropather::MicroPather::Status::START_END_SAME }
		});
	}
}