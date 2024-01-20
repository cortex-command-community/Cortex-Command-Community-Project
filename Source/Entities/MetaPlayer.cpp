#include "MetaPlayer.h"
#include "DataModule.h"
#include "PresetMan.h"
#include "ActivityMan.h"

namespace RTE {

	ConcreteClassInfo(MetaPlayer, Entity, 0);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int MetaPlayer::Save(Writer& writer) const {
		Entity::Save(writer);

		writer.NewProperty("Name");
		writer << m_Name;
		writer.NewProperty("Team");
		writer << m_Team;
		writer.NewProperty("Human");
		writer << m_Human;
		writer.NewProperty("InGamePlayer");
		writer << m_InGamePlayer;
		writer.NewProperty("Aggressiveness");
		writer << m_Aggressiveness;
		writer.NewProperty("GameOverRound");
		writer << m_GameOverRound;

		// Need to write out the name, and not just the index of the module. it might change
		writer.NewProperty("NativeTechModule");

		writer << g_PresetMan.GetDataModule(m_NativeTechModule)->GetFileName();
		writer.NewProperty("NativeCostMultiplier");
		writer << m_NativeCostMult;
		writer.NewProperty("ForeignCostMultiplier");
		writer << m_ForeignCostMult;
		writer.NewProperty("BrainPool");
		writer << m_BrainPool;
		writer.NewProperty("Funds");
		writer << m_Funds;
		writer.NewProperty("OffensiveBudget");
		writer << m_OffensiveBudget;
		writer.NewProperty("OffensiveTarget");
		writer << (m_OffensiveTarget.empty() ? "None" : m_OffensiveTarget);

		return 0;
	}
} // namespace RTE