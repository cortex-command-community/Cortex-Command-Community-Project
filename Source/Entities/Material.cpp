#include "Material.h"
#include "Constants.h"

using namespace RTE;

ConcreteClassInfo(Material, Entity, 0);

void Material::Clear() {
	m_Index = 0;
	m_Priority = -1;
	m_Piling = 0;
	m_Integrity = 0.0F;
	m_Restitution = 0.0F;
	m_Friction = 0.0F;
	m_Stickiness = 0.0F;
	m_VolumeDensity = 0.0F;
	m_PixelDensity = 0.0F;
	m_GibImpulseLimitPerLiter = 0.0F;
	m_GibWoundLimitPerLiter = 0.0F;
	m_SettleMaterialIndex = 0;
	m_SpawnMaterialIndex = 0;
	m_IsScrap = false;
	m_Color.Reset();
	m_UseOwnColor = false;
	m_FGTextureFile.Reset();
	m_BGTextureFile.Reset();
	m_TerrainFGTexture = nullptr;
	m_TerrainBGTexture = nullptr;
}

int Material::Create(const Material& reference) {
	Entity::Create(reference);

	m_Index = reference.m_Index;
	m_Priority = reference.m_Priority;
	m_Piling = reference.m_Piling;
	m_Integrity = reference.m_Integrity;
	m_Restitution = reference.m_Restitution;
	m_Friction = reference.m_Friction;
	m_Stickiness = reference.m_Stickiness;
	m_VolumeDensity = reference.m_VolumeDensity;
	m_PixelDensity = reference.m_PixelDensity;
	m_GibImpulseLimitPerLiter = reference.m_GibImpulseLimitPerLiter;
	m_GibWoundLimitPerLiter = reference.m_GibWoundLimitPerLiter;
	m_SettleMaterialIndex = reference.m_SettleMaterialIndex;
	m_SpawnMaterialIndex = reference.m_SpawnMaterialIndex;
	m_IsScrap = reference.m_IsScrap;
	m_Color = reference.m_Color;
	m_UseOwnColor = reference.m_UseOwnColor;
	m_FGTextureFile = reference.m_FGTextureFile;
	m_BGTextureFile = reference.m_BGTextureFile;
	m_TerrainFGTexture = reference.m_TerrainFGTexture;
	m_TerrainBGTexture = reference.m_TerrainBGTexture;

	return 0;
}

int Material::ReadProperty(const std::string_view& propName, Reader& reader) {
	StartPropertyList(return Entity::ReadProperty(propName, reader));

	MatchProperty("Index", {
		// TODO: Check for index collisions here
		reader >> m_Index;
	});
	MatchProperty("Priority", { reader >> m_Priority; });
	MatchProperty("Piling", { reader >> m_Piling; });
	MatchForwards("Integrity") MatchProperty("StructuralIntegrity", {
		reader >> m_Integrity;
		m_Integrity = (m_Integrity == -1.0F) ? std::numeric_limits<float>::max() : m_Integrity;
	});
	MatchForwards("Restitution") MatchProperty("Bounce", { reader >> m_Restitution; });
	MatchProperty("Friction", { reader >> m_Friction; });
	MatchProperty("Stickiness", { reader >> m_Stickiness; });
	MatchProperty("DensityKGPerVolumeL", {
		reader >> m_VolumeDensity;
		// Overrides the pixel density
		m_PixelDensity = m_VolumeDensity * c_LPP;
	});
	MatchProperty("DensityKGPerPixel", {
		reader >> m_PixelDensity;
		// Overrides the volume density
		m_VolumeDensity = m_PixelDensity * c_PPL;
	});
	MatchProperty("GibImpulseLimitPerVolumeL", { reader >> m_GibImpulseLimitPerLiter; });
	MatchProperty("GibWoundLimitPerVolumeL", { reader >> m_GibWoundLimitPerLiter; });
	MatchProperty("SettleMaterial", { reader >> m_SettleMaterialIndex; });
	MatchForwards("SpawnMaterial") MatchProperty("TransformsInto", { reader >> m_SpawnMaterialIndex; });
	MatchProperty("IsScrap", { reader >> m_IsScrap; });
	MatchProperty("Color", { reader >> m_Color; });
	MatchProperty("UseOwnColor", { reader >> m_UseOwnColor; });
	MatchProperty("FGTextureFile", {
		reader >> m_FGTextureFile;
		m_TerrainFGTexture = m_FGTextureFile.GetAsBitmap();
	});
	MatchProperty("BGTextureFile", {
		reader >> m_BGTextureFile;
		m_TerrainBGTexture = m_BGTextureFile.GetAsBitmap();
	});

	EndPropertyList;
}

