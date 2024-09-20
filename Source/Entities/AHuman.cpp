#include "AHuman.h"

#include "AtomGroup.h"
#include "ThrownDevice.h"
#include "Arm.h"
#include "Leg.h"
#include "Controller.h"
#include "MOPixel.h"
#include "FrameMan.h"
#include "AEJetpack.h"
#include "HDFirearm.h"
#include "SLTerrain.h"
#include "PresetMan.h"
#include "Scene.h"
#include "SettingsMan.h"
#include "PostProcessMan.h"
#include "PieMenu.h"

#include "GUI.h"
#include "AllegroBitmap.h"

#include "PrimitiveMan.h"

#include "tracy/Tracy.hpp"

using namespace RTE;

ConcreteClassInfo(AHuman, Actor, 20);

AHuman::AHuman() {
	Clear();
}

AHuman::~AHuman() {
	Destroy(true);
}

void AHuman::Clear() {
	m_pHead = 0;
	m_LookToAimRatio = 0.7F;
	m_pJetpack = nullptr;
	m_pFGArm = 0;
	m_pBGArm = 0;
	m_pFGLeg = 0;
	m_pBGLeg = 0;
	m_pFGHandGroup = 0;
	m_pBGHandGroup = 0;
	m_pFGFootGroup = 0;
	m_BackupFGFootGroup = nullptr;
	m_pBGFootGroup = 0;
	m_BackupBGFootGroup = nullptr;
	m_StrideSound = nullptr;
	m_ArmsState = WEAPON_READY;
	m_MoveState = STAND;
	m_ProneState = NOTPRONE;
	m_ProneTimer.Reset();
	m_MaxWalkPathCrouchShift = 6.0F;
	m_CrouchAmount = 0.0F;
	m_CrouchAmountOverride = -1.0F;
	for (int i = 0; i < MOVEMENTSTATECOUNT; ++i) {
		m_Paths[FGROUND][i].Reset();
		m_Paths[BGROUND][i].Reset();
		m_Paths[FGROUND][i].Terminate();
		m_Paths[BGROUND][i].Terminate();
		m_RotAngleTargets[i] = 0.0F;
	}
	// Default CROUCH RotAngleTarget to a more sensible number
	m_RotAngleTargets[CROUCH] = c_QuarterPI * 0.85F;
	m_Aiming = false;
	m_ArmClimbing[FGROUND] = false;
	m_ArmClimbing[BGROUND] = false;
	m_StrideFrame = false;
	m_StrideStart = false;
	m_CanActivateBGItem = false;
	m_TriggerPulled = false;
	m_WaitingToReloadOffhand = false;
	m_ThrowTmr.Reset();
	m_ThrowPrepTime = 1000;
	m_SharpAimRevertTimer.Reset();
	m_FGArmFlailScalar = 0.0F;
	m_BGArmFlailScalar = 0.7F;
	m_EquipHUDTimer.Reset();
	m_WalkAngle.fill(Matrix());
	m_WalkPathOffset.Reset();
	m_ArmSwingRate = 1.0F;
	m_DeviceArmSwayRate = 0.5F;

	m_DeviceState = SCANNING;
	m_SweepState = NOSWEEP;
	m_DigState = NOTDIGGING;
	m_JumpState = NOTJUMPING;
	m_JumpTarget.Reset();
	m_JumpingRight = true;
	m_Crawling = false;
	m_DigTunnelEndPos.Reset();
	m_SweepCenterAimAngle = 0;
	m_SweepRange = c_EighthPI;
	m_DigTarget.Reset();
	m_FireTimer.Reset();
	m_SweepTimer.Reset();
	m_PatrolTimer.Reset();
	m_JumpTimer.Reset();
}

int AHuman::Create() {
	if (Actor::Create() < 0) {
		return -1;
	}

	if (m_AIMode == Actor::AIMODE_NONE) {
		m_AIMode = Actor::AIMODE_BRAINHUNT;
	}

	// Cheat to make sure the FG Arm is always at the end of the Attachables list so it draws last.
	if (m_pFGArm) {
		m_Attachables.erase(std::find(m_Attachables.begin(), m_Attachables.end(), m_pFGArm));
		m_Attachables.push_back(m_pFGArm);
	}

	// Make the limb paths for the background limbs
	for (int i = 0; i < MOVEMENTSTATECOUNT; ++i) {
		// If BG path is not initalized, then copy the FG one to it
		if (!m_Paths[BGROUND][i].IsInitialized()) {
			m_Paths[BGROUND][i].Destroy();
			m_Paths[BGROUND][i].Create(m_Paths[FGROUND][i]);
		}
	}

	// If empty-handed, equip first thing in inventory
	if (m_pFGArm && m_pFGArm->IsAttached() && !m_pFGArm->GetHeldDevice()) {
		m_pFGArm->SetHeldDevice(dynamic_cast<HeldDevice*>(SwapNextInventory(nullptr, true)));
		m_pFGArm->SetHandPos(m_Pos + m_HolsterOffset.GetXFlipped(m_HFlipped));
	}

	// All AHumans by default avoid hitting each other ont he same team
	m_IgnoresTeamHits = true;

	return 0;
}

int AHuman::Create(const AHuman& reference) {
	if (reference.m_pBGArm) {
		m_ReferenceHardcodedAttachableUniqueIDs.insert(reference.m_pBGArm->GetUniqueID());
	}
	if (reference.m_pBGLeg) {
		m_ReferenceHardcodedAttachableUniqueIDs.insert(reference.m_pBGLeg->GetUniqueID());
	}
	if (reference.m_pJetpack) {
		m_ReferenceHardcodedAttachableUniqueIDs.insert(reference.m_pJetpack->GetUniqueID());
	}
	if (reference.m_pHead) {
		m_ReferenceHardcodedAttachableUniqueIDs.insert(reference.m_pHead->GetUniqueID());
	}
	if (reference.m_pFGLeg) {
		m_ReferenceHardcodedAttachableUniqueIDs.insert(reference.m_pFGLeg->GetUniqueID());
	}
	if (reference.m_pFGArm) {
		m_ReferenceHardcodedAttachableUniqueIDs.insert(reference.m_pFGArm->GetUniqueID());
	}

	Actor::Create(reference);

	// Note - hardcoded attachable copying is organized based on desired draw order here.
	if (reference.m_pBGArm) {
		SetBGArm(dynamic_cast<Arm*>(reference.m_pBGArm->Clone()));
	}
	if (reference.m_pBGLeg) {
		SetBGLeg(dynamic_cast<Leg*>(reference.m_pBGLeg->Clone()));
	}
	if (reference.m_pJetpack) {
		SetJetpack(dynamic_cast<AEJetpack*>(reference.m_pJetpack->Clone()));
	}
	if (reference.m_pHead) {
		SetHead(dynamic_cast<Attachable*>(reference.m_pHead->Clone()));
	}
	if (reference.m_pFGLeg) {
		SetFGLeg(dynamic_cast<Leg*>(reference.m_pFGLeg->Clone()));
	}
	if (reference.m_pFGArm) {
		SetFGArm(dynamic_cast<Arm*>(reference.m_pFGArm->Clone()));
	}

	m_LookToAimRatio = reference.m_LookToAimRatio;

	m_ThrowPrepTime = reference.m_ThrowPrepTime;
	m_WaitingToReloadOffhand = reference.m_WaitingToReloadOffhand;
	m_FGArmFlailScalar = reference.m_FGArmFlailScalar;
	m_BGArmFlailScalar = reference.m_BGArmFlailScalar;
	m_ArmSwingRate = reference.m_ArmSwingRate;
	m_DeviceArmSwayRate = reference.m_DeviceArmSwayRate;

	m_pFGHandGroup = dynamic_cast<AtomGroup*>(reference.m_pFGHandGroup->Clone());
	m_pFGHandGroup->SetOwner(this);
	m_pBGHandGroup = dynamic_cast<AtomGroup*>(reference.m_pBGHandGroup->Clone());
	m_pBGHandGroup->SetOwner(this);

	AtomGroup* atomGroupToUseAsFootGroupFG = reference.m_pFGFootGroup ? dynamic_cast<AtomGroup*>(reference.m_pFGFootGroup->Clone()) : m_pFGLeg->GetFootGroupFromFootAtomGroup();
	RTEAssert(atomGroupToUseAsFootGroupFG, "Failed to fallback to using FGFoot AtomGroup as FGFootGroup in preset " + this->GetModuleAndPresetName() + "!\nPlease define a FGFootGroup or FGLeg Foot attachable!");

	AtomGroup* atomGroupToUseAsFootGroupBG = reference.m_pBGFootGroup ? dynamic_cast<AtomGroup*>(reference.m_pBGFootGroup->Clone()) : m_pBGLeg->GetFootGroupFromFootAtomGroup();
	;
	RTEAssert(atomGroupToUseAsFootGroupBG, "Failed to fallback to using BGFoot AtomGroup as BGFootGroup in preset " + this->GetModuleAndPresetName() + "!\nPlease define a BGFootGroup or BGLeg Foot attachable!");

	m_pFGFootGroup = atomGroupToUseAsFootGroupFG;
	m_pFGFootGroup->SetOwner(this);
	m_BackupFGFootGroup = dynamic_cast<AtomGroup*>(atomGroupToUseAsFootGroupFG->Clone());
	m_BackupFGFootGroup->RemoveAllAtoms();
	m_BackupFGFootGroup->SetOwner(this);
	m_BackupFGFootGroup->SetLimbPos(atomGroupToUseAsFootGroupFG->GetLimbPos());
	m_pBGFootGroup = atomGroupToUseAsFootGroupBG;
	m_pBGFootGroup->SetOwner(this);
	m_BackupBGFootGroup = dynamic_cast<AtomGroup*>(atomGroupToUseAsFootGroupBG->Clone());
	m_BackupBGFootGroup->RemoveAllAtoms();
	m_BackupBGFootGroup->SetOwner(this);
	m_BackupBGFootGroup->SetLimbPos(atomGroupToUseAsFootGroupBG->GetLimbPos());

	m_MaxWalkPathCrouchShift = reference.m_MaxWalkPathCrouchShift;

	if (reference.m_StrideSound) {
		m_StrideSound = dynamic_cast<SoundContainer*>(reference.m_StrideSound->Clone());
	}

	m_ArmsState = reference.m_ArmsState;
	m_MoveState = reference.m_MoveState;
	m_ProneState = reference.m_ProneState;

	for (int i = 0; i < MOVEMENTSTATECOUNT; ++i) {
		m_Paths[FGROUND][i].Create(reference.m_Paths[FGROUND][i]);
		m_Paths[BGROUND][i].Create(reference.m_Paths[BGROUND][i]);
		m_RotAngleTargets[i] = reference.m_RotAngleTargets[i];
	}

	// Default the run walkpath to be the same as the walk one, if it doesn't exist
	if (m_Paths[FGROUND][RUN].GetSegCount() == 0) {
		m_Paths[FGROUND][RUN].Create(reference.m_Paths[FGROUND][WALK]);
		m_Paths[BGROUND][RUN].Create(reference.m_Paths[BGROUND][WALK]);
		m_Paths[FGROUND][RUN].SetBaseSpeedMultiplier(reference.m_Paths[FGROUND][WALK].GetSpeed(Speed::FAST) / reference.m_Paths[FGROUND][WALK].GetSpeed(Speed::NORMAL));
		m_Paths[BGROUND][RUN].SetBaseSpeedMultiplier(reference.m_Paths[BGROUND][WALK].GetSpeed(Speed::FAST) / reference.m_Paths[BGROUND][WALK].GetSpeed(Speed::NORMAL));
	}

	m_DeviceState = reference.m_DeviceState;
	m_SweepState = reference.m_SweepState;
	m_DigState = reference.m_DigState;
	m_JumpState = reference.m_JumpState;
	m_JumpTarget = reference.m_JumpTarget;
	m_JumpingRight = reference.m_JumpingRight;
	m_Crawling = reference.m_Crawling;
	m_DigTunnelEndPos = reference.m_DigTunnelEndPos;
	m_SweepCenterAimAngle = reference.m_SweepCenterAimAngle;
	m_SweepRange = reference.m_SweepRange;

	return 0;
}

int AHuman::ReadProperty(const std::string_view& propName, Reader& reader) {
	StartPropertyList(return Actor::ReadProperty(propName, reader));

	MatchProperty("ThrowPrepTime", { reader >> m_ThrowPrepTime; });
	MatchProperty("Head", { SetHead(dynamic_cast<Attachable*>(g_PresetMan.ReadReflectedPreset(reader))); });
	MatchProperty("LookToAimRatio", { reader >> m_LookToAimRatio; });
	MatchProperty("Jetpack", { SetJetpack(dynamic_cast<AEJetpack*>(g_PresetMan.ReadReflectedPreset(reader))); });
	MatchProperty("FGArmFlailScalar", { reader >> m_FGArmFlailScalar; });
	MatchProperty("BGArmFlailScalar", { reader >> m_BGArmFlailScalar; });
	MatchProperty("ArmSwingRate", { reader >> m_ArmSwingRate; });
	MatchProperty("DeviceArmSwayRate", { reader >> m_DeviceArmSwayRate; });
	MatchProperty("FGArm", { SetFGArm(dynamic_cast<Arm*>(g_PresetMan.ReadReflectedPreset(reader))); });
	MatchProperty("BGArm", { SetBGArm(dynamic_cast<Arm*>(g_PresetMan.ReadReflectedPreset(reader))); });
	MatchProperty("FGLeg", { SetFGLeg(dynamic_cast<Leg*>(g_PresetMan.ReadReflectedPreset(reader))); });
	MatchProperty("BGLeg", { SetBGLeg(dynamic_cast<Leg*>(g_PresetMan.ReadReflectedPreset(reader))); });
	MatchProperty("HandGroup", {
		delete m_pFGHandGroup;
		delete m_pBGHandGroup;
		m_pFGHandGroup = new AtomGroup();
		m_pBGHandGroup = new AtomGroup();
		reader >> m_pFGHandGroup;
		m_pBGHandGroup->Create(*m_pFGHandGroup);
		m_pFGHandGroup->SetOwner(this);
		m_pBGHandGroup->SetOwner(this);
	});
	MatchProperty("FGFootGroup", {
		delete m_pFGFootGroup;
		m_pFGFootGroup = new AtomGroup();
		reader >> m_pFGFootGroup;
		m_pFGFootGroup->SetOwner(this);
		m_BackupFGFootGroup = new AtomGroup(*m_pFGFootGroup);
		m_BackupFGFootGroup->RemoveAllAtoms();
	});
	MatchProperty("BGFootGroup", {
		delete m_pBGFootGroup;
		m_pBGFootGroup = new AtomGroup();
		reader >> m_pBGFootGroup;
		m_pBGFootGroup->SetOwner(this);
		m_BackupBGFootGroup = new AtomGroup(*m_pBGFootGroup);
		m_BackupBGFootGroup->RemoveAllAtoms();
	});
	MatchProperty("MaxWalkPathCrouchShift", { reader >> m_MaxWalkPathCrouchShift; });
	MatchProperty("StrideSound", {
		m_StrideSound = new SoundContainer;
		reader >> m_StrideSound;
	});
	MatchProperty("StandLimbPath", { reader >> m_Paths[FGROUND][STAND]; });
	MatchProperty("StandLimbPathBG", { reader >> m_Paths[BGROUND][STAND]; });
	MatchProperty("WalkLimbPath", { reader >> m_Paths[FGROUND][WALK]; });
	MatchProperty("RunLimbPath", { reader >> m_Paths[FGROUND][RUN]; });
	MatchProperty("CrouchLimbPath", { reader >> m_Paths[FGROUND][PRONE]; });
	MatchProperty("CrouchLimbPathBG", { reader >> m_Paths[BGROUND][PRONE]; });
	MatchProperty("CrawlLimbPath", { reader >> m_Paths[FGROUND][CRAWL]; });
	MatchProperty("ArmCrawlLimbPath", { reader >> m_Paths[FGROUND][ARMCRAWL]; });
	MatchProperty("ClimbLimbPath", { reader >> m_Paths[FGROUND][CLIMB]; });
	MatchProperty("JumpLimbPath", { reader >> m_Paths[FGROUND][JUMP]; });
	MatchProperty("DislodgeLimbPath", { reader >> m_Paths[FGROUND][DISLODGE]; });

	MatchProperty("StandRotAngleTarget", { reader >> m_RotAngleTargets[STAND]; });
	MatchProperty("WalkRotAngleTarget", { reader >> m_RotAngleTargets[WALK]; });
	MatchProperty("RunRotAngleTarget", { reader >> m_RotAngleTargets[RUN]; });
	MatchProperty("CrouchRotAngleTarget", { reader >> m_RotAngleTargets[CROUCH]; });
	MatchProperty("JumpRotAngleTarget", { reader >> m_RotAngleTargets[JUMP]; });

	EndPropertyList;
}

int AHuman::Save(Writer& writer) const {
	Actor::Save(writer);

	writer.NewProperty("ThrowPrepTime");
	writer << m_ThrowPrepTime;
	writer.NewProperty("Head");
	writer << m_pHead;
	writer.NewProperty("LookToAimRatio");
	writer << m_LookToAimRatio;
	writer.NewProperty("Jetpack");
	writer << m_pJetpack;
	writer.NewProperty("FGArmFlailScalar");
	writer << m_FGArmFlailScalar;
	writer.NewProperty("BGArmFlailScalar");
	writer << m_BGArmFlailScalar;
	writer.NewProperty("ArmSwingRate");
	writer << m_ArmSwingRate;
	writer.NewPropertyWithValue("DeviceArmSwayRate", m_DeviceArmSwayRate);
	writer.NewProperty("FGArm");
	writer << m_pFGArm;
	writer.NewProperty("BGArm");
	writer << m_pBGArm;
	writer.NewProperty("FGLeg");
	writer << m_pFGLeg;
	writer.NewProperty("BGLeg");
	writer << m_pBGLeg;
	writer.NewProperty("HandGroup");
	writer << m_pFGHandGroup;
	writer.NewProperty("FGFootGroup");
	writer << m_pFGFootGroup;
	writer.NewProperty("BGFootGroup");
	writer << m_pBGFootGroup;
	writer.NewProperty("MaxWalkPathCrouchShift");
	writer << m_MaxWalkPathCrouchShift;
	writer.NewProperty("StrideSound");
	writer << m_StrideSound;

	writer.NewProperty("StandLimbPath");
	writer << m_Paths[FGROUND][STAND];
	writer.NewProperty("StandLimbPathBG");
	writer << m_Paths[BGROUND][STAND];
	writer.NewProperty("WalkLimbPath");
	writer << m_Paths[FGROUND][WALK];
	writer.NewProperty("RunLimbPath");
	writer << m_Paths[FGROUND][RUN];
	writer.NewProperty("CrouchLimbPath");
	writer << m_Paths[FGROUND][PRONE];
	writer.NewProperty("CrawlLimbPath");
	writer << m_Paths[FGROUND][CRAWL];
	writer.NewProperty("ArmCrawlLimbPath");
	writer << m_Paths[FGROUND][ARMCRAWL];
	writer.NewProperty("ClimbLimbPath");
	writer << m_Paths[FGROUND][CLIMB];
	writer.NewProperty("JumpLimbPath");
	writer << m_Paths[FGROUND][JUMP];
	writer.NewProperty("DislodgeLimbPath");
	writer << m_Paths[FGROUND][DISLODGE];

	writer.NewPropertyWithValue("StandRotAngleTarget", m_RotAngleTargets[STAND]);
	writer.NewPropertyWithValue("WalkRotAngleTarget", m_RotAngleTargets[WALK]);
	writer.NewPropertyWithValue("RunRotAngleTarget", m_RotAngleTargets[RUN]);
	writer.NewPropertyWithValue("CrouchRotAngleTarget", m_RotAngleTargets[PRONE]);
	writer.NewPropertyWithValue("JumpRotAngleTarget", m_RotAngleTargets[JUMP]);

	return 0;
}

