#pragma once

#include "LuabindDefinitions.h"

#include "ActivityMan.h"
#include "AudioMan.h"
#include "CameraMan.h"
#include "ConsoleMan.h"
#include "FrameMan.h"
#include "MetaMan.h"
#include "MovableMan.h"
#include "PerformanceMan.h"
#include "PostProcessMan.h"
#include "PresetMan.h"
#include "PrimitiveMan.h"
#include "SceneMan.h"
#include "SettingsMan.h"
#include "TimerMan.h"
#include "UInputMan.h"

#include "Box.h"
#include "Controller.h"
#include "DataModule.h"
#include "GraphicalPrimitive.h"
#include "PathFinder.h"

#include "GUIBanner.h"
#include "BuyMenuGUI.h"
#include "SceneEditorGUI.h"

#include "GameActivity.h"
#include "GAScripted.h"
#include "ActorEditor.h"

#include "Entity.h"
#include "ACDropShip.h"
#include "ACrab.h"
#include "ACraft.h"
#include "ACRocket.h"
#include "Actor.h"
#include "ADoor.h"
#include "AEmitter.h"
#include "AEJetpack.h"
#include "AHuman.h"
#include "Arm.h"
#include "AtomGroup.h"
#include "Attachable.h"
#include "Deployment.h"
#include "Emission.h"
#include "Gib.h"
#include "GlobalScript.h"
#include "HDFirearm.h"
#include "HeldDevice.h"
#include "Leg.h"
#include "LimbPath.h"
#include "Magazine.h"
#include "Material.h"
#include "MetaPlayer.h"
#include "MOSParticle.h"
#include "MOPixel.h"
#include "MOSprite.h"
#include "MOSRotating.h"
#include "MovableObject.h"
#include "PEmitter.h"
#include "Round.h"
#include "Scene.h"
#include "SceneObject.h"
#include "SLBackground.h"
#include "SLTerrain.h"
#include "SoundContainer.h"
#include "TerrainObject.h"
#include "TDExplosive.h"
#include "ThrownDevice.h"
#include "Turret.h"
#include "PieMenu.h"
#include "PieSlice.h"

#include "System/MicroPather/micropather.h"

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

struct lua_State;

namespace RTE {

	class LuabindObjectWrapper;

#pragma region Entity Lua Adapter Macros
	struct LuaAdaptersEntityCreate {
/// Convenience macro to generate preset clone-create adapter functions that will return the exact pre-cast types, so we don't have to do: myNewActor = ToActor(PresetMan:GetPreset("AHuman", "Soldier Light", "All")):Clone()
/// But can instead do: myNewActor = CreateActor("Soldier Light", "All");
/// Or even: myNewActor = CreateActor("Soldier Light");
/// Or for a randomly selected Preset within a group: myNewActor = RandomActor("Light Troops");
#define LuaEntityCreateFunctionsDeclarationsForType(TYPE) \
	static TYPE* Create##TYPE(std::string preseName, std::string moduleName); \
	static TYPE* Create##TYPE(std::string preset); \
	static TYPE* Random##TYPE(std::string groupName, int moduleSpaceID); \
	static TYPE* Random##TYPE(std::string groupName, std::string dataModuleName); \
	static TYPE* Random##TYPE(std::string groupName)

		LuaEntityCreateFunctionsDeclarationsForType(SoundContainer);
		LuaEntityCreateFunctionsDeclarationsForType(Attachable);
		LuaEntityCreateFunctionsDeclarationsForType(Arm);
		LuaEntityCreateFunctionsDeclarationsForType(Leg);
		LuaEntityCreateFunctionsDeclarationsForType(AEmitter);
		LuaEntityCreateFunctionsDeclarationsForType(AEJetpack);
		LuaEntityCreateFunctionsDeclarationsForType(Turret);
		LuaEntityCreateFunctionsDeclarationsForType(Actor);
		LuaEntityCreateFunctionsDeclarationsForType(ADoor);
		LuaEntityCreateFunctionsDeclarationsForType(AHuman);
		LuaEntityCreateFunctionsDeclarationsForType(ACrab);
		LuaEntityCreateFunctionsDeclarationsForType(ACraft);
		LuaEntityCreateFunctionsDeclarationsForType(ACDropShip);
		LuaEntityCreateFunctionsDeclarationsForType(ACRocket);
		LuaEntityCreateFunctionsDeclarationsForType(MOSParticle);
		LuaEntityCreateFunctionsDeclarationsForType(MOSRotating);
		LuaEntityCreateFunctionsDeclarationsForType(MOPixel);
		LuaEntityCreateFunctionsDeclarationsForType(Scene);
		LuaEntityCreateFunctionsDeclarationsForType(HeldDevice);
		LuaEntityCreateFunctionsDeclarationsForType(Round);
		LuaEntityCreateFunctionsDeclarationsForType(Magazine);
		LuaEntityCreateFunctionsDeclarationsForType(HDFirearm);
		LuaEntityCreateFunctionsDeclarationsForType(ThrownDevice);
		LuaEntityCreateFunctionsDeclarationsForType(TDExplosive);
		LuaEntityCreateFunctionsDeclarationsForType(TerrainObject);
		LuaEntityCreateFunctionsDeclarationsForType(PEmitter);
		LuaEntityCreateFunctionsDeclarationsForType(PieSlice);
		LuaEntityCreateFunctionsDeclarationsForType(PieMenu);
	};

