#include "Leg.h"
#include "PresetMan.h"
#include "AtomGroup.h"

using namespace RTE;

ConcreteClassInfo(Leg, Attachable, 50);

Leg::Leg() {
	Clear();
}

Leg::~Leg() {
	Destroy(true);
}

void Leg::Destroy(bool notInherited) {
	if (!notInherited) {
		Attachable::Destroy();
	}
	Clear();
}

void Leg::Reset() {
	Clear();
	Attachable::Reset();
}

void Leg::Clear() {
	m_Foot = nullptr;

	m_ContractedOffset.Reset();
	m_ExtendedOffset.Reset();

	m_MinExtension = 0;
	m_MaxExtension = 0;
	m_NormalizedExtension = 0;

	m_TargetPosition.Reset();
	m_IdleOffset.Reset();

	m_AnkleOffset.Reset();

	m_WillIdle = false;
	m_MoveSpeed = 0;
}

int Leg::Create() {
	if (Attachable::Create() < 0) {
		return -1;
	}

	// Ensure Legs don't get flagged as inheriting RotAngle, since they never do and always set their RotAngle for themselves.
	m_InheritsRotAngle = false;

	// Ensure Legs don't collide with terrain when attached since their expansion/contraction is frame based so atom group doesn't know how to account for it.
	SetCollidesWithTerrainWhileAttached(false);

	if (m_ContractedOffset.GetSqrMagnitude() > m_ExtendedOffset.GetSqrMagnitude()) {
		std::swap(m_ContractedOffset, m_ExtendedOffset);
	}

	m_MinExtension = m_ContractedOffset.GetMagnitude();
	m_MaxExtension = m_ExtendedOffset.GetMagnitude();

	return 0;
}

int Leg::Create(const Leg& reference) {
	if (reference.m_Foot) {
		m_ReferenceHardcodedAttachableUniqueIDs.insert(reference.m_Foot->GetUniqueID());
		SetFoot(dynamic_cast<Attachable*>(reference.m_Foot->Clone()));
	}
	Attachable::Create(reference);

	m_ContractedOffset = reference.m_ContractedOffset;
	m_ExtendedOffset = reference.m_ExtendedOffset;

	m_MinExtension = reference.m_MinExtension;
	m_MaxExtension = reference.m_MaxExtension;
	m_NormalizedExtension = reference.m_NormalizedExtension;

	m_TargetPosition = reference.m_TargetPosition;
	m_IdleOffset = reference.m_IdleOffset;

	m_AnkleOffset = reference.m_AnkleOffset;

	m_WillIdle = reference.m_WillIdle;
	m_MoveSpeed = reference.m_MoveSpeed;

	return 0;
}

int Leg::ReadProperty(const std::string_view& propName, Reader& reader) {
	StartPropertyList(return Attachable::ReadProperty(propName, reader));

	MatchProperty("Foot", { SetFoot(dynamic_cast<Attachable*>(g_PresetMan.ReadReflectedPreset(reader))); });
	MatchProperty("ContractedOffset", {
		reader >> m_ContractedOffset;
		m_MinExtension = m_ContractedOffset.GetMagnitude();
	});
	MatchProperty("ExtendedOffset", {
		reader >> m_ExtendedOffset;
		m_MaxExtension = m_ExtendedOffset.GetMagnitude();
	});
	MatchProperty("IdleOffset", { reader >> m_IdleOffset; });
	MatchProperty("WillIdle", { reader >> m_WillIdle; });
	MatchProperty("MoveSpeed", { reader >> m_MoveSpeed; });

	EndPropertyList;
}

int Leg::Save(Writer& writer) const {
	Attachable::Save(writer);

	writer.NewProperty("Foot");
	writer << m_Foot;
	writer.NewProperty("ContractedOffset");
	writer << m_ContractedOffset;
	writer.NewProperty("ExtendedOffset");
	writer << m_ExtendedOffset;
	writer.NewProperty("IdleOffset");
	writer << m_IdleOffset;
	writer.NewProperty("WillIdle");
	writer << m_WillIdle;
	writer.NewProperty("MoveSpeed");
	writer << m_MoveSpeed;

	return 0;
}

Attachable* Leg::GetFoot() const {
	return m_Foot;
}

