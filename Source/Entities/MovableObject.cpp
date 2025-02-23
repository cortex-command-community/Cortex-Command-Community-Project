#include "MovableObject.h"

#include "ActivityMan.h"
#include "PresetMan.h"
#include "SceneMan.h"
#include "ConsoleMan.h"
#include "SettingsMan.h"
#include "LuaMan.h"
#include "Atom.h"
#include "Actor.h"
#include "SLTerrain.h"
#include "PieMenu.h"
#include "Serializable.h"
#include "System.h"
#include "PostProcessMan.h"

#include "Base64/base64.h"
#include "tracy/Tracy.hpp"

#include <array>

using namespace RTE;

AbstractClassInfo(MovableObject, SceneObject);

std::atomic<long> MovableObject::m_UniqueIDCounter = 1;
std::string MovableObject::ms_EmptyString = "";

MovableObject::MovableObject() {
	Clear();
}

MovableObject::~MovableObject() {
	Destroy(true);
}

void MovableObject::Clear() {
	m_MOType = TypeGeneric;
	m_Mass = 0;
	m_Vel.Reset();
	m_PrevPos.Reset();
	m_PrevVel.Reset();
	m_DistanceTravelled = 0;
	m_Scale = 1.0;
	m_GlobalAccScalar = 1.0;
	m_AirResistance = 0;
	m_AirThreshold = 5;
	m_PinStrength = 0;
	m_RestThreshold = 500;
	m_Forces.clear();
	m_ImpulseForces.clear();
	m_AgeTimer.Reset();
	m_RestTimer.Reset();
	m_Lifetime = 0;
	m_Sharpness = 1.0;
	//    m_MaterialId = 0;
	m_CheckTerrIntersection = false;
	m_HitsMOs = false;
	m_pMOToNotHit = 0;
	m_MOIgnoreTimer.Reset();
	m_GetsHitByMOs = false;
	m_IgnoresTeamHits = false;
	m_IgnoresAtomGroupHits = false;
	m_IgnoresAGHitsWhenSlowerThan = -1;
	m_IgnoresActorHits = false;
	m_MissionCritical = false;
	m_CanBeSquished = true;
	m_IsUpdated = false;
	m_WrapDoubleDraw = true;
	m_DidWrap = false;
	m_MOID = g_NoMOID;
	m_RootMOID = g_NoMOID;
	m_HasEverBeenAddedToMovableMan = false;
	m_MOIDFootprint = 0;
	m_AlreadyHitBy.clear();
	m_VelOscillations = 0;
	m_ToSettle = false;
	m_ToDelete = false;
	m_HUDVisible = true;
	m_IsTraveling = false;
	m_AllLoadedScripts.clear();
	m_FunctionsAndScripts.clear();
	m_StringValueMap.clear();
	m_NumberValueMap.clear();
	m_ObjectValueMap.clear();
	m_ThreadedLuaState = nullptr;
	m_ForceIntoMasterLuaState = false;
	m_ScriptObjectName.clear();
	m_ScreenEffectFile.Reset();
	m_pScreenEffect = 0;
	m_EffectRotAngle = 0;
	m_InheritEffectRotAngle = false;
	m_RandomizeEffectRotAngle = false;
	m_RandomizeEffectRotAngleEveryFrame = false;
	m_ScreenEffectHash = 0;
	m_EffectStartTime = 0;
	m_EffectStopTime = 0;
	m_EffectStartStrength = 128;
	m_EffectStopStrength = 128;
	m_EffectAlwaysShows = false;
	m_PostEffectEnabled = false;

	m_UniqueID = 0;

	m_RemoveOrphanTerrainRadius = 0;
	m_RemoveOrphanTerrainMaxArea = 0;
	m_RemoveOrphanTerrainRate = 0.0;
	m_DamageOnCollision = 0.0;
	m_DamageOnPenetration = 0.0;
	m_WoundDamageMultiplier = 1.0;
	m_ApplyWoundDamageOnCollision = false;
	m_ApplyWoundBurstDamageOnCollision = false;
	m_IgnoreTerrain = false;

	m_MOIDHit = g_NoMOID;
	m_TerrainMatHit = g_MaterialAir;
	m_ParticleUniqueIDHit = 0;

	m_LastCollisionSimFrameNumber = 0;

	m_SimUpdatesBetweenScriptedUpdates = 1;
	m_SimUpdatesSinceLastScriptedUpdate = 0;
	m_RequestedSyncedUpdate = false;
}

LuaStateWrapper& MovableObject::GetAndLockStateForScript(const std::string& scriptPath, const LuaFunction* function) {
	if (m_ForceIntoMasterLuaState) {
		m_ThreadedLuaState = &g_LuaMan.GetMasterScriptState();
	}

	if (m_ThreadedLuaState == nullptr) {
		m_ThreadedLuaState = g_LuaMan.GetAndLockFreeScriptState();
	} else {
		m_ThreadedLuaState->GetMutex().lock();
	}

	return *m_ThreadedLuaState;
}

int MovableObject::Create() {
	if (SceneObject::Create() < 0)
		return -1;

	m_AgeTimer.Reset();
	m_RestTimer.Reset();

	// If the stop time hasn't been assigned, just make the same as the life time.
	if (m_EffectStopTime <= 0)
		m_EffectStopTime = m_Lifetime;

	m_UniqueID = MovableObject::GetNextUniqueID();

	m_MOIDHit = g_NoMOID;
	m_TerrainMatHit = g_MaterialAir;
	m_ParticleUniqueIDHit = 0;

	g_MovableMan.RegisterObject(this);

	return 0;
}

int MovableObject::Create(const float mass,
                          const Vector& position,
                          const Vector& velocity,
                          float rotAngle,
                          float angleVel,
                          unsigned long lifetime,
                          bool hitMOs,
                          bool getHitByMOs) {
	m_Mass = mass;
	m_Pos = position;
	m_Vel = velocity;
	m_AgeTimer.Reset();
	m_RestTimer.Reset();
	m_Lifetime = lifetime;
	//    m_MaterialId = matId;
	m_HitsMOs = hitMOs;
	m_GetsHitByMOs = getHitByMOs;

	m_UniqueID = MovableObject::GetNextUniqueID();

	m_MOIDHit = g_NoMOID;
	m_TerrainMatHit = g_MaterialAir;
	m_ParticleUniqueIDHit = 0;

	g_MovableMan.RegisterObject(this);

	return 0;
}

