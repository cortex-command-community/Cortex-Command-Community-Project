#include "ThrownDevice.h"
#include "PresetMan.h"
#include "SoundContainer.h"

#include "Arm.h"

using namespace RTE;

ConcreteClassInfo(ThrownDevice, HeldDevice, 50);

ThrownDevice::ThrownDevice() {
	Clear();
}

ThrownDevice::~ThrownDevice() {
	Destroy(true);
}

void ThrownDevice::Clear() {
	m_ActivationSound = std::make_shared<SoundContainer>();
	m_ActivationSound->Reset();
	m_StartThrowOffset.Reset();
	m_EndThrowOffset.Reset();
	m_MinThrowVel = 0;
	m_MaxThrowVel = 0;
	m_TriggerDelay = 0;
	m_ActivatesWhenReleased = false;
	m_StrikerLever = nullptr;
}

int ThrownDevice::Create() {
	if (HeldDevice::Create() < 0) {
		return -1;
	}
	m_MOType = MovableObject::TypeThrownDevice;

	return 0;
}

int ThrownDevice::Create(const ThrownDevice& reference) {
	HeldDevice::Create(reference);

	m_MOType = MovableObject::TypeThrownDevice;

	m_ActivationSound = reference.m_ActivationSound;

	m_StartThrowOffset = reference.m_StartThrowOffset;
	m_EndThrowOffset = reference.m_EndThrowOffset;
	m_MinThrowVel = reference.m_MinThrowVel;
	m_MaxThrowVel = reference.m_MaxThrowVel;
	m_TriggerDelay = reference.m_TriggerDelay;
	m_ActivatesWhenReleased = reference.m_ActivatesWhenReleased;
	m_StrikerLever = reference.m_StrikerLever;

	return 0;
}

int ThrownDevice::ReadProperty(const std::string_view& propName, Reader& reader) {
	StartPropertyList(return HeldDevice::ReadProperty(propName, reader));

	MatchProperty("ActivationSound", { reader >> *m_ActivationSound; });
	MatchProperty("StartThrowOffset", { reader >> m_StartThrowOffset; });
	MatchProperty("EndThrowOffset", { reader >> m_EndThrowOffset; });
	MatchProperty("MinThrowVel", { reader >> m_MinThrowVel; });
	MatchProperty("MaxThrowVel", { reader >> m_MaxThrowVel; });
	MatchProperty("TriggerDelay", { reader >> m_TriggerDelay; });
	MatchProperty("ActivatesWhenReleased", { reader >> m_ActivatesWhenReleased; });
	MatchProperty("StrikerLever", { m_StrikerLever = dynamic_cast<const MovableObject*>(g_PresetMan.GetEntityPreset(reader)); });

	EndPropertyList;
}

int ThrownDevice::Save(Writer& writer) const {
	HeldDevice::Save(writer);

	writer.NewProperty("ActivationSound");
	writer << *m_ActivationSound;
	writer.NewPropertyWithValue("StartThrowOffset", m_StartThrowOffset);
	writer.NewPropertyWithValue("EndThrowOffset", m_EndThrowOffset);
	writer.NewPropertyWithValue("MinThrowVel", m_MinThrowVel);
	writer.NewPropertyWithValue("MaxThrowVel", m_MaxThrowVel);
	writer.NewPropertyWithValue("TriggerDelay", m_TriggerDelay);
	writer.NewPropertyWithValue("ActivatesWhenReleased", m_ActivatesWhenReleased);
	writer.NewPropertyWithValue("StrikerLever", m_StrikerLever);

	return 0;
}

uint64_t ThrownDevice::Hash() const {
	uint64_t hash = HeldDevice::Hash();
	hash ^= (m_ActivationSound ? m_ActivationSound->Hash() : 0) << 1;
	hash ^= m_StartThrowOffset.Hash() << 2;
	hash ^= m_EndThrowOffset.Hash() << 3;
	hash ^= std::hash<float>{}(m_MinThrowVel) << 4;
	hash ^= std::hash<float>{}(m_MaxThrowVel) << 5;
	hash ^= std::hash<long>{}(m_TriggerDelay) << 6;
	hash ^= std::hash<bool>{}(m_ActivatesWhenReleased) << 7;
	hash ^= (m_StrikerLever ? RTE::Hash(m_StrikerLever->GetEntityCharacteristic()) : 0) << 8;
	return hash;
}

float ThrownDevice::GetCalculatedMaxThrowVelIncludingArmThrowStrength() {
	if (m_MaxThrowVel > 0) {
		return m_MaxThrowVel;
	} else if (const Arm* parentAsArm = dynamic_cast<Arm*>(GetParent())) {
		return (parentAsArm->GetThrowStrength() + std::abs(GetRootParent()->GetAngularVel() * 0.5F)) / std::sqrt(std::abs(GetMass()) + 1.0F);
	}
	return 0;
}

void ThrownDevice::ResetAllTimers() {
	double elapsedTime = m_Activated ? m_ActivationTimer.GetElapsedSimTimeMS() : 0;
	HeldDevice::ResetAllTimers();
	if (m_Activated) {
		m_ActivationTimer.SetElapsedSimTimeMS(elapsedTime);
	}
}

void ThrownDevice::Activate() {
	if (!m_Activated) {
		m_ActivationTimer.Reset();
		m_ActivationSound->Play(m_Pos);
		if (MovableObject* strikerLever = m_StrikerLever ? dynamic_cast<MovableObject*>(m_StrikerLever->Clone()) : nullptr) {
			Vector randomVel(m_Vel.GetMagnitude() * 0.25F + 1.0F, 0);

			strikerLever->SetVel(m_Vel * 0.5F + randomVel.RadRotate(c_PI * RandomNormalNum()));
			strikerLever->SetPos(m_Pos);
			strikerLever->SetRotAngle(m_Rotation.GetRadAngle());
			strikerLever->SetAngularVel(m_AngularVel + strikerLever->GetAngularVel() * RandomNormalNum());
			strikerLever->SetHFlipped(m_HFlipped);
			strikerLever->SetTeam(m_Team);
			strikerLever->SetIgnoresTeamHits(true);
			g_MovableMan.AddParticle(strikerLever);
		}
		m_Activated = true;
	}
}
