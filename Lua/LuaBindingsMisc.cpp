// Make sure that binding definition files are always set to NOT use pre-compiled headers and conformance mode (/permissive) otherwise everything will be on fire!

#include "LuaBindingRegisterDefinitions.h"

#include "MovableMan.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(MiscLuaBindings, AlarmEvent) {
		auto luaType = SimpleTypeLuaClassDefinition(AlarmEvent);

		luaType.set(sol::meta_function::construct, sol::constructors<
			AlarmEvent(), 
			AlarmEvent(const Vector&, int, float)
		>());

		luaType["ScenePos"] = &AlarmEvent::m_ScenePos;
		luaType["Team"] = &AlarmEvent::m_Team;
		luaType["Range"] = &AlarmEvent::m_Range;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(MiscLuaBindings, Directions) {
		EnumTypeLuaClassDefinition("Directions", EnumList(Directions) {
			{ "None", Directions::None },
			{ "Up", Directions::Up },
			{ "Down", Directions::Down },
			{ "Left", Directions::Left },
			{ "Right", Directions::Right },
			{ "Any", Directions::Any }
		});
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(MiscLuaBindings, DrawBlendMode) {
		EnumTypeLuaClassDefinition("DrawBlendMode", EnumList(DrawBlendMode) {
			{ "NoBlend", DrawBlendMode::NoBlend },
			{ "Burn", DrawBlendMode::BlendBurn },
			{ "Color", DrawBlendMode::BlendColor },
			{ "Difference", DrawBlendMode::BlendDifference },
			{ "Dissolve", DrawBlendMode::BlendDissolve },
			{ "Dodge", DrawBlendMode::BlendDodge },
			{ "Invert", DrawBlendMode::BlendInvert },
			{ "Luminance", DrawBlendMode::BlendLuminance },
			{ "Multiply", DrawBlendMode::BlendMultiply },
			{ "Saturation", DrawBlendMode::BlendSaturation },
			{ "Screen", DrawBlendMode::BlendScreen },
			{ "Transparency", DrawBlendMode::BlendTransparency },
			{ "BlendModeCount", DrawBlendMode::BlendModeCount }
		});
	}
}