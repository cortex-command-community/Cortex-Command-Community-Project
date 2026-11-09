#include "Round.h"
#include "PresetMan.h"
#include "MOPixel.h"

using namespace RTE;

ConcreteClassInfo(Round, Entity, 500);

Round::Round() {
	Clear();
}

Round::~Round() {
	Destroy(true);
}

void Round::Clear() {
	m_Particle = 0;
	m_ParticleCount = 0;
	m_FireVel = 0;
	m_InheritsFirerVelocity = false;
	m_Separation = 0;
	m_LifeVariation = 0;
	m_Shell = 0;
	m_ShellVel = 0;
	m_FireSound.Reset();
	m_AILifeTime = 0;
	m_AIFireVel = -1;
	m_AIPenetration = -1;
}

int Round::Create() {
	if (Entity::Create() < 0) {
		return -1;
	}

	if (m_AILifeTime == 0) {
		const MovableObject* bullet = GetNextParticle();
		if (bullet) {
			m_AILifeTime = bullet->GetLifetime();
		}
	}
	if (m_AIFireVel < 0) {
		m_AIFireVel = m_FireVel;
	}

	if (m_AIPenetration < 0) {
		const MovableObject* bullet = GetNextParticle();
		m_AIPenetration = (bullet && dynamic_cast<const MOPixel*>(bullet)) ? bullet->GetMass() * bullet->GetSharpness() * m_AIFireVel : 0;
	}

	return 0;
}

int Round::Create(const Round& reference) {
	Entity::Create(reference);

	m_Particle = reference.m_Particle;
	m_ParticleCount = reference.m_ParticleCount;
	m_InheritsFirerVelocity = reference.m_InheritsFirerVelocity;
	m_FireVel = reference.m_FireVel;
	m_Separation = reference.m_Separation;
	m_LifeVariation = reference.m_LifeVariation;
	m_Shell = reference.m_Shell;
	m_ShellVel = reference.m_ShellVel;
	m_FireSound = reference.m_FireSound;
	m_AILifeTime = reference.m_AILifeTime;
	m_AIFireVel = reference.m_AIFireVel;
	m_AIPenetration = reference.m_AIPenetration;

	return 0;
}

int Round::ReadProperty(const std::string_view& propName, Reader& reader) {
	StartPropertyList(return Entity::ReadProperty(propName, reader));

	MatchProperty("Particle", {
		m_Particle = g_PresetMan.ReadPresetReferenceOfType<MovableObject>(reader, propName);
	});
	MatchProperty("ParticleCount", { reader >> m_ParticleCount; });
	MatchProperty("FireVelocity", { reader >> m_FireVel; });
	MatchProperty("InheritsFirerVelocity", { reader >> m_InheritsFirerVelocity; });
	MatchProperty("Separation", { reader >> m_Separation; });
	MatchProperty("LifeVariation", { reader >> m_LifeVariation; });
	MatchProperty("Shell", {
		m_Shell = g_PresetMan.ReadPresetReferenceOfType<MovableObject>(reader, propName);
	});
	MatchProperty("ShellVelocity", { reader >> m_ShellVel; });
	MatchProperty("FireSound", { reader >> m_FireSound; });
	MatchProperty("AILifeTime", { reader >> m_AILifeTime; });
	MatchProperty("AIFireVel", { reader >> m_AIFireVel; });
	MatchProperty("AIPenetration", { reader >> m_AIPenetration; });

	EndPropertyList;
}

int Round::Save(Writer& writer) const {
	Entity::Save(writer);

	writer.NewPresetReferenceProperty("Particle", m_Particle, static_cast<const MovableObject*>(nullptr));
	writer.NewDistinctProperty("ParticleCount", m_ParticleCount, 0);
	writer.NewDistinctProperty("FireVelocity", m_FireVel, 0.0F);
	writer.NewDistinctProperty("InheritsFirerVelocity", m_InheritsFirerVelocity, false);
	writer.NewDistinctProperty("Separation", m_Separation, 0.0F);
	writer.NewDistinctProperty("LifeVariation", m_LifeVariation, 0.0F);
	writer.NewPresetReferenceProperty("Shell", m_Shell, static_cast<const MovableObject*>(nullptr));
	writer.NewDistinctProperty("ShellVelocity", m_ShellVel, 0.0F);
	writer.NewPropertyWithValue("FireSound", m_FireSound);
	writer.NewDistinctProperty("AILifeTime", m_AILifeTime, 0UL);
	writer.NewDistinctProperty("AIFireVel", m_AIFireVel, -1);
	writer.NewDistinctProperty("AIPenetration", m_AIPenetration, -1);

	return 0;
}