int MovableObject::Create(const MovableObject& reference) {
	SceneObject::Create(reference);

	m_MOType = reference.m_MOType;
	m_Mass = reference.m_Mass;
	m_Pos = reference.m_Pos;
	m_Vel = reference.m_Vel;
	m_Scale = reference.m_Scale;
	m_GlobalAccScalar = reference.m_GlobalAccScalar;
	m_AirResistance = reference.m_AirResistance;
	m_AirThreshold = reference.m_AirThreshold;
	m_PinStrength = reference.m_PinStrength;
	m_RestThreshold = reference.m_RestThreshold;
	//    m_Force = reference.m_Force;
	//    m_ImpulseForce = reference.m_ImpulseForce;
	// Should reset age instead??
	//    m_AgeTimer = reference.m_AgeTimer;
	m_AgeTimer.Reset();
	m_RestTimer.Reset();
	m_Lifetime = reference.m_Lifetime;
	m_Sharpness = reference.m_Sharpness;
	//    m_MaterialId = reference.m_MaterialId;
	m_CheckTerrIntersection = reference.m_CheckTerrIntersection;
	m_HitsMOs = reference.m_HitsMOs;
	m_GetsHitByMOs = reference.m_GetsHitByMOs;
	m_IgnoresTeamHits = reference.m_IgnoresTeamHits;
	m_IgnoresAtomGroupHits = reference.m_IgnoresAtomGroupHits;
	m_IgnoresAGHitsWhenSlowerThan = reference.m_IgnoresAGHitsWhenSlowerThan;
	m_IgnoresActorHits = reference.m_IgnoresActorHits;
	m_pMOToNotHit = reference.m_pMOToNotHit;
	m_MOIgnoreTimer = reference.m_MOIgnoreTimer;
	m_MissionCritical = reference.m_MissionCritical;
	m_CanBeSquished = reference.m_CanBeSquished;
	m_HUDVisible = reference.m_HUDVisible;
	m_PostEffectEnabled = reference.m_PostEffectEnabled;

	m_ForceIntoMasterLuaState = reference.m_ForceIntoMasterLuaState;
	for (auto& [scriptPath, scriptEnabled]: reference.m_AllLoadedScripts) {
		LoadScript(scriptPath, scriptEnabled);
	}

	if (reference.m_pScreenEffect) {
		m_ScreenEffectFile = reference.m_ScreenEffectFile;
		m_pScreenEffect = m_ScreenEffectFile.GetAsBitmap();
	}
	m_EffectRotAngle = reference.m_EffectRotAngle;
	m_InheritEffectRotAngle = reference.m_InheritEffectRotAngle;
	m_RandomizeEffectRotAngle = reference.m_RandomizeEffectRotAngle;
	m_RandomizeEffectRotAngleEveryFrame = reference.m_RandomizeEffectRotAngleEveryFrame;

	if (m_RandomizeEffectRotAngle)
		m_EffectRotAngle = c_PI * RandomNum(-2.0F, 2.0F);

	m_ScreenEffectHash = reference.m_ScreenEffectHash;
	m_EffectStartTime = reference.m_EffectStartTime;
	m_EffectStopTime = reference.m_EffectStopTime;
	m_EffectStartStrength = reference.m_EffectStartStrength;
	m_EffectStopStrength = reference.m_EffectStopStrength;
	m_EffectAlwaysShows = reference.m_EffectAlwaysShows;
	m_RemoveOrphanTerrainRadius = reference.m_RemoveOrphanTerrainRadius;
	m_RemoveOrphanTerrainMaxArea = reference.m_RemoveOrphanTerrainMaxArea;
	m_RemoveOrphanTerrainRate = reference.m_RemoveOrphanTerrainRate;
	m_DamageOnCollision = reference.m_DamageOnCollision;
	m_DamageOnPenetration = reference.m_DamageOnPenetration;
	m_WoundDamageMultiplier = reference.m_WoundDamageMultiplier;
	m_IgnoreTerrain = reference.m_IgnoreTerrain;

	m_MOIDHit = reference.m_MOIDHit;
	m_TerrainMatHit = reference.m_TerrainMatHit;
	m_ParticleUniqueIDHit = reference.m_ParticleUniqueIDHit;

	m_SimUpdatesBetweenScriptedUpdates = reference.m_SimUpdatesBetweenScriptedUpdates;
	m_SimUpdatesSinceLastScriptedUpdate = reference.m_SimUpdatesSinceLastScriptedUpdate;

	m_StringValueMap = reference.m_StringValueMap;
	m_NumberValueMap = reference.m_NumberValueMap;
	m_ObjectValueMap = reference.m_ObjectValueMap;

	m_UniqueID = MovableObject::GetNextUniqueID();
	g_MovableMan.RegisterObject(this);

	return 0;
}

int MovableObject::ReadProperty(const std::string_view& propName, Reader& reader) {
	StartPropertyList(return SceneObject::ReadProperty(propName, reader));

	MatchProperty("Mass", { reader >> m_Mass; });
	MatchProperty("Velocity", { reader >> m_Vel; });
	MatchProperty("Scale", { reader >> m_Scale; });
	MatchProperty("GlobalAccScalar", { reader >> m_GlobalAccScalar; });
	MatchProperty("AirResistance", {
		reader >> m_AirResistance;
		// Backwards compatibility after we made this value scaled over time
		m_AirResistance /= 0.01666F;
	});
	MatchProperty("AirThreshold", { reader >> m_AirThreshold; });
	MatchProperty("PinStrength", { reader >> m_PinStrength; });
	MatchProperty("RestThreshold", { reader >> m_RestThreshold; });
	MatchProperty("LifeTime", { reader >> m_Lifetime; });
	MatchProperty("Age", {
		double age;
		reader >> age;
		m_AgeTimer.SetElapsedSimTimeMS(age);
	});
	MatchProperty("Sharpness", { reader >> m_Sharpness; });
	MatchProperty("HitsMOs", { reader >> m_HitsMOs; });
	MatchProperty("GetsHitByMOs", { reader >> m_GetsHitByMOs; });
	MatchProperty("IgnoresTeamHits", { reader >> m_IgnoresTeamHits; });
	MatchProperty("IgnoresAtomGroupHits", { reader >> m_IgnoresAtomGroupHits; });
	MatchProperty("IgnoresAGHitsWhenSlowerThan", { reader >> m_IgnoresAGHitsWhenSlowerThan; });
	MatchProperty("IgnoresActorHits", { reader >> m_IgnoresActorHits; });
	MatchProperty("RemoveOrphanTerrainRadius",
	              {
		              reader >> m_RemoveOrphanTerrainRadius;
		              if (m_RemoveOrphanTerrainRadius > MAXORPHANRADIUS)
			              m_RemoveOrphanTerrainRadius = MAXORPHANRADIUS;
	              });
	MatchProperty("RemoveOrphanTerrainMaxArea",
	              {
		              reader >> m_RemoveOrphanTerrainMaxArea;
		              if (m_RemoveOrphanTerrainMaxArea > MAXORPHANRADIUS * MAXORPHANRADIUS)
			              m_RemoveOrphanTerrainMaxArea = MAXORPHANRADIUS * MAXORPHANRADIUS;
	              });
	MatchProperty("RemoveOrphanTerrainRate", { reader >> m_RemoveOrphanTerrainRate; });
	MatchProperty("MissionCritical", { reader >> m_MissionCritical; });
	MatchProperty("CanBeSquished", { reader >> m_CanBeSquished; });
	MatchProperty("HUDVisible", { reader >> m_HUDVisible; });
	MatchProperty("_ClearScriptPaths", {  });
	MatchForwards("ScriptPath") MatchProperty("_AddScriptPath", {
		std::string scriptPath = g_PresetMan.GetFullModulePath(reader.ReadPropValue());
		switch (LoadScript(scriptPath)) {
			case 0:
				break;
			case -1:
				reader.ReportError("The script path " + scriptPath + " was empty.");
				break;
			case -2:
				reader.ReportError("The script path " + scriptPath + " did not point to a valid file.");
				break;
			case -3:
				reader.ReportError("The script path " + scriptPath + " is already loaded onto this object.");
				break;
			case -4:
				// Error in lua file, this'll pop up in the console so no need to report an error through the reader.
				break;
			default:
				RTEAbort("Reached default case while adding script in INI. This should never happen!");
				break;
		}
	});
	MatchProperty("ScreenEffect", {
		reader >> m_ScreenEffectFile;
		m_pScreenEffect = m_ScreenEffectFile.GetAsBitmap();
		m_ScreenEffectHash = m_ScreenEffectFile.GetHash();
	});
	MatchProperty("PostEffectEnabled", { reader >> m_PostEffectEnabled; });
	MatchProperty("EffectStartTime", { reader >> m_EffectStartTime; });
	MatchProperty("EffectRotAngle", { reader >> m_EffectRotAngle; });
	MatchProperty("InheritEffectRotAngle", { reader >> m_InheritEffectRotAngle; });
	MatchProperty("RandomizeEffectRotAngle", { reader >> m_RandomizeEffectRotAngle; });
	MatchProperty("RandomizeEffectRotAngleEveryFrame", { reader >> m_RandomizeEffectRotAngleEveryFrame; });
	MatchProperty("EffectStopTime", {
		reader >> m_EffectStopTime;
		m_EffectStopTime = std::max(m_EffectStopTime, static_cast<int>(g_TimerMan.GetDeltaTimeMS()) + 1);
	});
	MatchProperty("EffectStartStrength", {
		float strength;
		reader >> strength;
		m_EffectStartStrength = std::floor((float)255 * strength);
	});
	MatchProperty("EffectStopStrength", {
		float strength;
		reader >> strength;
		m_EffectStopStrength = std::floor((float)255 * strength);
	});
	MatchProperty("EffectAlwaysShows", { reader >> m_EffectAlwaysShows; });
	MatchProperty("DamageOnCollision", { reader >> m_DamageOnCollision; });
	MatchProperty("DamageOnPenetration", { reader >> m_DamageOnPenetration; });
	MatchProperty("WoundDamageMultiplier", { reader >> m_WoundDamageMultiplier; });
	MatchProperty("ApplyWoundDamageOnCollision", { reader >> m_ApplyWoundDamageOnCollision; });
	MatchProperty("ApplyWoundBurstDamageOnCollision", { reader >> m_ApplyWoundBurstDamageOnCollision; });
	MatchProperty("IgnoreTerrain", { reader >> m_IgnoreTerrain; });
	MatchProperty("SimUpdatesBetweenScriptedUpdates", { reader >> m_SimUpdatesBetweenScriptedUpdates; });
	MatchProperty("_ClearCustomNumberValues", { m_NumberValueMap.clear(); });
	MatchProperty("_ClearCustomStringValues", { m_StringValueMap.clear(); });
	MatchForwards("AddCustomValue") MatchProperty("_AddCustomValue", { ReadCustomValueProperty(reader); });
	MatchProperty("ForceIntoMasterLuaState", { reader >> m_ForceIntoMasterLuaState; });

	EndPropertyList;
}

