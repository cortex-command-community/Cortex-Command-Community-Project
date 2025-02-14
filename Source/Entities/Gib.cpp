#include "Gib.h"
#include "PresetMan.h"
#include "MovableObject.h"

using namespace RTE;

const std::string Gib::c_ClassName = "Gib";

Gib::Gib() {
	Clear();
}

Gib::~Gib() {
	Destroy();
}

void Gib::Clear() {
	m_GibParticle = nullptr;
	m_Offset.Reset();
	m_Count = 1;
	m_Spread = 0.1F;
	m_MinVelocity = 0;
	m_MaxVelocity = 0;
	m_LifeVariation = 0.1F;
	m_InheritsVel = 1.0F;
	m_InheritsAngularVel = 1.0F;
	m_IgnoresTeamHits = false;
	m_SpreadMode = SpreadMode::SpreadRandom;
}

int Gib::Create(const Gib& reference) {
	m_GibParticle = reference.m_GibParticle;
	m_Offset = reference.m_Offset;
	m_Count = reference.m_Count;
	m_Spread = reference.m_Spread;
	m_MinVelocity = reference.m_MinVelocity;
	m_MaxVelocity = reference.m_MaxVelocity;
	m_LifeVariation = reference.m_LifeVariation;
	m_InheritsVel = reference.m_InheritsVel;
	m_InheritsAngularVel = reference.m_InheritsAngularVel;
	m_IgnoresTeamHits = reference.m_IgnoresTeamHits;
	m_SpreadMode = reference.m_SpreadMode;

	return 0;
}

int Gib::ReadProperty(const std::string_view& propName, Reader& reader) {
	StartPropertyList(return Serializable::ReadProperty(propName, reader));

	MatchProperty("GibParticle", {
		m_GibParticle = dynamic_cast<const MovableObject*>(g_PresetMan.GetEntityPreset(reader));
		RTEAssert(m_GibParticle, "Stream suggests allocating an unallocable type in Gib::Create!");
	});
	MatchProperty("Offset", { reader >> m_Offset; });
	MatchProperty("Count", { reader >> m_Count; });
	MatchProperty("Spread", { reader >> m_Spread; });
	MatchProperty("MinVelocity", { reader >> m_MinVelocity; });
	MatchProperty("MaxVelocity", { reader >> m_MaxVelocity; });
	MatchProperty("LifeVariation", { reader >> m_LifeVariation; });
	MatchProperty("InheritsVel", { reader >> m_InheritsVel; });
	MatchProperty("InheritsAngularVel", { reader >> m_InheritsAngularVel; });
	MatchProperty("IgnoresTeamHits", { reader >> m_IgnoresTeamHits; });
	MatchProperty("SpreadMode", { m_SpreadMode = static_cast<SpreadMode>(std::stoi(reader.ReadPropValue())); });

	EndPropertyList;
}

int Gib::Save(Writer& writer) const {
	Serializable::Save(writer);

	writer.NewPropertyWithValue("GibParticle", m_GibParticle->GetEntityCharacteristic());
	writer.NewPropertyWithValue("Offset", m_Offset);
	writer.NewPropertyWithValue("Count", m_Count);
	writer.NewPropertyWithValue("Spread", m_Spread);
	writer.NewPropertyWithValue("MinVelocity", GetMinVelocity());
	writer.NewPropertyWithValue("MaxVelocity", GetMaxVelocity());
	writer.NewPropertyWithValue("LifeVariation", m_LifeVariation);
	writer.NewPropertyWithValue("InheritsVel", m_InheritsVel);
	writer.NewPropertyWithValue("InheritsAngularVel", m_InheritsAngularVel);

	return 0;
}

uint64_t Gib::Hash() const {
	uint64_t hash = RTE::Hash(m_GibParticle->GetEntityCharacteristic());
	hash ^= m_Offset.Hash() << 1;
	hash ^= std::hash<int>{}(m_Count) << 2;
	hash ^= std::hash<float>{}(m_Spread) << 3;
	hash ^= std::hash<float>{}(m_MinVelocity) << 4;
	hash ^= std::hash<float>{}(m_MaxVelocity) << 5;
	hash ^= std::hash<float>{}(m_LifeVariation) << 6;
	hash ^= std::hash<float>{}(m_InheritsVel) << 7;
	hash ^= std::hash<float>{}(m_InheritsAngularVel) << 8;
	hash ^= std::hash<bool>{}(m_IgnoresTeamHits) << 9;
	hash ^= std::hash<int>{}(m_SpreadMode) << 10;
	return hash;
}
