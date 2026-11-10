// Make sure that binding definition files are always set to NOT use pre-compiled headers and conformance mode (/permissive) otherwise everything will be on fire!

#include "LuaBindingRegisterDefinitions.h"

using namespace RTE;

LuaBindingRegisterFunctionDefinitionForType(ActivityLuaBindings, Activity) {
	AbstractTypeLuaClassDefinition(Activity, Entity)

	    .addConstructor<>()

	    .addProperty("Description", &Activity::GetDescription)
	    .addProperty("InCampaignStage", &Activity::GetInCampaignStage, &Activity::SetInCampaignStage)
	    .addProperty("ActivityState", &Activity::GetActivityState, &Activity::SetActivityState)
	    .addProperty("AllowsUserSaving", &Activity::GetAllowsUserSaving, &Activity::SetAllowsUserSaving)
	    .addProperty("SceneName", &Activity::GetSceneName, &Activity::SetSceneName)
	    .addProperty("PlayerCount", &Activity::GetPlayerCount)
	    .addProperty("HumanCount", &Activity::GetHumanCount)
	    .addProperty("TeamCount", &Activity::GetTeamCount)
	    .addProperty("Difficulty", &Activity::GetDifficulty, &Activity::SetDifficulty)

	    .addFunction("DeactivatePlayer", &Activity::DeactivatePlayer)
	    .addFunction("PlayerActive", &Activity::PlayerActive)
	    .addFunction("PlayerHuman", &Activity::PlayerHuman)
	    .addFunction("TeamActive", &Activity::TeamActive)
	    .addFunction("GetTeamOfPlayer", &Activity::GetTeamOfPlayer)
	    .addFunction("SetTeamOfPlayer", &Activity::SetTeamOfPlayer)
	    .addFunction("PlayersInTeamCount", &Activity::PlayersInTeamCount)
	    .addFunction("ScreenOfPlayer", &Activity::ScreenOfPlayer)
	    .addFunction("GetViewState", &Activity::GetViewState)
	    .addFunction("SetViewState", &Activity::SetViewState)
	    .addFunction("GetPlayerBrain", &Activity::GetPlayerBrain)
	    .addFunction("SetPlayerBrain", &Activity::SetPlayerBrain)
	    .addFunction("PlayerHadBrain", &Activity::PlayerHadBrain)
	    .addFunction("SetPlayerHadBrain", &Activity::SetPlayerHadBrain)
	    .addFunction("SetBrainEvacuated", &Activity::SetBrainEvacuated)
	    .addFunction("BrainWasEvacuated", &Activity::BrainWasEvacuated)
	    .addFunction("IsAssignedBrain", &Activity::IsAssignedBrain)
	    .addFunction("IsBrainOfWhichPlayer", &Activity::IsBrainOfWhichPlayer)
	    .addFunction("IsOtherPlayerBrain", &Activity::IsOtherPlayerBrain)
	    .addFunction("HumanBrainCount", &Activity::HumanBrainCount)
	    .addFunction("AIBrainCount", &Activity::AIBrainCount)
	    .addFunction("GetControlledActor", &Activity::GetControlledActor)
	    .addFunction("GetPlayerController", &Activity::GetPlayerController)
	    .addFunction("SetTeamFunds", &Activity::SetTeamFunds)
	    .addFunction("GetTeamFunds", &Activity::GetTeamFunds)
	    .addFunction("SetTeamAISkill", &Activity::SetTeamAISkill)
	    .addFunction("GetTeamAISkill", &Activity::GetTeamAISkill)
	    .addFunction("ChangeTeamFunds", &Activity::ChangeTeamFunds)
	    .addFunction("TeamFundsChanged", &Activity::TeamFundsChanged)
	    .addFunction("ReportDeath", &Activity::ReportDeath)
	    .addFunction("GetTeamDeathCount", &Activity::GetTeamDeathCount)
	    .addFunction("IsRunning", &Activity::IsRunning)
	    .addFunction("IsPaused", &Activity::IsPaused)
	    .addFunction("IsOver", &Activity::IsOver)
	    .addFunction("SwitchToActor", &Activity::SwitchToActor)
	    .addFunction("SwitchToNextActor", &Activity::SwitchToNextActor)
	    .addFunction("SwitchToPrevActor", &Activity::SwitchToPrevActor)
	    .addFunction("IsHumanTeam", &Activity::IsHumanTeam)
	    .addFunction("ResetMessageTimer", &Activity::ResetMessageTimer)
	    .addFunction("SaveString", &Activity::SaveString)
	    .addFunction("LoadString", &Activity::LoadString)
	    .addFunction("SaveNumber", &Activity::SaveNumber)
	    .addFunction("LoadNumber", &Activity::LoadNumber)
	    .addFunction("SendMessage", &LuaAdaptersActivity::SendMessage1)
	    .addFunction("SendMessage", &LuaAdaptersActivity::SendMessage2)

	    .addEnum({{"PLAYER_NONE", Players::NoPlayer},
	              {"PLAYER_1", Players::PlayerOne},
	              {"PLAYER_2", Players::PlayerTwo},
	              {"PLAYER_3", Players::PlayerThree},
	              {"PLAYER_4", Players::PlayerFour},
	              {"MAXPLAYERCOUNT", Players::MaxPlayerCount}})
	    .addEnum({{"NOACTIVITY", Activity::ActivityState::NoActivity},
	              {"NOTSTARTED", Activity::ActivityState::NotStarted},
	              {"STARTING", Activity::ActivityState::Starting},
	              {"EDITING", Activity::ActivityState::Editing},
	              {"PREGAME", Activity::ActivityState::PreGame},
	              {"RUNNING", Activity::ActivityState::Running},
	              {"INERROR", Activity::ActivityState::HasError},
	              {"OVER", Activity::ActivityState::Over}})
	    .addEnum({{"NOTEAM", Activity::Teams::NoTeam},
	              {"TEAM_1", Activity::Teams::TeamOne},
	              {"TEAM_2", Activity::Teams::TeamTwo},
	              {"TEAM_3", Activity::Teams::TeamThree},
	              {"TEAM_4", Activity::Teams::TeamFour},
	              {"MAXTEAMCOUNT", Activity::Teams::MaxTeamCount}})
	    .addEnum({{"NORMAL", Activity::ViewState::Normal},
	              {"OBSERVE", Activity::ViewState::Observe},
	              {"DEATHWATCH", Activity::ViewState::DeathWatch},
	              {"ACTORSELECT", Activity::ViewState::ActorSelect},
	              {"AISENTRYPOINT", Activity::ViewState::AISentryPoint},
	              {"AIPATROLPOINTS", Activity::ViewState::AIPatrolPoints},
	              {"AIGOLDDIGPOINT", Activity::ViewState::AIGoldDigPoint},
	              {"AIGOTOPOINT", Activity::ViewState::AIGoToPoint},
	              {"LZSELECT", Activity::ViewState::LandingZoneSelect}})
	    .addEnum({{"MINDIFFICULTY", Activity::DifficultySetting::MinDifficulty},
	              {"CAKEDIFFICULTY", Activity::DifficultySetting::CakeDifficulty},
	              {"EASYDIFFICULTY", Activity::DifficultySetting::EasyDifficulty},
	              {"MEDIUMDIFFICULTY", Activity::DifficultySetting::MediumDifficulty},
	              {"HARDDIFFICULTY", Activity::DifficultySetting::HardDifficulty},
	              {"NUTSDIFFICULTY", Activity::DifficultySetting::NutsDifficulty},
	              {"MAXDIFFICULTY", Activity::DifficultySetting::MaxDifficulty}})
	    .addEnum({{"MINSKILL", Activity::AISkillSetting::MinSkill},
	              {"INFERIORSKILL", Activity::AISkillSetting::InferiorSkill},
	              {"DEFAULTSKILL", Activity::AISkillSetting::DefaultSkill},
	              {"AVERAGESKILL", Activity::AISkillSetting::AverageSkill},
	              {"GOODSKILL", Activity::AISkillSetting::GoodSkill},
	              {"UNFAIRSKILL", Activity::AISkillSetting::UnfairSkill}});
}

