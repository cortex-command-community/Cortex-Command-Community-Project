// Make sure that binding definition files are always set to NOT use pre-compiled headers and conformance mode (/permissive) otherwise everything will be on fire!

#include "LuaBindingRegisterDefinitions.h"

using namespace RTE;

LuaBindingRegisterFunctionDefinitionForType(ManagerLuaBindings, ActivityMan) {
	registry.Class<ActivityMan>("ActivityManager")

	    .addProperty("DefaultActivityType", &ActivityMan::GetDefaultActivityType, &ActivityMan::SetDefaultActivityType)
	    .addProperty("DefaultActivityName", &ActivityMan::GetDefaultActivityName, &ActivityMan::SetDefaultActivityName)

	    .addFunction("SetStartActivity", &ActivityMan::SetStartActivity, LuaBinding::AdoptArg<2>) // Transfers ownership of the Activity to start into the ActivityMan, adopts ownership (_1 is the this ptr)
	    .addFunction("GetStartActivity", &ActivityMan::GetStartActivity)
	    .addFunction("GetActivity", &ActivityMan::GetActivity)
	    .addFunction("StartActivity", (int(ActivityMan::*)(Activity*)) & ActivityMan::StartActivity, LuaBinding::AdoptArg<2>) // Transfers ownership of the Activity to start into the ActivityMan, adopts ownership (_1 is the this ptr)
	    .addFunction("StartActivity", (int(ActivityMan::*)(const std::string&, const std::string&)) & ActivityMan::StartActivity)
	    .addFunction("RestartActivity", &ActivityMan::RestartActivity)
	    .addFunction("PauseActivity", &ActivityMan::PauseActivity)
	    .addFunction("EndActivity", &ActivityMan::EndActivity)
	    .addFunction("ActivityRunning", &ActivityMan::ActivityRunning)
	    .addFunction("ActivityPaused", &ActivityMan::ActivityPaused)
	    .addFunction("SaveGame", &ActivityMan::SaveCurrentGame)
	    .addFunction("LoadGame", &ActivityMan::LoadAndLaunchGame);
}

LuaBindingRegisterFunctionDefinitionForType(ManagerLuaBindings, AudioMan) {
	registry.Class<AudioMan>("AudioManager")

	    .addProperty("MusicVolume", &AudioMan::GetMusicVolume, &AudioMan::SetMusicVolume)
	    .addProperty("SoundsVolume", &AudioMan::GetSoundsVolume, &AudioMan::SetSoundsVolume)

	    .addFunction("StopAll", &AudioMan::StopAll)
	    .addFunction("GetGlobalPitch", &AudioMan::GetGlobalPitch)
	    .addFunction("IsMusicPlaying", &AudioMan::IsMusicPlaying)
	    .addFunction("SetMusicPitch", &AudioMan::SetMusicPitch)
	    .addFunction("SetMusicMuffledState", &AudioMan::SetMusicMuffledState)
	    .addFunction("PlaySound", (SoundContainer * (AudioMan::*)(const std::string& filePath)) & AudioMan::PlaySound, LuaBinding::AdoptResult)
	    .addFunction("PlaySound", (SoundContainer * (AudioMan::*)(const std::string& filePath, const Vector& position)) & AudioMan::PlaySound, LuaBinding::AdoptResult)
	    .addFunction("PlaySound", (SoundContainer * (AudioMan::*)(const std::string& filePath, const Vector& position, int player)) & AudioMan::PlaySound, LuaBinding::AdoptResult);
}

LuaBindingRegisterFunctionDefinitionForType(ManagerLuaBindings, MusicMan) {
	registry.Class<MusicMan>("MusicManager")

	    .addFunction("ResetMusicState", &MusicMan::ResetMusicState)
	    .addFunction("IsMusicPlaying", &MusicMan::IsMusicPlaying)
	    .addFunction("PlayDynamicSong", &LuaAdaptersMusicMan::PlayDynamicSong1)
	    .addFunction("PlayDynamicSong", &LuaAdaptersMusicMan::PlayDynamicSong2)
	    .addFunction("PlayDynamicSong", &LuaAdaptersMusicMan::PlayDynamicSong3)
	    .addFunction("PlayDynamicSong", &LuaAdaptersMusicMan::PlayDynamicSong4)
	    .addFunction("PlayDynamicSong", &LuaAdaptersMusicMan::PlayDynamicSong5)
	    .addFunction("SetNextDynamicSongSection", &LuaAdaptersMusicMan::SetNextDynamicSongSection1)
	    .addFunction("SetNextDynamicSongSection", &LuaAdaptersMusicMan::SetNextDynamicSongSection2)
	    .addFunction("SetNextDynamicSongSection", &LuaAdaptersMusicMan::SetNextDynamicSongSection3)
	    .addFunction("SetNextDynamicSongSection", &LuaAdaptersMusicMan::SetNextDynamicSongSection4)
	    .addFunction("CyclePlayingSoundContainers", &LuaAdaptersMusicMan::CyclePlayingSoundContainers1)
	    .addFunction("CyclePlayingSoundContainers", &LuaAdaptersMusicMan::CyclePlayingSoundContainers2)
	    .addFunction("GetCurrentDynamicSongSectionType", &MusicMan::GetCurrentSongSectionType)
	    .addFunction("EndDynamicMusic", &LuaAdaptersMusicMan::EndDynamicMusic1)
	    .addFunction("EndDynamicMusic", &LuaAdaptersMusicMan::EndDynamicMusic2)
	    .addFunction("PlayInterruptingMusic", &MusicMan::PlayInterruptingMusic)
	    .addFunction("EndInterruptingMusic", &MusicMan::EndInterruptingMusic);
}

LuaBindingRegisterFunctionDefinitionForType(ManagerLuaBindings, ConsoleMan) {
	registry.Class<ConsoleMan>("ConsoleManager")

	    .addFunction("PrintString", &ConsoleMan::PrintString)
	    .addFunction("SaveInputLog", &ConsoleMan::SaveInputLog)
	    .addFunction("SaveAllText", &ConsoleMan::SaveAllText)
	    .addFunction("Clear", &ConsoleMan::ClearLog);
}

