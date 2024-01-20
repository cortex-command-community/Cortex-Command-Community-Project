// Make sure that binding definition files are always set to NOT use pre-compiled headers and conformance mode (/permissive) otherwise everything will be on fire!

#include "LuaBindingRegisterDefinitions.h"

#include "LuaAdapterDefinitions.h"

#include "ACDropShip.h"
#include "ACrab.h"
#include "ACraft.h"
#include "ACRocket.h"
#include "Actor.h"
#include "ADoor.h"
#include "AHuman.h"
#include "Arm.h"
#include "Leg.h"
#include "LimbPath.h"
#include "HeldDevice.h"
#include "AEmitter.h"
#include "AEJetpack.h"
#include "Turret.h"

namespace RTE {

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, ACDropShip) {
		auto luaType = ConcreteTypeLuaClassDefinition(ACDropShip, ACraft, Actor, MOSRotating, MOSprite, MovableObject, SceneObject, Entity);

		luaType["RightEngine"] = sol::property(&ACDropShip::GetRightThruster, &LuaAdaptersPropertyOwnershipSafetyFaker::ACDropShipSetRightThruster);
		luaType["LeftEngine"] = sol::property(&ACDropShip::GetLeftThruster, &LuaAdaptersPropertyOwnershipSafetyFaker::ACDropShipSetLeftThruster);
		luaType["RightThruster"] = sol::property(&ACDropShip::GetURightThruster, &LuaAdaptersPropertyOwnershipSafetyFaker::ACDropShipSetURightThruster);
		luaType["LeftThruster"] = sol::property(&ACDropShip::GetULeftThruster, &LuaAdaptersPropertyOwnershipSafetyFaker::ACDropShipSetULeftThruster);
		luaType["RightHatch"] = sol::property(&ACDropShip::GetRightHatch, &LuaAdaptersPropertyOwnershipSafetyFaker::ACDropShipSetRightHatch);
		luaType["LeftHatch"] = sol::property(&ACDropShip::GetLeftHatch, &LuaAdaptersPropertyOwnershipSafetyFaker::ACDropShipSetLeftHatch);
		luaType["MaxEngineAngle"] = sol::property(&ACDropShip::GetMaxEngineAngle, &ACDropShip::SetMaxEngineAngle);
		luaType["LateralControlSpeed"] = sol::property(&ACDropShip::GetLateralControlSpeed, &ACDropShip::SetLateralControlSpeed);
		luaType["LateralControl"] = sol::property(&ACDropShip::GetLateralControl);
		luaType["HoverHeightModifier"] = sol::property(&ACDropShip::GetHoverHeightModifier, &ACDropShip::SetHoverHeightModifier);

		luaType["DetectObstacle"] = &ACDropShip::DetectObstacle;
		luaType["GetAltitude"] = &ACDropShip::GetAltitude;
	}

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, ACrab) {
		auto luaType = ConcreteTypeLuaClassDefinition(ACrab, Actor, MOSRotating, MOSprite, MovableObject, SceneObject);

		luaType.set(sol::call_constructor, sol::constructors<
		                                       ACrab()>());

		luaType["Turret"] = sol::property(&ACrab::GetTurret, &LuaAdaptersPropertyOwnershipSafetyFaker::ACrabSetTurret);
		luaType["Jetpack"] = sol::property(&ACrab::GetJetpack, &LuaAdaptersPropertyOwnershipSafetyFaker::ACrabSetJetpack);
		luaType["LeftFGLeg"] = sol::property(&ACrab::GetLeftFGLeg, &LuaAdaptersPropertyOwnershipSafetyFaker::ACrabSetLeftFGLeg);
		luaType["LeftBGLeg"] = sol::property(&ACrab::GetLeftBGLeg, &LuaAdaptersPropertyOwnershipSafetyFaker::ACrabSetLeftBGLeg);
		luaType["RightFGLeg"] = sol::property(&ACrab::GetRightFGLeg, &LuaAdaptersPropertyOwnershipSafetyFaker::ACrabSetRightFGLeg);
		luaType["RightBGLeg"] = sol::property(&ACrab::GetRightBGLeg, &LuaAdaptersPropertyOwnershipSafetyFaker::ACrabSetRightBGLeg);
		luaType["StrideSound"] = sol::property(&ACrab::GetStrideSound, &LuaAdaptersPropertyOwnershipSafetyFaker::ACrabSetStrideSound);
		luaType["StrideFrame"] = sol::property(&ACrab::StrideFrame);
		luaType["EquippedItem"] = sol::property(&ACrab::GetEquippedItem);
		luaType["FirearmIsReady"] = sol::property(&ACrab::FirearmIsReady);
		luaType["FirearmIsEmpty"] = sol::property(&ACrab::FirearmIsEmpty);
		luaType["FirearmNeedsReload"] = sol::property(&ACrab::FirearmNeedsReload);
		luaType["FirearmIsSemiAuto"] = sol::property(&ACrab::FirearmIsSemiAuto);
		luaType["FirearmActivationDelay"] = sol::property(&ACrab::FirearmActivationDelay);
		luaType["LimbPathPushForce"] = sol::property(&ACrab::GetLimbPathPushForce, &ACrab::SetLimbPathPushForce);
		luaType["AimRangeUpperLimit"] = sol::property(&ACrab::GetAimRangeUpperLimit, &ACrab::SetAimRangeUpperLimit);
		luaType["AimRangeLowerLimit"] = sol::property(&ACrab::GetAimRangeLowerLimit, &ACrab::SetAimRangeLowerLimit);

		luaType["ReloadFirearms"] = &ACrab::ReloadFirearms;
		luaType["IsWithinRange"] = &ACrab::IsWithinRange;
		luaType["Look"] = &ACrab::Look;
		luaType["LookForMOs"] = &ACrab::LookForMOs;
		luaType["GetLimbPath"] = &ACrab::GetLimbPath;
		luaType["GetLimbPathSpeed"] = &ACrab::GetLimbPathSpeed;
		luaType["SetLimbPathSpeed"] = &ACrab::SetLimbPathSpeed;

		{
			sol::table enumTable = LegacyEnumTypeTable("MovementState");
			enumTable["STAND"] = ACrab::MovementState::STAND;
			enumTable["WALK"] = ACrab::MovementState::WALK;
			enumTable["JUMP"] = ACrab::MovementState::JUMP;
			enumTable["DISLODGE"] = ACrab::MovementState::DISLODGE;
			enumTable["MOVEMENTSTATECOUNT"] = ACrab::MovementState::MOVEMENTSTATECOUNT;
		}
		{
			sol::table enumTable = LegacyEnumTypeTable("Side");
			enumTable["LEFTSIDE"] = ACrab::Side::LEFTSIDE;
			enumTable["RIGHTSIDE"] = ACrab::Side::RIGHTSIDE;
			enumTable["SIDECOUNT"] = ACrab::Side::SIDECOUNT;
		}
		{
			sol::table enumTable = LegacyEnumTypeTable("Layer");
			enumTable["FGROUND"] = ACrab::Layer::FGROUND;
			enumTable["BGROUND"] = ACrab::Layer::BGROUND;
		}
		{
			sol::table enumTable = LegacyEnumTypeTable("DeviceHandlingState");
			enumTable["STILL"] = ACrab::DeviceHandlingState::STILL;
			enumTable["POINTING"] = ACrab::DeviceHandlingState::POINTING;
			enumTable["SCANNING"] = ACrab::DeviceHandlingState::SCANNING;
			enumTable["AIMING"] = ACrab::DeviceHandlingState::AIMING;
			enumTable["FIRING"] = ACrab::DeviceHandlingState::FIRING;
			enumTable["THROWING"] = ACrab::DeviceHandlingState::THROWING;
			enumTable["DIGGING"] = ACrab::DeviceHandlingState::DIGGING;
		}
		{
			sol::table enumTable = LegacyEnumTypeTable("SweepState");
			enumTable["NOSWEEP"] = ACrab::SweepState::NOSWEEP;
			enumTable["SWEEPINGUP"] = ACrab::SweepState::SWEEPINGUP;
			enumTable["SWEEPUPPAUSE"] = ACrab::SweepState::SWEEPUPPAUSE;
			enumTable["SWEEPINGDOWN"] = ACrab::SweepState::SWEEPINGDOWN;
			enumTable["SWEEPDOWNPAUSE"] = ACrab::SweepState::SWEEPDOWNPAUSE;
		}
		{
			sol::table enumTable = LegacyEnumTypeTable("DigState");
			enumTable["NOTDIGGING"] = ACrab::DigState::NOTDIGGING;
			enumTable["PREDIG"] = ACrab::DigState::PREDIG;
			enumTable["STARTDIG"] = ACrab::DigState::STARTDIG;
			enumTable["TUNNELING"] = ACrab::DigState::TUNNELING;
			enumTable["FINISHINGDIG"] = ACrab::DigState::FINISHINGDIG;
			enumTable["PAUSEDIGGER"] = ACrab::DigState::PAUSEDIGGER;
		}
		{
			sol::table enumTable = LegacyEnumTypeTable("JumpState");
			enumTable["NOTJUMPING"] = ACrab::JumpState::NOTJUMPING;
			enumTable["FORWARDJUMP"] = ACrab::JumpState::FORWARDJUMP;
			enumTable["PREJUMP"] = ACrab::JumpState::PREUPJUMP;
			enumTable["UPJUMP"] = ACrab::JumpState::UPJUMP;
			enumTable["APEXJUMP"] = ACrab::JumpState::APEXJUMP;
			enumTable["LANDJUMP"] = ACrab::JumpState::LANDJUMP;
		}
	}

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, ACraft) {
		auto luaType = AbstractTypeLuaClassDefinition(ACraft, Actor, MOSRotating, MOSprite, MovableObject, SceneObject, Entity);

		luaType["HatchState"] = sol::property(&ACraft::GetHatchState);
		luaType["HatchOpenSound"] = sol::property(&ACraft::GetHatchOpenSound, &LuaAdaptersPropertyOwnershipSafetyFaker::ACraftSetHatchOpenSound);
		luaType["HatchCloseSound"] = sol::property(&ACraft::GetHatchCloseSound, &LuaAdaptersPropertyOwnershipSafetyFaker::ACraftSetHatchCloseSound);
		luaType["CrashSound"] = sol::property(&ACraft::GetCrashSound, &LuaAdaptersPropertyOwnershipSafetyFaker::ACraftSetCrashSound);
		luaType["MaxPassengers"] = sol::property(&ACraft::GetMaxPassengers);
		luaType["DeliveryDelayMultiplier"] = sol::property(&ACraft::GetDeliveryDelayMultiplier);
		luaType["ScuttleOnDeath"] = sol::property(&ACraft::GetScuttleOnDeath, &ACraft::SetScuttleOnDeath);
		luaType["HatchDelay"] = sol::property(&ACraft::GetHatchDelay, &ACraft::SetHatchDelay);

		luaType["OpenHatch"] = &ACraft::OpenHatch;
		luaType["CloseHatch"] = &ACraft::CloseHatch;

		{
			sol::table enumTable = LegacyEnumTypeTable("HatchState");
			enumTable["CLOSED"] = ACraft::HatchState::CLOSED;
			enumTable["OPENING"] = ACraft::HatchState::OPENING;
			enumTable["OPEN"] = ACraft::HatchState::OPEN;
			enumTable["CLOSING"] = ACraft::HatchState::CLOSING;
			enumTable["HatchStateCount"] = ACraft::HatchState::HatchStateCount;
		}
		{
			sol::table enumTable = LegacyEnumTypeTable("Side");
			enumTable["RIGHT"] = ACraft::Side::RIGHT;
			enumTable["LEFT"] = ACraft::Side::LEFT;
		}
		{
			sol::table enumTable = LegacyEnumTypeTable("CraftDeliverySequence");
			enumTable["FALL"] = ACraft::CraftDeliverySequence::FALL;
			enumTable["LAND"] = ACraft::CraftDeliverySequence::LAND;
			enumTable["STANDBY"] = ACraft::CraftDeliverySequence::STANDBY;
			enumTable["UNLOAD"] = ACraft::CraftDeliverySequence::UNLOAD;
			enumTable["LAUNCH"] = ACraft::CraftDeliverySequence::LAUNCH;
			enumTable["UNSTICK"] = ACraft::CraftDeliverySequence::UNSTICK;
		}
		{
			sol::table enumTable = LegacyEnumTypeTable("AltitudeMoveState");
			enumTable["HOVER"] = ACraft::AltitudeMoveState::HOVER;
			enumTable["DESCEND"] = ACraft::AltitudeMoveState::DESCEND;
			enumTable["ASCEND"] = ACraft::AltitudeMoveState::ASCEND;
		}
	}

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, ACRocket) {
		auto luaType = ConcreteTypeLuaClassDefinition(ACRocket, ACraft, Actor, MOSRotating, MOSprite, MovableObject, SceneObject, Entity);

		luaType["RightLeg"] = sol::property(&ACRocket::GetRightLeg, &LuaAdaptersPropertyOwnershipSafetyFaker::ACRocketSetRightLeg);
		luaType["LeftLeg"] = sol::property(&ACRocket::GetLeftLeg, &LuaAdaptersPropertyOwnershipSafetyFaker::ACRocketSetLeftLeg);
		luaType["MainEngine"] = sol::property(&ACRocket::GetMainThruster, &LuaAdaptersPropertyOwnershipSafetyFaker::ACRocketSetMainThruster);
		luaType["LeftEngine"] = sol::property(&ACRocket::GetLeftThruster, &LuaAdaptersPropertyOwnershipSafetyFaker::ACRocketSetLeftThruster);
		luaType["RightEngine"] = sol::property(&ACRocket::GetRightThruster, &LuaAdaptersPropertyOwnershipSafetyFaker::ACRocketSetRightThruster);
		luaType["LeftThruster"] = sol::property(&ACRocket::GetULeftThruster, &LuaAdaptersPropertyOwnershipSafetyFaker::ACRocketSetULeftThruster);
		luaType["RightThruster"] = sol::property(&ACRocket::GetURightThruster, &LuaAdaptersPropertyOwnershipSafetyFaker::ACRocketSetURightThruster);
		luaType["GearState"] = sol::property(&ACRocket::GetGearState);

		{
			sol::table enumTable = LegacyEnumTypeTable("LandingGearState");
			enumTable["RAISED"] = ACRocket::LandingGearState::RAISED;
			enumTable["LOWERED"] = ACRocket::LandingGearState::LOWERED;
			enumTable["LOWERING"] = ACRocket::LandingGearState::LOWERING;
			enumTable["RAISING"] = ACRocket::LandingGearState::RAISING;
			enumTable["GearStateCount"] = ACRocket::LandingGearState::GearStateCount;
		}
	}

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, Actor) {
		auto luaType = ConcreteTypeLuaClassDefinition(Actor, MOSRotating, MOSprite, MovableObject, SceneObject, Entity);

		luaType.set(sol::call_constructor, sol::constructors<
		                                       Actor()>());

		luaType["PlayerControllable"] = sol::property(&Actor::IsPlayerControllable, &Actor::SetPlayerControllable);
		luaType["BodyHitSound"] = sol::property(&Actor::GetBodyHitSound, &LuaAdaptersPropertyOwnershipSafetyFaker::ActorSetBodyHitSound);
		luaType["AlarmSound"] = sol::property(&Actor::GetAlarmSound, &LuaAdaptersPropertyOwnershipSafetyFaker::ActorSetAlarmSound);
		luaType["PainSound"] = sol::property(&Actor::GetPainSound, &LuaAdaptersPropertyOwnershipSafetyFaker::ActorSetPainSound);
		luaType["DeathSound"] = sol::property(&Actor::GetDeathSound, &LuaAdaptersPropertyOwnershipSafetyFaker::ActorSetDeathSound);
		luaType["DeviceSwitchSound"] = sol::property(&Actor::GetDeviceSwitchSound, &LuaAdaptersPropertyOwnershipSafetyFaker::ActorSetDeviceSwitchSound);
		luaType["ImpulseDamageThreshold"] = sol::property(&Actor::GetTravelImpulseDamage, &Actor::SetTravelImpulseDamage);
		luaType["StableRecoveryDelay"] = sol::property(&Actor::GetStableRecoverDelay, &Actor::SetStableRecoverDelay);
		luaType["Status"] = sol::property(&Actor::GetStatus, &Actor::SetStatus);
		luaType["Health"] = sol::property(&Actor::GetHealth, &Actor::SetHealth);
		luaType["PrevHealth"] = sol::property(&Actor::GetPrevHealth);
		luaType["MaxHealth"] = sol::property(&Actor::GetMaxHealth, &Actor::SetMaxHealth);
		luaType["InventoryMass"] = sol::property(&Actor::GetInventoryMass);
		luaType["GoldCarried"] = sol::property(&Actor::GetGoldCarried, &Actor::SetGoldCarried);
		luaType["AimRange"] = sol::property(&Actor::GetAimRange, &Actor::SetAimRange);
		luaType["CPUPos"] = sol::property(&Actor::GetCPUPos);
		luaType["EyePos"] = sol::property(&Actor::GetEyePos);
		luaType["HolsterOffset"] = sol::property(&Actor::GetHolsterOffset, &Actor::SetHolsterOffset);
		luaType["ReloadOffset"] = sol::property(&Actor::GetReloadOffset, &Actor::SetReloadOffset);
		luaType["ViewPoint"] = sol::property(&Actor::GetViewPoint, &Actor::SetViewPoint);
		luaType["ItemInReach"] = sol::property(&Actor::GetItemInReach, &Actor::SetItemInReach);
		luaType["SharpAimProgress"] = sol::property(&Actor::GetSharpAimProgress);
		luaType["Height"] = sol::property(&Actor::GetHeight);
		luaType["AIMode"] = sol::property(&Actor::GetAIMode, &Actor::SetAIMode);
		luaType["DeploymentID"] = sol::property(&Actor::GetDeploymentID);
		luaType["PassengerSlots"] = sol::property(&Actor::GetPassengerSlots, &Actor::SetPassengerSlots);
		luaType["Perceptiveness"] = sol::property(&Actor::GetPerceptiveness, &Actor::SetPerceptiveness);
		luaType["PainThreshold"] = sol::property(&Actor::GetPainThreshold, &Actor::SetPainThreshold);
		luaType["CanRevealUnseen"] = sol::property(&Actor::GetCanRevealUnseen, &Actor::SetCanRevealUnseen);
		luaType["InventorySize"] = sol::property(&Actor::GetInventorySize);
		luaType["MaxInventoryMass"] = sol::property(&Actor::GetMaxInventoryMass);
		luaType["MovePathSize"] = sol::property(&Actor::GetMovePathSize);
		luaType["MovePathEnd"] = sol::property(&Actor::GetMovePathEnd);
		luaType["IsWaitingOnNewMovePath"] = sol::property(&Actor::IsWaitingOnNewMovePath);
		luaType["AimDistance"] = sol::property(&Actor::GetAimDistance, &Actor::SetAimDistance);
		luaType["SightDistance"] = sol::property(&Actor::GetSightDistance, &Actor::SetSightDistance);
		luaType["PieMenu"] = sol::property(&Actor::GetPieMenu, &LuaAdaptersPropertyOwnershipSafetyFaker::ActorSetPieMenu);
		luaType["AIBaseDigStrength"] = sol::property(&Actor::GetAIBaseDigStrength, &Actor::SetAIBaseDigStrength);
		luaType["DigStrength"] = sol::property(&Actor::EstimateDigStrength);
		luaType["SceneWaypoints"] = sol::property(&LuaAdaptersActor::GetSceneWaypoints);
		luaType["LimbPushForcesAndCollisionsDisabled"] = sol::property(&Actor::GetLimbPushForcesAndCollisionsDisabled, &Actor::SetLimbPushForcesAndCollisionsDisabled);
		luaType["MoveProximityLimit"] = sol::property(&Actor::GetMoveProximityLimit, &Actor::SetMoveProximityLimit);

		luaType["MOMoveTarget"] = &Actor::m_pMOMoveTarget;
		luaType["MovePath"] = &Actor::m_MovePath;
		luaType["Inventory"] = &Actor::m_Inventory;

		luaType["GetController"] = &Actor::GetController;
		luaType["IsPlayerControlled"] = &Actor::IsPlayerControlled;
		luaType["IsControllable"] = &Actor::IsControllable;
		luaType["SetControllerMode"] = &Actor::SetControllerMode;
		luaType["SwapControllerModes"] = &Actor::SwapControllerModes;
		luaType["GetStableVelocityThreshold"] = &Actor::GetStableVel;
		luaType["SetStableVelocityThreshold"] = (void(Actor::*)(float, float)) & Actor::SetStableVel;
		luaType["SetStableVelocityThreshold"] = (void(Actor::*)(Vector)) & Actor::SetStableVel;
		luaType["GetAimAngle"] = &Actor::GetAimAngle;
		luaType["SetAimAngle"] = &Actor::SetAimAngle;
		luaType["HasObject"] = &Actor::HasObject;
		luaType["HasObjectInGroup"] = &Actor::HasObjectInGroup;
		luaType["IsWithinRange"] = &Actor::IsWithinRange;
		luaType["AddGold"] = &Actor::AddGold;
		luaType["AddHealth"] = &Actor::AddHealth;
		luaType["IsStatus"] = &Actor::IsStatus;
		luaType["IsDead"] = &Actor::IsDead;
		luaType["AddAISceneWaypoint"] = &Actor::AddAISceneWaypoint;
		luaType["AddAIMOWaypoint"] = &Actor::AddAIMOWaypoint;
		luaType["ClearAIWaypoints"] = &Actor::ClearAIWaypoints;
		luaType["GetLastAIWaypoint"] = &Actor::GetLastAIWaypoint;
		luaType["GetAIMOWaypointID"] = &Actor::GetAIMOWaypointID;
		luaType["GetWaypointListSize"] = &Actor::GetWaypointsSize;
		luaType["ClearMovePath"] = &Actor::ClearMovePath;
		luaType["AddToMovePathBeginning"] = &Actor::AddToMovePathBeginning;
		luaType["AddToMovePathEnd"] = &Actor::AddToMovePathEnd;
		luaType["RemoveMovePathBeginning"] = &Actor::RemoveMovePathBeginning;
		luaType["RemoveMovePathEnd"] = &Actor::RemoveMovePathEnd;
		luaType["AddInventoryItem"] = &Actor::AddInventoryItem; //; //, luabind::adopt(_2);
		luaType["RemoveInventoryItem"] = (void(Actor::*)(const std::string&)) & Actor::RemoveInventoryItem;
		luaType["RemoveInventoryItem"] = (void(Actor::*)(const std::string&, const std::string&)) & Actor::RemoveInventoryItem;
		luaType["RemoveInventoryItemAtIndex"] = &Actor::RemoveInventoryItemAtIndex; //, luabind::adopt(luabind::return_value))
		luaType["SwapNextInventory"] = &Actor::SwapNextInventory;
		luaType["SwapPrevInventory"] = &Actor::SwapPrevInventory;
		luaType["DropAllInventory"] = &Actor::DropAllInventory;
		luaType["DropAllGold"] = &Actor::DropAllGold;
		luaType["IsInventoryEmpty"] = &Actor::IsInventoryEmpty;
		luaType["FlashWhite"] = &Actor::FlashWhite;
		luaType["DrawWaypoints"] = &Actor::DrawWaypoints;
		luaType["SetMovePathToUpdate"] = &Actor::SetMovePathToUpdate;
		luaType["UpdateMovePath"] = &Actor::UpdateMovePath;
		luaType["SetAlarmPoint"] = &Actor::AlarmPoint;
		luaType["GetAlarmPoint"] = &Actor::GetAlarmPoint;
		luaType["IsOrganic"] = &Actor::IsOrganic;
		luaType["IsMechanical"] = &Actor::IsMechanical;

		{
			sol::table enumTable = LegacyEnumTypeTable("Status");
			enumTable["STABLE"] = Actor::Status::STABLE;
			enumTable["UNSTABLE"] = Actor::Status::UNSTABLE;
			enumTable["INACTIVE"] = Actor::Status::INACTIVE;
			enumTable["DYING"] = Actor::Status::DYING;
			enumTable["DEAD"] = Actor::Status::DEAD;
		}
		{
			sol::table enumTable = LegacyEnumTypeTable("MovementState");
			enumTable["NOMOVE"] = Actor::MovementState::NOMOVE;
			enumTable["STAND"] = Actor::MovementState::STAND;
			enumTable["WALK"] = Actor::MovementState::WALK;
			enumTable["JUMP"] = Actor::MovementState::JUMP;
			enumTable["DISLODGE"] = Actor::MovementState::DISLODGE;
			enumTable["CROUCH"] = Actor::MovementState::CROUCH;
			enumTable["CRAWL"] = Actor::MovementState::CRAWL;
			enumTable["ARMCRAWL"] = Actor::MovementState::ARMCRAWL;
			enumTable["CLIMB"] = Actor::MovementState::CLIMB;
			enumTable["MOVEMENTSTATECOUNT"] = Actor::MovementState::MOVEMENTSTATECOUNT;
		}
		{
			sol::table enumTable = LegacyEnumTypeTable("AIMode");
			enumTable["AIMODE_NONE"] = Actor::AIMode::AIMODE_NONE;
			enumTable["AIMODE_SENTRY"] = Actor::AIMode::AIMODE_SENTRY;
			enumTable["AIMODE_PATROL"] = Actor::AIMode::AIMODE_PATROL;
			enumTable["AIMODE_GOTO"] = Actor::AIMode::AIMODE_GOTO;
			enumTable["AIMODE_BRAINHUNT"] = Actor::AIMode::AIMODE_BRAINHUNT;
			enumTable["AIMODE_GOLDDIG"] = Actor::AIMode::AIMODE_GOLDDIG;
			enumTable["AIMODE_RETURN"] = Actor::AIMode::AIMODE_RETURN;
			enumTable["AIMODE_STAY"] = Actor::AIMode::AIMODE_STAY;
			enumTable["AIMODE_SCUTTLE"] = Actor::AIMode::AIMODE_SCUTTLE;
			enumTable["AIMODE_DELIVER"] = Actor::AIMode::AIMODE_DELIVER;
			enumTable["AIMODE_BOMB"] = Actor::AIMode::AIMODE_BOMB;
			enumTable["AIMODE_SQUAD"] = Actor::AIMode::AIMODE_SQUAD;
			enumTable["AIMODE_COUNT"] = Actor::AIMode::AIMODE_COUNT;
		}
		{
			sol::table enumTable = LegacyEnumTypeTable("ActionState");
			enumTable["MOVING"] = Actor::ActionState::MOVING;
			enumTable["MOVING_FAST"] = Actor::ActionState::MOVING_FAST;
			enumTable["FIRING"] = Actor::ActionState::FIRING;
			enumTable["ActionStateCount"] = Actor::ActionState::ActionStateCount;
		}
		{
			sol::table enumTable = LegacyEnumTypeTable("AimState");
			enumTable["AIMSTILL"] = Actor::AimState::AIMSTILL;
			enumTable["AIMUP"] = Actor::AimState::AIMUP;
			enumTable["AIMDOWN"] = Actor::AimState::AIMDOWN;
			enumTable["AimStateCount"] = Actor::AimState::AimStateCount;
		}
		{
			sol::table enumTable = LegacyEnumTypeTable("LateralMoveState");
			enumTable["LAT_STILL"] = Actor::LateralMoveState::LAT_STILL;
			enumTable["LAT_LEFT"] = Actor::LateralMoveState::LAT_LEFT;
			enumTable["LAT_RIGHT"] = Actor::LateralMoveState::LAT_RIGHT;
		}
		{
			sol::table enumTable = LegacyEnumTypeTable("ObstacleState");
			enumTable["PROCEEDING"] = Actor::ObstacleState::PROCEEDING;
			enumTable["BACKSTEPPING"] = Actor::ObstacleState::BACKSTEPPING;
			enumTable["DIGPAUSING"] = Actor::ObstacleState::DIGPAUSING;
			enumTable["JUMPING"] = Actor::ObstacleState::JUMPING;
			enumTable["SOFTLANDING"] = Actor::ObstacleState::SOFTLANDING;
		}
		{
			sol::table enumTable = LegacyEnumTypeTable("TeamBlockState");
			enumTable["NOTBLOCKED"] = Actor::TeamBlockState::NOTBLOCKED;
			enumTable["BLOCKED"] = Actor::TeamBlockState::BLOCKED;
			enumTable["IGNORINGBLOCK"] = Actor::TeamBlockState::IGNORINGBLOCK;
			enumTable["FOLLOWWAIT"] = Actor::TeamBlockState::FOLLOWWAIT;
		}
	}

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, ADoor) {
		auto luaType = ConcreteTypeLuaClassDefinition(ADoor, Actor, MOSRotating, MOSprite, MovableObject, SceneObject, Entity);

		luaType["Door"] = sol::property(&ADoor::GetDoor, &LuaAdaptersPropertyOwnershipSafetyFaker::ADoorSetDoor);
		luaType["DoorMoveStartSound"] = sol::property(&ADoor::GetDoorMoveStartSound, &LuaAdaptersPropertyOwnershipSafetyFaker::ADoorSetDoorMoveStartSound);
		luaType["DoorMoveSound"] = sol::property(&ADoor::GetDoorMoveSound, &LuaAdaptersPropertyOwnershipSafetyFaker::ADoorSetDoorMoveSound);
		luaType["DoorDirectionChangeSound"] = sol::property(&ADoor::GetDoorDirectionChangeSound, &LuaAdaptersPropertyOwnershipSafetyFaker::ADoorSetDoorDirectionChangeSound);
		luaType["DoorMoveEndSound"] = sol::property(&ADoor::GetDoorMoveEndSound, &LuaAdaptersPropertyOwnershipSafetyFaker::ADoorSetDoorMoveEndSound);

		luaType["GetDoorState"] = &ADoor::GetDoorState;
		luaType["OpenDoor"] = &ADoor::OpenDoor;
		luaType["CloseDoor"] = &ADoor::CloseDoor;
		luaType["StopDoor"] = &ADoor::StopDoor;
		luaType["ResetSensorTimer"] = &ADoor::ResetSensorTimer;
		luaType["SetClosedByDefault"] = &ADoor::SetClosedByDefault;

		{
			sol::table enumTable = LegacyEnumTypeTable("DoorState");
			enumTable["CLOSED"] = ADoor::DoorState::CLOSED;
			enumTable["OPENING"] = ADoor::DoorState::OPENING;
			enumTable["OPEN"] = ADoor::DoorState::OPEN;
			enumTable["CLOSING"] = ADoor::DoorState::CLOSING;
			enumTable["STOPPED"] = ADoor::DoorState::STOPPED;
		}
	}

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, AHuman) {
		auto luaType = ConcreteTypeLuaClassDefinition(AHuman, Actor, MOSRotating, MOSprite, MovableObject, SceneObject, Entity);

		luaType.set(sol::call_constructor, sol::constructors<
		                                       AHuman()>());

		luaType["Head"] = sol::property(&AHuman::GetHead, &LuaAdaptersPropertyOwnershipSafetyFaker::AHumanSetHead);
		luaType["Jetpack"] = sol::property(&AHuman::GetJetpack, &LuaAdaptersPropertyOwnershipSafetyFaker::AHumanSetJetpack);
		luaType["FGArm"] = sol::property(&AHuman::GetFGArm, &LuaAdaptersPropertyOwnershipSafetyFaker::AHumanSetFGArm);
		luaType["BGArm"] = sol::property(&AHuman::GetBGArm, &LuaAdaptersPropertyOwnershipSafetyFaker::AHumanSetBGArm);
		luaType["FGLeg"] = sol::property(&AHuman::GetFGLeg, &LuaAdaptersPropertyOwnershipSafetyFaker::AHumanSetFGLeg);
		luaType["BGLeg"] = sol::property(&AHuman::GetBGLeg, &LuaAdaptersPropertyOwnershipSafetyFaker::AHumanSetBGLeg);
		luaType["FGFoot"] = sol::property(&AHuman::GetFGFoot, &LuaAdaptersPropertyOwnershipSafetyFaker::AHumanSetFGFoot);
		luaType["BGFoot"] = sol::property(&AHuman::GetBGFoot, &LuaAdaptersPropertyOwnershipSafetyFaker::AHumanSetBGFoot);
		luaType["MaxWalkPathCrouchShift"] = sol::property(&AHuman::GetMaxWalkPathCrouchShift, &AHuman::SetMaxWalkPathCrouchShift);
		luaType["MaxCrouchRotation"] = sol::property(&AHuman::GetMaxCrouchRotation, &AHuman::SetMaxCrouchRotation);
		luaType["CrouchAmount"] = sol::property(&AHuman::GetCrouchAmount);
		luaType["CrouchAmountOverride"] = sol::property(&AHuman::GetCrouchAmountOverride, &AHuman::SetCrouchAmountOverride);
		luaType["StrideSound"] = sol::property(&AHuman::GetStrideSound, &LuaAdaptersPropertyOwnershipSafetyFaker::AHumanSetStrideSound);
		luaType["StrideFrame"] = sol::property(&AHuman::StrideFrame);
		luaType["UpperBodyState"] = sol::property(&AHuman::GetUpperBodyState, &AHuman::SetUpperBodyState);
		luaType["MovementState"] = sol::property(&AHuman::GetMovementState, &AHuman::SetMovementState);
		luaType["ProneState"] = sol::property(&AHuman::GetProneState, &AHuman::SetProneState);
		luaType["ThrowPrepTime"] = sol::property(&AHuman::GetThrowPrepTime, &AHuman::SetThrowPrepTime);
		luaType["ThrowProgress"] = sol::property(&AHuman::GetThrowProgress);
		luaType["EquippedItem"] = sol::property(&AHuman::GetEquippedItem);
		luaType["EquippedBGItem"] = sol::property(&AHuman::GetEquippedBGItem);
		luaType["EquippedMass"] = sol::property(&AHuman::GetEquippedMass);
		luaType["FirearmIsReady"] = sol::property(&AHuman::FirearmIsReady);
		luaType["ThrowableIsReady"] = sol::property(&AHuman::ThrowableIsReady);
		luaType["FirearmIsEmpty"] = sol::property(&AHuman::FirearmIsEmpty);
		luaType["FirearmNeedsReload"] = sol::property(&AHuman::FirearmNeedsReload);
		luaType["FirearmIsSemiAuto"] = sol::property(&AHuman::FirearmIsSemiAuto);
		luaType["FirearmActivationDelay"] = sol::property(&AHuman::FirearmActivationDelay);
		luaType["LimbPathPushForce"] = sol::property(&AHuman::GetLimbPathPushForce, &AHuman::SetLimbPathPushForce);
		luaType["IsClimbing"] = sol::property(&AHuman::IsClimbing);
		luaType["ArmSwingRate"] = sol::property(&AHuman::GetArmSwingRate, &AHuman::SetArmSwingRate);
		luaType["DeviceArmSwayRate"] = sol::property(&AHuman::GetDeviceArmSwayRate, &AHuman::SetDeviceArmSwayRate);

		luaType["EquipFirearm"] = &AHuman::EquipFirearm;
		luaType["EquipThrowable"] = &AHuman::EquipThrowable;
		luaType["EquipDiggingTool"] = &AHuman::EquipDiggingTool;
		luaType["EquipShield"] = &AHuman::EquipShield;
		luaType["EquipShieldInBGArm"] = &AHuman::EquipShieldInBGArm;
		luaType["EquipDeviceInGroup"] = &AHuman::EquipDeviceInGroup;
		luaType["EquipNamedDevice"] = (bool(AHuman::*)(const std::string&, bool)) & AHuman::EquipNamedDevice;
		luaType["EquipNamedDevice"] = (bool(AHuman::*)(const std::string&, const std::string&, bool)) & AHuman::EquipNamedDevice;
		luaType["EquipLoadedFirearmInGroup"] = &AHuman::EquipLoadedFirearmInGroup;
		luaType["UnequipFGArm"] = &AHuman::UnequipFGArm;
		luaType["UnequipBGArm"] = &AHuman::UnequipBGArm;
		luaType["UnequipArms"] = &AHuman::UnequipArms;
		luaType["ReloadFirearms"] = &LuaAdaptersAHuman::ReloadFirearms;
		luaType["ReloadFirearms"] = &AHuman::ReloadFirearms;
		luaType["FirearmsAreReloading"] = &AHuman::FirearmsAreReloading;
		luaType["IsWithinRange"] = &AHuman::IsWithinRange;
		luaType["Look"] = &AHuman::Look;
		luaType["LookForGold"] = &AHuman::LookForGold;
		luaType["LookForMOs"] = &AHuman::LookForMOs;
		luaType["GetLimbPath"] = &AHuman::GetLimbPath;
		luaType["GetLimbPathSpeed"] = &AHuman::GetLimbPathSpeed;
		luaType["SetLimbPathSpeed"] = &AHuman::SetLimbPathSpeed;
		luaType["GetRotAngleTarget"] = &AHuman::GetRotAngleTarget;
		luaType["SetRotAngleTarget"] = &AHuman::SetRotAngleTarget;
		luaType["GetWalkAngle"] = &AHuman::GetWalkAngle;
		luaType["SetWalkAngle"] = &AHuman::SetWalkAngle;

		{
			sol::table enumTable = LegacyEnumTypeTable("UpperBodyState");
			enumTable["WEAPON_READY"] = AHuman::UpperBodyState::WEAPON_READY;
			enumTable["AIMING_SHARP"] = AHuman::UpperBodyState::AIMING_SHARP;
			enumTable["HOLSTERING_BACK"] = AHuman::UpperBodyState::HOLSTERING_BACK;
			enumTable["HOLSTERING_BELT"] = AHuman::UpperBodyState::HOLSTERING_BELT;
			enumTable["DEHOLSTERING_BACK"] = AHuman::UpperBodyState::DEHOLSTERING_BACK;
			enumTable["DEHOLSTERING_BELT"] = AHuman::UpperBodyState::DEHOLSTERING_BELT;
			enumTable["THROWING_PREP"] = AHuman::UpperBodyState::THROWING_PREP;
			enumTable["THROWING_RELEASE"] = AHuman::UpperBodyState::THROWING_RELEASE;
		}
		{
			sol::table enumTable = LegacyEnumTypeTable("MovementState");
			enumTable["NOMOVE"] = AHuman::MovementState::NOMOVE;
			enumTable["STAND"] = AHuman::MovementState::STAND;
			enumTable["WALK"] = AHuman::MovementState::WALK;
			enumTable["CROUCH"] = AHuman::MovementState::CROUCH;
			enumTable["CRAWL"] = AHuman::MovementState::CRAWL;
			enumTable["ARMCRAWL"] = AHuman::MovementState::ARMCRAWL;
			enumTable["CLIMB"] = AHuman::MovementState::CLIMB;
			enumTable["JUMP"] = AHuman::MovementState::JUMP;
			enumTable["DISLODGE"] = AHuman::MovementState::DISLODGE;
			enumTable["MOVEMENTSTATECOUNT"] = AHuman::MovementState::MOVEMENTSTATECOUNT;
		}
		{
			sol::table enumTable = LegacyEnumTypeTable("ProneState");
			enumTable["NOTPRONE"] = AHuman::ProneState::NOTPRONE;
			enumTable["GOPRONE"] = AHuman::ProneState::GOPRONE;
			enumTable["PRONE"] = AHuman::ProneState::PRONE;
			enumTable["PRONESTATECOUNT"] = AHuman::ProneState::PRONESTATECOUNT;
		}
		{
			sol::table enumTable = LegacyEnumTypeTable("Layer");
			enumTable["FGROUND"] = AHuman::Layer::FGROUND;
			enumTable["BGROUND"] = AHuman::Layer::BGROUND;
		}
		{
			sol::table enumTable = LegacyEnumTypeTable("DeviceHandlingState");
			enumTable["STILL"] = AHuman::DeviceHandlingState::STILL;
			enumTable["POINTING"] = AHuman::DeviceHandlingState::POINTING;
			enumTable["SCANNING"] = AHuman::DeviceHandlingState::SCANNING;
			enumTable["AIMING"] = AHuman::DeviceHandlingState::AIMING;
			enumTable["FIRING"] = AHuman::DeviceHandlingState::FIRING;
			enumTable["THROWING"] = AHuman::DeviceHandlingState::THROWING;
			enumTable["DIGGING"] = AHuman::DeviceHandlingState::DIGGING;
		}
		{
			sol::table enumTable = LegacyEnumTypeTable("SweepState");
			enumTable["NOSWEEP"] = AHuman::SweepState::NOSWEEP;
			enumTable["SWEEPINGUP"] = AHuman::SweepState::SWEEPINGUP;
			enumTable["SWEEPUPPAUSE"] = AHuman::SweepState::SWEEPUPPAUSE;
			enumTable["SWEEPINGDOWN"] = AHuman::SweepState::SWEEPINGDOWN;
			enumTable["SWEEPDOWNPAUSE"] = AHuman::SweepState::SWEEPDOWNPAUSE;
		}
		{
			sol::table enumTable = LegacyEnumTypeTable("DigState");
			enumTable["NOTDIGGING"] = AHuman::DigState::NOTDIGGING;
			enumTable["PREDIG"] = AHuman::DigState::PREDIG;
			enumTable["STARTDIG"] = AHuman::DigState::STARTDIG;
			enumTable["TUNNELING"] = AHuman::DigState::TUNNELING;
			enumTable["FINISHINGDIG"] = AHuman::DigState::FINISHINGDIG;
			enumTable["PAUSEDIGGER"] = AHuman::DigState::PAUSEDIGGER;
		}
		{
			sol::table enumTable = LegacyEnumTypeTable("JumpState");
			enumTable["NOTJUMPING"] = AHuman::JumpState::NOTJUMPING;
			enumTable["FORWARDJUMP"] = AHuman::JumpState::FORWARDJUMP;
			enumTable["PREJUMP"] = AHuman::JumpState::PREUPJUMP;
			enumTable["UPJUMP"] = AHuman::JumpState::UPJUMP;
			enumTable["APEXJUMP"] = AHuman::JumpState::APEXJUMP;
			enumTable["LANDJUMP"] = AHuman::JumpState::LANDJUMP;
		}
	}

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, Arm) {
		auto luaType = ConcreteTypeLuaClassDefinition(Arm, Attachable, MOSRotating, MOSprite, MovableObject, SceneObject, Entity);

		luaType["MaxLength"] = sol::property(&Arm::GetMaxLength);
		luaType["MoveSpeed"] = sol::property(&Arm::GetMoveSpeed, &Arm::SetMoveSpeed);

		luaType["HandIdleOffset"] = sol::property(&Arm::GetHandIdleOffset, &Arm::SetHandIdleOffset);

		luaType["HandPos"] = sol::property(&Arm::GetHandPos, &Arm::SetHandPos);
		luaType["HasAnyHandTargets"] = sol::property(&Arm::HasAnyHandTargets);
		luaType["NumberOfHandTargets"] = sol::property(&Arm::GetNumberOfHandTargets);
		luaType["NextHandTargetDescription"] = sol::property(&Arm::GetNextHandTargetDescription);
		luaType["NextHandTargetPosition"] = sol::property(&Arm::GetNextHandTargetPosition);
		luaType["HandHasReachedCurrentTarget"] = sol::property(&Arm::GetHandHasReachedCurrentTarget);

		luaType["GripStrength"] = sol::property(&Arm::GetGripStrength, &Arm::SetGripStrength);
		luaType["ThrowStrength"] = sol::property(&Arm::GetThrowStrength, &Arm::SetThrowStrength);

		luaType["HeldDevice"] = sol::property(&Arm::GetHeldDevice, &LuaAdaptersPropertyOwnershipSafetyFaker::ArmSetHeldDevice);
		luaType["SupportedHeldDevice"] = sol::property(&Arm::GetHeldDeviceThisArmIsTryingToSupport);

		luaType["AddHandTarget"] = (void(Arm::*)(const std::string& description, const Vector& handTargetPositionToAdd)) & Arm::AddHandTarget;
		luaType["AddHandTarget"] = (void(Arm::*)(const std::string& description, const Vector& handTargetPositionToAdd, float delayAtTarget)) & Arm::AddHandTarget;
		luaType["RemoveNextHandTarget"] = &Arm::RemoveNextHandTarget;
		luaType["ClearHandTargets"] = &Arm::ClearHandTargets;
	}

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, Leg) {
		auto luaType = ConcreteTypeLuaClassDefinition(Leg, Attachable, MOSRotating, MOSprite, MovableObject, SceneObject, Entity);

		luaType["Foot"] = sol::property(&Leg::GetFoot, &LuaAdaptersPropertyOwnershipSafetyFaker::LegSetFoot);
		luaType["MoveSpeed"] = sol::property(&Leg::GetMoveSpeed, &Leg::SetMoveSpeed);
	}

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, LimbPath) {
		auto luaType = SimpleTypeLuaClassDefinition(LimbPath);

		luaType["StartOffset"] = sol::property(&LimbPath::GetStartOffset, &LimbPath::SetStartOffset);
		luaType["SegmentCount"] = sol::property(&LimbPath::GetSegCount);
		luaType["TravelSpeedMultiplier"] = sol::property(&LimbPath::GetTravelSpeedMultiplier, &LimbPath::SetTravelSpeedMultiplier);

		luaType["GetSegment"] = &LimbPath::GetSegment;
	}
} // namespace RTE
