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
		m_pEmission = g_PresetMan.ReadPresetReferenceOfType<MovableObject>(reader, propName);
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

	if (m_pEmission != nullptr) writer.NewPropertyWithPresetReference("EmittedParticle", m_pEmission);
	writer.NewDistinctProperty("ParticlesPerMinute", m_PPM, 0.0F);
	writer.NewDistinctProperty("BurstSize", m_BurstSize, 0);
	writer.NewDistinctProperty("Spread", m_Spread, 0.0F);
	writer.NewDistinctProperty("MinVelocity", m_MinVelocity, 0.0F);
	writer.NewDistinctProperty("MaxVelocity", m_MaxVelocity, 0.0F);
	writer.NewDistinctProperty("LifeVariation", m_LifeVariation, 0.1F);
	writer.NewDistinctProperty("PushesEmitter", m_PushesEmitter, true);
	writer.NewDistinctProperty("InheritsVel", m_InheritsVel, 0.0F);
	writer.NewDistinctProperty("InheritsAngularVel", m_InheritsAngularVel, 0.0F);
	if (!m_Offset.IsZero()) writer.NewPropertyWithValue("Offset", m_Offset);
	writer.NewDistinctProperty("StartTimeMS", m_StartTimer.GetSimTimeLimitMS(), 0.0);
	writer.NewDistinctProperty("StopTimeMS", m_StopTimer.GetSimTimeLimitMS(), 1000000.0);
	writer.NewDistinctProperty("ParticleCount", m_ParticleCount, 1);

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

	hashData.AddPresetReference(m_pEmission);
	hashData.Add(m_PPM);
	hashData.Add(m_BurstSize);
	hashData.Add(m_Spread);
	hashData.Add(m_MinVelocity);
	hashData.Add(m_MaxVelocity);
	hashData.Add(m_LifeVariation);
	hashData.Add(m_PushesEmitter);
	hashData.Add(m_InheritsVel);
	hashData.Add(m_InheritsAngularVel);
	hashData.Add(m_Offset);
	hashData.Add(m_StartTimer.GetSimTimeLimitMS());
	hashData.Add(m_StopTimer.GetSimTimeLimitMS());
	hashData.Add(m_ParticleCount);

	return hashData;
}
