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
	StartPropertyList(return Serializable::ReadProperty(propName, reader));

	MatchProperty("EmittedParticle",
	              {
		              m_pEmission = dynamic_cast<const MovableObject*>(g_PresetMan.GetEntityPreset(reader));
		              RTEAssert(m_pEmission, "Stream suggests allocating an unallocatable type in AEmitter::Emission::Create!");
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
	MatchProperty("StartTimeMS",
	              {
		              double startTime;
		              reader >> startTime;
		              m_StartTimer.SetSimTimeLimitMS(startTime);
	              });
	MatchProperty("StopTimeMS",
	              {
		              double stopTime;
		              reader >> stopTime;
		              m_StopTimer.SetSimTimeLimitMS(stopTime);
	              });

	EndPropertyList;
}

int Emission::Save(Writer& writer) const {
	Serializable::Save(writer);

	writer.NewProperty("EmittedParticle");
	writer << m_pEmission;
	writer.NewProperty("ParticlesPerMinute");
	writer << m_PPM;
	writer.NewProperty("BurstSize");
	writer << m_BurstSize;
	writer.NewProperty("Spread");
	writer << m_Spread;
	writer.NewProperty("MinVelocity");
	writer << m_MinVelocity;
	writer.NewProperty("MaxVelocity");
	writer << m_MaxVelocity;
	writer.NewProperty("LifeVariation");
	writer << m_LifeVariation;
	writer.NewProperty("PushesEmitter");
	writer << m_PushesEmitter;
	writer.NewProperty("InheritsVel");
	writer << m_InheritsVel;
	writer.NewProperty("InheritsAngularVel");
	writer << m_InheritsAngularVel;
	writer.NewProperty("Offset");
	writer << m_Offset;
	writer.NewProperty("StartTimeMS");
	writer << m_StartTimer.GetSimTimeLimitMS();
	writer.NewProperty("StopTimeMS");
	writer << m_StopTimer.GetSimTimeLimitMS();
	writer.NewProperty("ParticleCount");
	writer << m_ParticleCount;

	return 0;
}

uint64_t Emission::Hash() const {
	uint64_t hash = RTE::Hash(m_pEmission->GetEntityCharacteristic());
	hash ^= std::hash<float>{}(m_PPM) << 1;
	hash ^= std::hash<int>{}(m_BurstSize) << 2;
	hash ^= std::hash<float>{}(m_Spread) << 3;
	hash ^= std::hash<float>{}(m_MinVelocity) << 4;
	hash ^= std::hash<float>{}(m_MaxVelocity) << 5;
	hash ^= std::hash<float>{}(m_LifeVariation) << 6;
	hash ^= std::hash<bool>{}(m_PushesEmitter) << 7;
	hash ^= std::hash<float>{}(m_InheritsVel) << 8;
	hash ^= std::hash<float>{}(m_InheritsAngularVel) << 9;
	hash ^= m_Offset.Hash() << 10;
	hash ^= std::hash<double>{}(m_StartTimer.GetSimTimeLimitMS()) << 11;
	hash ^= std::hash<double>{}(m_StopTimer.GetSimTimeLimitMS()) << 12;
	hash ^= std::hash<int>{}(m_ParticleCount) << 13;
	return Entity::Hash() ^ (hash << 1);
}
