// Make sure that binding definition files are always set to NOT use pre-compiled headers and conformance mode (/permissive) otherwise everything will be on fire!

#include "LuaBindingRegisterDefinitions.h"

#include "PrimitiveMan.h"

namespace RTE {

	LuaBindingRegisterFunctionDefinitionForType(PrimitiveLuaBindings, GraphicalPrimitive) {
		auto luaType = SimpleTypeLuaClassDefinition(GraphicalPrimitive);
		;
	}

	LuaBindingRegisterFunctionDefinitionForType(PrimitiveLuaBindings, LinePrimitive) {
		auto luaType = SimpleTypeLuaClassDefinition(LinePrimitive);

		luaType.set(sol::call_constructor, sol::constructors<
		                                       LinePrimitive(int, const Vector&, const Vector&, unsigned char)>());
	}

	LuaBindingRegisterFunctionDefinitionForType(PrimitiveLuaBindings, ArcPrimitive) {
		auto luaType = SimpleTypeLuaClassDefinition(ArcPrimitive);

		luaType.set(sol::call_constructor, sol::constructors<
		                                       ArcPrimitive(int, const Vector&, float, float, int, int, unsigned char)>());
	}

	LuaBindingRegisterFunctionDefinitionForType(PrimitiveLuaBindings, SplinePrimitive) {
		auto luaType = SimpleTypeLuaClassDefinition(SplinePrimitive);

		luaType.set(sol::call_constructor, sol::constructors<
		                                       ArcPrimitive(int, const Vector&, const Vector&, const Vector&, const Vector&, unsigned char)>());
	}

	LuaBindingRegisterFunctionDefinitionForType(PrimitiveLuaBindings, BoxPrimitive) {
		auto luaType = SimpleTypeLuaClassDefinition(BoxPrimitive);

		luaType.set(sol::call_constructor, sol::constructors<
		                                       BoxPrimitive(int, const Vector&, const Vector&, unsigned char)>());
	}

	LuaBindingRegisterFunctionDefinitionForType(PrimitiveLuaBindings, BoxFillPrimitive) {
		auto luaType = SimpleTypeLuaClassDefinition(BoxFillPrimitive);

		luaType.set(sol::call_constructor, sol::constructors<
		                                       BoxFillPrimitive(int, const Vector&, const Vector&, unsigned char)>());
	}

	LuaBindingRegisterFunctionDefinitionForType(PrimitiveLuaBindings, RoundedBoxPrimitive) {
		auto luaType = SimpleTypeLuaClassDefinition(RoundedBoxPrimitive);

		luaType.set(sol::call_constructor, sol::constructors<
		                                       RoundedBoxPrimitive(int, const Vector&, const Vector&, int, unsigned char)>());
	}

	LuaBindingRegisterFunctionDefinitionForType(PrimitiveLuaBindings, RoundedBoxFillPrimitive) {
		auto luaType = SimpleTypeLuaClassDefinition(RoundedBoxFillPrimitive);

		luaType.set(sol::call_constructor, sol::constructors<
		                                       RoundedBoxFillPrimitive(int, const Vector&, const Vector&, int, unsigned char)>());
	}

	LuaBindingRegisterFunctionDefinitionForType(PrimitiveLuaBindings, CirclePrimitive) {
		auto luaType = SimpleTypeLuaClassDefinition(CirclePrimitive);

		luaType.set(sol::call_constructor, sol::constructors<
		                                       CirclePrimitive(int, const Vector&, int, unsigned char)>());
	}

	LuaBindingRegisterFunctionDefinitionForType(PrimitiveLuaBindings, CircleFillPrimitive) {
		auto luaType = SimpleTypeLuaClassDefinition(CircleFillPrimitive);

		luaType.set(sol::call_constructor, sol::constructors<
		                                       CircleFillPrimitive(int, const Vector&, int, unsigned char)>());
	}

	LuaBindingRegisterFunctionDefinitionForType(PrimitiveLuaBindings, EllipsePrimitive) {
		auto luaType = SimpleTypeLuaClassDefinition(EllipsePrimitive);

		luaType.set(sol::call_constructor, sol::constructors<
		                                       EllipsePrimitive(int, const Vector&, int, int, unsigned char)>());
	}

	LuaBindingRegisterFunctionDefinitionForType(PrimitiveLuaBindings, EllipseFillPrimitive) {
		auto luaType = SimpleTypeLuaClassDefinition(EllipseFillPrimitive);

		luaType.set(sol::call_constructor, sol::constructors<
		                                       EllipseFillPrimitive(int, const Vector&, int, int, unsigned char)>());
	}

	LuaBindingRegisterFunctionDefinitionForType(PrimitiveLuaBindings, TrianglePrimitive) {
		auto luaType = SimpleTypeLuaClassDefinition(TrianglePrimitive);

		luaType.set(sol::call_constructor, sol::constructors<
		                                       TrianglePrimitive(int, const Vector&, const Vector&, const Vector&, unsigned char)>());
	}

	LuaBindingRegisterFunctionDefinitionForType(PrimitiveLuaBindings, TriangleFillPrimitive) {
		auto luaType = SimpleTypeLuaClassDefinition(TriangleFillPrimitive);

		luaType.set(sol::call_constructor, sol::constructors<
		                                       TriangleFillPrimitive(int, const Vector&, const Vector&, const Vector&, unsigned char)>());
	}

	LuaBindingRegisterFunctionDefinitionForType(PrimitiveLuaBindings, TextPrimitive) {
		auto luaType = SimpleTypeLuaClassDefinition(TextPrimitive);

		luaType.set(sol::call_constructor, sol::constructors<
		                                       TextPrimitive(int, const Vector&, const std::string&, bool, int, float)>());
	}

	LuaBindingRegisterFunctionDefinitionForType(PrimitiveLuaBindings, BitmapPrimitive) {
		auto luaType = SimpleTypeLuaClassDefinition(BitmapPrimitive);

		luaType.set(sol::call_constructor, sol::constructors<
		                                       BitmapPrimitive(int, const Vector&, const MOSprite*, float, int, bool, bool),
		                                       BitmapPrimitive(int, const Vector&, const std::string&, float, bool, bool)>());
	}
} // namespace RTE
