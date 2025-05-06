#include "TerrainDebris.h"
#include "SLTerrain.h"
#include "PresetMan.h"

using namespace RTE;

ConcreteClassInfo(TerrainDebris, Entity, 0);

TerrainDebris::TerrainDebris() {
	Clear();
}

TerrainDebris::~TerrainDebris() {
	Destroy(true);
}

void TerrainDebris::Clear() {
	m_DebrisFile.Reset();
	m_Bitmaps.clear();
	m_BitmapCount = 0;
	m_Material = nullptr;
	m_TargetMaterial = nullptr;
	m_DebrisPlacementMode = DebrisPlacementMode::NoPlacementRestrictions;
	m_OnlyBuried = false;
	m_MinDepth = 0;
	m_MaxDepth = 10;
	m_MinRotation = 0;
	m_MaxRotation = 0;
	m_CanHFlip = false;
	m_CanVFlip = false;
	m_FlipChance = 0.5F;
	m_Density = 0.01F;
}

int TerrainDebris::Create() {
	Entity::Create();
	m_DebrisFile.GetAsAnimation(m_Bitmaps, m_BitmapCount);
	RTEAssert(!m_Bitmaps.empty() && m_Bitmaps[0], "Failed to load debris bitmaps during TerrainDebris::Create!");
	return 0;
}

int TerrainDebris::Create(const TerrainDebris& reference) {
	Entity::Create(reference);

	m_DebrisFile = reference.m_DebrisFile;
	m_Bitmaps.clear();
	m_Bitmaps = reference.m_Bitmaps;
	m_BitmapCount = reference.m_BitmapCount;
	m_Material = reference.m_Material;
	m_TargetMaterial = reference.m_TargetMaterial;
	m_DebrisPlacementMode = reference.m_DebrisPlacementMode;
	m_OnlyBuried = reference.m_OnlyBuried;
	m_MinDepth = reference.m_MinDepth;
	m_MaxDepth = reference.m_MaxDepth;
	m_MinRotation = reference.m_MinRotation;
	m_MaxRotation = reference.m_MaxRotation;
	m_CanHFlip = reference.m_CanHFlip;
	m_CanVFlip = reference.m_CanVFlip;
	m_FlipChance = reference.m_FlipChance;
	m_Density = reference.m_Density;

	return 0;
}

int TerrainDebris::ReadProperty(const std::string_view& propName, Reader& reader) {
	StartPropertyList(return Entity::ReadProperty(propName, reader));

	MatchProperty("DebrisFile", { reader >> m_DebrisFile; });
	MatchProperty("DebrisPieceCount", {
		reader >> m_BitmapCount;
		m_Bitmaps.reserve(m_BitmapCount);
	});
	MatchProperty("DebrisMaterial", {
		const Entity* entityReference = g_PresetMan.GetEntityPresetFromCharacteristic(reader);
		const Material* reference = dynamic_cast<const Material*>(entityReference);
		if (entityReference == nullptr || reference != nullptr) {
			m_Material = reference;
		} else {
			reader.ReportError("Tried to point DebrisMaterial to a non-Material type!");
		}
	});
	MatchProperty("TargetMaterial", {
		const Entity* entityReference = g_PresetMan.GetEntityPresetFromCharacteristic(reader);
		const Material* reference = dynamic_cast<const Material*>(entityReference);
		if (entityReference == nullptr || reference != nullptr) {
			m_TargetMaterial = reference;
		} else {
			reader.ReportError("Tried to point TargetMaterial to a non-Material type!");
		}
	});
	MatchProperty("DebrisPlacementMode", {
		m_DebrisPlacementMode = static_cast<DebrisPlacementMode>(std::stoi(reader.ReadPropValue()));
		if (m_DebrisPlacementMode < DebrisPlacementMode::NoPlacementRestrictions || m_DebrisPlacementMode > DebrisPlacementMode::OnOverhangAndCavityOverhang) {
			reader.ReportError("Invalid TerrainDebris placement mode!");
		}
	});
	MatchProperty("OnlyBuried", { reader >> m_OnlyBuried; });
	MatchProperty("MinDepth", { reader >> m_MinDepth; });
	MatchProperty("MaxDepth", { reader >> m_MaxDepth; });
	MatchProperty("MinRotation", { reader >> m_MinRotation; });
	MatchProperty("MaxRotation", { reader >> m_MaxRotation; });
	MatchProperty("CanHFlip", { reader >> m_CanHFlip; });
	MatchProperty("CanVFlip", { reader >> m_CanVFlip; });
	MatchProperty("FlipChance", { reader >> m_FlipChance; });
	MatchProperty("DensityPerMeter", { reader >> m_Density; });

	EndPropertyList;
}

