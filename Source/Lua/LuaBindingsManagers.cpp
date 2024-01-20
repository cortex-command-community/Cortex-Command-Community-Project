// Make sure that binding definition files are always set to NOT use pre-compiled headers and conformance mode (/permissive) otherwise everything will be on fire!

#include "LuaBindingRegisterDefinitions.h"

#include "LuaAdapterDefinitions.h"

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

#include "DataModule.h"
#include "SLTerrain.h"

namespace RTE {

	LuaBindingRegisterFunctionDefinitionForType(ManagerLuaBindings, ActivityMan) {
		auto luaType = SimpleNamedTypeLuaClassDefinition(ActivityMan, "ActivityManager");

		luaType["DefaultActivityType"] = sol::property(&ActivityMan::GetDefaultActivityType, &ActivityMan::SetDefaultActivityType);
		luaType["DefaultActivityName"] = sol::property(&ActivityMan::GetDefaultActivityName, &ActivityMan::SetDefaultActivityName);

		luaType["SetStartActivity"] = &ActivityMan::SetStartActivity; //; //, luabind::adopt(_2);
		luaType["GetStartActivity"] = &ActivityMan::GetStartActivity;
		luaType["GetActivity"] = &ActivityMan::GetActivity;
		luaType["StartActivity"] = (int(ActivityMan::*)(Activity*)) & ActivityMan::StartActivity; //; //, luabind::adopt(_2);
		luaType["StartActivity"] = (int(ActivityMan::*)(const std::string&, const std::string&)) & ActivityMan::StartActivity;
		luaType["RestartActivity"] = &ActivityMan::RestartActivity;
		luaType["PauseActivity"] = &ActivityMan::PauseActivity;
		luaType["EndActivity"] = &ActivityMan::EndActivity;
		luaType["ActivityRunning"] = &ActivityMan::ActivityRunning;
		luaType["ActivityPaused"] = &ActivityMan::ActivityPaused;
		luaType["SaveGame"] = &ActivityMan::SaveCurrentGame;
		luaType["LoadGame"] = &ActivityMan::LoadAndLaunchGame;
	}

	LuaBindingRegisterFunctionDefinitionForType(ManagerLuaBindings, AudioMan) {
		auto luaType = SimpleNamedTypeLuaClassDefinition(AudioMan, "AudioManager");

		luaType["MusicVolume"] = sol::property(&AudioMan::GetMusicVolume, &AudioMan::SetMusicVolume);
		luaType["SoundsVolume"] = sol::property(&AudioMan::GetSoundsVolume, &AudioMan::SetSoundsVolume);

		luaType["StopAll"] = &AudioMan::StopAll;
		luaType["GetGlobalPitch"] = &AudioMan::GetGlobalPitch;
		luaType["IsMusicPlaying"] = &AudioMan::IsMusicPlaying;
		luaType["SetTempMusicVolume"] = &AudioMan::SetTempMusicVolume;
		luaType["GetMusicPosition"] = &AudioMan::GetMusicPosition;
		luaType["SetMusicPosition"] = &AudioMan::SetMusicPosition;
		luaType["SetMusicPitch"] = &AudioMan::SetMusicPitch;
		luaType["StopMusic"] = &AudioMan::StopMusic;
		luaType["PlayMusic"] = &AudioMan::PlayMusic;
		luaType["PlayNextStream"] = &AudioMan::PlayNextStream;
		luaType["StopMusic"] = &AudioMan::StopMusic;
		luaType["QueueMusicStream"] = &AudioMan::QueueMusicStream;
		luaType["QueueSilence"] = &AudioMan::QueueSilence;
		luaType["ClearMusicQueue"] = &AudioMan::ClearMusicQueue;
		luaType["PlaySound"] = (SoundContainer * (AudioMan::*)(const std::string& filePath)) & AudioMan::PlaySound; //; //, luabind::adopt(luabind::result);
		luaType["PlaySound"] = (SoundContainer * (AudioMan::*)(const std::string& filePath, const Vector& position)) & AudioMan::PlaySound; //; //, luabind::adopt(luabind::result);
		luaType["PlaySound"] = (SoundContainer * (AudioMan::*)(const std::string& filePath, const Vector& position, int player)) & AudioMan::PlaySound; //; //, luabind::adopt(luabind::result);
	}

	LuaBindingRegisterFunctionDefinitionForType(ManagerLuaBindings, ConsoleMan) {
		auto luaType = SimpleNamedTypeLuaClassDefinition(ConsoleMan, "ConsoleManager");

		luaType["PrintString"] = &ConsoleMan::PrintString;
		luaType["SaveInputLog"] = &ConsoleMan::SaveInputLog;
		luaType["SaveAllText"] = &ConsoleMan::SaveAllText;
		luaType["Clear"] = &ConsoleMan::ClearLog;
	}