void AHuman::Destroy(bool notInherited) {
	delete m_pFGHandGroup;
	delete m_pBGHandGroup;
	delete m_pFGFootGroup;
	delete m_pBGFootGroup;

	delete m_StrideSound;

	if (!notInherited) {
		Actor::Destroy();
	}
	Clear();
}

float AHuman::GetTotalValue(int nativeModule, float foreignMult, float nativeMult) const {
	float totalValue = Actor::GetTotalValue(nativeModule, foreignMult, nativeMult);

	// If holding something, then add its value, too
	if (m_pFGArm && m_pFGArm->IsAttached() && m_pFGArm->GetHeldDevice())
		totalValue += m_pFGArm->GetHeldDevice()->GetTotalValue(nativeModule, foreignMult, nativeMult);

	return totalValue;
}

bool AHuman::HasObject(std::string objectName) const {
	bool found = Actor::HasObject(objectName);

	// If holding something, then check that too
	if (m_pFGArm && m_pFGArm->IsAttached() && m_pFGArm->GetHeldDevice())
		found = found || m_pFGArm->GetHeldDevice()->HasObject(objectName);
	if (m_pBGArm && m_pBGArm->IsAttached() && m_pBGArm->GetHeldDevice())
		found = found || m_pBGArm->GetHeldDevice()->HasObject(objectName);

	return found;
}

bool AHuman::HasObjectInGroup(std::string groupName) const {
	bool found = Actor::HasObjectInGroup(groupName);

	// If holding something, then check that too
	if (m_pFGArm && m_pFGArm->IsAttached() && m_pFGArm->GetHeldDevice())
		found = found || m_pFGArm->GetHeldDevice()->HasObjectInGroup(groupName);
	if (m_pBGArm && m_pBGArm->IsAttached() && m_pBGArm->GetHeldDevice())
		found = found || m_pBGArm->GetHeldDevice()->HasObjectInGroup(groupName);

	return found;
}

Vector AHuman::GetCPUPos() const {
	if (m_pHead && m_pHead->IsAttached())
		return m_Pos + ((m_pHead->GetParentOffset().GetXFlipped(m_HFlipped) * m_Rotation) * 1.5);

	return m_Pos;
}

Vector AHuman::GetEyePos() const {
	if (m_pHead && m_pHead->IsAttached()) {
		return m_Pos + m_pHead->GetParentOffset() * 1.2F;
	}

	return m_Pos;
}

void AHuman::SetHead(Attachable* newHead) {
	if (m_pHead && m_pHead->IsAttached()) {
		RemoveAndDeleteAttachable(m_pHead);
	}
	if (newHead == nullptr) {
		m_pHead = nullptr;
	} else {
		m_pHead = newHead;
		AddAttachable(newHead);

		m_HardcodedAttachableUniqueIDsAndSetters.insert({newHead->GetUniqueID(), [](MOSRotating* parent, Attachable* attachable) {
			                                                 dynamic_cast<AHuman*>(parent)->SetHead(attachable);
		                                                 }});

		if (m_pHead->HasNoSetDamageMultiplier()) {
			m_pHead->SetDamageMultiplier(4.0F);
		}
		if (m_pHead->IsDrawnAfterParent()) {
			m_pHead->SetDrawnNormallyByParent(false);
		}
		m_pHead->SetInheritsRotAngle(false);
	}
}

void AHuman::SetJetpack(AEJetpack* newJetpack) {
	if (m_pJetpack && m_pJetpack->IsAttached()) {
		RemoveAndDeleteAttachable(m_pJetpack);
	}
	if (newJetpack == nullptr) {
		m_pJetpack = nullptr;
	} else {
		m_pJetpack = newJetpack;
		AddAttachable(newJetpack);

		m_HardcodedAttachableUniqueIDsAndSetters.insert({newJetpack->GetUniqueID(), [](MOSRotating* parent, Attachable* attachable) {
			                                                 AEJetpack* castedAttachable = dynamic_cast<AEJetpack*>(attachable);
			                                                 RTEAssert(!attachable || castedAttachable, "Tried to pass incorrect Attachable subtype " + (attachable ? attachable->GetClassName() : "") + " to SetJetpack");
			                                                 dynamic_cast<AHuman*>(parent)->SetJetpack(castedAttachable);
		                                                 }});

		if (m_pJetpack->HasNoSetDamageMultiplier()) {
			m_pJetpack->SetDamageMultiplier(0.0F);
		}
		m_pJetpack->SetApplyTransferredForcesAtOffset(false);
	}
}

void AHuman::SetFGArm(Arm* newArm) {
	if (m_pFGArm && m_pFGArm->IsAttached()) {
		RemoveAndDeleteAttachable(m_pFGArm);
	}
	if (newArm == nullptr) {
		m_pFGArm = nullptr;
	} else {
		m_pFGArm = newArm;
		AddAttachable(newArm);

		m_HardcodedAttachableUniqueIDsAndSetters.insert({newArm->GetUniqueID(), [](MOSRotating* parent, Attachable* attachable) {
			                                                 Arm* castedAttachable = dynamic_cast<Arm*>(attachable);
			                                                 RTEAssert(!attachable || castedAttachable, "Tried to pass incorrect Attachable subtype " + (attachable ? attachable->GetClassName() : "") + " to SetFGArm");
			                                                 dynamic_cast<AHuman*>(parent)->SetFGArm(castedAttachable);
		                                                 }});

		if (m_pFGArm->HasNoSetDamageMultiplier()) {
			m_pFGArm->SetDamageMultiplier(1.0F);
		}
		m_pFGArm->SetDrawnAfterParent(true);
		m_pFGArm->SetDrawnNormallyByParent(false);
	}
}

void AHuman::SetBGArm(Arm* newArm) {
	if (m_pBGArm && m_pBGArm->IsAttached()) {
		RemoveAndDeleteAttachable(m_pBGArm);
	}
	if (newArm == nullptr) {
		m_pBGArm = nullptr;
	} else {
		m_pBGArm = newArm;
		AddAttachable(newArm);

		m_HardcodedAttachableUniqueIDsAndSetters.insert({newArm->GetUniqueID(), [](MOSRotating* parent, Attachable* attachable) {
			                                                 Arm* castedAttachable = dynamic_cast<Arm*>(attachable);
			                                                 RTEAssert(!attachable || castedAttachable, "Tried to pass incorrect Attachable subtype " + (attachable ? attachable->GetClassName() : "") + " to SetBGArm");
			                                                 dynamic_cast<AHuman*>(parent)->SetBGArm(castedAttachable);
		                                                 }});

		if (m_pBGArm->HasNoSetDamageMultiplier()) {
			m_pBGArm->SetDamageMultiplier(1.0F);
		}
		m_pBGArm->SetDrawnAfterParent(false);
	}
}

void AHuman::SetFGLeg(Leg* newLeg) {
	if (m_pFGLeg && m_pFGLeg->IsAttached()) {
		RemoveAndDeleteAttachable(m_pFGLeg);
	}
	if (newLeg == nullptr) {
		m_pFGLeg = nullptr;
	} else {
		m_pFGLeg = newLeg;
		AddAttachable(newLeg);

		m_HardcodedAttachableUniqueIDsAndSetters.insert({newLeg->GetUniqueID(), [](MOSRotating* parent, Attachable* attachable) {
			                                                 Leg* castedAttachable = dynamic_cast<Leg*>(attachable);
			                                                 RTEAssert(!attachable || castedAttachable, "Tried to pass incorrect Attachable subtype " + (attachable ? attachable->GetClassName() : "") + " to SetFGLeg");
			                                                 dynamic_cast<AHuman*>(parent)->SetFGLeg(castedAttachable);
		                                                 }});

		if (m_pFGLeg->HasNoSetDamageMultiplier()) {
			m_pFGLeg->SetDamageMultiplier(1.0F);
		}
	}
}

void AHuman::SetBGLeg(Leg* newLeg) {
	if (m_pBGLeg && m_pBGLeg->IsAttached()) {
		RemoveAndDeleteAttachable(m_pBGLeg);
	}
	if (newLeg == nullptr) {
		m_pBGLeg = nullptr;
	} else {
		m_pBGLeg = newLeg;
		AddAttachable(newLeg);

		m_HardcodedAttachableUniqueIDsAndSetters.insert({newLeg->GetUniqueID(), [](MOSRotating* parent, Attachable* attachable) {
			                                                 Leg* castedAttachable = dynamic_cast<Leg*>(attachable);
			                                                 RTEAssert(!attachable || castedAttachable, "Tried to pass incorrect Attachable subtype " + (attachable ? attachable->GetClassName() : "") + " to SetBGLeg");
			                                                 dynamic_cast<AHuman*>(parent)->SetBGLeg(castedAttachable);
		                                                 }});

		if (m_pBGLeg->HasNoSetDamageMultiplier()) {
			m_pBGLeg->SetDamageMultiplier(1.0F);
		}
		m_pBGLeg->SetDrawnAfterParent(false);
	}
}

BITMAP* AHuman::GetGraphicalIcon() const {
	return m_GraphicalIcon ? m_GraphicalIcon : (m_pHead ? m_pHead->GetSpriteFrame(0) : GetSpriteFrame(0));
}

bool AHuman::HandlePieCommand(PieSliceType pieSliceIndex) {
	if (pieSliceIndex != PieSliceType::NoType) {
		if (pieSliceIndex == PieSliceType::Pickup) {
			m_Controller.SetState(WEAPON_PICKUP);
		} else if (pieSliceIndex == PieSliceType::Drop) {
			m_Controller.SetState(WEAPON_DROP);
		} else if (pieSliceIndex == PieSliceType::Reload) {
			m_Controller.SetState(WEAPON_RELOAD);
		} else if (pieSliceIndex == PieSliceType::NextItem) {
			m_Controller.SetState(WEAPON_CHANGE_NEXT, true);
		} else if (pieSliceIndex == PieSliceType::PreviousItem) {
			m_Controller.SetState(WEAPON_CHANGE_PREV, true);
		} else if (pieSliceIndex == PieSliceType::Sentry) {
			m_AIMode = AIMODE_SENTRY;
		} else if (pieSliceIndex == PieSliceType::Patrol) {
			m_AIMode = AIMODE_PATROL;
		} else if (pieSliceIndex == PieSliceType::BrainHunt) {
			m_AIMode = AIMODE_BRAINHUNT;
			ClearAIWaypoints();
		} else if (pieSliceIndex == PieSliceType::GoTo) {
			m_AIMode = AIMODE_GOTO;
			ClearAIWaypoints();
		} else if (pieSliceIndex == PieSliceType::GoldDig) {
			m_AIMode = AIMODE_GOLDDIG;
		} else {
			return Actor::HandlePieCommand(pieSliceIndex);
		}
	}
	return false;
}

void AHuman::AddInventoryItem(MovableObject* pItemToAdd) {
	// If we have nothing in inventory, and nothing in our hands, just grab this first thing added to us.
	if (HeldDevice* itemToAddAsHeldDevice = dynamic_cast<HeldDevice*>(pItemToAdd); itemToAddAsHeldDevice && m_Inventory.empty() && m_pFGArm && m_pFGArm->IsAttached() && !m_pFGArm->GetHeldDevice()) {
		m_pFGArm->SetHeldDevice(itemToAddAsHeldDevice);
		m_pFGArm->SetHandPos(m_HolsterOffset.GetXFlipped(m_HFlipped));
	} else {
		Actor::AddInventoryItem(pItemToAdd);
	}

	EquipShieldInBGArm();
}

MovableObject* AHuman::SwapNextInventory(MovableObject* inventoryItemToSwapIn, bool muteSound) {
	MovableObject* swappedInventoryItem = Actor::SwapNextInventory(inventoryItemToSwapIn, muteSound);
	while (!dynamic_cast<HeldDevice*>(swappedInventoryItem) && !m_Inventory.empty()) {
		g_MovableMan.AddMO(swappedInventoryItem);
		swappedInventoryItem = Actor::SwapNextInventory(nullptr, muteSound);
	}

	return swappedInventoryItem;
}

MovableObject* AHuman::SwapPrevInventory(MovableObject* inventoryItemToSwapIn) {
	MovableObject* swappedInventoryItem = Actor::SwapPrevInventory(inventoryItemToSwapIn);
	while (!dynamic_cast<HeldDevice*>(swappedInventoryItem) && !m_Inventory.empty()) {
		g_MovableMan.AddMO(swappedInventoryItem);
		swappedInventoryItem = Actor::SwapNextInventory(nullptr);
	}

	return swappedInventoryItem;
}

bool AHuman::EquipFirearm(bool doEquip) {
	if (!(m_pFGArm && m_pFGArm->IsAttached())) {
		return false;
	}

	if (HDFirearm* heldDeviceAsFirearm = dynamic_cast<HDFirearm*>(m_pFGArm->GetHeldDevice()); heldDeviceAsFirearm && heldDeviceAsFirearm->IsWeapon()) {
		return true;
	} else {
		UnequipBGArm();
	}

	// Go through the inventory looking for the proper device
	for (std::deque<MovableObject*>::iterator itr = m_Inventory.begin(); itr != m_Inventory.end(); ++itr) {
		HDFirearm* pWeapon = dynamic_cast<HDFirearm*>(*itr);
		// Found proper device to equip, so make the switch!
		if (pWeapon && pWeapon->IsWeapon()) {
			if (doEquip) {
				// Erase the inventory entry containing the device we now have switched to
				*itr = 0;
				m_Inventory.erase(itr);

				// Put back into the inventory what we had in our hands, if anything
				if (HeldDevice* heldDevice = m_pFGArm->GetHeldDevice()) {
					heldDevice->Deactivate();
					AddToInventoryBack(m_pFGArm->RemoveAttachable(heldDevice));
				}

				// Now put the device we were looking for and found into the hand
				m_pFGArm->SetHeldDevice(pWeapon);
				// Move the hand to a poisition so it looks like the new device was drawn from inventory
				m_pFGArm->SetHandPos(m_Pos + m_HolsterOffset.GetXFlipped(m_HFlipped));

				// Equip shield in BG arm if applicable
				EquipShieldInBGArm();

				// Play the device switching sound
				if (m_DeviceSwitchSound) {
					m_DeviceSwitchSound->Play(m_Pos);
				}
			}

			return true;
		}
	}

	return false;
}

bool AHuman::EquipDeviceInGroup(std::string group, bool doEquip) {
	if (!(m_pFGArm && m_pFGArm->IsAttached())) {
		return false;
	}

	if (HeldDevice* heldDevice = m_pFGArm->GetHeldDevice(); heldDevice && heldDevice->IsInGroup(group)) {
		return true;
	}

	// Go through the inventory looking for the proper device
	for (std::deque<MovableObject*>::iterator itr = m_Inventory.begin(); itr != m_Inventory.end(); ++itr) {
		HeldDevice* pDevice = dynamic_cast<HeldDevice*>(*itr);
		// Found proper device to equip, so make the switch!
		if (pDevice && pDevice->IsInGroup(group)) {
			if (doEquip) {
				// Erase the inventory entry containing the device we now have switched to
				*itr = 0;
				m_Inventory.erase(itr);

				// Put back into the inventory what we had in our hands, if anything
				if (HeldDevice* heldDevice = m_pFGArm->GetHeldDevice()) {
					heldDevice->Deactivate();
					MovableObject* previouslyHeldItem = m_pFGArm->RemoveAttachable(heldDevice);
					if (previouslyHeldItem) {
						// Note - This is a fix to deal with an edge case bug when this method is called by a global script.
						// Because the global script runs before everything has finished traveling, the removed item needs to undraw itself from the MO layer, otherwise it can result in ghost collisions and crashes.
						if (previouslyHeldItem->GetsHitByMOs()) {
							previouslyHeldItem->SetTraveling(true);
						}

						AddToInventoryBack(previouslyHeldItem);
					}
				}

				// Now put the device we were looking for and found into the hand
				m_pFGArm->SetHeldDevice(pDevice);
				// Move the hand to a poisition so it looks like the new device was drawn from inventory
				m_pFGArm->SetHandPos(m_Pos + m_HolsterOffset.GetXFlipped(m_HFlipped));

				// Equip shield in BG arm if applicable
				EquipShieldInBGArm();

				// Play the device switching sound
				if (m_DeviceSwitchSound) {
					m_DeviceSwitchSound->Play(m_Pos);
				}
			}

			return true;
		}
	}

	return false;
}

bool AHuman::EquipLoadedFirearmInGroup(std::string group, std::string excludeGroup, bool doEquip) {
	if (!(m_pFGArm && m_pFGArm->IsAttached())) {
		return false;
	}

	if (HeldDevice* heldDevice = m_pFGArm->GetHeldDevice(); heldDevice && !heldDevice->NeedsReloading() && heldDevice->IsInGroup(group) && !heldDevice->IsInGroup(excludeGroup)) {
		return true;
	}

	// Go through the inventory looking for the proper device
	for (std::deque<MovableObject*>::iterator itr = m_Inventory.begin(); itr != m_Inventory.end(); ++itr) {
		HDFirearm* pFirearm = dynamic_cast<HDFirearm*>(*itr);
		// Found proper device to equip, so make the switch!
		if (pFirearm && !pFirearm->NeedsReloading() && pFirearm->IsInGroup(group) && !pFirearm->IsInGroup(excludeGroup)) {
			if (doEquip) {
				// Erase the inventory entry containing the device we now have switched to
				*itr = 0;
				m_Inventory.erase(itr);

				// Put back into the inventory what we had in our hands, if anything
				if (HeldDevice* heldDevice = m_pFGArm->GetHeldDevice()) {
					m_pFGArm->GetHeldDevice()->Deactivate();
					AddToInventoryBack(m_pFGArm->RemoveAttachable(heldDevice));
				}

				// Now put the device we were looking for and found into the hand
				m_pFGArm->SetHeldDevice(pFirearm);
				// Move the hand to a poisition so it looks like the new device was drawn from inventory
				m_pFGArm->SetHandPos(m_Pos + m_HolsterOffset.GetXFlipped(m_HFlipped));

				// Equip shield in BG arm if applicable
				EquipShieldInBGArm();

				// Play the device switching sound
				if (m_DeviceSwitchSound) {
					m_DeviceSwitchSound->Play(m_Pos);
				}
			}

			return true;
		}
	}

	return false;
}

