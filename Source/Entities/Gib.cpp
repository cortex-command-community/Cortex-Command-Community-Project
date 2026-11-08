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
		m_GibParticle = g_PresetMan.ReadPresetReferenceOfType<MovableObject>(reader, propName);
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

	if (m_GibParticle != nullptr)
		writer.NewPropertyWithPresetReference("GibParticle", m_GibParticle);
	if (!m_Offset.IsZero())
		writer.NewPropertyWithValue("Offset", m_Offset);
	writer.NewDistinctProperty("Count", m_Count, 1);
	writer.NewDistinctProperty("Spread", m_Spread, 0.1F);
	writer.NewDistinctProperty("MinVelocity", GetMinVelocity(), 0.0F);
	writer.NewDistinctProperty("MaxVelocity", GetMaxVelocity(), 0.0F);
	writer.NewDistinctProperty("LifeVariation", m_LifeVariation, 0.1F);
	writer.NewDistinctProperty("InheritsVel", m_InheritsVel, 1.0F);
	writer.NewDistinctProperty("InheritsAngularVel", m_InheritsAngularVel, 1.0F);
	writer.NewDistinctProperty("IgnoresTeamHits", m_IgnoresTeamHits, false);
	writer.NewDistinctProperty("SpreadMode", m_SpreadMode, SpreadMode::SpreadRandom);

	return 0;
}

HashingData Gib::Hash() const {
	HashingData hashData(Serializable::Hash());
	uint64_t& hash = hashData.m_Hash;

	AddToHash(hash, (m_GibParticle ? RTE::Hash(m_GibParticle->GetEntityCharacteristic()) : 0));
	AddToHash(hash, m_Offset.Hash().m_Hash);
	AddToHash(hash, m_Count);
	AddToHash(hash, m_Spread);
	AddToHash(hash, m_MinVelocity);
	AddToHash(hash, m_MaxVelocity);
	AddToHash(hash, m_LifeVariation);
	AddToHash(hash, m_InheritsVel);
	AddToHash(hash, m_InheritsAngularVel);
	AddToHash(hash, m_IgnoresTeamHits);
	AddToHash(hash, m_SpreadMode);

	return hashData;
}