	LuaBindingRegisterFunctionDefinitionForType(ManagerLuaBindings, FrameMan) {
		auto luaType = SimpleNamedTypeLuaClassDefinition(FrameMan, "FrameManager");

		luaType["PlayerScreenWidth"] = sol::property(&FrameMan::GetPlayerScreenWidth);
		luaType["PlayerScreenHeight"] = sol::property(&FrameMan::GetPlayerScreenHeight);
		luaType["ScreenCount"] = sol::property(&FrameMan::GetScreenCount);

		luaType["IsHudDisabled"] = &FrameMan::IsHudDisabled;
		luaType["SetHudDisabled"] = &FrameMan::SetHudDisabled;
		luaType["LoadPalette"] = &FrameMan::LoadPalette;
		luaType["SetScreenText"] = &FrameMan::SetScreenText;
		luaType["ClearScreenText"] = &FrameMan::ClearScreenText;
		luaType["FadeInPalette"] = &FrameMan::FadeInPalette;
		luaType["FadeOutPalette"] = &FrameMan::FadeOutPalette;
		luaType["SaveScreenToPNG"] = &FrameMan::SaveScreenToPNG;
		luaType["SaveBitmapToPNG"] = &FrameMan::SaveBitmapToPNG;
		luaType["FlashScreen"] = &FrameMan::FlashScreen;
		luaType["CalculateTextHeight"] = &FrameMan::CalculateTextHeight;
		luaType["CalculateTextWidth"] = &FrameMan::CalculateTextWidth;
		luaType["SplitStringToFitWidth"] = &FrameMan::SplitStringToFitWidth;
	}

	LuaBindingRegisterFunctionDefinitionForType(ManagerLuaBindings, MetaMan) {
		auto luaType = SimpleNamedTypeLuaClassDefinition(MetaMan, "MetaManager");

		luaType["GameName"] = sol::property(&MetaMan::GetGameName, &MetaMan::SetGameName);
		luaType["PlayerTurn"] = sol::property(&MetaMan::GetPlayerTurn);
		luaType["PlayerCount"] = sol::property(&MetaMan::GetPlayerCount);

		luaType["Players"] = &MetaMan::m_Players;

		luaType["GetTeamOfPlayer"] = &MetaMan::GetTeamOfPlayer;
		luaType["GetPlayer"] = &MetaMan::GetPlayer;
		luaType["GetMetaPlayerOfInGamePlayer"] = &MetaMan::GetMetaPlayerOfInGamePlayer;
	}

	LuaBindingRegisterFunctionDefinitionForType(ManagerLuaBindings, MovableMan) {
		auto luaType = SimpleNamedTypeLuaClassDefinition(MovableMan, "MovableManager");

		luaType["MaxDroppedItems"] = sol::property(&MovableMan::GetMaxDroppedItems, &MovableMan::SetMaxDroppedItems);

		luaType["Actors"] = &MovableMan::m_Actors;
		luaType["Items"] = &MovableMan::m_Items;
		luaType["Particles"] = &MovableMan::m_Particles;
		luaType["AddedActors"] = &MovableMan::m_AddedActors;
		luaType["AddedItems"] = &MovableMan::m_AddedItems;
		luaType["AddedParticles"] = &MovableMan::m_AddedParticles;
		luaType["AlarmEvents"] = &MovableMan::m_AlarmEvents;
		luaType["AddedAlarmEvents"] = &MovableMan::m_AddedAlarmEvents;

		luaType["GetMOFromID"] = &MovableMan::GetMOFromID;
		luaType["FindObjectByUniqueID"] = &MovableMan::FindObjectByUniqueID;
		luaType["GetMOIDCount"] = &MovableMan::GetMOIDCount;
		luaType["GetTeamMOIDCount"] = &MovableMan::GetTeamMOIDCount;
		luaType["PurgeAllMOs"] = &MovableMan::PurgeAllMOs;
		luaType["GetNextActorInGroup"] = &MovableMan::GetNextActorInGroup;
		luaType["GetPrevActorInGroup"] = &MovableMan::GetPrevActorInGroup;
		luaType["GetNextTeamActor"] = &MovableMan::GetNextTeamActor;
		luaType["GetPrevTeamActor"] = &MovableMan::GetPrevTeamActor;
		luaType["GetClosestTeamActor"] = (Actor * (MovableMan::*)(int team, int player, const Vector& scenePoint, int maxRadius, Vector& getDistance, const Actor* excludeThis)) & MovableMan::GetClosestTeamActor;
		luaType["GetClosestTeamActor"] = (Actor * (MovableMan::*)(int team, int player, const Vector& scenePoint, int maxRadius, Vector& getDistance, bool onlyPlayerControllableActors, const Actor* excludeThis)) & MovableMan::GetClosestTeamActor;
		luaType["GetClosestEnemyActor"] = &MovableMan::GetClosestEnemyActor;
		luaType["GetFirstTeamActor"] = &MovableMan::GetFirstTeamActor;
		luaType["GetClosestActor"] = &MovableMan::GetClosestActor;
		luaType["GetClosestBrainActor"] = &MovableMan::GetClosestBrainActor;
		luaType["GetFirstBrainActor"] = &MovableMan::GetFirstBrainActor;
		luaType["GetClosestOtherBrainActor"] = &MovableMan::GetClosestOtherBrainActor;
		luaType["GetFirstOtherBrainActor"] = &MovableMan::GetFirstOtherBrainActor;
		luaType["GetUnassignedBrain"] = &MovableMan::GetUnassignedBrain;
		luaType["GetParticleCount"] = &MovableMan::GetParticleCount;
		luaType["GetSplashRatio"] = &MovableMan::GetSplashRatio;
		luaType["SortTeamRoster"] = &MovableMan::SortTeamRoster;
		luaType["ChangeActorTeam"] = &MovableMan::ChangeActorTeam;
		luaType["RemoveActor"] = &MovableMan::RemoveActor; //, luabind::adopt(luabind::return_value))
		luaType["RemoveItem"] = &MovableMan::RemoveItem; //, luabind::adopt(luabind::return_value))
		luaType["RemoveParticle"] = &MovableMan::RemoveParticle; //, luabind::adopt(luabind::return_value))
		luaType["ValidMO"] = &MovableMan::ValidMO;
		luaType["IsActor"] = &MovableMan::IsActor;
		luaType["IsDevice"] = &MovableMan::IsDevice;
		luaType["IsParticle"] = &MovableMan::IsParticle;
		luaType["IsOfActor"] = &MovableMan::IsOfActor;
		luaType["GetRootMOID"] = &MovableMan::GetRootMOID;
		luaType["RemoveMO"] = &MovableMan::RemoveMO;
		luaType["KillAllTeamActors"] = &MovableMan::KillAllTeamActors;
		luaType["KillAllEnemyActors"] = &MovableMan::KillAllEnemyActors;
		luaType["OpenAllDoors"] = &MovableMan::OpenAllDoors;
		luaType["IsParticleSettlingEnabled"] = &MovableMan::IsParticleSettlingEnabled;
		luaType["EnableParticleSettling"] = &MovableMan::EnableParticleSettling;
		luaType["IsMOSubtractionEnabled"] = &MovableMan::IsMOSubtractionEnabled;
		luaType["GetMOsInBox"] = (const std::vector<MovableObject*> (MovableMan::*)(const Box& box) const) & MovableMan::GetMOsInBox;
		luaType["GetMOsInBox"] = (const std::vector<MovableObject*> (MovableMan::*)(const Box& box, int ignoreTeam) const) & MovableMan::GetMOsInBox;
		luaType["GetMOsInBox"] = (const std::vector<MovableObject*> (MovableMan::*)(const Box& box, int ignoreTeam, bool getsHitByMOsOnly) const) & MovableMan::GetMOsInBox;
		luaType["GetMOsInRadius"] = (const std::vector<MovableObject*> (MovableMan::*)(const Vector& centre, float radius) const) & MovableMan::GetMOsInRadius;
		luaType["GetMOsInRadius"] = (const std::vector<MovableObject*> (MovableMan::*)(const Vector& centre, float radius, int ignoreTeam) const) & MovableMan::GetMOsInRadius;
		luaType["GetMOsInRadius"] = (const std::vector<MovableObject*> (MovableMan::*)(const Vector& centre, float radius, int ignoreTeam, bool getsHitByMOsOnly) const) & MovableMan::GetMOsInRadius;

		luaType["SendGlobalMessage"] = &LuaAdaptersMovableMan::SendGlobalMessage1;
		luaType["SendGlobalMessage"] = &LuaAdaptersMovableMan::SendGlobalMessage2;
		luaType["AddMO"] = &LuaAdaptersMovableMan::AddMO; //, luabind::adopt(_2);
		luaType["AddActor"] = &LuaAdaptersMovableMan::AddActor; //, luabind::adopt(_2);
		luaType["AddItem"] = &LuaAdaptersMovableMan::AddItem; //, luabind::adopt(_2);
		luaType["AddParticle"] = &LuaAdaptersMovableMan::AddParticle; //, luabind::adopt(_2);
	}