void MovableObject::ReadCustomValueProperty(Reader& reader) {
	std::string customValueType;
	reader >> customValueType;
	std::string customKey = reader.ReadPropName();
	std::string customValue = reader.ReadPropValue();
	if (customValueType == "NumberValue") {
		try {
			SetNumberValue(customKey, std::stod(customValue));
		} catch (const std::invalid_argument) {
			reader.ReportError("Tried to read a non-number value for SetNumberValue.");
		}
	} else if (customValueType == "StringValue") {
		SetStringValue(customKey, customValue);
	} else {
		reader.ReportError("Invalid CustomValue type " + customValueType);
	}
	// Artificially end reading this property since we got all we needed
	reader.NextProperty();
}

int MovableObject::Save(Writer& writer) const {
	SceneObject::Save(writer);
	// TODO: Make proper save system that knows not to save redundant data!
	// Note - this function isn't even called when saving a scene. Turns out that scene special-cases this stuff, see Scene::Save()
	// In future, perhaps we ought to not do that. Who knows?

	if (m_Mass != 0) writer.NewPropertyWithValue("Mass", m_Mass);
	if (!m_Vel.IsZero()) writer.NewPropertyWithValue("Velocity", m_Vel);
	if (m_Scale != 1.0F) writer.NewPropertyWithValue("Scale", m_Scale);
	if (m_GlobalAccScalar != 1.0F) writer.NewPropertyWithValue("GlobalAccScalar", m_GlobalAccScalar);
	if (m_AirResistance != 0.0F) writer.NewPropertyWithValue("AirResistance", m_AirResistance * 0.01666F); // Backwards compatibility after we made this value scaled over time
	if (m_AirThreshold != 0.0F) writer.NewPropertyWithValue("AirThreshold", m_AirThreshold);
	if (m_PinStrength != 0.0F) writer.NewPropertyWithValue("PinStrength", m_PinStrength);
	if (m_RestThreshold != 500) writer.NewPropertyWithValue("RestThreshold", m_RestThreshold);
	if (m_Lifetime != 0.0F) writer.NewPropertyWithValue("LifeTime", m_Lifetime);
	if (m_Sharpness != 1.0F) writer.NewPropertyWithValue("Sharpness", m_Sharpness);
	if (m_HitsMOs != false) writer.NewPropertyWithValue("HitsMOs", m_HitsMOs);
	if (m_GetsHitByMOs != false) writer.NewPropertyWithValue("GetsHitByMOs", m_GetsHitByMOs);
	if (m_IgnoresTeamHits != false) writer.NewPropertyWithValue("IgnoresTeamHits", m_IgnoresTeamHits);
	if (m_IgnoresAtomGroupHits != false) writer.NewPropertyWithValue("IgnoresAtomGroupHits", m_IgnoresAtomGroupHits);
	if (m_IgnoresAGHitsWhenSlowerThan != -1.0F) writer.NewPropertyWithValue("IgnoresAGHitsWhenSlowerThan", m_IgnoresAGHitsWhenSlowerThan);
	if (m_IgnoresActorHits != false) writer.NewPropertyWithValue("IgnoresActorHits", m_IgnoresActorHits);
	if (m_MissionCritical != false) writer.NewPropertyWithValue("MissionCritical", m_MissionCritical);
	if (m_CanBeSquished != true) writer.NewPropertyWithValue("CanBeSquished", m_CanBeSquished);
	if (m_HUDVisible != true) writer.NewPropertyWithValue("HUDVisible", m_HUDVisible);

	for (const auto& [scriptPath, scriptEnabled]: m_AllLoadedScripts) {
		if (!scriptPath.empty()) {
			writer.NewPropertyWithValue("ScriptPath", scriptPath);
		}
	}

	if (!m_ScreenEffectFile.GetDataPath().empty()) writer.NewPropertyWithValue("ScreenEffect", m_ScreenEffectFile);
	if (m_PostEffectEnabled != false) writer.NewPropertyWithValue("PostEffectEnabled", m_PostEffectEnabled);
	if (m_EffectStartTime != 0) writer.NewPropertyWithValue("EffectStartTime", m_EffectStartTime);
	if (m_EffectStopTime != 0) writer.NewPropertyWithValue("EffectStopTime", m_EffectStopTime);
	if (m_EffectStartStrength != 128) writer.NewPropertyWithValue("EffectStartStrength", (float)m_EffectStartStrength / 255.0f);
	if (m_EffectStopStrength != 128) writer.NewPropertyWithValue("EffectStopStrength", (float)m_EffectStopStrength / 255.0f);
	if (m_EffectAlwaysShows != false) writer.NewPropertyWithValue("EffectAlwaysShows", m_EffectAlwaysShows);
	if (m_DamageOnCollision != 0.0F) writer.NewPropertyWithValue("DamageOnCollision", m_DamageOnCollision);
	if (m_DamageOnPenetration != 0.0F) writer.NewPropertyWithValue("DamageOnPenetration", m_DamageOnPenetration);
	if (m_WoundDamageMultiplier != 1.0F) writer.NewPropertyWithValue("WoundDamageMultiplier", m_WoundDamageMultiplier);
	if (m_ApplyWoundDamageOnCollision != false) writer.NewPropertyWithValue("ApplyWoundDamageOnCollision", m_ApplyWoundDamageOnCollision);
	if (m_ApplyWoundBurstDamageOnCollision != false) writer.NewPropertyWithValue("ApplyWoundBurstDamageOnCollision", m_ApplyWoundBurstDamageOnCollision);
	if (m_IgnoreTerrain != false) writer.NewPropertyWithValue("IgnoreTerrain", m_IgnoreTerrain);
	if (m_SimUpdatesBetweenScriptedUpdates != 1) writer.NewPropertyWithValue("SimUpdatesBetweenScriptedUpdates", m_SimUpdatesBetweenScriptedUpdates);

	for (const auto& [key, value]: m_NumberValueMap) {
		writer.NewLine();
		writer.ObjectStart("AddCustomValue = NumberValue");
		writer.NewPropertyWithValue(key, value);
		writer.ObjectEnd();
	}

	for (const auto& [key, value]: m_StringValueMap) {
		writer.NewLine();
		writer.ObjectStart("AddCustomValue = StringValue");
		writer.NewPropertyWithValue(key, value);
		writer.ObjectEnd();
	}

	if (m_ForceIntoMasterLuaState != false) writer.NewPropertyWithValue("ForceIntoMasterLuaState", m_ForceIntoMasterLuaState);

	return 0;
}