int TerrainDebris::Save(Writer& writer) const {
	Entity::Save(writer);

	writer.NewPropertyWithValue("DebrisFile", m_DebrisFile);
	writer.NewDistinctProperty("DebrisPieceCount", m_BitmapCount, 0);
	writer.NewPresetReferenceProperty("DebrisMaterial", m_Material, static_cast<const Material*>(nullptr));
	writer.NewPresetReferenceProperty("TargetMaterial", m_TargetMaterial, static_cast<const Material*>(nullptr));
	writer.NewDistinctProperty("DebrisPlacementMode", m_DebrisPlacementMode, DebrisPlacementMode::NoPlacementRestrictions);
	writer.NewDistinctProperty("OnlyBuried", m_OnlyBuried, false);
	writer.NewDistinctProperty("MinDepth", m_MinDepth, 0);
	writer.NewDistinctProperty("MaxDepth", m_MaxDepth, 10);
	writer.NewDistinctProperty("MinRotation", m_MinRotation, 0);
	writer.NewDistinctProperty("MaxRotation", m_MaxRotation, 0);
	writer.NewDistinctProperty("CanHFlip", m_CanHFlip, false);
	writer.NewDistinctProperty("CanVFlip", m_CanVFlip, false);
	writer.NewDistinctProperty("FlipChance", m_FlipChance, 0.5F);
	writer.NewDistinctProperty("DensityPerMeter", m_Density, 0.01F);

	return 0;
}

int TerrainDebris::Write(Writer& writer, const Entity& entityReference, HashingData& hashData) const {
	Entity::Write(writer, entityReference, hashData);

	const TerrainDebris& reference = static_cast<const TerrainDebris&>(entityReference);

	writer.NewDistinctHashedProperty("DebrisFile", m_DebrisFile, hashData);
	writer.NewDistinctProperty("DebrisPieceCount", m_BitmapCount, reference.m_BitmapCount);
	writer.NewPresetReferenceProperty("DebrisMaterial", m_Material, reference.m_Material);
	writer.NewPresetReferenceProperty("TargetMaterial", m_TargetMaterial, reference.m_TargetMaterial);
	writer.NewDistinctProperty("DebrisPlacementMode", m_DebrisPlacementMode, reference.m_DebrisPlacementMode);
	writer.NewDistinctProperty("OnlyBuried", m_OnlyBuried, reference.m_OnlyBuried);
	writer.NewDistinctProperty("MinDepth", m_MinDepth, reference.m_MinDepth);
	writer.NewDistinctProperty("MaxDepth", m_MaxDepth, reference.m_MaxDepth);
	writer.NewDistinctProperty("MinRotation", m_MinRotation, reference.m_MinRotation);
	writer.NewDistinctProperty("MaxRotation", m_MaxRotation, reference.m_MaxRotation);
	writer.NewDistinctProperty("CanHFlip", m_CanHFlip, reference.m_CanHFlip);
	writer.NewDistinctProperty("CanVFlip", m_CanVFlip, reference.m_CanVFlip);
	writer.NewDistinctProperty("FlipChance", m_FlipChance, reference.m_FlipChance);
	writer.NewDistinctProperty("DensityPerMeter", m_Density, reference.m_Density);

	return 0;
}

HashingData TerrainDebris::Hash() const {
	HashingData hashData(std::move(Entity::Hash()));
	uint64_t& hash = hashData.m_Hash;

	uint64_t fileHash = m_DebrisFile.Hash().m_Hash;
	hashData.m_Constituents.push_back(fileHash);
	hash ^= fileHash << 0;

	hash ^= static_cast<uint64_t>(m_BitmapCount) << 1;
	hash ^= RTE::Hash(m_Material->GetEntityCharacteristic()) << 2;
	hash ^= RTE::Hash(m_TargetMaterial->GetEntityCharacteristic()) << 3;
	hash ^= static_cast<uint64_t>(m_DebrisPlacementMode) << 4;
	hash ^= static_cast<uint64_t>(m_OnlyBuried) << 5;
	hash ^= static_cast<uint64_t>(m_MinDepth) << 6;
	hash ^= static_cast<uint64_t>(m_MaxDepth) << 7;
	hash ^= static_cast<uint64_t>(m_MinRotation) << 8;
	hash ^= static_cast<uint64_t>(m_MaxRotation) << 9;
	hash ^= static_cast<uint64_t>(m_CanHFlip) << 10;
	hash ^= static_cast<uint64_t>(m_CanVFlip) << 11;
	hash ^= static_cast<uint64_t>(m_FlipChance) << 12;
	hash ^= static_cast<uint64_t>(m_Density) << 13;

	return hashData;
}

