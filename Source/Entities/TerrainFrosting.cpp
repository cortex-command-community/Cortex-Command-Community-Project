#include "TerrainFrosting.h"
#include "SLTerrain.h"
#include "PresetMan.h"

using namespace RTE;

const std::string TerrainFrosting::c_ClassName = "TerrainFrosting";

void TerrainFrosting::Clear() {
	m_FrostingMaterial = 0;
	m_TargetMaterial = 0;
	m_MinThickness = 5;
	m_MaxThickness = 5;
	m_InAirOnly = true;
}

int TerrainFrosting::ReadProperty(const std::string_view& propName, Reader& reader) {
	StartPropertyList(return Serializable::ReadProperty(propName, reader));

	MatchProperty("FrostingMaterial", {
		const Entity* entityReference = g_PresetMan.GetEntityPresetFromCharacteristic(reader);
		if (const Material* reference = dynamic_cast<const Material*>(entityReference)) {
			m_FrostingMaterial = reference;
		} else {
			reader.ReportError("Tried to point FrostingMaterial to a non-Material type!");
		}
	});
	MatchProperty("TargetMaterial", {
		const Entity* entityReference = g_PresetMan.GetEntityPresetFromCharacteristic(reader);
		if (const Material* reference = dynamic_cast<const Material*>(entityReference)) {
			m_TargetMaterial = reference;
		} else {
			reader.ReportError("Tried to point TargetMaterial to a non-Material type!");
		}
	});
	MatchProperty("MinThickness", { reader >> m_MinThickness; });
	MatchProperty("MaxThickness", { reader >> m_MaxThickness; });
	MatchProperty("InAirOnly", { reader >> m_InAirOnly; });

	EndPropertyList;
}

int TerrainFrosting::Save(Writer& writer) const {
	Serializable::Save(writer);

	writer.NewPropertyWithValue("FrostingMaterial", m_FrostingMaterial->GetEntityCharacteristic());
	writer.NewPropertyWithValue("TargetMaterial", m_TargetMaterial->GetEntityCharacteristic());
	writer.NewPropertyWithValue("MinThickness", m_MinThickness);
	writer.NewPropertyWithValue("MaxThickness", m_MaxThickness);
	writer.NewPropertyWithValue("InAirOnly", m_InAirOnly);

	return 0;
}

HashingData TerrainFrosting::Hash() const {
	HashingData hashData(std::move(Serializable::Hash()));
	uint64_t& hash = hashData.m_Hash;

	hash ^= RTE::Hash(m_FrostingMaterial->GetEntityCharacteristic());
	hash ^= RTE::Hash(m_TargetMaterial->GetEntityCharacteristic());
	hash ^= std::hash<int>{}(m_MinThickness);
	hash ^= std::hash<int>{}(m_MaxThickness);
	hash ^= std::hash<bool>{}(m_InAirOnly);

	return hashData;
}

void TerrainFrosting::FrostTerrain(SLTerrain* terrain) const {
	BITMAP* frostingTexture = m_FrostingMaterial->GetFGTexture();
	BITMAP* fgColorBitmap = terrain->GetFGColorBitmap();
	BITMAP* matBitmap = terrain->GetBitmap();

	bool targetMatFound = false;
	bool applyingFrosting = false;
	int appliedThickness = 0;

	for (int xPos = 0; xPos < matBitmap->w; ++xPos) {
		int thicknessGoal = RandomNum(m_MinThickness, m_MaxThickness);

		for (int yPos = matBitmap->h - 1; yPos >= 0; --yPos) {
			int materialCheckPixel = _getpixel(matBitmap, xPos, yPos);

			if (!targetMatFound && materialCheckPixel == m_TargetMaterial->GetIndex()) {
				targetMatFound = true;
				appliedThickness = 0;
			} else if (targetMatFound && materialCheckPixel != m_TargetMaterial->GetIndex() && appliedThickness <= thicknessGoal) {
				targetMatFound = false;
				applyingFrosting = true;
			}
			if (applyingFrosting && (materialCheckPixel == MaterialColorKeys::g_MaterialAir || !m_InAirOnly) && appliedThickness <= thicknessGoal) {
				_putpixel(fgColorBitmap, xPos, yPos, frostingTexture ? _getpixel(frostingTexture, xPos % frostingTexture->w, yPos % frostingTexture->h) : m_FrostingMaterial->GetColor().GetIndex());
				_putpixel(matBitmap, xPos, yPos, m_FrostingMaterial->GetIndex());
				appliedThickness++;
			} else {
				applyingFrosting = false;
			}
		}
	}
}