	LuaBindingRegisterFunctionDefinitionForType(ManagerLuaBindings, PerformanceMan) {
		auto luaType = SimpleNamedTypeLuaClassDefinition(PerformanceMan, "PerformanceManager");

		luaType["ShowPerformanceStats"] = sol::property(&PerformanceMan::IsShowingPerformanceStats, &PerformanceMan::ShowPerformanceStats);
	}

	LuaBindingRegisterFunctionDefinitionForType(ManagerLuaBindings, PostProcessMan) {
		auto luaType = SimpleNamedTypeLuaClassDefinition(PostProcessMan, "PostProcessManager");

		luaType["RegisterPostEffect"] = &PostProcessMan::RegisterPostEffect;
	}

	LuaBindingRegisterFunctionDefinitionForType(ManagerLuaBindings, PresetMan) {
		auto luaType = SimpleNamedTypeLuaClassDefinition(PresetMan, "PresetManager");

		luaType["Modules"] = &PresetMan::m_pDataModules;

		luaType["LoadDataModule"] = (bool(PresetMan::*)(const std::string&)) & PresetMan::LoadDataModule;
		luaType["GetDataModule"] = &PresetMan::GetDataModule;
		luaType["GetModuleID"] = &PresetMan::GetModuleID;
		luaType["GetModuleIDFromPath"] = &PresetMan::GetModuleIDFromPath;
		luaType["GetTotalModuleCount"] = &PresetMan::GetTotalModuleCount;
		luaType["GetOfficialModuleCount"] = &PresetMan::GetOfficialModuleCount;
		luaType["AddPreset"] = &PresetMan::AddEntityPreset;
		luaType["GetPreset"] = (const Entity* (PresetMan::*)(std::string, std::string, int)) & PresetMan::GetEntityPreset;
		luaType["GetPreset"] = (const Entity* (PresetMan::*)(std::string, std::string, std::string)) & PresetMan::GetEntityPreset;
		luaType["GetLoadout"] = (Actor * (PresetMan::*)(std::string, std::string, bool)) & PresetMan::GetLoadout; //, luabind::adopt(luabind::result);
		luaType["GetLoadout"] = (Actor * (PresetMan::*)(std::string, int, bool)) & PresetMan::GetLoadout; //, luabind::adopt(luabind::result);
		luaType["GetRandomOfGroup"] = &PresetMan::GetRandomOfGroup;
		luaType["GetRandomOfGroupInModuleSpace"] = &PresetMan::GetRandomOfGroupInModuleSpace;
		luaType["GetEntityDataLocation"] = &PresetMan::GetEntityDataLocation;
		luaType["ReadReflectedPreset"] = &PresetMan::ReadReflectedPreset;
		luaType["ReloadEntityPreset"] = &LuaAdaptersPresetMan::ReloadEntityPreset1;
		luaType["ReloadEntityPreset"] = &LuaAdaptersPresetMan::ReloadEntityPreset2;
		luaType["GetAllEntities"] = &LuaAdaptersPresetMan::GetAllEntities; //, luabind::adopt(luabind::result) + luabind::return_stl_iterator)
		luaType["GetAllEntitiesOfGroup"] = &LuaAdaptersPresetMan::GetAllEntitiesOfGroup; //, luabind::adopt(luabind::result) + luabind::return_stl_iterator)
		luaType["GetAllEntitiesOfGroup"] = &LuaAdaptersPresetMan::GetAllEntitiesOfGroup2; //, luabind::adopt(luabind::result) + luabind::return_stl_iterator)
		luaType["GetAllEntitiesOfGroup"] = &LuaAdaptersPresetMan::GetAllEntitiesOfGroup3; //, luabind::adopt(luabind::result) + luabind::return_stl_iterator)
		luaType["ReloadAllScripts"] = &PresetMan::ReloadAllScripts;
		luaType["IsModuleOfficial"] = &PresetMan::IsModuleOfficial;
		luaType["IsModuleUserdata"] = &PresetMan::IsModuleUserdata;
		luaType["GetFullModulePath"] = &PresetMan::GetFullModulePath;
	}