	struct LuaAdaptersEntityClone {
/// Convenience macro to generate a preset clone adapter function for a type.
#define LuaEntityCloneFunctionDeclarationForType(TYPE) \
	static TYPE* Clone##TYPE(const TYPE* thisEntity)

		LuaEntityCloneFunctionDeclarationForType(Entity);
		LuaEntityCloneFunctionDeclarationForType(SoundContainer);
		LuaEntityCloneFunctionDeclarationForType(SceneObject);
		LuaEntityCloneFunctionDeclarationForType(MovableObject);
		LuaEntityCloneFunctionDeclarationForType(Attachable);
		LuaEntityCloneFunctionDeclarationForType(Arm);
		LuaEntityCloneFunctionDeclarationForType(Leg);
		LuaEntityCloneFunctionDeclarationForType(Emission);
		LuaEntityCloneFunctionDeclarationForType(AEmitter);
		LuaEntityCloneFunctionDeclarationForType(AEJetpack);
		LuaEntityCloneFunctionDeclarationForType(Turret);
		LuaEntityCloneFunctionDeclarationForType(Actor);
		LuaEntityCloneFunctionDeclarationForType(ADoor);
		LuaEntityCloneFunctionDeclarationForType(AHuman);
		LuaEntityCloneFunctionDeclarationForType(ACrab);
		LuaEntityCloneFunctionDeclarationForType(ACraft);
		LuaEntityCloneFunctionDeclarationForType(ACDropShip);
		LuaEntityCloneFunctionDeclarationForType(ACRocket);
		LuaEntityCloneFunctionDeclarationForType(MOSParticle);
		LuaEntityCloneFunctionDeclarationForType(MOSRotating);
		LuaEntityCloneFunctionDeclarationForType(MOPixel);
		LuaEntityCloneFunctionDeclarationForType(Scene);
		LuaEntityCloneFunctionDeclarationForType(HeldDevice);
		LuaEntityCloneFunctionDeclarationForType(Round);
		LuaEntityCloneFunctionDeclarationForType(Magazine);
		LuaEntityCloneFunctionDeclarationForType(HDFirearm);
		LuaEntityCloneFunctionDeclarationForType(ThrownDevice);
		LuaEntityCloneFunctionDeclarationForType(TDExplosive);
		LuaEntityCloneFunctionDeclarationForType(TerrainObject);
		LuaEntityCloneFunctionDeclarationForType(PEmitter);
		LuaEntityCloneFunctionDeclarationForType(PieSlice);
		LuaEntityCloneFunctionDeclarationForType(PieMenu);
	};

	struct LuaAdaptersEntityCast {
/// Convenience macro to generate type casting adapter functions for a type.
#define LuaEntityCastFunctionsDeclarationsForType(TYPE) \
	static TYPE* To##TYPE(Entity* entity); \
	static const TYPE* ToConst##TYPE(const Entity* entity); \
	static bool Is##TYPE(Entity* entity); \
	static LuabindObjectWrapper* ToLuabindObject##TYPE(Entity* entity, lua_State* luaState)

		static std::unordered_map<std::string, std::function<LuabindObjectWrapper*(Entity*, lua_State*)>> s_EntityToLuabindObjectCastFunctions; //!< Map of preset names to casting methods for ensuring objects are downcast properly when passed into Lua.

