#include "BunkerAssembly.h"
#include "PresetMan.h"
#include "ADoor.h"
#include "TerrainObject.h"
#include "BunkerAssemblyScheme.h"
#include "Deployment.h"

using namespace RTE;

ConcreteClassInfo(BunkerAssembly, SceneObject, 0);

BunkerAssembly::BunkerAssembly() {
	Clear();
}

BunkerAssembly::~BunkerAssembly() {
	Destroy(true);
}

void BunkerAssembly::Clear() {
	m_FGColorFile.Reset();
	m_MaterialFile.Reset();
	m_BGColorFile.Reset();
	m_FGColorBitmap = 0;
	m_MaterialBitmap = 0;
	m_BGColorBitmap = 0;
	m_BitmapOffset.Reset();
	m_OffsetDefined = false;
	m_ChildObjects.clear();
	m_PlacedObjects.clear();
	m_ParentAssemblyScheme.clear();
	m_pPresentationBitmap = 0;
	m_SymmetricAssembly.clear();
	m_ParentSchemeGroup.clear();
}

int BunkerAssembly::Create() {
	if (TerrainObject::Create() < 0)
		return -1;

	return 0;
}

void BunkerAssembly::AddPlacedObject(SceneObject* pSO) {
	m_PlacedObjects.push_back(pSO);

	// Increase gold value for every bunker module if it's not a deployment
	Deployment* pDeployment = dynamic_cast<Deployment*>(pSO);
	// Set fixed price
	// if (!pDeployment && !pSO->IsInGroup("Bunker Backgrounds"))
	//	m_OzValue += pSO->GetGoldValue();

	// Draw this terrain object to presentaion bitmap
	TerrainObject* pTObject = dynamic_cast<TerrainObject*>(pSO);
	if (pTObject) {
		Vector objectPos = pTObject->GetPos() + pTObject->GetBitmapOffset();

		// Regular drawing
		if (pTObject->HasMaterialBitmap()) {
			draw_sprite(m_MaterialBitmap, pTObject->GetMaterialBitmap(), objectPos.GetFloorIntX(), objectPos.GetFloorIntY());
		}
		if (pTObject->HasBGColorBitmap()) {
			draw_sprite(m_BGColorBitmap, pTObject->GetBGColorBitmap(), objectPos.GetFloorIntX(), objectPos.GetFloorIntY());
			draw_sprite(m_pPresentationBitmap, pTObject->GetBGColorBitmap(), objectPos.GetFloorIntX(), objectPos.GetFloorIntY());
		}
		if (pTObject->HasFGColorBitmap()) {
			draw_sprite(m_FGColorBitmap, pTObject->GetFGColorBitmap(), objectPos.GetFloorIntX(), objectPos.GetFloorIntY());
			draw_sprite(m_pPresentationBitmap, pTObject->GetFGColorBitmap(), objectPos.GetFloorIntX(), objectPos.GetFloorIntY());
		}

		// Read and add all child objects
		pTObject->SetTeam(GetTeam());

		for (const SceneObject::SOPlacer& childObject: pTObject->GetChildObjects()) {
			SceneObject::SOPlacer newPlacer = childObject;
			newPlacer.SetTeam(pTObject->GetTeam());
			// Explicitly set child object's offset, because it will be a part of a bigger 'terrain object'
			newPlacer.SetOffset(newPlacer.GetOffset() + pTObject->GetPos() + m_BitmapOffset);
			m_ChildObjects.push_back(newPlacer);
		}
	}
}

int BunkerAssembly::Create(BunkerAssemblyScheme* pScheme) {
	if (TerrainObject::Create() < 0)
		return -1;

	m_pPresentationBitmap = create_bitmap_ex(8, pScheme->GetBitmapWidth(), pScheme->GetBitmapHeight());
	clear_to_color(m_pPresentationBitmap, g_MaskColor);

	m_FGColorBitmap = create_bitmap_ex(8, pScheme->GetBitmapWidth(), pScheme->GetBitmapHeight());
	clear_to_color(m_FGColorBitmap, g_MaskColor);

	m_MaterialBitmap = create_bitmap_ex(8, pScheme->GetBitmapWidth(), pScheme->GetBitmapHeight());
	clear_to_color(m_MaterialBitmap, g_MaskColor);

	m_BGColorBitmap = create_bitmap_ex(8, pScheme->GetBitmapWidth(), pScheme->GetBitmapHeight());
	clear_to_color(m_BGColorBitmap, g_MaskColor);

	m_BitmapOffset = pScheme->GetBitmapOffset();
	m_ParentAssemblyScheme = pScheme->GetPresetName();
	m_Pos = pScheme->GetPos();

	AddToGroup("Assemblies");
	AddToGroup(m_ParentAssemblyScheme);
	if (pScheme->GetAssemblyGroup().length() > 0) {
		AddToGroup(pScheme->GetAssemblyGroup());
		m_ParentSchemeGroup = pScheme->GetAssemblyGroup();
	}

	return 0;
}

