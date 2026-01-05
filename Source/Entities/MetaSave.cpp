#include "MetaSave.h"
#include "GameActivity.h"
#include "MetaMan.h"
#include "ConsoleMan.h"

using namespace RTE;

ConcreteClassInfo(MetaSave, Entity, 0);

MetaSave::MetaSave() {
	Clear();
}

MetaSave::~MetaSave() {
	Destroy(true);
}

void MetaSave::Clear() {
	m_SavePath.clear();
	m_PlayerCount = 0;
	m_Difficulty = Activity::MediumDifficulty;
	m_RoundCount = 0;
	m_SiteCount = 0;
}

int MetaSave::Create(std::string savePath) {
	if (Entity::Create() < 0) {
		return -1;
	}
	if (!g_MetaMan.GameInProgress()) {
		g_ConsoleMan.PrintString("ERROR: Tried to save a Metagame that isn't in progress!?");
		return -1;
	}
	m_SavePath = std::move(savePath);

	m_PlayerCount = g_MetaMan.m_Players.size();
	m_Difficulty = g_MetaMan.m_Difficulty;
	m_RoundCount = g_MetaMan.m_CurrentRound;
	m_SiteCount = g_MetaMan.m_Scenes.size();

	return 0;
}

int MetaSave::Create(const MetaSave& reference) {
	Entity::Create(reference);

	m_SavePath = reference.m_SavePath;
	m_PlayerCount = reference.m_PlayerCount;
	m_Difficulty = reference.m_Difficulty;
	m_RoundCount = reference.m_RoundCount;
	m_SiteCount = reference.m_SiteCount;

	return 0;
}

int MetaSave::ReadProperty(const std::string_view& propName, Reader& reader) {
	StartPropertyList(return Entity::ReadProperty(propName, reader));

	MatchProperty("SavePath", { reader >> m_SavePath; });
	MatchProperty("PlayerCount", { reader >> m_PlayerCount; });
	MatchProperty("Difficulty", { reader >> m_Difficulty; });
	MatchProperty("RoundCount", { reader >> m_RoundCount; });
	MatchProperty("SiteCount", { reader >> m_SiteCount; });

	EndPropertyList;
}

int MetaSave::Save(Writer& writer) const {
	Entity::Save(writer);

	if (!m_SavePath.empty())
		writer.NewPropertyWithValue("SavePath", m_SavePath);

	writer.NewDistinctProperty("PlayerCount", m_PlayerCount, 0);
	writer.NewDistinctProperty("Difficulty", m_Difficulty, (int)Activity::MediumDifficulty);
	writer.NewDistinctProperty("RoundCount", m_RoundCount, 0);
	writer.NewDistinctProperty("SiteCount", m_SiteCount, 0);

	return 0;
}

int MetaSave::Write(Writer& writer, const Entity& entityReference, HashingData& hashData) const {
	Entity::Write(writer, entityReference, hashData);

	const MetaSave& reference = static_cast<const MetaSave&>(entityReference);

	writer.NewDistinctProperty("SavePath", m_SavePath, reference.m_SavePath);
	writer.NewDistinctProperty("PlayerCount", m_PlayerCount, reference.m_PlayerCount);
	writer.NewDistinctProperty("Difficulty", m_Difficulty, reference.m_Difficulty);
	writer.NewDistinctProperty("RoundCount", m_RoundCount, reference.m_RoundCount);
	writer.NewDistinctProperty("SiteCount", m_SiteCount, reference.m_SiteCount);

	return 0;
}

HashingData MetaSave::Hash() const {
	HashingData hashData(std::move(Entity::Hash()));
	uint64_t& hash = hashData.m_Hash;

	hash ^= RTE::Hash(m_SavePath) << 0;
	hash ^= static_cast<uint64_t>(m_PlayerCount) << 1;
	hash ^= static_cast<uint64_t>(m_Difficulty) << 2;
	hash ^= static_cast<uint64_t>(m_RoundCount) << 3;
	hash ^= static_cast<uint64_t>(m_SiteCount) << 4;

	return hashData;
}