LuaBindingRegisterFunctionDefinitionForType(ManagerLuaBindings, FrameMan) {
	registry.Class<FrameMan>("FrameManager")

	    .addProperty("PlayerScreenWidth", &FrameMan::GetPlayerScreenWidth)
	    .addProperty("PlayerScreenHeight", &FrameMan::GetPlayerScreenHeight)
	    .addProperty("ScreenCount", &FrameMan::GetScreenCount)
	    .addProperty("ResolutionMultiplier", &FrameMan::GetResolutionMultiplier)

	    .addFunction("IsHudDisabled", &FrameMan::IsHudDisabled)
	    .addFunction("SetHudDisabled", &FrameMan::SetHudDisabled)
	    .addFunction("LoadPalette", &FrameMan::LoadPalette)
	    .addFunction("SetScreenText", &FrameMan::SetScreenText)
	    .addFunction("ClearScreenText", &FrameMan::ClearScreenText)
	    .addFunction("FadeInPalette", &FrameMan::FadeInPalette)
	    .addFunction("FadeOutPalette", &FrameMan::FadeOutPalette)
	    .addFunction("SaveScreenToPNG", &FrameMan::SaveScreenToPNG)
	    .addFunction("SaveBitmapToPNG", &FrameMan::SaveBitmapToPNG)
	    .addFunction("FlashScreen", &FrameMan::FlashScreen)
	    .addFunction("CalculateTextHeight", &FrameMan::CalculateTextHeight)
	    .addFunction("CalculateTextWidth", &FrameMan::CalculateTextWidth)
	    .addFunction("SplitStringToFitWidth", &FrameMan::SplitStringToFitWidth);
}

LuaBindingRegisterFunctionDefinitionForType(ManagerLuaBindings, MetaMan) {
	registry.Class<MetaMan>("MetaManager")

	    .addProperty("GameName", &MetaMan::GetGameName, &MetaMan::SetGameName)
	    .addProperty("PlayerTurn", &MetaMan::GetPlayerTurn)
	    .addProperty("PlayerCount", &MetaMan::GetPlayerCount)

	    .addProperty("Players", &MetaMan::m_Players, LuaBinding::ReturnStlIterator)

	    .addFunction("GetTeamOfPlayer", &MetaMan::GetTeamOfPlayer)
	    .addFunction("GetPlayer", &MetaMan::GetPlayer)
	    .addFunction("GetMetaPlayerOfInGamePlayer", &MetaMan::GetMetaPlayerOfInGamePlayer);
}

LuaBindingRegisterFunctionDefinitionForType(ManagerLuaBindings, MovableMan) {
	registry.Class<MovableMan>("MovableManager")

	    .addProperty("MaxDroppedItems", &MovableMan::GetMaxDroppedItems, &MovableMan::SetMaxDroppedItems)

	    .addProperty("Actors", &MovableMan::m_Actors, LuaBinding::ReturnStlIterator)
	    .addProperty("Items", &MovableMan::m_Items, LuaBinding::ReturnStlIterator)
	    .addProperty("Particles", &MovableMan::m_Particles, LuaBinding::ReturnStlIterator)
	    .addProperty("AddedActors", &MovableMan::m_AddedActors, LuaBinding::ReturnStlIterator)
	    .addProperty("AddedItems", &MovableMan::m_AddedItems, LuaBinding::ReturnStlIterator)
	    .addProperty("AddedParticles", &MovableMan::m_AddedParticles, LuaBinding::ReturnStlIterator)
	    .addProperty("AlarmEvents", &MovableMan::m_AlarmEvents, LuaBinding::ReturnStlIterator)
	    .addProperty("AddedAlarmEvents", &MovableMan::m_AddedAlarmEvents, LuaBinding::ReturnStlIterator)

	    .addFunction("GetMOFromID", &MovableMan::GetMOFromID)
	    .addFunction("FindObjectByUniqueID", &MovableMan::FindObjectByUniqueID)
	    .addFunction("GetMOIDCount", &MovableMan::GetMOIDCount)
	    .addFunction("GetTeamMOIDCount", &MovableMan::GetTeamMOIDCount)
	    .addFunction("PurgeAllMOs", &MovableMan::PurgeAllMOs)
	    .addFunction("GetNextActorInGroup", &MovableMan::GetNextActorInGroup)
	    .addFunction("GetPrevActorInGroup", &MovableMan::GetPrevActorInGroup)
	    .addFunction("GetNextTeamActor", &MovableMan::GetNextTeamActor)
	    .addFunction("GetPrevTeamActor", &MovableMan::GetPrevTeamActor)
	    .addFunction("GetClosestTeamActor", (Actor * (MovableMan::*)(int team, int player, const Vector& scenePoint, int maxRadius, Vector& getDistance, const Actor* excludeThis)) & MovableMan::GetClosestTeamActor)
	    .addFunction("GetClosestTeamActor", (Actor * (MovableMan::*)(int team, int player, const Vector& scenePoint, int maxRadius, Vector& getDistance, bool onlyPlayerControllableActors, const Actor* excludeThis)) & MovableMan::GetClosestTeamActor)
	    .addFunction("GetClosestEnemyActor", &MovableMan::GetClosestEnemyActor)
	    .addFunction("GetFirstTeamActor", &MovableMan::GetFirstTeamActor)
	    .addFunction("GetClosestActor", &MovableMan::GetClosestActor)
	    .addFunction("GetClosestBrainActor", &MovableMan::GetClosestBrainActor)
	    .addFunction("GetFirstBrainActor", &MovableMan::GetFirstBrainActor)
	    .addFunction("GetClosestOtherBrainActor", &MovableMan::GetClosestOtherBrainActor)
	    .addFunction("GetFirstOtherBrainActor", &MovableMan::GetFirstOtherBrainActor)
	    .addFunction("GetUnassignedBrain", &MovableMan::GetUnassignedBrain)
	    .addFunction("GetParticleCount", &MovableMan::GetParticleCount)
	    .addFunction("GetSplashRatio", &MovableMan::GetSplashRatio)
	    .addFunction("SortTeamRoster", &MovableMan::SortTeamRoster)
	    .addFunction("ChangeActorTeam", &MovableMan::ChangeActorTeam)
	    .addFunction("RemoveActor", &MovableMan::RemoveActor, LuaBinding::AdoptResult)
	    .addFunction("RemoveItem", &MovableMan::RemoveItem, LuaBinding::AdoptResult)
	    .addFunction("RemoveParticle", &MovableMan::RemoveParticle, LuaBinding::AdoptResult)
	    .addFunction("ValidMO", &MovableMan::ValidMO)
	    .addFunction("IsActor", &MovableMan::IsActor)
	    .addFunction("IsDevice", &MovableMan::IsDevice)
	    .addFunction("IsParticle", &MovableMan::IsParticle)
	    .addFunction("IsOfActor", &MovableMan::IsOfActor)
	    .addFunction("GetRootMOID", &MovableMan::GetRootMOID)
	    .addFunction("RemoveMO", &MovableMan::RemoveMO)
	    .addFunction("KillAllTeamActors", &MovableMan::KillAllTeamActors)
	    .addFunction("KillAllEnemyActors", &MovableMan::KillAllEnemyActors)
	    .addFunction("OpenAllDoors", &MovableMan::OpenAllDoors)
	    .addFunction("IsParticleSettlingEnabled", &MovableMan::IsParticleSettlingEnabled)
	    .addFunction("EnableParticleSettling", &MovableMan::EnableParticleSettling)
	    .addFunction("IsMOSubtractionEnabled", &MovableMan::IsMOSubtractionEnabled)
	    .addFunction("GetMOsInBox", (const std::vector<MovableObject*>* (MovableMan::*)(const Box& box) const) & MovableMan::GetMOsInBox, LuaBinding::AdoptResult, LuaBinding::ReturnStlIterator)
	    .addFunction("GetMOsInBox", (const std::vector<MovableObject*>* (MovableMan::*)(const Box& box, int ignoreTeam) const) & MovableMan::GetMOsInBox, LuaBinding::AdoptResult, LuaBinding::ReturnStlIterator)
	    .addFunction("GetMOsInBox", (const std::vector<MovableObject*>* (MovableMan::*)(const Box& box, int ignoreTeam, bool getsHitByMOsOnly) const) & MovableMan::GetMOsInBox, LuaBinding::AdoptResult, LuaBinding::ReturnStlIterator)
	    .addFunction("GetMOsInRadius", (const std::vector<MovableObject*>* (MovableMan::*)(const Vector& centre, float radius) const) & MovableMan::GetMOsInRadius, LuaBinding::AdoptResult, LuaBinding::ReturnStlIterator)
	    .addFunction("GetMOsInRadius", (const std::vector<MovableObject*>* (MovableMan::*)(const Vector& centre, float radius, int ignoreTeam) const) & MovableMan::GetMOsInRadius, LuaBinding::AdoptResult, LuaBinding::ReturnStlIterator)
	    .addFunction("GetMOsInRadius", (const std::vector<MovableObject*>* (MovableMan::*)(const Vector& centre, float radius, int ignoreTeam, bool getsHitByMOsOnly) const) & MovableMan::GetMOsInRadius, LuaBinding::AdoptResult, LuaBinding::ReturnStlIterator)
	    .addFunction("GetMOsAtPosition", (const std::vector<MovableObject*>* (MovableMan::*)(int pixelX, int pixelY) const) & MovableMan::GetMOsAtPosition, LuaBinding::AdoptResult, LuaBinding::ReturnStlIterator)
	    .addFunction("GetMOsAtPosition", (const std::vector<MovableObject*>* (MovableMan::*)(int pixelX, int pixelY, int ignoreTeam) const) & MovableMan::GetMOsAtPosition, LuaBinding::AdoptResult, LuaBinding::ReturnStlIterator)
	    .addFunction("GetMOsAtPosition", (const std::vector<MovableObject*>* (MovableMan::*)(int pixelX, int pixelY, int ignoreTeam, bool getsHitByMOsOnly) const) & MovableMan::GetMOsAtPosition, LuaBinding::AdoptResult, LuaBinding::ReturnStlIterator)

	    .addFunction("SendGlobalMessage", &LuaAdaptersMovableMan::SendGlobalMessage1)
	    .addFunction("SendGlobalMessage", &LuaAdaptersMovableMan::SendGlobalMessage2)
	    .addFunction("AddMO", &LuaAdaptersMovableMan::AddMO, LuaBinding::AdoptArg<2>)
	    .addFunction("AddActor", &LuaAdaptersMovableMan::AddActor, LuaBinding::AdoptArg<2>)
	    .addFunction("AddItem", &LuaAdaptersMovableMan::AddItem, LuaBinding::AdoptArg<2>)
	    .addFunction("AddParticle", &LuaAdaptersMovableMan::AddParticle, LuaBinding::AdoptArg<2>);
}