int BunkerAssembly::Create(const BunkerAssembly& reference) {
	TerrainObject::Create(reference);

	for (std::list<SceneObject*>::const_iterator oItr = reference.m_PlacedObjects.begin(); oItr != reference.m_PlacedObjects.end(); ++oItr)
		m_PlacedObjects.push_back(dynamic_cast<SceneObject*>((*oItr)->Clone()));

	m_ParentAssemblyScheme = reference.m_ParentAssemblyScheme;
	m_pPresentationBitmap = reference.m_pPresentationBitmap;
	m_SymmetricAssembly = reference.m_SymmetricAssembly;
	m_ParentSchemeGroup = reference.m_ParentSchemeGroup;

	return 0;
}

int BunkerAssembly::ReadProperty(const std::string_view& propName, Reader& reader) {
	StartPropertyList(return SceneObject::ReadProperty(propName, reader));

	// Ignore TerrainObject's specific properties, but don't let parent class process them
	MatchProperty("FGColorFile", {});
	MatchProperty("MaterialFile", {});
	MatchProperty("BGColorFile", {});
	MatchProperty("BitmapOffset", {});
	MatchProperty("Location", {});
	MatchProperty("AddChildObject", {});

	MatchProperty("SymmetricAssembly",
	              {
		              reader >> m_SymmetricAssembly;
	              });
	MatchProperty("PlaceObject",
	              {
		              SceneObject* pSO = dynamic_cast<SceneObject*>(g_PresetMan.ReadReflectedPreset(reader));
		              if (pSO)
			              AddPlacedObject(pSO);
	              });
	MatchProperty("ParentScheme", {
		// Add to group like Entity::ReadProperty does
		std::string parentScheme;
		reader >> parentScheme;
		AddToGroup(parentScheme);
		g_PresetMan.RegisterGroup(parentScheme, reader.GetReadModuleID());

		// Find the scheme's group, read it's dimensions and create presentation bitmap
		const BunkerAssemblyScheme* pScheme = dynamic_cast<const BunkerAssemblyScheme*>(g_PresetMan.GetEntityPreset("BunkerAssemblyScheme", parentScheme, -1));
		if (pScheme) {
			// Calculate fixed scheme price based on the scheme size
			if (pScheme->GetGoldValue() == 0)
				m_OzValue = pScheme->GetArea() * 3;
			else
				pScheme->GetGoldValue();

			// Delete existing bitmaps to avoid leaks if someone adds assembly to multiple groups by mistake
			delete m_pPresentationBitmap;
			m_pPresentationBitmap = create_bitmap_ex(8, pScheme->GetBitmapWidth(), pScheme->GetBitmapHeight());
			clear_to_color(m_pPresentationBitmap, g_MaskColor);

			delete m_FGColorBitmap;
			m_FGColorBitmap = create_bitmap_ex(8, pScheme->GetBitmapWidth(), pScheme->GetBitmapHeight());
			clear_to_color(m_FGColorBitmap, g_MaskColor);

			delete m_MaterialBitmap;
			m_MaterialBitmap = create_bitmap_ex(8, pScheme->GetBitmapWidth(), pScheme->GetBitmapHeight());
			clear_to_color(m_MaterialBitmap, g_MaskColor);

			delete m_BGColorBitmap;
			m_BGColorBitmap = create_bitmap_ex(8, pScheme->GetBitmapWidth(), pScheme->GetBitmapHeight());
			clear_to_color(m_BGColorBitmap, g_MaskColor);

			m_ParentAssemblyScheme = parentScheme;
			m_BitmapOffset = pScheme->GetBitmapOffset();
			if (pScheme->GetAssemblyGroup().length() > 0) {
				AddToGroup(pScheme->GetAssemblyGroup());
				m_ParentSchemeGroup = pScheme->GetAssemblyGroup();
				g_PresetMan.RegisterGroup(pScheme->GetAssemblyGroup(), reader.GetReadModuleID());
			}

			// Also add to Assemblies group
			AddToGroup("Assemblies");
			g_PresetMan.RegisterGroup("Assemblies", reader.GetReadModuleID());
		} else {
			// Do not allow to define assemblies prior to corresponding assembly scheme
			char s[256];
			std::snprintf(s, sizeof(s), "Required BunkerAssemblyScheme '%s%' not found when trying to load BunkerAssembly '%s'! BunkerAssemblySchemes MUST be defined before dependent BunkerAssmeblies.", parentScheme.c_str(), m_PresetName.c_str());
			RTEAbort(s);
		}
	});

	EndPropertyList;
}