bool AHuman::EquipNamedDevice(const std::string& moduleName, const std::string& presetName, bool doEquip) {
	if (!(m_pFGArm && m_pFGArm->IsAttached())) {
		return false;
	}

	if (const HeldDevice* heldDevice = m_pFGArm->GetHeldDevice();
	    heldDevice && (moduleName.empty() || heldDevice->GetModuleName() == moduleName) && heldDevice->GetPresetName() == presetName) {
		return true;
	}

	// Go through the inventory looking for the proper device
	for (std::deque<MovableObject*>::iterator itr = m_Inventory.begin(); itr != m_Inventory.end(); ++itr) {
		HeldDevice* pDevice = dynamic_cast<HeldDevice*>(*itr);
		// Found proper device to equip, so make the switch!
		if (pDevice && (moduleName.empty() || pDevice->GetModuleName() == moduleName) && pDevice->GetPresetName() == presetName) {
			if (doEquip) {
				// Erase the inventory entry containing the device we now have switched to
				*itr = 0;
				m_Inventory.erase(itr);

				// Put back into the inventory what we had in our hands, if anything
				if (HeldDevice* heldDevice = m_pFGArm->GetHeldDevice()) {
					heldDevice->Deactivate();
					AddToInventoryBack(m_pFGArm->RemoveAttachable(heldDevice));
				}

				// Now put the device we were looking for and found into the hand
				m_pFGArm->SetHeldDevice(pDevice);
				// Move the hand to a poisition so it looks like the new device was drawn from inventory
				m_pFGArm->SetHandPos(m_Pos + m_HolsterOffset.GetXFlipped(m_HFlipped));

				// Equip shield in BG arm if applicable
				EquipShieldInBGArm();

				// Play the device switching sound
				if (m_DeviceSwitchSound) {
					m_DeviceSwitchSound->Play(m_Pos);
				}
			}

			return true;
		}
	}

	return false;
}

bool AHuman::EquipThrowable(bool doEquip) {
	if (!(m_pFGArm && m_pFGArm->IsAttached())) {
		return false;
	}

	if (dynamic_cast<ThrownDevice*>(m_pFGArm->GetHeldDevice())) {
		return true;
	}

	// Go through the inventory looking for the proper device
	for (std::deque<MovableObject*>::iterator itr = m_Inventory.begin(); itr != m_Inventory.end(); ++itr) {
		ThrownDevice* pThrown = dynamic_cast<ThrownDevice*>(*itr);
		// Found proper device to equip, so make the switch!
		// TODO: see if thrown is weapon or not, don't want to throw key items etc
		if (pThrown) // && pThrown->IsWeapon())
		{
			if (doEquip) {
				// Erase the inventory entry containing the device we now have switched to
				*itr = 0;
				m_Inventory.erase(itr);

				// Put back into the inventory what we had in our hands, if anything
				if (HeldDevice* heldDevice = m_pFGArm->GetHeldDevice()) {
					heldDevice->Deactivate();
					AddToInventoryBack(m_pFGArm->RemoveAttachable(heldDevice));
				}

				// Now put the device we were looking for and found into the hand
				m_pFGArm->SetHeldDevice(pThrown);
				// Move the hand to a poisition so it looks like the new device was drawn from inventory
				m_pFGArm->SetHandPos(m_Pos + m_HolsterOffset.GetXFlipped(m_HFlipped));

				// Equip shield in BG arm as applicable
				EquipShieldInBGArm();

				// Play the device switching sound
				if (m_DeviceSwitchSound) {
					m_DeviceSwitchSound->Play(m_Pos);
				}
			}

			return true;
		}
	}

	return false;
}

bool AHuman::EquipDiggingTool(bool doEquip) {
	if (!(m_pFGArm && m_pFGArm->IsAttached())) {
		return false;
	}

	const HDFirearm* strongestDigger = nullptr;
	float strongestDiggerDigStrength = 0;
	bool strongestDiggerIsHeld = false;
	if (const HDFirearm* heldDeviceAsFirearm = dynamic_cast<HDFirearm*>(m_pFGArm->GetHeldDevice()); heldDeviceAsFirearm && heldDeviceAsFirearm->IsInGroup("Tools - Diggers")) {
		strongestDigger = heldDeviceAsFirearm;
		strongestDiggerDigStrength = heldDeviceAsFirearm->EstimateDigStrength();
		strongestDiggerIsHeld = true;
	}

	if (doEquip || !strongestDigger) {
		for (MovableObject* inventoryItem: m_Inventory) {
			if (const HDFirearm* inventoryItemAsFirearm = dynamic_cast<HDFirearm*>(inventoryItem); inventoryItemAsFirearm && inventoryItemAsFirearm->IsInGroup("Tools - Diggers") && inventoryItemAsFirearm->EstimateDigStrength() > strongestDiggerDigStrength) {
				strongestDigger = inventoryItemAsFirearm;
				strongestDiggerDigStrength = inventoryItemAsFirearm->EstimateDigStrength();
				strongestDiggerIsHeld = false;
			}
		}
	}

	if (doEquip && strongestDigger && !strongestDiggerIsHeld) {
		EquipNamedDevice(strongestDigger->GetModuleName(), strongestDigger->GetPresetName(), true);
	}

	return strongestDigger != nullptr;
}

float AHuman::EstimateDigStrength() const {
	float maxPenetration = Actor::EstimateDigStrength();

	if (!(m_pFGArm && m_pFGArm->IsAttached())) {
		return maxPenetration;
	}

	if (const HDFirearm* heldDeviceAsHDFirearm = dynamic_cast<HDFirearm*>(m_pFGArm->GetHeldDevice()); heldDeviceAsHDFirearm && heldDeviceAsHDFirearm->IsInGroup("Tools - Diggers")) {
		maxPenetration = std::max(heldDeviceAsHDFirearm->EstimateDigStrength(), maxPenetration);
	}

	for (const MovableObject* inventoryItem: m_Inventory) {
		if (const HDFirearm* inventoryItemAsFirearm = dynamic_cast<const HDFirearm*>(inventoryItem); inventoryItemAsFirearm && inventoryItemAsFirearm->IsInGroup("Tools - Diggers")) {
			maxPenetration = std::max(inventoryItemAsFirearm->EstimateDigStrength(), maxPenetration);
		}
	}

	return maxPenetration;
}

float AHuman::EstimateJumpHeight() const {
	if (!m_pJetpack) {
		return 0.0F;
	}

	float totalMass = GetMass();
	float fuelTime = m_pJetpack->GetJetTimeTotal();
	float fuelUseMultiplier = m_pJetpack->GetThrottleFactor();
	float impulseBurst = m_pJetpack->EstimateImpulse(true) / totalMass;
	float impulseThrust = m_pJetpack->EstimateImpulse(false) / totalMass;

	Vector globalAcc = g_SceneMan.GetGlobalAcc() * g_TimerMan.GetDeltaTimeSecs();
	Vector currentVelocity = Vector(0.0F, -impulseBurst);
	float totalHeight = currentVelocity.GetY() * g_TimerMan.GetDeltaTimeSecs() * c_PPM;
	do {
		currentVelocity += globalAcc;
		totalHeight += currentVelocity.GetY() * g_TimerMan.GetDeltaTimeSecs() * c_PPM;
		if (fuelTime > 0.0F) {
			currentVelocity.m_Y -= impulseThrust;
			fuelTime -= g_TimerMan.GetDeltaTimeMS() * fuelUseMultiplier;
		}
	} while (currentVelocity.GetY() < 0.0F);

	float finalCalculatedHeight = totalHeight * -1.0F * c_MPP;
	float finalHeightMultipler = 0.8f; // Make us think we can do a little less because AI path following is shit
	return finalCalculatedHeight * finalHeightMultipler;
}

bool AHuman::EquipShield() {
	if (!(m_pFGArm && m_pFGArm->IsAttached())) {
		return false;
	}

	if (HeldDevice* heldDevice = m_pFGArm->GetHeldDevice(); heldDevice && heldDevice->IsShield()) {
		return true;
	}

	// Go through the inventory looking for the proper device
	for (std::deque<MovableObject*>::iterator itr = m_Inventory.begin(); itr != m_Inventory.end(); ++itr) {
		HeldDevice* pShield = dynamic_cast<HeldDevice*>(*itr);
		// Found proper device to equip, so make the switch!
		if (pShield && pShield->IsShield()) {
			// Erase the inventory entry containing the device we now have switched to
			*itr = 0;
			m_Inventory.erase(itr);

			// Put back into the inventory what we had in our hands, if anything
			if (HeldDevice* heldDevice = m_pFGArm->GetHeldDevice()) {
				heldDevice->Deactivate();
				AddToInventoryBack(m_pFGArm->RemoveAttachable(heldDevice));
			}

			// Now put the device we were looking for and found into the hand
			m_pFGArm->SetHeldDevice(pShield);
			// Move the hand to a poisition so it looks like the new device was drawn from inventory
			m_pFGArm->SetHandPos(m_Pos + m_HolsterOffset.GetXFlipped(m_HFlipped));

			// Equip shield in BG arm is applicable
			EquipShieldInBGArm();

			// Play the device switching sound
			if (m_DeviceSwitchSound) {
				m_DeviceSwitchSound->Play(m_Pos);
			}

			return true;
		}
	}

	return false;
}

bool AHuman::EquipShieldInBGArm() {
	if (!(m_pBGArm && m_pBGArm->IsAttached())) {
		return false;
	}

	if (HeldDevice* heldDevice = m_pBGArm->GetHeldDevice(); heldDevice && (heldDevice->IsShield() || heldDevice->IsDualWieldable())) {
		// If we're holding a shield, but aren't supposed to, because we need to support the FG hand's two-handed device, then let go of the shield and put it back in inventory.
		if (m_pFGArm && m_pFGArm->IsAttached() && m_pFGArm->GetHeldDevice() && !m_pFGArm->GetHeldDevice()->IsOneHanded()) {
			m_pBGArm->GetHeldDevice()->Deactivate();
			AddToInventoryBack(m_pBGArm->RemoveAttachable(heldDevice));
			return false;
		}
		return true;
	}

	// Only equip if the BG hand isn't occupied with supporting a two handed device
	if (m_pFGArm && m_pFGArm->IsAttached() && m_pFGArm->GetHeldDevice() && !m_pFGArm->GetHeldDevice()->IsOneHanded()) {
		return false;
	}

	// Go through the inventory looking for the proper device
	for (std::deque<MovableObject*>::iterator itr = m_Inventory.begin(); itr != m_Inventory.end(); ++itr) {
		HeldDevice* pShield = dynamic_cast<HeldDevice*>(*itr);
		// Found proper device to equip, so make the switch!
		if (pShield && (pShield->IsShield() || pShield->IsDualWieldable())) {
			// Erase the inventory entry containing the device we now have switched to
			*itr = 0;
			m_Inventory.erase(itr);

			// Put back into the inventory what we had in our hands, if anything
			if (HeldDevice* heldDevice = m_pBGArm->GetHeldDevice()) {
				heldDevice->Deactivate();
				AddToInventoryBack(m_pBGArm->RemoveAttachable(heldDevice));
			}

			// Now put the device we were looking for and found into the hand
			m_pBGArm->SetHeldDevice(pShield);
			// Move the hand to a poisition so it looks like the new device was drawn from inventory
			m_pBGArm->SetHandPos(m_Pos + m_HolsterOffset.GetXFlipped(m_HFlipped));

			if (m_DeviceSwitchSound) {
				m_DeviceSwitchSound->Play(m_Pos);
			}

			return true;
		}
	}

	return false;
}

bool AHuman::UnequipFGArm() {
	if (m_pFGArm) {
		if (HeldDevice* heldDevice = m_pFGArm->GetHeldDevice()) {
			heldDevice->Deactivate();
			AddToInventoryBack(m_pFGArm->RemoveAttachable(heldDevice));
			m_pFGArm->SetHandPos(m_Pos + RotateOffset(m_HolsterOffset));
			return true;
		}
	}
	return false;
}

bool AHuman::UnequipBGArm() {
	if (m_pBGArm) {
		if (HeldDevice* heldDevice = m_pBGArm->GetHeldDevice()) {
			heldDevice->Deactivate();
			AddToInventoryFront(m_pBGArm->RemoveAttachable(heldDevice));
			m_pBGArm->SetHandPos(m_Pos + RotateOffset(m_HolsterOffset));
			return true;
		}
	}
	return false;
}

float AHuman::GetEquippedMass() const {
	float equippedMass = 0;
	if (MovableObject* fgDevice = GetEquippedItem()) {
		equippedMass += fgDevice->GetMass();
	}
	if (MovableObject* bgDevice = GetEquippedBGItem()) {
		equippedMass += bgDevice->GetMass();
	}
	return equippedMass;
}

bool AHuman::FirearmIsReady() const {
	// Check if the currently held device is already the desired type
	if (m_pFGArm && m_pFGArm->IsAttached()) {
		const HDFirearm* pWeapon = dynamic_cast<HDFirearm*>(m_pFGArm->GetHeldDevice());
		if (pWeapon && pWeapon->GetRoundInMagCount() != 0)
			return true;
	}

	return false;
}

bool AHuman::ThrowableIsReady() const {
	// Check if the currently held thrown device is already the desired type
	if (m_pFGArm && m_pFGArm->IsAttached()) {
		const ThrownDevice* pThrown = dynamic_cast<ThrownDevice*>(m_pFGArm->GetHeldDevice());
		if (pThrown) // && pThrown->blah() > 0)
			return true;
	}

	return false;
}

bool AHuman::FirearmIsEmpty() const {
	if (m_pFGArm && m_pFGArm->IsAttached()) {
		const HDFirearm* pWeapon = dynamic_cast<HDFirearm*>(m_pFGArm->GetHeldDevice());
		if (pWeapon && pWeapon->GetRoundInMagCount() == 0)
			return true;
	}

	return false;
}

bool AHuman::FirearmNeedsReload() const {
	if (const HDFirearm* fgWeapon = dynamic_cast<HDFirearm*>(GetEquippedItem()); fgWeapon && fgWeapon->NeedsReloading()) {
		return true;
	}
	if (const HDFirearm* bgWeapon = dynamic_cast<HDFirearm*>(GetEquippedBGItem()); bgWeapon && bgWeapon->NeedsReloading()) {
		return true;
	}
	return false;
}

bool AHuman::FirearmsAreReloading(bool onlyIfAllFirearmsAreReloading) const {
	int reloadingFirearmCount = 0;
	int totalFirearmCount = 0;

	if (const HDFirearm* fgWeapon = dynamic_cast<HDFirearm*>(GetEquippedItem())) {
		totalFirearmCount++;
		if (fgWeapon->IsReloading()) {
			reloadingFirearmCount++;
		}
	}
	if (reloadingFirearmCount > 0 && !onlyIfAllFirearmsAreReloading) {
		return true;
	}

	if (const HDFirearm* bgWeapon = dynamic_cast<HDFirearm*>(GetEquippedBGItem())) {
		totalFirearmCount++;
		if (bgWeapon->IsReloading()) {
			reloadingFirearmCount++;
		}
	}

	return onlyIfAllFirearmsAreReloading ? reloadingFirearmCount == totalFirearmCount : reloadingFirearmCount > 0;
}

bool AHuman::FirearmIsSemiAuto() const {
	if (m_pFGArm && m_pFGArm->IsAttached()) {
		const HDFirearm* pWeapon = dynamic_cast<HDFirearm*>(m_pFGArm->GetHeldDevice());
		return pWeapon && !pWeapon->IsFullAuto();
	}
	return false;
}

void AHuman::ReloadFirearms(bool onlyReloadEmptyFirearms) {
	for (Arm* arm: {m_pFGArm, m_pBGArm}) {
		if (arm) {
			HDFirearm* heldFirearm = dynamic_cast<HDFirearm*>(arm->GetHeldDevice());
			if (!heldFirearm) {
				continue;
			}
			bool heldFirearmCanReload = heldFirearm->IsReloadable() && !heldFirearm->IsFull() && !heldFirearm->IsReloading() && (!onlyReloadEmptyFirearms || heldFirearm->IsEmpty());
			if (!heldFirearmCanReload) {
				continue;
			}

			Arm* otherArm = arm == m_pFGArm ? m_pBGArm : m_pFGArm;
			HDFirearm* otherHeldFirearm = otherArm ? dynamic_cast<HDFirearm*>(otherArm->GetHeldDevice()) : nullptr;

			bool reloadHeldFirearm = false;
			if (otherHeldFirearm && otherHeldFirearm->IsReloadable()) {
				if (heldFirearm->IsDualReloadable() && otherHeldFirearm->IsDualReloadable()) {
					reloadHeldFirearm = true;
				} else if (!otherHeldFirearm->IsReloading()) {
					reloadHeldFirearm = true;
					if (arm == m_pFGArm) {
						m_WaitingToReloadOffhand = true;
					}
				}
			} else {
				reloadHeldFirearm = true;
			}

			if (reloadHeldFirearm) {
				heldFirearm->Reload();
				if (m_DeviceSwitchSound) {
					m_DeviceSwitchSound->Play(m_Pos);
				}

				bool otherArmIsAvailable = otherArm && !otherArm->GetHeldDevice();

				// If using the support offset, other code in arm etc will handle where we should target
				otherArmIsAvailable = otherArmIsAvailable && !heldFirearm->GetUseSupportOffsetWhileReloading();

				if (otherArmIsAvailable) {
					float delayAtTarget = std::max(static_cast<float>(heldFirearm->GetReloadTime() - 200), 0.0F);
					otherArm->AddHandTarget("Magazine Pos", heldFirearm->GetMagazinePos());
					if (!m_ReloadOffset.IsZero()) {
						otherArm->AddHandTarget("Reload Offset", m_Pos + RotateOffset(m_ReloadOffset), delayAtTarget);
					} else {
						otherArm->AddHandTarget("Holster Offset", m_Pos + RotateOffset(m_HolsterOffset), delayAtTarget);
					}
				}
			}
		}
	}
}