		LuaEntityCastFunctionsDeclarationsForType(Entity);
		LuaEntityCastFunctionsDeclarationsForType(SoundContainer);
		LuaEntityCastFunctionsDeclarationsForType(SceneObject);
		LuaEntityCastFunctionsDeclarationsForType(MovableObject);
		LuaEntityCastFunctionsDeclarationsForType(Attachable);
		LuaEntityCastFunctionsDeclarationsForType(Arm);
		LuaEntityCastFunctionsDeclarationsForType(Leg);
		LuaEntityCastFunctionsDeclarationsForType(Emission);
		LuaEntityCastFunctionsDeclarationsForType(AEmitter);
		LuaEntityCastFunctionsDeclarationsForType(AEJetpack);
		LuaEntityCastFunctionsDeclarationsForType(Turret);
		LuaEntityCastFunctionsDeclarationsForType(Actor);
		LuaEntityCastFunctionsDeclarationsForType(ADoor);
		LuaEntityCastFunctionsDeclarationsForType(AHuman);
		LuaEntityCastFunctionsDeclarationsForType(ACrab);
		LuaEntityCastFunctionsDeclarationsForType(ACraft);
		LuaEntityCastFunctionsDeclarationsForType(ACDropShip);
		LuaEntityCastFunctionsDeclarationsForType(ACRocket);
		LuaEntityCastFunctionsDeclarationsForType(MOSParticle);
		LuaEntityCastFunctionsDeclarationsForType(MOSRotating);
		LuaEntityCastFunctionsDeclarationsForType(MOPixel);
		LuaEntityCastFunctionsDeclarationsForType(MOSprite);
		LuaEntityCastFunctionsDeclarationsForType(Scene);
		LuaEntityCastFunctionsDeclarationsForType(Deployment);
		LuaEntityCastFunctionsDeclarationsForType(GameActivity);
		LuaEntityCastFunctionsDeclarationsForType(GlobalScript);
		LuaEntityCastFunctionsDeclarationsForType(GAScripted);
		LuaEntityCastFunctionsDeclarationsForType(HeldDevice);
		LuaEntityCastFunctionsDeclarationsForType(Round);
		LuaEntityCastFunctionsDeclarationsForType(Magazine);
		LuaEntityCastFunctionsDeclarationsForType(HDFirearm);
		LuaEntityCastFunctionsDeclarationsForType(ThrownDevice);
		LuaEntityCastFunctionsDeclarationsForType(TDExplosive);
		LuaEntityCastFunctionsDeclarationsForType(TerrainObject);
		LuaEntityCastFunctionsDeclarationsForType(PEmitter);
		LuaEntityCastFunctionsDeclarationsForType(PieSlice);
		LuaEntityCastFunctionsDeclarationsForType(PieMenu);
	};

	struct LuaAdaptersPropertyOwnershipSafetyFaker {
/// Special handling for passing ownership through properties. If you try to pass null to this normally, LuaJIT crashes.
/// This handling avoids that, and is a bit safer since there's no actual ownership transfer from Lua to C++.
#define LuaPropertyOwnershipSafetyFakerFunctionDeclaration(OBJECTTYPE, PROPERTYTYPE, SETTERFUNCTION) \
	static void OBJECTTYPE##SETTERFUNCTION(OBJECTTYPE* luaSelfObject, PROPERTYTYPE* objectToSet)