void Leg::SetFoot(Attachable* newFoot) {
	if (m_Foot && m_Foot->IsAttached()) {
		RemoveAndDeleteAttachable(m_Foot);
	}
	if (newFoot == nullptr) {
		m_Foot = nullptr;
	} else {
		m_Foot = newFoot;
		AddAttachable(newFoot);

		m_HardcodedAttachableUniqueIDsAndSetters.insert({newFoot->GetUniqueID(), [](MOSRotating* parent, Attachable* attachable) {
			                                                 dynamic_cast<Leg*>(parent)->SetFoot(attachable);
		                                                 }});

		if (m_Foot->HasNoSetDamageMultiplier()) {
			m_Foot->SetDamageMultiplier(1.0F);
		}
		m_Foot->SetInheritsRotAngle(false);
		m_Foot->SetParentGibBlastStrengthMultiplier(0.0F);
		m_Foot->SetCollidesWithTerrainWhileAttached(false);
	}
}

float Leg::GetMinLength() const {
	return m_MinExtension;
}

float Leg::GetMaxLength() const {
	return m_MaxExtension;
}

float Leg::GetMoveSpeed() const {
	return m_MoveSpeed;
}

void Leg::SetMoveSpeed(float newMoveSpeed) {
	m_MoveSpeed = newMoveSpeed;
}

void Leg::SetTargetPosition(const Vector& targetPosition) {
	m_TargetPosition = targetPosition;
}

void Leg::EnableIdle(bool idle) {
	m_WillIdle = idle;
}

AtomGroup* Leg::GetFootGroupFromFootAtomGroup() {
	if (!m_Foot) {
		return nullptr;
	}

	AtomGroup* footGroup = dynamic_cast<AtomGroup*>(m_Foot->GetAtomGroup()->Clone());

	int atomLeftMostOffset = m_Foot->GetSpriteWidth();
	int atomTopMostOffset = m_Foot->GetSpriteHeight();
	int atomBottomMostOffset = 0;

	for (const Atom* atom: footGroup->GetAtomList()) {
		int atomOffsetX = atom->GetOriginalOffset().GetFloorIntX();
		int atomOffsetY = atom->GetOriginalOffset().GetFloorIntY();
		// Auto-generated AtomGroups are created empty so a single Atom is added at 0,0. Ignore it and any others to not screw up detecting the left-most and top-most offsets.
		if (atomOffsetX != 0 && atomOffsetY != 0) {
			atomLeftMostOffset = std::min(atomLeftMostOffset, atomOffsetX);
			atomTopMostOffset = std::min(atomTopMostOffset, atomOffsetY);
			atomBottomMostOffset = std::max(atomBottomMostOffset, atomOffsetY);
		}
	}
	int groupCenterOffsetY = (atomTopMostOffset + atomBottomMostOffset) / 2;

	std::vector<Atom*> filteredAtomList;

	// We want the FootGroup to end up with an "L" shape, so filter all the top and right Atoms while taking into account the heel might be slant and the sole might not be flat. The extra Atoms are not necessary and might (further) screw up some walking physics.
	// Start from the bottom so we can filter any Atom that might be above the bottom-most one on the same X offset.
	for (auto atomItr = footGroup->GetAtomList().crbegin(); atomItr != footGroup->GetAtomList().crend(); ++atomItr) {
		int atomOffsetX = (*atomItr)->GetOriginalOffset().GetFloorIntX();
		int atomOffsetY = (*atomItr)->GetOriginalOffset().GetFloorIntY();

		bool haveBottomMostAtomOnThisXOffset = false;
		for (const Atom* filteredAtom: filteredAtomList) {
			haveBottomMostAtomOnThisXOffset = (filteredAtom->GetOriginalOffset().GetFloorIntX() == atomOffsetX) && (filteredAtom->GetOriginalOffset().GetFloorIntY() > atomOffsetY);
			if (haveBottomMostAtomOnThisXOffset) {
				break;
			}
		}

		if (atomOffsetX == atomLeftMostOffset || atomOffsetY == atomBottomMostOffset || (!haveBottomMostAtomOnThisXOffset && atomOffsetX > atomLeftMostOffset && atomOffsetY >= groupCenterOffsetY)) {
			Atom* newAtom = new Atom(*(*atomItr));
			newAtom->SetMaterial(g_SceneMan.GetMaterialFromID(MaterialColorKeys::g_MaterialRubber));

			filteredAtomList.emplace_back(newAtom);
		}
	}
	footGroup->SetAtomList(filteredAtomList);
	footGroup->SetJointOffset(m_Foot->GetJointOffset());

	return footGroup;
}