	LuaBindingRegisterFunctionDefinitionForType(ManagerLuaBindings, PrimitiveMan) {
		auto luaType = SimpleNamedTypeLuaClassDefinition(PrimitiveMan, "PrimitiveManager");

		luaType["DrawLinePrimitive"] = (void(PrimitiveMan::*)(const Vector& start, const Vector& end, unsigned char color)) & PrimitiveMan::DrawLinePrimitive;
		luaType["DrawLinePrimitive"] = (void(PrimitiveMan::*)(const Vector& start, const Vector& end, unsigned char color, int thickness)) & PrimitiveMan::DrawLinePrimitive;
		luaType["DrawLinePrimitive"] = (void(PrimitiveMan::*)(int player, const Vector& start, const Vector& end, unsigned char color)) & PrimitiveMan::DrawLinePrimitive;
		luaType["DrawLinePrimitive"] = (void(PrimitiveMan::*)(int player, const Vector& start, const Vector& end, unsigned char color, int thickness)) & PrimitiveMan::DrawLinePrimitive;
		luaType["DrawArcPrimitive"] = (void(PrimitiveMan::*)(const Vector& pos, float startAngle, float endAngle, int radius, unsigned char color)) & PrimitiveMan::DrawArcPrimitive;
		luaType["DrawArcPrimitive"] = (void(PrimitiveMan::*)(const Vector& pos, float startAngle, float endAngle, int radius, unsigned char color, int thickness)) & PrimitiveMan::DrawArcPrimitive;
		luaType["DrawArcPrimitive"] = (void(PrimitiveMan::*)(int player, const Vector& pos, float startAngle, float endAngle, int radius, unsigned char color)) & PrimitiveMan::DrawArcPrimitive;
		luaType["DrawArcPrimitive"] = (void(PrimitiveMan::*)(int player, const Vector& pos, float startAngle, float endAngle, int radius, unsigned char color, int thickness)) & PrimitiveMan::DrawArcPrimitive;
		luaType["DrawSplinePrimitive"] = (void(PrimitiveMan::*)(const Vector& start, const Vector& guideA, const Vector& guideB, const Vector& end, unsigned char color)) & PrimitiveMan::DrawSplinePrimitive;
		luaType["DrawSplinePrimitive"] = (void(PrimitiveMan::*)(int player, const Vector& start, const Vector& guideA, const Vector& guideB, const Vector& end, unsigned char color)) & PrimitiveMan::DrawSplinePrimitive;
		luaType["DrawBoxPrimitive"] = (void(PrimitiveMan::*)(const Vector& start, const Vector& end, unsigned char color)) & PrimitiveMan::DrawBoxPrimitive;
		luaType["DrawBoxPrimitive"] = (void(PrimitiveMan::*)(int player, const Vector& start, const Vector& end, unsigned char color)) & PrimitiveMan::DrawBoxPrimitive;
		luaType["DrawBoxFillPrimitive"] = (void(PrimitiveMan::*)(const Vector& start, const Vector& end, unsigned char color)) & PrimitiveMan::DrawBoxFillPrimitive;
		luaType["DrawBoxFillPrimitive"] = (void(PrimitiveMan::*)(int player, const Vector& start, const Vector& end, unsigned char color)) & PrimitiveMan::DrawBoxFillPrimitive;
		luaType["DrawRoundedBoxPrimitive"] = (void(PrimitiveMan::*)(const Vector& start, const Vector& end, int cornerRadius, unsigned char color)) & PrimitiveMan::DrawRoundedBoxPrimitive;
		luaType["DrawRoundedBoxPrimitive"] = (void(PrimitiveMan::*)(int player, const Vector& start, const Vector& end, int cornerRadius, unsigned char color)) & PrimitiveMan::DrawRoundedBoxPrimitive;
		luaType["DrawRoundedBoxFillPrimitive"] = (void(PrimitiveMan::*)(const Vector& start, const Vector& end, int cornerRadius, unsigned char color)) & PrimitiveMan::DrawRoundedBoxFillPrimitive;
		luaType["DrawRoundedBoxFillPrimitive"] = (void(PrimitiveMan::*)(int player, const Vector& start, const Vector& end, int cornerRadius, unsigned char color)) & PrimitiveMan::DrawRoundedBoxFillPrimitive;
		luaType["DrawCirclePrimitive"] = (void(PrimitiveMan::*)(const Vector& pos, int radius, unsigned char color)) & PrimitiveMan::DrawCirclePrimitive;
		luaType["DrawCirclePrimitive"] = (void(PrimitiveMan::*)(int player, const Vector& pos, int radius, unsigned char color)) & PrimitiveMan::DrawCirclePrimitive;
		luaType["DrawCircleFillPrimitive"] = (void(PrimitiveMan::*)(const Vector& pos, int radius, unsigned char color)) & PrimitiveMan::DrawCircleFillPrimitive;
		luaType["DrawCircleFillPrimitive"] = (void(PrimitiveMan::*)(int player, const Vector& pos, int radius, unsigned char color)) & PrimitiveMan::DrawCircleFillPrimitive;
		luaType["DrawEllipsePrimitive"] = (void(PrimitiveMan::*)(const Vector& pos, int horizRadius, int vertRadius, unsigned char color)) & PrimitiveMan::DrawEllipsePrimitive;
		luaType["DrawEllipsePrimitive"] = (void(PrimitiveMan::*)(int player, const Vector& pos, int horizRadius, int vertRadius, unsigned char color)) & PrimitiveMan::DrawEllipsePrimitive;
		luaType["DrawEllipseFillPrimitive"] = (void(PrimitiveMan::*)(const Vector& pos, int horizRadius, int vertRadius, unsigned char color)) & PrimitiveMan::DrawEllipseFillPrimitive;
		luaType["DrawEllipseFillPrimitive"] = (void(PrimitiveMan::*)(int player, const Vector& pos, int horizRadius, int vertRadius, unsigned char color)) & PrimitiveMan::DrawEllipseFillPrimitive;
		luaType["DrawTrianglePrimitive"] = (void(PrimitiveMan::*)(const Vector& pointA, const Vector& pointB, const Vector& pointC, unsigned char color)) & PrimitiveMan::DrawTrianglePrimitive;
		luaType["DrawTrianglePrimitive"] = (void(PrimitiveMan::*)(int player, const Vector& pointA, const Vector& pointB, const Vector& pointC, unsigned char color)) & PrimitiveMan::DrawTrianglePrimitive;
		luaType["DrawTriangleFillPrimitive"] = (void(PrimitiveMan::*)(const Vector& pointA, const Vector& pointB, const Vector& pointC, unsigned char color)) & PrimitiveMan::DrawTriangleFillPrimitive;
		luaType["DrawTriangleFillPrimitive"] = (void(PrimitiveMan::*)(int player, const Vector& pointA, const Vector& pointB, const Vector& pointC, unsigned char color)) & PrimitiveMan::DrawTriangleFillPrimitive;
		luaType["DrawTextPrimitive"] = (void(PrimitiveMan::*)(const Vector& start, const std::string& text, bool isSmall, int alignment)) & PrimitiveMan::DrawTextPrimitive;
		luaType["DrawTextPrimitive"] = (void(PrimitiveMan::*)(const Vector& start, const std::string& text, bool isSmall, int alignment, float rotAngle)) & PrimitiveMan::DrawTextPrimitive;
		luaType["DrawTextPrimitive"] = (void(PrimitiveMan::*)(int player, const Vector& start, const std::string& text, bool isSmall, int alignment)) & PrimitiveMan::DrawTextPrimitive;
		luaType["DrawTextPrimitive"] = (void(PrimitiveMan::*)(int player, const Vector& start, const std::string& text, bool isSmall, int alignment, float rotAngle)) & PrimitiveMan::DrawTextPrimitive;
		luaType["DrawBitmapPrimitive"] = (void(PrimitiveMan::*)(const Vector& start, const MOSprite* moSprite, float rotAngle, int frame)) & PrimitiveMan::DrawBitmapPrimitive;
		luaType["DrawBitmapPrimitive"] = (void(PrimitiveMan::*)(const Vector& start, const MOSprite* moSprite, float rotAngle, int frame, bool hFlipped, bool vFlipped)) & PrimitiveMan::DrawBitmapPrimitive;
		luaType["DrawBitmapPrimitive"] = (void(PrimitiveMan::*)(int player, const Vector& start, const MOSprite* moSprite, float rotAngle, int frame)) & PrimitiveMan::DrawBitmapPrimitive;
		luaType["DrawBitmapPrimitive"] = (void(PrimitiveMan::*)(int player, const Vector& start, const MOSprite* moSprite, float rotAngle, int frame, bool hFlipped, bool vFlipped)) & PrimitiveMan::DrawBitmapPrimitive;
		luaType["DrawBitmapPrimitive"] = (void(PrimitiveMan::*)(const Vector& start, const std::string& filePath, float rotAngle)) & PrimitiveMan::DrawBitmapPrimitive;
		luaType["DrawBitmapPrimitive"] = (void(PrimitiveMan::*)(const Vector& start, const std::string& filePath, float rotAngle, bool hFlipped, bool vFlipped)) & PrimitiveMan::DrawBitmapPrimitive;
		luaType["DrawBitmapPrimitive"] = (void(PrimitiveMan::*)(int player, const Vector& start, const std::string& filePath, float rotAngle)) & PrimitiveMan::DrawBitmapPrimitive;
		luaType["DrawBitmapPrimitive"] = (void(PrimitiveMan::*)(int player, const Vector& start, const std::string& filePath, float rotAngle, bool hFlipped, bool vFlipped)) & PrimitiveMan::DrawBitmapPrimitive;
		luaType["DrawIconPrimitive"] = (void(PrimitiveMan::*)(const Vector& start, Entity* entity)) & PrimitiveMan::DrawIconPrimitive;
		luaType["DrawIconPrimitive"] = (void(PrimitiveMan::*)(int player, const Vector& start, Entity* entity)) & PrimitiveMan::DrawIconPrimitive;

		luaType["DrawPolygonPrimitive"] = &LuaAdaptersPrimitiveMan::DrawPolygonPrimitive;
		luaType["DrawPolygonPrimitive"] = &LuaAdaptersPrimitiveMan::DrawPolygonPrimitiveForPlayer;
		luaType["DrawPolygonFillPrimitive"] = &LuaAdaptersPrimitiveMan::DrawPolygonFillPrimitive;
		luaType["DrawPolygonFillPrimitive"] = &LuaAdaptersPrimitiveMan::DrawPolygonFillPrimitiveForPlayer;
		luaType["DrawPrimitives"] = &LuaAdaptersPrimitiveMan::DrawPrimitivesWithTransparency;
		luaType["DrawPrimitives"] = &LuaAdaptersPrimitiveMan::DrawPrimitivesWithBlending;
		luaType["DrawPrimitives"] = &LuaAdaptersPrimitiveMan::DrawPrimitivesWithBlendingPerChannel;
	}