		LuaPropertyOwnershipSafetyFakerFunctionDeclaration(MOSRotating, SoundContainer, SetGibSound);
		LuaPropertyOwnershipSafetyFakerFunctionDeclaration(Attachable, AEmitter, SetBreakWound);
		LuaPropertyOwnershipSafetyFakerFunctionDeclaration(Attachable, AEmitter, SetParentBreakWound);
		LuaPropertyOwnershipSafetyFakerFunctionDeclaration(AEmitter, Attachable, SetFlash);
		LuaPropertyOwnershipSafetyFakerFunctionDeclaration(AEmitter, SoundContainer, SetEmissionSound);
		LuaPropertyOwnershipSafetyFakerFunctionDeclaration(AEmitter, SoundContainer, SetBurstSound);
		LuaPropertyOwnershipSafetyFakerFunctionDeclaration(AEmitter, SoundContainer, SetEndSound);
		LuaPropertyOwnershipSafetyFakerFunctionDeclaration(ADoor, Attachable, SetDoor);
		LuaPropertyOwnershipSafetyFakerFunctionDeclaration(Arm, HeldDevice, SetHeldDevice);
		LuaPropertyOwnershipSafetyFakerFunctionDeclaration(Leg, Attachable, SetFoot);
		LuaPropertyOwnershipSafetyFakerFunctionDeclaration(Actor, PieMenu, SetPieMenu);
		LuaPropertyOwnershipSafetyFakerFunctionDeclaration(Actor, SoundContainer, SetBodyHitSound);
		LuaPropertyOwnershipSafetyFakerFunctionDeclaration(Actor, SoundContainer, SetAlarmSound);
		LuaPropertyOwnershipSafetyFakerFunctionDeclaration(Actor, SoundContainer, SetPainSound);
		LuaPropertyOwnershipSafetyFakerFunctionDeclaration(Actor, SoundContainer, SetDeathSound);
		LuaPropertyOwnershipSafetyFakerFunctionDeclaration(Actor, SoundContainer, SetDeviceSwitchSound);
		LuaPropertyOwnershipSafetyFakerFunctionDeclaration(ADoor, SoundContainer, SetDoorMoveStartSound);
		LuaPropertyOwnershipSafetyFakerFunctionDeclaration(ADoor, SoundContainer, SetDoorMoveSound);
		LuaPropertyOwnershipSafetyFakerFunctionDeclaration(ADoor, SoundContainer, SetDoorDirectionChangeSound);
		LuaPropertyOwnershipSafetyFakerFunctionDeclaration(ADoor, SoundContainer, SetDoorMoveEndSound);
		LuaPropertyOwnershipSafetyFakerFunctionDeclaration(AHuman, Attachable, SetHead);
		LuaPropertyOwnershipSafetyFakerFunctionDeclaration(AHuman, AEJetpack, SetJetpack);
		LuaPropertyOwnershipSafetyFakerFunctionDeclaration(AHuman, Arm, SetFGArm);
		LuaPropertyOwnershipSafetyFakerFunctionDeclaration(AHuman, Arm, SetBGArm);
		LuaPropertyOwnershipSafetyFakerFunctionDeclaration(AHuman, Leg, SetFGLeg);
		LuaPropertyOwnershipSafetyFakerFunctionDeclaration(AHuman, Leg, SetBGLeg);
		LuaPropertyOwnershipSafetyFakerFunctionDeclaration(AHuman, Attachable, SetFGFoot);
		LuaPropertyOwnershipSafetyFakerFunctionDeclaration(AHuman, Attachable, SetBGFoot);
		LuaPropertyOwnershipSafetyFakerFunctionDeclaration(AHuman, SoundContainer, SetStrideSound);
		LuaPropertyOwnershipSafetyFakerFunctionDeclaration(ACrab, Turret, SetTurret);
		LuaPropertyOwnershipSafetyFakerFunctionDeclaration(ACrab, AEJetpack, SetJetpack);
		LuaPropertyOwnershipSafetyFakerFunctionDeclaration(ACrab, Leg, SetLeftFGLeg);
		LuaPropertyOwnershipSafetyFakerFunctionDeclaration(ACrab, Leg, SetLeftBGLeg);
		LuaPropertyOwnershipSafetyFakerFunctionDeclaration(ACrab, Leg, SetRightFGLeg);
		LuaPropertyOwnershipSafetyFakerFunctionDeclaration(ACrab, Leg, SetRightBGLeg);
		LuaPropertyOwnershipSafetyFakerFunctionDeclaration(ACrab, SoundContainer, SetStrideSound);
		LuaPropertyOwnershipSafetyFakerFunctionDeclaration(Turret, HeldDevice, SetFirstMountedDevice);
		LuaPropertyOwnershipSafetyFakerFunctionDeclaration(ACraft, SoundContainer, SetHatchOpenSound);
		LuaPropertyOwnershipSafetyFakerFunctionDeclaration(ACraft, SoundContainer, SetHatchCloseSound);
		LuaPropertyOwnershipSafetyFakerFunctionDeclaration(ACraft, SoundContainer, SetCrashSound);
		LuaPropertyOwnershipSafetyFakerFunctionDeclaration(ACDropShip, AEmitter, SetRightThruster);
		LuaPropertyOwnershipSafetyFakerFunctionDeclaration(ACDropShip, AEmitter, SetLeftThruster);
		LuaPropertyOwnershipSafetyFakerFunctionDeclaration(ACDropShip, AEmitter, SetURightThruster);
		LuaPropertyOwnershipSafetyFakerFunctionDeclaration(ACDropShip, AEmitter, SetULeftThruster);
		LuaPropertyOwnershipSafetyFakerFunctionDeclaration(ACDropShip, Attachable, SetRightHatch);
		LuaPropertyOwnershipSafetyFakerFunctionDeclaration(ACDropShip, Attachable, SetLeftHatch);
		LuaPropertyOwnershipSafetyFakerFunctionDeclaration(ACRocket, Leg, SetRightLeg);
		LuaPropertyOwnershipSafetyFakerFunctionDeclaration(ACRocket, Leg, SetLeftLeg);
		LuaPropertyOwnershipSafetyFakerFunctionDeclaration(ACRocket, AEmitter, SetMainThruster);
		LuaPropertyOwnershipSafetyFakerFunctionDeclaration(ACRocket, AEmitter, SetLeftThruster);
		LuaPropertyOwnershipSafetyFakerFunctionDeclaration(ACRocket, AEmitter, SetRightThruster);
		LuaPropertyOwnershipSafetyFakerFunctionDeclaration(ACRocket, AEmitter, SetULeftThruster);
		LuaPropertyOwnershipSafetyFakerFunctionDeclaration(ACRocket, AEmitter, SetURightThruster);
		LuaPropertyOwnershipSafetyFakerFunctionDeclaration(HDFirearm, Magazine, SetMagazine);
		LuaPropertyOwnershipSafetyFakerFunctionDeclaration(HDFirearm, Attachable, SetFlash);
		LuaPropertyOwnershipSafetyFakerFunctionDeclaration(HDFirearm, SoundContainer, SetPreFireSound);
		LuaPropertyOwnershipSafetyFakerFunctionDeclaration(HDFirearm, SoundContainer, SetFireSound);
		LuaPropertyOwnershipSafetyFakerFunctionDeclaration(HDFirearm, SoundContainer, SetFireEchoSound);
		LuaPropertyOwnershipSafetyFakerFunctionDeclaration(HDFirearm, SoundContainer, SetActiveSound);
		LuaPropertyOwnershipSafetyFakerFunctionDeclaration(HDFirearm, SoundContainer, SetDeactivationSound);
		LuaPropertyOwnershipSafetyFakerFunctionDeclaration(HDFirearm, SoundContainer, SetEmptySound);
		LuaPropertyOwnershipSafetyFakerFunctionDeclaration(HDFirearm, SoundContainer, SetReloadStartSound);
		LuaPropertyOwnershipSafetyFakerFunctionDeclaration(HDFirearm, SoundContainer, SetReloadEndSound);
	};
#pragma endregion

#pragma region Entity Lua Adapters
	struct LuaAdaptersEntity {
		// TODO this is a temporary fix for lua PresetName setting causing scripts to have to rerun. It should be replaced with a DisplayName property someday.
		static void SetPresetName(Entity* luaSelfObject, const std::string& presetName);
	};
#pragma endregion

#pragma region Scene Lua Adapters
	struct LuaAdaptersScene {
		static int CalculatePath1(Scene* luaSelfObject, const Vector& start, const Vector& end, bool movePathToGround, float digStrength) { return CalculatePath2(luaSelfObject, start, end, movePathToGround, digStrength, Activity::Teams::NoTeam); }
		static int CalculatePath2(Scene* luaSelfObject, const Vector& start, const Vector& end, bool movePathToGround, float digStrength, Activity::Teams team);