size_t MovableObject::Write(Writer& writer, const Entity& entityReference, const HashingData& hashData) const {
	size_t constituentsConsumed = SceneObject::Write(writer, entityReference, hashData);
	// TODO: Make proper save system that knows not to save redundant data!
	// Note - this function isn't even called when saving a scene. Turns out that scene special-cases this stuff, see Scene::Save()
	// In future, perhaps we ought to not do that. Who knows?

	const MOSprite& reference = static_cast<const MOSprite&>(entityReference);

	if (m_Mass != reference.m_Mass)
		writer.NewPropertyWithValue("Mass", m_Mass);
	if (m_Vel != reference.m_Vel)
		writer.NewPropertyWithValue("Velocity", m_Vel);
	if (m_Scale != reference.m_Scale)
		writer.NewPropertyWithValue("Scale", m_Scale);
	if (m_GlobalAccScalar != reference.m_GlobalAccScalar)
		writer.NewPropertyWithValue("GlobalAccScalar", m_GlobalAccScalar);
	if (m_AirResistance != reference.m_AirResistance)
		writer.NewPropertyWithValue("AirResistance", m_AirResistance * 0.01666F); // Backwards compatibility after we made this value scaled over time
	if (m_AirThreshold != reference.m_AirThreshold)
		writer.NewPropertyWithValue("AirThreshold", m_AirThreshold);
	if (m_PinStrength != reference.m_PinStrength)
		writer.NewPropertyWithValue("PinStrength", m_PinStrength);
	if (m_RestThreshold != reference.m_RestThreshold)
		writer.NewPropertyWithValue("RestThreshold", m_RestThreshold);
	if (m_Lifetime != reference.m_Lifetime)
		writer.NewPropertyWithValue("LifeTime", m_Lifetime);
	if (m_Sharpness != reference.m_Sharpness)
		writer.NewPropertyWithValue("Sharpness", m_Sharpness);
	if (m_HitsMOs != reference.m_HitsMOs)
		writer.NewPropertyWithValue("HitsMOs", m_HitsMOs);
	if (m_GetsHitByMOs != reference.m_GetsHitByMOs)
		writer.NewPropertyWithValue("GetsHitByMOs", m_GetsHitByMOs);
	if (m_IgnoresTeamHits != reference.m_IgnoresTeamHits)
		writer.NewPropertyWithValue("IgnoresTeamHits", m_IgnoresTeamHits);
	if (m_IgnoresAtomGroupHits != reference.m_IgnoresAtomGroupHits)
		writer.NewPropertyWithValue("IgnoresAtomGroupHits", m_IgnoresAtomGroupHits);
	if (m_IgnoresAGHitsWhenSlowerThan != reference.m_IgnoresAGHitsWhenSlowerThan)
		writer.NewPropertyWithValue("IgnoresAGHitsWhenSlowerThan", m_IgnoresAGHitsWhenSlowerThan);
	if (m_IgnoresActorHits != reference.m_IgnoresActorHits)
		writer.NewPropertyWithValue("IgnoresActorHits", m_IgnoresActorHits);
	if (m_MissionCritical != reference.m_MissionCritical)
		writer.NewPropertyWithValue("MissionCritical", m_MissionCritical);
	if (m_CanBeSquished != reference.m_CanBeSquished)
		writer.NewPropertyWithValue("CanBeSquished", m_CanBeSquished);
	if (m_HUDVisible != reference.m_HUDVisible)
		writer.NewPropertyWithValue("HUDVisible", m_HUDVisible);

	/* TODO: Decide whether to delete this (clear script paths currently does nothing)
	if (reference.m_AllLoadedScripts.size() > 0) {
		writer.NewPropertyWithValue("_ClearScriptPaths", 1);
	}

	for (const auto& [scriptPath, scriptEnabled]: m_AllLoadedScripts) {
		if (!scriptPath.empty()) {
			writer.NewPropertyWithValue("_AddScriptPath", scriptPath);
		}
	}
	*/

	for (const auto& [scriptPath, scriptEnabled]: m_AllLoadedScripts) {
		if (!scriptPath.empty() && !reference.HasScript(scriptPath)) {
			writer.NewPropertyWithValue("_AddScriptPath", scriptPath);
		}
	}

	if (m_ScreenEffectFile.Hash().m_Hash != hashData.m_Constituents.at(constituentsConsumed++))
		writer.NewPropertyWithValue("ScreenEffect", m_ScreenEffectFile);

	if (m_PostEffectEnabled != reference.m_PostEffectEnabled)
		writer.NewPropertyWithValue("PostEffectEnabled", m_PostEffectEnabled);
	if (m_EffectStartTime != reference.m_EffectStartTime)
		writer.NewPropertyWithValue("EffectStartTime", m_EffectStartTime);
	if (m_EffectStopTime != reference.m_EffectStopTime)
		writer.NewPropertyWithValue("EffectStopTime", m_EffectStopTime);
	if (m_EffectStartStrength != reference.m_EffectStartStrength)
		writer.NewPropertyWithValue("EffectStartStrength", (float)m_EffectStartStrength / 255.0f);
	if (m_EffectStopStrength != reference.m_EffectStopStrength)
		writer.NewPropertyWithValue("EffectStopStrength", (float)m_EffectStopStrength / 255.0f);
	if (m_EffectAlwaysShows != reference.m_EffectAlwaysShows)
		writer.NewPropertyWithValue("EffectAlwaysShows", m_EffectAlwaysShows);
	if (m_DamageOnCollision != reference.m_DamageOnCollision)
		writer.NewPropertyWithValue("DamageOnCollision", m_DamageOnCollision);
	if (m_DamageOnPenetration != reference.m_DamageOnPenetration)
		writer.NewPropertyWithValue("DamageOnPenetration", m_DamageOnPenetration);
	if (m_WoundDamageMultiplier != reference.m_WoundDamageMultiplier)
		writer.NewPropertyWithValue("WoundDamageMultiplier", m_WoundDamageMultiplier);
	if (m_ApplyWoundDamageOnCollision != reference.m_ApplyWoundDamageOnCollision)
		writer.NewPropertyWithValue("ApplyWoundDamageOnCollision", m_ApplyWoundDamageOnCollision);
	if (m_ApplyWoundBurstDamageOnCollision != reference.m_ApplyWoundBurstDamageOnCollision)
		writer.NewPropertyWithValue("ApplyWoundBurstDamageOnCollision", m_ApplyWoundBurstDamageOnCollision);
	if (m_IgnoreTerrain != reference.m_IgnoreTerrain)
		writer.NewPropertyWithValue("IgnoreTerrain", m_IgnoreTerrain);
	if (m_SimUpdatesBetweenScriptedUpdates != reference.m_SimUpdatesBetweenScriptedUpdates)
		writer.NewPropertyWithValue("SimUpdatesBetweenScriptedUpdates", m_SimUpdatesBetweenScriptedUpdates);

	if (!reference.m_NumberValueMap.empty()) {
		writer.NewPropertyWithValue("_ClearCustomNumberValues", "1");
	}

	if (!reference.m_StringValueMap.empty()) {
		writer.NewPropertyWithValue("_ClearCustomStringValues", "1");
	}

	for (const auto& [key, value]: m_NumberValueMap) {
		writer.NewLine();
		writer.ObjectStart("_AddCustomValue = NumberValue");
		writer.NewPropertyWithValue(key, value);
		writer.ObjectEnd();
	}

	for (const auto& [key, value]: m_StringValueMap) {
		writer.NewLine();
		writer.ObjectStart("_AddCustomValue = StringValue");
		writer.NewPropertyWithValue(key, value);
		writer.ObjectEnd();
	}

	if (m_ForceIntoMasterLuaState != reference.m_ForceIntoMasterLuaState)
		writer.NewPropertyWithValue("ForceIntoMasterLuaState", m_ForceIntoMasterLuaState);

	return constituentsConsumed;
}