LuaBindingRegisterFunctionDefinitionForType(ManagerLuaBindings, PerformanceMan) {
	registry.Class<PerformanceMan>("PerformanceManager")

	    .addProperty("ShowPerformanceStats", &PerformanceMan::IsShowingPerformanceStats, &PerformanceMan::ShowPerformanceStats);
}

LuaBindingRegisterFunctionDefinitionForType(ManagerLuaBindings, PostProcessMan) {
	registry.Class<PostProcessMan>("PostProcessManager")

	    .addFunction("RegisterPostEffect", &PostProcessMan::RegisterPostEffect);
}

LuaBindingRegisterFunctionDefinitionForType(ManagerLuaBindings, PresetMan) {
	registry.Class<PresetMan>("PresetManager")

	    .addProperty("Modules", &PresetMan::m_pDataModules, LuaBinding::ReturnStlIterator)

	    .addFunction("LoadDataModule", (bool(PresetMan::*)(const std::string&)) & PresetMan::LoadDataModule)
	    .addFunction("GetDataModule", &PresetMan::GetDataModule)
	    .addFunction("GetModuleID", &PresetMan::GetModuleID)
	    .addFunction("GetModuleIDFromPath", &PresetMan::GetModuleIDFromPath)
	    .addFunction("GetTotalModuleCount", &PresetMan::GetTotalModuleCount)
	    .addFunction("GetOfficialModuleCount", &PresetMan::GetOfficialModuleCount)
	    .addFunction("AddPreset", &PresetMan::AddEntityPreset)
	    .addFunction("GetPreset", (const Entity* (PresetMan::*)(const std::string&, std::string, int)) & PresetMan::GetEntityPreset)
	    .addFunction("GetPreset", (const Entity* (PresetMan::*)(const std::string&, std::string, std::string)) & PresetMan::GetEntityPreset)
	    .addFunction("GetLoadout", (Actor * (PresetMan::*)(std::string, std::string, bool)) & PresetMan::GetLoadout, LuaBinding::AdoptResult)
	    .addFunction("GetLoadout", (Actor * (PresetMan::*)(std::string, int, bool)) & PresetMan::GetLoadout, LuaBinding::AdoptResult)
	    .addFunction("GetRandomOfGroup", &PresetMan::GetRandomOfGroup)
	    .addFunction("GetRandomOfGroupInModuleSpace", &PresetMan::GetRandomOfGroupInModuleSpace)
	    .addFunction("GetEntityDataLocation", &PresetMan::GetEntityDataLocation)
	    .addFunction("ReadReflectedPreset", &PresetMan::ReadReflectedPreset)
	    .addFunction("ReloadEntityPreset", &LuaAdaptersPresetMan::ReloadEntityPreset1)
	    .addFunction("ReloadEntityPreset", &LuaAdaptersPresetMan::ReloadEntityPreset2)
	    .addFunction("GetAllEntities", &LuaAdaptersPresetMan::GetAllEntities, LuaBinding::AdoptResult, LuaBinding::ReturnStlIterator)
	    .addFunction("GetAllEntitiesOfGroup", &LuaAdaptersPresetMan::GetAllEntitiesOfGroup, LuaBinding::AdoptResult, LuaBinding::ReturnStlIterator)
	    .addFunction("GetAllEntitiesOfGroup", &LuaAdaptersPresetMan::GetAllEntitiesOfGroup2, LuaBinding::AdoptResult, LuaBinding::ReturnStlIterator)
	    .addFunction("GetAllEntitiesOfGroup", &LuaAdaptersPresetMan::GetAllEntitiesOfGroup3, LuaBinding::AdoptResult, LuaBinding::ReturnStlIterator)
	    .addFunction("ReloadAllScripts", &PresetMan::ReloadAllScripts)
	    .addFunction("IsModuleOfficial", &PresetMan::IsModuleOfficial)
	    .addFunction("IsModuleUserdata", &PresetMan::IsModuleUserdata)
	    .addFunction("GetFullModulePath", &PresetMan::GetFullModulePath);
}

