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
		m_Particle = dynamic_cast<const MovableObject*>(g_PresetMan.GetEntityPreset(reader));
		RTEAssert(m_Particle, "Stream suggests allocating an unallocable type in Round::Create!");
	});
	MatchProperty("ParticleCount", { reader >> m_ParticleCount; });
	MatchProperty("FireVelocity", { reader >> m_FireVel; });
	MatchProperty("InheritsFirerVelocity", { reader >> m_InheritsFirerVelocity; });
	MatchProperty("Separation", { reader >> m_Separation; });
	MatchProperty("LifeVariation", { reader >> m_LifeVariation; });
	MatchProperty("Shell", { m_Shell = dynamic_cast<const MovableObject*>(g_PresetMan.GetEntityPreset(reader)); });
	MatchProperty("ShellVelocity", { reader >> m_ShellVel; });
	MatchProperty("FireSound", { reader >> m_FireSound; });
	MatchProperty("AILifeTime", { reader >> m_AILifeTime; });
	MatchProperty("AIFireVel", { reader >> m_AIFireVel; });
	MatchProperty("AIPenetration", { reader >> m_AIPenetration; });

	EndPropertyList;
}

int Round::Save(Writer& writer) const {
	Entity::Save(writer);

	writer.NewProperty("Particle");
	writer << m_Particle;
	writer.NewProperty("ParticleCount");
	writer << m_ParticleCount;
	writer.NewProperty("FireVelocity");
	writer << m_FireVel;
	writer.NewPropertyWithValue("InheritsFirerVelocity", m_InheritsFirerVelocity);
	writer.NewProperty("Separation");
	writer << m_Separation;
	writer.NewProperty("LifeVariation");
	writer << m_LifeVariation;
	writer.NewProperty("Shell");
	writer << m_Shell;
	writer.NewProperty("ShellVelocity");
	writer << m_ShellVel;
	writer.NewProperty("FireSound");
	writer << m_FireSound;
	writer.NewProperty("AILifeTime");
	writer << m_AILifeTime;
	writer.NewProperty("AIFireVel");
	writer << m_AIFireVel;
	writer.NewProperty("AIPenetration");
	writer << m_AIPenetration;

	return 0;
}

uint64_t Round::Hash() const {
	uint64_t hash = (m_Particle ? RTE::Hash(m_Particle->GetEntityCharacteristic()) : 0);
	hash ^= std::hash<int>{}(m_ParticleCount) << 1;
	hash ^= std::hash<float>{}(m_FireVel) << 2;
	hash ^= std::hash<bool>{}(m_InheritsFirerVelocity) << 3;
	hash ^= std::hash<float>{}(m_Separation) << 4;
	hash ^= std::hash<float>{}(m_LifeVariation) << 5;
	hash ^= (m_Shell ? RTE::Hash(m_Shell->GetEntityCharacteristic()) : 0) << 6;
	hash ^= std::hash<float>{}(m_ShellVel) << 7;
	hash ^= m_FireSound.Hash() << 8;
	hash ^= std::hash<unsigned long>{}(m_AILifeTime) << 9;
	hash ^= std::hash<int>{}(m_AIFireVel) << 10;
	hash ^= std::hash<int>{}(m_AIPenetration) << 11;
	return Entity::Hash() ^ (hash << 1);
}

MovableObject* Round::PopNextParticle() {
	MovableObject* tempParticle = (m_ParticleCount > 0) ? dynamic_cast<MovableObject*>(m_Particle->Clone()) : 0;
	m_ParticleCount--;
	return tempParticle;
}
