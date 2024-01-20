// Make sure that binding definition files are always set to NOT use pre-compiled headers and conformance mode (/permissive) otherwise everything will be on fire!

#include "LuaBindingRegisterDefinitions.h"

#include "LuaAdapterDefinitions.h"

#include "Entity.h"
#include "MovableObject.h"
#include "AtomGroup.h"
#include "MOSParticle.h"
#include "MOPixel.h"
#include "MOSprite.h"
#include "MOSRotating.h"
#include "Attachable.h"
#include "AEmitter.h"

namespace RTE {

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, Entity) {
		auto luaType = SimpleTypeLuaClassDefinition(Entity);

		luaType[sol::meta_function::to_string] = [](const Entity& obj) { std::stringstream stream; stream << obj; return stream.str(); };

		luaType["ClassName"] = sol::property(&Entity::GetClassName);
		luaType["PresetName"] = sol::property(&Entity::GetPresetName, &LuaAdaptersEntity::SetPresetName);
		luaType["Description"] = sol::property(&Entity::GetDescription, &Entity::SetDescription);
		luaType["IsOriginalPreset"] = sol::property(&Entity::IsOriginalPreset);
		luaType["ModuleID"] = sol::property(&Entity::GetModuleID);
		luaType["ModuleName"] = sol::property(&Entity::GetModuleName);
		luaType["RandomWeight"] = sol::property(&Entity::GetRandomWeight);
		luaType["Groups"] = sol::property(&Entity::GetGroups);

		luaType["Clone"] = &LuaAdaptersEntityClone::CloneEntity;
		luaType["Reset"] = &Entity::Reset;
		luaType["GetModuleAndPresetName"] = &Entity::GetModuleAndPresetName;
		luaType["AddToGroup"] = &Entity::AddToGroup;
		luaType["RemoveFromGroup"] = &Entity::RemoveFromGroup;
		luaType["IsInGroup"] = &Entity::IsInGroup;
	}

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, MOPixel) {
		auto luaType = ConcreteTypeLuaClassDefinition(MOPixel, MovableObject, SceneObject, Entity);

		luaType["TrailLength"] = sol::property(&MOPixel::GetTrailLength, &MOPixel::SetTrailLength);
		luaType["Staininess"] = sol::property(&MOPixel::GetStaininess, &MOPixel::SetStaininess);
	}

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, MOSParticle) {
		auto luaType = ConcreteTypeLuaClassDefinition(MOSParticle, MOSprite, MovableObject, SceneObject, Entity);
	}

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, MOSRotating) {
		auto luaType = ConcreteTypeLuaClassDefinition(MOSRotating, MOSprite, MovableObject, SceneObject, Entity);

		// luaType["Material"] = sol::property(&MOSRotating::GetMaterial);
		luaType["IndividualRadius"] = sol::property(&MOSRotating::GetIndividualRadius);
		luaType["IndividualDiameter"] = sol::property(&MOSRotating::GetIndividualDiameter);
		luaType["IndividualMass"] = sol::property(&MOSRotating::GetIndividualMass);
		luaType["RecoilForce"] = sol::property(&MOSRotating::GetRecoilForce);
		luaType["RecoilOffset"] = sol::property(&MOSRotating::GetRecoilOffset);
		luaType["TravelImpulse"] = sol::property(&MOSRotating::GetTravelImpulse, &MOSRotating::SetTravelImpulse);
		luaType["GibWoundLimit"] = sol::property((int(MOSRotating::*)() const) & MOSRotating::GetGibWoundLimit, &MOSRotating::SetGibWoundLimit);
		luaType["GibSound"] = sol::property(&MOSRotating::GetGibSound, &LuaAdaptersPropertyOwnershipSafetyFaker::MOSRotatingSetGibSound);
		luaType["GibImpulseLimit"] = sol::property(&MOSRotating::GetGibImpulseLimit, &MOSRotating::SetGibImpulseLimit);
		luaType["WoundCountAffectsImpulseLimitRatio"] = sol::property(&MOSRotating::GetWoundCountAffectsImpulseLimitRatio);
		luaType["GibAtEndOfLifetime"] = sol::property(&MOSRotating::GetGibAtEndOfLifetime, &MOSRotating::SetGibAtEndOfLifetime);
		luaType["DamageMultiplier"] = sol::property(&MOSRotating::GetDamageMultiplier, &MOSRotating::SetDamageMultiplier);
		luaType["WoundCount"] = sol::property((int(MOSRotating::*)() const) & MOSRotating::GetWoundCount);
		luaType["OrientToVel"] = sol::property(&MOSRotating::GetOrientToVel, &MOSRotating::SetOrientToVel);

		luaType["Attachables"] = sol::readonly(&MOSRotating::m_Attachables);
		luaType["Wounds"] = sol::readonly(&MOSRotating::m_Wounds);
		luaType["Gibs"] = sol::readonly(&MOSRotating::m_Gibs);

		luaType["AddRecoil"] = &MOSRotating::AddRecoil;
		luaType["SetRecoil"] = &MOSRotating::SetRecoil;
		luaType["IsRecoiled"] = &MOSRotating::IsRecoiled;
		luaType["EnableDeepCheck"] = &MOSRotating::EnableDeepCheck;
		luaType["ForceDeepCheck"] = &MOSRotating::ForceDeepCheck;
		luaType["GibThis"] = &MOSRotating::GibThis;
		luaType["MoveOutOfTerrain"] = &MOSRotating::MoveOutOfTerrain;
		luaType["FlashWhite"] = &MOSRotating::FlashWhite;
		luaType["GetGibWoundLimit"] = (int(MOSRotating::*)() const) & MOSRotating::GetGibWoundLimit;
		luaType["GetGibWoundLimit"] = (int(MOSRotating::*)(bool positiveDamage, bool negativeDamage, bool noDamage) const) & MOSRotating::GetGibWoundLimit;
		luaType["GetWoundCount"] = (int(MOSRotating::*)() const) & MOSRotating::GetWoundCount;
		luaType["GetWoundCount"] = (int(MOSRotating::*)(bool positiveDamage, bool negativeDamage, bool noDamage) const) & MOSRotating::GetWoundCount;
		luaType["GetWounds"] = &LuaAdaptersMOSRotating::GetWounds1;
		luaType["GetWounds"] = &LuaAdaptersMOSRotating::GetWounds2;
		luaType["AddWound"] = &MOSRotating::AddWound; //, luabind::adopt(_2);
		luaType["RemoveWounds"] = (float(MOSRotating::*)(int numberOfWoundsToRemove)) & MOSRotating::RemoveWounds;
		luaType["RemoveWounds"] = (float(MOSRotating::*)(int numberOfWoundsToRemove, bool positiveDamage, bool negativeDamage, bool noDamage)) & MOSRotating::RemoveWounds;
		luaType["IsOnScenePoint"] = &MOSRotating::IsOnScenePoint;
		luaType["EraseFromTerrain"] = &MOSRotating::EraseFromTerrain;
		luaType["GetStringValue"] = &MOSRotating::GetStringValue;
		luaType["GetNumberValue"] = &MOSRotating::GetNumberValue;
		luaType["GetObjectValue"] = &MOSRotating::GetObjectValue;
		luaType["SetStringValue"] = &MOSRotating::SetStringValue;
		luaType["SetNumberValue"] = &MOSRotating::SetNumberValue;
		luaType["SetObjectValue"] = &MOSRotating::SetObjectValue;
		luaType["RemoveStringValue"] = &MOSRotating::RemoveStringValue;
		luaType["RemoveNumberValue"] = &MOSRotating::RemoveNumberValue;
		luaType["RemoveObjectValue"] = &MOSRotating::RemoveObjectValue;
		luaType["StringValueExists"] = &MOSRotating::StringValueExists;
		luaType["NumberValueExists"] = &MOSRotating::NumberValueExists;
		luaType["ObjectValueExists"] = &MOSRotating::ObjectValueExists;
		luaType["AddAttachable"] = (void(MOSRotating::*)(Attachable * attachableToAdd)) & MOSRotating::AddAttachable; //, luabind::adopt(_2);
		luaType["AddAttachable"] = (void(MOSRotating::*)(Attachable * attachableToAdd, const Vector& parentOffset)) & MOSRotating::AddAttachable; //, luabind::adopt(_2);
		luaType["RemoveAttachable"] = (Attachable * (MOSRotating::*)(long uniqueIDOfAttachableToRemove)) & MOSRotating::RemoveAttachable; //, luabind::adopt(luabind::return_value);
		luaType["RemoveAttachable"] = (Attachable * (MOSRotating::*)(long uniqueIDOfAttachableToRemove, bool addToMovableMan, bool addBreakWounds)) & MOSRotating::RemoveAttachable; //, luabind::adopt(luabind::return_value);
		luaType["RemoveAttachable"] = (Attachable * (MOSRotating::*)(Attachable * attachableToRemove)) & MOSRotating::RemoveAttachable; //, luabind::adopt(luabind::return_value);
		luaType["RemoveAttachable"] = (Attachable * (MOSRotating::*)(Attachable * attachableToRemove, bool addToMovableMan, bool addBreakWounds)) & MOSRotating::RemoveAttachable;
		luaType["AddEmitter"] = (void(MOSRotating::*)(Attachable * attachableToAdd)) & MOSRotating::AddAttachable; //, luabind::adopt(_2);
		luaType["AddEmitter"] = (void(MOSRotating::*)(Attachable * attachableToAdd, const Vector& parentOffset)) & MOSRotating::AddAttachable; //, luabind::adopt(_2);
		luaType["RemoveEmitter"] = (Attachable * (MOSRotating::*)(long uniqueIDOfAttachableToRemove)) & MOSRotating::RemoveAttachable; //, luabind::adopt(luabind::return_value);
		luaType["RemoveEmitter"] = (Attachable * (MOSRotating::*)(long uniqueIDOfAttachableToRemove, bool addToMovableMan, bool addBreakWounds)) & MOSRotating::RemoveAttachable; //, luabind::adopt(luabind::return_value);
		luaType["RemoveEmitter"] = (Attachable * (MOSRotating::*)(Attachable * attachableToRemove)) & MOSRotating::RemoveAttachable; //, luabind::adopt(luabind::return_value);
		luaType["RemoveEmitter"] = (Attachable * (MOSRotating::*)(Attachable * attachableToRemove, bool addToMovableMan, bool addBreakWounds)) & MOSRotating::RemoveAttachable; //, luabind::adopt(luabind::return_value);

		luaType["GibThis"] = &LuaAdaptersMOSRotating::GibThis;
	}

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, MovableObject) {
		auto luaType = AbstractTypeLuaClassDefinition(MovableObject, SceneObject, Entity);

		luaType["Material"] = sol::property(&MovableObject::GetMaterial);
		luaType["Mass"] = sol::property(&MovableObject::GetMass, &MovableObject::SetMass);
		luaType["Pos"] = sol::property(&MovableObject::GetPos, &MovableObject::SetPos);
		luaType["Vel"] = sol::property(&MovableObject::GetVel, &MovableObject::SetVel);
		luaType["PrevPos"] = sol::property(&MovableObject::GetPrevPos);
		luaType["PrevVel"] = sol::property(&MovableObject::GetPrevVel);
		luaType["DistanceTravelled"] = sol::property(&MovableObject::GetDistanceTravelled);
		luaType["AngularVel"] = sol::property(&MovableObject::GetAngularVel, &MovableObject::SetAngularVel);
		luaType["Radius"] = sol::property(&MovableObject::GetRadius);
		luaType["Diameter"] = sol::property(&MovableObject::GetDiameter);
		luaType["Scale"] = sol::property(&MovableObject::GetScale, &MovableObject::SetScale);
		luaType["EffectRotAngle"] = sol::property(&MovableObject::GetEffectRotAngle, &MovableObject::SetEffectRotAngle);
		luaType["GlobalAccScalar"] = sol::property(&MovableObject::GetGlobalAccScalar, &MovableObject::SetGlobalAccScalar);
		luaType["AirResistance"] = sol::property(&MovableObject::GetAirResistance, &MovableObject::SetAirResistance);
		luaType["AirThreshold"] = sol::property(&MovableObject::GetAirThreshold, &MovableObject::SetAirThreshold);
		luaType["Age"] = sol::property(&MovableObject::GetAge, &MovableObject::SetAge);
		luaType["Lifetime"] = sol::property(&MovableObject::GetLifetime, &MovableObject::SetLifetime);
		luaType["ID"] = sol::property(&MovableObject::GetID);
		luaType["UniqueID"] = sol::property(&MovableObject::GetUniqueID);
		luaType["RootID"] = sol::property(&MovableObject::GetRootID);
		luaType["MOIDFootprint"] = sol::property(&MovableObject::GetMOIDFootprint);
		luaType["Sharpness"] = sol::property(&MovableObject::GetSharpness, &MovableObject::SetSharpness);
		luaType["HasEverBeenAddedToMovableMan"] = sol::property(&MovableObject::HasEverBeenAddedToMovableMan);
		luaType["AboveHUDPos"] = sol::property(&MovableObject::GetAboveHUDPos);
		luaType["HitsMOs"] = sol::property(&MovableObject::HitsMOs, &MovableObject::SetToHitMOs);
		luaType["GetsHitByMOs"] = sol::property(&MovableObject::GetsHitByMOs, &MovableObject::SetToGetHitByMOs);
		luaType["IgnoresTeamHits"] = sol::property(&MovableObject::IgnoresTeamHits, &MovableObject::SetIgnoresTeamHits);
		luaType["IgnoresWhichTeam"] = sol::property(&MovableObject::IgnoresWhichTeam);
		luaType["IgnoreTerrain"] = sol::property(&MovableObject::IgnoreTerrain, &MovableObject::SetIgnoreTerrain);
		luaType["IgnoresActorHits"] = sol::property(&MovableObject::GetIgnoresActorHits, &MovableObject::SetIgnoresActorHits);
		luaType["ToSettle"] = sol::property(&MovableObject::ToSettle, &MovableObject::SetToSettle);
		luaType["ToDelete"] = sol::property(&MovableObject::ToDelete, &MovableObject::SetToDelete);
		luaType["MissionCritical"] = sol::property(&MovableObject::IsMissionCritical, &MovableObject::SetMissionCritical);
		luaType["HUDVisible"] = sol::property(&MovableObject::GetHUDVisible, &MovableObject::SetHUDVisible);
		luaType["PinStrength"] = sol::property(&MovableObject::GetPinStrength, &MovableObject::SetPinStrength);
		luaType["RestThreshold"] = sol::property(&MovableObject::GetRestThreshold, &MovableObject::SetRestThreshold);
		luaType["DamageOnCollision"] = sol::property(&MovableObject::DamageOnCollision, &MovableObject::SetDamageOnCollision);
		luaType["DamageOnPenetration"] = sol::property(&MovableObject::DamageOnPenetration, &MovableObject::SetDamageOnPenetration);
		luaType["WoundDamageMultiplier"] = sol::property(&MovableObject::WoundDamageMultiplier, &MovableObject::SetWoundDamageMultiplier);
		luaType["HitWhatMOID"] = sol::property(&MovableObject::HitWhatMOID);
		luaType["HitWhatTerrMaterial"] = sol::property(&MovableObject::HitWhatTerrMaterial);
		luaType["HitWhatParticleUniqueID"] = sol::property(&MovableObject::HitWhatParticleUniqueID);
		luaType["ApplyWoundDamageOnCollision"] = sol::property(&MovableObject::GetApplyWoundDamageOnCollision, &MovableObject::SetApplyWoundDamageOnCollision);
		luaType["ApplyWoundBurstDamageOnCollision"] = sol::property(&MovableObject::GetApplyWoundBurstDamageOnCollision, &MovableObject::SetApplyWoundBurstDamageOnCollision);
		luaType["SimUpdatesBetweenScriptedUpdates"] = sol::property(&MovableObject::GetSimUpdatesBetweenScriptedUpdates, &MovableObject::SetSimUpdatesBetweenScriptedUpdates);

		luaType["GetParent"] = (MOSRotating * (MovableObject::*)()) & MovableObject::GetParent;
		luaType["GetParent"] = (const MOSRotating* (MovableObject::*)() const) & MovableObject::GetParent;
		luaType["GetRootParent"] = (MovableObject * (MovableObject::*)()) & MovableObject::GetRootParent;
		luaType["GetRootParent"] = (const MovableObject* (MovableObject::*)() const) & MovableObject::GetRootParent;
		luaType["ReloadScripts"] = &MovableObject::ReloadScripts;
		luaType["HasScript"] = &LuaAdaptersMovableObject::HasScript;
		luaType["AddScript"] = &LuaAdaptersMovableObject::AddScript;
		luaType["ScriptEnabled"] = &MovableObject::ScriptEnabled;
		luaType["EnableScript"] = &LuaAdaptersMovableObject::EnableScript;
		luaType["DisableScript"] = &LuaAdaptersMovableObject::DisableScript1;
		luaType["DisableScript"] = &LuaAdaptersMovableObject::DisableScript2;
		luaType["EnableOrDisableAllScripts"] = &MovableObject::EnableOrDisableAllScripts;
		luaType["GetStringValue"] = &MovableObject::GetStringValue;
		luaType["GetEncodedStringValue"] = &MovableObject::GetEncodedStringValue;
		luaType["GetNumberValue"] = &MovableObject::GetNumberValue;
		luaType["GetObjectValue"] = &MovableObject::GetObjectValue;
		luaType["SetStringValue"] = &MovableObject::SetStringValue;
		luaType["SetEncodedStringValue"] = &MovableObject::SetEncodedStringValue;
		luaType["SetNumberValue"] = &MovableObject::SetNumberValue;
		luaType["SetObjectValue"] = &MovableObject::SetObjectValue;
		luaType["RemoveStringValue"] = &MovableObject::RemoveStringValue;
		luaType["RemoveNumberValue"] = &MovableObject::RemoveNumberValue;
		luaType["RemoveObjectValue"] = &MovableObject::RemoveObjectValue;
		luaType["StringValueExists"] = &MovableObject::StringValueExists;
		luaType["NumberValueExists"] = &MovableObject::NumberValueExists;
		luaType["ObjectValueExists"] = &MovableObject::ObjectValueExists;
		luaType["GetAltitude"] = &MovableObject::GetAltitude;
		luaType["GetWhichMOToNotHit"] = &MovableObject::GetWhichMOToNotHit;
		luaType["SetWhichMOToNotHit"] = &MovableObject::SetWhichMOToNotHit;
		luaType["IsSetToDelete"] = &MovableObject::IsSetToDelete;
		luaType["IsMissionCritical"] = &MovableObject::IsMissionCritical;
		luaType["IsGeneric"] = &MovableObject::IsGeneric;
		luaType["IsActor"] = &MovableObject::IsActor;
		luaType["IsDevice"] = &MovableObject::IsDevice;
		luaType["IsHeldDevice"] = &MovableObject::IsHeldDevice;
		luaType["IsThrownDevice"] = &MovableObject::IsThrownDevice;
		luaType["IsGold"] = &MovableObject::IsGold;
		luaType["IsThrownDevice"] = &MovableObject::IsThrownDevice;
		luaType["HasObject"] = &MovableObject::HasObject;
		luaType["HasObjectInGroup"] = &MovableObject::HasObjectInGroup;
		luaType["AddForce"] = &MovableObject::AddForce;
		luaType["AddAbsForce"] = &MovableObject::AddAbsForce;
		luaType["AddImpulseForce"] = &MovableObject::AddImpulseForce;
		luaType["AddAbsImpulseForce"] = &MovableObject::AddAbsImpulseForce;
		luaType["ClearForces"] = &MovableObject::ClearForces;
		luaType["ClearImpulseForces"] = &MovableObject::ClearImpulseForces;
		luaType["GetForcesCount"] = &MovableObject::GetForcesCount;
		luaType["GetForceVector"] = &MovableObject::GetForceVector;
		luaType["GetForceOffset"] = &MovableObject::GetForceOffset;
		luaType["SetForceVector"] = &MovableObject::SetForceVector;
		luaType["SetForceOffset"] = &MovableObject::SetForceOffset;
		luaType["GetImpulsesCount"] = &MovableObject::GetImpulsesCount;
		luaType["GetImpulseVector"] = &MovableObject::GetImpulseVector;
		luaType["GetImpulseOffset"] = &MovableObject::GetImpulseOffset;
		luaType["SetImpulseVector"] = &MovableObject::SetImpulseVector;
		luaType["SetImpulseOffset"] = &MovableObject::SetImpulseOffset;
		luaType["RestDetection"] = &MovableObject::RestDetection;
		luaType["NotResting"] = &MovableObject::NotResting;
		luaType["IsAtRest"] = &MovableObject::IsAtRest;
		luaType["MoveOutOfTerrain"] = &MovableObject::MoveOutOfTerrain;
		luaType["RotateOffset"] = &MovableObject::RotateOffset;
		luaType["SendMessage"] = &LuaAdaptersMovableObject::SendMessage1;
		luaType["SendMessage"] = &LuaAdaptersMovableObject::SendMessage2;
		luaType["RequestSyncedUpdate"] = &MovableObject::RequestSyncedUpdate;
	}
} // namespace RTE
