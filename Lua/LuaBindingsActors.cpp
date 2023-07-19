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
#include "Turret.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, ACDropShip) {
		auto luaType = ConcreteTypeLuaClassDefinition(ACDropShip, ACraft);

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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, ACrab) {
		auto luaType = ConcreteTypeLuaClassDefinition(ACrab, Actor);

		luaType.set(sol::meta_function::construct, sol::constructors<
			ACrab()
		>());

		luaType["Turret"] = sol::property(&ACrab::GetTurret, &LuaAdaptersPropertyOwnershipSafetyFaker::ACrabSetTurret);
		luaType["Jetpack"] = sol::property(&ACrab::GetJetpack, &LuaAdaptersPropertyOwnershipSafetyFaker::ACrabSetJetpack);
		luaType["LeftFGLeg"] = sol::property(&ACrab::GetLeftFGLeg, &LuaAdaptersPropertyOwnershipSafetyFaker::ACrabSetLeftFGLeg);
		luaType["LeftBGLeg"] = sol::property(&ACrab::GetLeftBGLeg, &LuaAdaptersPropertyOwnershipSafetyFaker::ACrabSetLeftBGLeg);
		luaType["RightFGLeg"] = sol::property(&ACrab::GetRightFGLeg, &LuaAdaptersPropertyOwnershipSafetyFaker::ACrabSetRightFGLeg);
		luaType["RightBGLeg"] = sol::property(&ACrab::GetRightBGLeg, &LuaAdaptersPropertyOwnershipSafetyFaker::ACrabSetRightBGLeg);
		luaType["StrideSound"] = sol::property(&ACrab::GetStrideSound, &LuaAdaptersPropertyOwnershipSafetyFaker::ACrabSetStrideSound);
		luaType["JetTimeTotal"] = sol::property(&ACrab::GetJetTimeTotal, &ACrab::SetJetTimeTotal);
		luaType["JetTimeLeft"] = sol::property(&ACrab::GetJetTimeLeft);
		luaType["JetReplenishRate"] = sol::property(&ACrab::GetJetReplenishRate, &ACrab::SetJetReplenishRate);
		luaType["JetAngleRange"] = sol::property(&ACrab::GetJetAngleRange, &ACrab::SetJetAngleRange);
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

		luaType.new_enum("MovementState", EnumList(ACrab::MovementState) {
			{ "STAND", ACrab::MovementState::STAND },
			{ "WALK", ACrab::MovementState::WALK },
			{ "JUMP", ACrab::MovementState::JUMP },
			{ "DISLODGE", ACrab::MovementState::DISLODGE },
			{ "MOVEMENTSTATECOUNT", ACrab::MovementState::MOVEMENTSTATECOUNT }
		});
		luaType.new_enum("Side", EnumList(ACrab::Side) {
			{ "LEFTSIDE", ACrab::Side::LEFTSIDE },
			{ "RIGHTSIDE", ACrab::Side::RIGHTSIDE },
			{ "SIDECOUNT", ACrab::Side::SIDECOUNT }
		});
		luaType.new_enum("Layer", EnumList(ACrab::Layer) {
			{ "FGROUND", ACrab::Layer::FGROUND },
			{ "BGROUND", ACrab::Layer::BGROUND }
		});
		luaType.new_enum("DeviceHandlingState", EnumList(ACrab::DeviceHandlingState) {
			{ "STILL", ACrab::DeviceHandlingState::STILL },
			{ "POINTING", ACrab::DeviceHandlingState::POINTING },
			{ "SCANNING", ACrab::DeviceHandlingState::SCANNING },
			{ "AIMING", ACrab::DeviceHandlingState::AIMING },
			{ "FIRING", ACrab::DeviceHandlingState::FIRING },
			{ "THROWING", ACrab::DeviceHandlingState::THROWING },
			{ "DIGGING", ACrab::DeviceHandlingState::DIGGING }
		});
		luaType.new_enum("SweepState", EnumList(ACrab::SweepState) {
			{ "NOSWEEP", ACrab::SweepState::NOSWEEP },
			{ "SWEEPINGUP", ACrab::SweepState::SWEEPINGUP },
			{ "SWEEPUPPAUSE", ACrab::SweepState::SWEEPUPPAUSE },
			{ "SWEEPINGDOWN", ACrab::SweepState::SWEEPINGDOWN },
			{ "SWEEPDOWNPAUSE", ACrab::SweepState::SWEEPDOWNPAUSE }
		});
		luaType.new_enum("DigState", EnumList(ACrab::DigState) {
			{ "NOTDIGGING", ACrab::DigState::NOTDIGGING },
			{ "PREDIG", ACrab::DigState::PREDIG },
			{ "STARTDIG", ACrab::DigState::STARTDIG },
			{ "TUNNELING", ACrab::DigState::TUNNELING },
			{ "FINISHINGDIG", ACrab::DigState::FINISHINGDIG },
			{ "PAUSEDIGGER", ACrab::DigState::PAUSEDIGGER }
		});
		luaType.new_enum("JumpState", EnumList(ACrab::JumpState) {
			{ "NOTJUMPING", ACrab::JumpState::NOTJUMPING },
			{ "FORWARDJUMP", ACrab::JumpState::FORWARDJUMP },
			{ "PREJUMP", ACrab::JumpState::PREUPJUMP },
			{ "UPJUMP", ACrab::JumpState::UPJUMP },
			{ "APEXJUMP", ACrab::JumpState::APEXJUMP },
			{ "LANDJUMP", ACrab::JumpState::LANDJUMP }
		});
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, ACraft) {
		auto luaType = AbstractTypeLuaClassDefinition(ACraft, Actor);

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

		luaType.new_enum("HatchState", EnumList(ACraft::HatchState) {
			{ "CLOSED", ACraft::HatchState::CLOSED },
			{ "OPENING", ACraft::HatchState::OPENING },
			{ "OPEN", ACraft::HatchState::OPEN },
			{ "CLOSING", ACraft::HatchState::CLOSING },
			{ "HatchStateCount", ACraft::HatchState::HatchStateCount }
		});
		luaType.new_enum("Side", EnumList(ACraft::Side) {
			{ "RIGHT", ACraft::Side::RIGHT },
			{ "LEFT", ACraft::Side::LEFT }
		});
		luaType.new_enum("CraftDeliverySequence", EnumList(ACraft::CraftDeliverySequence) {
			{ "FALL", ACraft::CraftDeliverySequence::FALL },
			{ "LAND", ACraft::CraftDeliverySequence::LAND },
			{ "STANDBY", ACraft::CraftDeliverySequence::STANDBY },
			{ "UNLOAD", ACraft::CraftDeliverySequence::UNLOAD },
			{ "LAUNCH", ACraft::CraftDeliverySequence::LAUNCH },
			{ "UNSTICK", ACraft::CraftDeliverySequence::UNSTICK }
		});
		luaType.new_enum("AltitudeMoveState", EnumList(ACraft::AltitudeMoveState) {
			{ "HOVER", ACraft::AltitudeMoveState::HOVER },
			{ "DESCEND", ACraft::AltitudeMoveState::DESCEND },
			{ "ASCEND", ACraft::AltitudeMoveState::ASCEND }
		});
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, ACRocket) {
		auto luaType = ConcreteTypeLuaClassDefinition(ACRocket, ACraft);

		luaType["RightLeg"] = sol::property(&ACRocket::GetRightLeg, &LuaAdaptersPropertyOwnershipSafetyFaker::ACRocketSetRightLeg);
		luaType["LeftLeg"] = sol::property(&ACRocket::GetLeftLeg, &LuaAdaptersPropertyOwnershipSafetyFaker::ACRocketSetLeftLeg);
		luaType["MainEngine"] = sol::property(&ACRocket::GetMainThruster, &LuaAdaptersPropertyOwnershipSafetyFaker::ACRocketSetMainThruster);
		luaType["LeftEngine"] = sol::property(&ACRocket::GetLeftThruster, &LuaAdaptersPropertyOwnershipSafetyFaker::ACRocketSetLeftThruster);
		luaType["RightEngine"] = sol::property(&ACRocket::GetRightThruster, &LuaAdaptersPropertyOwnershipSafetyFaker::ACRocketSetRightThruster);
		luaType["LeftThruster"] = sol::property(&ACRocket::GetULeftThruster, &LuaAdaptersPropertyOwnershipSafetyFaker::ACRocketSetULeftThruster);
		luaType["RightThruster"] = sol::property(&ACRocket::GetURightThruster, &LuaAdaptersPropertyOwnershipSafetyFaker::ACRocketSetURightThruster);
		luaType["GearState"] = sol::property(&ACRocket::GetGearState);

		luaType.new_enum("LandingGearState", EnumList(ACRocket::LandingGearState) {
			{ "RAISED", ACRocket::LandingGearState::RAISED },
			{ "LOWERED", ACRocket::LandingGearState::LOWERED },
			{ "LOWERING", ACRocket::LandingGearState::LOWERING },
			{ "RAISING", ACRocket::LandingGearState::RAISING },
			{ "GearStateCount", ACRocket::LandingGearState::GearStateCount }
		});
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, Actor) {
		auto luaType = ConcreteTypeLuaClassDefinition(Actor, MOSRotating);

		luaType.set(sol::meta_function::construct, sol::constructors<
			Actor()
		>());

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

		luaType["MOMoveTarget"] = &Actor::m_pMOMoveTarget;
		luaType["MovePath"] = &Actor::m_MovePath;
		luaType["Inventory"] = &Actor::m_Inventory;

		luaType["GetController"] = &Actor::GetController;
		luaType["IsPlayerControlled"] = &Actor::IsPlayerControlled;
		luaType["IsControllable"] = &Actor::IsControllable;
		luaType["SetControllerMode"] = &Actor::SetControllerMode;
		luaType["SwapControllerModes"] = &Actor::SwapControllerModes;
		luaType["GetStableVelocityThreshold"] = &Actor::GetStableVel;
		luaType["SetStableVelocityThreshold"] = (void (Actor::*)(float, float))&Actor::SetStableVel;
		luaType["SetStableVelocityThreshold"] = (void (Actor::*)(Vector))&Actor::SetStableVel;
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
		luaType["AddInventoryItem"] = &Actor::AddInventoryItem;//; //, luabind::adopt(_2);
		luaType["RemoveInventoryItem"] = (void (Actor::*)(const std::string &))&Actor::RemoveInventoryItem;
		luaType["RemoveInventoryItem"] = (void (Actor::*)(const std::string &, const std::string &))&Actor::RemoveInventoryItem;
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

		luaType.new_enum("Status", EnumList(Actor::Status) {
			{ "STABLE", Actor::Status::STABLE },
			{ "UNSTABLE", Actor::Status::UNSTABLE },
			{ "INACTIVE", Actor::Status::INACTIVE },
			{ "DYING", Actor::Status::DYING },
			{ "DEAD", Actor::Status::DEAD }
		});
		luaType.new_enum("AIMode", EnumList(Actor::AIMode) {
			{ "AIMODE_NONE", Actor::AIMode::AIMODE_NONE },
			{ "AIMODE_SENTRY", Actor::AIMode::AIMODE_SENTRY },
			{ "AIMODE_PATROL", Actor::AIMode::AIMODE_PATROL },
			{ "AIMODE_GOTO", Actor::AIMode::AIMODE_GOTO },
			{ "AIMODE_BRAINHUNT", Actor::AIMode::AIMODE_BRAINHUNT },
			{ "AIMODE_GOLDDIG", Actor::AIMode::AIMODE_GOLDDIG },
			{ "AIMODE_RETURN", Actor::AIMode::AIMODE_RETURN },
			{ "AIMODE_STAY", Actor::AIMode::AIMODE_STAY },
			{ "AIMODE_SCUTTLE", Actor::AIMode::AIMODE_SCUTTLE },
			{ "AIMODE_DELIVER", Actor::AIMode::AIMODE_DELIVER },
			{ "AIMODE_BOMB", Actor::AIMode::AIMODE_BOMB },
			{ "AIMODE_SQUAD", Actor::AIMode::AIMODE_SQUAD },
			{ "AIMODE_COUNT", Actor::AIMode::AIMODE_COUNT }
		});
		luaType.new_enum("ActionState", EnumList(Actor::ActionState) {
			{ "MOVING", Actor::ActionState::MOVING },
			{ "MOVING_FAST", Actor::ActionState::MOVING_FAST },
			{ "FIRING", Actor::ActionState::FIRING },
			{ "ActionStateCount", Actor::ActionState::ActionStateCount }
		});
		luaType.new_enum("AimState", EnumList(Actor::AimState) {
			{ "AIMSTILL", Actor::AimState::AIMSTILL },
			{ "AIMUP", Actor::AimState::AIMUP },
			{ "AIMDOWN", Actor::AimState::AIMDOWN },
			{ "AimStateCount", Actor::AimState::AimStateCount }
		});
		luaType.new_enum("LateralMoveState", EnumList(Actor::LateralMoveState) {
			{ "LAT_STILL", Actor::LateralMoveState::LAT_STILL },
			{ "LAT_LEFT", Actor::LateralMoveState::LAT_LEFT },
			{ "LAT_RIGHT", Actor::LateralMoveState::LAT_RIGHT }
		});
		luaType.new_enum("ObstacleState", EnumList(Actor::ObstacleState) {
			{ "PROCEEDING", Actor::ObstacleState::PROCEEDING },
			{ "BACKSTEPPING", Actor::ObstacleState::BACKSTEPPING },
			{ "DIGPAUSING", Actor::ObstacleState::DIGPAUSING },
			{ "JUMPING", Actor::ObstacleState::JUMPING },
			{ "SOFTLANDING", Actor::ObstacleState::SOFTLANDING }
		});
		luaType.new_enum("TeamBlockState", EnumList(Actor::TeamBlockState) {
			{ "NOTBLOCKED", Actor::TeamBlockState::NOTBLOCKED },
			{ "BLOCKED", Actor::TeamBlockState::BLOCKED },
			{ "IGNORINGBLOCK", Actor::TeamBlockState::IGNORINGBLOCK },
			{ "FOLLOWWAIT", Actor::TeamBlockState::FOLLOWWAIT }
		});
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, ADoor) {
		auto luaType = ConcreteTypeLuaClassDefinition(ADoor, Actor);

		luaType["Door"] = sol::property(&ADoor::GetDoor, &LuaAdaptersPropertyOwnershipSafetyFaker::ADoorSetDoor);
		luaType["DoorMoveStartSound"] = sol::property(&ADoor::GetDoorMoveStartSound, &LuaAdaptersPropertyOwnershipSafetyFaker::ADoorSetDoorMoveStartSound);
		luaType["DoorMoveSound"] = sol::property(&ADoor::GetDoorMoveSound, &LuaAdaptersPropertyOwnershipSafetyFaker::ADoorSetDoorMoveSound);
		luaType["DoorDirectionChangeSound"] = sol::property(&ADoor::GetDoorDirectionChangeSound, &LuaAdaptersPropertyOwnershipSafetyFaker::ADoorSetDoorDirectionChangeSound);
		luaType["DoorMoveEndSound"] = sol::property(&ADoor::GetDoorMoveEndSound, &LuaAdaptersPropertyOwnershipSafetyFaker::ADoorSetDoorMoveEndSound);

		luaType["GetDoorState"] = &ADoor::GetDoorState;
		luaType["OpenDoor"] = &ADoor::OpenDoor;
		luaType["CloseDoor"] = &ADoor::CloseDoor;
		luaType["StopDoor"] = &ADoor::StopDoor;
		luaType["SetClosedByDefault"] = &ADoor::SetClosedByDefault;

		luaType.new_enum("DoorState", EnumList(ADoor::DoorState) {
			{ "CLOSED", ADoor::DoorState::CLOSED },
			{ "OPENING", ADoor::DoorState::OPENING },
			{ "OPEN", ADoor::DoorState::OPEN },
			{ "CLOSING", ADoor::DoorState::CLOSING },
			{ "STOPPED", ADoor::DoorState::STOPPED }
		});
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, AHuman) {
		auto luaType = ConcreteTypeLuaClassDefinition(AHuman, Actor);

		luaType.set(sol::meta_function::construct, sol::constructors<
			AHuman()
		>());

		luaType["Head"] = sol::property(&AHuman::GetHead, &LuaAdaptersPropertyOwnershipSafetyFaker::AHumanSetHead);
		luaType["Jetpack"] = sol::property(&AHuman::GetJetpack, &LuaAdaptersPropertyOwnershipSafetyFaker::AHumanSetJetpack);
		luaType["FGArm"] = sol::property(&AHuman::GetFGArm, &LuaAdaptersPropertyOwnershipSafetyFaker::AHumanSetFGArm);
		luaType["BGArm"] = sol::property(&AHuman::GetBGArm, &LuaAdaptersPropertyOwnershipSafetyFaker::AHumanSetBGArm);
		luaType["FGLeg"] = sol::property(&AHuman::GetFGLeg, &LuaAdaptersPropertyOwnershipSafetyFaker::AHumanSetFGLeg);
		luaType["BGLeg"] = sol::property(&AHuman::GetBGLeg, &LuaAdaptersPropertyOwnershipSafetyFaker::AHumanSetBGLeg);
		luaType["FGFoot"] = sol::property(&AHuman::GetFGFoot, &LuaAdaptersPropertyOwnershipSafetyFaker::AHumanSetFGFoot);
		luaType["BGFoot"] = sol::property(&AHuman::GetBGFoot, &LuaAdaptersPropertyOwnershipSafetyFaker::AHumanSetBGFoot);
		luaType["StrideSound"] = sol::property(&AHuman::GetStrideSound, &LuaAdaptersPropertyOwnershipSafetyFaker::AHumanSetStrideSound);
		luaType["JetTimeTotal"] = sol::property(&AHuman::GetJetTimeTotal, &AHuman::SetJetTimeTotal);
		luaType["JetTimeLeft"] = sol::property(&AHuman::GetJetTimeLeft, &AHuman::SetJetTimeLeft);
		luaType["JetReplenishRate"] = sol::property(&AHuman::GetJetReplenishRate, &AHuman::SetJetReplenishRate);
		luaType["JetAngleRange"] = sol::property(&AHuman::GetJetAngleRange, &AHuman::SetJetAngleRange);
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
		luaType["EquipNamedDevice"] = (bool (AHuman::*)(const std::string &, bool))&AHuman::EquipNamedDevice;
		luaType["EquipNamedDevice"] = (bool (AHuman::*)(const std::string &, const std::string &, bool))&AHuman::EquipNamedDevice;
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

		luaType.new_enum("UpperBodyState", EnumList(AHuman::UpperBodyState) {
			{ "WEAPON_READY", AHuman::UpperBodyState::WEAPON_READY },
			{ "AIMING_SHARP", AHuman::UpperBodyState::AIMING_SHARP },
			{ "HOLSTERING_BACK", AHuman::UpperBodyState::HOLSTERING_BACK },
			{ "HOLSTERING_BELT", AHuman::UpperBodyState::HOLSTERING_BELT },
			{ "DEHOLSTERING_BACK", AHuman::UpperBodyState::DEHOLSTERING_BACK },
			{ "DEHOLSTERING_BELT", AHuman::UpperBodyState::DEHOLSTERING_BELT },
			{ "THROWING_PREP", AHuman::UpperBodyState::THROWING_PREP },
			{ "THROWING_RELEASE", AHuman::UpperBodyState::THROWING_RELEASE }
		});
		luaType.new_enum("MovementState", EnumList(AHuman::MovementState) {
			{ "NOMOVE", AHuman::MovementState::NOMOVE },
			{ "STAND", AHuman::MovementState::STAND },
			{ "WALK", AHuman::MovementState::WALK },
			{ "CROUCH", AHuman::MovementState::CROUCH },
			{ "CRAWL", AHuman::MovementState::CRAWL },
			{ "ARMCRAWL", AHuman::MovementState::ARMCRAWL },
			{ "CLIMB", AHuman::MovementState::CLIMB },
			{ "JUMP", AHuman::MovementState::JUMP },
			{ "DISLODGE", AHuman::MovementState::DISLODGE },
			{ "MOVEMENTSTATECOUNT", AHuman::MovementState::MOVEMENTSTATECOUNT }
		});
		luaType.new_enum("ProneState", EnumList(AHuman::ProneState) {
			{ "NOTPRONE", AHuman::ProneState::NOTPRONE },
			{ "GOPRONE", AHuman::ProneState::GOPRONE },
			{ "PRONE", AHuman::ProneState::PRONE },
			{ "PRONESTATECOUNT", AHuman::ProneState::PRONESTATECOUNT }
		});
		luaType.new_enum("Layer", EnumList(AHuman::Layer) {
			{ "FGROUND", AHuman::Layer::FGROUND },
			{ "BGROUND", AHuman::Layer::BGROUND }
		});
		luaType.new_enum("DeviceHandlingState", EnumList(AHuman::DeviceHandlingState) {
			{ "STILL", AHuman::DeviceHandlingState::STILL },
			{ "POINTING", AHuman::DeviceHandlingState::POINTING },
			{ "SCANNING", AHuman::DeviceHandlingState::SCANNING },
			{ "AIMING", AHuman::DeviceHandlingState::AIMING },
			{ "FIRING", AHuman::DeviceHandlingState::FIRING },
			{ "THROWING", AHuman::DeviceHandlingState::THROWING },
			{ "DIGGING", AHuman::DeviceHandlingState::DIGGING }
		});
		luaType.new_enum("SweepState", EnumList(AHuman::SweepState) {
			{ "NOSWEEP", AHuman::SweepState::NOSWEEP },
			{ "SWEEPINGUP", AHuman::SweepState::SWEEPINGUP },
			{ "SWEEPUPPAUSE", AHuman::SweepState::SWEEPUPPAUSE },
			{ "SWEEPINGDOWN", AHuman::SweepState::SWEEPINGDOWN },
			{ "SWEEPDOWNPAUSE", AHuman::SweepState::SWEEPDOWNPAUSE }
		});
		luaType.new_enum("DigState", EnumList(AHuman::DigState) {
			{ "NOTDIGGING", AHuman::DigState::NOTDIGGING },
			{ "PREDIG", AHuman::DigState::PREDIG },
			{ "STARTDIG", AHuman::DigState::STARTDIG },
			{ "TUNNELING", AHuman::DigState::TUNNELING },
			{ "FINISHINGDIG", AHuman::DigState::FINISHINGDIG },
			{ "PAUSEDIGGER", AHuman::DigState::PAUSEDIGGER }
		});
		luaType.new_enum("JumpState", EnumList(AHuman::JumpState) {
			{ "NOTJUMPING", AHuman::JumpState::NOTJUMPING },
			{ "FORWARDJUMP", AHuman::JumpState::FORWARDJUMP },
			{ "PREJUMP", AHuman::JumpState::PREUPJUMP },
			{ "UPJUMP", AHuman::JumpState::UPJUMP },
			{ "APEXJUMP", AHuman::JumpState::APEXJUMP },
			{ "LANDJUMP", AHuman::JumpState::LANDJUMP }
		});
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, Arm) {
		auto luaType = ConcreteTypeLuaClassDefinition(Arm, Attachable);

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

		luaType["AddHandTarget"] = (void (Arm::*)(const std::string &description, const Vector &handTargetPositionToAdd))&Arm::AddHandTarget;
		luaType["AddHandTarget"] = (void (Arm::*)(const std::string &description, const Vector &handTargetPositionToAdd, float delayAtTarget))&Arm::AddHandTarget;
		luaType["RemoveNextHandTarget"] = &Arm::RemoveNextHandTarget;
		luaType["ClearHandTargets"] = &Arm::ClearHandTargets;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, Leg) {
		auto luaType = ConcreteTypeLuaClassDefinition(Leg, Attachable);

		luaType["Foot"] = sol::property(&Leg::GetFoot, &LuaAdaptersPropertyOwnershipSafetyFaker::LegSetFoot);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, LimbPath) {
		auto luaType = SimpleTypeLuaClassDefinition(LimbPath);

		luaType["StartOffset"] = sol::property(&LimbPath::GetStartOffset, &LimbPath::SetStartOffset);
		luaType["SegmentCount"] = sol::property(&LimbPath::GetSegCount);

		luaType["GetSegment"] = &LimbPath::GetSegment;
	}
}