bool TerrainDebris::GetPiecePlacementPosition(SLTerrain* terrain, Box& possiblePiecePosition) const {
	BITMAP* matBitmap = terrain->GetMaterialBitmap();
	int posX = RandomNum(0, matBitmap->w);
	int depth = RandomNum(m_MinDepth, m_MaxDepth);
	int buriedDepthOffset = m_OnlyBuried ? static_cast<int>(possiblePiecePosition.GetHeight() * 0.6F) : 0;
	int prevMaterialCheckPixel = -1;

	bool scanForOverhang = m_DebrisPlacementMode == DebrisPlacementMode::OnOverhangOnly || m_DebrisPlacementMode == DebrisPlacementMode::OnCavityOverhangOnly || m_DebrisPlacementMode == DebrisPlacementMode::OnOverhangAndCavityOverhang;

	// For overhangs scan from the bottom so it's easier to detect.
	for (int surfacePosY = 0, overhangPosY = matBitmap->h - 1; surfacePosY < matBitmap->h && overhangPosY > 0; surfacePosY++, overhangPosY--) {
		int posY = scanForOverhang ? overhangPosY : surfacePosY;
		int materialCheckPixel = _getpixel(matBitmap, posX, posY);
		if (materialCheckPixel != MaterialColorKeys::g_MaterialAir) {
			// TODO: Adding depth here will properly place debris only on target material, rather than any random pixel within depth range from the original target material pixel.
			int depthAdjustedPosY = posY; // + (depth * (scanForOverhang ? -1 : 1));
			if (scanForOverhang ? (depthAdjustedPosY > 0) : (depthAdjustedPosY < matBitmap->h)) {
				// TODO: Enable the depth check once multi TargetMaterial debris is supported.
				// materialCheckPixel = _getpixel(matBitmap, posX, depthAdjustedPosY);
				if (MaterialPixelIsValidTarget(materialCheckPixel, prevMaterialCheckPixel)) {
					surfacePosY += (depth + buriedDepthOffset);
					overhangPosY -= (depth + buriedDepthOffset);
					possiblePiecePosition.SetCenter(Vector(static_cast<float>(posX), static_cast<float>(scanForOverhang ? overhangPosY : surfacePosY)));
					if (!m_OnlyBuried || terrain->IsBoxBuried(possiblePiecePosition)) {
						return true;
					}
				}
			}
		}
		prevMaterialCheckPixel = materialCheckPixel;
	}
	return false;
}

bool TerrainDebris::MaterialPixelIsValidTarget(int materialCheckPixel, int prevMaterialCheckPixel) const {
	bool checkResult = true;

	if (materialCheckPixel != m_TargetMaterial->GetIndex()) {
		checkResult = false;
	} else {
		// TODO: Consider using disgustang bitwise/shifting junk instead of enum.
		switch (m_DebrisPlacementMode) {
			case DebrisPlacementMode::OnSurfaceOnly:
			case DebrisPlacementMode::OnOverhangOnly:
				if (prevMaterialCheckPixel != MaterialColorKeys::g_MaterialAir) {
					checkResult = false;
				}
				break;
			case DebrisPlacementMode::OnCavitySurfaceOnly:
			case DebrisPlacementMode::OnCavityOverhangOnly:
				if (prevMaterialCheckPixel != MaterialColorKeys::g_MaterialCavity) {
					checkResult = false;
				}
				break;
			case DebrisPlacementMode::OnSurfaceAndCavitySurface:
			case DebrisPlacementMode::OnOverhangAndCavityOverhang:
				if (prevMaterialCheckPixel != MaterialColorKeys::g_MaterialAir && prevMaterialCheckPixel != MaterialColorKeys::g_MaterialCavity) {
					checkResult = false;
				}
				break;
			default:
				// No placement restrictions, pixel just needs to be of target material.
				break;
		}
	}
	return checkResult;
}