int AHuman::FirearmActivationDelay() const {
	// Check if the currently held device is already the desired type
	if (m_pFGArm && m_pFGArm->IsAttached()) {
		const HDFirearm* pWeapon = dynamic_cast<HDFirearm*>(m_pFGArm->GetHeldDevice());
		if (pWeapon)
			return pWeapon->GetActivationDelay();
	}

	return 0;
}

bool AHuman::IsWithinRange(Vector& point) const {
	if (m_SharpAimMaxedOut)
		return true;

	Vector diff = g_SceneMan.ShortestDistance(m_Pos, point, false);
	float sqrDistance = diff.GetSqrMagnitude();

	// Really close!
	if (sqrDistance <= (m_CharHeight * m_CharHeight)) {
		return true;
	}

	float range = 0;

	if (FirearmIsReady()) {
		// Start with the default aim distance
		range = m_AimDistance;

		// Add the sharp range of the equipped weapon
		if (m_pFGArm && m_pFGArm->IsAttached())
			range += m_pFGArm->GetHeldDevice()->GetSharpLength() + 150;
	} else if (ThrowableIsReady()) {
		// TODO: make proper throw range calc based on the throwable's mass etc
		range += m_CharHeight * 4;
	}

	return sqrDistance <= (range * range);
}

bool AHuman::Look(float FOVSpread, float range) {
	if (!g_SceneMan.AnythingUnseen(m_Team) || m_CanRevealUnseen == false) {
		return false;
	}

	// Set the length of the look vector
	float aimDistance = m_AimDistance + range;
	Vector aimPos = m_Pos;

	// If aiming down the barrel, look through that
	if (m_Controller.IsState(AIM_SHARP) && m_pFGArm && m_pFGArm->IsAttached() && m_pFGArm->GetHeldDevice()) {
		aimPos = m_pFGArm->GetHeldDevice()->GetPos();
		aimDistance += m_pFGArm->GetHeldDevice()->GetSharpLength();
	}
	// If just looking, use the eyes on the head instead
	else if (m_pHead && m_pHead->IsAttached()) {
		aimPos = GetEyePos();
	}

	// Create the vector to trace along
	Vector lookVector(aimDistance, 0);
	// Set the rotation to the actual aiming angle
	Matrix aimMatrix(m_HFlipped ? -m_AimAngle : m_AimAngle);
	aimMatrix.SetXFlipped(m_HFlipped);
	lookVector *= aimMatrix;
	// Add the spread
	lookVector.DegRotate(FOVSpread * RandomNormalNum());

	// The smallest dimension of the fog block, divided by two, but always at least one, as the step for the casts
	int step = (int)g_SceneMan.GetUnseenResolution(m_Team).GetSmallest() / 2;

	// TODO: generate an alarm event if we spot an enemy actor?

	Vector ignored(0, 0);
	return g_SceneMan.CastSeeRay(m_Team, aimPos, lookVector, ignored, 25, step);
}

bool AHuman::LookForGold(float FOVSpread, float range, Vector& foundLocation) const {
	Vector ray(m_HFlipped ? -range : range, 0);
	ray.DegRotate(FOVSpread * RandomNormalNum());

	return g_SceneMan.CastMaterialRay(m_Pos, ray, g_MaterialGold, foundLocation, 4);
}

MovableObject* AHuman::LookForMOs(float FOVSpread, unsigned char ignoreMaterial, bool ignoreAllTerrain) {
	MovableObject* pSeenMO = 0;
	Vector aimPos = m_Pos;
	float aimDistance = m_AimDistance + g_FrameMan.GetPlayerScreenWidth() * 0.51; // Set the length of the look vector

	// If aiming down the barrel, look through that
	if (m_Controller.IsState(AIM_SHARP) && m_pFGArm && m_pFGArm->IsAttached() && m_pFGArm->GetHeldDevice()) {
		aimPos = m_pFGArm->GetHeldDevice()->GetPos();
		aimDistance += m_pFGArm->GetHeldDevice()->GetSharpLength();
	}
	// If just looking, use the eyes on the head instead
	else if (m_pHead && m_pHead->IsAttached()) {
		aimPos = GetEyePos();
	}

	// Create the vector to trace along
	Vector lookVector(aimDistance, 0);
	// Set the rotation to the actual aiming angle
	Matrix aimMatrix(m_HFlipped ? -m_AimAngle : m_AimAngle);
	aimMatrix.SetXFlipped(m_HFlipped);
	lookVector *= aimMatrix;
	// Add the spread
	lookVector.DegRotate(FOVSpread * RandomNormalNum());

	MOID seenMOID = g_SceneMan.CastMORay(aimPos, lookVector, m_MOID, IgnoresWhichTeam(), ignoreMaterial, ignoreAllTerrain, 5);
	pSeenMO = g_MovableMan.GetMOFromID(seenMOID);
	if (pSeenMO)
		return pSeenMO->GetRootParent();

	return pSeenMO;
}

void AHuman::ResetAllTimers() {
	Actor::ResetAllTimers();

	if (m_pFGArm && m_pFGArm->GetHeldDevice()) {
		m_pFGArm->GetHeldDevice()->ResetAllTimers();
	}
}

void AHuman::OnNewMovePath() {
	Actor::OnNewMovePath();
}

void AHuman::UpdateWalkAngle(AHuman::Layer whichLayer) {
	if (m_Controller.IsState(BODY_JUMP)) {
		m_WalkAngle[whichLayer] = Matrix(c_QuarterPI * GetFlipFactor());
	} else {
		float rayLength = 15.0F;
		Vector hipPos = m_Pos;
		if (whichLayer == AHuman::Layer::FGROUND && m_pFGLeg) {
			rayLength += m_pFGLeg->GetMaxLength();
			hipPos += RotateOffset(m_pFGLeg->GetParentOffset());
		} else if (m_pBGLeg) {
			rayLength += m_pBGLeg->GetMaxLength();
			hipPos += RotateOffset(m_pBGLeg->GetParentOffset());
		}

		// Cast a ray down from the left and right of us, to determine our angle of ascent
		// TODO Don't use a magic number here, calculate something based on stride length and maybe footgroup width.
		Vector hitPosLeft = hipPos + Vector(-10.0F, 0.0F);
		Vector hitPosRight = hipPos + Vector(10.0F, 0.0F);
		g_SceneMan.CastStrengthRay(hitPosLeft, Vector(0.0F, rayLength), 10.0F, hitPosLeft, 0, g_MaterialGrass);
		g_SceneMan.CastStrengthRay(hitPosRight, Vector(0.0F, rayLength), 10.0F, hitPosRight, 0, g_MaterialGrass);

		// Clamp the max angle, so we don't end up trying to walk at a 80 degree angle up sheer walls
		const float maxAngleDegrees = 40.0F;
		float terrainRotationDegs = std::clamp((hitPosRight - hitPosLeft).GetAbsDegAngle(), -maxAngleDegrees, maxAngleDegrees);

		//const float fastMovementAngleReduction = 10.0F;
		//const float ourMaxMovementSpeed = std::max(m_Paths[FGROUND][WALK].GetSpeed(), m_Paths[BGROUND][WALK].GetSpeed()) * 0.5F;
		//float speedRotationDegs = Lerp(0.0F, ourMaxMovementSpeed, fastMovementAngleReduction, -fastMovementAngleReduction, m_Vel.m_X);
		float speedRotationDegs = 0.0F; // TODO, this makes things stumble more although looks a bit better

		Matrix walkAngle;
		walkAngle.SetDegAngle(terrainRotationDegs + speedRotationDegs);
		m_WalkAngle[whichLayer] = walkAngle;
	}
}

void AHuman::UpdateCrouching() {
	float desiredWalkPathYOffset = 0.0F;
	if (m_CrouchAmountOverride != -1.0F) {
		// If overridden in script, use that directly
		desiredWalkPathYOffset = m_CrouchAmountOverride * m_MaxWalkPathCrouchShift;
	} else if (!m_Controller.IsState(BODY_PRONE)) {
		if (m_Controller.IsState(BODY_CROUCH)) {
			// Manually crouch fully when the crouch controller state is set
			desiredWalkPathYOffset = m_MaxWalkPathCrouchShift;
		} else if (!m_Controller.IsState(BODY_JUMP) && m_pHead) {
			// Otherwise figure out auto crouching
			// Cast a ray above our head to either side to determine whether we need to crouch
			float desiredCrouchHeadRoom = std::floor(m_pHead->GetRadius() + 2.0f);
			float toPredicted = std::floor(m_Vel.m_X * m_pHead->GetRadius()); // Check where we'll be a second from now
			Vector hitPosStart = (m_pHead->GetPos() + Vector(0.0F, m_SpriteRadius * 0.5F)).Floor();
			Vector hitPosPredictedStart = (m_pHead->GetPos() + Vector(toPredicted, m_SpriteRadius * 0.5F)).Floor();
			Vector hitPos, hitPosPredicted;
			g_SceneMan.CastStrengthRay(hitPosStart, Vector(0.0F, -desiredCrouchHeadRoom + m_SpriteRadius * -0.5F), 1.0F, hitPos, 0, g_MaterialGrass);
			g_SceneMan.CastStrengthRay(hitPosPredictedStart, Vector(0.0F, -desiredCrouchHeadRoom + m_SpriteRadius * -0.5F), 1.0F, hitPosPredicted, 0, g_MaterialGrass);

			// Don't do it if we're already hitting, we're probably in a weird spot
			if (hitPosStart.m_Y - hitPos.m_Y <= 2.0F) {
				hitPos.m_Y = 0.0F;
			}

			if (hitPosPredictedStart.m_Y - hitPosPredicted.m_Y <= 2.0F) {
				hitPosPredicted.m_Y = 0.0F;
			}

			float headroom = m_pHead->GetPos().m_Y - std::max(hitPos.m_Y, hitPosPredicted.m_Y);
			desiredWalkPathYOffset = desiredCrouchHeadRoom - headroom;
		}
	}

	float finalWalkPathYOffset = std::clamp(Lerp(0.0F, 1.0F, -m_WalkPathOffset.m_Y, desiredWalkPathYOffset, 0.3F), 0.0F, m_MaxWalkPathCrouchShift);
	m_CrouchAmount = std::clamp(0.0F, 1.0F, finalWalkPathYOffset / m_MaxWalkPathCrouchShift - 0.5F); // because it's lerped, it never hits 1 exactly. thus the -0.5F
	m_WalkPathOffset.m_Y = -finalWalkPathYOffset;

	// Adjust our X offset to try to keep our legs under our centre-of-mass
	const float ratioBetweenBodyAndHeadToAimFor = 0.15F;
	Vector headPos = m_pHead ? m_pHead->GetPos() : m_Pos;
	float predictedPosition = ((headPos.m_X - m_Pos.m_X) * ratioBetweenBodyAndHeadToAimFor) + m_Vel.m_X;
	m_WalkPathOffset.m_X = predictedPosition;
}

void AHuman::UpdateLimbPathSpeed() {
	// Reset travel speed so limbpath GetSpeed() gives us sensible values
	m_Paths[FGROUND][m_MoveState].SetTravelSpeedMultiplier(1.0F);
	m_Paths[BGROUND][m_MoveState].SetTravelSpeedMultiplier(1.0F);

	if (m_MoveState == WALK || m_MoveState == RUN || m_MoveState == CRAWL) {
		float travelSpeedMultiplier = 1.0F;
		
		// If crouching, move at reduced speed
		if (m_MoveState == WALK) {
			const float crouchSpeedMultiplier = 0.7F;
			travelSpeedMultiplier *= Lerp(0.0F, 1.0F, 1.0F, crouchSpeedMultiplier, m_CrouchAmount);
		}

		// If we're moving slowly horizontally, move at reduced speed (otherwise our legs kick about wildly as we're not yet up to speed)
		// Calculate a min multiplier that is based on the total walkpath speed (so a fast walkpath has a smaller multipler). This is so a slow walkpath gets up to speed faster
		const float ourMaxMovementSpeed = std::max(m_Paths[FGROUND][m_MoveState].GetSpeed(), m_Paths[BGROUND][m_MoveState].GetSpeed()) * 0.5F * travelSpeedMultiplier;
		const float minSpeed = 2.0F;
		const float minMultiplier = minSpeed / ourMaxMovementSpeed;
		travelSpeedMultiplier *= Lerp(0.0F, ourMaxMovementSpeed, minMultiplier, 1.0F, std::abs(m_Vel.m_X));

		m_Paths[FGROUND][m_MoveState].SetTravelSpeedMultiplier(travelSpeedMultiplier);
		m_Paths[BGROUND][m_MoveState].SetTravelSpeedMultiplier(travelSpeedMultiplier);

		// Also extend our stride depending on speed
		const float strideXMultiplier = Lerp(0.0F, ourMaxMovementSpeed, 0.8F, 1.2F, std::abs(m_Vel.m_X));
		const float strideYMultiplier = Lerp(0.0F, ourMaxMovementSpeed, 0.9F, 1.1F, std::abs(m_Vel.m_X));
		Vector scale = Vector(strideXMultiplier, strideYMultiplier);
		m_Paths[FGROUND][m_MoveState].SetScaleMultiplier(scale);
		m_Paths[BGROUND][m_MoveState].SetScaleMultiplier(scale);
	}
}

