#include "TerrainObject.h"
#include "SLTerrain.h"
#include "SceneMan.h"
#include "Draw.h"
#include "FrameMan.h"

using namespace RTE;

ConcreteClassInfo(TerrainObject, SceneObject, 0);

TerrainObject::TerrainObject() {
	Clear();
}

TerrainObject::~TerrainObject() {
	Destroy(true);
}

void TerrainObject::Clear() {
	m_FGColorFile.Reset();
	m_FGColorBitmap = nullptr;
	m_BGColorFile.Reset();
	m_BGColorBitmap = nullptr;
	m_MaterialFile.Reset();
	m_MaterialBitmap = nullptr;
	m_BitmapOffset.Reset();
	m_OffsetDefined = false;
	m_ChildObjects.clear();
}

int TerrainObject::Create() {
	SceneObject::Create();

	if (!m_OffsetDefined) {
		int bitmapWidth = GetBitmapWidth();
		int bitmapHeight = GetBitmapHeight();

		if (bitmapWidth > 24) {
			m_BitmapOffset.SetX(-(static_cast<float>(bitmapWidth / 2)));
		}
		if (bitmapHeight > 24) {
			m_BitmapOffset.SetY(-(static_cast<float>(bitmapHeight / 2)));
		}
	}
	return 0;
}

int TerrainObject::Create(const TerrainObject& reference) {
	SceneObject::Create(reference);

	m_FGColorFile = reference.m_FGColorFile;
	m_FGColorBitmap = reference.m_FGColorBitmap;
	m_BGColorFile = reference.m_BGColorFile;
	m_BGColorBitmap = reference.m_BGColorBitmap;
	m_MaterialFile = reference.m_MaterialFile;
	m_MaterialBitmap = reference.m_MaterialBitmap;
	m_BitmapOffset = reference.m_BitmapOffset;
	m_OffsetDefined = reference.m_OffsetDefined;

	for (const SceneObject::SOPlacer& childObject: reference.m_ChildObjects) {
		m_ChildObjects.emplace_back(childObject);
	}
	return 0;
}

int TerrainObject::ReadProperty(const std::string_view& propName, Reader& reader) {
	StartPropertyList(return SceneObject::ReadProperty(propName, reader));

	MatchProperty("FGColorFile", {
		reader >> m_FGColorFile;
		m_FGColorBitmap = m_FGColorFile.GetAsBitmap();
	});
	MatchProperty("BGColorFile", {
		reader >> m_BGColorFile;
		m_BGColorBitmap = m_BGColorFile.GetAsBitmap();
	});
	MatchProperty("MaterialFile", {
		reader >> m_MaterialFile;
		m_MaterialBitmap = m_MaterialFile.GetAsBitmap();
	});
	MatchProperty("BitmapOffset", {
		reader >> m_BitmapOffset;
		m_OffsetDefined = true;
	});
	MatchForwards("AddChildObject") MatchProperty("_AddChildObject", {
		SceneObject::SOPlacer newChildObject;
		reader >> newChildObject;
		newChildObject.SetTeam(m_Team);
		m_ChildObjects.emplace_back(newChildObject);
	});
	MatchForwards("ClearChildObjects") MatchProperty("_ClearChildObjects", {
		reader.ReadPropValue();
		m_ChildObjects.clear();
	});

	EndPropertyList;
}

int TerrainObject::Save(Writer& writer) const {
	SceneObject::Save(writer);

	if (!m_FGColorFile.GetDataPath().empty()) {
		writer.NewPropertyWithValue("FGColorFile", m_FGColorFile);
	}
	if (!m_BGColorFile.GetDataPath().empty()) {
		writer.NewPropertyWithValue("BGColorFile", m_BGColorFile);
	}
	if (!m_MaterialFile.GetDataPath().empty()) {
		writer.NewPropertyWithValue("MaterialFile", m_MaterialFile);
	}

	if (m_OffsetDefined) {
		writer.NewPropertyWithValue("BitmapOffset", m_BitmapOffset);
	}

	for (const SceneObject::SOPlacer& childObject: m_ChildObjects) {
		writer.NewPropertyWithValue("AddChildObject", childObject);
	}
	return 0;
}

