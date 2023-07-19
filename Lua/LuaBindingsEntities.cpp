// Make sure that binding definition files are always set to NOT use pre-compiled headers and conformance mode (/permissive) otherwise everything will be on fire!

#include "LuaBindingRegisterDefinitions.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, Entity) {
		auto luaType = SimpleTypeLuaClassDefinition(Entity);

		luaType[sol::meta_function::to_string] = sol::resolve<std::ostream & (std::ostream&, const Entity&)>(RTE::operator<<);

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
		luaType["AddInventoryItem"] = &Actor::AddInventoryItem, luabind::adopt(_2);
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

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, AEmitter) {
		auto luaType = ConcreteTypeLuaClassDefinition(AEmitter, Attachable);

		luaType["EmissionSound"] = sol::property(&AEmitter::GetEmissionSound, &LuaAdaptersPropertyOwnershipSafetyFaker::AEmitterSetEmissionSound);
		luaType["BurstSound"] = sol::property(&AEmitter::GetBurstSound, &LuaAdaptersPropertyOwnershipSafetyFaker::AEmitterSetBurstSound);
		luaType["EndSound"] = sol::property(&AEmitter::GetEndSound, &LuaAdaptersPropertyOwnershipSafetyFaker::AEmitterSetEndSound);
		luaType["BurstScale"] = sol::property(&AEmitter::GetBurstScale, &AEmitter::SetBurstScale);
		luaType["EmitAngle"] = sol::property(&AEmitter::GetEmitAngle, &AEmitter::SetEmitAngle);
		luaType["GetThrottle"] = sol::property(&AEmitter::GetThrottle, &AEmitter::SetThrottle);
		luaType["Throttle"] = sol::property(&AEmitter::GetThrottle, &AEmitter::SetThrottle);
		luaType["ThrottleFactor"] = sol::property(&AEmitter::GetThrottleFactor);
		luaType["NegativeThrottleMultiplier"] = sol::property(&AEmitter::GetNegativeThrottleMultiplier, &AEmitter::SetNegativeThrottleMultiplier);
		luaType["PositiveThrottleMultiplier"] = sol::property(&AEmitter::GetPositiveThrottleMultiplier, &AEmitter::SetPositiveThrottleMultiplier);
		luaType["BurstSpacing"] = sol::property(&AEmitter::GetBurstSpacing, &AEmitter::SetBurstSpacing);
		luaType["BurstDamage"] = sol::property(&AEmitter::GetBurstDamage, &AEmitter::SetBurstDamage);
		luaType["EmitterDamageMultiplier"] = sol::property(&AEmitter::GetEmitterDamageMultiplier, &AEmitter::SetEmitterDamageMultiplier);
		luaType["EmitCount"] = sol::property(&AEmitter::GetEmitCount);
		luaType["EmitCountLimit"] = sol::property(&AEmitter::GetEmitCountLimit, &AEmitter::SetEmitCountLimit);
		luaType["EmitDamage"] = sol::property(&AEmitter::GetEmitDamage, &AEmitter::SetEmitDamage);
		luaType["EmitOffset"] = sol::property(&AEmitter::GetEmitOffset, &AEmitter::SetEmitOffset);
		luaType["Flash"] = sol::property(&AEmitter::GetFlash, &LuaAdaptersPropertyOwnershipSafetyFaker::AEmitterSetFlash);
		luaType["FlashScale"] = sol::property(&AEmitter::GetFlashScale, &AEmitter::SetFlashScale);
		luaType["TotalParticlesPerMinute"] = sol::property(&AEmitter::GetTotalParticlesPerMinute);
		luaType["TotalBurstSize"] = sol::property(&AEmitter::GetTotalBurstSize);

		luaType["Emissions"] = &AEmitter::m_EmissionList;

		luaType["IsEmitting"] = &AEmitter::IsEmitting;
		luaType["EnableEmission"] = &AEmitter::EnableEmission;
		luaType["GetEmitVector"] = &AEmitter::GetEmitVector;
		luaType["GetRecoilVector"] = &AEmitter::GetRecoilVector;
		luaType["EstimateImpulse"] = &AEmitter::EstimateImpulse;
		luaType["TriggerBurst"] = &AEmitter::TriggerBurst;
		luaType["IsSetToBurst"] = &AEmitter::IsSetToBurst;
		luaType["CanTriggerBurst"] = &AEmitter::CanTriggerBurst;
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

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, Attachable) {
		auto luaType = ConcreteTypeLuaClassDefinition(Attachable, MOSRotating);

		luaType["ParentOffset"] = sol::property(&Attachable::GetParentOffset, &Attachable::SetParentOffset);
		luaType["JointStrength"] = sol::property(&Attachable::GetJointStrength, &Attachable::SetJointStrength);
		luaType["JointStiffness"] = sol::property(&Attachable::GetJointStiffness, &Attachable::SetJointStiffness);
		luaType["JointOffset"] = sol::property(&Attachable::GetJointOffset, &Attachable::SetJointOffset);
		luaType["JointPos"] = sol::property(&Attachable::GetJointPos);
		luaType["DeleteWhenRemovedFromParent"] = sol::property(&Attachable::GetDeleteWhenRemovedFromParent, &Attachable::SetDeleteWhenRemovedFromParent);
		luaType["GibWhenRemovedFromParent"] = sol::property(&Attachable::GetGibWhenRemovedFromParent, &Attachable::SetGibWhenRemovedFromParent);
		luaType["ApplyTransferredForcesAtOffset"] = sol::property(&Attachable::GetApplyTransferredForcesAtOffset, &Attachable::SetApplyTransferredForcesAtOffset);
		luaType["BreakWound"] = sol::property(&Attachable::GetBreakWound, &LuaAdaptersPropertyOwnershipSafetyFaker::AttachableSetBreakWound);
		luaType["ParentBreakWound"] = sol::property(&Attachable::GetParentBreakWound, &LuaAdaptersPropertyOwnershipSafetyFaker::AttachableSetParentBreakWound);
		luaType["InheritsHFlipped"] = sol::property(&Attachable::InheritsHFlipped, &Attachable::SetInheritsHFlipped);
		luaType["InheritsRotAngle"] = sol::property(&Attachable::InheritsRotAngle, &Attachable::SetInheritsRotAngle);
		luaType["InheritedRotAngleOffset"] = sol::property(&Attachable::GetInheritedRotAngleOffset, &Attachable::SetInheritedRotAngleOffset);
		luaType["AtomSubgroupID"] = sol::property(&Attachable::GetAtomSubgroupID);
		luaType["CollidesWithTerrainWhileAttached"] = sol::property(&Attachable::GetCollidesWithTerrainWhileAttached, &Attachable::SetCollidesWithTerrainWhileAttached);
		luaType["IgnoresParticlesWhileAttached"] = sol::property(&Attachable::GetIgnoresParticlesWhileAttached, &Attachable::SetIgnoresParticlesWhileAttached);
		luaType["CanCollideWithTerrain"] = sol::property(&Attachable::CanCollideWithTerrain);
		luaType["DrawnAfterParent"] = sol::property(&Attachable::IsDrawnAfterParent, &Attachable::SetDrawnAfterParent);
		luaType["InheritsFrame"] = sol::property(&Attachable::InheritsFrame, &Attachable::SetInheritsFrame);

		luaType["IsAttached"] = &Attachable::IsAttached;
		luaType["IsAttachedTo"] = &Attachable::IsAttachedTo;

		luaType["RemoveFromParent"] = &LuaAdaptersAttachable::RemoveFromParent1, luabind::adopt(luabind::return_value);
		luaType["RemoveFromParent"] = &LuaAdaptersAttachable::RemoveFromParent2, luabind::adopt(luabind::return_value);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, Deployment) {
		auto luaType = AbstractTypeLuaClassDefinition(Deployment, SceneObject);

		luaType["ID"] = sol::property(&Deployment::GetID);
		luaType["HFlipped"] = sol::property(&Deployment::IsHFlipped);
		luaType["SpawnRadius"] = sol::property(&Deployment::GetSpawnRadius);

		luaType["GetLoadoutName"] = &Deployment::GetLoadoutName;
		luaType["CreateDeployedActor"] = (Actor * (Deployment::*)())&Deployment::CreateDeployedActor, luabind::adopt(luabind::result);
		luaType["CreateDeployedObject"] = (SceneObject * (Deployment::*)())&Deployment::CreateDeployedObject, luabind::adopt(luabind::result);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, Emission) {
		auto luaType = AbstractTypeLuaClassDefinition(Emission, Entity);

		luaType["ParticlesPerMinute"] = sol::property(&Emission::GetRate, &Emission::SetRate);
		luaType["MinVelocity"] = sol::property(&Emission::GetMinVelocity, &Emission::SetMinVelocity);
		luaType["MaxVelocity"] = sol::property(&Emission::GetMaxVelocity, &Emission::SetMaxVelocity);
		luaType["PushesEmitter"] = sol::property(&Emission::PushesEmitter, &Emission::SetPushesEmitter);
		luaType["LifeVariation"] = sol::property(&Emission::GetLifeVariation, &Emission::SetLifeVariation);
		luaType["BurstSize"] = sol::property(&Emission::GetBurstSize, &Emission::SetBurstSize);
		luaType["Spread"] = sol::property(&Emission::GetSpread, &Emission::SetSpread);
		luaType["Offset"] = sol::property(&Emission::GetOffset, &Emission::SetOffset);

		luaType["ResetEmissionTimers"] = &Emission::ResetEmissionTimers;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, Gib) {
		auto luaType = SimpleTypeLuaClassDefinition(Gib);

		luaType["ParticlePreset"] = sol::property(&Gib::GetParticlePreset, &Gib::SetParticlePreset);
		luaType["MinVelocity"] = sol::property(&Gib::GetMinVelocity, &Gib::SetMinVelocity);
		luaType["MaxVelocity"] = sol::property(&Gib::GetMaxVelocity, &Gib::SetMaxVelocity);
		luaType["SpreadMode"] = sol::property(&Gib::GetSpreadMode, &Gib::SetSpreadMode);

		luaType["Offset"] = &Gib::m_Offset;
		luaType["Count"] = &Gib::m_Count;
		luaType["Spread"] = &Gib::m_Spread;
		luaType["LifeVariation"] = &Gib::m_LifeVariation;
		luaType["InheritsVel"] = &Gib::m_InheritsVel;
		luaType["IgnoresTeamHits"] = &Gib::m_IgnoresTeamHits;

		luaType.new_enum("SpreadMode", EnumList(Gib::SpreadMode) {
			{ "SpreadRandom", Gib::SpreadMode::SpreadRandom },
			{ "SpreadEven", Gib::SpreadMode::SpreadEven },
			{ "SpreadSpiral", Gib::SpreadMode::SpreadSpiral }
		});
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, GlobalScript) {
		auto luaType = AbstractTypeLuaClassDefinition(GlobalScript, Entity);

		luaType["Deactivate"] = &LuaAdaptersGlobalScript::Deactivate;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, HDFirearm) {
		auto luaType = ConcreteTypeLuaClassDefinition(HDFirearm, HeldDevice);

		luaType["RateOfFire"] = sol::property(&HDFirearm::GetRateOfFire, &HDFirearm::SetRateOfFire);
		luaType["MSPerRound"] = sol::property(&HDFirearm::GetMSPerRound);
		luaType["FullAuto"] = sol::property(&HDFirearm::IsFullAuto, &HDFirearm::SetFullAuto);
		luaType["Reloadable"] = sol::property(&HDFirearm::IsReloadable, &HDFirearm::SetReloadable);
		luaType["DualReloadable"] = sol::property(&HDFirearm::IsDualReloadable, &HDFirearm::SetDualReloadable);
		luaType["OneHandedReloadTimeMultiplier"] = sol::property(&HDFirearm::GetOneHandedReloadTimeMultiplier, &HDFirearm::SetOneHandedReloadTimeMultiplier);
		luaType["ReloadAngle"] = sol::property(&HDFirearm::GetReloadAngle, &HDFirearm::SetReloadAngle);
		luaType["OneHandedReloadAngle"] = sol::property(&HDFirearm::GetOneHandedReloadAngle, &HDFirearm::SetOneHandedReloadAngle);
		luaType["CurrentReloadAngle"] = sol::property(&HDFirearm::GetCurrentReloadAngle);
		luaType["RoundInMagCount"] = sol::property(&HDFirearm::GetRoundInMagCount);
		luaType["RoundInMagCapacity"] = sol::property(&HDFirearm::GetRoundInMagCapacity);
		luaType["Magazine"] = sol::property(&HDFirearm::GetMagazine, &LuaAdaptersPropertyOwnershipSafetyFaker::HDFirearmSetMagazine);
		luaType["Flash"] = sol::property(&HDFirearm::GetFlash, &LuaAdaptersPropertyOwnershipSafetyFaker::HDFirearmSetFlash);
		luaType["PreFireSound"] = sol::property(&HDFirearm::GetPreFireSound, &LuaAdaptersPropertyOwnershipSafetyFaker::HDFirearmSetPreFireSound);
		luaType["FireSound"] = sol::property(&HDFirearm::GetFireSound, &LuaAdaptersPropertyOwnershipSafetyFaker::HDFirearmSetFireSound);
		luaType["FireEchoSound"] = sol::property(&HDFirearm::GetFireEchoSound, &LuaAdaptersPropertyOwnershipSafetyFaker::HDFirearmSetFireEchoSound);
		luaType["ActiveSound"] = sol::property(&HDFirearm::GetActiveSound, &LuaAdaptersPropertyOwnershipSafetyFaker::HDFirearmSetActiveSound);
		luaType["DeactivationSound"] = sol::property(&HDFirearm::GetDeactivationSound, &LuaAdaptersPropertyOwnershipSafetyFaker::HDFirearmSetDeactivationSound);
		luaType["EmptySound"] = sol::property(&HDFirearm::GetEmptySound, &LuaAdaptersPropertyOwnershipSafetyFaker::HDFirearmSetEmptySound);
		luaType["ReloadStartSound"] = sol::property(&HDFirearm::GetReloadStartSound, &LuaAdaptersPropertyOwnershipSafetyFaker::HDFirearmSetReloadStartSound);
		luaType["ReloadEndSound"] = sol::property(&HDFirearm::GetReloadEndSound, &LuaAdaptersPropertyOwnershipSafetyFaker::HDFirearmSetReloadEndSound);
		luaType["ActivationDelay"] = sol::property(&HDFirearm::GetActivationDelay, &HDFirearm::SetActivationDelay);
		luaType["DeactivationDelay"] = sol::property(&HDFirearm::GetDeactivationDelay, &HDFirearm::SetDeactivationDelay);
		luaType["BaseReloadTime"] = sol::property(&HDFirearm::GetBaseReloadTime, &HDFirearm::SetBaseReloadTime);
		luaType["ReloadTime"] = sol::property(&HDFirearm::GetReloadTime);
		luaType["ReloadProgress"] = sol::property(&HDFirearm::GetReloadProgress);
		luaType["ShakeRange"] = sol::property(&HDFirearm::GetShakeRange, &HDFirearm::SetShakeRange);
		luaType["SharpShakeRange"] = sol::property(&HDFirearm::GetSharpShakeRange, &HDFirearm::SetSharpShakeRange);
		luaType["NoSupportFactor"] = sol::property(&HDFirearm::GetNoSupportFactor, &HDFirearm::SetNoSupportFactor);
		luaType["ParticleSpreadRange"] = sol::property(&HDFirearm::GetParticleSpreadRange, &HDFirearm::SetParticleSpreadRange);
		luaType["ShellVelVariation"] = sol::property(&HDFirearm::GetShellVelVariation, &HDFirearm::SetShellVelVariation);
		luaType["FiredOnce"] = sol::property(&HDFirearm::FiredOnce);
		luaType["FiredFrame"] = sol::property(&HDFirearm::FiredFrame);
		luaType["CanFire"] = sol::property(&HDFirearm::CanFire);
		luaType["RoundsFired"] = sol::property(&HDFirearm::RoundsFired);
		luaType["IsAnimatedManually"] = sol::property(&HDFirearm::IsAnimatedManually, &HDFirearm::SetAnimatedManually);
		luaType["RecoilTransmission"] = sol::property(&HDFirearm::GetJointStiffness, &HDFirearm::SetJointStiffness);

		luaType["GetAIFireVel"] = &HDFirearm::GetAIFireVel;
		luaType["GetAIBulletLifeTime"] = &HDFirearm::GetAIBulletLifeTime;
		luaType["GetBulletAccScalar"] = &HDFirearm::GetBulletAccScalar;
		luaType["GetAIBlastRadius"] = &HDFirearm::GetAIBlastRadius;
		luaType["GetAIPenetration"] = &HDFirearm::GetAIPenetration;
		luaType["CompareTrajectories"] = &HDFirearm::CompareTrajectories;
		luaType["GetNextMagazineName"] = &HDFirearm::GetNextMagazineName;
		luaType["SetNextMagazineName"] = &HDFirearm::SetNextMagazineName;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, HeldDevice) {
		auto luaType = ConcreteTypeLuaClassDefinition(HeldDevice, Attachable);

		luaType["SupportPos"] = sol::property(&HeldDevice::GetSupportPos);
		luaType["MagazinePos"] = sol::property(&HeldDevice::GetMagazinePos);
		luaType["MuzzlePos"] = sol::property(&HeldDevice::GetMuzzlePos);
		luaType["MuzzleOffset"] = sol::property(&HeldDevice::GetMuzzleOffset, &HeldDevice::SetMuzzleOffset);
		luaType["StanceOffset"] = sol::property(&HeldDevice::GetStanceOffset, &HeldDevice::SetStanceOffset);
		luaType["SharpStanceOffset"] = sol::property(&HeldDevice::GetSharpStanceOffset, &HeldDevice::SetSharpStanceOffset);
		luaType["SharpLength"] = sol::property(&HeldDevice::GetSharpLength, &HeldDevice::SetSharpLength);
		luaType["SharpLength"] = sol::property(&HeldDevice::GetSharpLength, &HeldDevice::SetSharpLength);
		luaType["Supportable"] = sol::property(&HeldDevice::IsSupportable, &HeldDevice::SetSupportable);
		luaType["SupportOffset"] = sol::property(&HeldDevice::GetSupportOffset, &HeldDevice::SetSupportOffset);
		luaType["HasPickupLimitations"] = sol::property(&HeldDevice::HasPickupLimitations);
		luaType["UnPickupable"] = sol::property(&HeldDevice::IsUnPickupable, &HeldDevice::SetUnPickupable);
		luaType["GripStrengthMultiplier"] = sol::property(&HeldDevice::GetGripStrengthMultiplier, &HeldDevice::SetGripStrengthMultiplier);
		luaType["Supported"] = sol::property(&HeldDevice::GetSupported, &HeldDevice::SetSupported);

		luaType["IsWeapon"] = &HeldDevice::IsWeapon;
		luaType["IsTool"] = &HeldDevice::IsTool;
		luaType["IsShield"] = &HeldDevice::IsShield;
		luaType["IsDualWieldable"] = &HeldDevice::IsDualWieldable;
		luaType["SetDualWieldable"] = &HeldDevice::SetDualWieldable;
		luaType["IsOneHanded"] = &HeldDevice::IsOneHanded;
		luaType["SetOneHanded"] = &HeldDevice::SetOneHanded;
		luaType["Activate"] = &HeldDevice::Activate;
		luaType["Deactivate"] = &HeldDevice::Deactivate;
		luaType["Reload"] = &HeldDevice::Reload;
		luaType["IsActivated"] = &HeldDevice::IsActivated;
		luaType["IsReloading"] = &HeldDevice::IsReloading;
		luaType["DoneReloading"] = &HeldDevice::DoneReloading;
		luaType["NeedsReloading"] = &HeldDevice::NeedsReloading;
		luaType["IsFull"] = &HeldDevice::IsFull;
		luaType["IsEmpty"] = &HeldDevice::IsEmpty;
		luaType["IsPickupableBy"] = &HeldDevice::IsPickupableBy;
		luaType["AddPickupableByPresetName"] = &HeldDevice::AddPickupableByPresetName;
		luaType["RemovePickupableByPresetName"] = &HeldDevice::RemovePickupableByPresetName;
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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, Magazine) {
		auto luaType = ConcreteTypeLuaClassDefinition(Magazine, Attachable);

		luaType["NextRound"] = sol::property(&Magazine::GetNextRound);
		luaType["RoundCount"] = sol::property(&Magazine::GetRoundCount, &Magazine::SetRoundCount);
		luaType["IsEmpty"] = sol::property(&Magazine::IsEmpty);
		luaType["IsFull"] = sol::property(&Magazine::IsFull);
		luaType["IsOverHalfFull"] = sol::property(&Magazine::IsOverHalfFull);
		luaType["Capacity"] = sol::property(&Magazine::GetCapacity);
		luaType["Discardable"] = sol::property(&Magazine::IsDiscardable);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, Material) {
		auto luaType = SimpleTypeLuaClassDefinition(Material);

		luaType["ID"] = sol::property(&Material::GetIndex);
		luaType["Restitution"] = sol::property(&Material::GetRestitution);
		luaType["Bounce"] = sol::property(&Material::GetRestitution);
		luaType["Friction"] = sol::property(&Material::GetFriction);
		luaType["Stickiness"] = sol::property(&Material::GetStickiness);
		luaType["Strength"] = sol::property(&Material::GetIntegrity);
		luaType["StructuralIntegrity"] = sol::property(&Material::GetIntegrity);
		luaType["DensityKGPerVolumeL"] = sol::property(&Material::GetVolumeDensity);
		luaType["DensityKGPerPixel"] = sol::property(&Material::GetPixelDensity);
		luaType["SettleMaterial"] = sol::property(&Material::GetSettleMaterial);
		luaType["SpawnMaterial"] = sol::property(&Material::GetSpawnMaterial);
		luaType["TransformsInto"] = sol::property(&Material::GetSpawnMaterial);
		luaType["IsScrap"] = sol::property(&Material::IsScrap);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, MetaPlayer) {
		auto luaType = SimpleTypeLuaClassDefinition(MetaPlayer);

		luaType.set(sol::meta_function::construct, sol::constructors<
			MetaPlayer()
		>());

		luaType["NativeTechModule"] = sol::property(&MetaPlayer::GetNativeTechModule);
		luaType["ForeignCostMultiplier"] = sol::property(&MetaPlayer::GetForeignCostMultiplier);
		luaType["NativeCostMultiplier"] = sol::property(&MetaPlayer::GetNativeCostMultiplier);
		luaType["InGamePlayer"] = sol::property(&MetaPlayer::GetInGamePlayer);
		luaType["BrainPoolCount"] = sol::property(&MetaPlayer::GetBrainPoolCount, &MetaPlayer::SetBrainPoolCount);

		luaType["ChangeBrainPoolCount"] = &MetaPlayer::ChangeBrainPoolCount;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, MOPixel) {
		auto luaType = ConcreteTypeLuaClassDefinition(MOPixel, MovableObject);

		luaType["TrailLength"] = sol::property(&MOPixel::GetTrailLength, &MOPixel::SetTrailLength);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, MOSParticle) {
		auto luaType = ConcreteTypeLuaClassDefinition(MOSParticle, MOSprite);;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, MOSprite) {
		auto luaType = AbstractTypeLuaClassDefinition(MOSprite, MovableObject);

		luaType["Diameter"] = sol::property(&MOSprite::GetDiameter);
		luaType["BoundingBox"] = sol::property(&MOSprite::GetBoundingBox);
		luaType["FrameCount"] = sol::property(&MOSprite::GetFrameCount);
		luaType["SpriteOffset"] = sol::property(&MOSprite::GetSpriteOffset, &MOSprite::SetSpriteOffset);
		luaType["HFlipped"] = sol::property(&MOSprite::IsHFlipped, &MOSprite::SetHFlipped);
		luaType["FlipFactor"] = sol::property(&MOSprite::GetFlipFactor);
		luaType["RotAngle"] = sol::property(&MOSprite::GetRotAngle, &MOSprite::SetRotAngle);
		luaType["PrevRotAngle"] = sol::property(&MOSprite::GetPrevRotAngle);
		luaType["AngularVel"] = sol::property(&MOSprite::GetAngularVel, &MOSprite::SetAngularVel);
		luaType["Frame"] = sol::property(&MOSprite::GetFrame, &MOSprite::SetFrame);
		luaType["SpriteAnimMode"] = sol::property(&MOSprite::GetSpriteAnimMode, &MOSprite::SetSpriteAnimMode);
		luaType["SpriteAnimDuration"] = sol::property(&MOSprite::GetSpriteAnimDuration, &MOSprite::SetSpriteAnimDuration);

		luaType["SetNextFrame"] = &MOSprite::SetNextFrame;
		luaType["IsTooFast"] = &MOSprite::IsTooFast;
		luaType["IsOnScenePoint"] = &MOSprite::IsOnScenePoint;
		luaType["RotateOffset"] = &MOSprite::RotateOffset;
		luaType["UnRotateOffset"] = &MOSprite::UnRotateOffset;
		luaType["FacingAngle"] = &MOSprite::FacingAngle;
		luaType["GetSpriteWidth"] = &MOSprite::GetSpriteWidth;
		luaType["GetSpriteHeight"] = &MOSprite::GetSpriteHeight;
		luaType["GetIconWidth"] = &MOSprite::GetIconWidth;
		luaType["GetIconHeight"] = &MOSprite::GetIconHeight;
		luaType["SetEntryWound"] = &MOSprite::SetEntryWound;
		luaType["SetExitWound"] = &MOSprite::SetExitWound;
		luaType["GetEntryWoundPresetName"] = &MOSprite::GetEntryWoundPresetName;
		luaType["GetExitWoundPresetName"] = &MOSprite::GetExitWoundPresetName;

		luaType.new_enum("SpriteAnimMode", EnumList(SpriteAnimMode) {
			{ "NOANIM", SpriteAnimMode::NOANIM },
			{ "ALWAYSLOOP", SpriteAnimMode::ALWAYSLOOP },
			{ "ALWAYSRANDOM", SpriteAnimMode::ALWAYSRANDOM },
			{ "ALWAYSPINGPONG", SpriteAnimMode::ALWAYSPINGPONG },
			{ "LOOPWHENACTIVE", SpriteAnimMode::LOOPWHENACTIVE },
			{ "LOOPWHENOPENCLOSE", SpriteAnimMode::LOOPWHENOPENCLOSE },
			{ "PINGPONGOPENCLOSE", SpriteAnimMode::PINGPONGOPENCLOSE },
			{ "OVERLIFETIME", SpriteAnimMode::OVERLIFETIME },
			{ "ONCOLLIDE", SpriteAnimMode::ONCOLLIDE }
		});
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, MOSRotating) {
		auto luaType = ConcreteTypeLuaClassDefinition(MOSRotating, MOSprite);

		//luaType["Material"] = sol::property(&MOSRotating::GetMaterial);
		luaType["IndividualRadius"] = sol::property(&MOSRotating::GetIndividualRadius);
		luaType["IndividualDiameter"] = sol::property(&MOSRotating::GetIndividualDiameter);
		luaType["IndividualMass"] = sol::property(&MOSRotating::GetIndividualMass);
		luaType["RecoilForce"] = sol::property(&MOSRotating::GetRecoilForce);
		luaType["RecoilOffset"] = sol::property(&MOSRotating::GetRecoilOffset);
		luaType["TravelImpulse"] = sol::property(&MOSRotating::GetTravelImpulse, &MOSRotating::SetTravelImpulse);
		luaType["GibWoundLimit"] = sol::property((int (MOSRotating:: *)() const) &MOSRotating::GetGibWoundLimit, &MOSRotating::SetGibWoundLimit);
		luaType["GibSound"] = sol::property(&MOSRotating::GetGibSound, &LuaAdaptersPropertyOwnershipSafetyFaker::MOSRotatingSetGibSound);
		luaType["GibImpulseLimit"] = sol::property(&MOSRotating::GetGibImpulseLimit, &MOSRotating::SetGibImpulseLimit);
		luaType["WoundCountAffectsImpulseLimitRatio"] = sol::property(&MOSRotating::GetWoundCountAffectsImpulseLimitRatio);
		luaType["GibAtEndOfLifetime"] = sol::property(&MOSRotating::GetGibAtEndOfLifetime, &MOSRotating::SetGibAtEndOfLifetime);
		luaType["DamageMultiplier"] = sol::property(&MOSRotating::GetDamageMultiplier, &MOSRotating::SetDamageMultiplier);
		luaType["WoundCount"] = sol::property((int (MOSRotating:: *)() const) &MOSRotating::GetWoundCount);
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
		luaType["GetGibWoundLimit"] = (int (MOSRotating:: *)() const) &MOSRotating::GetGibWoundLimit;
		luaType["GetGibWoundLimit"] = (int (MOSRotating:: *)(bool positiveDamage, bool negativeDamage, bool noDamage) const) &MOSRotating::GetGibWoundLimit;
		luaType["GetWoundCount"] = (int (MOSRotating:: *)() const) &MOSRotating::GetWoundCount;
		luaType["GetWoundCount"] = (int (MOSRotating:: *)(bool positiveDamage, bool negativeDamage, bool noDamage) const) &MOSRotating::GetWoundCount;
		luaType["GetWounds"] = &LuaAdaptersMOSRotating::GetWounds1;
		luaType["GetWounds"] = &LuaAdaptersMOSRotating::GetWounds2;
		luaType["AddWound"] = &MOSRotating::AddWound, luabind::adopt(_2);
		luaType["RemoveWounds"] = (float (MOSRotating:: *)(int numberOfWoundsToRemove)) &MOSRotating::RemoveWounds;
		luaType["RemoveWounds"] = (float (MOSRotating:: *)(int numberOfWoundsToRemove, bool positiveDamage, bool negativeDamage, bool noDamage)) &MOSRotating::RemoveWounds;
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
		luaType["AddAttachable"] = (void (MOSRotating::*)(Attachable *attachableToAdd))&MOSRotating::AddAttachable, luabind::adopt(_2);
		luaType["AddAttachable"] = (void (MOSRotating::*)(Attachable *attachableToAdd, const Vector &parentOffset))&MOSRotating::AddAttachable, luabind::adopt(_2);
		luaType["RemoveAttachable"] = (Attachable *(MOSRotating:: *)(long uniqueIDOfAttachableToRemove)) &MOSRotating::RemoveAttachable, luabind::adopt(luabind::return_value);
		luaType["RemoveAttachable"] = (Attachable *(MOSRotating:: *)(long uniqueIDOfAttachableToRemove, bool addToMovableMan, bool addBreakWounds)) &MOSRotating::RemoveAttachable, luabind::adopt(luabind::return_value);
		luaType["RemoveAttachable"] = (Attachable *(MOSRotating:: *)(Attachable *attachableToRemove))&MOSRotating::RemoveAttachable, luabind::adopt(luabind::return_value);
		luaType["RemoveAttachable"] = (Attachable *(MOSRotating:: *)(Attachable *attachableToRemove, bool addToMovableMan, bool addBreakWounds)) &MOSRotating::RemoveAttachable;
		luaType["AddEmitter"] = (void (MOSRotating::*)(Attachable *attachableToAdd))&MOSRotating::AddAttachable, luabind::adopt(_2);
		luaType["AddEmitter"] = (void (MOSRotating::*)(Attachable *attachableToAdd, const Vector &parentOffset))&MOSRotating::AddAttachable, luabind::adopt(_2);
		luaType["RemoveEmitter"] = (Attachable *(MOSRotating:: *)(long uniqueIDOfAttachableToRemove)) &MOSRotating::RemoveAttachable, luabind::adopt(luabind::return_value);
		luaType["RemoveEmitter"] = (Attachable *(MOSRotating:: *)(long uniqueIDOfAttachableToRemove, bool addToMovableMan, bool addBreakWounds)) &MOSRotating::RemoveAttachable, luabind::adopt(luabind::return_value);
		luaType["RemoveEmitter"] = (Attachable *(MOSRotating:: *)(Attachable *attachableToRemove)) &MOSRotating::RemoveAttachable, luabind::adopt(luabind::return_value);
		luaType["RemoveEmitter"] = (Attachable *(MOSRotating:: *)(Attachable *attachableToRemove, bool addToMovableMan, bool addBreakWounds)) &MOSRotating::RemoveAttachable, luabind::adopt(luabind::return_value);

		luaType["GibThis"] = &LuaAdaptersMOSRotating::GibThis;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, MovableObject) {
		auto luaType = AbstractTypeLuaClassDefinition(MovableObject, SceneObject);

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

		luaType["GetParent"] = (MOSRotating * (MovableObject::*)())&MovableObject::GetParent;
		luaType["GetParent"] = (const MOSRotating * (MovableObject::*)() const)&MovableObject::GetParent;
		luaType["GetRootParent"] = (MovableObject * (MovableObject::*)())&MovableObject::GetRootParent;
		luaType["GetRootParent"] = (const MovableObject * (MovableObject::*)() const)&MovableObject::GetRootParent;
		luaType["ReloadScripts"] = &MovableObject::ReloadScripts;
		luaType["HasScript"] = &LuaAdaptersMovableObject::HasScript;
		luaType["AddScript"] = &LuaAdaptersMovableObject::AddScript;
		luaType["ScriptEnabled"] = &MovableObject::ScriptEnabled;
		luaType["EnableScript"] = &LuaAdaptersMovableObject::EnableScript;
		luaType["DisableScript"] = &LuaAdaptersMovableObject::DisableScript;
		luaType["EnableOrDisableAllScripts"] = &MovableObject::EnableOrDisableAllScripts;
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
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, PEmitter) {
		auto luaType = ConcreteTypeLuaClassDefinition(PEmitter, MOSParticle);

		luaType["BurstScale"] = sol::property(&PEmitter::GetBurstScale, &PEmitter::SetBurstScale);
		luaType["EmitAngle"] = sol::property(&PEmitter::GetEmitAngle, &PEmitter::SetEmitAngle);
		luaType["GetThrottle"] = sol::property(&PEmitter::GetThrottle, &PEmitter::SetThrottle);
		luaType["Throttle"] = sol::property(&PEmitter::GetThrottle, &PEmitter::SetThrottle);
		luaType["ThrottleFactor"] = sol::property(&PEmitter::GetThrottleFactor);
		luaType["BurstSpacing"] = sol::property(&PEmitter::GetBurstSpacing, &PEmitter::SetBurstSpacing);
		luaType["EmitCountLimit"] = sol::property(&PEmitter::GetEmitCountLimit, &PEmitter::SetEmitCountLimit);
		luaType["FlashScale"] = sol::property(&PEmitter::GetFlashScale, &PEmitter::SetFlashScale);

		luaType["Emissions"] = &PEmitter::m_EmissionList;

		luaType["IsEmitting"] = &PEmitter::IsEmitting;
		luaType["EnableEmission"] = &PEmitter::EnableEmission;
		luaType["GetEmitVector"] = &PEmitter::GetEmitVector;
		luaType["GetRecoilVector"] = &PEmitter::GetRecoilVector;
		luaType["EstimateImpulse"] = &PEmitter::EstimateImpulse;
		luaType["TriggerBurst"] = &PEmitter::TriggerBurst;
		luaType["IsSetToBurst"] = &PEmitter::IsSetToBurst;
		luaType["CanTriggerBurst"] = &PEmitter::CanTriggerBurst;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, PieSlice) {
		auto luaType = ConcreteTypeLuaClassDefinition(PieSlice, Entity);

		luaType["Type"] = sol::property(&PieSlice::GetType, &PieSlice::SetType);
		luaType["Direction"] = sol::property(&PieSlice::GetDirection, &PieSlice::SetDirection);
		luaType["CanBeMiddleSlice"] = sol::property(&PieSlice::GetCanBeMiddleSlice, &PieSlice::SetCanBeMiddleSlice);
		luaType["OriginalSource"] = sol::property(&PieSlice::GetOriginalSource);
		luaType["Enabled"] = sol::property(&PieSlice::IsEnabled, &PieSlice::SetEnabled);

		luaType["ScriptPath"] = sol::property(&PieSlice::GetScriptPath, &PieSlice::SetScriptPath);
		luaType["FunctionName"] = sol::property(&PieSlice::GetFunctionName, &PieSlice::SetFunctionName);
		luaType["SubPieMenu"] = sol::property(&PieSlice::GetSubPieMenu, &PieSlice::SetSubPieMenu);

		luaType["DrawFlippedToMatchAbsoluteAngle"] = sol::property(&PieSlice::GetDrawFlippedToMatchAbsoluteAngle, &PieSlice::SetDrawFlippedToMatchAbsoluteAngle);

		luaType["ReloadScripts"] = &PieSlice::ReloadScripts;

		luaType.new_enum("SliceType", EnumList(PieSlice::SliceType) {
			{ "NoType", PieSlice::SliceType::NoType },
			{ "Pickup", PieSlice::SliceType::Pickup },
			{ "Drop", PieSlice::SliceType::Drop },
			{ "NextItem", PieSlice::SliceType::NextItem },
			{ "PreviousItem", PieSlice::SliceType::PreviousItem },
			{ "Reload", PieSlice::SliceType::Reload },
			{ "BuyMenu", PieSlice::SliceType::BuyMenu },
			{ "Stats", PieSlice::SliceType::Stats },
			{ "Map", PieSlice::SliceType::Map },
			{ "FormSquad", PieSlice::SliceType::FormSquad },
			{ "Ceasefire", PieSlice::SliceType::Ceasefire },
			{ "Sentry", PieSlice::SliceType::Sentry },
			{ "Patrol", PieSlice::SliceType::Patrol },
			{ "BrainHunt", PieSlice::SliceType::BrainHunt },
			{ "GoldDig", PieSlice::SliceType::GoldDig },
			{ "GoTo", PieSlice::SliceType::GoTo },
			{ "Return", PieSlice::SliceType::Return },
			{ "Stay", PieSlice::SliceType::Stay },
			{ "Deliver", PieSlice::SliceType::Deliver },
			{ "Scuttle", PieSlice::SliceType::Scuttle },
			{ "Done", PieSlice::SliceType::EditorDone },
			{ "Load", PieSlice::SliceType::EditorLoad },
			{ "Save", PieSlice::SliceType::EditorSave },
			{ "New", PieSlice::SliceType::EditorNew },
			{ "Pick", PieSlice::SliceType::EditorPick },
			{ "Move", PieSlice::SliceType::EditorMove },
			{ "Remove", PieSlice::SliceType::EditorRemove },
			{ "InFront", PieSlice::SliceType::EditorInFront },
			{ "Behind", PieSlice::SliceType::EditorBehind },
			{ "ZoomIn", PieSlice::SliceType::EditorZoomIn },
			{ "ZoomOut", PieSlice::SliceType::EditorZoomOut },
			{ "Team1", PieSlice::SliceType::EditorTeam1 },
			{ "Team2", PieSlice::SliceType::EditorTeam2 },
			{ "Team3", PieSlice::SliceType::EditorTeam3 },
			{ "Team4", PieSlice::SliceType::EditorTeam4 }
		});
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, PieMenu) {
		auto luaType = ConcreteTypeLuaClassDefinition(PieMenu, Entity);

		luaType["Owner"] = sol::property(&PieMenu::GetOwner);
		luaType["Controller"] = sol::property(&PieMenu::GetController);
		luaType["AffectedObject"] = sol::property(&PieMenu::GetAffectedObject);
		luaType["Pos"] = sol::property(&PieMenu::GetPos);
		luaType["RotAngle"] = sol::property(&PieMenu::GetRotAngle, &PieMenu::SetRotAngle);
		luaType["FullInnerRadius"] = sol::property(&PieMenu::GetFullInnerRadius, &PieMenu::SetFullInnerRadius);

		luaType["PieSlices"] = sol::property(&PieMenu::GetPieSlices);

		luaType["IsSubPieMenu"] = &PieMenu::IsSubPieMenu;

		luaType["IsEnabled"] = &PieMenu::IsEnabled;
		luaType["IsEnabling"] = &PieMenu::IsEnabling;
		luaType["IsDisabling"] = &PieMenu::IsDisabling;
		luaType["IsEnablingOrDisabling"] = &PieMenu::IsEnablingOrDisabling;
		luaType["IsVisible"] = &PieMenu::IsVisible;
		luaType["HasSubPieMenuOpen"] = &PieMenu::HasSubPieMenuOpen;

		luaType["SetAnimationModeToNormal"] = &PieMenu::SetAnimationModeToNormal;
		luaType["DoDisableAnimation"] = &PieMenu::DoDisableAnimation;
		luaType["Wobble"] = &PieMenu::Wobble;
		luaType["FreezeAtRadius"] = &PieMenu::FreezeAtRadius;

		luaType["GetPieCommand"] = &PieMenu::GetPieCommand;
		luaType["GetFirstPieSliceByPresetName"] = &PieMenu::GetFirstPieSliceByPresetName;
		luaType["GetFirstPieSliceByType"] = &PieMenu::GetFirstPieSliceByType;
		luaType["AddPieSlice"] = &PieMenu::AddPieSlice, luabind::adopt(_2);
		luaType["AddPieSlice"] = &LuaAdaptersPieMenu::AddPieSlice, luabind::adopt(_2);
		luaType["AddPieSliceIfPresetNameIsUnique"] = &PieMenu::AddPieSliceIfPresetNameIsUnique, luabind::adopt(_2);
		luaType["AddPieSliceIfPresetNameIsUnique"] = &LuaAdaptersPieMenu::AddPieSliceIfPresetNameIsUnique1, luabind::adopt(_2);
		luaType["AddPieSliceIfPresetNameIsUnique"] = &LuaAdaptersPieMenu::AddPieSliceIfPresetNameIsUnique2, luabind::adopt(_2);
		luaType["RemovePieSlice"] = &PieMenu::RemovePieSlice, luabind::adopt(luabind::return_value);
		luaType["RemovePieSlicesByPresetName"] = &PieMenu::RemovePieSlicesByPresetName;
		luaType["RemovePieSlicesByType"] = &PieMenu::RemovePieSlicesByType;
		luaType["RemovePieSlicesByOriginalSource"] = &PieMenu::RemovePieSlicesByOriginalSource;
		luaType["ReplacePieSlice"] = &PieMenu::ReplacePieSlice, luabind::adopt(luabind::result) + luabind::adopt(_3);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, Round) {
		auto luaType = ConcreteTypeLuaClassDefinition(Round, Entity);

		luaType["NextParticle"] = sol::property(&Round::GetNextParticle);
		luaType["Shell"] = sol::property(&Round::GetShell);
		luaType["FireVel"] = sol::property(&Round::GetFireVel);
		luaType["InheritsFirerVelocity"] = sol::property(&Round::GetInheritsFirerVelocity);
		luaType["ShellVel"] = sol::property(&Round::GetShellVel);
		luaType["Separation"] = sol::property(&Round::GetSeparation);
		luaType["ParticleCount"] = sol::property(&Round::ParticleCount);
		luaType["AILifeTime"] = sol::property(&Round::GetAILifeTime);
		luaType["AIFireVel"] = sol::property(&Round::GetAIFireVel);
		luaType["IsEmpty"] = sol::property(&Round::IsEmpty);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, Scene) {
		auto luaType = ConcreteTypeLuaClassDefinition(Scene, Entity);

		luaType["Location"] = sol::property(&Scene::GetLocation, &Scene::SetLocation);
		//luaType["Terrain"] = sol::property(&Scene::GetTerrain);
		luaType["Dimensions"] = sol::property(&Scene::GetDimensions);
		luaType["Width"] = sol::property(&Scene::GetWidth);
		luaType["Height"] = sol::property(&Scene::GetHeight);
		luaType["WrapsX"] = sol::property(&Scene::WrapsX);
		luaType["WrapsY"] = sol::property(&Scene::WrapsY);
		luaType["TeamOwnership"] = sol::property(&Scene::GetTeamOwnership, &Scene::SetTeamOwnership);
		luaType["GlobalAcc"] = sol::property(&Scene::GetGlobalAcc, &Scene::SetGlobalAcc);
		luaType["ScenePathSize"] = sol::property(&Scene::GetScenePathSize);

		luaType["Deployments"] = &Scene::m_Deployments;

		luaType["BackgroundLayers"] = sol::readonly(&Scene::m_BackLayerList);

		luaType["GetScenePath"] = &Scene::GetScenePath;
		luaType["GetBuildBudget"] = &Scene::GetBuildBudget;
		luaType["SetBuildBudget"] = &Scene::SetBuildBudget;
		luaType["IsScanScheduled"] = &Scene::IsScanScheduled;
		luaType["SetScheduledScan"] = &Scene::SetScheduledScan;
		luaType["ClearPlacedObjectSet"] = &Scene::ClearPlacedObjectSet;
		luaType["PlaceResidentBrain"] = &Scene::PlaceResidentBrain;
		luaType["PlaceResidentBrains"] = &Scene::PlaceResidentBrains;
		luaType["RetrieveResidentBrains"] = &Scene::RetrieveResidentBrains;
		luaType["GetResidentBrain"] = &Scene::GetResidentBrain;
		luaType["SetResidentBrain"] = &Scene::SetResidentBrain;
		luaType["Areas"] = &Scene::m_AreaList;
		luaType["SetArea"] = &Scene::SetArea;
		luaType["HasArea"] = &Scene::HasArea;
		luaType["GetArea"] = (Scene::Area * (Scene:: *)(const std::string &areaName)) &Scene::GetArea;
		luaType["GetOptionalArea"] = &Scene::GetOptionalArea;
		luaType["WithinArea"] = &Scene::WithinArea;
		luaType["ResetPathFinding"] = &Scene::ResetPathFinding;
		luaType["UpdatePathFinding"] = &Scene::UpdatePathFinding;
		luaType["PathFindingUpdated"] = &Scene::PathFindingUpdated;
		luaType["CalculatePath"] = &LuaAdaptersScene::CalculatePath1;
		luaType["CalculatePath"] = &LuaAdaptersScene::CalculatePath2;
		luaType["CalculatePathAsync"] = &LuaAdaptersScene::CalculatePathAsync1;
		luaType["CalculatePathAsync"] = &LuaAdaptersScene::CalculatePathAsync2;

		luaType.new_enum("PlacedObjectSets", EnumList(Scene::PlacedObjectSets) {
			{ "PLACEONLOAD", Scene::PlacedObjectSets::PLACEONLOAD },
			{ "BLUEPRINT", Scene::PlacedObjectSets::BLUEPRINT },
			{ "AIPLAN", Scene::PlacedObjectSets::AIPLAN },
			{ "PLACEDSETSCOUNT", Scene::PlacedObjectSets::PLACEDSETSCOUNT }
		});
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, SceneArea) {
		auto luaType = SimpleNamedTypeLuaClassDefinition(Scene::Area, "Area");

		luaType.set(sol::meta_function::construct, sol::constructors<
			Scene::Area(),
			Scene::Area(std::string),
			Scene::Area(const Scene::Area&)
		>());

		luaType["ClassName"] = sol::property(&Scene::Area::GetClassName);
		luaType["Name"] = sol::property(&Scene::Area::GetName);
		luaType["FirstBox"] = sol::property(&Scene::Area::GetFirstBox);
		luaType["Center"] = sol::property(&Scene::Area::GetCenterPoint);
		luaType["RandomPoint"] = sol::property(&Scene::Area::GetRandomPoint);

		luaType["Reset"] = &Scene::Area::Reset;
		luaType["Boxes"] = &Scene::Area::m_BoxList;
		luaType["AddBox"] = &Scene::Area::AddBox;
		luaType["RemoveBox"] = &Scene::Area::RemoveBox;
		luaType["HasNoArea"] = &Scene::Area::HasNoArea;
		luaType["IsInside"] = &Scene::Area::IsInside;
		luaType["IsInsideX"] = &Scene::Area::IsInsideX;
		luaType["IsInsideY"] = &Scene::Area::IsInsideY;
		luaType["GetBoxInside"] = &Scene::Area::GetBoxInside;
		luaType["RemoveBoxInside"] = &Scene::Area::RemoveBoxInside;
		luaType["GetCenterPoint"] = &Scene::Area::GetCenterPoint;
		luaType["GetRandomPoint"] = &Scene::Area::GetRandomPoint;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, SceneLayer) {
		auto luaType = SimpleTypeLuaClassDefinition(SceneLayer);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, SceneObject) {
		auto luaType = AbstractTypeLuaClassDefinition(SceneObject, Entity);

		luaType["Pos"] = sol::property(&SceneObject::GetPos, &SceneObject::SetPos);
		luaType["HFlipped"] = sol::property(&SceneObject::IsHFlipped, &SceneObject::SetHFlipped);
		luaType["RotAngle"] = sol::property(&SceneObject::GetRotAngle, &SceneObject::SetRotAngle);
		luaType["Team"] = sol::property(&SceneObject::GetTeam, &SceneObject::SetTeam);
		luaType["PlacedByPlayer"] = sol::property(&SceneObject::GetPlacedByPlayer, &SceneObject::SetPlacedByPlayer);
		luaType["IsBuyable"] = sol::property(&SceneObject::IsBuyable);

		luaType["IsOnScenePoint"] = &SceneObject::IsOnScenePoint;
		luaType["GetGoldValue"] = &SceneObject::GetGoldValueOld;
		luaType["GetGoldValue"] = &SceneObject::GetGoldValue;
		luaType["SetGoldValue"] = &SceneObject::SetGoldValue;
		luaType["GetGoldValueString"] = &SceneObject::GetGoldValueString;
		luaType["GetTotalValue"] = &SceneObject::GetTotalValue;

		luaType["GetTotalValue"] = &LuaAdaptersSceneObject::GetTotalValue;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, SLBackground) {
		auto luaType = SimpleTypeLuaClassDefinition(SLBackground);

		luaType["Frame"] = sol::property(&SLBackground::GetFrame, &SLBackground::SetFrame);
		luaType["SpriteAnimMode"] = sol::property(&SLBackground::GetSpriteAnimMode, &SLBackground::SetSpriteAnimMode);
		luaType["SpriteAnimDuration"] = sol::property(&SLBackground::GetSpriteAnimDuration, &SLBackground::SetSpriteAnimDuration);
		luaType["IsAnimatedManually"] = sol::property(&SLBackground::IsAnimatedManually, &SLBackground::SetAnimatedManually);
		luaType["AutoScrollX"] = sol::property(&SLBackground::GetAutoScrollX, &SLBackground::SetAutoScrollX);
		luaType["AutoScrollY"] = sol::property(&SLBackground::GetAutoScrollY, &SLBackground::SetAutoScrollY);
		luaType["AutoScrollInterval"] = sol::property(&SLBackground::GetAutoScrollStepInterval, &SLBackground::SetAutoScrollStepInterval);
		luaType["AutoScrollStep"] = sol::property(&SLBackground::GetAutoScrollStep, &SLBackground::SetAutoScrollStep);
		luaType["AutoScrollStepX"] = sol::property(&SLBackground::GetAutoScrollStepX, &SLBackground::SetAutoScrollStepX);
		luaType["AutoScrollStepY"] = sol::property(&SLBackground::GetAutoScrollStepY, &SLBackground::SetAutoScrollStepY);

		luaType["IsAutoScrolling"] = &SLBackground::IsAutoScrolling;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, SoundContainer) {
		auto luaType = ConcreteTypeLuaClassDefinition(SoundContainer, Entity);

		luaType.set(sol::meta_function::construct, sol::constructors<
			Scene::Area()
		>());

		luaType["SoundOverlapMode"] = sol::property(&SoundContainer::GetSoundOverlapMode, &SoundContainer::SetSoundOverlapMode);
		luaType["Immobile"] = sol::property(&SoundContainer::IsImmobile, &SoundContainer::SetImmobile);
		luaType["AttenuationStartDistance"] = sol::property(&SoundContainer::GetAttenuationStartDistance, &SoundContainer::SetAttenuationStartDistance);
		luaType["Loops"] = sol::property(&SoundContainer::GetLoopSetting, &SoundContainer::SetLoopSetting);
		luaType["Priority"] = sol::property(&SoundContainer::GetPriority, &SoundContainer::SetPriority);
		luaType["AffectedByGlobalPitch"] = sol::property(&SoundContainer::IsAffectedByGlobalPitch, &SoundContainer::SetAffectedByGlobalPitch);
		luaType["Pos"] = sol::property(&SoundContainer::GetPosition, &SoundContainer::SetPosition);
		luaType["Volume"] = sol::property(&SoundContainer::GetVolume, &SoundContainer::SetVolume);
		luaType["Pitch"] = sol::property(&SoundContainer::GetPitch, &SoundContainer::SetPitch);
		luaType["PitchVariation"] = sol::property(&SoundContainer::GetPitchVariation, &SoundContainer::SetPitchVariation);

		luaType["HasAnySounds"] = &SoundContainer::HasAnySounds;
		luaType["GetTopLevelSoundSet"] = &SoundContainer::GetTopLevelSoundSet;
		luaType["SetTopLevelSoundSet"] = &SoundContainer::SetTopLevelSoundSet;
		luaType["IsBeingPlayed"] = &SoundContainer::IsBeingPlayed;
		luaType["Play"] = (bool (SoundContainer:: *)()) &SoundContainer::Play;
		luaType["Play"] = (bool (SoundContainer:: *)(const int player)) &SoundContainer::Play;
		luaType["Play"] = (bool (SoundContainer:: *)(const Vector &position)) &SoundContainer::Play;
		luaType["Play"] = (bool (SoundContainer:: *)(const Vector &position, int player)) &SoundContainer::Play;
		luaType["Stop"] = (bool (SoundContainer:: *)()) &SoundContainer::Stop;
		luaType["Stop"] = (bool (SoundContainer:: *)(int player)) &SoundContainer::Stop;
		luaType["Restart"] = (bool (SoundContainer:: *)()) &SoundContainer::Restart;
		luaType["Restart"] = (bool (SoundContainer:: *)(int player)) &SoundContainer::Restart;
		luaType["FadeOut"] = &SoundContainer::FadeOut;

		luaType.new_enum("SoundOverlapMode", EnumList(SoundContainer::SoundOverlapMode) {
			{ "OVERLAP", SoundContainer::SoundOverlapMode::OVERLAP },
			{ "RESTART", SoundContainer::SoundOverlapMode::RESTART },
			{ "IGNORE_PLAY", SoundContainer::SoundOverlapMode::IGNORE_PLAY }
		});
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, SoundSet) {
		auto luaType = SimpleTypeLuaClassDefinition(SoundSet);

		luaType.set(sol::meta_function::construct, sol::constructors<
			SoundSet()
		>());

		luaType["SoundSelectionCycleMode"] = sol::property(&SoundSet::GetSoundSelectionCycleMode, &SoundSet::SetSoundSelectionCycleMode);

		luaType["SubSoundSets"] = sol::readonly(&SoundSet::m_SubSoundSets);

		luaType["HasAnySounds"] = &SoundSet::HasAnySounds;
		luaType["SelectNextSounds"] = &SoundSet::SelectNextSounds;
		luaType["AddSound"] = (void (SoundSet:: *)(const std::string &soundFilePath)) &SoundSet::AddSound;
		luaType["AddSound"] = (void (SoundSet:: *)(const std::string &soundFilePath, const Vector &offset, float minimumAudibleDistance, float attenuationStartDistance)) &SoundSet::AddSound;
		luaType["RemoveSound"] = (bool (SoundSet:: *)(const std::string &soundFilePath)) &SoundSet::RemoveSound;
		luaType["RemoveSound"] = (bool (SoundSet:: *)(const std::string &soundFilePath, bool removeFromSubSoundSets)) &SoundSet::RemoveSound;
		luaType["AddSoundSet"] = &SoundSet::AddSoundSet;

		luaType.new_enum("SoundSelectionCycleMode", EnumList(SoundSet::SoundSelectionCycleMode) {
			{ "RANDOM", SoundSet::SoundSelectionCycleMode::RANDOM },
			{ "FORWARDS", SoundSet::SoundSelectionCycleMode::FORWARDS },
			{ "ALL", SoundSet::SoundSelectionCycleMode::ALL }
		});
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, TDExplosive) {
		auto luaType = ConcreteTypeLuaClassDefinition(TDExplosive, ThrownDevice);

		luaType["IsAnimatedManually"] = sol::property(&TDExplosive::IsAnimatedManually, &TDExplosive::SetAnimatedManually);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, TerrainObject) {
		auto luaType = ConcreteTypeLuaClassDefinition(TerrainObject, SceneObject);

		luaType["GetBitmapOffset"] = &TerrainObject::GetBitmapOffset;
		luaType["GetBitmapWidth"] = &TerrainObject::GetBitmapWidth;
		luaType["GetBitmapHeight"] = &TerrainObject::GetBitmapHeight;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, ThrownDevice) {
		auto luaType = ConcreteTypeLuaClassDefinition(ThrownDevice, HeldDevice);

		luaType["MinThrowVel"] = sol::property(&ThrownDevice::GetMinThrowVel, &ThrownDevice::SetMinThrowVel);
		luaType["MaxThrowVel"] = sol::property(&ThrownDevice::GetMaxThrowVel, &ThrownDevice::SetMaxThrowVel);
		luaType["StartThrowOffset"] = sol::property(&ThrownDevice::GetStartThrowOffset, &ThrownDevice::SetStartThrowOffset);
		luaType["EndThrowOffset"] = sol::property(&ThrownDevice::GetEndThrowOffset, &ThrownDevice::SetEndThrowOffset);

		luaType["GetCalculatedMaxThrowVelIncludingArmThrowStrength"] = &ThrownDevice::GetCalculatedMaxThrowVelIncludingArmThrowStrength;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, Turret) {
		auto luaType = ConcreteTypeLuaClassDefinition(Turret, Attachable);

		luaType["MountedDevice"] = sol::property(&Turret::GetFirstMountedDevice, &LuaAdaptersPropertyOwnershipSafetyFaker::TurretSetFirstMountedDevice);
		luaType["MountedDeviceRotationOffset"] = sol::property(&Turret::GetMountedDeviceRotationOffset, &Turret::SetMountedDeviceRotationOffset);

		luaType["GetMountedDevices"] = &Turret::GetMountedDevices;
		luaType["AddMountedDevice"] = &Turret::AddMountedDevice, luabind::adopt(_2);
		luaType["AddMountedDevice"] = &LuaAdaptersTurret::AddMountedFirearm, luabind::adopt(_2);
	}
}