LuaBindingRegisterFunctionDefinitionForType(ManagerLuaBindings, PrimitiveMan) {
	registry.Class<PrimitiveMan>("PrimitiveManager")

	    .addFunction("DrawLinePrimitive", (void(PrimitiveMan::*)(const Vector& start, const Vector& end, unsigned char color)) & PrimitiveMan::DrawLinePrimitive)
	    .addFunction("DrawLinePrimitive", (void(PrimitiveMan::*)(const Vector& start, const Vector& end, unsigned char color, int thickness)) & PrimitiveMan::DrawLinePrimitive)
	    .addFunction("DrawLinePrimitive", (void(PrimitiveMan::*)(int player, const Vector& start, const Vector& end, unsigned char color)) & PrimitiveMan::DrawLinePrimitive)
	    .addFunction("DrawLinePrimitive", (void(PrimitiveMan::*)(int player, const Vector& start, const Vector& end, unsigned char color, int thickness)) & PrimitiveMan::DrawLinePrimitive)
	    .addFunction("DrawArcPrimitive", (void(PrimitiveMan::*)(const Vector& pos, float startAngle, float endAngle, int radius, unsigned char color)) & PrimitiveMan::DrawArcPrimitive)
	    .addFunction("DrawArcPrimitive", (void(PrimitiveMan::*)(const Vector& pos, float startAngle, float endAngle, int radius, unsigned char color, int thickness)) & PrimitiveMan::DrawArcPrimitive)
	    .addFunction("DrawArcPrimitive", (void(PrimitiveMan::*)(int player, const Vector& pos, float startAngle, float endAngle, int radius, unsigned char color)) & PrimitiveMan::DrawArcPrimitive)
	    .addFunction("DrawArcPrimitive", (void(PrimitiveMan::*)(int player, const Vector& pos, float startAngle, float endAngle, int radius, unsigned char color, int thickness)) & PrimitiveMan::DrawArcPrimitive)
	    .addFunction("DrawSplinePrimitive", (void(PrimitiveMan::*)(const Vector& start, const Vector& guideA, const Vector& guideB, const Vector& end, unsigned char color)) & PrimitiveMan::DrawSplinePrimitive)
	    .addFunction("DrawSplinePrimitive", (void(PrimitiveMan::*)(int player, const Vector& start, const Vector& guideA, const Vector& guideB, const Vector& end, unsigned char color)) & PrimitiveMan::DrawSplinePrimitive)
	    .addFunction("DrawBoxPrimitive", (void(PrimitiveMan::*)(const Vector& start, const Vector& end, unsigned char color)) & PrimitiveMan::DrawBoxPrimitive)
	    .addFunction("DrawBoxPrimitive", (void(PrimitiveMan::*)(int player, const Vector& start, const Vector& end, unsigned char color)) & PrimitiveMan::DrawBoxPrimitive)
	    .addFunction("DrawBoxFillPrimitive", (void(PrimitiveMan::*)(const Vector& start, const Vector& end, unsigned char color)) & PrimitiveMan::DrawBoxFillPrimitive)
	    .addFunction("DrawBoxFillPrimitive", (void(PrimitiveMan::*)(int player, const Vector& start, const Vector& end, unsigned char color)) & PrimitiveMan::DrawBoxFillPrimitive)
	    .addFunction("DrawRoundedBoxPrimitive", (void(PrimitiveMan::*)(const Vector& start, const Vector& end, int cornerRadius, unsigned char color)) & PrimitiveMan::DrawRoundedBoxPrimitive)
	    .addFunction("DrawRoundedBoxPrimitive", (void(PrimitiveMan::*)(int player, const Vector& start, const Vector& end, int cornerRadius, unsigned char color)) & PrimitiveMan::DrawRoundedBoxPrimitive)
	    .addFunction("DrawRoundedBoxFillPrimitive", (void(PrimitiveMan::*)(const Vector& start, const Vector& end, int cornerRadius, unsigned char color)) & PrimitiveMan::DrawRoundedBoxFillPrimitive)
	    .addFunction("DrawRoundedBoxFillPrimitive", (void(PrimitiveMan::*)(int player, const Vector& start, const Vector& end, int cornerRadius, unsigned char color)) & PrimitiveMan::DrawRoundedBoxFillPrimitive)
	    .addFunction("DrawCirclePrimitive", (void(PrimitiveMan::*)(const Vector& pos, int radius, unsigned char color)) & PrimitiveMan::DrawCirclePrimitive)
	    .addFunction("DrawCirclePrimitive", (void(PrimitiveMan::*)(int player, const Vector& pos, int radius, unsigned char color)) & PrimitiveMan::DrawCirclePrimitive)
	    .addFunction("DrawCircleFillPrimitive", (void(PrimitiveMan::*)(const Vector& pos, int radius, unsigned char color)) & PrimitiveMan::DrawCircleFillPrimitive)
	    .addFunction("DrawCircleFillPrimitive", (void(PrimitiveMan::*)(int player, const Vector& pos, int radius, unsigned char color)) & PrimitiveMan::DrawCircleFillPrimitive)
	    .addFunction("DrawEllipsePrimitive", (void(PrimitiveMan::*)(const Vector& pos, int horizRadius, int vertRadius, unsigned char color)) & PrimitiveMan::DrawEllipsePrimitive)
	    .addFunction("DrawEllipsePrimitive", (void(PrimitiveMan::*)(int player, const Vector& pos, int horizRadius, int vertRadius, unsigned char color)) & PrimitiveMan::DrawEllipsePrimitive)
	    .addFunction("DrawEllipseFillPrimitive", (void(PrimitiveMan::*)(const Vector& pos, int horizRadius, int vertRadius, unsigned char color)) & PrimitiveMan::DrawEllipseFillPrimitive)
	    .addFunction("DrawEllipseFillPrimitive", (void(PrimitiveMan::*)(int player, const Vector& pos, int horizRadius, int vertRadius, unsigned char color)) & PrimitiveMan::DrawEllipseFillPrimitive)
	    .addFunction("DrawTrianglePrimitive", (void(PrimitiveMan::*)(const Vector& pointA, const Vector& pointB, const Vector& pointC, unsigned char color)) & PrimitiveMan::DrawTrianglePrimitive)
	    .addFunction("DrawTrianglePrimitive", (void(PrimitiveMan::*)(int player, const Vector& pointA, const Vector& pointB, const Vector& pointC, unsigned char color)) & PrimitiveMan::DrawTrianglePrimitive)
	    .addFunction("DrawTriangleFillPrimitive", (void(PrimitiveMan::*)(const Vector& pointA, const Vector& pointB, const Vector& pointC, unsigned char color)) & PrimitiveMan::DrawTriangleFillPrimitive)
	    .addFunction("DrawTriangleFillPrimitive", (void(PrimitiveMan::*)(int player, const Vector& pointA, const Vector& pointB, const Vector& pointC, unsigned char color)) & PrimitiveMan::DrawTriangleFillPrimitive)
	    .addFunction("DrawTextPrimitive", (void(PrimitiveMan::*)(const Vector& start, const std::string& text, bool isSmall, int alignment)) & PrimitiveMan::DrawTextPrimitive)
	    .addFunction("DrawTextPrimitive", (void(PrimitiveMan::*)(const Vector& start, const std::string& text, bool isSmall, int alignment, float rotAngle)) & PrimitiveMan::DrawTextPrimitive)
	    .addFunction("DrawTextPrimitive", (void(PrimitiveMan::*)(int player, const Vector& start, const std::string& text, bool isSmall, int alignment)) & PrimitiveMan::DrawTextPrimitive)
	    .addFunction("DrawTextPrimitive", (void(PrimitiveMan::*)(int player, const Vector& start, const std::string& text, bool isSmall, int alignment, float rotAngle)) & PrimitiveMan::DrawTextPrimitive)
	    .addFunction("DrawBitmapPrimitive", (void(PrimitiveMan::*)(const Vector& start, const MOSprite* moSprite, float rotAngle, unsigned int frame)) & PrimitiveMan::DrawBitmapPrimitive)
	    .addFunction("DrawBitmapPrimitive", (void(PrimitiveMan::*)(const Vector& start, const MOSprite* moSprite, float rotAngle, unsigned int frame, float scale)) & PrimitiveMan::DrawBitmapPrimitive)
	    .addFunction("DrawBitmapPrimitive", (void(PrimitiveMan::*)(const Vector& start, const MOSprite* moSprite, float rotAngle, unsigned int frame, bool hFlipped, bool vFlipped)) & PrimitiveMan::DrawBitmapPrimitive)
	    .addFunction("DrawBitmapPrimitive", (void(PrimitiveMan::*)(const Vector& start, const MOSprite* moSprite, float rotAngle, unsigned int frame, float scale, bool hFlipped, bool vFlipped)) & PrimitiveMan::DrawBitmapPrimitive)
	    .addFunction("DrawBitmapPrimitive", (void(PrimitiveMan::*)(int player, const Vector& start, const MOSprite* moSprite, float rotAngle, unsigned int frame)) & PrimitiveMan::DrawBitmapPrimitive)
	    .addFunction("DrawBitmapPrimitive", (void(PrimitiveMan::*)(int player, const Vector& start, const MOSprite* moSprite, float rotAngle, unsigned int frame, float scale)) & PrimitiveMan::DrawBitmapPrimitive)
	    .addFunction("DrawBitmapPrimitive", (void(PrimitiveMan::*)(int player, const Vector& start, const MOSprite* moSprite, float rotAngle, unsigned int frame, bool hFlipped, bool vFlipped)) & PrimitiveMan::DrawBitmapPrimitive)
	    .addFunction("DrawBitmapPrimitive", (void(PrimitiveMan::*)(int player, const Vector& start, const MOSprite* moSprite, float rotAngle, unsigned int frame, float scale, bool hFlipped, bool vFlipped)) & PrimitiveMan::DrawBitmapPrimitive)
	    .addFunction("DrawBitmapPrimitive", (void(PrimitiveMan::*)(const Vector& start, const std::string& filePath, float rotAngle)) & PrimitiveMan::DrawBitmapPrimitive)
	    .addFunction("DrawBitmapPrimitive", (void(PrimitiveMan::*)(const Vector& start, const std::string& filePath, float rotAngle, float scale)) & PrimitiveMan::DrawBitmapPrimitive)
	    .addFunction("DrawBitmapPrimitive", (void(PrimitiveMan::*)(const Vector& start, const std::string& filePath, float rotAngle, bool hFlipped, bool vFlipped)) & PrimitiveMan::DrawBitmapPrimitive)
	    .addFunction("DrawBitmapPrimitive", (void(PrimitiveMan::*)(const Vector& start, const std::string& filePath, float rotAngle, float scale, bool hFlipped, bool vFlipped)) & PrimitiveMan::DrawBitmapPrimitive)
	    .addFunction("DrawBitmapPrimitive", (void(PrimitiveMan::*)(int player, const Vector& start, const std::string& filePath, float rotAngle)) & PrimitiveMan::DrawBitmapPrimitive)
	    .addFunction("DrawBitmapPrimitive", (void(PrimitiveMan::*)(int player, const Vector& start, const std::string& filePath, float rotAngle, float scale)) & PrimitiveMan::DrawBitmapPrimitive)
	    .addFunction("DrawBitmapPrimitive", (void(PrimitiveMan::*)(int player, const Vector& start, const std::string& filePath, float rotAngle, bool hFlipped, bool vFlipped)) & PrimitiveMan::DrawBitmapPrimitive)
	    .addFunction("DrawBitmapPrimitive", (void(PrimitiveMan::*)(int player, const Vector& start, const std::string& filePath, float rotAngle, float scale, bool hFlipped, bool vFlipped)) & PrimitiveMan::DrawBitmapPrimitive)
	    .addFunction("DrawIconPrimitive", (void(PrimitiveMan::*)(const Vector& start, Entity* entity)) & PrimitiveMan::DrawIconPrimitive)
	    .addFunction("DrawIconPrimitive", (void(PrimitiveMan::*)(int player, const Vector& start, Entity* entity)) & PrimitiveMan::DrawIconPrimitive)

	    .addFunction("DrawPolygonPrimitive", &LuaAdaptersPrimitiveMan::DrawPolygonPrimitive)
	    .addFunction("DrawPolygonPrimitive", &LuaAdaptersPrimitiveMan::DrawPolygonPrimitiveForPlayer)
	    .addFunction("DrawPolygonFillPrimitive", &LuaAdaptersPrimitiveMan::DrawPolygonFillPrimitive)
	    .addFunction("DrawPolygonFillPrimitive", &LuaAdaptersPrimitiveMan::DrawPolygonFillPrimitiveForPlayer)
	    .addFunction("DrawPrimitives", &LuaAdaptersPrimitiveMan::DrawPrimitivesWithTransparency)
	    .addFunction("DrawPrimitives", &LuaAdaptersPrimitiveMan::DrawPrimitivesWithBlending)
	    .addFunction("DrawPrimitives", &LuaAdaptersPrimitiveMan::DrawPrimitivesWithBlendingPerChannel);
}