int Material::Save(Writer& writer) const {
	Entity::Save(writer);

	writer.NewDistinctProperty("Priority", m_Priority, -1);
	writer.NewDistinctProperty("Piling", m_Piling, 0);
	writer.NewDistinctProperty("StructuralIntegrity", m_Integrity, 0.0F);
	writer.NewDistinctProperty("Restitution", m_Restitution, 0.0F);
	writer.NewDistinctProperty("Friction", m_Friction, 0.0F);
	writer.NewDistinctProperty("Stickiness", m_Stickiness, 0.0F);
	writer.NewDistinctProperty("DensityKGPerVolumeL", m_VolumeDensity, 0.0F);
	writer.NewDistinctProperty("GibImpulseLimitPerVolumeL", m_GibImpulseLimitPerLiter, 0.0F);
	writer.NewDistinctProperty("GibWoundLimitPerVolumeL", m_GibWoundLimitPerLiter, 0.0F);
	writer.NewDistinctProperty("SettleMaterial", m_SettleMaterialIndex, (unsigned char) 0);
	writer.NewDistinctProperty("SpawnMaterial", m_SpawnMaterialIndex, (unsigned char) 0);
	writer.NewDistinctProperty("IsScrap", m_IsScrap, false);
	writer.NewPropertyWithValue("Color", m_Color);
	writer.NewDistinctProperty("UseOwnColor", m_UseOwnColor, false);
	writer.NewPropertyWithValue("FGTextureFile", m_FGTextureFile);
	writer.NewPropertyWithValue("BGTextureFile", m_BGTextureFile);

	return 0;
}

int Material::Write(Writer& writer, const Entity& entityReference, HashingData& hashData) const {
	Entity::Write(writer, entityReference, hashData);

	const Material& reference = static_cast<const Material&>(entityReference);

	writer.NewDistinctProperty("Priority", m_Priority, reference.m_Priority);
	writer.NewDistinctProperty("Piling", m_Piling, reference.m_Piling);
	writer.NewDistinctProperty("StructuralIntegrity", m_Integrity, reference.m_Integrity);
	writer.NewDistinctProperty("Restitution", m_Restitution, reference.m_Restitution);
	writer.NewDistinctProperty("Friction", m_Friction, reference.m_Friction);
	writer.NewDistinctProperty("Stickiness", m_Stickiness, reference.m_Stickiness);
	writer.NewDistinctProperty("DensityKGPerVolumeL", m_VolumeDensity, reference.m_VolumeDensity);
	writer.NewDistinctProperty("GibImpulseLimitPerVolumeL", m_GibImpulseLimitPerLiter, reference.m_GibImpulseLimitPerLiter);
	writer.NewDistinctProperty("GibWoundLimitPerVolumeL", m_GibWoundLimitPerLiter, reference.m_GibWoundLimitPerLiter);
	writer.NewDistinctProperty("SettleMaterial", m_SettleMaterialIndex, reference.m_SettleMaterialIndex);
	writer.NewDistinctProperty("SpawnMaterial", m_SpawnMaterialIndex, reference.m_SpawnMaterialIndex);
	writer.NewDistinctProperty("IsScrap", m_IsScrap, reference.m_IsScrap);
	writer.NewDistinctHashedProperty("Color", m_Color, hashData);
	writer.NewDistinctProperty("UseOwnColor", m_UseOwnColor, reference.m_UseOwnColor);
	writer.NewDistinctHashedProperty("FGTextureFile", m_FGTextureFile, hashData);
	writer.NewDistinctHashedProperty("BGTextureFile", m_BGTextureFile, hashData);

	return 0;
}

HashingData Material::Hash() const {
	HashingData hashData(Entity::Hash());
	uint64_t& hash = hashData.m_Hash;

	hash ^= static_cast<uint64_t>(m_Priority) << 0;
	hash ^= static_cast<uint64_t>(m_Piling) << 1;
	hash ^= static_cast<uint64_t>(m_Integrity) << 2;
	hash ^= static_cast<uint64_t>(m_Restitution) << 3;
	hash ^= static_cast<uint64_t>(m_Friction) << 4;
	hash ^= static_cast<uint64_t>(m_Stickiness) << 5;
	hash ^= static_cast<uint64_t>(m_VolumeDensity) << 6;
	hash ^= static_cast<uint64_t>(m_GibImpulseLimitPerLiter) << 7;
	hash ^= static_cast<uint64_t>(m_GibWoundLimitPerLiter) << 8;
	hash ^= std::hash<unsigned char>{}(m_SettleMaterialIndex) << 9;
	hash ^= std::hash<unsigned char>{}(m_SpawnMaterialIndex) << 10;
	hash ^= static_cast<uint64_t>(m_IsScrap) << 11;

	uint64_t colorHash = m_Color.Hash().m_Hash;
	hashData.m_Constituents.push_back(colorHash);
	hash ^= colorHash << 12;

	hash ^= static_cast<uint64_t>(m_UseOwnColor) << 13;

	uint64_t fgTextureHash = m_FGTextureFile.Hash().m_Hash;
	hashData.m_Constituents.push_back(fgTextureHash);
	hash ^= fgTextureHash << 14;

	uint64_t bgTextureHash = m_BGTextureFile.Hash().m_Hash;
	hashData.m_Constituents.push_back(bgTextureHash);
	hash ^= bgTextureHash << 15;

	return hashData;
}