int BunkerAssembly::Save(Writer& writer) const {
	SceneObject::Save(writer);

	// Groups are essential for BunkerAssemblies so save them, because entity seem to ignore them
	for (auto itr = m_Groups.begin(); itr != m_Groups.end(); ++itr) {
		if ((*itr) != m_ParentAssemblyScheme && (*itr) != m_ParentSchemeGroup) {
			writer.NewProperty("AddToGroup");
			writer << *itr;
		}
	}
	if (m_SymmetricAssembly.size() > 0) {
		writer.NewProperty("SymmetricAssembly");
		writer << m_SymmetricAssembly;
	}
	if (m_ParentAssemblyScheme.size() > 0) {
		writer.NewProperty("ParentScheme");
		writer << m_ParentAssemblyScheme;
	}

	for (std::list<SceneObject*>::const_iterator oItr = m_PlacedObjects.begin(); oItr != m_PlacedObjects.end(); ++oItr) {
		writer.NewProperty("PlaceObject");
		writer.ObjectStart((*oItr)->GetClassName());
		writer.NewProperty("CopyOf");
		writer << (*oItr)->GetModuleAndPresetName();

		writer.NewProperty("Position");
		writer << (*oItr)->GetPos();

		// Only write certain properties if they are applicable to the type of SceneObject being written
		MOSRotating* pSpriteObj = dynamic_cast<MOSRotating*>(*oItr);
		if (pSpriteObj) {
			writer.NewProperty("HFlipped");
			writer << pSpriteObj->IsHFlipped();
			Actor* pActor = dynamic_cast<Actor*>(pSpriteObj);
			if (pActor) {
				writer.NewProperty("Team");
				writer << pActor->GetTeam();
				// Rotation of doors is important
				ADoor* pDoor = dynamic_cast<ADoor*>(pActor);
				if (pDoor) {
					writer.NewProperty("Rotation");
					writer << pDoor->GetRotMatrix();
				}
			}
		}
		TerrainObject* pTObject = dynamic_cast<TerrainObject*>(*oItr);
		if (pTObject && !pTObject->GetChildObjects().empty()) {
			writer.NewProperty("Team");
			writer << pTObject->GetTeam();
		}
		Deployment* pDeployment = dynamic_cast<Deployment*>(*oItr);
		if (pDeployment) {
			writer.NewProperty("HFlipped");
			writer << pDeployment->IsHFlipped();
		}

		writer.ObjectEnd();
	}

	return 0;
}

void BunkerAssembly::Destroy(bool notInherited) {
	for (std::list<SceneObject*>::iterator oItr = m_PlacedObjects.begin(); oItr != m_PlacedObjects.end(); ++oItr) {
		delete (*oItr);
		*oItr = 0;
	}

	// Probably no need to delete those, as bitmaps are only created when preset is read from file
	// and then they just copy pointers in via Clone()
	// delete m_pPresentationBitmap;
	// m_pPresentationBitmap = 0;

	// delete m_FGColorBitmap;
	// m_FGColorBitmap = 0;

	// delete m_MaterialBitmap;
	// m_MaterialBitmap = 0;

	// delete m_BGColorBitmap;
	// m_BGColorBitmap = 0;

	if (!notInherited)
		SceneObject::Destroy();
	Clear();
}