LuaBindingRegisterFunctionDefinitionForType(ActivityLuaBindings, GameActivity) {
	registry.Class<GameActivity, Activity>("GameActivity")

	    .addConstructor<>()

	    .addProperty("WinnerTeam", &GameActivity::GetWinnerTeam, &GameActivity::SetWinnerTeam)
	    .addProperty("CPUTeam", &GameActivity::GetCPUTeam, &GameActivity::SetCPUTeam)
	    .addProperty("DeliveryDelay", &GameActivity::GetDeliveryDelay, &GameActivity::SetDeliveryDelay)
	    .addProperty("BuyMenuEnabled", &GameActivity::GetBuyMenuEnabled, &GameActivity::SetBuyMenuEnabled)
	    .addProperty("CraftsOrbitAtTheEdge", &GameActivity::GetCraftOrbitAtTheEdge, &GameActivity::SetCraftOrbitAtTheEdge)

	    //.addReadWriteMember("ActorCursor", &GameActivity::m_ActorCursor)
	    .addReadWriteMember("CursorTimer", &GameActivity::m_CursorTimer)
	    .addReadWriteMember("GameTimer", &GameActivity::m_GameTimer)
	    .addReadWriteMember("GameOverTimer", &GameActivity::m_GameOverTimer)
	    .addReadWriteMember("GameOverPeriod", &GameActivity::m_GameOverPeriod)

	    .addFunction("SetObservationTarget", &GameActivity::SetObservationTarget)
	    .addFunction("SetDeathViewTarget", &GameActivity::SetDeathViewTarget)
	    .addFunction("SetLandingZone", &GameActivity::SetLandingZone)
	    .addFunction("GetLandingZone", &GameActivity::GetLandingZone)
	    .addFunction("SetActorSelectCursor", &GameActivity::SetActorSelectCursor)
	    .addFunction("GetBuyGUI", &GameActivity::GetBuyGUI)
	    .addFunction("IsBuyGUIVisible", &GameActivity::IsBuyGUIVisible)
	    .addFunction("GetEditorGUI", &GameActivity::GetEditorGUI)
	    .addFunction("LockControlledActor", &GameActivity::LockControlledActor)
	    .addFunction("OtherTeam", &GameActivity::OtherTeam)
	    .addFunction("OneOrNoneTeamsLeft", &GameActivity::OneOrNoneTeamsLeft)
	    .addFunction("WhichTeamLeft", &GameActivity::WhichTeamLeft)
	    .addFunction("NoTeamLeft", &GameActivity::NoTeamLeft)
	    .addFunction("OnlyOneTeamLeft", &GameActivity::OneOrNoneTeamsLeft) // Backwards compat
	    .addFunction("GetBanner", &GameActivity::GetBanner)
	    .addFunction("SetLZArea", &GameActivity::SetLZArea)
	    .addFunction("GetLZArea", &GameActivity::GetLZArea)
	    .addFunction("SetBrainLZWidth", &GameActivity::SetBrainLZWidth)
	    .addFunction("GetBrainLZWidth", &GameActivity::GetBrainLZWidth)
	    .addFunction("GetActiveCPUTeamCount", &GameActivity::GetActiveCPUTeamCount)
	    .addFunction("GetActiveHumanTeamCount", &GameActivity::GetActiveHumanTeamCount)
	    .addFunction("AddObjectivePoint", &GameActivity::AddObjectivePoint)
	    .addFunction("YSortObjectivePoints", &GameActivity::YSortObjectivePoints)
	    .addFunction("ClearObjectivePoints", &GameActivity::ClearObjectivePoints)
	    .addFunction("AddOverridePurchase", &GameActivity::AddOverridePurchase)
	    .addFunction("SetOverridePurchaseList", (int(GameActivity::*)(const Loadout*, int)) & GameActivity::SetOverridePurchaseList)
	    .addFunction("SetOverridePurchaseList", (int(GameActivity::*)(const std::string&, int)) & GameActivity::SetOverridePurchaseList)
	    .addFunction("ClearOverridePurchase", &GameActivity::ClearOverridePurchase)
	    .addFunction("CreateDelivery", (bool(GameActivity::*)(int)) & GameActivity::CreateDelivery)
	    .addFunction("CreateDelivery", (bool(GameActivity::*)(int, int)) & GameActivity::CreateDelivery)
	    .addFunction("CreateDelivery", (bool(GameActivity::*)(int, int, Vector&)) & GameActivity::CreateDelivery)
	    .addFunction("CreateDelivery", (bool(GameActivity::*)(int, int, Actor*)) & GameActivity::CreateDelivery)
	    .addFunction("GetDeliveryCount", &GameActivity::GetDeliveryCount)
	    .addFunction("GetTeamTech", &GameActivity::GetTeamTech)
	    .addFunction("SetTeamTech", &GameActivity::SetTeamTech)
	    .addFunction("GetCrabToHumanSpawnRatio", &GameActivity::GetCrabToHumanSpawnRatio)
	    .addFunction("TeamIsCPU", &GameActivity::TeamIsCPU)
	    .addFunction("GetStartingGold", &GameActivity::GetStartingGold)
	    .addFunction("GetFogOfWarEnabled", &GameActivity::GetFogOfWarEnabled)
	    .addFunction("UpdateEditing", &GameActivity::UpdateEditing)
	    .addFunction("DisableAIs", &GameActivity::DisableAIs)
	    .addFunction("InitAIs", &GameActivity::InitAIs)

	    .addEnum({{"ARROWDOWN", GameActivity::ObjectiveArrowDir::ARROWDOWN},
	              {"ARROWLEFT", GameActivity::ObjectiveArrowDir::ARROWLEFT},
	              {"ARROWRIGHT", GameActivity::ObjectiveArrowDir::ARROWRIGHT},
	              {"ARROWUP", GameActivity::ObjectiveArrowDir::ARROWUP}});
}