void Leg::Update() {
	Attachable::PreUpdate();

	UpdateCurrentAnkleOffset();

	UpdateLegRotation();

	if (m_Foot) {
		// In order to keep the foot in the right place, we need to convert its offset (the ankle offset) to work as the ParentOffset for the foot.
		// The foot will then use this to set its JointPos when it's updated. Unfortunately UnRotateOffset doesn't work for this, since it's Vector/Matrix division, which isn't commutative.
		Vector ankleOffsetAsParentOffset = RotateOffset(m_JointOffset) + m_AnkleOffset;
		ankleOffsetAsParentOffset.RadRotate(-m_Rotation.GetRadAngle()).FlipX(m_HFlipped);
		m_Foot->SetParentOffset(ankleOffsetAsParentOffset);
	}

	Attachable::Update();

	if (m_FrameCount != 1) {
		m_NormalizedExtension = std::clamp((m_AnkleOffset.GetMagnitude() - m_MinExtension) / (m_MaxExtension - m_MinExtension), 0.0F, 1.0F);
		m_Frame = std::min(m_FrameCount - 1, static_cast<unsigned int>(std::floor(m_NormalizedExtension * static_cast<float>(m_FrameCount))));
	}

	UpdateFootFrameAndRotation();
}

void Leg::UpdateCurrentAnkleOffset() {
	if (IsAttached()) {
		Vector targetOffset = g_SceneMan.ShortestDistance(m_JointPos, m_TargetPosition, g_SceneMan.SceneWrapsX());
		Vector rotatedTargetOffset = targetOffset.GetRadRotatedCopy(m_Parent->GetRotAngle());
		if (m_WillIdle && rotatedTargetOffset.m_Y < -std::abs(rotatedTargetOffset.m_X)) {
			targetOffset = m_Parent->RotateOffset(m_IdleOffset);
		}

		Vector distanceFromTargetOffsetToAnkleOffset(targetOffset - m_AnkleOffset);
		m_AnkleOffset += distanceFromTargetOffsetToAnkleOffset * m_MoveSpeed;
		m_AnkleOffset.ClampMagnitude(m_MaxExtension, m_MinExtension + 0.1F);
	} else {
		m_AnkleOffset.SetXY(m_MaxExtension * 0.60F, 0);
		m_AnkleOffset.RadRotate((m_HFlipped ? c_PI : 0) + m_Rotation.GetRadAngle());
	}
}

void Leg::UpdateLegRotation() {
	if (IsAttached()) {
		m_Rotation = m_AnkleOffset.GetAbsRadAngle() + (m_HFlipped ? c_PI : 0);

		// Get a normalized scalar for where the Leg should be rotated to between the contracted and extended offsets. EaseOut is used to get the sine effect needed.
		float extraRotationRatio = (EaseOut(m_MinExtension, m_MaxExtension, m_NormalizedExtension) - m_MinExtension) / (m_MaxExtension - m_MinExtension);

		// The contracted offset's inverse angle is the base for the rotation correction.
		float extraRotation = -(m_ContractedOffset.GetAbsRadAngle());

		// Get the actual amount of extra rotation correction needed from the ratio, somewhere on the arc between contracted and extended angles.
		// This is negative because it's a correction, the bitmap needs to rotate back to align the ankle with where it's supposed to be in the sprite.
		extraRotation -= (m_ExtendedOffset.GetAbsRadAngle() - m_ContractedOffset.GetAbsRadAngle()) * extraRotationRatio;

		m_Rotation.SetRadAngle(m_Rotation.GetRadAngle() + extraRotation * GetFlipFactor());
		m_AngularVel = 0.0F;
	}
}

void Leg::UpdateFootFrameAndRotation() {
	if (m_Foot) {
		if (IsAttached() && m_AnkleOffset.GetY() > std::abs(m_AnkleOffset.GetX() * 0.3F)) {
			float ankleOffsetHorizontalDistanceAccountingForFlipping = m_AnkleOffset.GetXFlipped(m_HFlipped).GetX();
			if (ankleOffsetHorizontalDistanceAccountingForFlipping < -m_MaxExtension * 0.6F) {
				m_Foot->SetFrame(3);
			} else if (ankleOffsetHorizontalDistanceAccountingForFlipping < -m_MaxExtension * 0.4F) {
				m_Foot->SetFrame(2);
			} else if (ankleOffsetHorizontalDistanceAccountingForFlipping > m_MaxExtension * 0.4F) {
				m_Foot->SetFrame(1);
			} else {
				m_Foot->SetFrame(0);
			}
			m_Foot->SetRotAngle(0.0F);
		} else {
			m_Foot->SetRotAngle(m_Rotation.GetRadAngle() + c_HalfPI * GetFlipFactor());
		}
	}
}