	LuaBindingRegisterFunctionDefinitionForType(ManagerLuaBindings, SceneMan) {
		auto luaType = SimpleNamedTypeLuaClassDefinition(SceneMan, "SceneManager");

		luaType["Scene"] = sol::property(&SceneMan::GetScene);
		luaType["SceneDim"] = sol::property(&SceneMan::GetSceneDim);
		luaType["SceneWidth"] = sol::property(&SceneMan::GetSceneWidth);
		luaType["SceneHeight"] = sol::property(&SceneMan::GetSceneHeight);
		luaType["SceneWrapsX"] = sol::property(&SceneMan::SceneWrapsX);
		luaType["SceneWrapsY"] = sol::property(&SceneMan::SceneWrapsY);
		luaType["SceneOrbitDirection"] = sol::property(&SceneMan::GetSceneOrbitDirection);
		luaType["LayerDrawMode"] = sol::property(&SceneMan::GetLayerDrawMode, &SceneMan::SetLayerDrawMode);
		luaType["GlobalAcc"] = sol::property(&SceneMan::GetGlobalAcc);
		luaType["OzPerKg"] = sol::property(&SceneMan::GetOzPerKg);
		luaType["KgPerOz"] = sol::property(&SceneMan::GetKgPerOz);
		luaType["ScrapCompactingHeight"] = sol::property(&SceneMan::GetScrapCompactingHeight, &SceneMan::SetScrapCompactingHeight);

		luaType["LoadScene"] = (int(SceneMan::*)(std::string, bool, bool)) & SceneMan::LoadScene;
		luaType["LoadScene"] = (int(SceneMan::*)(std::string, bool)) & SceneMan::LoadScene;

		luaType["GetTerrain"] = &SceneMan::GetTerrain;
		luaType["GetMaterial"] = &SceneMan::GetMaterial;
		luaType["GetMaterialFromID"] = &SceneMan::GetMaterialFromID;
		luaType["GetTerrMatter"] = &SceneMan::GetTerrMatter;
		luaType["GetMOIDPixel"] = (MOID(SceneMan::*)(int, int)) & SceneMan::GetMOIDPixel;
		luaType["GetMOIDPixel"] = (MOID(SceneMan::*)(int, int, int)) & SceneMan::GetMOIDPixel;
		luaType["SetLayerDrawMode"] = &SceneMan::SetLayerDrawMode;
		luaType["LoadUnseenLayer"] = &SceneMan::LoadUnseenLayer;
		luaType["MakeAllUnseen"] = &SceneMan::MakeAllUnseen;
		luaType["AnythingUnseen"] = &SceneMan::AnythingUnseen;
		luaType["GetUnseenResolution"] = &SceneMan::GetUnseenResolution;
		luaType["IsUnseen"] = &SceneMan::IsUnseen;
		luaType["RevealUnseen"] = &SceneMan::RevealUnseen;
		luaType["RevealUnseenBox"] = &SceneMan::RevealUnseenBox;
		luaType["RestoreUnseen"] = &SceneMan::RestoreUnseen;
		luaType["RestoreUnseenBox"] = &SceneMan::RestoreUnseenBox;
		luaType["CastSeeRay"] = &SceneMan::CastSeeRay;
		luaType["CastUnseeRay"] = &SceneMan::CastUnseeRay;
		luaType["CastUnseenRay"] = &SceneMan::CastUnseenRay;
		luaType["CastMaterialRay"] = (bool(SceneMan::*)(const Vector&, const Vector&, unsigned char, Vector&, int, bool)) & SceneMan::CastMaterialRay;
		luaType["CastMaterialRay"] = (float(SceneMan::*)(const Vector&, const Vector&, unsigned char, int)) & SceneMan::CastMaterialRay;
		luaType["CastNotMaterialRay"] = (bool(SceneMan::*)(const Vector&, const Vector&, unsigned char, Vector&, int, bool)) & SceneMan::CastNotMaterialRay;
		luaType["CastNotMaterialRay"] = (float(SceneMan::*)(const Vector&, const Vector&, unsigned char, int, bool)) & SceneMan::CastNotMaterialRay;
		luaType["CastStrengthSumRay"] = &SceneMan::CastStrengthSumRay;
		luaType["CastMaxStrengthRay"] = (float(SceneMan::*)(const Vector&, const Vector&, int, unsigned char)) & SceneMan::CastMaxStrengthRay;
		luaType["CastMaxStrengthRay"] = (float(SceneMan::*)(const Vector&, const Vector&, int)) & SceneMan::CastMaxStrengthRay;
		luaType["CastStrengthRay"] = &SceneMan::CastStrengthRay;
		luaType["CastWeaknessRay"] = &SceneMan::CastWeaknessRay;
		luaType["CastMORay"] = &SceneMan::CastMORay;
		luaType["CastFindMORay"] = &SceneMan::CastFindMORay;
		luaType["CastObstacleRay"] = &SceneMan::CastObstacleRay;
		luaType["GetLastRayHitPos"] = &SceneMan::GetLastRayHitPos;
		luaType["FindAltitude"] = (float(SceneMan::*)(const Vector&, int, int)) & SceneMan::FindAltitude;
		luaType["FindAltitude"] = (float(SceneMan::*)(const Vector&, int, int, bool)) & SceneMan::FindAltitude;
		luaType["MovePointToGround"] = &SceneMan::MovePointToGround;
		luaType["IsWithinBounds"] = &SceneMan::IsWithinBounds;
		luaType["ForceBounds"] = (bool(SceneMan::*)(Vector&)) & SceneMan::ForceBounds;
		luaType["WrapPosition"] = (bool(SceneMan::*)(Vector&)) & SceneMan::WrapPosition;
		luaType["SnapPosition"] = &SceneMan::SnapPosition;
		luaType["ShortestDistance"] = &SceneMan::ShortestDistance;
		luaType["WrapBox"] = &LuaAdaptersSceneMan::WrapBoxes;
		luaType["ObscuredPoint"] = (bool(SceneMan::*)(Vector&, int)) & SceneMan::ObscuredPoint;
		luaType["ObscuredPoint"] = (bool(SceneMan::*)(int, int, int)) & SceneMan::ObscuredPoint;
		luaType["AddSceneObject"] = &SceneMan::AddSceneObject; //, luabind::adopt(_2);
		luaType["CheckAndRemoveOrphans"] = (int(SceneMan::*)(int, int, int, int, bool)) & SceneMan::RemoveOrphans;
		luaType["DislodgePixel"] = &SceneMan::DislodgePixel;
	}