void TerrainDebris::DrawToTerrain(SLTerrain* terrain, BITMAP* bitmapToDraw, const Vector& position) const {
	// Create a square temp bitmap that is larger than the original to avoid clipping if rotating.
	int dimensions = 10 + std::max(bitmapToDraw->w, bitmapToDraw->h);
	BITMAP* tempDrawBitmap = create_bitmap_ex(8, dimensions, dimensions);
	clear_bitmap(tempDrawBitmap);

	// Offset the original bitmap on the temp bitmap so it's centered, otherwise will be positioned incorrectly and can clip if rotated or flipped.
	int offsetX = (dimensions - bitmapToDraw->w) / 2;
	int offsetY = (dimensions - bitmapToDraw->h) / 2;
	blit(bitmapToDraw, tempDrawBitmap, 0, 0, offsetX, offsetY, bitmapToDraw->w, bitmapToDraw->h);

	BITMAP* tempFlipAndRotBitmap = nullptr;
	if (m_CanHFlip || m_CanVFlip || m_MinRotation != 0 || m_MaxRotation != 0) {
		tempFlipAndRotBitmap = create_bitmap_ex(8, dimensions, dimensions);

		if (m_CanHFlip && m_FlipChance >= RandomNum()) {
			clear_bitmap(tempFlipAndRotBitmap);
			draw_sprite_h_flip(tempFlipAndRotBitmap, tempDrawBitmap, 0, 0);
			blit(tempFlipAndRotBitmap, tempDrawBitmap, 0, 0, 0, 0, dimensions, dimensions);
		}
		if (m_CanVFlip && m_FlipChance >= RandomNum()) {
			clear_bitmap(tempFlipAndRotBitmap);
			draw_sprite_v_flip(tempFlipAndRotBitmap, tempDrawBitmap, 0, 0);
			blit(tempFlipAndRotBitmap, tempDrawBitmap, 0, 0, 0, 0, dimensions, dimensions);
		}
		if (m_MinRotation != 0 || m_MaxRotation != 0) {
			clear_bitmap(tempFlipAndRotBitmap);
			rotate_sprite(tempFlipAndRotBitmap, tempDrawBitmap, 0, 0, ftofix(GetAllegroAngle(static_cast<float>(RandomNum(m_MinRotation, m_MaxRotation)))));
			blit(tempFlipAndRotBitmap, tempDrawBitmap, 0, 0, 0, 0, dimensions, dimensions);
		}
	}
	draw_sprite(terrain->GetFGColorBitmap(), tempDrawBitmap, position.GetFloorIntX() - offsetX, position.GetFloorIntY() - offsetY);
	draw_character_ex(terrain->GetMaterialBitmap(), tempDrawBitmap, position.GetFloorIntX() - offsetX, position.GetFloorIntY() - offsetY, m_Material->GetIndex(), -1);

	if (tempFlipAndRotBitmap) {
		destroy_bitmap(tempFlipAndRotBitmap);
	}
	destroy_bitmap(tempDrawBitmap);
}

void TerrainDebris::ScatterOnTerrain(SLTerrain* terrain) {
	RTEAssert(!m_Bitmaps.empty() && m_BitmapCount > 0, "No bitmaps loaded for terrain debris during TerrainDebris::ScatterOnTerrain!");

	int possiblePieceToPlaceCount = static_cast<int>((static_cast<float>(terrain->GetMaterialBitmap()->w) * c_MPP) * m_Density);
	for (int piece = 0; piece < possiblePieceToPlaceCount; ++piece) {
		int pieceBitmapIndex = RandomNum(0, m_BitmapCount - 1);
		RTEAssert(pieceBitmapIndex >= 0 && pieceBitmapIndex < m_BitmapCount, "Bitmap index was out of bounds during TerrainDebris::ScatterOnTerrain!");
		Box possiblePiecePosition(Vector(), static_cast<float>(m_Bitmaps[pieceBitmapIndex]->w), static_cast<float>(m_Bitmaps.at(pieceBitmapIndex)->h));
		if (GetPiecePlacementPosition(terrain, possiblePiecePosition)) {
			DrawToTerrain(terrain, m_Bitmaps[pieceBitmapIndex], possiblePiecePosition.GetCorner());
		}
	}
}