void AHuman::PreControllerUpdate() {
	ZoneScoped;

	Actor::PreControllerUpdate();

	float deltaTime = g_TimerMan.GetDeltaTimeSecs();
	float rot = m_Rotation.GetRadAngle();

	Vector analogAim = m_Controller.GetAnalogAim();
	const float analogDeadzone = 0.1F;

	m_Paths[FGROUND][m_MoveState].SetHFlip(m_HFlipped);
	m_Paths[BGROUND][m_MoveState].SetHFlip(m_HFlipped);

	if (m_pJetpack && m_pJetpack->IsAttached()) {
		m_pJetpack->UpdateBurstState(*this);

		if (m_Controller.IsState(BODY_JUMP) && !m_pJetpack->IsOutOfFuel() && m_Status != INACTIVE) {
			m_Paths[FGROUND][JUMP].Restart();
			m_Paths[BGROUND][JUMP].Restart();
		}
	}

	////////////////////////////////////
	// Movement direction

	const float movementThreshold = 1.0F;
	bool isStill = (m_Vel + m_PrevVel).MagnitudeIsLessThan(movementThreshold);
	bool isSharpAiming = m_Controller.IsState(AIM_SHARP);

	// If the pie menu is on, try to preserve whatever move state we had before it going into effect.
	// This is only done for digital input, where the user needs to use the keyboard to choose pie slices.
	// For analog input, this doesn't matter - the mouse or aiming analog stick controls the pie menu.
	bool keepOldState = m_Controller.IsKeyboardOnlyControlled() && m_Controller.IsState(PIE_MENU_ACTIVE);

	if (!keepOldState) {
		bool prone = m_Controller.IsState(BODY_PRONE);
		// Engage prone state, this makes the body's rotational spring pull it horizontal instead of upright.
		if (prone && m_ProneState == NOTPRONE) {
			m_ProneState = GOPRONE;
			m_ProneTimer.Reset();
		}
		if ((m_Controller.IsState(MOVE_RIGHT) || m_Controller.IsState(MOVE_LEFT) || m_MoveState == JUMP) && m_Status != INACTIVE) {
			for (int i = WALK; i < MOVEMENTSTATECOUNT; ++i) {
				m_Paths[FGROUND][i].SetHFlip(m_HFlipped);
				m_Paths[BGROUND][i].SetHFlip(m_HFlipped);
			}
			// Only if not jumping, OR if jumping, and apparently stuck on something - then help out with the limbs.
			if (m_MoveState != JUMP || isStill) {
				MovementState oldMoveState = m_MoveState;
				if (!m_ProneState == NOTPRONE) {
					m_MoveState = CRAWL;
				} else if (m_CanRun && m_Controller.IsState(MOVE_FAST) && !m_Controller.IsState(BODY_CROUCH)) {
					m_MoveState = RUN;
				} else {
					m_MoveState = WALK;
				}

				// Restart the stride if we're just starting to walk or crawl.
				if (m_MoveState != oldMoveState) {
					m_StrideStart = true;
					MoveOutOfTerrain(g_MaterialGrass);
				}
			}

			// Walk backwards if the aiming is already focused in the opposite direction of travel.
			// Note that we check against zero here rather than the deadzone, because using the deadzone makes jetpacking mouse players unable to fly one way and aim the other.
			if (!analogAim.IsZero() || isSharpAiming) {
				m_Paths[FGROUND][m_MoveState].SetHFlip(m_Controller.IsState(MOVE_LEFT));
				m_Paths[BGROUND][m_MoveState].SetHFlip(m_Controller.IsState(MOVE_LEFT));
			} else if ((m_Controller.IsState(MOVE_RIGHT) && m_HFlipped) || (m_Controller.IsState(MOVE_LEFT) && !m_HFlipped)) {
				m_HFlipped = !m_HFlipped;
				m_CheckTerrIntersection = true;
				if (m_ProneState == NOTPRONE) {
					MoveOutOfTerrain(g_MaterialGrass);
				}

				for (int i = WALK; i < MOVEMENTSTATECOUNT; ++i) {
					m_Paths[FGROUND][i].SetHFlip(m_HFlipped);
					m_Paths[BGROUND][i].SetHFlip(m_HFlipped);
					m_Paths[FGROUND][i].Terminate();
					m_Paths[BGROUND][i].Terminate();
				}
				m_StrideStart = true;
				// Stop the going prone spring.
				if (m_ProneState == GOPRONE) {
					m_ProneState = LAYINGPRONE;
				}
			}
		} else {
			m_ArmClimbing[FGROUND] = false;
			m_ArmClimbing[BGROUND] = false;
			if (prone) {
				// Don't go back to prone if we're already prone, the player has to let go of the crouch button first. If already laying down, just stay put.
				m_MoveState = m_ProneState == NOTPRONE ? PRONE : NOMOVE;
			} else if (m_CrouchAmount >= 1.0F) {
				// Fully crouching will set the appropriate state so we can use CrouchLimbPath
				m_MoveState = CROUCH;
			} else {
				m_MoveState = STAND;
			}
		}
		// Disengage the prone state as soon as prone is released.
		if (!prone && m_ProneState != NOTPRONE) {
			EquipShieldInBGArm();
			m_ProneState = NOTPRONE;
		}
	}

	////////////////////////////////////
	// Standard Reloading

	for (const Arm* arm: {m_pFGArm, m_pBGArm}) {
		if (arm) {
			if (HDFirearm* heldFirearm = dynamic_cast<HDFirearm*>(arm->GetHeldDevice())) {
				Arm* otherArm = arm == m_pFGArm ? m_pBGArm : m_pFGArm;
				bool otherArmIsAvailable = otherArm && !otherArm->GetHeldDevice();
				if (otherArmIsAvailable && heldFirearm->DoneReloading()) {
					otherArm->SetHandPos(heldFirearm->GetMagazinePos());
				};
				heldFirearm->SetSupportAvailable(otherArmIsAvailable);
			}
		}
	}
	if (m_Controller.IsState(ControlState::WEAPON_RELOAD)) {
		ReloadFirearms();
	}
	if (m_WaitingToReloadOffhand) {
		if (HeldDevice* equippedItem = GetEquippedItem(); equippedItem && !equippedItem->IsReloading()) {
			ReloadFirearms();
			m_WaitingToReloadOffhand = false;
		}
	}

	////////////////////////////////////
	// Change held MovableObjects

	if (m_pFGArm && m_Status != INACTIVE) {
		bool changeNext = m_Controller.IsState(WEAPON_CHANGE_NEXT);
		bool changePrev = m_Controller.IsState(WEAPON_CHANGE_PREV);
		if (changeNext || changePrev) {
			if (changeNext && changePrev) {
				UnequipArms();
			} else if (!m_Inventory.empty() || UnequipBGArm()) {
				if (HDFirearm* firearm = dynamic_cast<HDFirearm*>(m_pFGArm->GetHeldDevice())) {
					firearm->StopActivationSound();
				}
				if (changeNext) {
					m_pFGArm->SetHeldDevice(dynamic_cast<HeldDevice*>(SwapNextInventory(m_pFGArm->RemoveAttachable(m_pFGArm->GetHeldDevice()))));
				} else {
					m_pFGArm->SetHeldDevice(dynamic_cast<HeldDevice*>(SwapPrevInventory(m_pFGArm->RemoveAttachable(m_pFGArm->GetHeldDevice()))));
				}
				EquipShieldInBGArm();
				m_pFGArm->SetHandPos(m_Pos + RotateOffset(m_HolsterOffset));
			}
			m_EquipHUDTimer.Reset();
			m_SharpAimProgress = 0;
			// Reload empty firearms when we swap to them, for convenience.
			ReloadFirearms(true);
		}
	}

	////////////////////////////////////
	// Aiming

	if (m_Controller.IsState(AIM_UP) && m_Status != INACTIVE) {
		// Set the timer to a base number so we don't get a sluggish feeling at start.
		if (m_AimState != AIMUP) {
			m_AimTmr.SetElapsedSimTimeMS(m_AimState == AIMSTILL ? 150 : 300);
		}
		m_AimState = AIMUP;
		m_AimAngle += isSharpAiming ? std::min(static_cast<float>(m_AimTmr.GetElapsedSimTimeMS()) * 0.00005F, 0.05F) : std::min(static_cast<float>(m_AimTmr.GetElapsedSimTimeMS()) * 0.00015F, 0.15F) * m_Controller.GetDigitalAimSpeed();
		if (m_AimAngle > m_AimRange) {
			m_AimAngle = m_AimRange;
		}

	} else if (m_Controller.IsState(AIM_DOWN) && m_Status != INACTIVE) {
		// Set the timer to a base number so we don't get a sluggish feeling at start.
		if (m_AimState != AIMDOWN) {
			m_AimTmr.SetElapsedSimTimeMS(m_AimState == AIMSTILL ? 150 : 300);
		}
		m_AimState = AIMDOWN;
		m_AimAngle -= isSharpAiming ? std::min(static_cast<float>(m_AimTmr.GetElapsedSimTimeMS()) * 0.00005F, 0.05F) : std::min(static_cast<float>(m_AimTmr.GetElapsedSimTimeMS()) * 0.00015F, 0.15F) * m_Controller.GetDigitalAimSpeed();
		if (m_AimAngle < -m_AimRange) {
			m_AimAngle = -m_AimRange;
		}

	} else if (analogAim.MagnitudeIsGreaterThan(analogDeadzone) && m_Status != INACTIVE) {
		// Hack to avoid the GetAbsRadAngle from mangling an aim angle straight down.
		if (analogAim.m_X == 0) {
			analogAim.m_X += 0.01F * GetFlipFactor();
		}
		m_AimAngle = analogAim.GetAbsRadAngle();

		if ((analogAim.m_X > 0 && m_HFlipped) || (analogAim.m_X < 0 && !m_HFlipped)) {
			m_HFlipped = !m_HFlipped;
			m_CheckTerrIntersection = true;
			if (m_ProneState == NOTPRONE) {
				MoveOutOfTerrain(g_MaterialGrass);
			}
			for (int i = STAND; i < CLIMB; ++i) {
				m_Paths[FGROUND][i].SetHFlip(m_HFlipped);
				m_Paths[BGROUND][i].SetHFlip(m_HFlipped);
				m_Paths[FGROUND][i].Terminate();
				m_Paths[BGROUND][i].Terminate();
			}
			m_StrideStart = true;
			// Stop the going prone spring.
			if (m_ProneState == GOPRONE) {
				m_ProneState = LAYINGPRONE;
			}
		}
		// Correct angle based on flip.
		m_AimAngle = FacingAngle(m_AimAngle);
		// Clamp so it's within the range.
		Clamp(m_AimAngle, m_AimRange, -m_AimRange);
	} else {
		m_AimState = AIMSTILL;
	}
	float adjustedAimAngle = m_AimAngle * GetFlipFactor();

	//////////////////////////////
	// Sharp aim calculation

	// TODO: make the delay data driven by both the actor and the device!
	//
	if (isSharpAiming && m_Status == STABLE && m_Vel.MagnitudeIsLessThan(5.0F) && GetEquippedItem() &&
	    (m_MoveState == STAND || m_MoveState == CROUCH || m_MoveState == PRONE || m_MoveState == NOMOVE || m_MoveState == WALK)) {
		float aimMag = analogAim.GetMagnitude();

		// If aim sharp is being done digitally, then translate to full analog aim mag
		if (aimMag < 0.1F) {
			aimMag = 1.0F;
		}
		
		if (m_MoveState == WALK) {
			aimMag *= 0.3F;
		}

		if (m_SharpAimTimer.IsPastSimMS(m_SharpAimDelay)) {
			// Only go slower outward
			if (m_SharpAimProgress < aimMag) {
				m_SharpAimProgress += (aimMag - m_SharpAimProgress) * 0.035F;
			} else {
				m_SharpAimProgress = aimMag;
			}
			m_SharpAimRevertTimer.Reset();
		} else {
			m_SharpAimProgress *= 0.95F;
			m_SharpAimRevertTimer.SetElapsedSimTimeMS(m_SharpAimDelay - m_SharpAimTimer.GetElapsedSimTimeMS());
		}
	} else {
		m_SharpAimProgress = std::max(m_SharpAimProgress * 0.95F - 0.1F, 0.0F);
		if (m_SharpAimRevertTimer.IsPastSimMS(m_SharpAimDelay)) {
			m_SharpAimTimer.Reset();
		} else {
			m_SharpAimTimer.SetElapsedSimTimeMS(m_SharpAimDelay - m_SharpAimRevertTimer.GetElapsedSimTimeMS());
		}
	}

	////////////////////////////////////
	// Handle firing/activating/throwing HeldDevices and ThrownDevices.
	// Also deal with certain reload cases and setting sharp aim progress for HeldDevices.

	ThrownDevice* thrownDevice = nullptr;
	if (HeldDevice* device = GetEquippedItem(); device && m_Status != INACTIVE) {
		if (!dynamic_cast<ThrownDevice*>(device)) {
			device->SetSharpAim(m_SharpAimProgress);

			if (HDFirearm* deviceAsFirearm = dynamic_cast<HDFirearm*>(device)) {
				if (m_Controller.IsState(WEAPON_FIRE)) {
					if (!m_CanActivateBGItem) {
						if (deviceAsFirearm->IsFullAuto()) {
							deviceAsFirearm->Activate();
							m_CanActivateBGItem = deviceAsFirearm->FiredOnce() && deviceAsFirearm->HalfwayToNextRound();
						} else if (!m_TriggerPulled) {
							deviceAsFirearm->Activate();
							if (deviceAsFirearm->FiredOnce()) {
								m_CanActivateBGItem = true;
								m_TriggerPulled = true;
							} else {
								m_CanActivateBGItem = !deviceAsFirearm->CanFire();
							}
						}
					}
				} else {
					deviceAsFirearm->Deactivate();
					m_TriggerPulled = false;
				}
			} else {
				m_CanActivateBGItem = true;
				if (m_Controller.IsState(WEAPON_FIRE)) {
					device->Activate();
					if (device->IsEmpty()) {
						ReloadFirearms(true);
					}
				} else {
					device->Deactivate();
				}
			}
			// If reloading 2 guns one-at-a-time, the automatic reload when firing empty won't trigger, so this makes sure it happens automatically.
			if (device->IsEmpty()) {
				ReloadFirearms(true);
			}

			if (device->IsReloading()) {
				m_CanActivateBGItem = true;
				m_SharpAimTimer.Reset();
				m_SharpAimProgress = 0;
				device->SetSharpAim(m_SharpAimProgress);
			}
		} else {
			m_CanActivateBGItem = true;
			if (thrownDevice = dynamic_cast<ThrownDevice*>(device)) {
				thrownDevice->SetSharpAim(isSharpAiming ? 1.0F : 0);
				if (m_Controller.IsState(WEAPON_FIRE)) {
					if (m_ArmsState != THROWING_PREP) {
						m_ThrowTmr.Reset();
						if (!thrownDevice->ActivatesWhenReleased()) {
							thrownDevice->Activate();
						}
					}
					float throwProgress = GetThrowProgress();
					m_ArmsState = THROWING_PREP;
					m_pFGArm->SetHandPos(m_pFGArm->GetJointPos() + (thrownDevice->GetStartThrowOffset().GetXFlipped(m_HFlipped) * throwProgress + thrownDevice->GetStanceOffset() * (1.0F - throwProgress)).RadRotate(adjustedAimAngle));
				} else if (m_ArmsState == THROWING_PREP) {
					m_ArmsState = THROWING_RELEASE;
					m_pFGArm->SetHandPos(m_pFGArm->GetJointPos() + thrownDevice->GetEndThrowOffset().RadRotate(adjustedAimAngle).GetXFlipped(m_HFlipped));

					float maxThrowVel = thrownDevice->GetCalculatedMaxThrowVelIncludingArmThrowStrength();
					if (MovableObject* pMO = m_pFGArm->RemoveAttachable(thrownDevice)) {
						pMO->SetPos(m_pFGArm->GetJointPos() + Vector(m_pFGArm->GetMaxLength() * GetFlipFactor(), -m_pFGArm->GetMaxLength() * 0.5F).RadRotate(adjustedAimAngle));
						float minThrowVel = thrownDevice->GetMinThrowVel();
						if (minThrowVel == 0) {
							minThrowVel = maxThrowVel * 0.2F;
						}

						Vector tossVec(minThrowVel + (maxThrowVel - minThrowVel) * GetThrowProgress(), 0.5F * RandomNormalNum());
						pMO->SetVel(m_Vel * 0.5F + tossVec.RadRotate(m_AimAngle).GetXFlipped(m_HFlipped));
						pMO->SetAngularVel(m_AngularVel + RandomNum(-5.0F, 2.5F) * GetFlipFactor());
						pMO->SetRotAngle(adjustedAimAngle);

						if (HeldDevice* moAsHeldDevice = dynamic_cast<HeldDevice*>(pMO)) {
							moAsHeldDevice->SetTeam(m_Team);
							moAsHeldDevice->SetIgnoresTeamHits(true);
							g_MovableMan.AddItem(moAsHeldDevice);
						}
						pMO = 0;
					}
					if (thrownDevice->ActivatesWhenReleased()) {
						thrownDevice->Activate();
					}
					m_ThrowTmr.Reset();
				}
			} else if (m_ArmsState == THROWING_RELEASE && m_ThrowTmr.GetElapsedSimTimeMS() > 100) {
				m_pFGArm->SetHeldDevice(dynamic_cast<HeldDevice*>(SwapNextInventory()));
				m_pFGArm->SetHandPos(m_Pos + RotateOffset(m_HolsterOffset));
				EquipShieldInBGArm();
				m_ArmsState = WEAPON_READY;
			} else if (m_ArmsState == THROWING_RELEASE) {
				m_pFGArm->AddHandTarget("Adjusted Aim Angle", m_Pos + Vector(m_pFGArm->GetMaxLength() * GetFlipFactor(), -m_pFGArm->GetMaxLength() * 0.5F).RadRotate(adjustedAimAngle));
			}
		}
	} else if (m_ArmsState == THROWING_RELEASE && m_ThrowTmr.GetElapsedSimTimeMS() > 100) {
		if (m_pFGArm) {
			m_pFGArm->SetHeldDevice(dynamic_cast<HeldDevice*>(SwapNextInventory()));
			m_pFGArm->SetHandPos(m_Pos + RotateOffset(m_HolsterOffset));
		}
		EquipShieldInBGArm();
		m_ArmsState = WEAPON_READY;
	} else if (m_ArmsState == THROWING_RELEASE && m_pFGArm) {
		m_pFGArm->AddHandTarget("Adjusted Aim Angle", m_Pos + Vector(m_pFGArm->GetMaxLength() * GetFlipFactor(), -m_pFGArm->GetMaxLength() * 0.5F).RadRotate(adjustedAimAngle));
	} else {
		m_CanActivateBGItem = true;
	}

	if (HeldDevice* device = GetEquippedBGItem(); device && m_Status != INACTIVE) {
		if (HDFirearm* deviceAsFirearm = dynamic_cast<HDFirearm*>(device)) {
			if (m_Controller.IsState(WEAPON_FIRE)) {
				if (m_CanActivateBGItem && (!m_TriggerPulled || (deviceAsFirearm->IsFullAuto() && deviceAsFirearm->HalfwayToNextRound()))) {
					deviceAsFirearm->Activate();
					if (deviceAsFirearm->FiredOnce()) {
						m_CanActivateBGItem = false;
						m_TriggerPulled = true;
					} else {
						m_CanActivateBGItem = deviceAsFirearm->CanFire();
					}
				}
			} else {
				deviceAsFirearm->Deactivate();
				m_TriggerPulled = false;
			}
		} else {
			m_CanActivateBGItem = false;
			if (m_Controller.IsState(WEAPON_FIRE)) {
				device->Activate();
			} else {
				device->Deactivate();
			}
		}
		// If reloading 2 guns one-at-a-time, the automatic reload when firing empty won't trigger, so this makes sure it happens automatically.
		if (device->IsEmpty()) {
			ReloadFirearms(true);
		}
		device->SetSharpAim(m_SharpAimProgress);

		if (device->IsReloading()) {
			m_CanActivateBGItem = false;
			m_SharpAimTimer.Reset();
			m_SharpAimProgress = 0;
			device->SetSharpAim(m_SharpAimProgress);
		}
	} else {
		m_CanActivateBGItem = false;
	}

	if (m_ArmsState == THROWING_PREP && !thrownDevice) {
		m_ArmsState = WEAPON_READY;
	}

	//    m_aSprite->SetAngle((m_AimAngle / 180) * 3.141592654);
	//    m_aSprite->SetScale(2.0);

	////////////////////////////////////////
	// Item dropping logic

	if (m_Controller.IsState(WEAPON_DROP) && m_Status != INACTIVE) {
		Arm* dropperArm = nullptr;
		for (Arm* arm: {m_pFGArm, m_pBGArm}) {
			if (arm && arm->GetHeldDevice()) {
				HeldDevice* heldDevice = arm->GetHeldDevice();
				arm->RemoveAttachable(heldDevice, true, false);
				if (dropperArm) {
					if (heldDevice) {
						dropperArm->SetHeldDevice(heldDevice);
						arm->SetHandPos(dropperArm->GetPos());
					}
				} else {
					heldDevice->SetPos(arm->GetJointPos() + Vector(arm->GetMaxLength() * GetFlipFactor(), 0).RadRotate(adjustedAimAngle));
					Vector tossVec(1.0F + std::sqrt(std::abs(arm->GetThrowStrength()) / std::sqrt(std::abs(heldDevice->GetMass()) + 1.0F)), RandomNormalNum());
					heldDevice->SetVel(heldDevice->GetVel() * 0.5F + tossVec.RadRotate(m_AimAngle).GetXFlipped(m_HFlipped));
					heldDevice->SetAngularVel(heldDevice->GetAngularVel() + m_AngularVel * 0.5F + 3.0F * RandomNormalNum());

					arm->SetHandPos(heldDevice->GetPos());
				}
				dropperArm = arm;
			} else if (dropperArm && !m_Inventory.empty()) {
				dropperArm->SetHeldDevice(dynamic_cast<HeldDevice*>(SwapNextInventory()));
				dropperArm->SetHandPos(m_Pos + RotateOffset(m_HolsterOffset));
			}
		}
		if (!dropperArm && !m_Inventory.empty() && !m_pFGArm) {
			DropAllInventory();
			if (m_pBGArm) {
				m_pBGArm->SetHandPos(m_Pos + RotateOffset(m_HolsterOffset));
			}
		}
		EquipShieldInBGArm();
		m_SharpAimProgress = 0;
		m_EquipHUDTimer.Reset();
	}

	////////////////////////////////////////
	// Item pickup logic

	float reach = m_SpriteRadius;
	Vector reachPoint = m_Pos;

	// Try to detect a new item
	if ((m_pFGArm || m_pBGArm) && m_Status == STABLE) {
		reach += m_pFGArm ? m_pFGArm->GetMaxLength() : m_pBGArm->GetMaxLength();
		reachPoint = m_pFGArm ? m_pFGArm->GetJointPos() : m_pBGArm->GetJointPos();

		MOID itemMOID = g_SceneMan.CastMORay(reachPoint, Vector(reach * RandomNum(0.5F, 1.0F) * GetFlipFactor(), 0).RadRotate(m_pItemInReach ? adjustedAimAngle : RandomNum(-(c_HalfPI + c_EighthPI), m_AimAngle * 0.75F + c_EighthPI) * GetFlipFactor()), m_MOID, m_Team, g_MaterialGrass, true, 3);

		if (MovableObject* foundMO = g_MovableMan.GetMOFromID(itemMOID)) {
			if (HeldDevice* foundDevice = dynamic_cast<HeldDevice*>(foundMO->GetRootParent())) {
				m_pItemInReach = (m_pFGArm || foundDevice->IsOneHanded()) ? foundDevice : nullptr;
			}
		}
	}

	// Item currently set to be within reach has expired or is now out of range
	if (m_pItemInReach && (!m_pItemInReach->IsPickupableBy(this) || !g_MovableMan.IsDevice(m_pItemInReach) || g_SceneMan.ShortestDistance(reachPoint, m_pItemInReach->GetPos(), g_SceneMan.SceneWrapsX()).MagnitudeIsGreaterThan(reach + m_pItemInReach->GetRadius()))) {
		m_pItemInReach = nullptr;
	}

	if (m_pItemInReach && (m_pFGArm || m_pBGArm) && m_Controller.IsState(WEAPON_PICKUP) && m_Status != INACTIVE && g_MovableMan.RemoveMO(m_pItemInReach)) {
		Arm* armToUse = m_pFGArm ? m_pFGArm : m_pBGArm;
		Attachable* pMO = armToUse->RemoveAttachable(armToUse->GetHeldDevice());
		AddToInventoryBack(pMO);
		armToUse->SetHandPos(m_pItemInReach->GetJointPos());
		armToUse->SetHeldDevice(m_pItemInReach);
		m_pItemInReach = nullptr;

		if (armToUse != m_pBGArm) {
			EquipShieldInBGArm();
		}
		m_SharpAimProgress = 0;
		if (m_DeviceSwitchSound) {
			m_DeviceSwitchSound->Play(m_Pos);
		}

		m_EquipHUDTimer.Reset();
	}

	///////////////////////////////////////////////////
	// Travel the limb AtomGroup:s

	m_StrideFrame = false;

	UpdateCrouching();

	Vector pathOffset;
	if (m_MoveState == WALK) {
		pathOffset = m_WalkPathOffset;
	}

	if (m_Status == STABLE && !m_LimbPushForcesAndCollisionsDisabled && m_MoveState != NOMOVE) {
		// This exists to support disabling foot collisions if the limbpath has that flag set.
		if ((m_pFGFootGroup->GetAtomCount() == 0 && m_BackupFGFootGroup->GetAtomCount() > 0) != m_Paths[FGROUND][m_MoveState].FootCollisionsShouldBeDisabled()) {
			m_BackupFGFootGroup->SetLimbPos(m_pFGFootGroup->GetLimbPos());
			std::swap(m_pFGFootGroup, m_BackupFGFootGroup);
		}
		if ((m_pBGFootGroup->GetAtomCount() == 0 && m_BackupBGFootGroup->GetAtomCount() > 0) != m_Paths[BGROUND][m_MoveState].FootCollisionsShouldBeDisabled()) {
			m_BackupBGFootGroup->SetLimbPos(m_pBGFootGroup->GetLimbPos());
			std::swap(m_pBGFootGroup, m_BackupBGFootGroup);
		}

		if (m_pFGLeg) {
			UpdateWalkAngle(FGROUND);
		}
		if (m_pBGLeg) {
			UpdateWalkAngle(BGROUND);
		}

		UpdateLimbPathSpeed();

		// WALKING, OR WE ARE JETPACKING AND STUCK
		if (m_MoveState == WALK || m_MoveState == RUN || (m_MoveState == JUMP && isStill)) {
			m_Paths[FGROUND][STAND].Terminate();
			m_Paths[BGROUND][STAND].Terminate();

			MovementState movementPath = m_MoveState == RUN ? RUN : WALK;
			float FGLegProg = m_Paths[FGROUND][movementPath].GetRegularProgress();
			float BGLegProg = m_Paths[BGROUND][movementPath].GetRegularProgress();

			bool restarted = false;

			// Make sure we are starting a stride if we're basically stopped.
			if (isStill) {
				m_StrideStart = true;
			}

			if (m_pFGLeg && (!m_pBGLeg || !(m_Paths[FGROUND][movementPath].PathEnded() && BGLegProg < 0.5F) || m_StrideStart)) {
				// Reset the stride timer if the path is about to restart.
				if (m_Paths[FGROUND][movementPath].PathEnded() || m_Paths[FGROUND][movementPath].PathIsAtStart()) {
					m_StrideTimer.Reset();
				}
				Vector jointPos = m_Pos + RotateOffset(m_pFGLeg->GetParentOffset());
				m_ArmClimbing[BGROUND] = !m_pFGFootGroup->PushAsLimb(jointPos, m_Vel, m_WalkAngle[FGROUND], m_Paths[FGROUND][movementPath], deltaTime, &restarted, false, Vector(0.0F, m_Paths[FGROUND][movementPath].GetLowestY()), pathOffset);
			} else {
				m_ArmClimbing[BGROUND] = false;
			}
			if (m_pBGLeg && (!m_pFGLeg || !(m_Paths[BGROUND][movementPath].PathEnded() && FGLegProg < 0.5F))) {
				m_StrideStart = false;
				// Reset the stride timer if the path is about to restart.
				if (m_Paths[BGROUND][movementPath].PathEnded() || m_Paths[BGROUND][movementPath].PathIsAtStart()) {
					m_StrideTimer.Reset();
				}
				Vector jointPos = m_Pos + RotateOffset(m_pBGLeg->GetParentOffset());
				m_ArmClimbing[FGROUND] = !m_pBGFootGroup->PushAsLimb(jointPos, m_Vel, m_WalkAngle[BGROUND], m_Paths[BGROUND][movementPath], deltaTime, &restarted, false, Vector(0.0F, m_Paths[BGROUND][movementPath].GetLowestY()), pathOffset);
			} else {
				if (m_pBGLeg) {
					m_pBGFootGroup->FlailAsLimb(m_Pos, RotateOffset(m_pBGLeg->GetParentOffset()), m_pBGLeg->GetMaxLength(), m_PrevVel, m_AngularVel, m_pBGLeg->GetMass(), deltaTime);
				}
				m_ArmClimbing[FGROUND] = false;
			}
			bool climbing = m_ArmClimbing[FGROUND] || m_ArmClimbing[BGROUND];

			if (m_StrideSound) {
				m_StrideSound->SetPosition(m_Pos);
				if (m_StrideSound->GetLoopSetting() < 0) {
					if (!m_StrideSound->IsBeingPlayed()) {
						m_StrideSound->Play();
					}
				} else if (restarted && !climbing) {
					m_StrideSound->Play();
				}
			}
			if (restarted) {
				if (climbing) {
					m_WalkAngle[FGROUND] = Matrix();
					m_WalkAngle[BGROUND] = Matrix();
				} else {
					m_StrideFrame = true;
					RunScriptedFunctionInAppropriateScripts("OnStride");
				}
			}

			////////////////////////////////////////
			// Arm Climbing if the leg paths failed to find clear spot to restart

			//            float FGArmProg = MAX(m_Paths[FGROUND][CLIMB].GetRegularProgress(), m_Paths[FGROUND][CLIMB].GetTotalTimeProgress());
			//            float BGArmProg = MAX(m_Paths[BGROUND][CLIMB].GetRegularProgress(), m_Paths[BGROUND][CLIMB].GetTotalTimeProgress());
			float FGArmProg = m_Paths[FGROUND][CLIMB].GetRegularProgress();
			float BGArmProg = m_Paths[BGROUND][CLIMB].GetRegularProgress();

			// TODO: Figure out what this comment means, and then rephrase it better!
			// Slightly negative BGArmProg makes sense because any progress on the starting segments are reported as negative,
			// and there's many starting segments on properly formed climbing paths
			if (climbing) {
				if (m_pFGArm && !m_pFGArm->GetHeldDevice() && !(m_Paths[FGROUND][CLIMB].PathEnded() && BGArmProg > 0.1F)) { // < 0.5F
					m_ArmClimbing[FGROUND] = true;
					m_Paths[FGROUND][movementPath].Terminate();
					m_StrideStart = true;
					// Reset the stride timer if the path is about to restart.
					if (m_Paths[FGROUND][CLIMB].PathEnded() || m_Paths[FGROUND][CLIMB].PathIsAtStart()) {
						m_StrideTimer.Reset();
					}
					m_pFGHandGroup->PushAsLimb(m_Pos + Vector(0, m_pFGArm->GetParentOffset().m_Y).RadRotate(-rot), m_Vel, Matrix(), m_Paths[FGROUND][CLIMB], deltaTime, 0, false);
				} else {
					m_ArmClimbing[FGROUND] = false;
					m_Paths[FGROUND][CLIMB].Terminate();
				}
				if (m_pBGArm) {
					m_ArmClimbing[BGROUND] = true;
					m_Paths[BGROUND][movementPath].Terminate();
					m_StrideStart = true;
					// Reset the stride timer if the path is about to restart.
					if (m_Paths[BGROUND][CLIMB].PathEnded() || m_Paths[BGROUND][CLIMB].PathIsAtStart()) {
						m_StrideTimer.Reset();
					}
					m_pBGHandGroup->PushAsLimb(m_Pos + Vector(0, m_pBGArm->GetParentOffset().m_Y).RadRotate(-rot), m_Vel, Matrix(), m_Paths[BGROUND][CLIMB], deltaTime, 0, false);
				} else {
					m_ArmClimbing[BGROUND] = false;
					m_Paths[BGROUND][CLIMB].Terminate();
				}
			}

			// Restart the climbing stroke if the current one seems to be taking too long with no movement.
			if (climbing && isStill && m_StrideTimer.IsPastSimMS(static_cast<double>(m_Paths[BGROUND][CLIMB].GetTotalPathTime() * 0.5F))) {
				m_StrideStart = true;
				m_Paths[FGROUND][CLIMB].Terminate();
				m_Paths[BGROUND][CLIMB].Terminate();
			} else if (m_StrideTimer.IsPastSimMS(static_cast<double>(m_Paths[FGROUND][movementPath].GetTotalPathTime() * 1.1F))) {
				// Reset the walking stride if it's taking longer than it should.
				m_StrideStart = true;
				m_Paths[FGROUND][movementPath].Terminate();
				m_Paths[BGROUND][movementPath].Terminate();
			}
		} else if (m_MoveState == CRAWL) {
			// Start crawling only once we are fully prone.
			if (m_ProneState == LAYINGPRONE) {

				float FGLegProg = m_Paths[FGROUND][CRAWL].GetRegularProgress();
				float BGLegProg = m_Paths[BGROUND][CRAWL].GetRegularProgress();

				if (m_pFGLeg && (!m_pBGLeg || (!(m_Paths[FGROUND][CRAWL].PathEnded() && BGLegProg < 0.5F) || m_StrideStart))) {
					if (m_Paths[FGROUND][CRAWL].PathEnded() || m_Paths[FGROUND][CRAWL].PathIsAtStart()) {
						m_StrideTimer.Reset();
					}
					m_pFGFootGroup->PushAsLimb(m_Pos + RotateOffset(m_pFGLeg->GetParentOffset()), m_Vel, m_Rotation, m_Paths[FGROUND][CRAWL], deltaTime);
				} else {
					m_Paths[FGROUND][CRAWL].Terminate();
				}
				if (m_pBGLeg && (!m_pFGLeg || !(m_Paths[BGROUND][CRAWL].PathEnded() && FGLegProg < 0.5F))) {
					m_StrideStart = false;
					if (m_Paths[BGROUND][CRAWL].PathEnded() || m_Paths[BGROUND][CRAWL].PathIsAtStart()) {
						m_StrideTimer.Reset();
					}
					m_pBGFootGroup->PushAsLimb(m_Pos + RotateOffset(m_pBGLeg->GetParentOffset()), m_Vel, m_Rotation, m_Paths[BGROUND][CRAWL], deltaTime);
				} else {
					m_Paths[BGROUND][CRAWL].Terminate();
				}
				if (m_pBGArm) {
					m_ArmClimbing[BGROUND] = true;
					m_pBGHandGroup->PushAsLimb(m_Pos + RotateOffset(Vector(0, m_pBGArm->GetParentOffset().m_Y)), m_Vel, m_Rotation, m_Paths[BGROUND][ARMCRAWL], deltaTime);
				}
				if (m_pFGArm && !m_pFGArm->GetHeldDevice() && !(m_Paths[FGROUND][ARMCRAWL].PathEnded() && m_Paths[BGROUND][ARMCRAWL].GetRegularProgress() < 0.5F)) {
					m_ArmClimbing[FGROUND] = true;
					m_pFGHandGroup->PushAsLimb(m_Pos + RotateOffset(Vector(0, m_pFGArm->GetParentOffset().m_Y)), m_Vel, m_Rotation, m_Paths[FGROUND][ARMCRAWL], deltaTime);
				}
				// Restart the stride if the current one seems to be taking too long.
				if (m_StrideTimer.IsPastSimMS(m_Paths[FGROUND][CRAWL].GetTotalPathTime())) {
					m_StrideStart = true;
					m_Paths[FGROUND][CRAWL].Terminate();
					m_Paths[BGROUND][CRAWL].Terminate();
				}
			} else {
				if (m_pFGLeg) {
					m_pFGFootGroup->FlailAsLimb(m_Pos, RotateOffset(m_pFGLeg->GetParentOffset()), m_pFGLeg->GetMaxLength(), m_PrevVel, m_AngularVel, m_pFGLeg->GetMass(), deltaTime);
				}

				if (m_pBGLeg) {
					m_pBGFootGroup->FlailAsLimb(m_Pos, RotateOffset(m_pBGLeg->GetParentOffset()), m_pBGLeg->GetMaxLength(), m_PrevVel, m_AngularVel, m_pBGLeg->GetMass(), deltaTime);
				}
			}
		} else if (m_pFGLeg || m_pBGLeg) {
			if (m_MoveState == JUMP) {
				// TODO: Utilize jump paths in an intuitive way!
				if (m_pFGLeg) {
					m_pFGFootGroup->FlailAsLimb(m_Pos, RotateOffset(m_pFGLeg->GetParentOffset()), m_pFGLeg->GetMaxLength(), m_PrevVel, m_AngularVel, m_pFGLeg->GetMass(), deltaTime);
				}

				if (m_pBGLeg) {
					m_pBGFootGroup->FlailAsLimb(m_Pos, RotateOffset(m_pBGLeg->GetParentOffset()), m_pBGLeg->GetMaxLength(), m_PrevVel, m_AngularVel, m_pBGLeg->GetMass(), deltaTime);
				}
			} else {
				m_Paths[FGROUND][JUMP].Terminate();
				m_Paths[BGROUND][JUMP].Terminate();
				if (m_MoveState == CROUCH) {
					m_Paths[FGROUND][WALK].Terminate();
					m_Paths[BGROUND][WALK].Terminate();
					m_Paths[FGROUND][CRAWL].Terminate();
					m_Paths[BGROUND][CRAWL].Terminate();

					if (m_pFGLeg) {
						m_pFGFootGroup->PushAsLimb(m_Pos.GetFloored() + m_pFGLeg->GetParentOffset().GetXFlipped(m_HFlipped), m_Vel, Matrix(), m_Paths[FGROUND][PRONE], deltaTime);
					}

					if (m_pBGLeg) {
						m_pBGFootGroup->PushAsLimb(m_Pos.GetFloored() + m_pBGLeg->GetParentOffset().GetXFlipped(m_HFlipped), m_Vel, Matrix(), m_Paths[BGROUND][PRONE], deltaTime);
					}

				} else {
					m_Paths[FGROUND][WALK].Terminate();
					m_Paths[BGROUND][WALK].Terminate();
					m_Paths[FGROUND][CRAWL].Terminate();
					m_Paths[BGROUND][CRAWL].Terminate();
					m_Paths[FGROUND][ARMCRAWL].Terminate();
					m_Paths[BGROUND][ARMCRAWL].Terminate();

					if (m_pFGLeg) {
						Vector jointPos = m_Pos.GetFloored() + m_pFGLeg->GetParentOffset().GetXFlipped(m_HFlipped);
						m_pFGFootGroup->PushAsLimb(jointPos, m_Vel, m_WalkAngle[FGROUND], m_Paths[FGROUND][STAND], deltaTime, nullptr, !m_pBGLeg, Vector(0.0F, m_Paths[FGROUND][STAND].GetLowestY()), pathOffset);
					}

					if (m_pBGLeg) {
						Vector jointPos = m_Pos.GetFloored() + m_pBGLeg->GetParentOffset().GetXFlipped(m_HFlipped);
						m_pBGFootGroup->PushAsLimb(jointPos, m_Vel, m_WalkAngle[BGROUND], m_Paths[BGROUND][STAND], deltaTime, nullptr, !m_pFGLeg, Vector(0.0F, m_Paths[FGROUND][STAND].GetLowestY()), pathOffset);
					}
				}
			}
		}
	} else {
		// Not stable/standing, so make sure the end of limbs are moving around limply in a ragdoll fashion.
		// TODO: Make the limb atom groups fly around and react to terrain, without getting stuck etc.
		if (m_pFGArm) {
			m_pFGHandGroup->FlailAsLimb(m_Pos, RotateOffset(m_pFGArm->GetParentOffset()), m_pFGArm->GetMaxLength(), m_PrevVel * m_pFGArm->GetJointStiffness(), m_AngularVel, m_pFGArm->GetMass(), deltaTime);
		}

		if (m_pBGArm) {
			m_pBGHandGroup->FlailAsLimb(m_Pos, RotateOffset(m_pBGArm->GetParentOffset()), m_pBGArm->GetMaxLength(), m_PrevVel * m_pBGArm->GetJointStiffness(), m_AngularVel, m_pBGArm->GetMass(), deltaTime);
		}

		if (m_pFGLeg) {
			m_pFGFootGroup->FlailAsLimb(m_Pos, RotateOffset(m_pFGLeg->GetParentOffset()), m_pFGLeg->GetMaxLength(), m_PrevVel * m_pFGLeg->GetJointStiffness(), m_AngularVel, m_pFGLeg->GetMass(), deltaTime);
		}

		if (m_pBGLeg) {
			m_pBGFootGroup->FlailAsLimb(m_Pos, RotateOffset(m_pBGLeg->GetParentOffset()), m_pBGLeg->GetMaxLength(), m_PrevVel * m_pBGLeg->GetJointStiffness(), m_AngularVel, m_pBGLeg->GetMass(), deltaTime);
		}
	}
	if (m_MoveState != WALK && m_StrideSound && m_StrideSound->GetLoopSetting() < 0) {
		m_StrideSound->Stop();
	}

	/////////////////////////////////
	// Manage Attachables

	if (m_pHead) {
		float toRotate = 0;
		// Only rotate the head to match the aim angle if body is stable and upright
		if (m_Status == STABLE && std::abs(rot) < (c_HalfPI + c_QuarterPI)) {
			toRotate = m_pHead->GetRotMatrix().GetRadAngleTo((adjustedAimAngle)*m_LookToAimRatio + rot * (0.9F - m_LookToAimRatio)) * 0.15F;
		} else {
			// Rotate the head loosely along with the body if upside down, unstable or dying.
			toRotate = m_pHead->GetRotMatrix().GetRadAngleTo(rot) * m_pHead->GetJointStiffness() * c_QuarterPI;
		}
		m_pHead->SetRotAngle(m_pHead->GetRotAngle() + toRotate);
	}

	if (m_pFGLeg) {
		m_pFGLeg->EnableIdle(m_ProneState == NOTPRONE && m_Status != UNSTABLE);
		m_pFGLeg->SetTargetPosition(m_pFGFootGroup->GetLimbPos(m_HFlipped));
	}

	if (m_pBGLeg) {
		m_pBGLeg->EnableIdle(m_ProneState == NOTPRONE && m_Status != UNSTABLE);
		m_pBGLeg->SetTargetPosition(m_pBGFootGroup->GetLimbPos(m_HFlipped));
	}

	// FG Arm rotating and climbing
	if (m_pFGArm) {
		float affectingBodyAngle = m_Status < INACTIVE ? m_FGArmFlailScalar : 1.0F;
		if (affectingBodyAngle != 0 && m_SharpAimDelay != 0) {
			float aimScalar = std::min(static_cast<float>(m_SharpAimTimer.GetElapsedSimTimeMS()) / static_cast<float>(m_SharpAimDelay), 1.0F);
			float revertScalar = std::min(static_cast<float>(m_SharpAimRevertTimer.GetElapsedSimTimeMS()) / static_cast<float>(m_SharpAimDelay), 1.0F);
			aimScalar = (aimScalar > revertScalar) ? aimScalar : 1.0F - revertScalar;

			affectingBodyAngle *= std::abs(std::sin(rot)) * rot * (1.0F - aimScalar);
		}
		m_pFGArm->SetRotAngle(affectingBodyAngle + adjustedAimAngle);

		if (m_Status == STABLE) {
			if (m_ArmClimbing[FGROUND]) {
				m_pFGArm->AddHandTarget("Hand AtomGroup Limb Pos", m_pFGHandGroup->GetLimbPos(m_HFlipped));
			}
		} else if (!m_pFGArm->GetHeldDevice()) {
			m_pFGArm->ClearHandTargets();
			m_pFGArm->AddHandTarget("Arm Flail", m_pFGHandGroup->GetLimbPos(m_HFlipped));
		}
	}

	// BG Arm rotating, climbing, throw animations, supporting fg weapon
	if (m_pBGArm) {
		float affectingBodyAngle = m_Status < INACTIVE ? m_BGArmFlailScalar : 1.0F;
		m_pBGArm->SetRotAngle(std::abs(std::sin(rot)) * rot * affectingBodyAngle + adjustedAimAngle);

		if (m_Status == STABLE) {
			if (m_ArmClimbing[BGROUND]) {
				// Can't climb or crawl with the shield
				if (m_MoveState != CRAWL || m_ProneState == LAYINGPRONE) {
					UnequipBGArm();
				}
				m_pBGArm->AddHandTarget("Hand AtomGroup Limb Pos", m_pBGHandGroup->GetLimbPos(m_HFlipped));
			} else {
				HeldDevice* heldDevice = GetEquippedItem();
				ThrownDevice* thrownDevice = dynamic_cast<ThrownDevice*>(heldDevice);
				if (thrownDevice && (m_ArmsState == THROWING_PREP || isSharpAiming)) {
					m_pBGArm->AddHandTarget("End Throw Offset", m_pBGArm->GetJointPos() + thrownDevice->GetEndThrowOffset().GetXFlipped(m_HFlipped).RadRotate(adjustedAimAngle));
				} else if (heldDevice) {
					if (HeldDevice* bgDevice = GetEquippedBGItem(); bgDevice && !heldDevice->IsOneHanded()) {
						UnequipBGArm();
					} else if (!bgDevice && (!heldDevice->IsReloading() || heldDevice->GetUseSupportOffsetWhileReloading()) && heldDevice->IsSupportable()) {
						m_pBGArm->SetHeldDeviceThisArmIsTryingToSupport(heldDevice);

						if (!m_pBGArm->HasAnyHandTargets() && m_pBGArm->GetHandHasReachedCurrentTarget()) {
							heldDevice->SetSupported(true);
							m_pBGArm->SetRecoil(heldDevice->GetRecoilForce(), heldDevice->GetRecoilOffset(), heldDevice->IsRecoiled());
						} else {
							// BGArm did not reach to support the device. Count device as supported anyway, if crouching or prone.
							heldDevice->SetSupported(m_MoveState == PRONE || m_ProneState == LAYINGPRONE);
							m_pBGArm->SetRecoil(Vector(), Vector(), false);
						}
					}
				}
			}
		} else {
			m_pBGArm->ClearHandTargets();
			m_pBGArm->AddHandTarget("Arm Flail", m_pBGHandGroup->GetLimbPos(m_HFlipped));
		}
	} else if (HeldDevice* heldDevice = GetEquippedItem()) {
		heldDevice->SetSupported(false);
	}
	// Make sure the bg arm doesn't think it's supporting something when it isn't.
	if (m_pBGArm && (!m_pFGArm || !m_pFGArm->GetHeldDevice() || m_pBGArm->GetHeldDevice())) {
		m_pBGArm->SetHeldDeviceThisArmIsTryingToSupport(nullptr);
	}

	/////////////////////////////////
	// Arm swinging or device swaying walking animations

	if (m_MoveState != MovementState::STAND && (m_ArmSwingRate != 0 || m_DeviceArmSwayRate != 0)) {
		for (Arm* arm: {m_pFGArm, m_pBGArm}) {
			if (arm && !arm->GetHeldDeviceThisArmIsTryingToSupport()) {
				Leg* legToSwingWith = arm == m_pFGArm ? m_pBGLeg : m_pFGLeg;
				Leg* otherLeg = legToSwingWith == m_pBGLeg ? m_pFGLeg : m_pBGLeg;
				if (!legToSwingWith || m_MoveState == JUMP || m_MoveState == PRONE) {
					std::swap(legToSwingWith, otherLeg);
				}

				if (legToSwingWith) {
					float armMovementRateToUse = m_ArmSwingRate;
					if (HeldDevice* heldDevice = arm->GetHeldDevice()) {
						armMovementRateToUse = m_DeviceArmSwayRate * (1.0F - m_SharpAimProgress) * std::sin(std::abs(heldDevice->GetStanceOffset().GetAbsRadAngle()));
					}
					float angleToSwingTo = std::sin(legToSwingWith->GetRotAngle() + (c_HalfPI * GetFlipFactor()));
					arm->SetHandIdleRotation(angleToSwingTo * armMovementRateToUse);
				}
			}
		}
	}
}