	LuaBindingRegisterFunctionDefinitionForType(ManagerLuaBindings, CameraMan) {
		auto luaType = SimpleNamedTypeLuaClassDefinition(CameraMan, "CameraManager");

		luaType["GetOffset"] = &CameraMan::GetOffset;
		luaType["SetOffset"] = &CameraMan::SetOffset;
		luaType["GetScreenOcclusion"] = &CameraMan::GetScreenOcclusion;
		luaType["SetScreenOcclusion"] = &CameraMan::SetScreenOcclusion;
		luaType["GetScrollTarget"] = &CameraMan::GetScrollTarget;
		luaType["SetScrollTarget"] = &CameraMan::SetScrollTarget;
		luaType["TargetDistanceScalar"] = &CameraMan::TargetDistanceScalar;
		luaType["CheckOffset"] = &CameraMan::CheckOffset;
		luaType["SetScroll"] = &CameraMan::SetScroll;
		luaType["AddScreenShake"] = (void(CameraMan::*)(float, int)) & CameraMan::AddScreenShake;
		luaType["AddScreenShake"] = (void(CameraMan::*)(float, const Vector&)) & CameraMan::AddScreenShake;
	}

	LuaBindingRegisterFunctionDefinitionForType(ManagerLuaBindings, SettingsMan) {
		auto luaType = SimpleNamedTypeLuaClassDefinition(SettingsMan, "SettingsManager");

		luaType["PrintDebugInfo"] = sol::property(&SettingsMan::PrintDebugInfo, &SettingsMan::SetPrintDebugInfo);
		luaType["RecommendedMOIDCount"] = sol::property(&SettingsMan::RecommendedMOIDCount);
		luaType["AIUpdateInterval"] = sol::property(&SettingsMan::GetAIUpdateInterval, &SettingsMan::SetAIUpdateInterval);
		luaType["ShowEnemyHUD"] = sol::property(&SettingsMan::ShowEnemyHUD);
		luaType["AutomaticGoldDeposit"] = sol::property(&SettingsMan::GetAutomaticGoldDeposit);
	}