HashingData MovableObject::Hash() const {
	HashingData hashData = SceneObject::Hash();
	uint64_t& hash = hashData.m_Hash;

	// TODO: These are all written under the assumption that the reading and writing functions are already approximately accurate.
	// If the write function doesn't save things correctly, this got it's distinction criteria from that, so it's wrong too.
	// I guess that's not much of a todo, more like a keep in mind, but you could look at it as an ongoing task.

	hash ^= std::hash<float>{}(m_Mass) << 1;
	hash ^= m_Vel.Hash().m_Hash << 2;
	hash ^= std::hash<float>{}(m_Scale) << 3;
	hash ^= std::hash<float>{}(m_GlobalAccScalar) << 4;
	hash ^= std::hash<float>{}(m_AirResistance) << 5;
	hash ^= std::hash<float>{}(m_AirThreshold) << 6;
	hash ^= std::hash<float>{}(m_PinStrength) << 7;
	hash ^= std::hash<int>{}(m_RestThreshold) << 8;
	hash ^= std::hash<unsigned long>{}(m_Lifetime) << 9;
	hash ^= std::hash<float>{}(m_Sharpness) << 10;
	hash ^= std::hash<bool>{}(m_HitsMOs) << 11;
	hash ^= std::hash<bool>{}(m_GetsHitByMOs) << 12;
	hash ^= std::hash<bool>{}(m_IgnoresTeamHits) << 13;
	hash ^= std::hash<bool>{}(m_IgnoresAtomGroupHits) << 14;
	hash ^= std::hash<float>{}(m_IgnoresAGHitsWhenSlowerThan) << 15;
	hash ^= std::hash<bool>{}(m_IgnoresActorHits) << 0;
	hash ^= std::hash<bool>{}(m_MissionCritical) << 1;
	hash ^= std::hash<bool>{}(m_CanBeSquished) << 2;
	hash ^= std::hash<bool>{}(m_HUDVisible) << 3;

	int i = 0;

	for (const auto& [scriptPath, scriptEnabled]: m_AllLoadedScripts) {
		if (!scriptPath.empty()) {
			hash ^= RTE::Hash(scriptPath) << (i++ % sizeof(uint64_t) * 8);
		}
	}

	uint64_t effectHash = m_ScreenEffectFile.Hash().m_Hash;
	hashData.m_Constituents.push_back(effectHash);
	hash ^= effectHash << 4;

	hash ^= std::hash<bool>{}(m_PostEffectEnabled) << 5;
	hash ^= std::hash<int>{}(m_EffectStartTime) << 6;
	hash ^= std::hash<int>{}(m_EffectStopTime) << 7;
	hash ^= std::hash<float>{}(m_EffectStartStrength) << 8;
	hash ^= std::hash<float>{}(m_EffectStopStrength) << 9;
	hash ^= std::hash<bool>{}(m_EffectAlwaysShows) << 10;
	hash ^= std::hash<float>{}(m_DamageOnCollision) << 11;
	hash ^= std::hash<float>{}(m_DamageOnPenetration) << 12;
	hash ^= std::hash<float>{}(m_WoundDamageMultiplier) << 13;
	hash ^= std::hash<bool>{}(m_ApplyWoundDamageOnCollision) << 14;
	hash ^= std::hash<bool>{}(m_ApplyWoundBurstDamageOnCollision) << 15;
	hash ^= std::hash<bool>{}(m_IgnoreTerrain) << 0;
	hash ^= std::hash<int>{}(m_SimUpdatesBetweenScriptedUpdates) << 1;

	for (const auto& [key, value]: m_NumberValueMap) {
		hash ^= RTE::Hash(key) << 16;
		hash ^= std::hash<double>{}(value) << 17;
	}

	for (const auto& [key, value]: m_StringValueMap) {
		hash ^= RTE::Hash(key) << 32;
		hash ^= RTE::Hash(value) << 33;
	}

	hash ^= std::hash<bool>{}(m_ForceIntoMasterLuaState) << 2;

	return hashData;
}

void MovableObject::DestroyScriptState() {
	if (m_ThreadedLuaState) {
		std::lock_guard<std::recursive_mutex> lock(m_ThreadedLuaState->GetMutex());

		if (ObjectScriptsInitialized()) {
			RunScriptedFunctionInAppropriateScripts("Destroy");
			m_ThreadedLuaState->RunScriptString(m_ScriptObjectName + " = nil;");
			m_ScriptObjectName.clear();
		}

		m_ThreadedLuaState->UnregisterMO(this);
		m_ThreadedLuaState = nullptr;
	}
}

void MovableObject::Destroy(bool notInherited) {
	// Unfortunately, shit can still get destroyed at random from Lua states having ownership and their GC deciding to delete it.
	// This skips the DestroyScriptState call... so there's leftover stale script state that we just can't do shit about.
	// This means Destroy() doesn't get called, and the lua memory shit leaks because it never gets set to nil. But oh well.
	// So.. we need to do this shit... I guess. Even though it's fucking awful. And it definitely results in possible deadlocks depending on how different lua states interact.
	// TODO: try to make this at least reasonably workable
	// DestroyScriptState();

	if (m_ThreadedLuaState) {
		m_ThreadedLuaState->UnregisterMO(this);
	}

	g_MovableMan.UnregisterObject(this);
	if (!notInherited) {
		SceneObject::Destroy();
	}
	Clear();
}

int MovableObject::LoadScript(const std::string& scriptPath, bool loadAsEnabledScript) {
	if (scriptPath.empty()) {
		return -1;
	} else if (!System::PathExistsCaseSensitive(scriptPath)) {
		return -2;
	} else if (HasScript(scriptPath)) {
		return -3;
	}

	LuaStateWrapper& usedState = GetAndLockStateForScript(scriptPath);
	std::lock_guard<std::recursive_mutex> lock(usedState.GetMutex(), std::adopt_lock);

	for (const std::string& functionName: GetSupportedScriptFunctionNames()) {
		if (m_FunctionsAndScripts.find(functionName) == m_FunctionsAndScripts.end()) {
			m_FunctionsAndScripts.try_emplace(functionName);
		}
	}

	m_AllLoadedScripts.try_emplace(scriptPath, loadAsEnabledScript);

	std::unordered_map<std::string, LuabindObjectWrapper*> scriptFileFunctions;
	if (usedState.RunScriptFileAndRetrieveFunctions(scriptPath, GetSupportedScriptFunctionNames(), scriptFileFunctions) < 0) {
		return -4;
	}

	for (const auto& [functionName, functionObject]: scriptFileFunctions) {
		LuaFunction& luaFunction = m_FunctionsAndScripts.at(functionName).emplace_back();
		luaFunction.m_ScriptIsEnabled = loadAsEnabledScript;
		luaFunction.m_LuaFunction = std::unique_ptr<LuabindObjectWrapper>(functionObject);
	}

	if (ObjectScriptsInitialized()) {
		if (RunFunctionOfScript(scriptPath, "Create") < 0) {
			return -5;
		}
	}

	return 0;
}

int MovableObject::ReloadScripts() {
	if (m_AllLoadedScripts.empty()) {
		return 0;
	}

	int status = 0;

	// TODO consider getting rid of this const_cast. It would require either code duplication or creating some non-const methods (specifically of PresetMan::GetEntityPreset, which may be unsafe. Could be this gross exceptional handling is the best way to go.
	MovableObject* movableObjectPreset = const_cast<MovableObject*>(dynamic_cast<const MovableObject*>(g_PresetMan.GetEntityPreset(GetClassName(), GetPresetName(), GetModuleID())));
	if (movableObjectPreset && this != movableObjectPreset) {
		movableObjectPreset->ReloadScripts();
	}

	std::unordered_map<std::string, bool> loadedScriptsCopy = m_AllLoadedScripts;
	m_AllLoadedScripts.clear();
	m_FunctionsAndScripts.clear();
	for (const auto& [scriptPath, scriptEnabled]: loadedScriptsCopy) {
		status = LoadScript(scriptPath, scriptEnabled);
		// If the script fails to load because of an error in its Lua, we need to manually add the script path so it's not lost forever.
		if (status == -4) {
			m_AllLoadedScripts.try_emplace(scriptPath, scriptEnabled);
		} else if (status < 0) {
			break;
		}
	}

	return status;
}

