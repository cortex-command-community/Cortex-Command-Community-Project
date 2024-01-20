// Make sure that binding definition files are always set to NOT use pre-compiled headers and conformance mode (/permissive) otherwise everything will be on fire!

#include "LuaBindingRegisterDefinitions.h"

#include "LuaAdapterDefinitions.h"

#include "GameActivity.h"
#include "GAScripted.h"
#include "ActorEditor.h"
#include "GUIBanner.h"
#include "BuyMenuGUI.h"
#include "SceneEditorGUI.h"

namespace RTE {

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(ActivityLuaBindings, Activity) {
		auto luaType = AbstractTypeLuaClassDefinition(Activity, Entity);

		luaType.set(sol::call_constructor, sol::constructors<
		                                       Activity()>());

		luaType["Description"] = sol::property(&Activity::GetDescription);
		luaType["InCampaignStage"] = sol::property(&Activity::GetInCampaignStage, &Activity::SetInCampaignStage);
		luaType["ActivityState"] = sol::property(&Activity::GetActivityState, &Activity::SetActivityState);
		luaType["AllowsUserSaving"] = sol::property(&Activity::GetAllowsUserSaving, &Activity::SetAllowsUserSaving);
		luaType["SceneName"] = sol::property(&Activity::GetSceneName, &Activity::SetSceneName);
		luaType["PlayerCount"] = sol::property(&Activity::GetPlayerCount);
		luaType["HumanCount"] = sol::property(&Activity::GetHumanCount);
		luaType["TeamCount"] = sol::property(&Activity::GetTeamCount);
		luaType["Difficulty"] = sol::property(&Activity::GetDifficulty, &Activity::SetDifficulty);

		luaType["DeactivatePlayer"] = &Activity::DeactivatePlayer;
		luaType["PlayerActive"] = &Activity::PlayerActive;
		luaType["PlayerHuman"] = &Activity::PlayerHuman;
		luaType["TeamActive"] = &Activity::TeamActive;
		luaType["GetTeamOfPlayer"] = &Activity::GetTeamOfPlayer;
		luaType["SetTeamOfPlayer"] = &Activity::SetTeamOfPlayer;
		luaType["PlayersInTeamCount"] = &Activity::PlayersInTeamCount;
		luaType["ScreenOfPlayer"] = &Activity::ScreenOfPlayer;
		luaType["GetViewState"] = &Activity::GetViewState;
		luaType["SetViewState"] = &Activity::SetViewState;
		luaType["GetPlayerBrain"] = &Activity::GetPlayerBrain;
		luaType["SetPlayerBrain"] = &Activity::SetPlayerBrain;
		luaType["PlayerHadBrain"] = &Activity::PlayerHadBrain;
		luaType["SetPlayerHadBrain"] = &Activity::SetPlayerHadBrain;
		luaType["SetBrainEvacuated"] = &Activity::SetBrainEvacuated;
		luaType["BrainWasEvacuated"] = &Activity::BrainWasEvacuated;
		luaType["IsAssignedBrain"] = &Activity::IsAssignedBrain;
		luaType["IsBrainOfWhichPlayer"] = &Activity::IsBrainOfWhichPlayer;
		luaType["IsOtherPlayerBrain"] = &Activity::IsOtherPlayerBrain;
		luaType["HumanBrainCount"] = &Activity::HumanBrainCount;
		luaType["AIBrainCount"] = &Activity::AIBrainCount;
		luaType["GetControlledActor"] = &Activity::GetControlledActor;
		luaType["GetPlayerController"] = &Activity::GetPlayerController;
		luaType["SetTeamFunds"] = &Activity::SetTeamFunds;
		luaType["GetTeamFunds"] = &Activity::GetTeamFunds;
		luaType["SetTeamAISkill"] = &Activity::SetTeamAISkill;
		luaType["GetTeamAISkill"] = &Activity::GetTeamAISkill;
		luaType["ChangeTeamFunds"] = &Activity::ChangeTeamFunds;
		luaType["TeamFundsChanged"] = &Activity::TeamFundsChanged;
		luaType["ReportDeath"] = &Activity::ReportDeath;
		luaType["GetTeamDeathCount"] = &Activity::GetTeamDeathCount;
		luaType["IsRunning"] = &Activity::IsRunning;
		luaType["IsPaused"] = &Activity::IsPaused;
		luaType["IsOver"] = &Activity::IsOver;
		luaType["SwitchToActor"] = &Activity::SwitchToActor;
		luaType["SwitchToNextActor"] = &Activity::SwitchToNextActor;
		luaType["SwitchToPrevActor"] = &Activity::SwitchToPrevActor;
		luaType["IsHumanTeam"] = &Activity::IsHumanTeam;
		luaType["ResetMessageTimer"] = &Activity::ResetMessageTimer;
		luaType["SaveString"] = &Activity::SaveString;
		luaType["LoadString"] = &Activity::LoadString;
		luaType["SaveNumber"] = &Activity::SaveNumber;
		luaType["LoadNumber"] = &Activity::LoadNumber;
		luaType["SendMessage"] = &LuaAdaptersActivity::SendMessage1;
		luaType["SendMessage"] = &LuaAdaptersActivity::SendMessage2;

		{
			sol::table enumTable = LegacyEnumTypeTable("Players");
			enumTable["PLAYER_NONE"] = Players::NoPlayer;
			enumTable["PLAYER_1"] = Players::PlayerOne;
			enumTable["PLAYER_2"] = Players::PlayerTwo;
			enumTable["PLAYER_3"] = Players::PlayerThree;
			enumTable["PLAYER_4"] = Players::PlayerFour;
			enumTable["MAXPLAYERCOUNT"] = Players::MaxPlayerCount;
		}
		{
			sol::table enumTable = LegacyEnumTypeTable("ActivityState");
			enumTable["NOACTIVITY"] = Activity::ActivityState::NoActivity;
			enumTable["NOTSTARTED"] = Activity::ActivityState::NotStarted;
			enumTable["STARTING"] = Activity::ActivityState::Starting;
			enumTable["EDITING"] = Activity::ActivityState::Editing;
			enumTable["PREGAME"] = Activity::ActivityState::PreGame;
			enumTable["RUNNING"] = Activity::ActivityState::Running;
			enumTable["INERROR"] = Activity::ActivityState::HasError;
			enumTable["OVER"] = Activity::ActivityState::Over;
		}
		{
			sol::table enumTable = LegacyEnumTypeTable("Team");
			enumTable["NOTEAM"] = Activity::Teams::NoTeam;
			enumTable["TEAM_1"] = Activity::Teams::TeamOne;
			enumTable["TEAM_2"] = Activity::Teams::TeamTwo;
			enumTable["TEAM_3"] = Activity::Teams::TeamThree;
			enumTable["TEAM_4"] = Activity::Teams::TeamFour;
			enumTable["MAXTEAMCOUNT"] = Activity::Teams::MaxTeamCount;
		}
		{
			sol::table enumTable = LegacyEnumTypeTable("ViewState");
			enumTable["NORMAL"] = Activity::ViewState::Normal;
			enumTable["OBSERVE"] = Activity::ViewState::Observe;
			enumTable["DEATHWATCH"] = Activity::ViewState::DeathWatch;
			enumTable["ACTORSELECT"] = Activity::ViewState::ActorSelect;
			enumTable["AISENTRYPOINT"] = Activity::ViewState::AISentryPoint;
			enumTable["AIPATROLPOINTS"] = Activity::ViewState::AIPatrolPoints;
			enumTable["AIGOLDDIGPOINT"] = Activity::ViewState::AIGoldDigPoint;
			enumTable["AIGOTOPOINT"] = Activity::ViewState::AIGoToPoint;
			enumTable["LZSELECT"] = Activity::ViewState::LandingZoneSelect;
		}
		{
			sol::table enumTable = LegacyEnumTypeTable("DifficultySetting");
			enumTable["MINDIFFICULTY"] = Activity::DifficultySetting::MinDifficulty;
			enumTable["CAKEDIFFICULTY"] = Activity::DifficultySetting::CakeDifficulty;
			enumTable["EASYDIFFICULTY"] = Activity::DifficultySetting::EasyDifficulty;
			enumTable["MEDIUMDIFFICULTY"] = Activity::DifficultySetting::MediumDifficulty;
			enumTable["HARDDIFFICULTY"] = Activity::DifficultySetting::HardDifficulty;
			enumTable["NUTSDIFFICULTY"] = Activity::DifficultySetting::NutsDifficulty;
			enumTable["MAXDIFFICULTY"] = Activity::DifficultySetting::MaxDifficulty;
		}
		{
			sol::table enumTable = LegacyEnumTypeTable("AISkillSetting");
			enumTable["MINSKILL"] = Activity::AISkillSetting::MinSkill;
			enumTable["INFERIORSKILL"] = Activity::AISkillSetting::InferiorSkill;
			enumTable["DEFAULTSKILL"] = Activity::AISkillSetting::DefaultSkill;
			enumTable["AVERAGESKILL"] = Activity::AISkillSetting::AverageSkill;
			enumTable["GOODSKILL"] = Activity::AISkillSetting::GoodSkill;
			enumTable["UNFAIRSKILL"] = Activity::AISkillSetting::UnfairSkill;
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(ActivityLuaBindings, GameActivity) {
		auto luaType = AbstractTypeLuaClassDefinition(GameActivity, Activity, Entity);
		MarkLuaDynamicObject(luaType);

		luaType.set(sol::call_constructor, sol::constructors<
		                                       GameActivity()>());

		luaType["WinnerTeam"] = sol::property(&GameActivity::GetWinnerTeam, &GameActivity::SetWinnerTeam);
		luaType["CPUTeam"] = sol::property(&GameActivity::GetCPUTeam, &GameActivity::SetCPUTeam);
		luaType["DeliveryDelay"] = sol::property(&GameActivity::GetDeliveryDelay, &GameActivity::SetDeliveryDelay);
		luaType["BuyMenuEnabled"] = sol::property(&GameActivity::GetBuyMenuEnabled, &GameActivity::SetBuyMenuEnabled);
		luaType["CraftsOrbitAtTheEdge"] = sol::property(&GameActivity::GetCraftOrbitAtTheEdge, &GameActivity::SetCraftOrbitAtTheEdge);

		// luaType["ActorCursor"] = &GameActivity::m_ActorCursor;
		luaType["CursorTimer"] = &GameActivity::m_CursorTimer;
		luaType["GameTimer"] = &GameActivity::m_GameTimer;
		luaType["GameOverTimer"] = &GameActivity::m_GameOverTimer;
		luaType["GameOverPeriod"] = &GameActivity::m_GameOverPeriod;

		luaType["SetObservationTarget"] = &GameActivity::SetObservationTarget;
		luaType["SetDeathViewTarget"] = &GameActivity::SetDeathViewTarget;
		luaType["SetLandingZone"] = &GameActivity::SetLandingZone;
		luaType["GetLandingZone"] = &GameActivity::GetLandingZone;
		luaType["SetActorSelectCursor"] = &GameActivity::SetActorSelectCursor;
		luaType["GetBuyGUI"] = &GameActivity::GetBuyGUI;
		luaType["GetEditorGUI"] = &GameActivity::GetEditorGUI;
		luaType["LockControlledActor"] = &GameActivity::LockControlledActor;
		luaType["OtherTeam"] = &GameActivity::OtherTeam;
		luaType["OneOrNoneTeamsLeft"] = &GameActivity::OneOrNoneTeamsLeft;
		luaType["WhichTeamLeft"] = &GameActivity::WhichTeamLeft;
		luaType["NoTeamLeft"] = &GameActivity::NoTeamLeft;
		luaType["OnlyOneTeamLeft"] = &GameActivity::OneOrNoneTeamsLeft; // Backwards compat
		luaType["GetBanner"] = &GameActivity::GetBanner;
		luaType["SetLZArea"] = &GameActivity::SetLZArea;
		luaType["GetLZArea"] = &GameActivity::GetLZArea;
		luaType["SetBrainLZWidth"] = &GameActivity::SetBrainLZWidth;
		luaType["GetBrainLZWidth"] = &GameActivity::GetBrainLZWidth;
		luaType["GetActiveCPUTeamCount"] = &GameActivity::GetActiveCPUTeamCount;
		luaType["GetActiveHumanTeamCount"] = &GameActivity::GetActiveHumanTeamCount;
		luaType["AddObjectivePoint"] = &GameActivity::AddObjectivePoint;
		luaType["YSortObjectivePoints"] = &GameActivity::YSortObjectivePoints;
		luaType["ClearObjectivePoints"] = &GameActivity::ClearObjectivePoints;
		luaType["AddOverridePurchase"] = &GameActivity::AddOverridePurchase;
		luaType["SetOverridePurchaseList"] = (int(GameActivity::*)(const Loadout*, int)) & GameActivity::SetOverridePurchaseList;
		luaType["SetOverridePurchaseList"] = (int(GameActivity::*)(std::string, int)) & GameActivity::SetOverridePurchaseList;
		luaType["ClearOverridePurchase"] = &GameActivity::ClearOverridePurchase;
		luaType["CreateDelivery"] = (bool(GameActivity::*)(int)) & GameActivity::CreateDelivery;
		luaType["CreateDelivery"] = (bool(GameActivity::*)(int, int)) & GameActivity::CreateDelivery;
		luaType["CreateDelivery"] = (bool(GameActivity::*)(int, int, Vector&)) & GameActivity::CreateDelivery;
		luaType["CreateDelivery"] = (bool(GameActivity::*)(int, int, Actor*)) & GameActivity::CreateDelivery;
		luaType["GetDeliveryCount"] = &GameActivity::GetDeliveryCount;
		luaType["GetTeamTech"] = &GameActivity::GetTeamTech;
		luaType["SetTeamTech"] = &GameActivity::SetTeamTech;
		luaType["GetCrabToHumanSpawnRatio"] = &GameActivity::GetCrabToHumanSpawnRatio;
		luaType["TeamIsCPU"] = &GameActivity::TeamIsCPU;
		luaType["GetStartingGold"] = &GameActivity::GetStartingGold;
		luaType["GetFogOfWarEnabled"] = &GameActivity::GetFogOfWarEnabled;
		luaType["UpdateEditing"] = &GameActivity::UpdateEditing;
		luaType["DisableAIs"] = &GameActivity::DisableAIs;
		luaType["InitAIs"] = &GameActivity::InitAIs;

		{
			sol::table enumTable = LegacyEnumTypeTable("ObjectiveArrowDir");
			enumTable["ARROWDOWN"] = GameActivity::ObjectiveArrowDir::ARROWDOWN;
			enumTable["ARROWLEFT"] = GameActivity::ObjectiveArrowDir::ARROWLEFT;
			enumTable["ARROWRIGHT"] = GameActivity::ObjectiveArrowDir::ARROWRIGHT;
			enumTable["ARROWUP"] = GameActivity::ObjectiveArrowDir::ARROWUP;
		}

		// Lots of Lua scripts use this in GameActivity instead of Activity, so redefine here (enums aren't inherited)
		{
			sol::table enumTable = LegacyEnumTypeTable("DifficultySetting");
			enumTable["MINDIFFICULTY"] = Activity::DifficultySetting::MinDifficulty;
			enumTable["CAKEDIFFICULTY"] = Activity::DifficultySetting::CakeDifficulty;
			enumTable["EASYDIFFICULTY"] = Activity::DifficultySetting::EasyDifficulty;
			enumTable["MEDIUMDIFFICULTY"] = Activity::DifficultySetting::MediumDifficulty;
			enumTable["HARDDIFFICULTY"] = Activity::DifficultySetting::HardDifficulty;
			enumTable["NUTSDIFFICULTY"] = Activity::DifficultySetting::NutsDifficulty;
			enumTable["MAXDIFFICULTY"] = Activity::DifficultySetting::MaxDifficulty;
		}
	}
} // namespace RTE