void AHuman::Update() {
	ZoneScoped;

	float rot = m_Rotation.GetRadAngle(); // eugh, for backwards compat to be the same behaviour as with multithreaded AI

	Actor::Update();

	////////////////////////////////////
	// Update viewpoint

	// Set viewpoint based on how we are aiming etc.
	Vector aimSight(m_AimDistance, 0);
	Matrix aimMatrix(m_HFlipped ? -m_AimAngle : m_AimAngle);
	aimMatrix.SetXFlipped(m_HFlipped);
	// Reset this each frame
	m_SharpAimMaxedOut = false;

	if (HeldDevice* heldDevice = GetEquippedItem()) {
		float maxLength = heldDevice->GetSharpLength();
		if (maxLength == 0.0F || m_MoveState == RUN) {
			m_SharpAimProgress = 0;
			m_SharpAimMaxedOut = true;
		} else if (m_MoveState == WALK) {
			maxLength *= 0.7F;
		} else if (m_MoveState == CROUCH || m_MoveState == PRONE) {
			// Only when crouching still, otherwise it's WALK
			maxLength *= 1.2F;
		}

		// Use a non-terrain check ray to cap the magnitude, so we can't see into objects etc
		if (m_SharpAimProgress > 0) {
			// TODO: make an uniform function to get the total GripStrength of an AHuman?
			float totalGripStrength = m_pFGArm->GetGripStrength();
			if (m_pBGArm) {
				if (m_pBGArm->GetHeldDevice()) {
					HeldDevice* heldBGDevice = m_pBGArm->GetHeldDevice();
					if (heldBGDevice->IsRecoiled()) {
						m_SharpAimProgress *= 1.0F - std::min(heldBGDevice->GetRecoilForce().GetMagnitude() / std::max(m_pBGArm->GetGripStrength() * heldBGDevice->GetGripStrengthMultiplier(), 1.0F), 1.0F);
					}
				} else if (heldDevice->GetSupported()) {
					totalGripStrength += m_pBGArm->GetGripStrength();
				}
			}
			if (heldDevice->IsRecoiled()) {
				m_SharpAimProgress *= 1.0F - std::min(heldDevice->GetRecoilForce().GetMagnitude() / std::max(totalGripStrength * heldDevice->GetGripStrengthMultiplier(), 1.0F), 1.0F);
			}
			Vector notUsed;
			Vector sharpAimVector(maxLength, 0);
			sharpAimVector *= aimMatrix;

			// See how far along the sharp aim vector there is opaque air
			float result = g_SceneMan.CastObstacleRay(heldDevice->GetMuzzlePos(), sharpAimVector, notUsed, notUsed, GetRootID(), IgnoresWhichTeam(), g_MaterialAir, 5);
			// If we didn't find anything but air before the sharpdistance, then don't alter the sharp distance
			if (result >= 0 && result < (maxLength * m_SharpAimProgress)) {
				m_SharpAimProgress = result / maxLength;
				m_SharpAimMaxedOut = true;
			}
		}
		if (m_SharpAimProgress > 0.9F) {
			m_SharpAimMaxedOut = true;
		}
		aimSight.m_X += maxLength * m_SharpAimProgress;
	}

	// Rotate the aiming spot vector and add it to the view point
	aimSight *= aimMatrix;
	m_ViewPoint = m_Pos.GetFloored() + aimSight;

	// Add velocity also so the viewpoint moves ahead at high speeds
	if (m_Vel.MagnitudeIsGreaterThan(10.0F)) {
		m_ViewPoint += m_Vel * std::sqrt(m_Vel.GetMagnitude() * 0.1F);
	}

	////////////////////////////////////////
	// Balance stuff

	// Eliminate full rotations
	while (std::abs(rot) > c_TwoPI) {
		rot -= rot > 0 ? c_TwoPI : -c_TwoPI;
	}
	// Eliminate rotations over half a turn
	if (std::abs(rot) > c_PI) {
		rot = (rot > 0 ? -c_PI : c_PI) + (rot - (rot > 0 ? c_PI : -c_PI));
		// If we're upside down, we're unstable damnit
		if (m_Status == STABLE) {
			m_Status = UNSTABLE;
		}
		m_StableRecoverTimer.Reset();
	}

	// Rotational balancing spring calc
	if (m_Status == STABLE) {

		// If we're supposed to be laying down on the ground, make the spring pull the body that way until we reach that angle
		if (m_ProneState != NOTPRONE) {
			float rotTarget = m_HFlipped ? c_HalfPI : -c_HalfPI;
			float rotDiff = rotTarget - rot;

			if (m_ProneState == GOPRONE) {
				if (!m_ProneTimer.IsPastSimMS(333)) {
					if (std::abs(rotDiff) > 0.1F && std::abs(rotDiff) < c_PI) {
						m_AngularVel += rotDiff * 0.4F;
						m_Vel.m_X += (m_HFlipped ? -std::abs(rotDiff) : std::abs(rotDiff)) / std::max(m_Vel.GetMagnitude(), 4.0F);
					}
				} else {
					// Done going down, now stay down without spring.
					m_AngularVel *= 0.5F;
					m_ProneState = LAYINGPRONE;
				}
			} else if (m_ProneState == LAYINGPRONE) {
				// If down, try to keep flat against the ground.
				if (std::abs(rotDiff) > c_SixteenthPI && std::abs(rotDiff) < c_HalfPI) {
					m_AngularVel += rotDiff * 0.65F;
				} else if (std::abs(m_AngularVel) > 0.3F) {
					m_AngularVel *= 0.85F;
				}
			}
		} else {
			// Upright body posture
			float rotTarget = (GetRotAngleTarget(m_MoveState) * (m_AimAngle > 0 ? 1.0F - (m_AimAngle / c_HalfPI) : 1.0F) * GetFlipFactor());

			if (m_MoveState != CROUCH) {
				// In crouch state the above is rotated already, but in any other state we do the incremental lean here
				float crouchAngleAdjust = m_HFlipped ? -m_RotAngleTargets[CROUCH] : m_RotAngleTargets[CROUCH];
				float difference = crouchAngleAdjust - rotTarget;
				rotTarget += Lerp(0.0F, 1.0F, 0.0F, difference, m_CrouchAmount);
			}
			
			float rotDiff = rot - rotTarget;
			if (std::abs(rotDiff) > c_PI * 0.75F) {
				// We've h-flipped, so just snap to new orientation
				rot = rotTarget;
			} else {
				// Lerp towards the angle
				m_AngularVel = m_AngularVel * (0.98F - 0.06F * (m_Health / m_MaxHealth)) - (rotDiff * 0.5F);
			}
			
		}
	} else if (m_Status == UNSTABLE) {
		float rotTarget = 0;
		// If traveling at speed, always start falling forward.
		if (std::abs(m_Vel.m_X) > 1.0F) {
			rotTarget = m_HFlipped ? c_HalfPI : -c_HalfPI;
		} else {
			// Otherwise, go whichever way we're already rotated.
			rotTarget = rot > 0 ? c_HalfPI : -c_HalfPI;
		}

		float rotDiff = rotTarget - rot;
		if (std::abs(rotDiff) > 0.1F && std::abs(rotDiff) < c_PI) {
			m_AngularVel += rotDiff * 0.05F;
		}
	} else if (m_Status == DYING) {
		float rotTarget = m_Vel.m_X - (rot + m_AngularVel) > 0 ? -c_HalfPI : c_HalfPI;
		float rotDiff = rotTarget - rot;
		if (!m_DeathTmr.IsPastSimMS(125) && std::abs(rotDiff) > 0.1F && std::abs(rotDiff) < c_PI) {
			// TODO: finetune this for situations like low gravity!
			float velScalar = 0.5F; //* (g_SceneMan.GetGlobalAcc().GetY() * m_GlobalAccScalar) / c_PPM;
			m_AngularVel += rotDiff * velScalar;
			m_Vel.m_X += (rotTarget > 0 ? -std::abs(rotDiff) : std::abs(rotDiff)) * velScalar * 0.5F;
		} else {
			m_Status = DEAD;
		}
	}
	m_Rotation.SetRadAngle(rot);

	///////////////////////////////////////////////////
	// Death detection and handling

	if (!m_pHead && m_Status != DYING && m_Status != DEAD) {
		m_Health -= m_MaxHealth + 1.0F;
	} else if (!m_pFGArm && !m_pBGArm && !m_pFGLeg && !m_pBGLeg && m_Status != DYING && m_Status != DEAD) {
		m_Health -= 0.1F;
	}

	if (m_Status == DYING) {
		if (m_pFGArm) {
			m_pFGArm->RemoveAttachable(m_pFGArm->GetHeldDevice(), true, false);
		}
		if (m_pBGArm) {
			m_pBGArm->RemoveAttachable(m_pBGArm->GetHeldDevice(), true, false);
		}
	}

	/////////////////////////////////////////
	// Misc.

	//    m_DeepCheck = true/*m_Status == DEAD*/;
}