int MovableObject::InitializeObjectScripts() {
	std::lock_guard<std::recursive_mutex> lock(m_ThreadedLuaState->GetMutex());
	m_ScriptObjectName = "_ScriptedObjects[\"" + std::to_string(m_UniqueID) + "\"]";
	m_ThreadedLuaState->RegisterMO(this);
	m_ThreadedLuaState->SetTempEntity(this);
	if (m_ThreadedLuaState->RunScriptString("_ScriptedObjects = _ScriptedObjects or {}; " + m_ScriptObjectName + " = To" + GetClassName() + "(LuaMan.TempEntity); ") < 0) {
		RTEAbort("Failed to initialize object scripts for " + GetModuleAndPresetName() + ". Please report this to a developer.");
	}

	if (!m_FunctionsAndScripts.at("Create").empty() && RunScriptedFunctionInAppropriateScripts("Create", false, true) < 0) {
		m_ScriptObjectName = "ERROR";
		return -1;
	}

	return 0;
}

bool MovableObject::EnableOrDisableScript(const std::string& scriptPath, bool enableScript) {
	if (m_AllLoadedScripts.empty()) {
		return false;
	}

	if (auto scriptEntryIterator = m_AllLoadedScripts.find(scriptPath); scriptEntryIterator != m_AllLoadedScripts.end() && scriptEntryIterator->second == !enableScript) {
		if (ObjectScriptsInitialized() && RunFunctionOfScript(scriptPath, enableScript ? "OnScriptEnable" : "OnScriptDisable") < 0) {
			return false;
		}

		scriptEntryIterator->second = enableScript;

		// Slow, but better to spend this time here in EnableOrDisableScript than every update hashing the script path as we used to do
		for (auto& [functionName, functionObjects]: m_FunctionsAndScripts) {
			for (LuaFunction& luaFunction: functionObjects) {
				if (luaFunction.m_LuaFunction->GetFilePath() == scriptPath) {
					luaFunction.m_ScriptIsEnabled = enableScript;
				}
			}
		}

		return true;
	}
	return false;
}

void MovableObject::EnableOrDisableAllScripts(bool enableScripts) {
	for (const auto& [scriptPath, scriptIsEnabled]: m_AllLoadedScripts) {
		if (enableScripts != scriptIsEnabled) {
			EnableOrDisableScript(scriptPath, enableScripts);
		}
	}
}

int MovableObject::RunScriptedFunctionInAppropriateScripts(const std::string& functionName, bool runOnDisabledScripts, bool stopOnError, const std::vector<const Entity*>& functionEntityArguments, const std::vector<std::string_view>& functionLiteralArguments, const std::vector<LuabindObjectWrapper*>& functionObjectArguments) {
	int status = 0;

	auto itr = m_FunctionsAndScripts.find(functionName);
	if (itr == m_FunctionsAndScripts.end() || itr->second.empty()) {
		return -1;
	}

	if (!ObjectScriptsInitialized()) {
		status = InitializeObjectScripts();
	}

	if (status >= 0) {
		ZoneScoped;
		ZoneText(functionName.c_str(), functionName.length());
		for (const LuaFunction& luaFunction: itr->second) {
			const LuabindObjectWrapper* luabindObjectWrapper = luaFunction.m_LuaFunction.get();
			if (runOnDisabledScripts || luaFunction.m_ScriptIsEnabled) {
				LuaStateWrapper& usedState = GetAndLockStateForScript(luabindObjectWrapper->GetFilePath(), &luaFunction);
				std::lock_guard<std::recursive_mutex> lock(usedState.GetMutex(), std::adopt_lock);
				status = usedState.RunScriptFunctionObject(luabindObjectWrapper, "_ScriptedObjects", std::to_string(m_UniqueID), functionEntityArguments, functionLiteralArguments, functionObjectArguments);
				if (status < 0 && stopOnError) {
					return status;
				}
			}
		}
	}
	return status;
}

int MovableObject::RunFunctionOfScript(const std::string& scriptPath, const std::string& functionName, const std::vector<const Entity*>& functionEntityArguments, const std::vector<std::string_view>& functionLiteralArguments) {
	if (m_AllLoadedScripts.empty() || !ObjectScriptsInitialized()) {
		return -1;
	}

	LuaStateWrapper& usedState = GetAndLockStateForScript(scriptPath);
	std::lock_guard<std::recursive_mutex> lock(usedState.GetMutex(), std::adopt_lock);

	for (const LuaFunction& luaFunction: m_FunctionsAndScripts.at(functionName)) {
		const LuabindObjectWrapper* luabindObjectWrapper = luaFunction.m_LuaFunction.get();
		if (scriptPath == luabindObjectWrapper->GetFilePath() && usedState.RunScriptFunctionObject(luabindObjectWrapper, "_ScriptedObjects", std::to_string(m_UniqueID), functionEntityArguments, functionLiteralArguments) < 0) {
			if (m_AllLoadedScripts.size() > 1) {
				g_ConsoleMan.PrintString("ERROR: An error occured while trying to run the " + functionName + " function for script at path " + scriptPath);
			}
			return -2;
		}
	}

	return 0;
}

/*
MovableObject::MovableObject(const MovableObject &reference):
    m_Mass(reference.GetMass()),
    m_Pos(reference.GetPos()),
    m_Vel(reference.GetVel()),
    m_AgeTimer(reference.GetAge()),
    m_Lifetime(reference.GetLifetime())
{

}
*/

void MovableObject::SetTeam(int team) {
	SceneObject::SetTeam(team);
	if (Activity* activity = g_ActivityMan.GetActivity()) {
		activity->ForceSetTeamAsActive(team);
	}
}

float MovableObject::GetAltitude(int max, int accuracy) {
	return g_SceneMan.FindAltitude(m_Pos, max, accuracy);
}

void MovableObject::AddAbsForce(const Vector& force, const Vector& absPos) {
	m_Forces.push_back(std::make_pair(force, g_SceneMan.ShortestDistance(m_Pos, absPos) * c_MPP));
}

void MovableObject::AddAbsImpulseForce(const Vector& impulse, const Vector& absPos) {
#ifndef RELEASE_BUILD
	RTEAssert(impulse.GetLargest() < 500000, "HUEG IMPULSE FORCE");
#endif

	m_ImpulseForces.push_back(std::make_pair(impulse, g_SceneMan.ShortestDistance(m_Pos, absPos) * c_MPP));
}

void MovableObject::RestDetection() {
	// Translational settling detection.
	if (m_Vel.Dot(m_PrevVel) < 0) {
		++m_VelOscillations;
	} else {
		m_VelOscillations = 0;
	}
	if ((m_Pos - m_PrevPos).MagnitudeIsGreaterThan(1.0F)) {
		m_RestTimer.Reset();
	}
}

bool MovableObject::IsAtRest() {
	if (m_RestThreshold < 0 || m_PinStrength) {
		return false;
	} else {
		if (m_VelOscillations > 2) {
			return true;
		}
		return m_RestTimer.IsPastSimMS(m_RestThreshold);
	}
}

bool MovableObject::OnMOHit(HitData& hd) {
	if (hd.RootBody[HITOR] != hd.RootBody[HITEE] && (hd.Body[HITOR] == this || hd.Body[HITEE] == this)) {
		RunScriptedFunctionInAppropriateScripts("OnCollideWithMO", false, false, {hd.Body[hd.Body[HITOR] == this ? HITEE : HITOR], hd.RootBody[hd.Body[HITOR] == this ? HITEE : HITOR]});
	}
	return hd.Terminate[hd.RootBody[HITOR] == this ? HITOR : HITEE] = false;
}

unsigned char MovableObject::HitWhatTerrMaterial() const {
	return m_LastCollisionSimFrameNumber == g_MovableMan.GetSimUpdateFrameNumber() ? m_TerrainMatHit : g_MaterialAir;
}

void MovableObject::SetHitWhatTerrMaterial(unsigned char matID) {
	m_TerrainMatHit = matID;
	m_LastCollisionSimFrameNumber = g_MovableMan.GetSimUpdateFrameNumber();
	RunScriptedFunctionInAppropriateScripts("OnCollideWithTerrain", false, false, {}, {std::to_string(m_TerrainMatHit)});
}

Vector MovableObject::GetTotalForce() {
	Vector totalForceVector;
	for (const auto& [force, forceOffset]: m_Forces) {
		totalForceVector += force;
	}
	return totalForceVector;
}

