#include "Emission.h"
#include "PresetMan.h"

using namespace RTE;

// const string Emission::m_sClassName = "Emission";

ConcreteClassInfo(Emission, Entity, 100);

void Emission::Clear() {
	m_pEmission = 0;
	m_PPM = 0;
	m_BurstSize = 0;
	m_Accumulator = 0;
	m_Spread = 0;
	m_MinVelocity = 0;
	m_MaxVelocity = 0;
	m_LifeVariation = 0.1;
	m_PushesEmitter = true;
	m_InheritsVel = 0.0F;
	m_InheritsAngularVel = 0.0F;
	m_StartTimer.SetSimTimeLimitMS(0);
	m_StartTimer.Reset();
	m_StopTimer.SetSimTimeLimitMS(1000000);
	m_StopTimer.Reset();
	m_Offset.Reset();
	m_ParticleCount = 1;
}

/*
int AEmitter::Emission::Create()
{
if (Serializable::Create() < 0)
return -1;

return 0;
}
*/

int Emission::Create(const Emission& reference) {
	m_pEmission = reference.m_pEmission;
	m_PPM = reference.m_PPM;
	m_BurstSize = reference.m_BurstSize;
	m_Accumulator = reference.m_Accumulator;
	m_Spread = reference.m_Spread;
	m_MinVelocity = reference.m_MinVelocity;
	m_MaxVelocity = reference.m_MaxVelocity;
	m_LifeVariation = reference.m_LifeVariation;
	m_PushesEmitter = reference.m_PushesEmitter;
	m_InheritsVel = reference.m_InheritsVel;
	m_InheritsAngularVel = reference.m_InheritsAngularVel;
	m_StartTimer = reference.m_StartTimer;
	m_StopTimer = reference.m_StopTimer;
	m_Offset = reference.m_Offset;
	m_ParticleCount = reference.m_ParticleCount;

	return 0;
}

int Emission::ReadProperty(const std::string_view& propName, Reader& reader) {
	StartPropertyList(return Entity::ReadProperty(propName, reader));

	MatchProperty("EmittedParticle", {
		const Entity* entityReference = g_PresetMan.GetEntityPresetFromCharacteristic(reader);
		const MovableObject* reference = dynamic_cast<const MovableObject*>(entityReference);
		if (entityReference == nullptr || reference != nullptr) {
			m_pEmission = reference;
		} else {
			reader.ReportError("Tried to point EmittedParticle to a non-MovableObject type!");
		}
	});
	MatchProperty("ParticlesPerMinute", { reader >> m_PPM; });
	MatchProperty("BurstSize", { reader >> m_BurstSize; });
	MatchProperty("Spread", { reader >> m_Spread; });
	MatchProperty("MinVelocity", { reader >> m_MinVelocity; });
	MatchProperty("MaxVelocity", { reader >> m_MaxVelocity; });
	MatchProperty("LifeVariation", { reader >> m_LifeVariation; });
	MatchProperty("PushesEmitter", { reader >> m_PushesEmitter; });
	MatchProperty("Offset", { reader >> m_Offset; });
	MatchProperty("ParticleCount", { reader >> m_ParticleCount; });
	MatchProperty("InheritsVel", { reader >> m_InheritsVel; });
	MatchProperty("InheritsAngularVel", { reader >> m_InheritsAngularVel; });
	MatchProperty("StartTimeMS", {
		double startTime;
		reader >> startTime;
		m_StartTimer.SetSimTimeLimitMS(startTime);
	});
	MatchProperty("StopTimeMS", {
		double stopTime;
		reader >> stopTime;
		m_StopTimer.SetSimTimeLimitMS(stopTime);
	});

	EndPropertyList;
}

int Emission::Save(Writer& writer) const {
	Entity::Save(writer);

	if (m_pEmission != nullptr) writer.NewPropertyWithValue("EmittedParticle", m_pEmission->GetEntityCharacteristic());
	if (m_PPM != 0.0F) writer.NewPropertyWithValue("ParticlesPerMinute", m_PPM);
	if (m_BurstSize != 0) writer.NewPropertyWithValue("BurstSize", m_BurstSize);
	if (m_Spread != 0.0F) writer.NewPropertyWithValue("Spread", m_Spread);
	if (m_MinVelocity != 0.0F) writer.NewPropertyWithValue("MinVelocity", m_MinVelocity);
	if (m_MaxVelocity != 0.0F) writer.NewPropertyWithValue("MaxVelocity", m_MaxVelocity);
	if (m_LifeVariation != 0.1F) writer.NewPropertyWithValue("LifeVariation", m_LifeVariation);
	if (m_PushesEmitter != true) writer.NewPropertyWithValue("PushesEmitter", m_PushesEmitter);
	if (m_InheritsVel != 0.0F) writer.NewPropertyWithValue("InheritsVel", m_InheritsVel);
	if (m_InheritsAngularVel != 0.0F) writer.NewPropertyWithValue("InheritsAngularVel", m_InheritsAngularVel);
	if (!m_Offset.IsZero()) writer.NewPropertyWithValue("Offset", m_Offset);
	if (m_StartTimer.GetSimTimeLimitMS() != 0.0) writer.NewPropertyWithValue("StartTimeMS", m_StartTimer.GetSimTimeLimitMS());
	if (m_StopTimer.GetSimTimeLimitMS() != 0.0) writer.NewPropertyWithValue("StopTimeMS", m_StopTimer.GetSimTimeLimitMS());
	if (m_ParticleCount != 1) writer.NewPropertyWithValue("ParticleCount", m_ParticleCount);

	return 0;
}

int Emission::Write(Writer& writer, const Entity& entityReference, HashingData& hashData) const {
	Entity::Write(writer, entityReference, hashData);

	const Emission& reference = static_cast<const Emission&>(entityReference);

	writer.NewPresetReferenceProperty("EmittedParticle", m_pEmission, reference.m_pEmission);
	writer.NewDistinctProperty("ParticlesPerMinute", m_PPM, reference.m_PPM);
	writer.NewDistinctProperty("BurstSize", m_BurstSize, reference.m_BurstSize);
	writer.NewDistinctProperty("Spread", m_Spread, reference.m_Spread);
	writer.NewDistinctProperty("MinVelocity", m_MinVelocity, reference.m_MinVelocity);
	writer.NewDistinctProperty("MaxVelocity", m_MaxVelocity, reference.m_MaxVelocity);
	writer.NewDistinctProperty("LifeVariation", m_LifeVariation, reference.m_LifeVariation);
	writer.NewDistinctProperty("PushesEmitter", m_PushesEmitter, reference.m_PushesEmitter);
	writer.NewDistinctProperty("InheritsVel", m_InheritsVel, reference.m_InheritsVel);
	writer.NewDistinctProperty("InheritsAngularVel", m_InheritsAngularVel, reference.m_InheritsAngularVel);
	writer.NewDistinctProperty("Offset", m_Offset, reference.m_Offset);
	writer.NewDistinctProperty("StartTimeMS", m_StartTimer.GetSimTimeLimitMS(), reference.m_StartTimer.GetSimTimeLimitMS());
	writer.NewDistinctProperty("StopTimeMS", m_StopTimer.GetSimTimeLimitMS(), reference.m_StopTimer.GetSimTimeLimitMS());
	writer.NewDistinctProperty("ParticleCount", m_ParticleCount, reference.m_ParticleCount);

	return 0;
}

HashingData Emission::Hash() const {
	HashingData hashData(Entity::Hash());
	uint64_t& hash = hashData.m_Hash;

	hash ^= (m_pEmission ? RTE::Hash(m_pEmission->GetEntityCharacteristic()) : 0) << 0;
	hash ^= static_cast<uint64_t>(m_PPM) << 1;
	hash ^= static_cast<uint64_t>(m_BurstSize) << 2;
	hash ^= static_cast<uint64_t>(m_Spread) << 3;
	hash ^= static_cast<uint64_t>(m_MinVelocity) << 4;
	hash ^= static_cast<uint64_t>(m_MaxVelocity) << 5;
	hash ^= static_cast<uint64_t>(m_LifeVariation) << 6;
	hash ^= static_cast<uint64_t>(m_PushesEmitter) << 7;
	hash ^= static_cast<uint64_t>(m_InheritsVel) << 8;
	hash ^= static_cast<uint64_t>(m_InheritsAngularVel) << 9;
	hash ^= m_Offset.Hash().m_Hash << 10;
	hash ^= static_cast<uint64_t>(m_StartTimer.GetSimTimeLimitMS()) << 11;
	hash ^= static_cast<uint64_t>(m_StopTimer.GetSimTimeLimitMS()) << 12;
	hash ^= static_cast<uint64_t>(m_ParticleCount) << 13;

	return hashData;
}