		static void CalculatePathAsync1(Scene* luaSelfObject, const luabind::object& callback, const Vector& start, const Vector& end, bool movePathToGround, float digStrength) { return CalculatePathAsync2(luaSelfObject, callback, start, end, movePathToGround, digStrength, Activity::Teams::NoTeam); }
		static void CalculatePathAsync2(Scene* luaSelfObject, const luabind::object& callback, const Vector& start, const Vector& end, bool movePathToGround, float digStrength, Activity::Teams team);
	};
#pragma endregion

#pragma region Actor Lua Adapters
	struct LuaAdaptersActor {
		static std::vector<Vector>* GetSceneWaypoints(Actor* luaSelfObject);
	};
#pragma endregion

#pragma region AHuman Lua Adapters
	struct LuaAdaptersAHuman {
		static void ReloadFirearms(AHuman* luaSelfObject);
	};
#pragma endregion

#pragma region Attachable Lua Adapters
	struct LuaAdaptersAttachable {
		static Attachable* RemoveFromParent1(Attachable* luaSelfObject);
		static Attachable* RemoveFromParent2(Attachable* luaSelfObject, bool addToMovableMan, bool addBreakWounds);
	};
#pragma endregion

#pragma region GlobalScript Lua Adapters
	struct LuaAdaptersGlobalScript {
		static void Deactivate(GlobalScript* luaSelfObject);
	};
#pragma endregion

#pragma region Activity Lua Adapters
	struct LuaAdaptersActivity {
		static void SendMessage1(Activity* luaSelfObject, const std::string& message);
		static void SendMessage2(Activity* luaSelfObject, const std::string& message, luabind::object context);
	};
#pragma endregion

#pragma region MovableObject Lua Adapters
	struct LuaAdaptersMovableObject {
		static bool HasScript(MovableObject* luaSelfObject, const std::string& scriptPath);
		static bool AddScript(MovableObject* luaSelfObject, const std::string& scriptPath);
		static bool EnableScript(MovableObject* luaSelfObject, const std::string& scriptPath);
		static bool DisableScript1(MovableObject* luaSelfObject);
		static bool DisableScript2(MovableObject* luaSelfObject, const std::string& scriptPath);
		static void SendMessage1(MovableObject* luaSelfObject, const std::string& message);
		static void SendMessage2(MovableObject* luaSelfObject, const std::string& message, luabind::object context);
	};
#pragma endregion

#pragma region MOSRotating Lua Adapters
	struct LuaAdaptersMOSRotating {
		static void GibThis(MOSRotating* luaSelfObject);
		static std::vector<AEmitter*>* GetWounds1(const MOSRotating* luaSelfObject);
		static std::vector<AEmitter*>* GetWounds2(const MOSRotating* luaSelfObject, bool includePositiveDamageAttachables, bool includeNegativeDamageAttachables, bool includeNoDamageAttachables);
		// Need a seperate implementation function without the return so we can safely recurse.
		static void GetWoundsImpl(const MOSRotating* luaSelfObject, bool includePositiveDamageAttachables, bool includeNegativeDamageAttachables, bool includeNoDamageAttachables, std::vector<AEmitter*>& wounds);
	};
#pragma endregion

#pragma region BuyMenuGUI Lua Adapters
	struct LuaAdaptersBuyMenuGUI {
		static std::list<SceneObject*>* GetOrderList(const BuyMenuGUI* luaSelfObject);
	};
#pragma endregion

#pragma region PieMenu Lua Adapters
	struct LuaAdaptersPieMenu {
		static int GetPieCommand(PieMenu* luaSelfObject);
		static bool AddPieSlice(PieMenu* luaSelfObject, PieSlice* pieSliceToAdd, const Entity* pieSliceOriginalSource);
		static bool AddPieSliceIfPresetNameIsUnique1(PieMenu* luaSelfObject, PieSlice* pieSliceToAdd, const Entity* pieSliceOriginalSource);
		static bool AddPieSliceIfPresetNameIsUnique2(PieMenu* luaSelfObject, PieSlice* pieSliceToAdd, const Entity* pieSliceOriginalSource, bool onlyCheckPieSlicesWithSameOriginalSource);
	};
#pragma endregion

#pragma region PieSlice Lua Adapters
	struct LuaAdaptersPieSlice {
		static int GetType(PieSlice* luaSelfObject);
	};
#pragma endregion

#pragma region SceneObject Lua Adapters
	struct LuaAdaptersSceneObject {
		static float GetTotalValue(const SceneObject* luaSelfObject, int nativeModule, float foreignMult);
		static int GetBuyableMode(const SceneObject* luaSelfObject);
	};
#pragma endregion

#pragma region Turret Lua Adapters
	struct LuaAdaptersTurret {
		static void AddMountedFirearm(Turret* luaSelfObject, HDFirearm* newMountedDevice);
	};
#pragma endregion

#pragma region MovableMan Lua Adapters
	struct LuaAdaptersMovableMan {
		/// Adds the given MovableObject to MovableMan if it doesn't already exist in there, or prints an error if it does.
		/// @param movableMan A reference to MovableMan, provided by Lua.
		/// @param movableObject A pointer to the MovableObject to be added.
		static void AddMO(MovableMan& movableMan, MovableObject* movableObject);