void MovableObject::ApplyForces() {
	// Don't apply forces to pinned objects
	if (m_PinStrength > 0) {
		m_Forces.clear();
		return;
	}

	float deltaTime = g_TimerMan.GetDeltaTimeSecs();

	//// TODO: remove this!$@#$%#@%#@%#@^#@^#@^@#^@#")
	//    if (m_PresetName != "Test Player")
	// Apply global acceleration (gravity), scaled by the scalar we have that can even be negative.
	m_Vel += g_SceneMan.GetGlobalAcc() * m_GlobalAccScalar * deltaTime;

	// Calculate air resistance effects, only when something flies faster than a threshold
	if (m_AirResistance > 0 && m_Vel.GetLargest() >= m_AirThreshold)
		m_Vel *= 1.0 - (m_AirResistance * deltaTime);

	// Apply the translational effects of all the forces accumulated during the Update().
	if (m_Forces.size() > 0) {
		// Continuous force application to transformational velocity (F = m * a -> a = F / m).
		m_Vel += GetTotalForce() / (GetMass() != 0 ? GetMass() : 0.0001F) * deltaTime;
	}

	// Clear out the forces list
	m_Forces.clear();
}

void MovableObject::ApplyImpulses() {
	// Don't apply forces to pinned objects
	if (m_PinStrength > 0) {
		m_ImpulseForces.clear();
		return;
	}

	//    float totalImpulses.

	// Apply the translational effects of all the impulses accumulated during the Update()
	for (auto iItr = m_ImpulseForces.begin(); iItr != m_ImpulseForces.end(); ++iItr) {
		// Impulse force application to the transformational velocity of this MO.
		// Don't timescale these because they're already in kg * m/s (as opposed to kg * m/s^2).
		m_Vel += (*iItr).first / (GetMass() != 0 ? GetMass() : 0.0001F);
	}

	// Clear out the impulses list
	m_ImpulseForces.clear();
}

void MovableObject::PreTravel() {
	// Temporarily remove the representation of this from the scene MO sampler
	if (m_GetsHitByMOs) {
		m_IsTraveling = true;
	}

	// Save previous position and velocities before moving
	m_PrevPos = m_Pos;
	m_PrevVel = m_Vel;

	m_MOIDHit = g_NoMOID;
	m_TerrainMatHit = g_MaterialAir;
	m_ParticleUniqueIDHit = 0;
}

void MovableObject::Travel() {
}

void MovableObject::PostTravel() {
	// Toggle whether this gets hit by other AtomGroup MOs depending on whether it's going slower than a set threshold
	if (m_IgnoresAGHitsWhenSlowerThan > 0) {
		m_IgnoresAtomGroupHits = m_Vel.MagnitudeIsLessThan(m_IgnoresAGHitsWhenSlowerThan);
	}

	if (m_GetsHitByMOs) {
		if (!GetParent()) {
			m_IsTraveling = false;
		}
		m_AlreadyHitBy.clear();
	}
	m_IsUpdated = true;

	// Check for age expiration
	if (m_Lifetime && m_AgeTimer.GetElapsedSimTimeMS() > m_Lifetime) {
		m_ToDelete = true;
	}

	// Check for stupid positions
	if (!GetParent() && !g_SceneMan.IsWithinBounds(m_Pos.m_X, m_Pos.m_Y, 1000)) {
		m_ToDelete = true;
	}

	// Fix speeds that are too high
	FixTooFast();

	// Never let mission critical stuff settle or delete
	if (m_MissionCritical) {
		m_ToSettle = false;
	}

	// Reset the terrain intersection warning
	m_CheckTerrIntersection = false;

	m_DistanceTravelled += m_Vel.GetMagnitude() * c_PPM * g_TimerMan.GetDeltaTimeSecs();
}

void MovableObject::Update() {
	if (m_RandomizeEffectRotAngleEveryFrame) {
		m_EffectRotAngle = c_PI * 2.0F * RandomNormalNum();
	}

	if (m_pScreenEffect && m_PostEffectEnabled) {
		SetPostScreenEffectToDraw();
	}
}

void MovableObject::Draw(BITMAP* targetBitmap, const Vector& targetPos, DrawMode mode, bool onlyPhysical) const {
	if (mode == g_DrawMOID && m_MOID == g_NoMOID) {
		return;
	}

	g_SceneMan.RegisterDrawing(targetBitmap, m_MOID, m_Pos - targetPos, 1.0F);
}

int MovableObject::UpdateScripts() {
	m_SimUpdatesSinceLastScriptedUpdate++;

	if (m_AllLoadedScripts.empty()) {
		return -1;
	}

	int status = 0;
	if (!ObjectScriptsInitialized()) {
		status = InitializeObjectScripts();
	}

	if (m_SimUpdatesSinceLastScriptedUpdate < m_SimUpdatesBetweenScriptedUpdates) {
		return 1;
	}

	m_SimUpdatesSinceLastScriptedUpdate = 0;

	if (status >= 0) {
		status = RunScriptedFunctionInAppropriateScripts("Update", false, true, {}, {}, {});
	}

	return status;
}

const std::string& MovableObject::GetStringValue(const std::string& key) const {
	auto itr = m_StringValueMap.find(key);
	if (itr == m_StringValueMap.end()) {
		return ms_EmptyString;
	}

	return itr->second;
}

std::string MovableObject::GetEncodedStringValue(const std::string& key) const {
	auto itr = m_StringValueMap.find(key);
	if (itr == m_StringValueMap.end()) {
		return ms_EmptyString;
	}

	return base64_decode(itr->second);
}

double MovableObject::GetNumberValue(const std::string& key) const {
	auto itr = m_NumberValueMap.find(key);
	if (itr == m_NumberValueMap.end()) {
		return 0.0;
	}

	return itr->second;
}

Entity* MovableObject::GetObjectValue(const std::string& key) const {
	auto itr = m_ObjectValueMap.find(key);
	if (itr == m_ObjectValueMap.end()) {
		return nullptr;
	}

	return itr->second;
}

void MovableObject::SetStringValue(const std::string& key, const std::string& value) {
	m_StringValueMap[key] = value;
}

void MovableObject::SetEncodedStringValue(const std::string& key, const std::string& value) {
	m_StringValueMap[key] = base64_encode(value, true);
}

void MovableObject::SetNumberValue(const std::string& key, double value) {
	m_NumberValueMap[key] = value;
}

void MovableObject::SetObjectValue(const std::string& key, Entity* value) {
	m_ObjectValueMap[key] = value;
}

void MovableObject::RemoveStringValue(const std::string& key) {
	m_StringValueMap.erase(key);
}

void MovableObject::RemoveNumberValue(const std::string& key) {
	m_NumberValueMap.erase(key);
}

void MovableObject::RemoveObjectValue(const std::string& key) {
	m_ObjectValueMap.erase(key);
}

bool MovableObject::StringValueExists(const std::string& key) const {
	return m_StringValueMap.find(key) != m_StringValueMap.end();
}

bool MovableObject::NumberValueExists(const std::string& key) const {
	return m_NumberValueMap.find(key) != m_NumberValueMap.end();
}

bool MovableObject::ObjectValueExists(const std::string& key) const {
	return m_ObjectValueMap.find(key) != m_ObjectValueMap.end();
}

int MovableObject::WhilePieMenuOpenListener(const PieMenu* pieMenu) {
	return RunScriptedFunctionInAppropriateScripts("WhilePieMenuOpen", false, false, {pieMenu});
}

void MovableObject::UpdateMOID(std::vector<MovableObject*>& MOIDIndex, MOID rootMOID, bool makeNewMOID) {
	// Register the own MOID
	RegMOID(MOIDIndex, rootMOID, makeNewMOID);

	// Register all the attachaed children of this, going through the class hierarchy
	UpdateChildMOIDs(MOIDIndex, rootMOID, makeNewMOID);

	// Figure out the total MOID footstep of this and all its children combined
	m_MOIDFootprint = MOIDIndex.size() - m_MOID;
}

