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
		const Entity* entityReference = g_PresetMan.GetEntityPresetFromCharacteristic(reader);
		const MovableObject* reference = dynamic_cast<const MovableObject*>(entityReference);
		if (entityReference == nullptr || reference != nullptr) {
			m_GibParticle = reference;
		} else {
			reader.ReportError("Tried to point GibParticle to a non-MovableObject type!");
		}
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
		writer.NewPropertyWithValue("GibParticle", m_GibParticle->GetEntityCharacteristic());
	if (!m_Offset.IsZero())
		writer.NewPropertyWithValue("Offset", m_Offset);
	if (m_Count != 1)
		writer.NewPropertyWithValue("Count", m_Count);
	if (m_Spread != 0.1F)
		writer.NewPropertyWithValue("Spread", m_Spread);
	if (GetMinVelocity() != 0.0F)
		writer.NewPropertyWithValue("MinVelocity", GetMinVelocity());
	if (GetMaxVelocity() != 0.0F)
		writer.NewPropertyWithValue("MaxVelocity", GetMaxVelocity());
	if (m_LifeVariation != 0.1F)
		writer.NewPropertyWithValue("LifeVariation", m_LifeVariation);
	if (m_InheritsVel != 1.0F)
		writer.NewPropertyWithValue("InheritsVel", m_InheritsVel);
	if (m_InheritsAngularVel != 1.0F)
		writer.NewPropertyWithValue("InheritsAngularVel", m_InheritsAngularVel);
	if (m_IgnoresTeamHits != false)
		writer.NewPropertyWithValue("IgnoresTeamHits", m_IgnoresTeamHits);
	if (m_SpreadMode != SpreadMode::SpreadRandom)
		writer.NewPropertyWithValue("SpreadMode", m_SpreadMode);

	return 0;
}

HashingData Gib::Hash() const {
	HashingData hashData(std::move(Serializable::Hash()));
	uint64_t& hash = hashData.m_Hash;

	hash ^= (m_GibParticle ? RTE::Hash(m_GibParticle->GetEntityCharacteristic()) : 0) << 0;
	hash ^= m_Offset.Hash().m_Hash << 1;
	hash ^= static_cast<uint64_t>(m_Count) << 2;
	hash ^= static_cast<uint64_t>(m_Spread) << 3;
	hash ^= static_cast<uint64_t>(m_MinVelocity) << 4;
	hash ^= static_cast<uint64_t>(m_MaxVelocity) << 5;
	hash ^= static_cast<uint64_t>(m_LifeVariation) << 6;
	hash ^= static_cast<uint64_t>(m_InheritsVel) << 7;
	hash ^= static_cast<uint64_t>(m_InheritsAngularVel) << 8;
	hash ^= static_cast<uint64_t>(m_IgnoresTeamHits) << 9;
	hash ^= static_cast<uint64_t>(m_SpreadMode) << 10;

	return hashData;
}
