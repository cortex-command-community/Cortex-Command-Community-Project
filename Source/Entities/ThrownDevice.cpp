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

	MatchProperty("ActivationSound", { m_ActivationSound = std::shared_ptr<SoundContainer>(g_PresetMan.ReadReflectedPresetOfType<SoundContainer>(reader, propName)); });
	MatchProperty("StartThrowOffset", { reader >> m_StartThrowOffset; });
	MatchProperty("EndThrowOffset", { reader >> m_EndThrowOffset; });
	MatchProperty("MinThrowVel", { reader >> m_MinThrowVel; });
	MatchProperty("MaxThrowVel", { reader >> m_MaxThrowVel; });
	MatchProperty("TriggerDelay", { reader >> m_TriggerDelay; });
	MatchProperty("ActivatesWhenReleased", { reader >> m_ActivatesWhenReleased; });
	MatchProperty("StrikerLever", {
		m_StrikerLever = g_PresetMan.ReadPresetReferenceOfType<MovableObject>(reader, propName);
	});

	EndPropertyList;
}

template <typename WriterType> int ThrownDevice::WriteTo(WriterType& writer) const {
	HeldDevice::WriteTo(writer);

	writer.NewOptionalEntityPointerProperty("ActivationSound", &ThrownDevice::m_ActivationSound);
	writer.NewDistinctProperty("StartThrowOffset", &ThrownDevice::m_StartThrowOffset);
	writer.NewDistinctProperty("EndThrowOffset", &ThrownDevice::m_EndThrowOffset);
	writer.NewDistinctProperty("MinThrowVel", &ThrownDevice::m_MinThrowVel);
	writer.NewDistinctProperty("MaxThrowVel", &ThrownDevice::m_MaxThrowVel);
	writer.NewDistinctProperty("TriggerDelay", &ThrownDevice::m_TriggerDelay);
	writer.NewDistinctProperty("ActivatesWhenReleased", &ThrownDevice::m_ActivatesWhenReleased);
	writer.NewPresetReferenceProperty("StrikerLever", &ThrownDevice::m_StrikerLever);

	return 0;
}
template int ThrownDevice::WriteTo(IniWriter& writer) const;
template int ThrownDevice::WriteTo(HashWriter& writer) const;

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
		if (m_ActivationSound) {
			m_ActivationSound->Play(m_Pos);
		}
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