void MovableObject::GetMOIDs(std::vector<MOID>& MOIDs) const {
	if (m_MOID != g_NoMOID) {
		MOIDs.push_back(m_MOID);
	}
}

MOID MovableObject::HitWhatMOID() const {
	return m_LastCollisionSimFrameNumber == g_MovableMan.GetSimUpdateFrameNumber() ? m_MOIDHit : g_NoMOID;
}

void MovableObject::SetHitWhatMOID(MOID id) {
	m_MOIDHit = id;
	m_LastCollisionSimFrameNumber = g_MovableMan.GetSimUpdateFrameNumber();
}

long int MovableObject::HitWhatParticleUniqueID() const {
	return m_LastCollisionSimFrameNumber == g_MovableMan.GetSimUpdateFrameNumber() ? m_ParticleUniqueIDHit : 0;
}

void MovableObject::SetHitWhatParticleUniqueID(long int id) {
	m_ParticleUniqueIDHit = id;
	m_LastCollisionSimFrameNumber = g_MovableMan.GetSimUpdateFrameNumber();
}

void MovableObject::RegMOID(std::vector<MovableObject*>& MOIDIndex, MOID rootMOID, bool makeNewMOID) {
	if (!makeNewMOID && GetParent()) {
		m_MOID = GetParent()->GetID();
	} else {
		if (MOIDIndex.size() == g_NoMOID) {
			MOIDIndex.push_back(0);
		}

		m_MOID = MOIDIndex.size();
		MOIDIndex.push_back(this);
	}

	m_RootMOID = rootMOID == g_NoMOID ? m_MOID : rootMOID;
}

bool MovableObject::DrawToTerrain(SLTerrain* terrain) {
	if (!terrain) {
		return false;
	}
	if (dynamic_cast<MOSprite*>(this)) {
		auto wrappedMaskedBlit = [](BITMAP* sourceBitmap, BITMAP* destinationBitmap, const Vector& bitmapPos, bool swapSourceWithDestination) {
			std::array<BITMAP*, 2> bitmaps = {sourceBitmap, destinationBitmap};
			std::array<Vector, 5> srcPos = {
			    Vector(bitmapPos.GetX(), bitmapPos.GetY()),
			    Vector(bitmapPos.GetX() + static_cast<float>(g_SceneMan.GetSceneWidth()), bitmapPos.GetY()),
			    Vector(bitmapPos.GetX() - static_cast<float>(g_SceneMan.GetSceneWidth()), bitmapPos.GetY()),
			    Vector(bitmapPos.GetX(), bitmapPos.GetY() + static_cast<float>(g_SceneMan.GetSceneHeight())),
			    Vector(bitmapPos.GetX(), bitmapPos.GetY() - static_cast<float>(g_SceneMan.GetSceneHeight()))};
			std::array<Vector, 5> destPos;
			destPos.fill(Vector());

			if (swapSourceWithDestination) {
				std::swap(bitmaps[0], bitmaps[1]);
				std::swap(srcPos, destPos);
			}
			masked_blit(bitmaps[0], bitmaps[1], srcPos[0].GetFloorIntX(), srcPos[0].GetFloorIntY(), destPos[0].GetFloorIntX(), destPos[0].GetFloorIntY(), destinationBitmap->w, destinationBitmap->h);
			if (g_SceneMan.SceneWrapsX()) {
				if (bitmapPos.GetFloorIntX() < 0) {
					masked_blit(bitmaps[0], bitmaps[1], srcPos[1].GetFloorIntX(), srcPos[1].GetFloorIntY(), destPos[1].GetFloorIntX(), destPos[1].GetFloorIntY(), destinationBitmap->w, destinationBitmap->h);
				} else if (bitmapPos.GetFloorIntX() + destinationBitmap->w > g_SceneMan.GetSceneWidth()) {
					masked_blit(bitmaps[0], bitmaps[1], srcPos[2].GetFloorIntX(), srcPos[2].GetFloorIntY(), destPos[2].GetFloorIntX(), destPos[2].GetFloorIntY(), destinationBitmap->w, destinationBitmap->h);
				}
			}
			if (g_SceneMan.SceneWrapsY()) {
				if (bitmapPos.GetFloorIntY() < 0) {
					masked_blit(bitmaps[0], bitmaps[1], srcPos[3].GetFloorIntX(), srcPos[3].GetFloorIntY(), destPos[3].GetFloorIntX(), destPos[3].GetFloorIntY(), destinationBitmap->w, destinationBitmap->h);
				} else if (bitmapPos.GetFloorIntY() + destinationBitmap->h > g_SceneMan.GetSceneHeight()) {
					masked_blit(bitmaps[0], bitmaps[1], srcPos[4].GetFloorIntX(), srcPos[4].GetFloorIntY(), destPos[4].GetFloorIntX(), destPos[4].GetFloorIntY(), destinationBitmap->w, destinationBitmap->h);
				}
			}
		};
		BITMAP* tempBitmap = g_SceneMan.GetIntermediateBitmapForSettlingIntoTerrain(static_cast<int>(GetDiameter()));
		Vector tempBitmapPos = m_Pos.GetFloored() - Vector(static_cast<float>(tempBitmap->w / 2), static_cast<float>(tempBitmap->w / 2));

		clear_bitmap(tempBitmap);
		// Draw the object to the temp bitmap, then draw the foreground layer on top of it, then draw it to the foreground layer.
		Draw(tempBitmap, tempBitmapPos, DrawMode::g_DrawColor, true);
		wrappedMaskedBlit(terrain->GetFGColorBitmap(), tempBitmap, tempBitmapPos, false);
		wrappedMaskedBlit(terrain->GetFGColorBitmap(), tempBitmap, tempBitmapPos, true);

		clear_bitmap(tempBitmap);
		// Draw the object to the temp bitmap, then draw the material layer on top of it, then draw it to the material layer.
		Draw(tempBitmap, tempBitmapPos, DrawMode::g_DrawMaterial, true);
		wrappedMaskedBlit(terrain->GetMaterialBitmap(), tempBitmap, tempBitmapPos, false);
		wrappedMaskedBlit(terrain->GetMaterialBitmap(), tempBitmap, tempBitmapPos, true);

		terrain->AddUpdatedMaterialArea(Box(tempBitmapPos, static_cast<float>(tempBitmap->w), static_cast<float>(tempBitmap->h)));
		g_SceneMan.RegisterTerrainChange(tempBitmapPos.GetFloorIntX(), tempBitmapPos.GetFloorIntY(), tempBitmap->w, tempBitmap->h, ColorKeys::g_MaskColor, false);
	} else {
		Draw(terrain->GetFGColorBitmap(), Vector(), DrawMode::g_DrawColor, true);
		Material const* terrMat = g_SceneMan.GetMaterialFromID(g_SceneMan.GetTerrain()->GetMaterialPixel(m_Pos.GetFloorIntX(), m_Pos.GetFloorIntY()));
		if (GetMaterial()->GetPriority() > terrMat->GetPriority()) {
			Draw(terrain->GetMaterialBitmap(), Vector(), DrawMode::g_DrawMaterial, true);
		}
		g_SceneMan.RegisterTerrainChange(m_Pos.GetFloorIntX(), m_Pos.GetFloorIntY(), 1, 1, DrawMode::g_DrawColor, false);
	}
	return true;
}

void MovableObject::SetPostScreenEffectToDraw() const {
	if (m_AgeTimer.GetElapsedSimTimeMS() >= m_EffectStartTime && (m_EffectStopTime == 0 || !m_AgeTimer.IsPastSimMS(m_EffectStopTime))) {
		if (m_EffectAlwaysShows || !g_SceneMan.ObscuredPoint(m_Pos.GetFloorIntX(), m_Pos.GetFloorIntY())) {
			g_PostProcessMan.RegisterPostEffect(m_Pos, m_pScreenEffect, m_ScreenEffectHash, Lerp(m_EffectStartTime, m_EffectStopTime, m_EffectStartStrength, m_EffectStopStrength, m_AgeTimer.GetElapsedSimTimeMS()), m_EffectRotAngle);
		}
	}
}
