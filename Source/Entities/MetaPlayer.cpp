#include "MetaPlayer.h"
#include "DataModule.h"
#include "PresetMan.h"
#include "ActivityMan.h"

using namespace RTE;

ConcreteClassInfo(MetaPlayer, Entity, 0);

MetaPlayer::MetaPlayer() {
	Clear();
}

MetaPlayer::MetaPlayer(const MetaPlayer& reference) {
	if (this != &reference) {
		Clear();
		Create(reference);
	}
}

int MetaPlayer::Create() {
	return Entity::Create();
}

int MetaPlayer::Create(const MetaPlayer& reference) {
	Entity::Create(reference);

	m_Name = reference.m_Name;
	m_Team = reference.m_Team;
	m_Human = reference.m_Human;
	m_InGamePlayer = reference.m_InGamePlayer;
	m_Aggressiveness = reference.m_Aggressiveness;
	m_GameOverRound = reference.m_GameOverRound;
	m_NativeTechModule = reference.m_NativeTechModule;
	m_NativeCostMult = reference.m_NativeCostMult;
	m_ForeignCostMult = reference.m_ForeignCostMult;
	m_BrainPool = reference.m_BrainPool;
	m_BrainsInTransit = reference.m_BrainsInTransit;
	m_Funds = reference.m_Funds;
	m_PhaseStartFunds = reference.m_PhaseStartFunds;
	m_OffensiveBudget = reference.m_OffensiveBudget;
	m_OffensiveTarget = reference.m_OffensiveTarget;

	return 0;
}

MetaPlayer::~MetaPlayer() {
	Destroy(true);
}

void MetaPlayer::Clear() {
	m_Name = "";
	m_Team = Activity::NoTeam;
	m_Human = true;
	m_InGamePlayer = Players::PlayerOne;
	m_Aggressiveness = 0.5F;
	m_GameOverRound = -1;

	// Everything is natively priced
	m_NativeTechModule = 0;
	m_NativeCostMult = 1.0F;
	m_ForeignCostMult = 4.0F;

	m_BrainPool = 0;
	m_BrainsInTransit = 0;
	m_Funds = 0;
	m_PhaseStartFunds = 0;
	m_OffensiveBudget = 0;
	m_OffensiveTarget = "";
}

int MetaPlayer::ReadProperty(const std::string_view& propName, Reader& reader) {
	StartPropertyList(return Entity::ReadProperty(propName, reader));

	MatchProperty("Name", { reader >> m_Name; });
	MatchProperty("Team", { reader >> m_Team; });
	MatchProperty("Human", { reader >> m_Human; });
	MatchProperty("InGamePlayer", { reader >> m_InGamePlayer; });
	MatchProperty("Aggressiveness", { reader >> m_Aggressiveness; });
	MatchProperty("GameOverRound", {
		reader >> m_GameOverRound;
		// Need to match the name to the index
	});
	MatchProperty("NativeTechModule", {
		m_NativeTechModule = g_PresetMan.GetModuleID(reader.ReadPropValue());
		// Default to no native tech if the one we're looking for couldn't be found
		if (m_NativeTechModule < 0) {
			m_NativeTechModule = 0;
		}
	});
	MatchProperty("NativeCostMultiplier", { reader >> m_NativeCostMult; });
	MatchProperty("ForeignCostMultiplier", { reader >> m_ForeignCostMult; });
	MatchProperty("BrainPool", { reader >> m_BrainPool; });
	MatchProperty("Funds", { reader >> m_Funds; });
	MatchProperty("OffensiveBudget", { reader >> m_OffensiveBudget; });
	MatchProperty("OffensiveTarget", { reader >> m_OffensiveTarget; });

	EndPropertyList;
}