std::vector<Deployment*> BunkerAssembly::GetDeployments() {
	std::vector<Deployment*> deploymentsList;
	std::vector<Deployment*> candidatesList;

	// Sort objects, brains are added by default, everything else are candidates
	for (std::list<SceneObject*>::const_iterator itr = m_PlacedObjects.begin(); itr != m_PlacedObjects.end(); ++itr) {
		Deployment* pDeployment = dynamic_cast<Deployment*>(*itr);

		if (pDeployment) {
			if (pDeployment->IsInGroup("Brains"))
				deploymentsList.push_back(pDeployment);
			else
				candidatesList.push_back(pDeployment);
		}
	}

	int maxDeployments = 0;

	const BunkerAssemblyScheme* pScheme = dynamic_cast<const BunkerAssemblyScheme*>(g_PresetMan.GetEntityPreset("BunkerAssemblyScheme", m_ParentAssemblyScheme, -1));
	if (pScheme)
		maxDeployments = pScheme->GetMaxDeployments();

	int selected = 0;
	for (int i = 0; i < maxDeployments; i++) {
		if (candidatesList.size() == 0)
			break;

		int selection = RandomNum<int>(0, candidatesList.size() - 1);
		deploymentsList.push_back(candidatesList.at(selection));
		candidatesList.erase(candidatesList.begin() + selection);
	}

	return deploymentsList;
}

bool BunkerAssembly::IsOnScenePoint(Vector& scenePoint) const {
	if (!m_pPresentationBitmap)
		return false;

	Vector bitmapPos = m_Pos + m_BitmapOffset;
	if (WithinBox(scenePoint, bitmapPos, m_pPresentationBitmap->w, m_pPresentationBitmap->h)) {
		// Scene point on the bitmap
		Vector bitmapPoint = scenePoint - bitmapPos;
		if (getpixel(m_pPresentationBitmap, bitmapPoint.m_X, bitmapPoint.m_Y) != g_MaskColor)
			return true;
	}

	return false;
}

void BunkerAssembly::SetTeam(int team) {
	TerrainObject::SetTeam(team);

	// Make sure all the objects to be placed will be of the same team
	for (std::list<SceneObject*>::iterator itr = m_PlacedObjects.begin(); itr != m_PlacedObjects.end(); ++itr)
		(*itr)->SetTeam(team);
}

void BunkerAssembly::Draw(BITMAP* pTargetBitmap, const Vector& targetPos, DrawMode mode, bool onlyPhysical) const {
	if (!m_FGColorBitmap)
		RTEAbort("TerrainObject's bitmaps are null when drawing!");

	// Take care of wrapping situations
	Vector aDrawPos[4];
	aDrawPos[0] = m_Pos + m_BitmapOffset - targetPos;
	int passes = 1;

	// See if need to double draw this across the scene seam if we're being drawn onto a scenewide bitmap
	if (targetPos.IsZero() && g_SceneMan.GetSceneWidth() <= pTargetBitmap->w) {
		if (aDrawPos[0].m_X < m_FGColorBitmap->w) {
			aDrawPos[passes] = aDrawPos[0];
			aDrawPos[passes].m_X += pTargetBitmap->w;
			passes++;
		} else if (aDrawPos[0].m_X > pTargetBitmap->w - m_FGColorBitmap->w) {
			aDrawPos[passes] = aDrawPos[0];
			aDrawPos[passes].m_X -= pTargetBitmap->w;
			passes++;
		}
	}
	// Only screenwide target bitmap, so double draw within the screen if the screen is straddling a scene seam
	else {
		if (g_SceneMan.SceneWrapsX()) {
			int sceneWidth = g_SceneMan.GetSceneWidth();
			if (targetPos.m_X < 0) {
				aDrawPos[passes] = aDrawPos[0];
				aDrawPos[passes].m_X -= sceneWidth;
				passes++;
			}
			if (targetPos.m_X + pTargetBitmap->w > sceneWidth) {
				aDrawPos[passes] = aDrawPos[0];
				aDrawPos[passes].m_X += sceneWidth;
				passes++;
			}
		}
	}

	// Draw all the passes needed
	for (int i = 0; i < passes; ++i) {
		if (mode == g_DrawColor) {
			masked_blit(m_pPresentationBitmap, pTargetBitmap, 0, 0, aDrawPos[i].GetFloorIntX(), aDrawPos[i].GetFloorIntY(), m_BGColorBitmap->w, m_BGColorBitmap->h);
		} else if (mode == g_DrawMaterial) {
			masked_blit(m_MaterialBitmap, pTargetBitmap, 0, 0, aDrawPos[i].GetFloorIntX(), aDrawPos[i].GetFloorIntY(), m_MaterialBitmap->w, m_MaterialBitmap->h);
		} else if (mode == g_DrawTrans) {
			draw_trans_sprite(pTargetBitmap, m_pPresentationBitmap, aDrawPos[i].GetFloorIntX(), aDrawPos[i].GetFloorIntY());
		}
	}
}