int TerrainObject::Write(Writer& writer, const Entity& entityReference, HashingData& hashData) const {
	SceneObject::Write(writer, entityReference, hashData);

	writer.NewDistinctHashedProperty("FGColorFile", m_FGColorFile, hashData);
	writer.NewDistinctHashedProperty("BGColorFile", m_BGColorFile, hashData);
	writer.NewDistinctHashedProperty("MaterialFile", m_MaterialFile, hashData);
	writer.NewDistinctHashedProperty("BitmapOffset", m_BitmapOffset, hashData);
	writer.NewSequence("_ClearChildObjects", "_AddChildObject", m_ChildObjects, hashData);

	return 0;
}

HashingData TerrainObject::Hash() const {
	HashingData hashData(std::move(SceneObject::Hash()));
	uint64_t& hash = hashData.m_Hash;

	uint64_t fgColorHash = m_FGColorFile.Hash().m_Hash;
	hashData.m_Constituents.push_back(fgColorHash);
	hash ^= fgColorHash << 0;

	uint64_t bgColorHash = m_BGColorFile.Hash().m_Hash;
	hashData.m_Constituents.push_back(bgColorHash);
	hash ^= bgColorHash << 1;

	uint64_t materialHash = m_MaterialFile.Hash().m_Hash;
	hashData.m_Constituents.push_back(materialHash);
	hash ^= materialHash << 2;

	hash ^= m_BitmapOffset.Hash().m_Hash << 3;

	int i = 0;
	
	for (const SceneObject::SOPlacer& childObject: m_ChildObjects) {
		uint64_t childHash = childObject.Hash().m_Hash;
		hashData.m_Constituents.push_back(childHash);
		hash ^= childHash << (i++ % sizeof(uint64_t) * 8);
	}

	hashData.m_ParseValues.push_back(i);

	return hashData;
}

BITMAP* TerrainObject::GetGraphicalIcon() const {
	if (m_FGColorBitmap) {
		// Check several spots on the FG bitmap, to be sure it has parts that aren't transparent. If not, show the background layer instead.
		int piece = m_FGColorBitmap->w / 10;
		if (_getpixel(m_FGColorBitmap, m_FGColorBitmap->w / 2, m_FGColorBitmap->h / 2) != ColorKeys::g_MaskColor || _getpixel(m_FGColorBitmap, piece, piece) != ColorKeys::g_MaskColor || _getpixel(m_FGColorBitmap, m_FGColorBitmap->w - piece, piece) != ColorKeys::g_MaskColor || _getpixel(m_FGColorBitmap, piece, m_FGColorBitmap->h - piece) != ColorKeys::g_MaskColor) {
			return m_FGColorBitmap;
		}
	}
	return m_BGColorBitmap;
}

void TerrainObject::SetTeam(int team) {
	SceneObject::SetTeam(team);
	for (SceneObject::SOPlacer& childObject: m_ChildObjects) {
		childObject.SetTeam(team);
	}
}

bool TerrainObject::IsOnScenePoint(Vector& scenePoint) const {
	// TODO: TAKE CARE OF WRAPPING
	Vector bitmapPos = m_Pos + m_BitmapOffset;
	if (WithinBox(scenePoint, bitmapPos, static_cast<float>(GetBitmapWidth()), static_cast<float>(GetBitmapHeight()))) {
		Vector bitmapPoint = scenePoint - bitmapPos;
		if (m_FGColorBitmap && getpixel(m_FGColorBitmap, bitmapPoint.GetFloorIntX(), bitmapPoint.GetFloorIntY()) != MaterialColorKeys::g_MaterialAir) {
			return true;
		}
		if (m_BGColorBitmap && getpixel(m_BGColorBitmap, bitmapPoint.GetFloorIntX(), bitmapPoint.GetFloorIntY()) != MaterialColorKeys::g_MaterialAir) {
			return true;
		}
		if (m_MaterialBitmap && getpixel(m_MaterialBitmap, bitmapPoint.GetFloorIntX(), bitmapPoint.GetFloorIntY()) != MaterialColorKeys::g_MaterialAir) {
			return true;
		}
	}
	return false;
}

bool TerrainObject::PlaceOnTerrain(SLTerrain* terrain) {
	if (!terrain) {
		return false;
	}
	DrawToTerrain(terrain);

	// Reapply the team so all children are guaranteed to be on the same team.
	SetTeam(GetTeam());

	for (const SceneObject::SOPlacer& childObject: GetChildObjects()) {
		// TODO: check if we're placing a brain, and have it replace the resident brain of the scene!
		g_SceneMan.AddSceneObject(childObject.GetPlacedCopy(this));
	}
	return true;
}

