#pragma once

#include "LuaBridgeDefinitions.h"
#include "LuaAdapterDefinitions.h"

namespace RTE {

#pragma region Lua Binding Registration Macros
/// Convenience macro for declaring a binding register function.
#define LuaBindingRegisterFunctionDeclarationForType(TYPE) \
	static void Register##TYPE##LuaBindings(LuaBinding::Registry& registry)

/// Convenience macro for defining a binding register function.
#define LuaBindingRegisterFunctionDefinitionForType(OWNINGSCOPE, TYPE) \
	void OWNINGSCOPE::Register##TYPE##LuaBindings(LuaBinding::Registry& registry)

/// Convenience macro for a class registration of an abstract type.
#define AbstractTypeLuaClassDefinition(TYPE, PARENTTYPE) \
	registry.Class<TYPE, PARENTTYPE>(#TYPE) \
	    .addProperty("ClassName", &TYPE::GetClassName)

/// Convenience macro for a class registration of a concrete type.
#define ConcreteTypeLuaClassDefinition(TYPE, PARENTTYPE) \
	registry.Class<TYPE, PARENTTYPE>(#TYPE) \
	    .addFunction("Clone", &LuaAdaptersEntityClone::Clone##TYPE, LuaBinding::AdoptResult) \
	    .addProperty("ClassName", &TYPE::GetClassName)

/// Convenience macro for calling a register function of a type.
#define RegisterLuaBindingsOfType(OWNINGSCOPE, TYPE) \
	OWNINGSCOPE::Register##TYPE##LuaBindings(registry)

/// Convenience macro for calling a register function of an abstract type, along with registering global bindings for adapters relevant to the type.
#define RegisterLuaBindingsOfAbstractType(OWNINGSCOPE, TYPE) \
	registry.Function("To" #TYPE, static_cast<TYPE* (*)(Entity*)>(&LuaAdaptersEntityCast::To##TYPE)); \
	registry.Function("To" #TYPE, static_cast<const TYPE* (*)(const Entity*)>(&LuaAdaptersEntityCast::ToConst##TYPE)); \
	registry.Function("Is" #TYPE, static_cast<bool (*)(const Entity*)>(&LuaAdaptersEntityCast::Is##TYPE)); \
	OWNINGSCOPE::Register##TYPE##LuaBindings(registry)

/// Convenience macro for calling a register function of a concrete type, along with registering global bindings for adapters relevant to the type.
#define RegisterLuaBindingsOfConcreteType(OWNINGSCOPE, TYPE) \
	registry.Function("Create" #TYPE, static_cast<TYPE* (*)(std::string, std::string)>(&LuaAdaptersEntityCreate::Create##TYPE), LuaBinding::AdoptResult); \
	registry.Function("Create" #TYPE, static_cast<TYPE* (*)(std::string)>(&LuaAdaptersEntityCreate::Create##TYPE), LuaBinding::AdoptResult); \
	registry.Function("Random" #TYPE, static_cast<TYPE* (*)(std::string, int)>(&LuaAdaptersEntityCreate::Random##TYPE), LuaBinding::AdoptResult); \
	registry.Function("Random" #TYPE, static_cast<TYPE* (*)(std::string, std::string)>(&LuaAdaptersEntityCreate::Random##TYPE), LuaBinding::AdoptResult); \
	registry.Function("Random" #TYPE, static_cast<TYPE* (*)(std::string)>(&LuaAdaptersEntityCreate::Random##TYPE), LuaBinding::AdoptResult); \
	registry.Function("To" #TYPE, static_cast<TYPE* (*)(Entity*)>(&LuaAdaptersEntityCast::To##TYPE)); \
	registry.Function("To" #TYPE, static_cast<const TYPE* (*)(const Entity*)>(&LuaAdaptersEntityCast::ToConst##TYPE)); \
	registry.Function("Is" #TYPE, static_cast<bool (*)(const Entity*)>(&LuaAdaptersEntityCast::Is##TYPE)); \
	OWNINGSCOPE::Register##TYPE##LuaBindings(registry)
#pragma endregion

	/// Struct that contains Lua binding registration functions for System classes.
	struct SystemLuaBindings {
		LuaBindingRegisterFunctionDeclarationForType(Box);
		LuaBindingRegisterFunctionDeclarationForType(Controller);
		LuaBindingRegisterFunctionDeclarationForType(DataModule);
		LuaBindingRegisterFunctionDeclarationForType(Timer);
		LuaBindingRegisterFunctionDeclarationForType(Vector);
		LuaBindingRegisterFunctionDeclarationForType(PathRequest);
	};

	/// Struct that contains Lua binding registration functions for Manager classes.
	struct ManagerLuaBindings {
		LuaBindingRegisterFunctionDeclarationForType(ActivityMan);
		LuaBindingRegisterFunctionDeclarationForType(AudioMan);
		LuaBindingRegisterFunctionDeclarationForType(MusicMan);
		LuaBindingRegisterFunctionDeclarationForType(CameraMan);
		LuaBindingRegisterFunctionDeclarationForType(ConsoleMan);
		LuaBindingRegisterFunctionDeclarationForType(FrameMan);
		LuaBindingRegisterFunctionDeclarationForType(MetaMan);
		LuaBindingRegisterFunctionDeclarationForType(MovableMan);
		LuaBindingRegisterFunctionDeclarationForType(PerformanceMan);
		LuaBindingRegisterFunctionDeclarationForType(PostProcessMan);
		LuaBindingRegisterFunctionDeclarationForType(PresetMan);
		LuaBindingRegisterFunctionDeclarationForType(PrimitiveMan);
		LuaBindingRegisterFunctionDeclarationForType(SceneMan);
		LuaBindingRegisterFunctionDeclarationForType(SettingsMan);
		LuaBindingRegisterFunctionDeclarationForType(TimerMan);
		LuaBindingRegisterFunctionDeclarationForType(UInputMan);
	};

	/// Struct that contains Lua binding registration functions for Entity classes.
	struct EntityLuaBindings {
		LuaBindingRegisterFunctionDeclarationForType(Entity);
		LuaBindingRegisterFunctionDeclarationForType(ACDropShip);
		LuaBindingRegisterFunctionDeclarationForType(ACrab);
		LuaBindingRegisterFunctionDeclarationForType(ACraft);
		LuaBindingRegisterFunctionDeclarationForType(ACRocket);
		LuaBindingRegisterFunctionDeclarationForType(Actor);
		LuaBindingRegisterFunctionDeclarationForType(ADoor);
		LuaBindingRegisterFunctionDeclarationForType(AEmitter);
		LuaBindingRegisterFunctionDeclarationForType(AEJetpack);
		LuaBindingRegisterFunctionDeclarationForType(AHuman);
		LuaBindingRegisterFunctionDeclarationForType(Arm);
		LuaBindingRegisterFunctionDeclarationForType(Attachable);
		LuaBindingRegisterFunctionDeclarationForType(Deployment);
		LuaBindingRegisterFunctionDeclarationForType(Emission);
		LuaBindingRegisterFunctionDeclarationForType(Gib);
		LuaBindingRegisterFunctionDeclarationForType(GlobalScript);
		LuaBindingRegisterFunctionDeclarationForType(HDFirearm);
		LuaBindingRegisterFunctionDeclarationForType(HeldDevice);
		LuaBindingRegisterFunctionDeclarationForType(Leg);
		LuaBindingRegisterFunctionDeclarationForType(LimbPath);
		LuaBindingRegisterFunctionDeclarationForType(Magazine);
		LuaBindingRegisterFunctionDeclarationForType(Material);
		LuaBindingRegisterFunctionDeclarationForType(MetaPlayer);
		LuaBindingRegisterFunctionDeclarationForType(MOPixel);
		LuaBindingRegisterFunctionDeclarationForType(MOSParticle);
		LuaBindingRegisterFunctionDeclarationForType(MOSprite);
		LuaBindingRegisterFunctionDeclarationForType(MOSRotating);
		LuaBindingRegisterFunctionDeclarationForType(MovableObject);
		LuaBindingRegisterFunctionDeclarationForType(PEmitter);
		LuaBindingRegisterFunctionDeclarationForType(PieSlice);
		LuaBindingRegisterFunctionDeclarationForType(PieMenu);
		LuaBindingRegisterFunctionDeclarationForType(Round);
		LuaBindingRegisterFunctionDeclarationForType(Scene);
		LuaBindingRegisterFunctionDeclarationForType(SceneArea);
		LuaBindingRegisterFunctionDeclarationForType(StaticSceneLayer);
		LuaBindingRegisterFunctionDeclarationForType(SceneObject);
		LuaBindingRegisterFunctionDeclarationForType(SLBackground);
		LuaBindingRegisterFunctionDeclarationForType(SoundContainer);
		LuaBindingRegisterFunctionDeclarationForType(SoundSet);
		LuaBindingRegisterFunctionDeclarationForType(TDExplosive);
		LuaBindingRegisterFunctionDeclarationForType(TerrainObject);
		LuaBindingRegisterFunctionDeclarationForType(ThrownDevice);
		LuaBindingRegisterFunctionDeclarationForType(Turret);
	};

	/// Struct that contains Lua binding registration functions for Activity classes.
	struct ActivityLuaBindings {
		LuaBindingRegisterFunctionDeclarationForType(Activity);
		LuaBindingRegisterFunctionDeclarationForType(GameActivity);
	};

	/// Struct that contains Lua binding registration functions for GUI classes.
	struct GUILuaBindings {
		LuaBindingRegisterFunctionDeclarationForType(GUIBanner);
		LuaBindingRegisterFunctionDeclarationForType(BuyMenuGUI);
		LuaBindingRegisterFunctionDeclarationForType(SceneEditorGUI);
	};

	/// Struct that contains Lua binding registration functions for GraphicalPrimitive classes.
	struct PrimitiveLuaBindings {
		LuaBindingRegisterFunctionDeclarationForType(GraphicalPrimitive);
		LuaBindingRegisterFunctionDeclarationForType(LinePrimitive);
		LuaBindingRegisterFunctionDeclarationForType(ArcPrimitive);
		LuaBindingRegisterFunctionDeclarationForType(SplinePrimitive);
		LuaBindingRegisterFunctionDeclarationForType(BoxPrimitive);
		LuaBindingRegisterFunctionDeclarationForType(BoxFillPrimitive);
		LuaBindingRegisterFunctionDeclarationForType(RoundedBoxPrimitive);
		LuaBindingRegisterFunctionDeclarationForType(RoundedBoxFillPrimitive);
		LuaBindingRegisterFunctionDeclarationForType(CirclePrimitive);
		LuaBindingRegisterFunctionDeclarationForType(CircleFillPrimitive);
		LuaBindingRegisterFunctionDeclarationForType(EllipsePrimitive);
		LuaBindingRegisterFunctionDeclarationForType(EllipseFillPrimitive);
		LuaBindingRegisterFunctionDeclarationForType(TrianglePrimitive);
		LuaBindingRegisterFunctionDeclarationForType(TriangleFillPrimitive);
		LuaBindingRegisterFunctionDeclarationForType(TextPrimitive);
		LuaBindingRegisterFunctionDeclarationForType(BitmapPrimitive);
	};

	/// Struct that contains Lua binding registration functions for various input enumerations.
	struct InputLuaBindings {
		LuaBindingRegisterFunctionDeclarationForType(InputDevice);
		LuaBindingRegisterFunctionDeclarationForType(InputElements);
		LuaBindingRegisterFunctionDeclarationForType(MouseButtons);
		LuaBindingRegisterFunctionDeclarationForType(JoyButtons);
		LuaBindingRegisterFunctionDeclarationForType(JoyDirections);
		LuaBindingRegisterFunctionDeclarationForType(SDL_Scancode);
		LuaBindingRegisterFunctionDeclarationForType(SDL_Keycode);
		LuaBindingRegisterFunctionDeclarationForType(SDL_GamepadButton);
		LuaBindingRegisterFunctionDeclarationForType(SDL_GamepadAxis);
	};

	/// Struct that contains Lua binding registration functions for types that don't really belong in any of the other binding structs.
	struct MiscLuaBindings {
		LuaBindingRegisterFunctionDeclarationForType(AlarmEvent);
		LuaBindingRegisterFunctionDeclarationForType(Directions);
		LuaBindingRegisterFunctionDeclarationForType(DrawBlendMode);
		LuaBindingRegisterFunctionDeclarationForType(DrawDepth);
	};
} // namespace RTE