LuaBindingRegisterFunctionDefinitionForType(ManagerLuaBindings, SceneMan) {
	registry.Class<SceneMan>("SceneManager")

	    .addProperty("Scene", &SceneMan::GetScene)
	    .addProperty("SceneDim", &SceneMan::GetSceneDim)
	    .addProperty("SceneWidth", &SceneMan::GetSceneWidth)
	    .addProperty("SceneHeight", &SceneMan::GetSceneHeight)
	    .addProperty("SceneWrapsX", &SceneMan::SceneWrapsX)
	    .addProperty("SceneWrapsY", &SceneMan::SceneWrapsY)
	    .addProperty("SceneOrbitDirection", &SceneMan::GetSceneOrbitDirection)
	    .addProperty("LayerDrawMode", &SceneMan::GetLayerDrawMode, &SceneMan::SetLayerDrawMode)
	    .addProperty("GlobalAcc", &SceneMan::GetGlobalAcc)
	    .addProperty("OzPerKg", &SceneMan::GetOzPerKg)
	    .addProperty("KgPerOz", &SceneMan::GetKgPerOz)
	    .addProperty("ScrapCompactingHeight", &SceneMan::GetScrapCompactingHeight, &SceneMan::SetScrapCompactingHeight)

	    .addFunction("LoadScene", (int(SceneMan::*)(const std::string&, bool, bool)) & SceneMan::LoadScene)
	    .addFunction("LoadScene", (int(SceneMan::*)(const std::string&, bool)) & SceneMan::LoadScene)

	    .addFunction("GetTerrain", &SceneMan::GetTerrain)
	    .addFunction("GetMaterial", &SceneMan::GetMaterial)
	    .addFunction("GetMaterialFromID", &SceneMan::GetMaterialFromID)
	    .addFunction("GetTerrMatter", &SceneMan::GetTerrMatter)
	    .addFunction("GetMOIDPixel", (MOID(SceneMan::*)(int, int)) & SceneMan::GetMOIDPixel)
	    .addFunction("GetMOIDPixel", (MOID(SceneMan::*)(int, int, int)) & SceneMan::GetMOIDPixel)
	    .addFunction("SetLayerDrawMode", &SceneMan::SetLayerDrawMode)
	    .addFunction("LoadUnseenLayer", &SceneMan::LoadUnseenLayer)
	    .addFunction("MakeAllUnseen", &SceneMan::MakeAllUnseen)
	    .addFunction("AnythingUnseen", &SceneMan::AnythingUnseen)
	    .addFunction("GetUnseenResolution", &SceneMan::GetUnseenResolution)
	    .addFunction("IsUnseen", &SceneMan::IsUnseen)
	    .addFunction("RevealUnseen", &SceneMan::RevealUnseen)
	    .addFunction("RevealUnseenBox", &SceneMan::RevealUnseenBox)
	    .addFunction("RestoreUnseen", &SceneMan::RestoreUnseen)
	    .addFunction("RestoreUnseenBox", &SceneMan::RestoreUnseenBox)
	    .addFunction("CastSeeRay", &SceneMan::CastSeeRay)
	    .addFunction("CastUnseeRay", &SceneMan::CastUnseeRay)
	    .addFunction("CastUnseenRay", &SceneMan::CastUnseenRay)
	    .addFunction("CastMaterialRay", (bool(SceneMan::*)(const Vector&, const Vector&, unsigned char, Vector&, int, bool)) & SceneMan::CastMaterialRay)
	    .addFunction("CastMaterialRay", (float(SceneMan::*)(const Vector&, const Vector&, unsigned char, int)) & SceneMan::CastMaterialRay)
	    .addFunction("CastNotMaterialRay", (bool(SceneMan::*)(const Vector&, const Vector&, unsigned char, Vector&, int, bool)) & SceneMan::CastNotMaterialRay)
	    .addFunction("CastNotMaterialRay", (float(SceneMan::*)(const Vector&, const Vector&, unsigned char, int, bool)) & SceneMan::CastNotMaterialRay)
	    .addFunction("CastStrengthSumRay", &SceneMan::CastStrengthSumRay)
	    .addFunction("CastMaxStrengthRay", (float(SceneMan::*)(const Vector&, const Vector&, int, unsigned char)) & SceneMan::CastMaxStrengthRay)
	    .addFunction("CastMaxStrengthRay", (float(SceneMan::*)(const Vector&, const Vector&, int)) & SceneMan::CastMaxStrengthRay)
	    .addFunction("CastStrengthRay", &SceneMan::CastStrengthRay)
	    .addFunction("CastWeaknessRay", &SceneMan::CastWeaknessRay)
	    .addFunction("CastMORay", &LuaAdaptersSceneMan::CastMORay1)
	    .addFunction("CastMORay", &LuaAdaptersSceneMan::CastMORay2)
	    .addFunction("CastAllMOsRay", &LuaAdaptersSceneMan::CastAllMOsRay, LuaBinding::AdoptResult, LuaBinding::ReturnStlIterator) // The result is allocated for Lua, so the iterator owns it rather than leaking it like luabind did.
	    .addFunction("CastFindMORay", (bool(SceneMan::*)(const Vector&, const Vector&, MOID, const Vector&, unsigned char, bool, int)) & SceneMan::CastFindMORay)
	    .addFunction("CastFindMORay", &SceneMan::CastFindMORay)
	    .addFunction("CastObstacleRay", &LuaAdaptersSceneMan::CastObstacleRay1)
	    .addFunction("CastObstacleRay", &LuaAdaptersSceneMan::CastObstacleRay2)
	    .addFunction("CastTerrainPenetrationRay", &SceneMan::CastTerrainPenetrationRay)
	    .addFunction("GetLastRayHitPos", &SceneMan::GetLastRayHitPos)
	    .addFunction("FindAltitude", (float(SceneMan::*)(const Vector&, int, int)) & SceneMan::FindAltitude)
	    .addFunction("FindAltitude", (float(SceneMan::*)(const Vector&, int, int, bool)) & SceneMan::FindAltitude)
	    .addFunction("MovePointToGround", (Vector(SceneMan::*)(const Vector&, int, int)) & SceneMan::MovePointToGround)
	    .addFunction("MovePointToGround", (Vector(SceneMan::*)(const Vector&, int, int, int)) & SceneMan::MovePointToGround)
	    .addFunction("IsWithinBounds", &SceneMan::IsWithinBounds)
	    .addFunction("ForceBounds", (bool(SceneMan::*)(Vector&) const) & SceneMan::ForceBounds) //, out_value(_2))
	    .addFunction("WrapPosition", (bool(SceneMan::*)(Vector&) const) & SceneMan::WrapPosition) //, out_value(_2))
	    .addFunction("SnapPosition", &SceneMan::SnapPosition)
	    .addFunction("ShortestDistance", &SceneMan::ShortestDistance)
	    .addFunction("WrapBox", &LuaAdaptersSceneMan::WrapBoxes, LuaBinding::AdoptResult, LuaBinding::ReturnStlIterator) // The result is allocated for Lua, so the iterator owns it rather than leaking it like luabind did.
	    .addFunction("ObscuredPoint", (bool(SceneMan::*)(Vector&, int)) & SceneMan::ObscuredPoint) //, out_value(_2))
	    .addFunction("ObscuredPoint", (bool(SceneMan::*)(int, int, int)) & SceneMan::ObscuredPoint)
	    .addFunction("AddSceneObject", &SceneMan::AddSceneObject, LuaBinding::AdoptArg<2>)
	    .addFunction("CheckAndRemoveOrphans", (int(SceneMan::*)(int, int, int, int, bool)) & SceneMan::RemoveOrphans)
	    .addFunction("DislodgePixel", &SceneMan::DislodgePixel)
	    .addFunction("DislodgePixel", &SceneMan::DislodgePixelBool)
	    .addFunction("DislodgePixelLine", (const std::vector<MOPixel*>* (SceneMan::*)(const Vector& start, const Vector& ray, int skip, bool deletePixels) const) & SceneMan::DislodgePixelLine, LuaBinding::AdoptResult, LuaBinding::ReturnStlIterator)
	    .addFunction("DislodgePixelLine", (const std::vector<MOPixel*>* (SceneMan::*)(const Vector& start, const Vector& ray, int skip) const) & SceneMan::DislodgePixelLineNoBool, LuaBinding::AdoptResult, LuaBinding::ReturnStlIterator)
	    .addFunction("DislodgePixelCircle", (const std::vector<MOPixel*>* (SceneMan::*)(const Vector& centre, float radius, bool deletePixels) const) & SceneMan::DislodgePixelCircle, LuaBinding::AdoptResult, LuaBinding::ReturnStlIterator)
	    .addFunction("DislodgePixelCircle", (const std::vector<MOPixel*>* (SceneMan::*)(const Vector& centre, float radius) const) & SceneMan::DislodgePixelCircleNoBool, LuaBinding::AdoptResult, LuaBinding::ReturnStlIterator)
	    .addFunction("DislodgePixelBox", (const std::vector<MOPixel*>* (SceneMan::*)(const Vector& upperLeftCorner, const Vector& lowerRightCorner, bool deletePixels) const) & SceneMan::DislodgePixelBox, LuaBinding::AdoptResult, LuaBinding::ReturnStlIterator)
	    .addFunction("DislodgePixelBox", (const std::vector<MOPixel*>* (SceneMan::*)(const Vector& upperLeftCorner, const Vector& lowerRightCorner) const) & SceneMan::DislodgePixelBoxNoBool, LuaBinding::AdoptResult, LuaBinding::ReturnStlIterator)
	    .addFunction("DislodgePixelRing", (const std::vector<MOPixel*>* (SceneMan::*)(const Vector& centre, float innerRadius, float outerRadius, bool deletePixels) const) & SceneMan::DislodgePixelRing, LuaBinding::AdoptResult, LuaBinding::ReturnStlIterator)
	    .addFunction("DislodgePixelRing", (const std::vector<MOPixel*>* (SceneMan::*)(const Vector& centre, float innerRadius, float outerRadius) const) & SceneMan::DislodgePixelRingNoBool, LuaBinding::AdoptResult, LuaBinding::ReturnStlIterator);
}