	LuaBindingRegisterFunctionDefinitionForType(ManagerLuaBindings, TimerMan) {
		auto luaType = SimpleNamedTypeLuaClassDefinition(TimerMan, "TimerManager");

		luaType["TimeScale"] = sol::property(&TimerMan::GetTimeScale, &TimerMan::SetTimeScale);
		luaType["RealToSimCap"] = sol::property(&TimerMan::GetRealToSimCap);
		luaType["DeltaTimeTicks"] = sol::property(&LuaAdaptersTimerMan::GetDeltaTimeTicks, &TimerMan::SetDeltaTimeTicks);
		luaType["DeltaTimeSecs"] = sol::property(&TimerMan::GetDeltaTimeSecs, &TimerMan::SetDeltaTimeSecs);
		luaType["DeltaTimeMS"] = sol::property(&TimerMan::GetDeltaTimeMS);
		luaType["AIDeltaTimeSecs"] = sol::property(&TimerMan::GetAIDeltaTimeSecs);
		luaType["AIDeltaTimeMS"] = sol::property(&TimerMan::GetAIDeltaTimeMS);

		luaType["TicksPerSecond"] = sol::property(&LuaAdaptersTimerMan::GetTicksPerSecond);

		luaType["TimeForSimUpdate"] = &TimerMan::TimeForSimUpdate;
		luaType["DrawnSimUpdate"] = &TimerMan::DrawnSimUpdate;
	}

