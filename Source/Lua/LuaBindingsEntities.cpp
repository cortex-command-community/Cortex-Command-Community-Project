// Make sure that binding definition files are always set to NOT use pre-compiled headers and conformance mode (/permissive) otherwise everything will be on fire!

#include "LuaBindingRegisterDefinitions.h"

#include "PieSlice.h"
#include "SoundSet.h"

using namespace RTE;

LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, Entity) {
	registry.Class<Entity>("Entity")

	    .addToString()

	    .addProperty("ClassName", &Entity::GetClassName)
	    .addProperty("PresetName", &Entity::GetPresetName, &LuaAdaptersEntity::SetPresetName)
	    .addProperty("Description", &Entity::GetDescription, &Entity::SetDescription)
	    .addProperty("IsOriginalPreset", &Entity::IsOriginalPreset)
	    .addProperty("ModuleID", &Entity::GetModuleID)
	    .addProperty("ModuleName", &Entity::GetModuleName)
	    .addProperty("RandomWeight", &Entity::GetRandomWeight)
	    .addProperty("Groups", &Entity::GetGroups, LuaBinding::ReturnStlIterator)

	    .addFunction("Clone", &LuaAdaptersEntityClone::CloneEntity, LuaBinding::AdoptResult)
	    .addFunction("Reset", &Entity::Reset)
	    .addFunction("GetModuleAndPresetName", &Entity::GetModuleAndPresetName)
	    .addFunction("AddToGroup", &Entity::AddToGroup)
	    .addFunction("RemoveFromGroup", &Entity::RemoveFromGroup)
	    .addFunction("IsInGroup", &Entity::IsInGroup);
}

LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, ACDropShip) {
	ConcreteTypeLuaClassDefinition(ACDropShip, ACraft)

	    .addProperty("RightEngine", &ACDropShip::GetRightThruster, &LuaAdaptersPropertyOwnershipSafetyFaker::ACDropShipSetRightThruster)
	    .addProperty("LeftEngine", &ACDropShip::GetLeftThruster, &LuaAdaptersPropertyOwnershipSafetyFaker::ACDropShipSetLeftThruster)
	    .addProperty("RightThruster", &ACDropShip::GetURightThruster, &LuaAdaptersPropertyOwnershipSafetyFaker::ACDropShipSetURightThruster)
	    .addProperty("LeftThruster", &ACDropShip::GetULeftThruster, &LuaAdaptersPropertyOwnershipSafetyFaker::ACDropShipSetULeftThruster)
	    .addProperty("RightHatch", &ACDropShip::GetRightHatch, &LuaAdaptersPropertyOwnershipSafetyFaker::ACDropShipSetRightHatch)
	    .addProperty("LeftHatch", &ACDropShip::GetLeftHatch, &LuaAdaptersPropertyOwnershipSafetyFaker::ACDropShipSetLeftHatch)
	    .addProperty("MaxEngineAngle", &ACDropShip::GetMaxEngineAngle, &ACDropShip::SetMaxEngineAngle)
	    .addProperty("LateralControlSpeed", &ACDropShip::GetLateralControlSpeed, &ACDropShip::SetLateralControlSpeed)
	    .addProperty("LateralControl", &ACDropShip::GetLateralControl)
	    .addProperty("HoverHeightModifier", &ACDropShip::GetHoverHeightModifier, &ACDropShip::SetHoverHeightModifier)

	    .addFunction("DetectObstacle", &ACDropShip::DetectObstacle)
	    .addFunction("GetAltitude", &ACDropShip::GetAltitude);
}

LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, ACrab) {
	ConcreteTypeLuaClassDefinition(ACrab, Actor)

	    .addConstructor<>()

	    .addProperty("Turret", &ACrab::GetTurret, &LuaAdaptersPropertyOwnershipSafetyFaker::ACrabSetTurret)
	    .addProperty("Jetpack", &ACrab::GetJetpack, &LuaAdaptersPropertyOwnershipSafetyFaker::ACrabSetJetpack)
	    .addProperty("LeftFGLeg", &ACrab::GetLeftFGLeg, &LuaAdaptersPropertyOwnershipSafetyFaker::ACrabSetLeftFGLeg)
	    .addProperty("LeftBGLeg", &ACrab::GetLeftBGLeg, &LuaAdaptersPropertyOwnershipSafetyFaker::ACrabSetLeftBGLeg)
	    .addProperty("RightFGLeg", &ACrab::GetRightFGLeg, &LuaAdaptersPropertyOwnershipSafetyFaker::ACrabSetRightFGLeg)
	    .addProperty("RightBGLeg", &ACrab::GetRightBGLeg, &LuaAdaptersPropertyOwnershipSafetyFaker::ACrabSetRightBGLeg)
	    .addProperty("StrideSound", &ACrab::GetStrideSound, &LuaAdaptersPropertyOwnershipSafetyFaker::ACrabSetStrideSound)
	    .addProperty("StrideFrame", &ACrab::StrideFrame)
	    .addProperty("EquippedItem", &ACrab::GetEquippedItem)
	    .addProperty("FirearmIsReady", &ACrab::FirearmIsReady)
	    .addProperty("FirearmIsEmpty", &ACrab::FirearmIsEmpty)
	    .addProperty("FirearmNeedsReload", &ACrab::FirearmNeedsReload)
	    .addProperty("FirearmIsSemiAuto", &ACrab::FirearmIsSemiAuto)
	    .addProperty("FirearmActivationDelay", &ACrab::FirearmActivationDelay)
	    .addProperty("AimRangeUpperLimit", &ACrab::GetAimRangeUpperLimit, &ACrab::SetAimRangeUpperLimit)
	    .addProperty("AimRangeLowerLimit", &ACrab::GetAimRangeLowerLimit, &ACrab::SetAimRangeLowerLimit)

	    .addFunction("ReloadFirearms", &ACrab::ReloadFirearms)
	    .addFunction("IsWithinRange", &ACrab::IsWithinRange)
	    .addFunction("Look", &ACrab::Look)
	    .addFunction("LookForMOs", &ACrab::LookForMOs)
	    .addFunction("GetLimbPath", &ACrab::GetLimbPath)
	    .addFunction("GetLimbPathTravelSpeed", &ACrab::GetLimbPathTravelSpeed)
	    .addFunction("SetLimbPathTravelSpeed", &ACrab::SetLimbPathTravelSpeed)
	    .addFunction("GetLimbPathPushForce", &ACrab::GetLimbPathPushForce)
	    .addFunction("SetLimbPathPushForce", &ACrab::SetLimbPathPushForce)

	    .addEnum({{"LEFTSIDE", ACrab::Side::LEFTSIDE},
	              {"RIGHTSIDE", ACrab::Side::RIGHTSIDE},
	              {"SIDECOUNT", ACrab::Side::SIDECOUNT}})
	    .addEnum({{"FGROUND", ACrab::Layer::FGROUND},
	              {"BGROUND", ACrab::Layer::BGROUND}})
	    .addEnum({{"STILL", ACrab::DeviceHandlingState::STILL},
	              {"POINTING", ACrab::DeviceHandlingState::POINTING},
	              {"SCANNING", ACrab::DeviceHandlingState::SCANNING},
	              {"AIMING", ACrab::DeviceHandlingState::AIMING},
	              {"FIRING", ACrab::DeviceHandlingState::FIRING},
	              {"THROWING", ACrab::DeviceHandlingState::THROWING},
	              {"DIGGING", ACrab::DeviceHandlingState::DIGGING}})
	    .addEnum({{"NOSWEEP", ACrab::SweepState::NOSWEEP},
	              {"SWEEPINGUP", ACrab::SweepState::SWEEPINGUP},
	              {"SWEEPUPPAUSE", ACrab::SweepState::SWEEPUPPAUSE},
	              {"SWEEPINGDOWN", ACrab::SweepState::SWEEPINGDOWN},
	              {"SWEEPDOWNPAUSE", ACrab::SweepState::SWEEPDOWNPAUSE}})
	    .addEnum({{"NOTDIGGING", ACrab::DigState::NOTDIGGING},
	              {"PREDIG", ACrab::DigState::PREDIG},
	              {"STARTDIG", ACrab::DigState::STARTDIG},
	              {"TUNNELING", ACrab::DigState::TUNNELING},
	              {"FINISHINGDIG", ACrab::DigState::FINISHINGDIG},
	              {"PAUSEDIGGER", ACrab::DigState::PAUSEDIGGER}})
	    .addEnum({{"NOTJUMPING", ACrab::JumpState::NOTJUMPING},
	              {"FORWARDJUMP", ACrab::JumpState::FORWARDJUMP},
	              {"PREJUMP", ACrab::JumpState::PREUPJUMP},
	              {"UPJUMP", ACrab::JumpState::UPJUMP},
	              {"APEXJUMP", ACrab::JumpState::APEXJUMP},
	              {"LANDJUMP", ACrab::JumpState::LANDJUMP}});
}

LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, ACraft) {
	AbstractTypeLuaClassDefinition(ACraft, Actor)

	    .addProperty("HatchState", &ACraft::GetHatchState)
	    .addProperty("HatchOpenSound", &ACraft::GetHatchOpenSound, &LuaAdaptersPropertyOwnershipSafetyFaker::ACraftSetHatchOpenSound)
	    .addProperty("HatchCloseSound", &ACraft::GetHatchCloseSound, &LuaAdaptersPropertyOwnershipSafetyFaker::ACraftSetHatchCloseSound)
	    .addProperty("CrashSound", &ACraft::GetCrashSound, &LuaAdaptersPropertyOwnershipSafetyFaker::ACraftSetCrashSound)
	    .addProperty("CanEnterOrbit", &ACraft::GetCanEnterOrbit, &ACraft::SetCanEnterOrbit)
	    .addProperty("MaxPassengers", &ACraft::GetMaxPassengers)
	    .addProperty("DeliveryDelayMultiplier", &ACraft::GetDeliveryDelayMultiplier)
	    .addProperty("ScuttleOnDeath", &ACraft::GetScuttleOnDeath, &ACraft::SetScuttleOnDeath)
	    .addProperty("HatchDelay", &ACraft::GetHatchDelay, &ACraft::SetHatchDelay)

	    .addFunction("OpenHatch", &ACraft::OpenHatch)
	    .addFunction("CloseHatch", &ACraft::CloseHatch)

	    .addEnum({{"CLOSED", ACraft::HatchState::CLOSED},
	              {"OPENING", ACraft::HatchState::OPENING},
	              {"OPEN", ACraft::HatchState::OPEN},
	              {"CLOSING", ACraft::HatchState::CLOSING},
	              {"HatchStateCount", ACraft::HatchState::HatchStateCount}})
	    .addEnum({{"RIGHT", ACraft::Side::RIGHT},
	              {"LEFT", ACraft::Side::LEFT}})

	    .addEnum({{"FALL", ACraft::CraftDeliverySequence::FALL},
	              {"LAND", ACraft::CraftDeliverySequence::LAND},
	              {"STANDBY", ACraft::CraftDeliverySequence::STANDBY},
	              {"UNLOAD", ACraft::CraftDeliverySequence::UNLOAD},
	              {"LAUNCH", ACraft::CraftDeliverySequence::LAUNCH},
	              {"UNSTICK", ACraft::CraftDeliverySequence::UNSTICK}})
	    .addEnum({{"HOVER", ACraft::AltitudeMoveState::HOVER},
	              {"DESCEND", ACraft::AltitudeMoveState::DESCEND},
	              {"ASCEND", ACraft::AltitudeMoveState::ASCEND}});
}

LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, ACRocket) {
	ConcreteTypeLuaClassDefinition(ACRocket, ACraft)

	    .addProperty("RightLeg", &ACRocket::GetRightLeg, &LuaAdaptersPropertyOwnershipSafetyFaker::ACRocketSetRightLeg)
	    .addProperty("LeftLeg", &ACRocket::GetLeftLeg, &LuaAdaptersPropertyOwnershipSafetyFaker::ACRocketSetLeftLeg)
	    .addProperty("MainEngine", &ACRocket::GetMainThruster, &LuaAdaptersPropertyOwnershipSafetyFaker::ACRocketSetMainThruster)
	    .addProperty("LeftEngine", &ACRocket::GetLeftThruster, &LuaAdaptersPropertyOwnershipSafetyFaker::ACRocketSetLeftThruster)
	    .addProperty("RightEngine", &ACRocket::GetRightThruster, &LuaAdaptersPropertyOwnershipSafetyFaker::ACRocketSetRightThruster)
	    .addProperty("LeftThruster", &ACRocket::GetULeftThruster, &LuaAdaptersPropertyOwnershipSafetyFaker::ACRocketSetULeftThruster)
	    .addProperty("RightThruster", &ACRocket::GetURightThruster, &LuaAdaptersPropertyOwnershipSafetyFaker::ACRocketSetURightThruster)
	    .addProperty("GearState", &ACRocket::GetGearState)

	    .addEnum({{"RAISED", ACRocket::LandingGearState::RAISED},
	              {"LOWERED", ACRocket::LandingGearState::LOWERED},
	              {"LOWERING", ACRocket::LandingGearState::LOWERING},
	              {"RAISING", ACRocket::LandingGearState::RAISING},
	              {"GearStateCount", ACRocket::LandingGearState::GearStateCount}});
}

LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, Actor) {
	ConcreteTypeLuaClassDefinition(Actor, MOSRotating)

	    .addConstructor<>()

	    .addProperty("PlayerControllable", &Actor::IsPlayerControllable, &Actor::SetPlayerControllable)
	    .addProperty("BodyHitSound", &Actor::GetBodyHitSound, &LuaAdaptersPropertyOwnershipSafetyFaker::ActorSetBodyHitSound)
	    .addProperty("AlarmSound", &Actor::GetAlarmSound, &LuaAdaptersPropertyOwnershipSafetyFaker::ActorSetAlarmSound)
	    .addProperty("PainSound", &Actor::GetPainSound, &LuaAdaptersPropertyOwnershipSafetyFaker::ActorSetPainSound)
	    .addProperty("DeathSound", &Actor::GetDeathSound, &LuaAdaptersPropertyOwnershipSafetyFaker::ActorSetDeathSound)
	    .addProperty("DeviceSwitchSound", &Actor::GetDeviceSwitchSound, &LuaAdaptersPropertyOwnershipSafetyFaker::ActorSetDeviceSwitchSound)
	    .addProperty("ImpulseDamageThreshold", &Actor::GetTravelImpulseDamage, &Actor::SetTravelImpulseDamage)
	    .addProperty("StableRecoveryDelay", &Actor::GetStableRecoverDelay, &Actor::SetStableRecoverDelay)
	    .addProperty("CanRun", &Actor::GetCanRun, &Actor::SetCanRun)
	    .addProperty("CrouchWalkSpeedMultiplier", &Actor::GetCrouchWalkSpeedMultiplier, &Actor::SetCrouchWalkSpeedMultiplier)
	    .addProperty("Status", &Actor::GetStatus, &Actor::SetStatus)
	    .addProperty("MovementState", &Actor::GetMovementState, &Actor::SetMovementState)
	    .addProperty("Health", &Actor::GetHealth, &Actor::SetHealth)
	    .addProperty("PrevHealth", &Actor::GetPrevHealth)
	    .addProperty("MaxHealth", &Actor::GetMaxHealth, &Actor::SetMaxHealth)
	    .addProperty("InventoryMass", &Actor::GetInventoryMass)
	    .addProperty("GoldCarried", &Actor::GetGoldCarried, &Actor::SetGoldCarried)
	    .addProperty("AimRange", &Actor::GetAimRange, &Actor::SetAimRange)
	    .addProperty("CPUPos", &Actor::GetCPUPos)
	    .addProperty("EyePos", &Actor::GetEyePos)
	    .addProperty("HolsterOffset", &Actor::GetHolsterOffset, &Actor::SetHolsterOffset)
	    .addProperty("ReloadOffset", &Actor::GetReloadOffset, &Actor::SetReloadOffset)
	    .addProperty("ViewPoint", &Actor::GetViewPoint, &Actor::SetViewPoint)
	    .addProperty("ItemInReach", &Actor::GetItemInReach, &Actor::SetItemInReach)
	    .addProperty("SharpAimProgress", &Actor::GetSharpAimProgress)
	    .addProperty("Height", &Actor::GetHeight)
	    .addProperty("AIMode", &Actor::GetAIMode, &Actor::SetAIMode)
	    .addProperty("DeploymentID", &Actor::GetDeploymentID)
	    .addProperty("PassengerSlots", &Actor::GetPassengerSlots, &Actor::SetPassengerSlots)
	    .addProperty("Perceptiveness", &Actor::GetPerceptiveness, &Actor::SetPerceptiveness)
	    .addProperty("PainThreshold", &Actor::GetPainThreshold, &Actor::SetPainThreshold)
	    .addProperty("CanRevealUnseen", &Actor::GetCanRevealUnseen, &Actor::SetCanRevealUnseen)
	    .addProperty("InventorySize", &Actor::GetInventorySize)
	    .addProperty("MaxInventoryMass", &Actor::GetMaxInventoryMass)
	    .addProperty("MovePathSize", &Actor::GetMovePathSize)
	    .addProperty("MovePathEnd", &Actor::GetMovePathEnd)
	    .addProperty("IsWaitingOnNewMovePath", &Actor::IsWaitingOnNewMovePath)
	    .addProperty("AimDistance", &Actor::GetAimDistance, &Actor::SetAimDistance)
	    .addProperty("SightDistance", &Actor::GetSightDistance, &Actor::SetSightDistance)
	    .addProperty("PieMenu", &Actor::GetPieMenu, &LuaAdaptersPropertyOwnershipSafetyFaker::ActorSetPieMenu)
	    .addProperty("AIBaseDigStrength", &Actor::GetAIBaseDigStrength, &Actor::SetAIBaseDigStrength)
	    .addProperty("DigStrength", &Actor::EstimateDigStrength)
	    .addProperty("JumpHeight", &Actor::EstimateJumpHeight)
	    .addProperty("SceneWaypoints", &LuaAdaptersActor::GetSceneWaypoints, LuaBinding::AdoptResult, LuaBinding::ReturnStlIterator)
	    .addProperty("LimbPushForcesAndCollisionsDisabled", &Actor::GetLimbPushForcesAndCollisionsDisabled, &Actor::SetLimbPushForcesAndCollisionsDisabled)
	    .addProperty("MoveProximityLimit", &Actor::GetMoveProximityLimit, &Actor::SetMoveProximityLimit)

	    .addReadWriteMember("MOMoveTarget", &Actor::m_pMOMoveTarget)
	    .addProperty("MovePath", &Actor::m_MovePath, LuaBinding::ReturnStlIterator)
	    .addProperty("Inventory", &Actor::m_Inventory, LuaBinding::ReturnStlIterator)

	    .addFunction("GetController", &Actor::GetController)
	    .addFunction("IsPlayerControlled", &Actor::IsPlayerControlled)
	    .addFunction("IsControllable", &Actor::IsControllable)
	    .addFunction("SetControllerMode", &Actor::SetControllerMode)
	    .addFunction("SwapControllerModes", &Actor::SwapControllerModes)
	    .addFunction("GetStableVelocityThreshold", &Actor::GetStableVel)
	    .addFunction("SetStableVelocityThreshold", (void(Actor::*)(float, float)) & Actor::SetStableVel)
	    .addFunction("SetStableVelocityThreshold", (void(Actor::*)(Vector)) & Actor::SetStableVel)
	    .addFunction("GetAimAngle", &Actor::GetAimAngle)
	    .addFunction("SetAimAngle", &Actor::SetAimAngle)
	    .addFunction("HasObject", &Actor::HasObject)
	    .addFunction("HasObjectInGroup", &Actor::HasObjectInGroup)
	    .addFunction("IsWithinRange", &Actor::IsWithinRange)
	    .addFunction("AddGold", &Actor::AddGold)
	    .addFunction("AddHealth", &Actor::AddHealth)
	    .addFunction("IsStatus", &Actor::IsStatus)
	    .addFunction("IsDead", &Actor::IsDead)
	    .addFunction("AddAISceneWaypoint", &Actor::AddAISceneWaypoint)
	    .addFunction("AddAIMOWaypoint", &Actor::AddAIMOWaypoint)
	    .addFunction("ClearAIWaypoints", &Actor::ClearAIWaypoints)
	    .addFunction("GetLastAIWaypoint", &Actor::GetLastAIWaypoint)
	    .addFunction("GetAIMOWaypointID", &Actor::GetAIMOWaypointID)
	    .addFunction("GetWaypointListSize", &Actor::GetWaypointsSize)
	    .addFunction("ClearMovePath", &Actor::ClearMovePath)
	    .addFunction("AddToMovePathBeginning", &Actor::AddToMovePathBeginning)
	    .addFunction("AddToMovePathEnd", &Actor::AddToMovePathEnd)
	    .addFunction("RemoveMovePathBeginning", &Actor::RemoveMovePathBeginning)
	    .addFunction("RemoveMovePathEnd", &Actor::RemoveMovePathEnd)
	    .addFunction("AddInventoryItem", &Actor::AddInventoryItem, LuaBinding::AdoptArg<2>)
	    .addFunction("RemoveInventoryItem", (void(Actor::*)(const std::string&)) & Actor::RemoveInventoryItem)
	    .addFunction("RemoveInventoryItem", (void(Actor::*)(const std::string&, const std::string&)) & Actor::RemoveInventoryItem)
	    .addFunction("RemoveInventoryItemAtIndex", &Actor::RemoveInventoryItemAtIndex, LuaBinding::AdoptResult)
	    .addFunction("SwapNextInventory", &Actor::SwapNextInventory, LuaBinding::AdoptResult, LuaBinding::AdoptArg<2>)
	    .addFunction("SwapPrevInventory", &Actor::SwapPrevInventory, LuaBinding::AdoptResult, LuaBinding::AdoptArg<2>)
	    .addFunction("DropAllInventory", &Actor::DropAllInventory)
	    .addFunction("DropAllGold", &Actor::DropAllGold)
	    .addFunction("IsInventoryEmpty", &Actor::IsInventoryEmpty)
	    .addFunction("ActivateHotkeyAction", &Actor::ActivateHotkeyAction)
	    .addFunction("DeactivateHotkeyAction", &Actor::DeactivateHotkeyAction)
	    .addFunction("HotkeyActionIsActivated", &Actor::HotkeyActionIsActivated)
	    .addFunction("DrawWaypoints", &Actor::DrawWaypoints)
	    .addFunction("SetMovePathToUpdate", &Actor::SetMovePathToUpdate)
	    .addFunction("UpdateMovePath", &Actor::UpdateMovePath)
	    .addFunction("SetAlarmPoint", &Actor::AlarmPoint)
	    .addFunction("GetAlarmPoint", &Actor::GetAlarmPoint)
	    .addFunction("IsOrganic", &Actor::IsOrganic)
	    .addFunction("IsMechanical", &Actor::IsMechanical)

	    .addEnum({{"STABLE", Actor::Status::STABLE},
	              {"UNSTABLE", Actor::Status::UNSTABLE},
	              {"INACTIVE", Actor::Status::INACTIVE},
	              {"DYING", Actor::Status::DYING},
	              {"DEAD", Actor::Status::DEAD}})
	    .addEnum({{"NOMOVE", Actor::MovementState::NOMOVE},
	              {"CROUCH", Actor::MovementState::CROUCH},
	              {"STAND", Actor::MovementState::STAND},
	              {"WALK", Actor::MovementState::WALK},
	              {"RUN", Actor::MovementState::RUN},
	              {"JUMP", Actor::MovementState::JUMP},
	              {"DISLODGE", Actor::MovementState::DISLODGE},
	              {"PRONE", Actor::MovementState::PRONE},
	              {"CRAWL", Actor::MovementState::CRAWL},
	              {"ARMCRAWL", Actor::MovementState::ARMCRAWL},
	              {"CLIMB", Actor::MovementState::CLIMB},
	              {"MOVEMENTSTATECOUNT", Actor::MovementState::MOVEMENTSTATECOUNT}})
	    .addEnum({{"AIMODE_NONE", Actor::AIMode::AIMODE_NONE},
	              {"AIMODE_SENTRY", Actor::AIMode::AIMODE_SENTRY},
	              {"AIMODE_PATROL", Actor::AIMode::AIMODE_PATROL},
	              {"AIMODE_GOTO", Actor::AIMode::AIMODE_GOTO},
	              {"AIMODE_BRAINHUNT", Actor::AIMode::AIMODE_BRAINHUNT},
	              {"AIMODE_GOLDDIG", Actor::AIMode::AIMODE_GOLDDIG},
	              {"AIMODE_RETURN", Actor::AIMode::AIMODE_RETURN},
	              {"AIMODE_STAY", Actor::AIMode::AIMODE_STAY},
	              {"AIMODE_SCUTTLE", Actor::AIMode::AIMODE_SCUTTLE},
	              {"AIMODE_DELIVER", Actor::AIMode::AIMODE_DELIVER},
	              {"AIMODE_BOMB", Actor::AIMode::AIMODE_BOMB},
	              {"AIMODE_SQUAD", Actor::AIMode::AIMODE_SQUAD},
	              {"AIMODE_COUNT", Actor::AIMode::AIMODE_COUNT}})
	    .addEnum({{"MOVING", Actor::ActionState::MOVING},
	              {"MOVING_FAST", Actor::ActionState::MOVING_FAST},
	              {"FIRING", Actor::ActionState::FIRING},
	              {"ActionStateCount", Actor::ActionState::ActionStateCount}})
	    .addEnum({{"AIMSTILL", Actor::AimState::AIMSTILL},
	              {"AIMUP", Actor::AimState::AIMUP},
	              {"AIMDOWN", Actor::AimState::AIMDOWN},
	              {"AimStateCount", Actor::AimState::AimStateCount}})
	    .addEnum({{"LAT_STILL", Actor::LateralMoveState::LAT_STILL},
	              {"LAT_LEFT", Actor::LateralMoveState::LAT_LEFT},
	              {"LAT_RIGHT", Actor::LateralMoveState::LAT_RIGHT}})
	    .addEnum({{"PROCEEDING", Actor::ObstacleState::PROCEEDING},
	              {"BACKSTEPPING", Actor::ObstacleState::BACKSTEPPING},
	              {"DIGPAUSING", Actor::ObstacleState::DIGPAUSING},
	              {"JUMPING", Actor::ObstacleState::JUMPING},
	              {"SOFTLANDING", Actor::ObstacleState::SOFTLANDING}})
	    .addEnum({{"NOTBLOCKED", Actor::TeamBlockState::NOTBLOCKED},
	              {"BLOCKED", Actor::TeamBlockState::BLOCKED},
	              {"IGNORINGBLOCK", Actor::TeamBlockState::IGNORINGBLOCK},
	              {"FOLLOWWAIT", Actor::TeamBlockState::FOLLOWWAIT}})
	    .addEnum({{"PRIMARYHOTKEY", Actor::ActorHotkeyType::PRIMARYHOTKEY},
	              {"AUXILIARYHOTKEY", Actor::ActorHotkeyType::AUXILIARYHOTKEY},
	              {"ACTORHOTKEYTYPECOUNT", Actor::ActorHotkeyType::ACTORHOTKEYTYPECOUNT}});
}

LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, ADoor) {
	ConcreteTypeLuaClassDefinition(ADoor, Actor)

	    .addProperty("Door", &ADoor::GetDoor, &LuaAdaptersPropertyOwnershipSafetyFaker::ADoorSetDoor)
	    .addProperty("DoorMoveStartSound", &ADoor::GetDoorMoveStartSound, &LuaAdaptersPropertyOwnershipSafetyFaker::ADoorSetDoorMoveStartSound)
	    .addProperty("DoorMoveSound", &ADoor::GetDoorMoveSound, &LuaAdaptersPropertyOwnershipSafetyFaker::ADoorSetDoorMoveSound)
	    .addProperty("DoorDirectionChangeSound", &ADoor::GetDoorDirectionChangeSound, &LuaAdaptersPropertyOwnershipSafetyFaker::ADoorSetDoorDirectionChangeSound)
	    .addProperty("DoorMoveEndSound", &ADoor::GetDoorMoveEndSound, &LuaAdaptersPropertyOwnershipSafetyFaker::ADoorSetDoorMoveEndSound)

	    .addFunction("GetDoorState", &ADoor::GetDoorState)
	    .addFunction("OpenDoor", &ADoor::OpenDoor)
	    .addFunction("CloseDoor", &ADoor::CloseDoor)
	    .addFunction("StopDoor", &ADoor::StopDoor)
	    .addFunction("ResetSensorTimer", &ADoor::ResetSensorTimer)
	    .addFunction("SetClosedByDefault", &ADoor::SetClosedByDefault)

	    .addEnum({{"CLOSED", ADoor::DoorState::CLOSED},
	              {"OPENING", ADoor::DoorState::OPENING},
	              {"OPEN", ADoor::DoorState::OPEN},
	              {"CLOSING", ADoor::DoorState::CLOSING},
	              {"STOPPED", ADoor::DoorState::STOPPED}});
}

LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, AEmitter) {
	ConcreteTypeLuaClassDefinition(AEmitter, Attachable)

	    .addProperty("EmissionSound", &AEmitter::GetEmissionSound, &LuaAdaptersPropertyOwnershipSafetyFaker::AEmitterSetEmissionSound)
	    .addProperty("BurstSound", &AEmitter::GetBurstSound, &LuaAdaptersPropertyOwnershipSafetyFaker::AEmitterSetBurstSound)
	    .addProperty("EndSound", &AEmitter::GetEndSound, &LuaAdaptersPropertyOwnershipSafetyFaker::AEmitterSetEndSound)
	    .addProperty("BurstScale", &AEmitter::GetBurstScale, &AEmitter::SetBurstScale)
	    .addProperty("PlayBurstSound", &AEmitter::GetPlayBurstSound, &AEmitter::SetPlayBurstSound)
	    .addProperty("EmitAngle", &AEmitter::GetEmitAngle, &AEmitter::SetEmitAngle)
	    .addProperty("GetThrottle", &AEmitter::GetThrottle, &AEmitter::SetThrottle)
	    .addProperty("Throttle", &AEmitter::GetThrottle, &AEmitter::SetThrottle)
	    .addProperty("ThrottleFactor", &AEmitter::GetThrottleFactor)
	    .addProperty("NegativeThrottleMultiplier", &AEmitter::GetNegativeThrottleMultiplier, &AEmitter::SetNegativeThrottleMultiplier)
	    .addProperty("PositiveThrottleMultiplier", &AEmitter::GetPositiveThrottleMultiplier, &AEmitter::SetPositiveThrottleMultiplier)
	    .addProperty("BurstSpacing", &AEmitter::GetBurstSpacing, &AEmitter::SetBurstSpacing)
	    .addProperty("BurstDamage", &AEmitter::GetBurstDamage, &AEmitter::SetBurstDamage)
	    .addProperty("EmitterDamageMultiplier", &AEmitter::GetEmitterDamageMultiplier, &AEmitter::SetEmitterDamageMultiplier)
	    .addProperty("EmitCount", &AEmitter::GetEmitCount)
	    .addProperty("EmitCountLimit", &AEmitter::GetEmitCountLimit, &AEmitter::SetEmitCountLimit)
	    .addProperty("EmitDamage", &AEmitter::GetEmitDamage, &AEmitter::SetEmitDamage)
	    .addProperty("EmitOffset", &AEmitter::GetEmitOffset, &AEmitter::SetEmitOffset)
	    .addProperty("Flash", &AEmitter::GetFlash, &LuaAdaptersPropertyOwnershipSafetyFaker::AEmitterSetFlash)
	    .addProperty("FlashScale", &AEmitter::GetFlashScale, &AEmitter::SetFlashScale)
	    .addProperty("TotalParticlesPerMinute", &AEmitter::GetTotalParticlesPerMinute)
	    .addProperty("TotalBurstSize", &AEmitter::GetTotalBurstSize)

	    .addProperty("Emissions", &AEmitter::m_EmissionList, LuaBinding::ReturnStlIterator)

	    .addFunction("IsEmitting", &AEmitter::IsEmitting)
	    .addFunction("WasEmitting", &AEmitter::WasEmitting)
	    .addFunction("EnableEmission", &AEmitter::EnableEmission)
	    .addFunction("GetEmitVector", &AEmitter::GetEmitVector)
	    .addFunction("GetRecoilVector", &AEmitter::GetRecoilVector)
	    .addFunction("EstimateImpulse", &AEmitter::EstimateImpulse)
	    .addFunction("TriggerBurst", &AEmitter::TriggerBurst)
	    .addFunction("IsSetToBurst", &AEmitter::IsSetToBurst)
	    .addFunction("CanTriggerBurst", &AEmitter::CanTriggerBurst)
	    .addFunction("GetScaledThrottle", &AEmitter::GetScaledThrottle)
	    .addFunction("JustStartedEmitting", &AEmitter::JustStartedEmitting);
}

LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, AEJetpack) {
	ConcreteTypeLuaClassDefinition(AEJetpack, AEmitter)

	    .addProperty("JetpackType", &AEJetpack::GetJetpackType, &AEJetpack::SetJetpackType)
	    .addProperty("JetTimeTotal", &AEJetpack::GetJetTimeTotal, &AEJetpack::SetJetTimeTotal)
	    .addProperty("JetTimeLeft", &AEJetpack::GetJetTimeLeft, &AEJetpack::SetJetTimeLeft)
	    .addProperty("JetReplenishRate", &AEJetpack::GetJetReplenishRate, &AEJetpack::SetJetReplenishRate)
	    .addProperty("MinimumFuelRatio", &AEJetpack::GetMinimumFuelRatio, &AEJetpack::SetMinimumFuelRatio)
	    .addProperty("JetAngleRange", &AEJetpack::GetJetAngleRange, &AEJetpack::SetJetAngleRange)
	    .addProperty("CanAdjustAngleWhileFiring", &AEJetpack::GetCanAdjustAngleWhileFiring, &AEJetpack::SetCanAdjustAngleWhileFiring)
	    .addProperty("AdjustsThrottleForWeight", &AEJetpack::GetAdjustsThrottleForWeight, &AEJetpack::SetAdjustsThrottleForWeight)

	    .addEnum({{"Standard", AEJetpack::Standard},
	              {"JumpPack", AEJetpack::JumpPack}});
}

LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, AHuman) {
	ConcreteTypeLuaClassDefinition(AHuman, Actor)

	    .addConstructor<>()

	    .addProperty("Head", &AHuman::GetHead, &LuaAdaptersPropertyOwnershipSafetyFaker::AHumanSetHead)
	    .addProperty("Jetpack", &AHuman::GetJetpack, &LuaAdaptersPropertyOwnershipSafetyFaker::AHumanSetJetpack)
	    .addProperty("FGArm", &AHuman::GetFGArm, &LuaAdaptersPropertyOwnershipSafetyFaker::AHumanSetFGArm)
	    .addProperty("BGArm", &AHuman::GetBGArm, &LuaAdaptersPropertyOwnershipSafetyFaker::AHumanSetBGArm)
	    .addProperty("FGLeg", &AHuman::GetFGLeg, &LuaAdaptersPropertyOwnershipSafetyFaker::AHumanSetFGLeg)
	    .addProperty("BGLeg", &AHuman::GetBGLeg, &LuaAdaptersPropertyOwnershipSafetyFaker::AHumanSetBGLeg)
	    .addProperty("FGFoot", &AHuman::GetFGFoot, &LuaAdaptersPropertyOwnershipSafetyFaker::AHumanSetFGFoot)
	    .addProperty("BGFoot", &AHuman::GetBGFoot, &LuaAdaptersPropertyOwnershipSafetyFaker::AHumanSetBGFoot)
	    .addProperty("MaxWalkPathCrouchShift", &AHuman::GetMaxWalkPathCrouchShift, &AHuman::SetMaxWalkPathCrouchShift)
	    .addProperty("CrouchAmount", &AHuman::GetCrouchAmount)
	    .addProperty("CrouchAmountOverride", &AHuman::GetCrouchAmountOverride, &AHuman::SetCrouchAmountOverride)
	    .addProperty("StrideSound", &AHuman::GetStrideSound, &LuaAdaptersPropertyOwnershipSafetyFaker::AHumanSetStrideSound)
	    .addProperty("UpperBodyState", &AHuman::GetUpperBodyState, &AHuman::SetUpperBodyState)
	    .addProperty("ProneState", &AHuman::GetProneState, &AHuman::SetProneState)
	    .addProperty("ThrowPrepTime", &AHuman::GetThrowPrepTime, &AHuman::SetThrowPrepTime)
	    .addProperty("ThrowProgress", &AHuman::GetThrowProgress)
	    .addProperty("EquippedItem", &AHuman::GetEquippedItem)
	    .addProperty("EquippedBGItem", &AHuman::GetEquippedBGItem)
	    .addProperty("EquippedMass", &AHuman::GetEquippedMass)
	    .addProperty("FirearmIsReady", &AHuman::FirearmIsReady)
	    .addProperty("ThrowableIsReady", &AHuman::ThrowableIsReady)
	    .addProperty("FirearmIsEmpty", &AHuman::FirearmIsEmpty)
	    .addProperty("FirearmNeedsReload", &AHuman::FirearmNeedsReload)
	    .addProperty("FirearmIsSemiAuto", &AHuman::FirearmIsSemiAuto)
	    .addProperty("FirearmActivationDelay", &AHuman::FirearmActivationDelay)
	    .addProperty("IsClimbing", &AHuman::IsClimbing)
	    .addProperty("StrideFrame", &AHuman::StrideFrame)
	    .addProperty("ArmSwingRate", &AHuman::GetArmSwingRate, &AHuman::SetArmSwingRate)
	    .addProperty("DeviceArmSwayRate", &AHuman::GetDeviceArmSwayRate, &AHuman::SetDeviceArmSwayRate)

	    .addFunction("EquipFirearm", &AHuman::EquipFirearm)
	    .addFunction("EquipThrowable", &AHuman::EquipThrowable)
	    .addFunction("EquipDiggingTool", &AHuman::EquipDiggingTool)
	    .addFunction("EquipShield", &AHuman::EquipShield)
	    .addFunction("EquipShieldInBGArm", &LuaAdaptersAHuman::EquipShieldInBGArm)
	    .addFunction("EquipDeviceInGroup", &AHuman::EquipDeviceInGroup)
	    .addFunction("EquipNamedDevice", (bool(AHuman::*)(const std::string&, bool)) & AHuman::EquipNamedDevice)
	    .addFunction("EquipNamedDevice", (bool(AHuman::*)(const std::string&, const std::string&, bool)) & AHuman::EquipNamedDevice)
	    .addFunction("EquipLoadedFirearmInGroup", &AHuman::EquipLoadedFirearmInGroup)
	    .addFunction("UnequipFGArm", &AHuman::UnequipFGArm)
	    .addFunction("UnequipBGArm", &AHuman::UnequipBGArm)
	    .addFunction("UnequipArms", &AHuman::UnequipArms)
	    .addFunction("ReloadFirearms", &LuaAdaptersAHuman::ReloadFirearms)
	    .addFunction("ReloadFirearms", &AHuman::ReloadFirearms)
	    .addFunction("FirearmsAreReloading", &AHuman::FirearmsAreReloading)
	    .addFunction("IsWithinRange", &AHuman::IsWithinRange)
	    .addFunction("Look", &AHuman::Look)
	    .addFunction("LookForGold", &AHuman::LookForGold)
	    .addFunction("LookForMOs", &AHuman::LookForMOs)
	    .addFunction("GetLimbPath", &AHuman::GetLimbPath)
	    .addFunction("GetLimbPathTravelSpeed", &AHuman::GetLimbPathTravelSpeed)
	    .addFunction("SetLimbPathTravelSpeed", &AHuman::SetLimbPathTravelSpeed)
	    .addFunction("GetLimbPathPushForce", &AHuman::GetLimbPathPushForce)
	    .addFunction("SetLimbPathPushForce", &AHuman::SetLimbPathPushForce)
	    .addFunction("GetRotAngleTarget", &AHuman::GetRotAngleTarget)
	    .addFunction("SetRotAngleTarget", &AHuman::SetRotAngleTarget)
	    .addFunction("GetWalkAngle", &AHuman::GetWalkAngle)
	    .addFunction("SetWalkAngle", &AHuman::SetWalkAngle)

	    .addEnum({{"WEAPON_READY", AHuman::UpperBodyState::WEAPON_READY},
	              {"AIMING_SHARP", AHuman::UpperBodyState::AIMING_SHARP},
	              {"HOLSTERING_BACK", AHuman::UpperBodyState::HOLSTERING_BACK},
	              {"HOLSTERING_BELT", AHuman::UpperBodyState::HOLSTERING_BELT},
	              {"DEHOLSTERING_BACK", AHuman::UpperBodyState::DEHOLSTERING_BACK},
	              {"DEHOLSTERING_BELT", AHuman::UpperBodyState::DEHOLSTERING_BELT},
	              {"THROWING_PREP", AHuman::UpperBodyState::THROWING_PREP},
	              {"THROWING_RELEASE", AHuman::UpperBodyState::THROWING_RELEASE}})
	    .addEnum({{"NOTPRONE", AHuman::ProneState::NOTPRONE},
	              {"GOPRONE", AHuman::ProneState::GOPRONE},
	              {"PRONE", AHuman::ProneState::LAYINGPRONE},
	              {"PRONESTATECOUNT", AHuman::ProneState::PRONESTATECOUNT}})
	    .addEnum({{"FGROUND", AHuman::Layer::FGROUND},
	              {"BGROUND", AHuman::Layer::BGROUND}})
	    .addEnum({{"STILL", AHuman::DeviceHandlingState::STILL},
	              {"POINTING", AHuman::DeviceHandlingState::POINTING},
	              {"SCANNING", AHuman::DeviceHandlingState::SCANNING},
	              {"AIMING", AHuman::DeviceHandlingState::AIMING},
	              {"FIRING", AHuman::DeviceHandlingState::FIRING},
	              {"THROWING", AHuman::DeviceHandlingState::THROWING},
	              {"DIGGING", AHuman::DeviceHandlingState::DIGGING}})
	    .addEnum({{"NOSWEEP", AHuman::SweepState::NOSWEEP},
	              {"SWEEPINGUP", AHuman::SweepState::SWEEPINGUP},
	              {"SWEEPUPPAUSE", AHuman::SweepState::SWEEPUPPAUSE},
	              {"SWEEPINGDOWN", AHuman::SweepState::SWEEPINGDOWN},
	              {"SWEEPDOWNPAUSE", AHuman::SweepState::SWEEPDOWNPAUSE}})
	    .addEnum({{"NOTDIGGING", AHuman::DigState::NOTDIGGING},
	              {"PREDIG", AHuman::DigState::PREDIG},
	              {"STARTDIG", AHuman::DigState::STARTDIG},
	              {"TUNNELING", AHuman::DigState::TUNNELING},
	              {"FINISHINGDIG", AHuman::DigState::FINISHINGDIG},
	              {"PAUSEDIGGER", AHuman::DigState::PAUSEDIGGER}})
	    .addEnum({{"NOTJUMPING", AHuman::JumpState::NOTJUMPING},
	              {"FORWARDJUMP", AHuman::JumpState::FORWARDJUMP},
	              {"PREJUMP", AHuman::JumpState::PREUPJUMP},
	              {"UPJUMP", AHuman::JumpState::UPJUMP},
	              {"APEXJUMP", AHuman::JumpState::APEXJUMP},
	              {"LANDJUMP", AHuman::JumpState::LANDJUMP}});
}

LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, Arm) {
	ConcreteTypeLuaClassDefinition(Arm, Attachable)

	    .addProperty("MaxLength", &Arm::GetMaxLength)
	    .addProperty("MoveSpeed", &Arm::GetMoveSpeed, &Arm::SetMoveSpeed)

	    .addProperty("HandIdleOffset", &Arm::GetHandIdleOffset, &Arm::SetHandIdleOffset)

	    .addProperty("HandPos", &Arm::GetHandPos, &Arm::SetHandPos)
	    .addProperty("HasAnyHandTargets", &Arm::HasAnyHandTargets)
	    .addProperty("NumberOfHandTargets", &Arm::GetNumberOfHandTargets)
	    .addProperty("NextHandTargetDescription", &Arm::GetNextHandTargetDescription)
	    .addProperty("NextHandTargetPosition", &Arm::GetNextHandTargetPosition)
	    .addProperty("HandHasReachedCurrentTarget", &Arm::GetHandHasReachedCurrentTarget)

	    .addProperty("GripStrength", &Arm::GetGripStrength, &Arm::SetGripStrength)
	    .addProperty("ThrowStrength", &Arm::GetThrowStrength, &Arm::SetThrowStrength)

	    .addProperty("HeldDevice", &Arm::GetHeldDevice, &LuaAdaptersPropertyOwnershipSafetyFaker::ArmSetHeldDevice)
	    .addProperty("SupportedHeldDevice", &Arm::GetHeldDeviceThisArmIsTryingToSupport)

	    .addFunction("AddHandTarget", (void(Arm::*)(const std::string& description, const Vector& handTargetPositionToAdd)) & Arm::AddHandTarget)
	    .addFunction("AddHandTarget", (void(Arm::*)(const std::string& description, const Vector& handTargetPositionToAdd, float delayAtTarget)) & Arm::AddHandTarget)
	    .addFunction("RemoveNextHandTarget", &Arm::RemoveNextHandTarget)
	    .addFunction("ClearHandTargets", &Arm::ClearHandTargets);
}

LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, Attachable) {
	ConcreteTypeLuaClassDefinition(Attachable, MOSRotating)

	    .addProperty("ParentOffset", &Attachable::GetParentOffset, &Attachable::SetParentOffset)
	    .addProperty("JointStrength", &Attachable::GetJointStrength, &Attachable::SetJointStrength)
	    .addProperty("JointStiffness", &Attachable::GetJointStiffness, &Attachable::SetJointStiffness)
	    .addProperty("JointOffset", &Attachable::GetJointOffset, &Attachable::SetJointOffset)
	    .addProperty("JointPos", &Attachable::GetJointPos)
	    .addProperty("DeleteWhenRemovedFromParent", &Attachable::GetDeleteWhenRemovedFromParent, &Attachable::SetDeleteWhenRemovedFromParent)
	    .addProperty("GibWhenRemovedFromParent", &Attachable::GetGibWhenRemovedFromParent, &Attachable::SetGibWhenRemovedFromParent)
	    .addProperty("ApplyTransferredForcesAtOffset", &Attachable::GetApplyTransferredForcesAtOffset, &Attachable::SetApplyTransferredForcesAtOffset)
	    .addProperty("BreakWound", &Attachable::GetBreakWound, &LuaAdaptersPropertyOwnershipSafetyFaker::AttachableSetBreakWound)
	    .addProperty("ParentBreakWound", &Attachable::GetParentBreakWound, &LuaAdaptersPropertyOwnershipSafetyFaker::AttachableSetParentBreakWound)
	    .addProperty("InheritsHFlipped", &Attachable::InheritsHFlipped, &Attachable::SetInheritsHFlipped)
	    .addProperty("InheritsRotAngle", &Attachable::InheritsRotAngle, &Attachable::SetInheritsRotAngle)
	    .addProperty("InheritedRotAngleOffset", &Attachable::GetInheritedRotAngleOffset, &Attachable::SetInheritedRotAngleOffset)
	    .addProperty("AtomSubgroupID", &Attachable::GetAtomSubgroupID)
	    .addProperty("CollidesWithTerrainWhileAttached", &Attachable::GetCollidesWithTerrainWhileAttached, &Attachable::SetCollidesWithTerrainWhileAttached)
	    .addProperty("IgnoresParticlesWhileAttached", &Attachable::GetIgnoresParticlesWhileAttached, &Attachable::SetIgnoresParticlesWhileAttached)
	    .addProperty("CanCollideWithTerrain", &Attachable::CanCollideWithTerrain)
	    .addProperty("DrawnAfterParent", &Attachable::IsDrawnAfterParent, &Attachable::SetDrawnAfterParent)
	    .addProperty("InheritsFrame", &Attachable::InheritsFrame, &Attachable::SetInheritsFrame)
	    .addProperty("InheritsVelWhenDetached", &Attachable::InheritsVelocityWhenDetached, &Attachable::SetInheritsVelocityWhenDetached)
	    .addProperty("InheritsAngularVelWhenDetached", &Attachable::InheritsAngularVelocityWhenDetached, &Attachable::SetInheritsAngularVelocityWhenDetached)

	    .addFunction("IsAttached", &Attachable::IsAttached)
	    .addFunction("IsAttachedTo", &Attachable::IsAttachedTo)

	    .addFunction("RemoveFromParent", &LuaAdaptersAttachable::RemoveFromParent1, LuaBinding::AdoptResult)
	    .addFunction("RemoveFromParent", &LuaAdaptersAttachable::RemoveFromParent2, LuaBinding::AdoptResult);
}

LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, Deployment) {
	AbstractTypeLuaClassDefinition(Deployment, SceneObject)

	    .addProperty("ID", &Deployment::GetID)
	    .addProperty("HFlipped", &Deployment::IsHFlipped)
	    .addProperty("SpawnRadius", &Deployment::GetSpawnRadius)

	    .addFunction("GetLoadoutName", &Deployment::GetLoadoutName)
	    .addFunction("CreateDeployedActor", (Actor * (Deployment::*)()) & Deployment::CreateDeployedActor, LuaBinding::AdoptResult)
	    .addFunction("CreateDeployedObject", (SceneObject * (Deployment::*)()) & Deployment::CreateDeployedObject, LuaBinding::AdoptResult);
}

LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, Emission) {
	AbstractTypeLuaClassDefinition(Emission, Entity)

	    .addProperty("ParticlesPerMinute", &Emission::GetRate, &Emission::SetRate)
	    .addProperty("MinVelocity", &Emission::GetMinVelocity, &Emission::SetMinVelocity)
	    .addProperty("MaxVelocity", &Emission::GetMaxVelocity, &Emission::SetMaxVelocity)
	    .addProperty("PushesEmitter", &Emission::PushesEmitter, &Emission::SetPushesEmitter)
	    .addProperty("LifeVariation", &Emission::GetLifeVariation, &Emission::SetLifeVariation)
	    .addProperty("BurstSize", &Emission::GetBurstSize, &Emission::SetBurstSize)
	    .addProperty("Spread", &Emission::GetSpread, &Emission::SetSpread)
	    .addProperty("Offset", &Emission::GetOffset, &Emission::SetOffset)
	    .addProperty("ParticleCount", &Emission::GetParticleCount, &Emission::SetParticleCount)
	    .addProperty("InheritsVel", &Emission::InheritsVelocity, &Emission::SetInheritsVelocity)
	    .addProperty("InheritsAngularVel", &Emission::InheritsAngularVelocity, &Emission::SetInheritsAngularVelocity)

	    .addFunction("ResetEmissionTimers", &Emission::ResetEmissionTimers);
}

LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, Gib) {
	registry.Class<Gib>("Gib")

	    .addProperty("ParticlePreset", &Gib::GetParticlePreset, &Gib::SetParticlePreset)
	    .addProperty("MinVelocity", &Gib::GetMinVelocity, &Gib::SetMinVelocity)
	    .addProperty("MaxVelocity", &Gib::GetMaxVelocity, &Gib::SetMaxVelocity)
	    .addProperty("SpreadMode", &Gib::GetSpreadMode, &Gib::SetSpreadMode)

	    .addReadWriteMember("Offset", &Gib::m_Offset)
	    .addReadWriteMember("Count", &Gib::m_Count)
	    .addReadWriteMember("Spread", &Gib::m_Spread)
	    .addReadWriteMember("LifeVariation", &Gib::m_LifeVariation)
	    .addReadWriteMember("InheritsVel", &Gib::m_InheritsVel)
	    .addReadWriteMember("InheritsAngularVel", &Gib::m_InheritsAngularVel)
	    .addReadWriteMember("IgnoresTeamHits", &Gib::m_IgnoresTeamHits)

	    .addEnum({{"SpreadRandom", Gib::SpreadMode::SpreadRandom},
	              {"SpreadEven", Gib::SpreadMode::SpreadEven},
	              {"SpreadSpiral", Gib::SpreadMode::SpreadSpiral}});
}

LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, GlobalScript) {
	AbstractTypeLuaClassDefinition(GlobalScript, Entity)

	    .addFunction("Deactivate", &LuaAdaptersGlobalScript::Deactivate);
}

LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, HDFirearm) {
	ConcreteTypeLuaClassDefinition(HDFirearm, HeldDevice)

	    .addProperty("ReloadEndOffset", &HDFirearm::GetReloadEndOffset, &HDFirearm::SetReloadEndOffset)
	    .addProperty("EjectionPos", &HDFirearm::GetEjectionPos)
	    .addProperty("EjectionOffset", &HDFirearm::GetEjectionOffset, &HDFirearm::SetEjectionOffset)
	    .addProperty("RateOfFire", &HDFirearm::GetRateOfFire, &HDFirearm::SetRateOfFire)
	    .addProperty("MSPerRound", &HDFirearm::GetMSPerRound)
	    .addProperty("FullAuto", &HDFirearm::IsFullAuto, &HDFirearm::SetFullAuto)
	    .addProperty("Reloadable", &HDFirearm::IsReloadable, &HDFirearm::SetReloadable)
	    .addProperty("DualReloadable", &HDFirearm::IsDualReloadable, &HDFirearm::SetDualReloadable)
	    .addProperty("OneHandedReloadTimeMultiplier", &HDFirearm::GetOneHandedReloadTimeMultiplier, &HDFirearm::SetOneHandedReloadTimeMultiplier)
	    .addProperty("ReloadAngle", &HDFirearm::GetReloadAngle, &HDFirearm::SetReloadAngle)
	    .addProperty("OneHandedReloadAngle", &HDFirearm::GetOneHandedReloadAngle, &HDFirearm::SetOneHandedReloadAngle)
	    .addProperty("CurrentReloadAngle", &HDFirearm::GetCurrentReloadAngle)
	    .addProperty("RoundInMagCount", &HDFirearm::GetRoundInMagCount)
	    .addProperty("RoundInMagCapacity", &HDFirearm::GetRoundInMagCapacity)
	    .addProperty("Magazine", &HDFirearm::GetMagazine, &LuaAdaptersPropertyOwnershipSafetyFaker::HDFirearmSetMagazine)
	    .addProperty("Flash", &HDFirearm::GetFlash, &LuaAdaptersPropertyOwnershipSafetyFaker::HDFirearmSetFlash)
	    .addProperty("PreFireSound", &HDFirearm::GetPreFireSound, &LuaAdaptersPropertyOwnershipSafetyFaker::HDFirearmSetPreFireSound)
	    .addProperty("FireSound", &HDFirearm::GetFireSound, &LuaAdaptersPropertyOwnershipSafetyFaker::HDFirearmSetFireSound)
	    .addProperty("FireEchoSound", &HDFirearm::GetFireEchoSound, &LuaAdaptersPropertyOwnershipSafetyFaker::HDFirearmSetFireEchoSound)
	    .addProperty("ActiveSound", &HDFirearm::GetActiveSound, &LuaAdaptersPropertyOwnershipSafetyFaker::HDFirearmSetActiveSound)
	    .addProperty("DeactivationSound", &HDFirearm::GetDeactivationSound, &LuaAdaptersPropertyOwnershipSafetyFaker::HDFirearmSetDeactivationSound)
	    .addProperty("EmptySound", &HDFirearm::GetEmptySound, &LuaAdaptersPropertyOwnershipSafetyFaker::HDFirearmSetEmptySound)
	    .addProperty("ReloadStartSound", &HDFirearm::GetReloadStartSound, &LuaAdaptersPropertyOwnershipSafetyFaker::HDFirearmSetReloadStartSound)
	    .addProperty("ReloadEndSound", &HDFirearm::GetReloadEndSound, &LuaAdaptersPropertyOwnershipSafetyFaker::HDFirearmSetReloadEndSound)
	    .addProperty("ActivationDelay", &HDFirearm::GetActivationDelay, &HDFirearm::SetActivationDelay)
	    .addProperty("DeactivationDelay", &HDFirearm::GetDeactivationDelay, &HDFirearm::SetDeactivationDelay)
	    .addProperty("BaseReloadTime", &HDFirearm::GetBaseReloadTime, &HDFirearm::SetBaseReloadTime)
	    .addProperty("ReloadTime", &HDFirearm::GetReloadTime)
	    .addProperty("ReloadProgress", &HDFirearm::GetReloadProgress, &HDFirearm::SetReloadProgress)
	    .addProperty("ShakeRange", &HDFirearm::GetShakeRange, &HDFirearm::SetShakeRange)
	    .addProperty("SharpShakeRange", &HDFirearm::GetSharpShakeRange, &HDFirearm::SetSharpShakeRange)
	    .addProperty("NoSupportFactor", &HDFirearm::GetNoSupportFactor, &HDFirearm::SetNoSupportFactor)
	    .addProperty("ParticleSpreadRange", &HDFirearm::GetParticleSpreadRange, &HDFirearm::SetParticleSpreadRange)
	    .addProperty("ShellVelVariation", &HDFirearm::GetShellVelVariation, &HDFirearm::SetShellVelVariation)
	    .addProperty("FiredOnce", &HDFirearm::FiredOnce)
	    .addProperty("FiredFrame", &HDFirearm::FiredFrame)
	    .addProperty("CanFire", &HDFirearm::CanFire)
	    .addProperty("RoundsFired", &HDFirearm::RoundsFired)
	    .addProperty("IsAnimatedManually", &HDFirearm::IsAnimatedManually, &HDFirearm::SetAnimatedManually)
	    .addProperty("RecoilTransmission", &HDFirearm::GetJointStiffness, &HDFirearm::SetJointStiffness)

	    .addFunction("GetReloadTimer", &HDFirearm::GetReloadTimer)
	    .addFunction("GetAIFireVel", &HDFirearm::GetAIFireVel)
	    .addFunction("GetAIBulletLifeTime", &HDFirearm::GetAIBulletLifeTime)
	    .addFunction("GetBulletAccScalar", &HDFirearm::GetBulletAccScalar)
	    .addFunction("GetAIBlastRadius", &HDFirearm::GetAIBlastRadius)
	    .addFunction("GetAIPenetration", &HDFirearm::GetAIPenetration)
	    .addFunction("CompareTrajectories", &HDFirearm::CompareTrajectories)
	    .addFunction("GetNextMagazineName", &HDFirearm::GetNextMagazineName)
	    .addFunction("SetNextMagazineName", &HDFirearm::SetNextMagazineName);
}

LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, HeldDevice) {
	ConcreteTypeLuaClassDefinition(HeldDevice, Attachable)

	    .addProperty("SupportPos", &HeldDevice::GetSupportPos)
	    .addProperty("MagazinePos", &HeldDevice::GetMagazinePos)
	    .addProperty("MuzzlePos", &HeldDevice::GetMuzzlePos)
	    .addProperty("MuzzleOffset", &HeldDevice::GetMuzzleOffset, &HeldDevice::SetMuzzleOffset)
	    .addProperty("StanceOffset", &HeldDevice::GetStanceOffset, &HeldDevice::SetStanceOffset)
	    .addProperty("SharpStanceOffset", &HeldDevice::GetSharpStanceOffset, &HeldDevice::SetSharpStanceOffset)
	    .addProperty("SharpLength", &HeldDevice::GetSharpLength, &HeldDevice::SetSharpLength)
	    .addProperty("SharpLength", &HeldDevice::GetSharpLength, &HeldDevice::SetSharpLength)
	    .addProperty("Supportable", &HeldDevice::IsSupportable, &HeldDevice::SetSupportable)
	    .addProperty("SupportOffset", &HeldDevice::GetSupportOffset, &HeldDevice::SetSupportOffset)
	    .addProperty("UseSupportOffsetWhileReloading", &HeldDevice::GetUseSupportOffsetWhileReloading, &HeldDevice::SetUseSupportOffsetWhileReloading)
	    .addProperty("HasPickupLimitations", &HeldDevice::HasPickupLimitations)
	    .addProperty("UnPickupable", &HeldDevice::IsUnPickupable, &HeldDevice::SetUnPickupable)
	    .addProperty("GripStrengthMultiplier", &HeldDevice::GetGripStrengthMultiplier, &HeldDevice::SetGripStrengthMultiplier)
	    .addProperty("Supported", &HeldDevice::GetSupported, &HeldDevice::SetSupported)
	    .addProperty("GetsHitByMOsWhenHeld", &HeldDevice::GetsHitByMOsWhenHeld, &HeldDevice::SetGetsHitByMOsWhenHeld)
	    .addProperty("VisualRecoilMultiplier", &HeldDevice::GetVisualRecoilMultiplier, &HeldDevice::SetVisualRecoilMultiplier)

	    .addFunction("IsBeingHeld", &HeldDevice::IsBeingHeld)
	    .addFunction("IsWeapon", &HeldDevice::IsWeapon)
	    .addFunction("IsTool", &HeldDevice::IsTool)
	    .addFunction("IsShield", &HeldDevice::IsShield)
	    .addFunction("IsDualWieldable", &HeldDevice::IsDualWieldable)
	    .addFunction("SetDualWieldable", &HeldDevice::SetDualWieldable)
	    .addFunction("IsOneHanded", &HeldDevice::IsOneHanded)
	    .addFunction("SetOneHanded", &HeldDevice::SetOneHanded)
	    .addFunction("Activate", &HeldDevice::Activate)
	    .addFunction("Deactivate", &HeldDevice::Deactivate)
	    .addFunction("IsActivated", &HeldDevice::IsActivated)
	    .addFunction("ActivateHotkeyAction", &HeldDevice::ActivateHotkeyAction)
	    .addFunction("DeactivateHotkeyAction", &HeldDevice::DeactivateHotkeyAction)
	    .addFunction("HotkeyActionIsActivated", &HeldDevice::HotkeyActionIsActivated)
	    .addFunction("Reload", &HeldDevice::Reload)
	    .addFunction("IsReloading", &HeldDevice::IsReloading)
	    .addFunction("DoneReloading", &HeldDevice::DoneReloading)
	    .addFunction("NeedsReloading", &HeldDevice::NeedsReloading)
	    .addFunction("IsFull", &HeldDevice::IsFull)
	    .addFunction("IsEmpty", &HeldDevice::IsEmpty)
	    .addFunction("IsPickupableBy", &HeldDevice::IsPickupableBy)
	    .addFunction("AddPickupableByPresetName", &HeldDevice::AddPickupableByPresetName)
	    .addFunction("RemovePickupableByPresetName", &HeldDevice::RemovePickupableByPresetName)

	    .addEnum({{"PRIMARYHOTKEY", HeldDeviceHotkeyType::PRIMARYHOTKEY},
	              {"AUXILIARYHOTKEY", HeldDeviceHotkeyType::AUXILIARYHOTKEY},
	              {"HELDDEVICEHOTKEYTYPECOUNT", HeldDeviceHotkeyType::HELDDEVICEHOTKEYTYPECOUNT}});
}

LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, Leg) {
	ConcreteTypeLuaClassDefinition(Leg, Attachable)

	    .addProperty("Foot", &Leg::GetFoot, &LuaAdaptersPropertyOwnershipSafetyFaker::LegSetFoot)
	    .addProperty("MoveSpeed", &Leg::GetMoveSpeed, &Leg::SetMoveSpeed);
}

LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, LimbPath) {
	registry.Class<LimbPath>("LimbPath")

	    .addProperty("StartOffset", &LimbPath::GetStartOffset, &LimbPath::SetStartOffset)
	    .addProperty("SegmentCount", &LimbPath::GetSegCount)
	    .addProperty("BaseTravelSpeedMultiplier", &LimbPath::GetBaseTravelSpeedMultiplier, &LimbPath::SetBaseTravelSpeedMultiplier)
	    .addProperty("TravelSpeed", &LimbPath::GetTravelSpeed, &LimbPath::SetTravelSpeed)
	    .addProperty("PushForce", &LimbPath::GetPushForce, &LimbPath::SetPushForce)

	    .addFunction("GetSegment", &LimbPath::GetSegment);
}

LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, Magazine) {
	ConcreteTypeLuaClassDefinition(Magazine, Attachable)

	    .addProperty("NextRound", &Magazine::GetNextRound)
	    .addProperty("RoundCount", &Magazine::GetRoundCount, &Magazine::SetRoundCount)
	    .addProperty("IsEmpty", &Magazine::IsEmpty)
	    .addProperty("IsFull", &Magazine::IsFull)
	    .addProperty("IsOverHalfFull", &Magazine::IsOverHalfFull)
	    .addProperty("Capacity", &Magazine::GetCapacity)
	    .addProperty("Discardable", &Magazine::IsDiscardable);
}

LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, Material) {
	registry.Class<Material, Entity>("Material")

	    .addProperty("ID", &Material::GetIndex)
	    .addProperty("Restitution", &Material::GetRestitution)
	    .addProperty("Bounce", &Material::GetRestitution)
	    .addProperty("Friction", &Material::GetFriction)
	    .addProperty("Stickiness", &Material::GetStickiness)
	    .addProperty("Strength", &Material::GetIntegrity)
	    .addProperty("StructuralIntegrity", &Material::GetIntegrity)
	    .addProperty("DensityKGPerVolumeL", &Material::GetVolumeDensity)
	    .addProperty("DensityKGPerPixel", &Material::GetPixelDensity)
	    .addProperty("SettleMaterial", &Material::GetSettleMaterial)
	    .addProperty("SpawnMaterial", &Material::GetSpawnMaterial)
	    .addProperty("TransformsInto", &Material::GetSpawnMaterial)
	    .addProperty("IsScrap", &Material::IsScrap)

	    .addFunction("GetColorIndex", &Material::GetColorIndex);
}

LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, MetaPlayer) {
	registry.Class<MetaPlayer>("MetaPlayer")

	    .addConstructor<>()

	    .addProperty("NativeTechModule", &MetaPlayer::GetNativeTechModule)
	    .addProperty("ForeignCostMultiplier", &MetaPlayer::GetForeignCostMultiplier)
	    .addProperty("NativeCostMultiplier", &MetaPlayer::GetNativeCostMultiplier)
	    .addProperty("InGamePlayer", &MetaPlayer::GetInGamePlayer)
	    .addProperty("BrainPoolCount", &MetaPlayer::GetBrainPoolCount, &MetaPlayer::SetBrainPoolCount)

	    .addFunction("ChangeBrainPoolCount", &MetaPlayer::ChangeBrainPoolCount);
}

LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, MOPixel) {
	ConcreteTypeLuaClassDefinition(MOPixel, MovableObject)

	    .addProperty("TrailLength", &MOPixel::GetTrailLength, &MOPixel::SetTrailLength)
	    .addProperty("Staininess", &MOPixel::GetStaininess, &MOPixel::SetStaininess)

	    .addFunction("GetColorIndex", &MOPixel::GetColorIndex)
	    .addFunction("SetColorIndex", &MOPixel::SetColorIndex);
}

LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, MOSParticle) {
	ConcreteTypeLuaClassDefinition(MOSParticle, MOSprite);
}

LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, MOSprite) {
	AbstractTypeLuaClassDefinition(MOSprite, MovableObject)

	    .addProperty("Diameter", &MOSprite::GetDiameter)
	    .addProperty("BoundingBox", &MOSprite::GetBoundingBox)
	    .addProperty("FrameCount", &MOSprite::GetFrameCount)
	    .addProperty("SpriteOffset", &MOSprite::GetSpriteOffset, &MOSprite::SetSpriteOffset)
	    .addProperty("HFlipped", &MOSprite::IsHFlipped, &MOSprite::SetHFlipped)
	    .addProperty("ForcedHFlip", &MOSprite::GetForcedHFlip, &MOSprite::SetForcedHFlip)
	    .addProperty("FlipFactor", &MOSprite::GetFlipFactor)
	    .addProperty("RotAngle", &MOSprite::GetRotAngle, &MOSprite::SetRotAngle)
	    .addProperty("PrevRotAngle", &MOSprite::GetPrevRotAngle)
	    .addProperty("AngularVel", &MOSprite::GetAngularVel, &MOSprite::SetAngularVel)
	    .addProperty("Frame", &MOSprite::GetFrame, &MOSprite::SetFrame)
	    .addProperty("SpriteAnimMode", &MOSprite::GetSpriteAnimMode, &MOSprite::SetSpriteAnimMode)
	    .addProperty("SpriteAnimDuration", &MOSprite::GetSpriteAnimDuration, &MOSprite::SetSpriteAnimDuration)

	    .addFunction("SetNextFrame", &MOSprite::SetNextFrame)
	    .addFunction("IsTooFast", &MOSprite::IsTooFast)
	    .addFunction("IsOnScenePoint", &MOSprite::IsOnScenePoint)
	    .addFunction("RotateOffset", &MOSprite::RotateOffset)
	    .addFunction("UnRotateOffset", &MOSprite::UnRotateOffset)
	    .addFunction("FacingAngle", &MOSprite::FacingAngle)
	    .addFunction("GetSpriteWidth", &MOSprite::GetSpriteWidth)
	    .addFunction("GetSpriteHeight", &MOSprite::GetSpriteHeight)
	    .addFunction("GetIconWidth", &MOSprite::GetIconWidth)
	    .addFunction("GetIconHeight", &MOSprite::GetIconHeight)
	    .addFunction("SetEntryWound", &MOSprite::SetEntryWound)
	    .addFunction("SetExitWound", &MOSprite::SetExitWound)
	    .addFunction("GetEntryWoundPresetName", &MOSprite::GetEntryWoundPresetName)
	    .addFunction("GetExitWoundPresetName", &MOSprite::GetExitWoundPresetName)
	    .addFunction("GetSpritePixelIndex", &MOSprite::GetSpritePixelIndex)
	    .addFunction("SetSpritePixelIndex", &MOSprite::SetSpritePixelIndex)
	    .addFunction("GetAllSpritePixelPositions", &MOSprite::GetAllSpritePixelPositions, LuaBinding::ReturnStlIterator)
	    .addFunction("GetAllVisibleSpritePixelPositions", &MOSprite::GetAllVisibleSpritePixelPositions, LuaBinding::ReturnStlIterator)
	    .addFunction("SetAllSpritePixelIndexes", &MOSprite::SetAllSpritePixelIndexes)
	    .addFunction("SetAllVisibleSpritePixelIndexes", &MOSprite::SetAllVisibleSpritePixelIndexes)

	    .addEnum({{"NOANIM", SpriteAnimMode::NOANIM},
	              {"ALWAYSLOOP", SpriteAnimMode::ALWAYSLOOP},
	              {"ALWAYSRANDOM", SpriteAnimMode::ALWAYSRANDOM},
	              {"ALWAYSPINGPONG", SpriteAnimMode::ALWAYSPINGPONG},
	              {"LOOPWHENACTIVE", SpriteAnimMode::LOOPWHENACTIVE},
	              {"LOOPWHENOPENCLOSE", SpriteAnimMode::LOOPWHENOPENCLOSE},
	              {"PINGPONGOPENCLOSE", SpriteAnimMode::PINGPONGOPENCLOSE},
	              {"OVERLIFETIME", SpriteAnimMode::OVERLIFETIME},
	              {"ONCOLLIDE", SpriteAnimMode::ONCOLLIDE}});
}

LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, MOSRotating) {
	ConcreteTypeLuaClassDefinition(MOSRotating, MOSprite)

	    //.addProperty("Material", &MOSRotating::GetMaterial)
	    .addProperty("IndividualRadius", &MOSRotating::GetIndividualRadius)
	    .addProperty("IndividualDiameter", &MOSRotating::GetIndividualDiameter)
	    .addProperty("IndividualMass", &MOSRotating::GetIndividualMass)
	    .addProperty("RecoilForce", &MOSRotating::GetRecoilForce)
	    .addProperty("RecoilOffset", &MOSRotating::GetRecoilOffset)
	    .addProperty("TravelImpulse", &MOSRotating::GetTravelImpulse, &MOSRotating::SetTravelImpulse)
	    .addProperty("GibWoundLimit", (int(MOSRotating::*)() const) & MOSRotating::GetGibWoundLimit, &MOSRotating::SetGibWoundLimit)
	    .addProperty("GibSound", &MOSRotating::GetGibSound, &LuaAdaptersPropertyOwnershipSafetyFaker::MOSRotatingSetGibSound)
	    .addProperty("GibImpulseLimit", &MOSRotating::GetGibImpulseLimit, &MOSRotating::SetGibImpulseLimit)
	    .addProperty("WoundCountAffectsImpulseLimitRatio", &MOSRotating::GetWoundCountAffectsImpulseLimitRatio)
	    .addProperty("GibAtEndOfLifetime", &MOSRotating::GetGibAtEndOfLifetime, &MOSRotating::SetGibAtEndOfLifetime)
	    .addProperty("DamageMultiplier", &MOSRotating::GetDamageMultiplier, &MOSRotating::SetDamageMultiplier)
	    .addProperty("WoundCount", (int(MOSRotating::*)() const) & MOSRotating::GetWoundCount)
	    .addProperty("OrientToVel", &MOSRotating::GetOrientToVel, &MOSRotating::SetOrientToVel)

	    .addProperty("Attachables", &MOSRotating::m_Attachables, LuaBinding::ReturnStlIterator)
	    .addProperty("Wounds", &MOSRotating::m_Wounds, LuaBinding::ReturnStlIterator)
	    .addProperty("Gibs", &MOSRotating::m_Gibs, LuaBinding::ReturnStlIterator)

	    .addFunction("AddRecoil", &MOSRotating::AddRecoil)
	    .addFunction("SetRecoil", &MOSRotating::SetRecoil)
	    .addFunction("IsRecoiled", &MOSRotating::IsRecoiled)
	    .addFunction("EnableDeepCheck", &MOSRotating::EnableDeepCheck)
	    .addFunction("ForceDeepCheck", &MOSRotating::ForceDeepCheck)
	    .addFunction("GibThis", &MOSRotating::GibThis)
	    .addFunction("MoveOutOfTerrain", &MOSRotating::MoveOutOfTerrain)
	    .addFunction("FlashWhite", &MOSRotating::FlashWhite)
	    .addFunction("GetGibWoundLimit", (int(MOSRotating::*)() const) & MOSRotating::GetGibWoundLimit)
	    .addFunction("GetGibWoundLimit", (int(MOSRotating::*)(bool positiveDamage, bool negativeDamage, bool noDamage) const) & MOSRotating::GetGibWoundLimit)
	    .addFunction("GetWoundCount", (int(MOSRotating::*)() const) & MOSRotating::GetWoundCount)
	    .addFunction("GetWoundCount", (int(MOSRotating::*)(bool positiveDamage, bool negativeDamage, bool noDamage) const) & MOSRotating::GetWoundCount)
	    .addFunction("GetWounds", &LuaAdaptersMOSRotating::GetWounds1, LuaBinding::AdoptResult, LuaBinding::ReturnStlIterator)
	    .addFunction("GetWounds", &LuaAdaptersMOSRotating::GetWounds2, LuaBinding::AdoptResult, LuaBinding::ReturnStlIterator)
	    .addFunction("AddWound", &MOSRotating::AddWound, LuaBinding::AdoptArg<2>)
	    .addFunction("AddWound", &MOSRotating::AddWoundExt, LuaBinding::AdoptArg<2>)
	    .addFunction("RemoveWounds", (float(MOSRotating::*)(int numberOfWoundsToRemove)) & MOSRotating::RemoveWounds)
	    .addFunction("RemoveWounds", (float(MOSRotating::*)(int numberOfWoundsToRemove, bool positiveDamage, bool negativeDamage, bool noDamage)) & MOSRotating::RemoveWounds)
	    .addFunction("IsOnScenePoint", &MOSRotating::IsOnScenePoint)
	    .addFunction("EraseFromTerrain", &MOSRotating::EraseFromTerrain)
	    .addFunction("AddAttachable", (void(MOSRotating::*)(Attachable * attachableToAdd)) & MOSRotating::AddAttachable, LuaBinding::AdoptArg<2>)
	    .addFunction("AddAttachable", (void(MOSRotating::*)(Attachable * attachableToAdd, const Vector& parentOffset)) & MOSRotating::AddAttachable, LuaBinding::AdoptArg<2>)
	    .addFunction("RemoveAttachable", (Attachable * (MOSRotating::*)(long uniqueIDOfAttachableToRemove)) & MOSRotating::RemoveAttachable, LuaBinding::AdoptResult)
	    .addFunction("RemoveAttachable", (Attachable * (MOSRotating::*)(long uniqueIDOfAttachableToRemove, bool addToMovableMan, bool addBreakWounds)) & MOSRotating::RemoveAttachable, LuaBinding::AdoptResult)
	    .addFunction("RemoveAttachable", (Attachable * (MOSRotating::*)(Attachable * attachableToRemove)) & MOSRotating::RemoveAttachable, LuaBinding::AdoptResult)
	    .addFunction("RemoveAttachable", (Attachable * (MOSRotating::*)(Attachable * attachableToRemove, bool addToMovableMan, bool addBreakWounds)) & MOSRotating::RemoveAttachable)
	    .addFunction("AddEmitter", (void(MOSRotating::*)(Attachable * attachableToAdd)) & MOSRotating::AddAttachable, LuaBinding::AdoptArg<2>)
	    .addFunction("AddEmitter", (void(MOSRotating::*)(Attachable * attachableToAdd, const Vector& parentOffset)) & MOSRotating::AddAttachable, LuaBinding::AdoptArg<2>)
	    .addFunction("RemoveEmitter", (Attachable * (MOSRotating::*)(long uniqueIDOfAttachableToRemove)) & MOSRotating::RemoveAttachable, LuaBinding::AdoptResult)
	    .addFunction("RemoveEmitter", (Attachable * (MOSRotating::*)(long uniqueIDOfAttachableToRemove, bool addToMovableMan, bool addBreakWounds)) & MOSRotating::RemoveAttachable, LuaBinding::AdoptResult)
	    .addFunction("RemoveEmitter", (Attachable * (MOSRotating::*)(Attachable * attachableToRemove)) & MOSRotating::RemoveAttachable, LuaBinding::AdoptResult)
	    .addFunction("RemoveEmitter", (Attachable * (MOSRotating::*)(Attachable * attachableToRemove, bool addToMovableMan, bool addBreakWounds)) & MOSRotating::RemoveAttachable, LuaBinding::AdoptResult)

	    .addFunction("GibThis", &LuaAdaptersMOSRotating::GibThis);
}

LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, MovableObject) {
	AbstractTypeLuaClassDefinition(MovableObject, SceneObject)

	    .addProperty("Material", &MovableObject::GetMaterial)
	    .addProperty("Mass", &MovableObject::GetMass, &MovableObject::SetMass)
	    .addProperty("Pos", &MovableObject::GetPos, &MovableObject::SetPos)
	    .addProperty("Vel", &MovableObject::GetVel, &MovableObject::SetVel)
	    .addProperty("PrevPos", &MovableObject::GetPrevPos)
	    .addProperty("PrevVel", &MovableObject::GetPrevVel)
	    .addProperty("DistanceTravelled", &MovableObject::GetDistanceTravelled)
	    .addProperty("AngularVel", &MovableObject::GetAngularVel, &MovableObject::SetAngularVel)
	    .addProperty("Radius", &MovableObject::GetRadius)
	    .addProperty("Diameter", &MovableObject::GetDiameter)
	    .addProperty("Scale", &MovableObject::GetScale, &MovableObject::SetScale)
	    .addProperty("EffectRotAngle", &MovableObject::GetEffectRotAngle, &MovableObject::SetEffectRotAngle)
	    .addProperty("EffectAlwaysShows", &MovableObject::GetEffectAlwaysShows, &MovableObject::SetEffectAlwaysShows)
	    .addProperty("EffectStartStrength", &MovableObject::GetEffectStartStrengthFloat, &MovableObject::SetEffectStartStrengthFloat)
	    .addProperty("EffectStopStrength", &MovableObject::GetEffectStopStrengthFloat, &MovableObject::SetEffectStopStrengthFloat)
	    .addProperty("GlobalAccScalar", &MovableObject::GetGlobalAccScalar, &MovableObject::SetGlobalAccScalar)
	    .addProperty("AirResistance", &MovableObject::GetAirResistance, &MovableObject::SetAirResistance)
	    .addProperty("AirThreshold", &MovableObject::GetAirThreshold, &MovableObject::SetAirThreshold)
	    .addProperty("Age", &MovableObject::GetAge, &MovableObject::SetAge)
	    .addProperty("Lifetime", &MovableObject::GetLifetime, &MovableObject::SetLifetime)
	    .addProperty("ID", &MovableObject::GetID)
	    .addProperty("UniqueID", &MovableObject::GetUniqueID)
	    .addProperty("RootID", &MovableObject::GetRootID)
	    .addProperty("MOIDFootprint", &MovableObject::GetMOIDFootprint)
	    .addProperty("Sharpness", &MovableObject::GetSharpness, &MovableObject::SetSharpness)
	    .addProperty("HasEverBeenAddedToMovableMan", &MovableObject::HasEverBeenAddedToMovableMan)
	    .addProperty("AboveHUDPos", &MovableObject::GetAboveHUDPos)
	    .addProperty("HitsMOs", &MovableObject::HitsMOs, &MovableObject::SetToHitMOs)
	    .addProperty("GetsHitByMOs", &MovableObject::GetsHitByMOs, &MovableObject::SetToGetHitByMOs)
	    .addProperty("IgnoresTeamHits", &MovableObject::IgnoresTeamHits, &MovableObject::SetIgnoresTeamHits)
	    .addProperty("IgnoresWhichTeam", &MovableObject::IgnoresWhichTeam)
	    .addProperty("IgnoreTerrain", &MovableObject::IgnoreTerrain, &MovableObject::SetIgnoreTerrain)
	    .addProperty("IgnoresActorHits", &MovableObject::GetIgnoresActorHits, &MovableObject::SetIgnoresActorHits)
	    .addProperty("ToSettle", &MovableObject::ToSettle, &MovableObject::SetToSettle)
	    .addProperty("ToDelete", &MovableObject::ToDelete, &MovableObject::SetToDelete)
	    .addProperty("MissionCritical", &MovableObject::IsMissionCritical, &MovableObject::SetMissionCritical)
	    .addProperty("HUDVisible", &MovableObject::GetHUDVisible, &MovableObject::SetHUDVisible)
	    .addProperty("PinStrength", &MovableObject::GetPinStrength, &MovableObject::SetPinStrength)
	    .addProperty("RestThreshold", &MovableObject::GetRestThreshold, &MovableObject::SetRestThreshold)
	    .addProperty("DamageOnCollision", &MovableObject::DamageOnCollision, &MovableObject::SetDamageOnCollision)
	    .addProperty("DamageOnPenetration", &MovableObject::DamageOnPenetration, &MovableObject::SetDamageOnPenetration)
	    .addProperty("WoundDamageMultiplier", &MovableObject::WoundDamageMultiplier, &MovableObject::SetWoundDamageMultiplier)
	    .addProperty("HitWhatMOID", &MovableObject::HitWhatMOID)
	    .addProperty("HitWhatTerrMaterial", &MovableObject::HitWhatTerrMaterial)
	    .addProperty("HitWhatParticleUniqueID", &MovableObject::HitWhatParticleUniqueID)
	    .addProperty("ApplyWoundDamageOnCollision", &MovableObject::GetApplyWoundDamageOnCollision, &MovableObject::SetApplyWoundDamageOnCollision)
	    .addProperty("ApplyWoundBurstDamageOnCollision", &MovableObject::GetApplyWoundBurstDamageOnCollision, &MovableObject::SetApplyWoundBurstDamageOnCollision)
	    .addProperty("SimUpdatesBetweenScriptedUpdates", &MovableObject::GetSimUpdatesBetweenScriptedUpdates, &MovableObject::SetSimUpdatesBetweenScriptedUpdates)
	    .addProperty("PostEffectEnabled", &MovableObject::GetPostEffectEnabled, &MovableObject::SetPostEffectEnabled)

	    .addFunction("GetParent", (MOSRotating * (MovableObject::*)()) & MovableObject::GetParent)
	    .addFunction("GetParent", (const MOSRotating* (MovableObject::*)() const) & MovableObject::GetParent)
	    .addFunction("GetRootParent", (MovableObject * (MovableObject::*)()) & MovableObject::GetRootParent)
	    .addFunction("GetRootParent", (const MovableObject* (MovableObject::*)() const) & MovableObject::GetRootParent)
	    .addFunction("ReloadScripts", &MovableObject::ReloadScripts)
	    .addFunction("HasScript", &LuaAdaptersMovableObject::HasScript)
	    .addFunction("AddScript", &LuaAdaptersMovableObject::AddScript)
	    .addFunction("ScriptEnabled", &MovableObject::ScriptEnabled)
	    .addFunction("EnableScript", &LuaAdaptersMovableObject::EnableScript)
	    .addFunction("DisableScript", &LuaAdaptersMovableObject::DisableScript1)
	    .addFunction("DisableScript", &LuaAdaptersMovableObject::DisableScript2)
	    .addFunction("EnableOrDisableAllScripts", &MovableObject::EnableOrDisableAllScripts)
	    .addFunction("GetStringValue", &MovableObject::GetStringValue)
	    .addFunction("GetEncodedStringValue", &MovableObject::GetEncodedStringValue)
	    .addFunction("GetNumberValue", &MovableObject::GetNumberValue)
	    .addFunction("GetObjectValue", &MovableObject::GetObjectValue)
	    .addFunction("SetStringValue", &MovableObject::SetStringValue)
	    .addFunction("SetEncodedStringValue", &MovableObject::SetEncodedStringValue)
	    .addFunction("SetNumberValue", &MovableObject::SetNumberValue)
	    .addFunction("SetObjectValue", &MovableObject::SetObjectValue)
	    .addFunction("RemoveStringValue", &MovableObject::RemoveStringValue)
	    .addFunction("RemoveNumberValue", &MovableObject::RemoveNumberValue)
	    .addFunction("RemoveObjectValue", &MovableObject::RemoveObjectValue)
	    .addFunction("StringValueExists", &MovableObject::StringValueExists)
	    .addFunction("NumberValueExists", &MovableObject::NumberValueExists)
	    .addFunction("ObjectValueExists", &MovableObject::ObjectValueExists)
	    .addFunction("GetAltitude", &MovableObject::GetAltitude)
	    .addFunction("GetWhichMOToNotHit", &MovableObject::GetWhichMOToNotHit)
	    .addFunction("SetWhichMOToNotHit", &MovableObject::SetWhichMOToNotHit)
	    .addFunction("IsSetToDelete", &MovableObject::IsSetToDelete)
	    .addFunction("IsMissionCritical", &MovableObject::IsMissionCritical)
	    .addFunction("IsGeneric", &MovableObject::IsGeneric)
	    .addFunction("IsActor", &MovableObject::IsActor)
	    .addFunction("IsDevice", &MovableObject::IsDevice)
	    .addFunction("IsHeldDevice", &MovableObject::IsHeldDevice)
	    .addFunction("IsThrownDevice", &MovableObject::IsThrownDevice)
	    .addFunction("IsGold", &MovableObject::IsGold)
	    .addFunction("IsThrownDevice", &MovableObject::IsThrownDevice)
	    .addFunction("HasObject", &MovableObject::HasObject)
	    .addFunction("HasObjectInGroup", &MovableObject::HasObjectInGroup)
	    .addFunction("AddForce", &MovableObject::AddForce)
	    .addFunction("AddAbsForce", &MovableObject::AddAbsForce)
	    .addFunction("AddImpulseForce", &MovableObject::AddImpulseForce)
	    .addFunction("AddAbsImpulseForce", &MovableObject::AddAbsImpulseForce)
	    .addFunction("ClearForces", &MovableObject::ClearForces)
	    .addFunction("ClearImpulseForces", &MovableObject::ClearImpulseForces)
	    .addFunction("GetForcesCount", &MovableObject::GetForcesCount)
	    .addFunction("GetForceVector", &MovableObject::GetForceVector)
	    .addFunction("GetForceOffset", &MovableObject::GetForceOffset)
	    .addFunction("SetForceVector", &MovableObject::SetForceVector)
	    .addFunction("SetForceOffset", &MovableObject::SetForceOffset)
	    .addFunction("GetImpulsesCount", &MovableObject::GetImpulsesCount)
	    .addFunction("GetImpulseVector", &MovableObject::GetImpulseVector)
	    .addFunction("GetImpulseOffset", &MovableObject::GetImpulseOffset)
	    .addFunction("SetImpulseVector", &MovableObject::SetImpulseVector)
	    .addFunction("SetImpulseOffset", &MovableObject::SetImpulseOffset)
	    .addFunction("RestDetection", &MovableObject::RestDetection)
	    .addFunction("NotResting", &MovableObject::NotResting)
	    .addFunction("IsAtRest", &MovableObject::IsAtRest)
	    .addFunction("MoveOutOfTerrain", &MovableObject::MoveOutOfTerrain)
	    .addFunction("RotateOffset", &MovableObject::RotateOffset)
	    .addFunction("SendMessage", &LuaAdaptersMovableObject::SendMessage1)
	    .addFunction("SendMessage", &LuaAdaptersMovableObject::SendMessage2)
	    .addFunction("RequestSyncedUpdate", &MovableObject::RequestSyncedUpdate)
	    .addFunction("SetEffectStrength", &MovableObject::SetEffectStrength)
	    .addFunction("GetScreenEffectPath", &MovableObject::GetScreenEffectPath)
	    .addFunction("SetScreenEffectPath", &MovableObject::SetScreenEffectPath);
}

LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, PEmitter) {
	ConcreteTypeLuaClassDefinition(PEmitter, MOSParticle)

	    .addProperty("BurstScale", &PEmitter::GetBurstScale, &PEmitter::SetBurstScale)
	    .addProperty("PlayBurstSound", &PEmitter::GetPlayBurstSound, &PEmitter::SetPlayBurstSound)
	    .addProperty("EmitAngle", &PEmitter::GetEmitAngle, &PEmitter::SetEmitAngle)
	    .addProperty("GetThrottle", &PEmitter::GetThrottle, &PEmitter::SetThrottle)
	    .addProperty("Throttle", &PEmitter::GetThrottle, &PEmitter::SetThrottle)
	    .addProperty("ThrottleFactor", &PEmitter::GetThrottleFactor)
	    .addProperty("BurstSpacing", &PEmitter::GetBurstSpacing, &PEmitter::SetBurstSpacing)
	    .addProperty("EmitCountLimit", &PEmitter::GetEmitCountLimit, &PEmitter::SetEmitCountLimit)
	    .addProperty("FlashScale", &PEmitter::GetFlashScale, &PEmitter::SetFlashScale)

	    .addProperty("Emissions", &PEmitter::m_EmissionList, LuaBinding::ReturnStlIterator)

	    .addFunction("IsEmitting", &PEmitter::IsEmitting)
	    .addFunction("WasEmitting", &PEmitter::WasEmitting)
	    .addFunction("EnableEmission", &PEmitter::EnableEmission)
	    .addFunction("GetEmitVector", &PEmitter::GetEmitVector)
	    .addFunction("GetRecoilVector", &PEmitter::GetRecoilVector)
	    .addFunction("EstimateImpulse", &PEmitter::EstimateImpulse)
	    .addFunction("TriggerBurst", &PEmitter::TriggerBurst)
	    .addFunction("IsSetToBurst", &PEmitter::IsSetToBurst)
	    .addFunction("CanTriggerBurst", &PEmitter::CanTriggerBurst)
	    .addFunction("JustStartedEmitting", &PEmitter::JustStartedEmitting);
}

LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, PieSlice) {
	ConcreteTypeLuaClassDefinition(PieSlice, Entity)

	    .addProperty("Type", &LuaAdaptersPieSlice::GetType, &PieSlice::SetType)
	    .addProperty("Direction", &PieSlice::GetDirection, &PieSlice::SetDirection)
	    .addProperty("CanBeMiddleSlice", &PieSlice::GetCanBeMiddleSlice, &PieSlice::SetCanBeMiddleSlice)
	    .addProperty("OriginalSource", &PieSlice::GetOriginalSource)
	    .addProperty("Enabled", &PieSlice::IsEnabled, &PieSlice::SetEnabled)

	    .addProperty("ScriptPath", &PieSlice::GetScriptPath, &PieSlice::SetScriptPath)
	    .addProperty("FunctionName", &PieSlice::GetFunctionName, &PieSlice::SetFunctionName)
	    .addProperty("SubPieMenu", &PieSlice::GetSubPieMenu, &PieSlice::SetSubPieMenu)

	    .addProperty("DrawFlippedToMatchAbsoluteAngle", &PieSlice::GetDrawFlippedToMatchAbsoluteAngle, &PieSlice::SetDrawFlippedToMatchAbsoluteAngle)

	    .addFunction("ReloadScripts", &PieSlice::ReloadScripts)

	    .addEnum({{"NoType", static_cast<int>(PieSliceType::NoType)},
	              {"Pickup", static_cast<int>(PieSliceType::Pickup)},
	              {"Drop", static_cast<int>(PieSliceType::Drop)},
	              {"NextItem", static_cast<int>(PieSliceType::NextItem)},
	              {"PreviousItem", static_cast<int>(PieSliceType::PreviousItem)},
	              {"Reload", static_cast<int>(PieSliceType::Reload)},
	              {"BuyMenu", static_cast<int>(PieSliceType::BuyMenu)},
	              {"Stats", static_cast<int>(PieSliceType::Stats)},
	              {"Map", static_cast<int>(PieSliceType::Map)},
	              {"FormSquad", static_cast<int>(PieSliceType::FormSquad)},
	              {"Ceasefire", static_cast<int>(PieSliceType::Ceasefire)},
	              {"Sentry", static_cast<int>(PieSliceType::Sentry)},
	              {"Patrol", static_cast<int>(PieSliceType::Patrol)},
	              {"BrainHunt", static_cast<int>(PieSliceType::BrainHunt)},
	              {"GoldDig", static_cast<int>(PieSliceType::GoldDig)},
	              {"GoTo", static_cast<int>(PieSliceType::GoTo)},
	              {"Return", static_cast<int>(PieSliceType::Return)},
	              {"Stay", static_cast<int>(PieSliceType::Stay)},
	              {"Deliver", static_cast<int>(PieSliceType::Deliver)},
	              {"Scuttle", static_cast<int>(PieSliceType::Scuttle)},
	              {"Done", static_cast<int>(PieSliceType::EditorDone)},
	              {"Load", static_cast<int>(PieSliceType::EditorLoad)},
	              {"Save", static_cast<int>(PieSliceType::EditorSave)},
	              {"New", static_cast<int>(PieSliceType::EditorNew)},
	              {"Pick", static_cast<int>(PieSliceType::EditorPick)},
	              {"Move", static_cast<int>(PieSliceType::EditorMove)},
	              {"Remove", static_cast<int>(PieSliceType::EditorRemove)},
	              {"InFront", static_cast<int>(PieSliceType::EditorInFront)},
	              {"Behind", static_cast<int>(PieSliceType::EditorBehind)},
	              {"ZoomIn", static_cast<int>(PieSliceType::EditorZoomIn)},
	              {"ZoomOut", static_cast<int>(PieSliceType::EditorZoomOut)},
	              {"Team1", static_cast<int>(PieSliceType::EditorTeam1)},
	              {"Team2", static_cast<int>(PieSliceType::EditorTeam2)},
	              {"Team3", static_cast<int>(PieSliceType::EditorTeam3)},
	              {"Team4", static_cast<int>(PieSliceType::EditorTeam4)}});
}

LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, PieMenu) {
	ConcreteTypeLuaClassDefinition(PieMenu, Entity)

	    .addProperty("Owner", &PieMenu::GetOwner)
	    .addProperty("Controller", &PieMenu::GetController)
	    .addProperty("AffectedObject", &PieMenu::GetAffectedObject)
	    .addProperty("Pos", &PieMenu::GetPos)
	    .addProperty("RotAngle", &PieMenu::GetRotAngle, &PieMenu::SetRotAngle)
	    .addProperty("FullInnerRadius", &PieMenu::GetFullInnerRadius, &PieMenu::SetFullInnerRadius)

	    .addProperty("PieSlices", &PieMenu::GetPieSlices, LuaBinding::ReturnStlIterator)

	    .addFunction("IsSubPieMenu", &PieMenu::IsSubPieMenu)

	    .addFunction("IsEnabled", &PieMenu::IsEnabled)
	    .addFunction("IsEnabling", &PieMenu::IsEnabling)
	    .addFunction("IsDisabling", &PieMenu::IsDisabling)
	    .addFunction("IsEnablingOrDisabling", &PieMenu::IsEnablingOrDisabling)
	    .addFunction("IsVisible", &PieMenu::IsVisible)
	    .addFunction("HasSubPieMenuOpen", &PieMenu::HasSubPieMenuOpen)

	    .addFunction("SetAnimationModeToNormal", &PieMenu::SetAnimationModeToNormal)
	    .addFunction("DoDisableAnimation", &PieMenu::DoDisableAnimation)
	    .addFunction("Wobble", &PieMenu::Wobble)
	    .addFunction("FreezeAtRadius", &PieMenu::FreezeAtRadius)

	    .addFunction("GetPieCommand", &LuaAdaptersPieMenu::GetPieCommand)
	    .addFunction("GetFirstPieSliceByPresetName", &PieMenu::GetFirstPieSliceByPresetName)
	    .addFunction("GetFirstPieSliceByType", &PieMenu::GetFirstPieSliceByType)
	    .addFunction("AddPieSlice", &PieMenu::AddPieSlice, LuaBinding::AdoptArg<2>)
	    .addFunction("AddPieSlice", &LuaAdaptersPieMenu::AddPieSlice, LuaBinding::AdoptArg<2>)
	    .addFunction("AddPieSliceIfPresetNameIsUnique", &PieMenu::AddPieSliceIfPresetNameIsUnique)
	    .addFunction("AddPieSliceIfPresetNameIsUnique", &LuaAdaptersPieMenu::AddPieSliceIfPresetNameIsUnique1)
	    .addFunction("AddPieSliceIfPresetNameIsUnique", &LuaAdaptersPieMenu::AddPieSliceIfPresetNameIsUnique2)
	    .addFunction("RemovePieSlice", &PieMenu::RemovePieSlice, LuaBinding::AdoptResult)
	    .addFunction("RemovePieSlicesByPresetName", &PieMenu::RemovePieSlicesByPresetName)
	    .addFunction("RemovePieSlicesByType", &PieMenu::RemovePieSlicesByType)
	    .addFunction("RemovePieSlicesByOriginalSource", &PieMenu::RemovePieSlicesByOriginalSource)
	    .addFunction("ReplacePieSlice", &PieMenu::ReplacePieSlice, LuaBinding::AdoptResult, LuaBinding::AdoptArg<3>);
}

LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, Round) {
	ConcreteTypeLuaClassDefinition(Round, Entity)

	    .addProperty("NextParticle", &Round::GetNextParticle)
	    .addProperty("Shell", &Round::GetShell)
	    .addProperty("FireVel", &Round::GetFireVel)
	    .addProperty("InheritsFirerVelocity", &Round::GetInheritsFirerVelocity)
	    .addProperty("ShellVel", &Round::GetShellVel)
	    .addProperty("Separation", &Round::GetSeparation)
	    .addProperty("ParticleCount", &Round::ParticleCount)
	    .addProperty("AILifeTime", &Round::GetAILifeTime)
	    .addProperty("AIFireVel", &Round::GetAIFireVel)
	    .addProperty("IsEmpty", &Round::IsEmpty);
}

LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, Scene) {
	ConcreteTypeLuaClassDefinition(Scene, Entity)

	    .addProperty("Location", &Scene::GetLocation, &Scene::SetLocation)
	    //.addProperty("Terrain", &Scene::GetTerrain)
	    .addProperty("Dimensions", &Scene::GetDimensions)
	    .addProperty("Width", &Scene::GetWidth)
	    .addProperty("Height", &Scene::GetHeight)
	    .addProperty("WrapsX", &Scene::WrapsX)
	    .addProperty("WrapsY", &Scene::WrapsY)
	    .addProperty("TeamOwnership", &Scene::GetTeamOwnership, &Scene::SetTeamOwnership)
	    .addProperty("GlobalAcc", &Scene::GetGlobalAcc, &Scene::SetGlobalAcc)
	    .addProperty("ScenePathSize", &Scene::GetScenePathSize)

	    .addProperty("Deployments", &Scene::m_Deployments, LuaBinding::ReturnStlIterator)
	    .addProperty("Areas", &Scene::m_AreaList, LuaBinding::ReturnStlIterator)
	    .addProperty("BackgroundLayers", &Scene::m_BackLayerList, LuaBinding::ReturnStlIterator)

	    .addFunction("GetScenePath", &Scene::GetScenePath, LuaBinding::ReturnStlIterator)
	    .addFunction("GetBuildBudget", &Scene::GetBuildBudget)
	    .addFunction("SetBuildBudget", &Scene::SetBuildBudget)
	    .addFunction("IsScanScheduled", &Scene::IsScanScheduled)
	    .addFunction("SetScheduledScan", &Scene::SetScheduledScan)
	    .addFunction("ClearPlacedObjectSet", &Scene::ClearPlacedObjectSet)
	    .addFunction("PlaceResidentBrain", &Scene::PlaceResidentBrain)
	    .addFunction("PlaceResidentBrains", &Scene::PlaceResidentBrains)
	    .addFunction("RetrieveResidentBrains", &Scene::RetrieveResidentBrains)
	    .addFunction("GetResidentBrain", &Scene::GetResidentBrain)
	    .addFunction("SetResidentBrain", &Scene::SetResidentBrain)
	    .addFunction("SetArea", &Scene::SetArea)
	    .addFunction("HasArea", &Scene::HasArea)
	    .addFunction("GetArea", &Scene::GetArea)
	    .addFunction("WithinArea", &Scene::WithinArea)
	    .addFunction("AddNavigableArea", &Scene::AddNavigableArea)
	    .addFunction("ClearNavigableAreas", &Scene::ClearNavigableAreas)
	    .addFunction("ResetPathFinding", &Scene::ResetPathFinding)
	    .addFunction("UpdatePathFinding", &Scene::UpdatePathFinding)
	    .addFunction("PathFindingUpdated", &Scene::PathFindingUpdated)
	    .addFunction("CalculatePath", &LuaAdaptersScene::CalculatePath)
	    .addFunction("CalculatePath", &LuaAdaptersScene::CalculatePath1)
	    .addFunction("CalculatePathAsync", &LuaAdaptersScene::CalculatePathAsync)
	    .addFunction("CalculatePathAsync", &LuaAdaptersScene::CalculatePathAsync1)

	    .addEnum({{"PLACEONLOAD", Scene::PlacedObjectSets::PLACEONLOAD},
	              {"BLUEPRINT", Scene::PlacedObjectSets::BLUEPRINT},
	              {"AIPLAN", Scene::PlacedObjectSets::AIPLAN},
	              {"PLACEDSETSCOUNT", Scene::PlacedObjectSets::PLACEDSETSCOUNT}});
}

LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, SceneArea) {
	registry.Class<Scene::Area>("Area")

	    .addConstructor<>()
	    .addConstructor<std::string>()
	    .addConstructor<const Scene::Area&>()

	    .addProperty("ClassName", &Scene::Area::GetClassName)
	    .addProperty("Name", &Scene::Area::GetName)
	    .addProperty("FirstBox", &Scene::Area::GetFirstBox)
	    .addProperty("Center", &Scene::Area::GetCenterPoint)
	    .addProperty("RandomPoint", &Scene::Area::GetRandomPoint)

	    .addProperty("Boxes", &Scene::Area::m_BoxList, LuaBinding::ReturnStlIterator)

	    .addFunction("Reset", &Scene::Area::Reset)
	    .addFunction("AddBox", &Scene::Area::AddBox)
	    .addFunction("RemoveBox", &Scene::Area::RemoveBox)
	    .addFunction("HasNoArea", &Scene::Area::HasNoArea)
	    .addFunction("IsInside", &Scene::Area::IsInside)
	    .addFunction("IsInsideX", &Scene::Area::IsInsideX)
	    .addFunction("IsInsideY", &Scene::Area::IsInsideY)
	    .addFunction("GetBoxInside", &Scene::Area::GetBoxInside)
	    .addFunction("RemoveBoxInside", &Scene::Area::RemoveBoxInside)
	    .addFunction("GetCenterPoint", &Scene::Area::GetCenterPoint)
	    .addFunction("GetRandomPoint", &Scene::Area::GetRandomPoint);
}

LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, StaticSceneLayer) {
	registry.Class<StaticSceneLayer, Entity>("StaticSceneLayer");
}

LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, SceneObject) {
	AbstractTypeLuaClassDefinition(SceneObject, Entity)

	    .addProperty("Pos", &SceneObject::GetPos, &SceneObject::SetPos)
	    .addProperty("HFlipped", &SceneObject::IsHFlipped, &SceneObject::SetHFlipped)
	    .addProperty("RotAngle", &SceneObject::GetRotAngle, &SceneObject::SetRotAngle)
	    .addProperty("Team", &SceneObject::GetTeam, &SceneObject::SetTeam)
	    .addProperty("PlacedByPlayer", &SceneObject::GetPlacedByPlayer, &SceneObject::SetPlacedByPlayer)
	    .addProperty("Buyable", &SceneObject::IsBuyable)

	    .addProperty("BuyableMode", &LuaAdaptersSceneObject::GetBuyableMode)

	    .addFunction("IsOnScenePoint", &SceneObject::IsOnScenePoint)
	    .addFunction("GetGoldValue", &SceneObject::GetGoldValueOld)
	    .addFunction("GetGoldValue", &SceneObject::GetGoldValue)
	    .addFunction("SetGoldValue", &SceneObject::SetGoldValue)
	    .addFunction("GetGoldValueString", &SceneObject::GetGoldValueString)

	    .addFunction("GetTotalValue", &SceneObject::GetTotalValue)
	    .addFunction("GetTotalValue", &LuaAdaptersSceneObject::GetTotalValue)

	    .addEnum({{"NORESTRICTIONS", static_cast<int>(SceneObject::BuyableMode::NoRestrictions)},
	              {"BUYMENUONLY", static_cast<int>(SceneObject::BuyableMode::BuyMenuOnly)},
	              {"OBJECTPICKERONLY", static_cast<int>(SceneObject::BuyableMode::ObjectPickerOnly)},
	              {"SCRIPTONLY", static_cast<int>(SceneObject::BuyableMode::ScriptOnly)}});
}

LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, SLBackground) {
	registry.Class<SLBackground, StaticSceneLayer>("SLBackground")

	    .addProperty("Frame", &SLBackground::GetFrame, &SLBackground::SetFrame)
	    .addProperty("SpriteAnimMode", &SLBackground::GetSpriteAnimMode, &SLBackground::SetSpriteAnimMode)
	    .addProperty("SpriteAnimDuration", &SLBackground::GetSpriteAnimDuration, &SLBackground::SetSpriteAnimDuration)
	    .addProperty("IsAnimatedManually", &SLBackground::IsAnimatedManually, &SLBackground::SetAnimatedManually)
	    .addProperty("AutoScrollX", &SLBackground::GetAutoScrollX, &SLBackground::SetAutoScrollX)
	    .addProperty("AutoScrollY", &SLBackground::GetAutoScrollY, &SLBackground::SetAutoScrollY)
	    .addProperty("AutoScrollInterval", &SLBackground::GetAutoScrollStepInterval, &SLBackground::SetAutoScrollStepInterval)
	    .addProperty("AutoScrollStep", &SLBackground::GetAutoScrollStep, &SLBackground::SetAutoScrollStep)
	    .addProperty("AutoScrollStepX", &SLBackground::GetAutoScrollStepX, &SLBackground::SetAutoScrollStepX)
	    .addProperty("AutoScrollStepY", &SLBackground::GetAutoScrollStepY, &SLBackground::SetAutoScrollStepY)

	    .addFunction("IsAutoScrolling", &SLBackground::IsAutoScrolling);
}

LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, SoundContainer) {
	ConcreteTypeLuaClassDefinition(SoundContainer, Entity)

	    .addConstructor<>()

	    .addProperty("SoundOverlapMode", &SoundContainer::GetSoundOverlapMode, &SoundContainer::SetSoundOverlapMode)
	    .addProperty("BusRouting", &SoundContainer::GetBusRouting, &SoundContainer::SetBusRouting)
	    .addProperty("Immobile", &SoundContainer::IsImmobile, &SoundContainer::SetImmobile)
	    .addProperty("AttenuationStartDistance", &SoundContainer::GetAttenuationStartDistance, &SoundContainer::SetAttenuationStartDistance)
	    .addProperty("CustomPanValue", &SoundContainer::GetCustomPanValue, &SoundContainer::SetCustomPanValue)
	    .addProperty("PanningStrengthMultiplier", &SoundContainer::GetPanningStrengthMultiplier, &SoundContainer::SetPanningStrengthMultiplier)
	    .addProperty("Loops", &SoundContainer::GetLoopSetting, &SoundContainer::SetLoopSetting)
	    .addProperty("Priority", &SoundContainer::GetPriority, &SoundContainer::SetPriority)
	    .addProperty("AffectedByGlobalPitch", &SoundContainer::IsAffectedByGlobalPitch, &SoundContainer::SetAffectedByGlobalPitch)
	    .addProperty("Pos", &SoundContainer::GetPosition, &SoundContainer::SetPosition)
	    .addProperty("Volume", &SoundContainer::GetVolume, &SoundContainer::SetVolume)
	    .addProperty("Pitch", &SoundContainer::GetPitch, &SoundContainer::SetPitch)
	    .addProperty("Paused", &SoundContainer::IsPaused, &SoundContainer::SetPaused)
	    .addProperty("PitchVariation", &SoundContainer::GetPitchVariation, &SoundContainer::SetPitchVariation)

	    .addFunction("HasAnySounds", &SoundContainer::HasAnySounds)
	    .addFunction("GetTopLevelSoundSet", &SoundContainer::GetTopLevelSoundSet)
	    .addFunction("SetTopLevelSoundSet", &SoundContainer::SetTopLevelSoundSet)
	    .addFunction("GetAudibleVolume", &SoundContainer::GetAudibleVolume)
	    .addFunction("IsBeingPlayed", &SoundContainer::IsBeingPlayed)
	    .addFunction("Play", (bool(SoundContainer::*)()) & SoundContainer::Play)
	    .addFunction("Play", (bool(SoundContainer::*)(const int player)) & SoundContainer::Play)
	    .addFunction("Play", (bool(SoundContainer::*)(const Vector& position)) & SoundContainer::Play)
	    .addFunction("Play", (bool(SoundContainer::*)(const Vector& position, int player)) & SoundContainer::Play)
	    .addFunction("Stop", (bool(SoundContainer::*)()) & SoundContainer::Stop)
	    .addFunction("Stop", (bool(SoundContainer::*)(int player)) & SoundContainer::Stop)
	    .addFunction("Restart", (bool(SoundContainer::*)()) & SoundContainer::Restart)
	    .addFunction("Restart", (bool(SoundContainer::*)(int player)) & SoundContainer::Restart)
	    .addFunction("FadeOut", &SoundContainer::FadeOut)

	    .addEnum({{"SFX", SoundContainer::BusRouting::SFX},
	              {"UI", SoundContainer::BusRouting::UI},
	              {"MUSIC", SoundContainer::BusRouting::MUSIC}})

	    .addEnum({{"OVERLAP", SoundContainer::SoundOverlapMode::OVERLAP},
	              {"RESTART", SoundContainer::SoundOverlapMode::RESTART},
	              {"IGNORE_PLAY", SoundContainer::SoundOverlapMode::IGNORE_PLAY}});
}

LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, SoundSet) {
	registry.Class<SoundSet>("SoundSet")

	    .addConstructor<>()

	    .addProperty("SoundSelectionCycleMode", &SoundSet::GetSoundSelectionCycleMode, &SoundSet::SetSoundSelectionCycleMode)

	    .addProperty("SubSoundSets", &SoundSet::m_SubSoundSets, LuaBinding::ReturnStlIterator)

	    .addFunction("HasAnySounds", &SoundSet::HasAnySounds)
	    .addFunction("SelectNextSounds", &SoundSet::SelectNextSounds)
	    .addFunction("AddSound", (void(SoundSet::*)(const std::string& soundFilePath)) & SoundSet::AddSound)
	    .addFunction("AddSound", (void(SoundSet::*)(const std::string& soundFilePath, const Vector& offset, float minimumAudibleDistance, float attenuationStartDistance)) & SoundSet::AddSound)
	    .addFunction("RemoveSound", (bool(SoundSet::*)(const std::string& soundFilePath)) & SoundSet::RemoveSound)
	    .addFunction("RemoveSound", (bool(SoundSet::*)(const std::string& soundFilePath, bool removeFromSubSoundSets)) & SoundSet::RemoveSound)
	    .addFunction("AddSoundSet", &SoundSet::AddSoundSet)

	    .addEnum({{"RANDOM", SoundSet::SoundSelectionCycleMode::RANDOM},
	              {"FORWARDS", SoundSet::SoundSelectionCycleMode::FORWARDS},
	              {"ALL", SoundSet::SoundSelectionCycleMode::ALL}});
}

LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, TDExplosive) {
	ConcreteTypeLuaClassDefinition(TDExplosive, ThrownDevice)

	    .addProperty("IsAnimatedManually", &TDExplosive::IsAnimatedManually, &TDExplosive::SetAnimatedManually);
}

LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, TerrainObject) {
	ConcreteTypeLuaClassDefinition(TerrainObject, SceneObject)

	    .addFunction("GetBitmapOffset", &TerrainObject::GetBitmapOffset)
	    .addFunction("GetBitmapWidth", &TerrainObject::GetBitmapWidth)
	    .addFunction("GetBitmapHeight", &TerrainObject::GetBitmapHeight);
}

LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, ThrownDevice) {
	ConcreteTypeLuaClassDefinition(ThrownDevice, HeldDevice)

	    .addProperty("MinThrowVel", &ThrownDevice::GetMinThrowVel, &ThrownDevice::SetMinThrowVel)
	    .addProperty("MaxThrowVel", &ThrownDevice::GetMaxThrowVel, &ThrownDevice::SetMaxThrowVel)
	    .addProperty("StartThrowOffset", &ThrownDevice::GetStartThrowOffset, &ThrownDevice::SetStartThrowOffset)
	    .addProperty("EndThrowOffset", &ThrownDevice::GetEndThrowOffset, &ThrownDevice::SetEndThrowOffset)

	    .addFunction("GetCalculatedMaxThrowVelIncludingArmThrowStrength", &ThrownDevice::GetCalculatedMaxThrowVelIncludingArmThrowStrength);
}

LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, Turret) {
	ConcreteTypeLuaClassDefinition(Turret, Attachable)

	    .addProperty("MountedDevice", &Turret::GetFirstMountedDevice, &LuaAdaptersPropertyOwnershipSafetyFaker::TurretSetFirstMountedDevice)
	    .addProperty("MountedDeviceRotationOffset", &Turret::GetMountedDeviceRotationOffset, &Turret::SetMountedDeviceRotationOffset)

	    .addFunction("GetMountedDevices", &Turret::GetMountedDevices, LuaBinding::ReturnStlIterator)
	    .addFunction("AddMountedDevice", &Turret::AddMountedDevice, LuaBinding::AdoptArg<2>)
	    .addFunction("AddMountedDevice", &LuaAdaptersTurret::AddMountedFirearm, LuaBinding::AdoptArg<2>);
}
