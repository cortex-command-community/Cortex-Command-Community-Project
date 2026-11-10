// Make sure that binding definition files are always set to NOT use pre-compiled headers and conformance mode (/permissive) otherwise everything will be on fire!

#include "LuaBindingRegisterDefinitions.h"

using namespace RTE;

LuaBindingRegisterFunctionDefinitionForType(MiscLuaBindings, AlarmEvent) {
	registry.Class<AlarmEvent>("AlarmEvent")

	    .addConstructor<>()
	    .addConstructor<const Vector&, int, float>()

	    .addReadWriteMember("ScenePos", &AlarmEvent::m_ScenePos)
	    .addReadWriteMember("Team", &AlarmEvent::m_Team)
	    .addReadWriteMember("Range", &AlarmEvent::m_Range);
}

LuaBindingRegisterFunctionDefinitionForType(MiscLuaBindings, Directions) {
	registry.Class<directions>("Directions")

	    .addEnum({{"None", Directions::None},
	              {"Up", Directions::Up},
	              {"Down", Directions::Down},
	              {"Left", Directions::Left},
	              {"Right", Directions::Right},
	              {"Any", Directions::Any}});
}

LuaBindingRegisterFunctionDefinitionForType(MiscLuaBindings, DrawBlendMode) {
	registry.Class<blend_modes>("DrawBlendMode")

	    .addEnum({{"NoBlend", DrawBlendMode::NoBlend},
	              {"Burn", DrawBlendMode::BlendBurn},
	              {"Color", DrawBlendMode::BlendColor},
	              {"Difference", DrawBlendMode::BlendDifference},
	              {"Dissolve", DrawBlendMode::BlendDissolve},
	              {"Dodge", DrawBlendMode::BlendDodge},
	              {"Invert", DrawBlendMode::BlendInvert},
	              {"Luminance", DrawBlendMode::BlendLuminance},
	              {"Multiply", DrawBlendMode::BlendMultiply},
	              {"Saturation", DrawBlendMode::BlendSaturation},
	              {"Screen", DrawBlendMode::BlendScreen},
	              {"Transparency", DrawBlendMode::BlendTransparency},
	              {"BlendModeCount", DrawBlendMode::BlendModeCount}});
}

LuaBindingRegisterFunctionDefinitionForType(MiscLuaBindings, DrawDepth) {
	registry.Class<draw_depths>("DrawDepth")
	    .addEnum({{"Default", c_DefaultDrawDepth},
	              {"GUI", c_GuiDepth},
	              {"Primitive", c_PrimitiveDepth},
	              {"TerrainBackground", c_TerrainBGDepth},
	              {"Background", c_BackgroundDepth}});
}