int MetaPlayer::Save(Writer& writer) const {
	Entity::Save(writer);

	if (!m_Name.empty())
		writer.NewPropertyWithValue("Name", m_Name);

	writer.NewDistinctProperty("Team", m_Team, (int) Activity::NoTeam);
	writer.NewDistinctProperty("Human", m_Human, true);
	writer.NewDistinctProperty("InGamePlayer", m_InGamePlayer, (int) Players::PlayerOne);
	writer.NewDistinctProperty("Aggressiveness", m_Aggressiveness, 0.5F);
	writer.NewDistinctProperty("GameOverRound", m_GameOverRound, -1);

	if (m_NativeTechModule != 0)
		writer.NewPropertyWithValue("NativeTechModule", g_PresetMan.GetDataModule(m_NativeTechModule)->GetFileName());

	writer.NewDistinctProperty("NativeCostMultiplier", m_NativeCostMult, 1.0F);
	writer.NewDistinctProperty("ForeignCostMultiplier", m_ForeignCostMult, 4.0F);
	writer.NewDistinctProperty("BrainPool", m_BrainPool, 0);
	writer.NewDistinctProperty("Funds", m_Funds, 0.0F);
	writer.NewDistinctProperty("OffensiveBudget", m_OffensiveBudget, 0.0F);

	if (!m_OffensiveTarget.empty())
		writer.NewPropertyWithValue("OffensiveTarget", m_OffensiveTarget);

	return 0;
}

int MetaPlayer::Write(Writer& writer, const Entity& entityReference, HashingData& hashData) const {
	Entity::Write(writer, entityReference, hashData);

	const MetaPlayer& reference = static_cast<const MetaPlayer&>(entityReference);

	writer.NewDistinctProperty("Name", m_Name, reference.m_Name);
	writer.NewDistinctProperty("Team", m_Team, reference.m_Team);
	writer.NewDistinctProperty("Human", m_Human, reference.m_Human);
	writer.NewDistinctProperty("InGamePlayer", m_InGamePlayer, reference.m_InGamePlayer);
	writer.NewDistinctProperty("Aggressiveness", m_Aggressiveness, reference.m_Aggressiveness);
	writer.NewDistinctProperty("GameOverRound", m_GameOverRound, reference.m_GameOverRound);

	if (m_NativeTechModule != reference.m_NativeTechModule)
		writer.NewPropertyWithValue("NativeTechModule", g_PresetMan.GetDataModule(m_NativeTechModule)->GetFileName());

	writer.NewDistinctProperty("NativeCostMultiplier", m_NativeCostMult, reference.m_NativeCostMult);
	writer.NewDistinctProperty("ForeignCostMultiplier", m_ForeignCostMult, reference.m_ForeignCostMult);
	writer.NewDistinctProperty("BrainPool", m_BrainPool, reference.m_BrainPool);
	writer.NewDistinctProperty("Funds", m_Funds, reference.m_Funds);
	writer.NewDistinctProperty("OffensiveBudget", m_OffensiveBudget, reference.m_OffensiveBudget);

	if (m_OffensiveTarget != reference.m_OffensiveTarget)
		writer.NewPropertyWithValue("OffensiveTarget", m_OffensiveTarget.empty() ? "None" : m_OffensiveTarget);

	return 0;
}

HashingData MetaPlayer::Hash() const {
	HashingData hashData(Entity::Hash());
	uint64_t& hash = hashData.m_Hash;

	hash ^= RTE::Hash(m_Name) << 0;
	hash ^= static_cast<uint64_t>(m_Team) << 1;
	hash ^= static_cast<uint64_t>(m_Human) << 2;
	hash ^= static_cast<uint64_t>(m_InGamePlayer) << 3;
	hash ^= static_cast<uint64_t>(m_Aggressiveness) << 4;
	hash ^= static_cast<uint64_t>(m_GameOverRound) << 5;
	hash ^= RTE::Hash(g_PresetMan.GetDataModule(m_NativeTechModule)->GetFileName()) << 6;
	hash ^= static_cast<uint64_t>(m_NativeCostMult) << 7;
	hash ^= static_cast<uint64_t>(m_ForeignCostMult) << 8;
	hash ^= static_cast<uint64_t>(m_BrainPool) << 9;
	hash ^= static_cast<uint64_t>(m_Funds) << 10;
	hash ^= static_cast<uint64_t>(m_OffensiveBudget) << 11;
	hash ^= RTE::Hash(m_OffensiveTarget) << 12;

	return hashData;
}