void AHuman::DrawThrowingReticle(BITMAP* targetBitmap, const Vector& targetPos, float progressScalar) const {
	const int pointCount = 9;
	Vector points[pointCount];

	for (int index = 0; index < pointCount; index++) {
		points[index].SetXY(static_cast<float>(index * 4), 0.0F);
	}
	Vector outOffset(m_pFGArm->GetMaxLength() * GetFlipFactor(), -m_pFGArm->GetMaxLength() * 0.5F);
	float adjustedAimAngle = m_AimAngle * GetFlipFactor();

	for (int i = 0; i < pointCount * progressScalar; ++i) {
		points[i].FlipX(m_HFlipped);
		points[i] += outOffset;
		points[i].RadRotate(adjustedAimAngle);
		points[i] += m_pFGArm->GetJointPos();

		g_PostProcessMan.RegisterGlowDotEffect(points[i], YellowDot, RandomNum(63, 127));
		putpixel(targetBitmap, points[i].GetFloorIntX() - targetPos.GetFloorIntX(), points[i].GetFloorIntY() - targetPos.GetFloorIntY(), g_YellowGlowColor);
	}
}

void AHuman::Draw(BITMAP* pTargetBitmap, const Vector& targetPos, DrawMode mode, bool onlyPhysical) const {
	Actor::Draw(pTargetBitmap, targetPos, mode, onlyPhysical);

	DrawMode realMode = (mode == g_DrawColor && !m_FlashWhiteTimer.IsPastRealTimeLimit()) ? g_DrawWhite : mode;
	// Note: For some reason the ordering of the attachables list can get messed up. The most important thing here is that the FGArm is on top of everything else.
	if (m_pHead && m_pHead->IsDrawnAfterParent()) {
		m_pHead->Draw(pTargetBitmap, targetPos, realMode, onlyPhysical);
	}
	if (m_pFGArm) {
		m_pFGArm->Draw(pTargetBitmap, targetPos, realMode, onlyPhysical);
	}

	// Draw background Arm's hand after the HeldDevice of FGArm is drawn if the FGArm is holding a weapon.
	if (m_pFGArm && m_pBGArm && !onlyPhysical && mode == g_DrawColor && m_pBGArm->GetHandHasReachedCurrentTarget() && !GetEquippedBGItem()) {
		if (HeldDevice* heldDevice = m_pFGArm->GetHeldDevice(); heldDevice && !dynamic_cast<ThrownDevice*>(heldDevice) && !heldDevice->IsReloading() && !heldDevice->IsShield()) {
			m_pBGArm->DrawHand(pTargetBitmap, targetPos, realMode);
		}
	}

	if (mode == g_DrawColor && !onlyPhysical && g_SettingsMan.DrawHandAndFootGroupVisualizations()) {
		m_pFGFootGroup->Draw(pTargetBitmap, targetPos, true, 13);
		m_pBGFootGroup->Draw(pTargetBitmap, targetPos, true, 13);
		m_pFGHandGroup->Draw(pTargetBitmap, targetPos, true, 13);
		m_pBGHandGroup->Draw(pTargetBitmap, targetPos, true, 13);
	}

	if (mode == g_DrawColor && !onlyPhysical && g_SettingsMan.DrawLimbPathVisualizations()) {
		m_Paths[m_HFlipped][WALK].Draw(pTargetBitmap, targetPos, 122);
		m_Paths[m_HFlipped][CRAWL].Draw(pTargetBitmap, targetPos, 122);
		m_Paths[m_HFlipped][ARMCRAWL].Draw(pTargetBitmap, targetPos, 13);
		m_Paths[m_HFlipped][CLIMB].Draw(pTargetBitmap, targetPos, 165);
	}
}