LuaBindingRegisterFunctionDefinitionForType(ManagerLuaBindings, CameraMan) {
	registry.Class<CameraMan>("CameraManager")

	    .addFunction("GetOffset", &CameraMan::GetOffset)
	    .addFunction("SetOffset", &CameraMan::SetOffset)
	    .addFunction("GetScreenOcclusion", &CameraMan::GetScreenOcclusion)
	    .addFunction("SetScreenOcclusion", &CameraMan::SetScreenOcclusion)
	    .addFunction("GetScrollTarget", &CameraMan::GetScrollTarget)
	    .addFunction("SetScrollTarget", &CameraMan::SetScrollTarget)
	    .addFunction("TargetDistanceScalar", &CameraMan::TargetDistanceScalar)
	    .addFunction("CheckOffset", &CameraMan::CheckOffset)
	    .addFunction("SetScroll", &CameraMan::SetScroll)
	    .addFunction("AddScreenShake", (void(CameraMan::*)(float, int)) & CameraMan::AddScreenShake)
	    .addFunction("AddScreenShake", (void(CameraMan::*)(float, const Vector&)) & CameraMan::AddScreenShake);
}

LuaBindingRegisterFunctionDefinitionForType(ManagerLuaBindings, SettingsMan) {
	registry.Class<SettingsMan>("SettingsManager")

	    .addProperty("PrintDebugInfo", &SettingsMan::PrintDebugInfo, &SettingsMan::SetPrintDebugInfo)
	    .addProperty("RecommendedMOIDCount", &SettingsMan::RecommendedMOIDCount)
	    .addProperty("AIUpdateInterval", &SettingsMan::GetAIUpdateInterval, &SettingsMan::SetAIUpdateInterval)
	    .addProperty("ShowEnemyHUD", &SettingsMan::ShowEnemyHUD)
	    .addProperty("AutomaticGoldDeposit", &SettingsMan::GetAutomaticGoldDeposit);
}