		/// Adds the given Actor to MovableMan if it doesn't already exist in there, or prints an error if it does.
		/// @param movableMan A reference to MovableMan, provided by Lua.
		/// @param actor A pointer to the Actor to be added.
		static void AddActor(MovableMan& movableMan, Actor* actor);

		/// Adds the given item MovableObject (generally a HeldDevice) to MovableMan if it doesn't already exist in there, or prints an error if it does.
		/// @param movableMan A reference to MovableMan, provided by Lua.
		/// @param item A pointer to the item to be added.
		static void AddItem(MovableMan& movableMan, HeldDevice* item);

		/// Adds the given particle MovableObject to MovableMan if it doesn't already exist in there, or prints an error if it does.
		/// @param movableMan A reference to MovableMan, provided by Lua.
		/// @param particle A pointer to the particle to be added.
		static void AddParticle(MovableMan& movableMan, MovableObject* particle);

		static void SendGlobalMessage1(MovableMan& movableMan, const std::string& message);
		static void SendGlobalMessage2(MovableMan& movableMan, const std::string& message, luabind::object context);
	};
#pragma endregion

#pragma region TimerMan Lua Adapters
	struct LuaAdaptersTimerMan {
		/// Gets the current number of ticks that the simulation should be updating with. Lua can't handle int64 (or long long apparently) so we'll expose this specialized function.
		/// @return The current fixed delta time that the simulation should be updating with, in ticks.
		static double GetDeltaTimeTicks(const TimerMan& timerMan);

