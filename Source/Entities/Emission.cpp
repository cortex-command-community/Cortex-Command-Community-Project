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

template <typename WriterType> int Emission::WriteTo(WriterType& writer) const {
	Entity::WriteTo(writer);

	writer.NewPresetReferenceProperty("EmittedParticle", &Emission::m_pEmission);
	writer.NewDistinctProperty("ParticlesPerMinute", &Emission::m_PPM);
	writer.NewDistinctProperty("BurstSize", &Emission::m_BurstSize);
	writer.NewDistinctProperty("Spread", &Emission::m_Spread);
	writer.NewDistinctProperty("MinVelocity", &Emission::m_MinVelocity);
	writer.NewDistinctProperty("MaxVelocity", &Emission::m_MaxVelocity);
	writer.NewDistinctProperty("LifeVariation", &Emission::m_LifeVariation);
	writer.NewDistinctProperty("PushesEmitter", &Emission::m_PushesEmitter);
	writer.NewDistinctProperty("InheritsVel", &Emission::m_InheritsVel);
	writer.NewDistinctProperty("InheritsAngularVel", &Emission::m_InheritsAngularVel);
	writer.NewDistinctProperty("Offset", &Emission::m_Offset);
	writer.NewDistinctProperty("StartTimeMS", [](const Emission& object) { return object.m_StartTimer.GetSimTimeLimitMS(); });
	writer.NewDistinctProperty("StopTimeMS", [](const Emission& object) { return object.m_StopTimer.GetSimTimeLimitMS(); });
	writer.NewDistinctProperty("ParticleCount", &Emission::m_ParticleCount);

	return 0;
}
template int Emission::WriteTo(IniWriter& writer) const;
template int Emission::WriteTo(HashWriter& writer) const;
