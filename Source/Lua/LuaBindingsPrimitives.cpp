// Make sure that binding definition files are always set to NOT use pre-compiled headers and conformance mode (/permissive) otherwise everything will be on fire!

#include "LuaBindingRegisterDefinitions.h"

using namespace RTE;

LuaBindingRegisterFunctionDefinitionForType(PrimitiveLuaBindings, GraphicalPrimitive) {
	registry.Class<GraphicalPrimitive>("GraphicalPrimitive");
}

LuaBindingRegisterFunctionDefinitionForType(PrimitiveLuaBindings, LinePrimitive) {
	registry.Class<LinePrimitive, GraphicalPrimitive>("LinePrimitive")

	    .addConstructor<int, const Vector&, const Vector&, unsigned char>()
	    .addConstructor<int, const Vector&, const Vector&, float, unsigned char>()
	    .addConstructor<int, const Vector&, const Vector&, float, unsigned char, float>();
}

LuaBindingRegisterFunctionDefinitionForType(PrimitiveLuaBindings, ArcPrimitive) {
	registry.Class<ArcPrimitive, GraphicalPrimitive>("ArcPrimitive")

	    .addConstructor<int, const Vector&, float, float, int, int, unsigned char>()
	    .addConstructor<int, const Vector&, float, float, int, int, unsigned char, float>();
}

LuaBindingRegisterFunctionDefinitionForType(PrimitiveLuaBindings, SplinePrimitive) {
	registry.Class<SplinePrimitive, GraphicalPrimitive>("SplinePrimitive")

	    .addConstructor<int, const Vector&, const Vector&, const Vector&, const Vector&, unsigned char>()
	    .addConstructor<int, const Vector&, const Vector&, const Vector&, const Vector&, unsigned char, float>();
}

LuaBindingRegisterFunctionDefinitionForType(PrimitiveLuaBindings, BoxPrimitive) {
	registry.Class<BoxPrimitive, GraphicalPrimitive>("BoxPrimitive")

	    .addConstructor<int, const Vector&, const Vector&, unsigned char>()
	    .addConstructor<int, const Vector&, const Vector&, unsigned char, float>();
}

LuaBindingRegisterFunctionDefinitionForType(PrimitiveLuaBindings, BoxFillPrimitive) {
	registry.Class<BoxFillPrimitive, GraphicalPrimitive>("BoxFillPrimitive")

	    .addConstructor<int, const Vector&, const Vector&, unsigned char>()
	    .addConstructor<int, const Vector&, const Vector&, unsigned char, float>();
}

LuaBindingRegisterFunctionDefinitionForType(PrimitiveLuaBindings, RoundedBoxPrimitive) {
	registry.Class<RoundedBoxPrimitive, GraphicalPrimitive>("RoundedBoxPrimitive")

	    .addConstructor<int, const Vector&, const Vector&, int, unsigned char>()
	    .addConstructor<int, const Vector&, const Vector&, int, unsigned char, float>();
}

LuaBindingRegisterFunctionDefinitionForType(PrimitiveLuaBindings, RoundedBoxFillPrimitive) {
	registry.Class<RoundedBoxFillPrimitive, GraphicalPrimitive>("RoundedBoxFillPrimitive")

	    .addConstructor<int, const Vector&, const Vector&, int, unsigned char>()
	    .addConstructor<int, const Vector&, const Vector&, int, unsigned char, float>();
}

LuaBindingRegisterFunctionDefinitionForType(PrimitiveLuaBindings, CirclePrimitive) {
	registry.Class<CirclePrimitive, GraphicalPrimitive>("CirclePrimitive")

	    .addConstructor<int, const Vector&, int, unsigned char>()
	    .addConstructor<int, const Vector&, int, unsigned char, float>();
}

LuaBindingRegisterFunctionDefinitionForType(PrimitiveLuaBindings, CircleFillPrimitive) {
	registry.Class<CircleFillPrimitive, GraphicalPrimitive>("CircleFillPrimitive")

	    .addConstructor<int, const Vector&, int, unsigned char>()
	    .addConstructor<int, const Vector&, int, unsigned char, float>();
}

LuaBindingRegisterFunctionDefinitionForType(PrimitiveLuaBindings, EllipsePrimitive) {
	registry.Class<EllipsePrimitive, GraphicalPrimitive>("EllipsePrimitive")

	    .addConstructor<int, const Vector&, int, int, unsigned char>()
	    .addConstructor<int, const Vector&, int, int, unsigned char, float>();
}

LuaBindingRegisterFunctionDefinitionForType(PrimitiveLuaBindings, EllipseFillPrimitive) {
	registry.Class<EllipseFillPrimitive, GraphicalPrimitive>("EllipseFillPrimitive")

	    .addConstructor<int, const Vector&, int, int, unsigned char>()
	    .addConstructor<int, const Vector&, int, int, unsigned char, float>();
}

LuaBindingRegisterFunctionDefinitionForType(PrimitiveLuaBindings, TrianglePrimitive) {
	registry.Class<TrianglePrimitive, GraphicalPrimitive>("TrianglePrimitive")

	    .addConstructor<int, const Vector&, const Vector&, const Vector&, unsigned char>()
	    .addConstructor<int, const Vector&, const Vector&, const Vector&, unsigned char, float>();
}

LuaBindingRegisterFunctionDefinitionForType(PrimitiveLuaBindings, TriangleFillPrimitive) {
	registry.Class<TriangleFillPrimitive, GraphicalPrimitive>("TriangleFillPrimitive")

	    .addConstructor<int, const Vector&, const Vector&, const Vector&, unsigned char>()
	    .addConstructor<int, const Vector&, const Vector&, const Vector&, unsigned char, float>();
}

LuaBindingRegisterFunctionDefinitionForType(PrimitiveLuaBindings, TextPrimitive) {
	registry.Class<TextPrimitive, GraphicalPrimitive>("TextPrimitive")

	    .addConstructor<int, const Vector&, const std::string&, bool, int, float>()
	    .addConstructor<int, const Vector&, const std::string&, bool, int, float, float>();
}

LuaBindingRegisterFunctionDefinitionForType(PrimitiveLuaBindings, BitmapPrimitive) {
	registry.Class<BitmapPrimitive, GraphicalPrimitive>("BitmapPrimitive")

	    .addConstructor<int, const Vector&, const MOSprite*, float, unsigned int, float, bool, bool>()
	    .addConstructor<int, const Vector&, const std::string&, float, float, bool, bool>()
	    .addConstructor<int, const Vector&, const MOSprite*, float, unsigned int, bool, bool>()
	    .addConstructor<int, const Vector&, const MOSprite*, float, unsigned int, bool, bool, float>()
	    .addConstructor<int, const Vector&, const std::string&, float, bool, bool>()
	    .addConstructor<int, const Vector&, const std::string&, float, bool, bool, float>();
}