	LuaBindingRegisterFunctionDefinitionForType(ManagerLuaBindings, UInputMan) {
		auto luaType = SimpleNamedTypeLuaClassDefinition(UInputMan, "UInputManager");

		luaType["FlagAltState"] = sol::property(&UInputMan::FlagAltState);
		luaType["FlagCtrlState"] = sol::property(&UInputMan::FlagCtrlState);
		luaType["FlagShiftState"] = sol::property(&UInputMan::FlagShiftState);

		luaType["GetInputDevice"] = &UInputMan::GetInputDevice;
		luaType["ElementPressed"] = &UInputMan::ElementPressed;
		luaType["ElementReleased"] = &UInputMan::ElementReleased;
		luaType["ElementHeld"] = &UInputMan::ElementHeld;
		luaType["KeyPressed"] = (bool(UInputMan::*)(SDL_Keycode) const) & UInputMan::KeyPressed;
		luaType["KeyReleased"] = (bool(UInputMan::*)(SDL_Keycode) const) & UInputMan::KeyReleased;
		luaType["KeyHeld"] = (bool(UInputMan::*)(SDL_Keycode) const) & UInputMan::KeyHeld;
		luaType["ScancodePressed"] = (bool(UInputMan::*)(SDL_Scancode) const) & UInputMan::KeyPressed;
		luaType["ScancodeReleased"] = (bool(UInputMan::*)(SDL_Scancode) const) & UInputMan::KeyReleased;
		luaType["ScancodeHeld"] = (bool(UInputMan::*)(SDL_Scancode) const) & UInputMan::KeyHeld;
		luaType["MouseButtonPressed"] = &UInputMan::MouseButtonPressed;
		luaType["MouseButtonReleased"] = &UInputMan::MouseButtonReleased;
		luaType["MouseButtonHeld"] = &UInputMan::MouseButtonHeld;
		luaType["GetMousePos"] = &UInputMan::GetAbsoluteMousePosition;
		luaType["MouseWheelMoved"] = &UInputMan::MouseWheelMoved;
		luaType["JoyButtonPressed"] = &UInputMan::JoyButtonPressed;
		luaType["JoyButtonReleased"] = &UInputMan::JoyButtonReleased;
		luaType["JoyButtonHeld"] = &UInputMan::JoyButtonHeld;
		luaType["WhichJoyButtonPressed"] = &UInputMan::WhichJoyButtonPressed;
		luaType["JoyDirectionPressed"] = &UInputMan::JoyDirectionPressed;
		luaType["JoyDirectionReleased"] = &UInputMan::JoyDirectionReleased;
		luaType["JoyDirectionHeld"] = &UInputMan::JoyDirectionHeld;
		luaType["AnalogMoveValues"] = &UInputMan::AnalogMoveValues;
		luaType["AnalogAimValues"] = &UInputMan::AnalogAimValues;
		luaType["SetMouseValueMagnitude"] = &UInputMan::SetMouseValueMagnitude;
		luaType["AnalogAxisValue"] = &UInputMan::AnalogAxisValue;
		luaType["MouseUsedByPlayer"] = &UInputMan::MouseUsedByPlayer;
		luaType["AnyMouseButtonPress"] = &UInputMan::AnyMouseButtonPress;
		luaType["GetMouseMovement"] = &UInputMan::GetMouseMovement;
		luaType["DisableMouseMoving"] = &UInputMan::DisableMouseMoving;
		luaType["SetMousePos"] = &UInputMan::SetMousePos;
		luaType["ForceMouseWithinBox"] = &UInputMan::ForceMouseWithinBox;
		luaType["AnyKeyPress"] = &UInputMan::AnyKeyPress;
		luaType["AnyJoyInput"] = &UInputMan::AnyJoyInput;
		luaType["AnyJoyPress"] = &UInputMan::AnyJoyPress;
		luaType["AnyJoyButtonPress"] = &UInputMan::AnyJoyButtonPress;
		luaType["AnyInput"] = &UInputMan::AnyKeyOrJoyInput;
		luaType["AnyPress"] = &UInputMan::AnyPress;
		luaType["AnyStartPress"] = &UInputMan::AnyStartPress;
		luaType["HasTextInput"] = &UInputMan::HasTextInput;
		luaType["GetTextInput"] = (const std::string& (UInputMan::*)() const) & UInputMan::GetTextInput;

		luaType["MouseButtonPressed"] = &LuaAdaptersUInputMan::MouseButtonPressed;
		luaType["MouseButtonReleased"] = &LuaAdaptersUInputMan::MouseButtonReleased;
		luaType["MouseButtonHeld"] = &LuaAdaptersUInputMan::MouseButtonHeld;
	}
} // namespace RTE
