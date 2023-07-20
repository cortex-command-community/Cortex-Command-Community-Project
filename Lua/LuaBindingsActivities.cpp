// Make sure that binding definition files are always set to NOT use pre-compiled headers and conformance mode (/permissive) otherwise everything will be on fire!

#include "LuaBindingRegisterDefinitions.h"

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

		luaType.set(sol::meta_function::construct, sol::constructors<
			Activity()
		>());

		luaType["Description"] = sol::property(&Activity::GetDescription);
		luaType["InCampaignStage"] = sol::property(&Activity::GetInCampaignStage, &Activity::SetInCampaignStage);
		luaType["ActivityState"] = sol::property(&Activity::GetActivityState, &Activity::SetActivityState);
		luaType["SceneName"] = sol::property(&Activity::GetSceneName, &Activity::SetSceneName);
		luaType["PlayerCount"] = sol::property(&Activity::GetPlayerCount);
		luaType["HumanCount"] = sol::property(&Activity::GetHumanCount);
		luaType["TeamCount"] = sol::property(&Activity::GetTeamCount);
		luaType["Difficulty"] = sol::property(&Activity::GetDifficulty, &Activity::SetDifficulty);

		luaType["DeactivatePlayer"] = &Activity::DeactivatePlayer;
		luaType["PlayerActive"] = &Activity::PlayerActive;
		luaType["PlayerHuman"] = &Activity::PlayerHuman;
		luaType["TeamActive"] = &Activity::TeamActive;
		luaType["ForceSetTeamAsActive"] = &Activity::ForceSetTeamAsActive;
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

		luaType.new_enum<Players>("Players", {
			{ "PLAYER_NONE", Players::NoPlayer },
			{ "PLAYER_1", Players::PlayerOne },
			{ "PLAYER_2", Players::PlayerTwo },
			{ "PLAYER_3", Players::PlayerThree },
			{ "PLAYER_4", Players::PlayerFour },
			{ "MAXPLAYERCOUNT", Players::MaxPlayerCount }
		});
		luaType.new_enum<Activity::ActivityState>("ActivityState", {
			{ "NOACTIVITY", Activity::ActivityState::NoActivity },
			{ "NOTSTARTED", Activity::ActivityState::NotStarted },
			{ "STARTING", Activity::ActivityState::Starting },
			{ "EDITING", Activity::ActivityState::Editing },
			{ "PREGAME", Activity::ActivityState::PreGame },
			{ "RUNNING", Activity::ActivityState::Running },
			{ "INERROR", Activity::ActivityState::HasError },
			{ "OVER", Activity::ActivityState::Over }
		});
		luaType.new_enum<Activity::Teams>("Team", {
			{ "NOTEAM", Activity::Teams::NoTeam },
			{ "TEAM_1", Activity::Teams::TeamOne },
			{ "TEAM_2", Activity::Teams::TeamTwo },
			{ "TEAM_3", Activity::Teams::TeamThree },
			{ "TEAM_4", Activity::Teams::TeamFour },
			{ "MAXTEAMCOUNT", Activity::Teams::MaxTeamCount }
		});
		luaType.new_enum<Activity::ViewState>("ViewState", {
			{ "NORMAL", Activity::ViewState::Normal },
			{ "OBSERVE", Activity::ViewState::Observe },
			{ "DEATHWATCH", Activity::ViewState::DeathWatch },
			{ "ACTORSELECT", Activity::ViewState::ActorSelect },
			{ "AISENTRYPOINT", Activity::ViewState::AISentryPoint },
			{ "AIPATROLPOINTS", Activity::ViewState::AIPatrolPoints },
			{ "AIGOLDDIGPOINT", Activity::ViewState::AIGoldDigPoint },
			{ "AIGOTOPOINT", Activity::ViewState::AIGoToPoint },
			{ "LZSELECT", Activity::ViewState::LandingZoneSelect }
		});
		luaType.new_enum<Activity::DifficultySetting>("DifficultySetting", {
			{ "MINDIFFICULTY", Activity::DifficultySetting::MinDifficulty },
			{ "CAKEDIFFICULTY", Activity::DifficultySetting::CakeDifficulty },
			{ "EASYDIFFICULTY", Activity::DifficultySetting::EasyDifficulty },
			{ "MEDIUMDIFFICULTY", Activity::DifficultySetting::MediumDifficulty },
			{ "HARDDIFFICULTY", Activity::DifficultySetting::HardDifficulty },
			{ "NUTSDIFFICULTY", Activity::DifficultySetting::NutsDifficulty },
			{ "MAXDIFFICULTY", Activity::DifficultySetting::MaxDifficulty }
		});
		luaType.new_enum<Activity::AISkillSetting>("AISkillSetting", {
			{ "MINSKILL", Activity::AISkillSetting::MinSkill },
			{ "INFERIORSKILL", Activity::AISkillSetting::InferiorSkill },
			{ "DEFAULTSKILL", Activity::AISkillSetting::DefaultSkill },
			{ "AVERAGESKILL", Activity::AISkillSetting::AverageSkill },
			{ "GOODSKILL", Activity::AISkillSetting::GoodSkill },
			{ "UNFAIRSKILL", Activity::AISkillSetting::UnfairSkill }
		});
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(ActivityLuaBindings, GameActivity) {
		auto luaType = SimpleTypeLuaClassDefinition(GameActivity);

		luaType.set(sol::meta_function::construct, sol::constructors<
			GameActivity()
		>());

		luaType["WinnerTeam"] = sol::property(&GameActivity::GetWinnerTeam, &GameActivity::SetWinnerTeam);
		luaType["CPUTeam"] = sol::property(&GameActivity::GetCPUTeam, &GameActivity::SetCPUTeam);
		luaType["DeliveryDelay"] = sol::property(&GameActivity::GetDeliveryDelay, &GameActivity::SetDeliveryDelay);
		luaType["BuyMenuEnabled"] = sol::property(&GameActivity::GetBuyMenuEnabled, &GameActivity::SetBuyMenuEnabled);
		luaType["CraftsOrbitAtTheEdge"] = sol::property(&GameActivity::GetCraftOrbitAtTheEdge, &GameActivity::SetCraftOrbitAtTheEdge);

		//luaType["ActorCursor"] = &GameActivity::m_ActorCursor;
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
		luaType["SetOverridePurchaseList"] = (int (GameActivity::*)(const Loadout *, int))&GameActivity::SetOverridePurchaseList;
		luaType["SetOverridePurchaseList"] = (int (GameActivity::*)(std::string, int))&GameActivity::SetOverridePurchaseList;
		luaType["ClearOverridePurchase"] = &GameActivity::ClearOverridePurchase;
		luaType["CreateDelivery"] = (bool (GameActivity::*)(int))&GameActivity::CreateDelivery;
		luaType["CreateDelivery"] = (bool (GameActivity::*)(int, int))&GameActivity::CreateDelivery;
		luaType["CreateDelivery"] = (bool (GameActivity::*)(int, int, Vector&))&GameActivity::CreateDelivery;
		luaType["CreateDelivery"] = (bool (GameActivity::*)(int, int, Actor*))&GameActivity::CreateDelivery;
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

		luaType.new_enum<GameActivity::ObjectiveArrowDir>("ObjectiveArrowDir", {
			{ "ARROWDOWN", GameActivity::ObjectiveArrowDir::ARROWDOWN },
			{ "ARROWLEFT", GameActivity::ObjectiveArrowDir::ARROWLEFT },
			{ "ARROWRIGHT", GameActivity::ObjectiveArrowDir::ARROWRIGHT },
			{ "ARROWUP", GameActivity::ObjectiveArrowDir::ARROWUP }
		});
	}
}