		/// Gets the number of ticks per second. Lua can't handle int64 (or long long apparently) so we'll expose this specialized function.
		/// @return The number of ticks per second.
		static double GetTicksPerSecond(const TimerMan& timerMan);
	};
#pragma endregion

#pragma region UInputMan Lua Adapters
	struct LuaAdaptersUInputMan {
		/// Gets whether a mouse button is being held down right now.
		/// @param whichButton Which button to check for.
		/// @return Whether the mouse button is held or not.
		static bool MouseButtonHeld(const UInputMan& uinputMan, int whichButton);

		/// Gets whether a mouse button was pressed between the last update and the one previous to it.
		/// @param whichButton Which button to check for.
		/// @return Whether the mouse button is pressed or not.
		static bool MouseButtonPressed(const UInputMan& uinputMan, int whichButton);

		/// Gets whether a mouse button was released between the last update and the one previous to it.
		/// @param whichButton Which button to check for.
		/// @return Whether the mouse button is released or not.
		static bool MouseButtonReleased(const UInputMan& uinputMan, int whichButton);
	};
#pragma endregion

#pragma region PresetMan Lua Adapters
	struct LuaAdaptersPresetMan {
		/// Reloads the specified Entity preset in PresetMan.
		/// @param presetName The preset name of the Entity to reload.
		/// @param className The class name of the Entity to reload.
		/// @param moduleName The module name of the Entity to reload.
		/// @return Whether or not the Entity was reloaded.
		static bool ReloadEntityPreset1(PresetMan& presetMan, const std::string& presetName, const std::string& className, const std::string& moduleName);

		/// Reloads the specified Entity preset in PresetMan.
		/// @param presetName The preset name of the Entity to reload.
		/// @param className The class name of the Entity to reload.
		/// @return Whether or not the Entity was reloaded.
		static bool ReloadEntityPreset2(PresetMan& presetMan, const std::string& presetName, const std::string& className);

		/// Gets a list all previously read in (defined) Entities which are associated with a specific group.
		/// @param group The group to look for. "All" will look in all.
		/// @param type The name of the least common denominator type of the Entities you want. "All" will look at all types.
		/// @param whichModule Whether to only get those of one specific DataModule (0-n), or all (-1).
		/// @return The list of all Entities with the given group and type in the module.
		static std::list<Entity*>* GetAllEntitiesOfGroup(PresetMan& presetMan, const std::string& group, const std::string& type, int whichModule);
		static std::list<Entity*>* GetAllEntitiesOfGroup2(PresetMan& presetMan, const std::string& group, const std::string& type) { return GetAllEntitiesOfGroup(presetMan, group, type, -1); }
		static std::list<Entity*>* GetAllEntitiesOfGroup3(PresetMan& presetMan, const std::string& group) { return GetAllEntitiesOfGroup2(presetMan, group, "All"); }

		/// Gets a list all previously read in (defined) Entities.
		/// @return The list of all Entities.
		static std::list<Entity*>* GetAllEntities(PresetMan& presetMan) { return GetAllEntitiesOfGroup3(presetMan, "All"); }
	};
#pragma endregion

#pragma region SceneMan Lua Adapters
	struct LuaAdaptersSceneMan {
		/// Takes a Box and returns a list of Boxes that describe the Box, wrapped appropriately for the current Scene.
		/// @param boxToWrap The Box to wrap.
		/// @return A list of Boxes that make up the Box to wrap, wrapped appropriately for the current Scene.
		static const std::list<Box>* WrapBoxes(SceneMan& sceneMan, const Box& boxToWrap);
	};
#pragma endregion

#pragma region PrimitiveMan Lua Adapters
	struct LuaAdaptersPrimitiveMan {
		/// Schedule to draw a polygon primitive.
		/// @param primitiveMan A reference to PrimitiveMan, provided by Lua.
		/// @param centerPos Position of primitive's center in Scene coordinates.
		/// @param color Color to draw primitive with.
		/// @param verticesTable A Lua table that contains the positions of the primitive's vertices, relative to the center position.
		static void DrawPolygonPrimitive(PrimitiveMan& primitiveMan, const Vector& centerPos, int color, const luabind::object& verticesTable);

