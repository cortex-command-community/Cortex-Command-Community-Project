#include "Emission.h"
#include "PresetMan.h"

using namespace RTE;

// const string Emission::m_sClassName = "Emission";

ConcreteClassInfo(Emission, Entity, 100);

Emission::Emission() {
	Clear();
}

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
	m_InheritsVel = 0;
	m_StartTimer.SetSimTimeLimitMS(0);
	m_StartTimer.Reset();
	m_StopTimer.SetSimTimeLimitMS(1000000);
	m_StopTimer.Reset();
	m_Offset.Reset();
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
	m_StartTimer = reference.m_StartTimer;
	m_StopTimer = reference.m_StopTimer;
	m_Offset = reference.m_Offset;

	return 0;
}

void Emission::Reset() {
	Clear();
}

const MovableObject* Emission::GetEmissionParticlePreset() {
	return m_pEmission;
}

float Emission::GetRate() const {
	return m_PPM;
}

void Emission::SetRate(float newPPM) {
	m_PPM = newPPM;
}

int Emission::GetBurstSize() const {
	return m_BurstSize;
}

void Emission::SetBurstSize(int newSize) {
	m_BurstSize = newSize;
}

float Emission::GetSpread() const {
	return m_Spread;
}

void Emission::SetSpread(float newSpread) {
	m_Spread = newSpread;
}

float Emission::GetMinVelocity() const {
	return m_MinVelocity;
}

void Emission::SetMinVelocity(float newVel) {
	m_MinVelocity = newVel;
}

float Emission::GetMaxVelocity() const {
	return m_MaxVelocity;
}

void Emission::SetMaxVelocity(float newVel) {
	m_MaxVelocity = newVel;
}

float Emission::GetLifeVariation() const {
	return m_LifeVariation;
}

void Emission::SetLifeVariation(float newVariation) {
	m_LifeVariation = newVariation;
}

bool Emission::PushesEmitter() const {
	return m_PushesEmitter;
}

void Emission::SetPushesEmitter(bool newValue) {
	m_PushesEmitter = newValue;
}

bool Emission::IsEmissionTime() {
	return m_StartTimer.IsPastSimTimeLimit() && !m_StopTimer.IsPastSimTimeLimit();
}

void Emission::ResetEmissionTimers() {
	m_StartTimer.Reset();
	m_StopTimer.Reset();
}

float Emission::InheritsVelocity() {
	return m_InheritsVel;
}

Vector Emission::GetOffset() const {
	return m_Offset;
}

void Emission::SetOffset(Vector offset) {
	m_Offset = offset;
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
	MatchProperty("InheritsVel",
	              {
		              reader >> m_InheritsVel;
		              Clamp(m_InheritsVel, 1, 0);
	              });
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
	writer.NewProperty("Offset");
	writer << m_Offset;
	writer.NewProperty("StartTimeMS");
	writer << m_StartTimer.GetSimTimeLimitMS();
	writer.NewProperty("StopTimeMS");
	writer << m_StopTimer.GetSimTimeLimitMS();

	return 0;
}