int Round::Write(Writer& writer, const Entity& entityReference, HashingData& hashData) const {
	Entity::Write(writer, entityReference, hashData);

	const Round& reference = static_cast<const Round&>(entityReference);

	writer.NewPresetReferenceProperty("Particle", m_Particle, reference.m_Particle);
	writer.NewDistinctProperty("ParticleCount", m_ParticleCount, reference.m_ParticleCount);
	writer.NewDistinctProperty("FireVelocity", m_FireVel, reference.m_FireVel);
	writer.NewDistinctProperty("InheritsFirerVelocity", m_InheritsFirerVelocity, reference.m_InheritsFirerVelocity);
	writer.NewDistinctProperty("Separation", m_Separation, reference.m_Separation);
	writer.NewDistinctProperty("LifeVariation", m_LifeVariation, reference.m_LifeVariation);
	writer.NewPresetReferenceProperty("Shell", m_Shell, reference.m_Shell);
	writer.NewDistinctProperty("ShellVelocity", m_ShellVel, reference.m_ShellVel);
	writer.NewDistinctHashedProperty("FireSound", m_FireSound, hashData);
	writer.NewDistinctProperty("AILifeTime", m_AILifeTime, reference.m_AILifeTime);
	writer.NewDistinctProperty("AIFireVel", m_AIFireVel, reference.m_AIFireVel);
	writer.NewDistinctProperty("AIPenetration", m_AIPenetration, reference.m_AIPenetration);

	return 0;
}

HashingData Round::Hash() const {
	HashingData hashData(Entity::Hash());

	hashData.AddPresetReference(m_Particle);
	hashData.Add(m_ParticleCount);
	hashData.Add(m_FireVel);
	hashData.Add(m_InheritsFirerVelocity);
	hashData.Add(m_Separation);
	hashData.Add(m_LifeVariation);
	hashData.AddPresetReference(m_Shell);
	hashData.Add(m_ShellVel);

	hashData.AddHashed(m_FireSound);

	hashData.Add(m_AILifeTime);
	hashData.Add(m_AIFireVel);
	hashData.Add(m_AIPenetration);

	return hashData;
}

template <typename WriterType> int Round::WriteTo(WriterType& writer) const {
	Entity::WriteTo(writer);

	writer.NewPresetReferenceProperty("Particle", &Round::m_Particle);
	writer.NewDistinctProperty("ParticleCount", &Round::m_ParticleCount);
	writer.NewDistinctProperty("FireVelocity", &Round::m_FireVel);
	writer.NewDistinctProperty("InheritsFirerVelocity", &Round::m_InheritsFirerVelocity);
	writer.NewDistinctProperty("Separation", &Round::m_Separation);
	writer.NewDistinctProperty("LifeVariation", &Round::m_LifeVariation);
	writer.NewPresetReferenceProperty("Shell", &Round::m_Shell);
	writer.NewDistinctProperty("ShellVelocity", &Round::m_ShellVel);
	writer.NewDistinctProperty("FireSound", &Round::m_FireSound);
	writer.NewDistinctProperty("AILifeTime", &Round::m_AILifeTime);
	writer.NewDistinctProperty("AIFireVel", &Round::m_AIFireVel);
	writer.NewDistinctProperty("AIPenetration", &Round::m_AIPenetration);

	return 0;
}
template int Round::WriteTo(IniWriter& writer) const;
template int Round::WriteTo(HashWriter& writer) const;

MovableObject* Round::PopNextParticle() {
	MovableObject* tempParticle = (m_ParticleCount > 0) ? dynamic_cast<MovableObject*>(m_Particle->Clone()) : 0;
	m_ParticleCount--;
	return tempParticle;
}