LuaBindingRegisterFunctionDefinitionForType(ManagerLuaBindings, TimerMan) {
	registry.Class<TimerMan>("TimerManager")

	    .addProperty("TimeScale", &TimerMan::GetTimeScale, &TimerMan::SetTimeScale)
	    .addProperty("RealToSimCap", &TimerMan::GetRealToSimCap)
	    .addProperty("DeltaTimeTicks", &LuaAdaptersTimerMan::GetDeltaTimeTicks, &TimerMan::SetDeltaTimeTicks)
	    .addProperty("DeltaTimeSecs", &TimerMan::GetDeltaTimeSecs, &TimerMan::SetDeltaTimeSecs)
	    .addProperty("DeltaTimeMS", &TimerMan::GetDeltaTimeMS)
	    .addProperty("AIDeltaTimeSecs", &TimerMan::GetAIDeltaTimeSecs)
	    .addProperty("AIDeltaTimeMS", &TimerMan::GetAIDeltaTimeMS)

	    .addProperty("TicksPerSecond", &LuaAdaptersTimerMan::GetTicksPerSecond)

	    .addFunction("TimeForSimUpdate", &TimerMan::TimeForSimUpdate)
	    .addFunction("DrawnSimUpdate", &TimerMan::DrawnSimUpdate);
}