void AHuman::DrawHUD(BITMAP* pTargetBitmap, const Vector& targetPos, int whichScreen, bool playerControlled) {
	m_HUDStack = -m_CharHeight / 2;

	// Only do HUD if on a team
	if (m_Team < 0) {
		return;
	}

	// Only draw if the team viewing this is on the same team OR has seen the space where this is located.
	int viewingTeam = g_ActivityMan.GetActivity()->GetTeamOfPlayer(g_ActivityMan.GetActivity()->PlayerOfScreen(whichScreen));
	if (viewingTeam != m_Team && viewingTeam != Activity::NoTeam && (!g_SettingsMan.ShowEnemyHUD() || g_SceneMan.IsUnseen(m_Pos.GetFloorIntX(), m_Pos.GetFloorIntY(), viewingTeam))) {
		return;
	}

	Actor::DrawHUD(pTargetBitmap, targetPos, whichScreen);

	if (!m_HUDVisible) {
		return;
	}

#ifdef DEBUG_BUILD
	// Limbpath debug drawing
	m_Paths[FGROUND][WALK].Draw(pTargetBitmap, targetPos, 122);
	m_Paths[FGROUND][CRAWL].Draw(pTargetBitmap, targetPos, 122);
	m_Paths[FGROUND][ARMCRAWL].Draw(pTargetBitmap, targetPos, 13);
	m_Paths[FGROUND][CLIMB].Draw(pTargetBitmap, targetPos, 98);

	m_Paths[BGROUND][WALK].Draw(pTargetBitmap, targetPos, 122);
	m_Paths[BGROUND][CRAWL].Draw(pTargetBitmap, targetPos, 122);
	m_Paths[BGROUND][ARMCRAWL].Draw(pTargetBitmap, targetPos, 13);
	m_Paths[BGROUND][CLIMB].Draw(pTargetBitmap, targetPos, 98);

	// Draw the AI paths
	std::list<Vector>::iterator last = m_MovePath.begin();
	Vector waypoint, lastPoint, lineVec;
	for (std::list<Vector>::iterator lItr = m_MovePath.begin(); lItr != m_MovePath.end(); ++lItr) {
		lastPoint = (*last) - targetPos;
		waypoint = lastPoint + g_SceneMan.ShortestDistance(lastPoint, (*lItr) - targetPos);
		line(pTargetBitmap, lastPoint.m_X, lastPoint.m_Y, waypoint.m_X, waypoint.m_Y, g_RedColor);
		last = lItr;
	}
	waypoint = m_MoveTarget - targetPos;
	circlefill(pTargetBitmap, waypoint.m_X, waypoint.m_Y, 3, g_RedColor);
	lastPoint = m_PrevPathTarget - targetPos;
	circlefill(pTargetBitmap, lastPoint.m_X, lastPoint.m_Y, 2, g_YellowGlowColor);
	lastPoint = m_DigTunnelEndPos - targetPos;
	circlefill(pTargetBitmap, lastPoint.m_X, lastPoint.m_Y, 2, g_YellowGlowColor);
	// Radius
//    waypoint = m_Pos - targetPos;
//    circle(pTargetBitmap, waypoint.m_X, waypoint.m_Y, m_MoveProximityLimit, g_RedColor);
#endif

	// Player AI drawing

	if (m_pFGArm && m_pFGArm->GetHeldDevice()) {
		// Draw the aiming dots for the currently held device.
		if (m_ArmsState == THROWING_PREP) {
			DrawThrowingReticle(pTargetBitmap, targetPos, GetThrowProgress());
		} else if (m_Controller.IsState(AIM_SHARP) || (m_Controller.IsPlayerControlled() && !m_Controller.IsState(PIE_MENU_ACTIVE))) {
			m_pFGArm->GetHeldDevice()->DrawHUD(pTargetBitmap, targetPos, whichScreen, m_Controller.IsState(AIM_SHARP) && m_Controller.IsPlayerControlled());
		}
	}

	//////////////////////////////////////
	// Draw stat info HUD
	char str[64];

	GUIFont* pSymbolFont = g_FrameMan.GetLargeFont();
	GUIFont* pSmallFont = g_FrameMan.GetSmallFont();

	// Only show extra HUD if this guy is controlled by the same player that this screen belongs to
	if (m_Controller.IsPlayerControlled() && g_ActivityMan.GetActivity()->ScreenOfPlayer(m_Controller.GetPlayer()) == whichScreen && pSmallFont && pSymbolFont) {
		AllegroBitmap allegroBitmap(pTargetBitmap);
		/*
		        // Device aiming reticle
		        if (m_Controller.IsState(AIM_SHARP) &&
		            m_pFGArm && m_pFGArm->IsAttached() && m_pFGArm->HoldsHeldDevice())
		            m_pFGArm->GetHeldDevice()->DrawHUD(pTargetBitmap, targetPos, whichScreen);*/

		Vector drawPos = m_Pos - targetPos;

		// Adjust the draw position to work if drawn to a target screen bitmap that is straddling a scene seam
		if (!targetPos.IsZero()) {
			// Spans vertical scene seam
			int sceneWidth = g_SceneMan.GetSceneWidth();
			if (g_SceneMan.SceneWrapsX() && pTargetBitmap->w < sceneWidth) {
				if ((targetPos.m_X < 0) && (m_Pos.m_X > (sceneWidth - pTargetBitmap->w)))
					drawPos.m_X -= sceneWidth;
				else if (((targetPos.m_X + pTargetBitmap->w) > sceneWidth) && (m_Pos.m_X < pTargetBitmap->w))
					drawPos.m_X += sceneWidth;
			}
			// Spans horizontal scene seam
			int sceneHeight = g_SceneMan.GetSceneHeight();
			if (g_SceneMan.SceneWrapsY() && pTargetBitmap->h < sceneHeight) {
				if ((targetPos.m_Y < 0) && (m_Pos.m_Y > (sceneHeight - pTargetBitmap->h)))
					drawPos.m_Y -= sceneHeight;
				else if (((targetPos.m_Y + pTargetBitmap->h) > sceneHeight) && (m_Pos.m_Y < pTargetBitmap->h))
					drawPos.m_Y += sceneHeight;
			}
		}

		if (m_pFGArm || m_pBGArm) {
			// Held-related GUI stuff
			HDFirearm* fgHeldFirearm = dynamic_cast<HDFirearm*>(GetEquippedItem());
			HDFirearm* bgHeldFirearm = dynamic_cast<HDFirearm*>(GetEquippedBGItem());

			if (fgHeldFirearm || bgHeldFirearm) {
				str[0] = -56;
				str[1] = 0;

				std::string fgWeaponString = "EMPTY";
				if (fgHeldFirearm) {
					if (fgHeldFirearm->IsReloading()) {
						fgWeaponString = "Reloading";
						int barColorIndex = 77;
						if (!fgHeldFirearm->GetSupportAvailable()) {
							float reloadMultiplier = fgHeldFirearm->GetOneHandedReloadTimeMultiplier();
							if (reloadMultiplier != 1.0F) {
								// Add a hand icon next to the ammo icon when reloading without supporting hand.
								str[0] = -37;
								str[1] = -49;
								str[2] = -56;
								str[3] = 0;
								if (reloadMultiplier > 1.0F) {
									if (m_IconBlinkTimer.AlternateSim(250)) {
										barColorIndex = 13;
									}
								} else {
									barColorIndex = 133;
								}
							}
						}
						rectfill(pTargetBitmap, drawPos.GetFloorIntX() + 1, drawPos.GetFloorIntY() + m_HUDStack + 13, drawPos.GetFloorIntX() + 29, drawPos.GetFloorIntY() + m_HUDStack + 14, 245);
						rectfill(pTargetBitmap, drawPos.GetFloorIntX(), drawPos.GetFloorIntY() + m_HUDStack + 12, drawPos.GetFloorIntX() + static_cast<int>(28.0F * fgHeldFirearm->GetReloadProgress() + 0.5F), drawPos.GetFloorIntY() + m_HUDStack + 13, barColorIndex);
					} else {
						fgWeaponString = fgHeldFirearm->GetRoundInMagCount() < 0 ? "Infinite" : std::to_string(fgHeldFirearm->GetRoundInMagCount());
					}
				}

				std::string bgWeaponString;
				if (bgHeldFirearm) {
					if (bgHeldFirearm->IsReloading()) {
						bgWeaponString = "Reloading";
						int barColorIndex = 77;
						if (!bgHeldFirearm->GetSupportAvailable()) {
							float reloadMultiplier = bgHeldFirearm->GetOneHandedReloadTimeMultiplier();
							if (reloadMultiplier != 1.0F) {
								// Add a hand icon next to the ammo icon when reloading without supporting hand.
								str[0] = -37;
								str[1] = -49;
								str[2] = -56;
								str[3] = 0;
								if (reloadMultiplier > 1.0F) {
									if (m_IconBlinkTimer.AlternateSim(250)) {
										barColorIndex = 13;
									}
								} else {
									barColorIndex = 133;
								}
							}
						}
						int totalTextWidth = pSmallFont->CalculateWidth(fgWeaponString) + 6;
						rectfill(pTargetBitmap, drawPos.GetFloorIntX() + 1 + totalTextWidth, drawPos.GetFloorIntY() + m_HUDStack + 13, drawPos.GetFloorIntX() + 29 + totalTextWidth, drawPos.GetFloorIntY() + m_HUDStack + 14, 245);
						rectfill(pTargetBitmap, drawPos.GetFloorIntX() + totalTextWidth, drawPos.GetFloorIntY() + m_HUDStack + 12, drawPos.GetFloorIntX() + static_cast<int>(28.0F * bgHeldFirearm->GetReloadProgress() + 0.5F) + totalTextWidth, drawPos.GetFloorIntY() + m_HUDStack + 13, barColorIndex);
					} else {
						bgWeaponString = bgHeldFirearm->GetRoundInMagCount() < 0 ? "Infinite" : std::to_string(bgHeldFirearm->GetRoundInMagCount());
					}
				}
				pSymbolFont->DrawAligned(&allegroBitmap, drawPos.GetFloorIntX() - pSymbolFont->CalculateWidth(str) - 3, drawPos.GetFloorIntY() + m_HUDStack, str, GUIFont::Left);
				std::snprintf(str, sizeof(str), bgHeldFirearm ? "%s | %s" : "%s", fgWeaponString.c_str(), bgWeaponString.c_str());
				pSmallFont->DrawAligned(&allegroBitmap, drawPos.GetFloorIntX(), drawPos.GetFloorIntY() + m_HUDStack + 3, str, GUIFont::Left);

				m_HUDStack -= 9;
			}
			if (m_Controller.IsState(PIE_MENU_ACTIVE) || !m_EquipHUDTimer.IsPastRealMS(700)) {
				HeldDevice* fgEquippedItem = GetEquippedItem();
				HeldDevice* bgEquippedItem = GetEquippedBGItem();
				std::string equippedItemsString = (fgEquippedItem ? fgEquippedItem->GetPresetName() : "EMPTY") + (bgEquippedItem ? " | " + bgEquippedItem->GetPresetName() : "");
				pSmallFont->DrawAligned(&allegroBitmap, drawPos.GetFloorIntX() + 1, drawPos.GetFloorIntY() + m_HUDStack + 3, equippedItemsString, GUIFont::Centre);
				m_HUDStack -= 9;
			}
		} else {
			std::snprintf(str, sizeof(str), "NO ARM!");
			pSmallFont->DrawAligned(&allegroBitmap, drawPos.m_X + 2, drawPos.m_Y + m_HUDStack + 3, str, GUIFont::Centre);
			m_HUDStack -= 9;
		}

		if (m_pJetpack && m_Status != INACTIVE && !m_Controller.IsState(PIE_MENU_ACTIVE) && (m_Controller.IsState(BODY_JUMP) || !m_pJetpack->IsFullyFueled())) {
			if (m_pJetpack->GetJetTimeLeft() < 100.0F) {
				str[0] = m_IconBlinkTimer.AlternateSim(100) ? -26 : -25;
			} else if (m_pJetpack->IsEmitting()) {
				float acceleration = m_pJetpack->EstimateImpulse(false) / std::max(GetMass(), 0.1F);
				if (acceleration > 0.41F) {
					str[0] = acceleration > 0.47F ? -31 : -30;
				} else {
					str[0] = acceleration > 0.35F ? -29 : -28;
					if (m_IconBlinkTimer.AlternateSim(200)) {
						str[0] = -27;
					}
				}
			} else {
				str[0] = -27;
			}
			str[1] = 0;
			pSymbolFont->DrawAligned(&allegroBitmap, drawPos.GetFloorIntX() - 7, drawPos.GetFloorIntY() + m_HUDStack, str, GUIFont::Centre);

			rectfill(pTargetBitmap, drawPos.GetFloorIntX() + 1, drawPos.GetFloorIntY() + m_HUDStack + 7, drawPos.GetFloorIntX() + 15, drawPos.GetFloorIntY() + m_HUDStack + 8, 245);
			if (m_pJetpack->GetJetTimeTotal() > 0) {
				float jetTimeRatio = m_pJetpack->GetJetTimeRatio();
				int gaugeColor;
				if (jetTimeRatio > 0.75F) {
					gaugeColor = 149;
				} else if (jetTimeRatio > 0.5F) {
					gaugeColor = 133;
				} else if (jetTimeRatio > 0.375F) {
					gaugeColor = 77;
				} else if (jetTimeRatio > 0.25F) {
					gaugeColor = 48;
				} else {
					gaugeColor = 13;
				}
				rectfill(pTargetBitmap, drawPos.GetFloorIntX(), drawPos.GetFloorIntY() + m_HUDStack + 6, drawPos.GetFloorIntX() + static_cast<int>(15.0F * jetTimeRatio), drawPos.GetFloorIntY() + m_HUDStack + 7, gaugeColor);
			}
			m_HUDStack -= 9;
		}

		// Pickup GUI
		if (!m_Controller.IsState(PIE_MENU_ACTIVE) && m_pItemInReach) {
			std::snprintf(str, sizeof(str), " %c %s", -49, m_pItemInReach->GetPresetName().c_str());
			pSmallFont->DrawAligned(&allegroBitmap, drawPos.GetFloorIntX(), drawPos.GetFloorIntY() + m_HUDStack + 3, str, GUIFont::Centre);
			m_HUDStack -= 9;
		}
	}
}

float AHuman::GetLimbPathSpeed(int speedPreset) const {
	return m_Paths[FGROUND][WALK].GetSpeed(speedPreset);
}

void AHuman::SetLimbPathSpeed(int speedPreset, float speed) {
	m_Paths[FGROUND][WALK].OverrideSpeed(speedPreset, speed);
	m_Paths[BGROUND][WALK].OverrideSpeed(speedPreset, speed);
}

float AHuman::GetLimbPathPushForce() const {
	return m_Paths[FGROUND][WALK].GetDefaultPushForce();
}

void AHuman::SetLimbPathPushForce(float force) {
	m_Paths[FGROUND][WALK].OverridePushForce(force);
	m_Paths[BGROUND][WALK].OverridePushForce(force);
}

int AHuman::WhilePieMenuOpenListener(const PieMenu* pieMenu) {
	int result = Actor::WhilePieMenuOpenListener(pieMenu);

	for (PieSlice* pieSlice: GetPieMenu()->GetPieSlices()) {
		switch (pieSlice->GetType()) {
			case PieSliceType::Pickup:
			case PieSliceType::Reload:
				pieSlice->SetType(m_pItemInReach ? PieSliceType::Pickup : PieSliceType::Reload);
				pieSlice->SetIcon(dynamic_cast<Icon*>(g_PresetMan.GetEntityPreset("Icon", m_pItemInReach ? "Pick Up" : "Refresh")->Clone()));

				if (pieSlice->GetType() == PieSliceType::Pickup) {
					if (m_pFGArm || (m_pBGArm && m_pItemInReach->IsOneHanded())) {
						pieSlice->SetEnabled(m_Status != INACTIVE);
						pieSlice->SetDescription("Pick Up " + m_pItemInReach->GetPresetName());
					} else {
						pieSlice->SetEnabled(false);
						pieSlice->SetDescription("No Arm");
					}
				} else {
					const HeldDevice* fgHeldDevice = dynamic_cast<const HeldDevice*>(GetEquippedItem());
					const HeldDevice* bgHeldDevice = dynamic_cast<const HeldDevice*>(GetEquippedBGItem());
					if (fgHeldDevice || bgHeldDevice) {
						pieSlice->SetEnabled(m_Status != INACTIVE && ((fgHeldDevice && !fgHeldDevice->IsFull()) || (bgHeldDevice && !bgHeldDevice->IsFull())));
						pieSlice->SetDescription("Reload");
					} else {
						pieSlice->SetEnabled(false);
						pieSlice->SetDescription(m_pFGArm ? "Not Holding Anything" : "No Arm");
					}
				}
				break;
			case PieSliceType::NextItem:
				if (!IsInventoryEmpty() && m_pFGArm) {
					pieSlice->SetEnabled(m_Status != INACTIVE);
					pieSlice->SetDescription("Next Item");
				} else {
					pieSlice->SetEnabled(false);
					pieSlice->SetDescription(m_pFGArm ? "Not Holding Anything" : "No Arm");
				}
				break;
			case PieSliceType::PreviousItem:
				if (!IsInventoryEmpty() && m_pFGArm) {
					pieSlice->SetEnabled(m_Status != INACTIVE);
					pieSlice->SetDescription("Prev Item");
				} else {
					pieSlice->SetEnabled(false);
					pieSlice->SetDescription(m_pFGArm ? "Not Holding Anything" : "No Arm");
				}
				break;
			case PieSliceType::Drop:
				if (const MovableObject* equippedFGItem = GetEquippedItem()) {
					pieSlice->SetEnabled(m_Status != INACTIVE);
					pieSlice->SetDescription("Drop " + equippedFGItem->GetPresetName());
				} else if (const MovableObject* equippedBGItem = GetEquippedBGItem()) {
					pieSlice->SetDescription("Drop " + equippedBGItem->GetPresetName());
					pieSlice->SetEnabled(m_Status != INACTIVE);
				} else if (!IsInventoryEmpty() && !m_pFGArm) {
					pieSlice->SetEnabled(m_Status != INACTIVE);
					pieSlice->SetDescription("Drop Inventory");
				} else {
					pieSlice->SetEnabled(false);
					pieSlice->SetDescription((m_pFGArm || m_pBGArm) ? "Not Holding Anything" : "No Arm");
				}
				break;
		}
	}
	return result;
}