void TerrainObject::Draw(BITMAP* targetBitmap, const Vector& targetPos, DrawMode drawMode, bool onlyPhysical) const {
	std::array<Vector, 4> drawPos = {m_Pos + m_BitmapOffset - targetPos};
	int wrapPasses = 1;

	// See if need to double draw this across the scene seam if we're being drawn onto a scenewide bitmap.
	if (targetPos.IsZero() && g_SceneMan.GetSceneWidth() <= targetBitmap->w) {
		if (drawPos[0].GetFloorIntX() < m_FGColorBitmap->w) {
			drawPos.at(wrapPasses) = drawPos[0];
			drawPos.at(wrapPasses).m_X += static_cast<float>(targetBitmap->w);
			wrapPasses++;
		} else if (drawPos[0].GetFloorIntX() > targetBitmap->w - m_FGColorBitmap->w) {
			drawPos.at(wrapPasses) = drawPos[0];
			drawPos.at(wrapPasses).m_X -= static_cast<float>(targetBitmap->w);
			wrapPasses++;
		}
	} else {
		// Only screenwide target bitmap, so double draw within the screen if the screen is straddling a scene seam.
		if (g_SceneMan.SceneWrapsX()) {
			float sceneWidth = static_cast<float>(g_SceneMan.GetSceneWidth());
			if (targetPos.m_X < 0) {
				drawPos.at(wrapPasses) = drawPos[0];
				drawPos.at(wrapPasses).m_X -= sceneWidth;
				wrapPasses++;
			}
			if (targetPos.m_X + static_cast<float>(targetBitmap->w) > sceneWidth) {
				drawPos.at(wrapPasses) = drawPos[0];
				drawPos.at(wrapPasses).m_X += sceneWidth;
				wrapPasses++;
			}
		}
	}
	for (int i = 0; i < wrapPasses; ++i) {
		switch (drawMode) {
			case DrawMode::g_DrawColor:
				if (HasBGColorBitmap()) {
					masked_blit(m_BGColorBitmap, targetBitmap, 0, 0, drawPos.at(i).GetFloorIntX(), drawPos.at(i).GetFloorIntY(), m_BGColorBitmap->w, m_BGColorBitmap->h);
				}
				if (HasFGColorBitmap()) {
					masked_blit(m_FGColorBitmap, targetBitmap, 0, 0, drawPos.at(i).GetFloorIntX(), drawPos.at(i).GetFloorIntY(), m_FGColorBitmap->w, m_FGColorBitmap->h);
				}
				break;
			case DrawMode::g_DrawMaterial:
				if (HasMaterialBitmap()) {
					masked_blit(m_MaterialBitmap, targetBitmap, 0, 0, drawPos.at(i).GetFloorIntX(), drawPos.at(i).GetFloorIntY(), m_MaterialBitmap->w, m_MaterialBitmap->h);
				}
				break;
			case DrawMode::g_DrawTrans:
				if (HasFGColorBitmap()) {
					DrawTexture(m_FGColorBitmap, drawPos.at(i).GetFloorIntX(), drawPos.at(i).GetFloorIntY(), {255, 255, 255, g_FrameMan.GetCurrentAlpha()});
				}
				if (HasBGColorBitmap()) {
					DrawTexture(m_BGColorBitmap, drawPos.at(i).GetFloorIntX(), drawPos.at(i).GetFloorIntY(), {255, 255, 255, g_FrameMan.GetCurrentAlpha()});
				}
				break;
			default:
				break;
		}
	}
}