LuaBindingRegisterFunctionDefinitionForType(ManagerLuaBindings, UInputMan) {
	registry.Class<UInputMan>("UInputManager")

	    .addProperty("FlagLAltState", &UInputMan::FlagLAltState)
	    .addProperty("FlagRAltState", &UInputMan::FlagRAltState)
	    .addProperty("FlagAltState", &UInputMan::FlagAltState)
	    .addProperty("FlagLCtrlState", &UInputMan::FlagLCtrlState)
	    .addProperty("FlagRCtrlState", &UInputMan::FlagRCtrlState)
	    .addProperty("FlagCtrlState", &UInputMan::FlagCtrlState)
	    .addProperty("FlagLShiftState", &UInputMan::FlagLShiftState)
	    .addProperty("FlagRShiftState", &UInputMan::FlagRShiftState)
	    .addProperty("FlagShiftState", &UInputMan::FlagShiftState)

	    .addFunction("GetInputDevice", &UInputMan::GetInputDevice)
	    .addFunction("ElementPressed", &UInputMan::ElementPressed)
	    .addFunction("ElementReleased", &UInputMan::ElementReleased)
	    .addFunction("ElementHeld", &UInputMan::ElementHeld)
	    .addFunction("KeyPressed", (bool(UInputMan::*)(SDL_Keycode, int) const) & UInputMan::KeyPressed)
	    .addFunction("KeyPressed", (bool(UInputMan::*)(SDL_Keycode) const) & UInputMan::KeyPressedKeycode)
	    .addFunction("KeyReleased", (bool(UInputMan::*)(SDL_Keycode, int) const) & UInputMan::KeyReleased)
	    .addFunction("KeyReleased", (bool(UInputMan::*)(SDL_Keycode) const) & UInputMan::KeyReleasedKeycode)
	    .addFunction("KeyHeld", (bool(UInputMan::*)(SDL_Keycode, int) const) & UInputMan::KeyHeld)
	    .addFunction("KeyHeld", (bool(UInputMan::*)(SDL_Keycode) const) & UInputMan::KeyHeldKeycode)
	    .addFunction("ScancodePressed", (bool(UInputMan::*)(SDL_Scancode, int) const) & UInputMan::KeyPressed)
	    .addFunction("ScancodePressed", (bool(UInputMan::*)(SDL_Scancode) const) & UInputMan::KeyPressedScancode)
	    .addFunction("ScancodeReleased", (bool(UInputMan::*)(SDL_Scancode, int) const) & UInputMan::KeyReleased)
	    .addFunction("ScancodeReleased", (bool(UInputMan::*)(SDL_Scancode) const) & UInputMan::KeyReleasedScancode)
	    .addFunction("ScancodeHeld", (bool(UInputMan::*)(SDL_Scancode, int) const) & UInputMan::KeyHeld)
	    .addFunction("ScancodeHeld", (bool(UInputMan::*)(SDL_Scancode) const) & UInputMan::KeyHeldScancode)
	    .addFunction("MouseButtonPressed", &UInputMan::MouseButtonPressed)
	    .addFunction("MouseButtonReleased", &UInputMan::MouseButtonReleased)
	    .addFunction("MouseButtonHeld", &UInputMan::MouseButtonHeld)
	    .addFunction("GetMousePos", &UInputMan::GetAbsoluteMousePosition)
	    .addFunction("MouseWheelMoved", &UInputMan::MouseWheelMoved)
	    .addFunction("JoyButtonPressed", &UInputMan::JoyButtonPressed)
	    .addFunction("JoyButtonReleased", &UInputMan::JoyButtonReleased)
	    .addFunction("JoyButtonHeld", &UInputMan::JoyButtonHeld)
	    .addFunction("WhichJoyButtonPressed", &UInputMan::WhichJoyButtonPressed)
	    .addFunction("JoyDirectionPressed", &UInputMan::JoyDirectionPressed)
	    .addFunction("JoyDirectionReleased", &UInputMan::JoyDirectionReleased)
	    .addFunction("JoyDirectionHeld", &UInputMan::JoyDirectionHeld)
	    .addFunction("AnalogMoveValues", &UInputMan::AnalogMoveValues)
	    .addFunction("AnalogAimValues", &UInputMan::AnalogAimValues)
	    .addFunction("SetMouseValueMagnitude", &UInputMan::SetMouseValueMagnitude)
	    .addFunction("AnalogAxisValue", &UInputMan::AnalogAxisValue)
	    .addFunction("MouseUsedByPlayer", &UInputMan::MouseUsedByPlayer)
	    .addFunction("AnyMouseButtonPress", &UInputMan::AnyMouseButtonPress)
	    .addFunction("GetMouseMovement", &UInputMan::GetMouseMovement)
	    .addFunction("DisableMouseMoving", &UInputMan::DisableMouseMoving)
	    .addFunction("SetMousePos", &UInputMan::SetMousePos)
	    .addFunction("ForceMouseWithinBox", &UInputMan::ForceMouseWithinBox)
	    .addFunction("AnyKeyPress", &UInputMan::AnyKeyPress)
	    .addFunction("AnyJoyInput", &UInputMan::AnyJoyInput)
	    .addFunction("AnyJoyPress", &UInputMan::AnyJoyPress)
	    .addFunction("AnyJoyButtonPress", &UInputMan::AnyJoyButtonPress)
	    .addFunction("AnyInput", &UInputMan::AnyKeyOrJoyInput)
	    .addFunction("AnyPress", &UInputMan::AnyPress)
	    .addFunction("AnyStartPress", &UInputMan::AnyStartPress)
	    .addFunction("HasTextInput", &UInputMan::HasTextInput)
	    .addFunction("GetTextInput", (const std::string& (UInputMan::*)() const) & UInputMan::GetTextInput)

	    .addFunction("MouseButtonPressed", &LuaAdaptersUInputMan::MouseButtonPressed)
	    .addFunction("MouseButtonReleased", &LuaAdaptersUInputMan::MouseButtonReleased)
	    .addFunction("MouseButtonHeld", &LuaAdaptersUInputMan::MouseButtonHeld);
}