		/// Schedule to draw a polygon primitive visible only to a specified player.
		/// @param primitiveMan A reference to PrimitiveMan, provided by Lua.
		/// @param player Player screen to draw primitive on.
		/// @param centerPos Position of primitive's center in Scene coordinates.
		/// @param color Color to draw primitive with.
		/// @param verticesTable A Lua table that contains the positions of the primitive's vertices, relative to the center position.
		static void DrawPolygonPrimitiveForPlayer(PrimitiveMan& primitiveMan, int player, const Vector& centerPos, int color, const luabind::object& verticesTable);

		/// Schedule to draw a filled polygon primitive.
		/// @param primitiveMan A reference to PrimitiveMan, provided by Lua.
		/// @param startPos Start position of the primitive in Scene coordinates.
		/// @param color Color to draw primitive with.
		/// @param verticesTable A Lua table that contains the positions of the primitive's vertices, relative to the center position.
		static void DrawPolygonFillPrimitive(PrimitiveMan& primitiveMan, const Vector& startPos, int color, const luabind::object& verticesTable);

		/// Schedule to draw a filled polygon primitive visible only to a specified player.
		/// @param primitiveMan A reference to PrimitiveMan, provided by Lua.
		/// @param player Player screen to draw primitive on.
		/// @param startPos Start position of the primitive in Scene coordinates.
		/// @param color Color to draw primitive with.
		/// @param verticesTable A Lua table that contains the positions of the primitive's vertices, relative to the center position.
		static void DrawPolygonFillPrimitiveForPlayer(PrimitiveMan& primitiveMan, int player, const Vector& startPos, int color, const luabind::object& verticesTable);

		/// Schedules to draw multiple primitives of varying type with transparency enabled.
		/// @param primitiveMan A reference to PrimitiveMan, provided by Lua.
		/// @param transValue The transparency value the primitives should be drawn at. From 0 (opaque) to 100 (transparent).
		/// @param primitivesTable A Lua table of primitives to schedule drawing for.
		static void DrawPrimitivesWithTransparency(PrimitiveMan& primitiveMan, int transValue, const luabind::object& primitivesTable);

		/// Schedule to draw multiple primitives of varying type with blending enabled.
		/// @param primitiveMan A reference to PrimitiveMan, provided by Lua.
		/// @param blendMode The blending mode the primitives should be drawn with. See DrawBlendMode enumeration.
		/// @param blendAmount The blending amount for all the channels. 0-100.
		/// @param primitivesTable A Lua table of primitives to schedule drawing for.
		static void DrawPrimitivesWithBlending(PrimitiveMan& primitiveMan, int blendMode, int blendAmount, const luabind::object& primitivesTable);

		/// Schedule to draw multiple primitives of varying type with blending enabled.
		/// @param primitiveMan A reference to PrimitiveMan, provided by Lua.
		/// @param blendMode The blending mode the primitives should be drawn with. See DrawBlendMode enumeration.
		/// @param blendAmountR The blending amount for the Red channel. 0-100.
		/// @param blendAmountG The blending amount for the Green channel. 0-100.
		/// @param blendAmountB The blending amount for the Blue channel. 0-100.
		/// @param blendAmountA The blending amount for the Alpha channel. 0-100.
		/// @param primitivesTable A Lua table of primitives to schedule drawing for.
		static void DrawPrimitivesWithBlendingPerChannel(PrimitiveMan& primitiveMan, int blendMode, int blendAmountR, int blendAmountG, int blendAmountB, int blendAmountA, const luabind::object& primitivesTable);
	};
#pragma endregion

#pragma region Utility Lua Adapters
	struct LuaAdaptersUtility {
		/// Gets the ratio between the physics engine's meters and on-screen pixels.
		/// @return A float describing the current MPP ratio.
		static float GetMPP();

		/// Gets the ratio between on-screen pixels and the physics engine's meters.
		/// @return A float describing the current PPM ratio.
		static float GetPPM();

		/// Gets the ratio between the physics engine's Liters and on-screen pixels.
		/// @return A float describing the current LPP ratio.
		static float GetLPP();

		/// Gets the ratio between the on-screen pixels and the physics engine's Liters.
		/// @return A float describing the current PPL ratio.
		static float GetPPL();

		/// Gets the default pathfinder penetration value that'll allow pathing through corpses, debris, and such stuff.
		/// @return A float describing the default pathfinder penetration value.
		static float GetPathFindingDefaultDigStrength();

		/// Explicit deletion of any Entity instance that Lua owns. It will probably be handled by the GC, but this makes it instantaneous.
		/// @param entityToDelete The Entity to delete.
		static void DeleteEntity(Entity* entityToDelete);
	};
#pragma endregion
} // namespace RTE

#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif
