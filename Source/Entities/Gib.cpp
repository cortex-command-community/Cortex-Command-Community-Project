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

template <typename WriterType> int Gib::WriteTo(WriterType& writer) const {
	Serializable::WriteTo(writer);

	if (m_GibParticle != nullptr)
		writer.NewPropertyWithPresetReference("GibParticle", m_GibParticle);
	if (!m_Offset.IsZero())
		writer.NewPropertyWithValue("Offset", m_Offset);
	if (m_Count != 1) {
		writer.NewPropertyWithValue("Count", m_Count);
	}
	if (m_Spread != 0.1F) {
		writer.NewPropertyWithValue("Spread", m_Spread);
	}
	if (GetMinVelocity() != 0.0F) {
		writer.NewPropertyWithValue("MinVelocity", GetMinVelocity());
	}
	if (GetMaxVelocity() != 0.0F) {
		writer.NewPropertyWithValue("MaxVelocity", GetMaxVelocity());
	}
	if (m_LifeVariation != 0.1F) {
		writer.NewPropertyWithValue("LifeVariation", m_LifeVariation);
	}
	if (m_InheritsVel != 1.0F) {
		writer.NewPropertyWithValue("InheritsVel", m_InheritsVel);
	}
	if (m_InheritsAngularVel != 1.0F) {
		writer.NewPropertyWithValue("InheritsAngularVel", m_InheritsAngularVel);
	}
	if (m_IgnoresTeamHits != false) {
		writer.NewPropertyWithValue("IgnoresTeamHits", m_IgnoresTeamHits);
	}
	if (m_SpreadMode != SpreadMode::SpreadRandom) {
		writer.NewPropertyWithValue("SpreadMode", m_SpreadMode);
	}

	return 0;
}
template int Gib::WriteTo(IniWriter& writer) const;
template int Gib::WriteTo(HashWriter& writer) const;