void TerrainObject::DrawToTerrain(SLTerrain* terrain) {
	BITMAP* terrainMatBitmap = terrain->GetMaterialBitmap();
	BITMAP* terrainBGBitmap = terrain->GetBGColorBitmap();
	BITMAP* terrainFGBitmap = terrain->GetFGColorBitmap();

	Vector posOnScene = m_Pos + m_BitmapOffset;

	if (terrain->WrapsX()) {
		if (posOnScene.GetFloorIntX() < 0) {
			if (HasMaterialBitmap()) {
				draw_sprite(terrainMatBitmap, m_MaterialBitmap, posOnScene.GetFloorIntX() + terrainMatBitmap->w, posOnScene.GetFloorIntY());
			}
			if (HasBGColorBitmap()) {
				draw_sprite(terrainBGBitmap, m_BGColorBitmap, posOnScene.GetFloorIntX() + terrainBGBitmap->w, posOnScene.GetFloorIntY());
			}
			if (HasFGColorBitmap()) {
				draw_sprite(terrainFGBitmap, m_FGColorBitmap, posOnScene.GetFloorIntX() + terrainFGBitmap->w, posOnScene.GetFloorIntY());
			}
		} else if (posOnScene.GetFloorIntX() >= terrainMatBitmap->w - GetBitmapWidth()) {
			if (HasMaterialBitmap()) {
				draw_sprite(terrainMatBitmap, m_MaterialBitmap, posOnScene.GetFloorIntX() - terrainMatBitmap->w, posOnScene.GetFloorIntY());
			}
			if (HasBGColorBitmap()) {
				draw_sprite(terrainBGBitmap, m_BGColorBitmap, posOnScene.GetFloorIntX() - terrainBGBitmap->w, posOnScene.GetFloorIntY());
			}
			if (HasFGColorBitmap()) {
				draw_sprite(terrainFGBitmap, m_FGColorBitmap, posOnScene.GetFloorIntX() - terrainFGBitmap->w, posOnScene.GetFloorIntY());
			}
		}
	}
	if (terrain->WrapsY()) {
		if (posOnScene.GetFloorIntY() < 0) {
			if (HasMaterialBitmap()) {
				draw_sprite(terrainMatBitmap, m_MaterialBitmap, posOnScene.GetFloorIntX(), posOnScene.GetFloorIntY() + terrainMatBitmap->h);
			}
			if (HasBGColorBitmap()) {
				draw_sprite(terrainBGBitmap, m_BGColorBitmap, posOnScene.GetFloorIntX(), posOnScene.GetFloorIntY() + terrainBGBitmap->h);
			}
			if (HasFGColorBitmap()) {
				draw_sprite(terrainFGBitmap, m_FGColorBitmap, posOnScene.GetFloorIntX(), posOnScene.GetFloorIntY() + terrainFGBitmap->h);
			}
		} else if (posOnScene.GetFloorIntY() >= terrainMatBitmap->h - GetBitmapHeight()) {
			if (HasMaterialBitmap()) {
				draw_sprite(terrainMatBitmap, m_MaterialBitmap, posOnScene.GetFloorIntX(), posOnScene.GetFloorIntY() - terrainMatBitmap->h);
			}
			if (HasBGColorBitmap()) {
				draw_sprite(terrainBGBitmap, m_BGColorBitmap, posOnScene.GetFloorIntX(), posOnScene.GetFloorIntY() - terrainBGBitmap->h);
			}
			if (HasFGColorBitmap()) {
				draw_sprite(terrainFGBitmap, m_FGColorBitmap, posOnScene.GetFloorIntX(), posOnScene.GetFloorIntY() - terrainFGBitmap->h);
			}
		}
	}
	if (HasMaterialBitmap()) {
		draw_sprite(terrainMatBitmap, m_MaterialBitmap, posOnScene.GetFloorIntX(), posOnScene.GetFloorIntY());
		terrain->AddUpdatedMaterialArea(Box(posOnScene, static_cast<float>(m_MaterialBitmap->w), static_cast<float>(m_MaterialBitmap->h)));
	}
	if (HasBGColorBitmap()) {
		draw_sprite(terrainBGBitmap, m_BGColorBitmap, posOnScene.GetFloorIntX(), posOnScene.GetFloorIntY());
		g_SceneMan.RegisterTerrainChange(posOnScene.GetFloorIntX(), posOnScene.GetFloorIntY(), m_BGColorBitmap->w, m_BGColorBitmap->h, ColorKeys::g_MaskColor, true);
	}
	if (HasFGColorBitmap()) {
		draw_sprite(terrainFGBitmap, m_FGColorBitmap, posOnScene.GetFloorIntX(), posOnScene.GetFloorIntY());
		g_SceneMan.RegisterTerrainChange(posOnScene.GetFloorIntX(), posOnScene.GetFloorIntY(), m_FGColorBitmap->w, m_FGColorBitmap->h, ColorKeys::g_MaskColor, false);
	}
}
