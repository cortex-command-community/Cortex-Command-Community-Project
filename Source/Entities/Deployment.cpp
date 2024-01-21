#include "Deployment.h"
#include "ModuleMan.h"
#include "PresetMan.h"
#include "MetaMan.h"
#include "ContentFile.h"
#include "Loadout.h"
#include "MetaPlayer.h"
#include "Actor.h"
#include "ACraft.h"
#include "ActivityMan.h"

using namespace RTE;

	ConcreteClassInfo(Deployment, SceneObject, 0);

	std::vector<BITMAP*> Deployment::m_apArrowLeftBitmap;
	std::vector<BITMAP*> Deployment::m_apArrowRightBitmap;

	Deployment::Deployment() {
		Clear();
	}

	Deployment::~Deployment() {
		Destroy(true);
	}

	void Deployment::Clear() {
		m_LoadoutName = "Default";
		m_Icon.Reset();
		m_SpawnRadius = 40;
		m_WalkRadius = 250;
		m_ID = 0;
		m_HFlipped = false;
	}

	int Deployment::Create() {
		if (SceneObject::Create() < 0)
			return -1;

		if (m_apArrowLeftBitmap.empty()) {
			ContentFile("Base.rte/GUIs/DeploymentIcons/ArrowLeft.png").GetAsAnimation(m_apArrowLeftBitmap, 1);
		}
		if (m_apArrowRightBitmap.empty()) {
			ContentFile("Base.rte/GUIs/DeploymentIcons/ArrowRight.png").GetAsAnimation(m_apArrowRightBitmap, 1);
		}

		return 0;
	}

	int Deployment::Create(std::string loadoutName, const Icon& icon, float spawnRadius) {
		m_LoadoutName = loadoutName;
		m_Icon = icon;
		m_SpawnRadius = spawnRadius;
		m_WalkRadius = 250;
		m_ID = 0;
		m_HFlipped = false;

		if (m_apArrowLeftBitmap.empty()) {
			ContentFile("Base.rte/GUIs/DeploymentIcons/ArrowLeft.png").GetAsAnimation(m_apArrowLeftBitmap, 1);
		}
		if (m_apArrowRightBitmap.empty()) {
			ContentFile("Base.rte/GUIs/DeploymentIcons/ArrowRight.png").GetAsAnimation(m_apArrowRightBitmap, 1);
		}

		return 0;
	}

	int Deployment::Create(const Deployment& reference) {
		SceneObject::Create(reference);

		m_LoadoutName = reference.m_LoadoutName;
		m_Icon = reference.m_Icon;
		m_SpawnRadius = reference.m_SpawnRadius;
		m_WalkRadius = reference.m_WalkRadius;
		m_ID = reference.m_ID;
		m_HFlipped = reference.m_HFlipped;

		return 0;
	}

	int Deployment::ReadProperty(const std::string_view& propName, Reader& reader) {
		StartPropertyList(return SceneObject::ReadProperty(propName, reader));

		MatchProperty("LoadoutName", { reader >> m_LoadoutName; });
		MatchProperty("Icon", { reader >> m_Icon; });
		MatchProperty("SpawnRadius", { reader >> m_SpawnRadius; });
		MatchProperty("WalkRadius", { reader >> m_WalkRadius; });
		MatchProperty("ID", { reader >> m_ID; });
		MatchProperty("HFlipped", { reader >> m_HFlipped; });

		EndPropertyList;
	}

	int Deployment::Save(Writer& writer) const {
		SceneObject::Save(writer);

		writer.NewProperty("LoadoutName");
		writer << m_LoadoutName;
		writer.NewProperty("Icon");
		writer << m_Icon;
		writer.NewProperty("SpawnRadius");
		writer << m_SpawnRadius;
		writer.NewProperty("WalkRadius");
		writer << m_WalkRadius;
		writer.NewProperty("HFlipped");
		writer << m_HFlipped;

		return 0;
	}

	void Deployment::Destroy(bool notInherited) {

		if (!notInherited)
			SceneObject::Destroy();
		Clear();
	}

	Actor* Deployment::CreateDeployedActor() {
		float cost;
		return CreateDeployedActor(-1, cost);
	}

	Actor* Deployment::CreateDeployedActor(int player, float& costTally) {
		// The Actor instance we return and pass ownership of
		Actor* pReturnActor = 0;

		// Take metaplayer tech modifiers into account when calculating costs of this Deployment spawn
		int nativeModule = 0;
		float foreignCostMult = 1.0;
		float nativeCostMult = 1.0;
		MetaPlayer* pMetaPlayer = g_MetaMan.GetMetaPlayerOfInGamePlayer(player);
		// Put
		if (g_MetaMan.GameInProgress() && pMetaPlayer) {
			nativeModule = pMetaPlayer->GetNativeTechModule();
			foreignCostMult = pMetaPlayer->GetForeignCostMultiplier();
			nativeCostMult = pMetaPlayer->GetNativeCostMultiplier();
			// Also set the team of this Deployable to match the player's
			m_Team = pMetaPlayer->GetTeam();
		} else {
			GameActivity* activity = dynamic_cast<GameActivity*>(g_ActivityMan.GetActivity());
			if (activity) {
				// Also set the team of this Deployable to match the player's
				// m_Team = activity->GetTeamOfPlayer(player);
				nativeModule = g_ModuleMan.GetModuleID(activity->GetTeamTech(m_Team));
				// Select some random module if player selected all or something else
				if (nativeModule < 0) {
					std::vector<std::string> moduleList;

					for (int moduleID = 0; moduleID < g_ModuleMan.GetTotalModuleCount(); ++moduleID) {
						if (const DataModule* dataModule = g_ModuleMan.GetDataModule(moduleID)) {
							if (dataModule->IsFaction()) {
								moduleList.emplace_back(dataModule->GetFileName());
							}
						}
					}
					int selection = RandomNum<int>(1, moduleList.size() - 1);
					nativeModule = g_ModuleMan.GetModuleID(moduleList.at(selection));
				}
				foreignCostMult = 1.0;
				nativeCostMult = 1.0;
			}
		}

		// Find the Loadout that this Deployment is referring to
		if (const Loadout* pLoadout = dynamic_cast<const Loadout*>(g_PresetMan.GetEntityPreset("Loadout", m_LoadoutName, nativeModule))) {
			if (std::unique_ptr<Actor> rawLoadoutActor = std::unique_ptr<Actor>(pLoadout->CreateFirstActor(nativeModule, foreignCostMult, nativeCostMult, costTally))) {
				rawLoadoutActor->SetPos(m_Pos);
				rawLoadoutActor->SetTeam(m_Team);
				rawLoadoutActor->SetHFlipped(m_HFlipped);
				rawLoadoutActor->SetControllerMode(Controller::CIM_AI);
				rawLoadoutActor->SetAIMode(Actor::AIMODE_SENTRY);
				rawLoadoutActor->SetDeploymentID(m_ID);
				pReturnActor = dynamic_cast<Actor*>(rawLoadoutActor->Clone());
			}
		}

		// PASSING OWNERSHIP
		return pReturnActor;
	}

	SceneObject* Deployment::CreateDeployedObject() {
		float cost;
		return CreateDeployedObject(-1, cost);
	}

	SceneObject* Deployment::CreateDeployedObject(int player, float& costTally) {
		// The Actor instance we return and pass ownership of
		SceneObject* pReturnObject = 0;

		// Take metaplayer tech modifiers into account when calculating costs of this Deployment spawn
		int nativeModule = 0;
		float foreignCostMult = 1.0;
		float nativeCostMult = 1.0;
		MetaPlayer* pMetaPlayer = g_MetaMan.GetMetaPlayerOfInGamePlayer(player);
		if (g_MetaMan.GameInProgress() && pMetaPlayer) {
			nativeModule = pMetaPlayer->GetNativeTechModule();
			foreignCostMult = pMetaPlayer->GetForeignCostMultiplier();
			nativeCostMult = pMetaPlayer->GetNativeCostMultiplier();
			// Also set the team of this Deployable to match the player's
			m_Team = pMetaPlayer->GetTeam();
		} else {
			GameActivity* activity = dynamic_cast<GameActivity*>(g_ActivityMan.GetActivity());
			if (activity) {
				// Also set the team of this Deployable to match the player's
				// m_Team = activity->GetTeamOfPlayer(player);
				nativeModule = g_ModuleMan.GetModuleID(activity->GetTeamTech(m_Team));
				// Select some random module if player selected all or something else
				if (nativeModule < 0) {
					std::vector<std::string> moduleList;

					for (int moduleID = 0; moduleID < g_ModuleMan.GetTotalModuleCount(); ++moduleID) {
						if (const DataModule* dataModule = g_ModuleMan.GetDataModule(moduleID)) {
							if (dataModule->IsFaction()) {
								moduleList.emplace_back(dataModule->GetFileName());
							}
						}
					}
					int selection = RandomNum<int>(1, moduleList.size() - 1);
					nativeModule = g_ModuleMan.GetModuleID(moduleList.at(selection));
				}
				foreignCostMult = 1.0;
				nativeCostMult = 1.0;
			}
		}

		// Find the Loadout that this Deployment is referring to
		const Loadout* pLoadout = dynamic_cast<const Loadout*>(g_PresetMan.GetEntityPreset("Loadout", m_LoadoutName, nativeModule));
		if (pLoadout) {
			// Get the first object in the Loadout
			pReturnObject = pLoadout->CreateFirstDevice(nativeModule, foreignCostMult, nativeCostMult, costTally);
			// Set the position and team etc for the Actor we are prepping to spawn
			pReturnObject->SetPos(m_Pos);
			pReturnObject->SetHFlipped(m_HFlipped);
			pReturnObject->SetTeam(m_Team);
		}

		// PASSING OWNERSHIP
		return pReturnObject;
	}

	bool Deployment::DeploymentBlocked(int player, const std::list<SceneObject*>& existingObjects) {
		bool blocked = false;

		// Take metaplayer tech modifiers into account when calculating costs of this Deployment spawn
		int nativeModule = 0;
		float foreignCostMult = 1.0;
		float nativeCostMult = 1.0;
		MetaPlayer* pMetaPlayer = g_MetaMan.GetMetaPlayerOfInGamePlayer(player);
		if (g_MetaMan.GameInProgress() && pMetaPlayer) {
			nativeModule = pMetaPlayer->GetNativeTechModule();
			foreignCostMult = pMetaPlayer->GetForeignCostMultiplier();
			nativeCostMult = pMetaPlayer->GetNativeCostMultiplier();
		}

		// First try to find an object via ID's, objects with ID can be far enough
		// Go through all already-placed things in the Scene to see if there's anything with matching spawn ID
		if (m_ID) {
			for (std::list<SceneObject*>::const_iterator existingItr = existingObjects.begin(); existingItr != existingObjects.end(); ++existingItr) {
				Actor* pActor = dynamic_cast<Actor*>(*existingItr);
				if (pActor && pActor->GetDeploymentID() == m_ID) {
					// Do ghetto distance calc between the thing we want to place and the similar thing we found already placed
					// Note this doesn't take into account Scene wrapping, which is problematic when the Scene might not be loaded.. it's okay in this case though
					Vector distance = (*existingItr)->GetPos() - m_Pos;
					// If the same thing is within the spawn radius, then signal that this Deployment location is indeed BLOCKED
					if (distance.MagnitudeIsLessThan(m_WalkRadius)) {
						blocked = true;
						break;
					}
				}
			}
		}

		// No need to do expensive search if it's already blocked
		if (blocked)
			return blocked;

		// The object we would spawn by this Deployment, IF it's not blocked by somehting already
		const SceneObject* pSpawnObject = 0;

		// Find the Loadout that this Deployment is referring to
		Loadout* pLoadout = dynamic_cast<Loadout*>(g_PresetMan.GetEntityPreset("Loadout", m_LoadoutName, nativeModule)->Clone());
		if (pLoadout) {
			// Now go through the Loadout list of items and tally the cost of all devices that would go into inventory of the first Actor found in the list
			const std::list<const SceneObject*>* pMOList = pLoadout->GetCargoList();
			if (pMOList && !pMOList->empty()) {
				// Go through the list of things ordered, and give any actors all the items that is present after them,
				// until the next actor. Also, the first actor gets all stuff in the list above him.
				const MovableObject* pInventoryObject = 0;
				const Actor* pActor = 0;
				for (std::list<const SceneObject*>::const_iterator itr = pMOList->begin(); itr != pMOList->end(); ++itr) {
					// Save pointer of the preset in the list
					pInventoryObject = dynamic_cast<const MovableObject*>(*itr);
					// See if it's actually a passenger, as opposed to a regular item
					pActor = dynamic_cast<const Actor*>(pInventoryObject);
					// If it's an actor, then that's the guy which would be spawned, so use him to check agianst blockage
					if (pActor) {
						pSpawnObject = pActor;
						// We're done looking for the object that would be spawned
						break;
					}
					// If not an Actor, then skip because we're still looking for one
				}
				pActor = 0;

				// If no Actor was found, then see if we can use the delivery Craft instead
				if (!pSpawnObject && pLoadout->GetDeliveryCraft()) {
					// The craft is now the Actor we are looking at spawning
					pSpawnObject = pLoadout->GetDeliveryCraft();
				}

				// If there's no Actor in this Deployment's Loadout at all, then we should just count the first Item or device in the Loadout
				if (!pSpawnObject) {
					// Find the first non-actor
					for (std::list<const SceneObject*>::const_iterator itr = pMOList->begin(); itr != pMOList->end(); ++itr) {
						// If not an Actor, then we should count it and then stop
						if (!dynamic_cast<const Actor*>(*itr)) {
							pSpawnObject = *itr;
							// We're done finding the spawning object
							break;
						}
					}
				}
			}

			// Delete the Loadout instance we have
			delete pLoadout;
			pLoadout = 0;
		}

		// Now check for whether the object that is going to be spawned is blocked by anyhting sufficently similar positioned within the spawn radius
		if (pSpawnObject) {
			// Go through all already-placed things in the Scene to see if there's anything similar/same
			for (std::list<SceneObject*>::const_iterator existingItr = existingObjects.begin(); existingItr != existingObjects.end(); ++existingItr) {
				if (((*existingItr)->GetClassName() == pSpawnObject->GetClassName()) && ((*existingItr)->GetPresetName() == pSpawnObject->GetPresetName())) {
					// Do ghetto distance calc between the thing we want to place and the similar thing we found already placed
					// Note this doesn't take into account Scene wrapping, which is problematic when the Scene might not be loaded.. it's okay in this case though
					Vector distance = (*existingItr)->GetPos() - m_Pos;
					// If the same thing is within the spawn radius, then signal that this Deployment location is indeed BLOCKED
					if (distance.MagnitudeIsLessThan(m_SpawnRadius)) {
						blocked = true;
						break;
					}
				}
			}
		}

		return blocked;
	}

	float Deployment::GetTotalValue(int nativeModule, float foreignMult, float nativeMult) const {
		float totalValue = 0;
		const Actor* pFirstActor = 0;

		// Find the Loadout that this Deployment is referring to
		Loadout* pLoadout = dynamic_cast<Loadout*>(g_PresetMan.GetEntityPreset("Loadout", m_LoadoutName, nativeModule)->Clone());
		if (pLoadout) {
			// Now go through the Loadout list of items and tally the cost ofall devices that would go into inventory of the first Actor found in the list
			const std::list<const SceneObject*>* pMOList = pLoadout->GetCargoList();
			if (pMOList && !pMOList->empty()) {
				// Go through the list of things ordered, and give any actors all the items that is present after them,
				// until the next actor. Also, the first actor gets all stuff in the list above him.
				const MovableObject* pInventoryObject = 0;
				const Actor* pActor = 0;
				std::list<const MovableObject*> cargoItems;
				for (std::list<const SceneObject*>::const_iterator itr = pMOList->begin(); itr != pMOList->end(); ++itr) {
					// Save pointer of the preset in the list
					pInventoryObject = dynamic_cast<const MovableObject*>(*itr);
					// See if it's actually a passenger, as opposed to a regular item
					pActor = dynamic_cast<const Actor*>(pInventoryObject);
					// If it's an actor, then that's the guy which will be spawned
					if (pActor) {
						// Add to the total cost tally
						totalValue += (*itr)->GetGoldValue(nativeModule, foreignMult, nativeMult);
						// If this is the first passenger, then give him all the shit found in the list before him
						if (!pFirstActor) {
							for (std::list<const MovableObject*>::iterator iItr = cargoItems.begin(); iItr != cargoItems.end(); ++iItr)
								totalValue += (*iItr)->GetTotalValue(nativeModule, foreignMult);
						}
						// This isn't the first passenger, so give the previous guy all the stuff that was found since processing him
						else {
							for (std::list<const MovableObject*>::iterator iItr = cargoItems.begin(); iItr != cargoItems.end(); ++iItr)
								totalValue += (*iItr)->GetTotalValue(nativeModule, foreignMult);

							// Also stop going through the list; we only need to count the value of ONE actor and his stuff
							break;
						}
						// Clear out the temporary cargo list since we've assigned all the stuff in it to the return Actor
						cargoItems.clear();

						// Now set the current Actor as the one we found first
						pFirstActor = pActor;
					}
					// If not an Actor, then add it to the temp list of items which will be added to the last passenger's inventory
					else
						cargoItems.push_back(pInventoryObject);
				}
				pActor = 0;

				// If there was a last passenger and only things after him, count the value of all the items in his inventory
				if (pFirstActor) {
					// Passing ownership
					for (std::list<const MovableObject*>::iterator iItr = cargoItems.begin(); iItr != cargoItems.end(); ++iItr)
						totalValue += (*iItr)->GetTotalValue(nativeModule, foreignMult);
					cargoItems.clear();
				}
				// If there wa NO actor in the Loadout's cargo list, then see if there's a craft we can stuff thigns into instead
				else if (pLoadout->GetDeliveryCraft()) {
					// The craft is now the Actor we are counting, so make an instance
					pFirstActor = pLoadout->GetDeliveryCraft();
					// Add the cost of the ship
					totalValue += pFirstActor->GetGoldValue(nativeModule, foreignMult);
					// Count the stuff it would be filled with, passing ownership
					for (std::list<const MovableObject*>::iterator iItr = cargoItems.begin(); iItr != cargoItems.end(); ++iItr)
						totalValue += (*iItr)->GetTotalValue(nativeModule, foreignMult);
					cargoItems.clear();
				}

				// If there's no Actor in this Deployment's Loadout, then we should just count the first Item or device in the Loadout
				if (!pFirstActor) {
					// Start over the count; we might have only had items/devices in the Loadout list, but no Actors yet
					totalValue = 0;
					for (std::list<const SceneObject*>::const_iterator itr = pMOList->begin(); itr != pMOList->end(); ++itr) {
						// If not an Actor, then we should count it and then stop
						if (!dynamic_cast<const Actor*>(*itr)) {
							// Add to the total cost tally
							totalValue += (*itr)->GetGoldValue(nativeModule, foreignMult);
							// We're done
							break;
						}
					}
				}
			}

			// Delete the Loadout instance we have
			delete pLoadout;
			pLoadout = 0;
		}

		return totalValue;
	}

	bool Deployment::IsOnScenePoint(Vector& scenePoint) const {
		if (m_Icon.GetBitmaps8().empty() || !(m_Icon.GetBitmaps8().at(0)))
			return false;
		// TODO: TAKE CARE OF WRAPPING
		/*
		    // Take care of wrapping situations
		    bitmapPos = m_Pos + m_BitmapOffset;
		    Vector aScenePoint[4];
		    aScenePoint[0] = scenePoint;
		    int passes = 1;

		    // See if need to double draw this across the scene seam if we're being drawn onto a scenewide bitmap
		    if (targetPos.IsZero())
		    {
		        if (g_SceneMan.SceneWrapsX())
		        {
		            if (bitmapPos.m_X < m_pFGColor->w)
		            {
		                aScenePoint[passes] = aScenePoint[0];
		                aScenePoint[passes].m_X += g_SceneMan.GetSceneWidth();
		                passes++;
		            }
		            else if (aScenePoint[0].m_X > pTargetBitmap->w - m_pFGColor->w)
		            {
		                aScenePoint[passes] = aScenePoint[0];
		                aScenePoint[passes].m_X -= g_SceneMan.GetSceneWidth();
		                passes++;
		            }
		        }
		        if (g_SceneMan.SceneWrapsY())
		        {

		        }
		    }

		    // Check all the passes needed
		    for (int i = 0; i < passes; ++i)
		    {

		        if (IsWithinBox(aScenePoint[i], m_Pos + m_BitmapOffset, m_pFGColor->w, m_pFGColor->h))
		        {
		            if (getpixel(m_pFGColor, aScenePoint[i].m_X, aScenePoint[i].m_Y) != g_MaskColor ||
		               (m_pBGColor && getpixel(m_pBGColor, aScenePoint[i].m_X, aScenePoint[i].m_Y) != g_MaskColor) ||
		               (m_pMaterial && getpixel(m_pMaterial, aScenePoint[i].m_X, aScenePoint[i].m_Y) != g_MaterialAir))
		               return true;
		        }
		    }
		*/
		BITMAP* pBitmap = m_Icon.GetBitmaps8().at(0);
		Vector bitmapPos = m_Pos - Vector(pBitmap->w / 2, pBitmap->h / 2);
		if (WithinBox(scenePoint, bitmapPos, pBitmap->w, pBitmap->h)) {
			// Scene point on the bitmap
			Vector bitmapPoint = scenePoint - bitmapPos;
			if (getpixel(pBitmap, bitmapPoint.m_X, bitmapPoint.m_Y) != g_MaskColor)
				return true;
		}

		return false;
	}

	void Deployment::Draw(BITMAP* pTargetBitmap, const Vector& targetPos, DrawMode mode, bool onlyPhysical) const {
		if (m_Icon.GetBitmaps8().empty() || !(m_Icon.GetBitmaps8().at(0)))
			RTEAbort("Deployment's Icon bitmaps are null when drawing!");

		if (m_apArrowLeftBitmap.empty() || m_apArrowRightBitmap.empty())
			RTEAbort("Deployment's Arrow bitmaps are null when drawing!");

		{
			BITMAP* pBitmap = m_Icon.GetBitmaps8().at(0);

			// Take care of wrapping situations
			Vector aDrawPos[4];
			aDrawPos[0] = m_Pos - Vector(pBitmap->w / 2, pBitmap->h / 2) - targetPos;
			int passes = 1;

			// See if need to double draw this across the scene seam if we're being drawn onto a scenewide bitmap
			if (targetPos.IsZero()) {
				if (aDrawPos[0].m_X < pBitmap->w) {
					aDrawPos[passes] = aDrawPos[0];
					aDrawPos[passes].m_X += pTargetBitmap->w;
					passes++;
				} else if (aDrawPos[0].m_X > pTargetBitmap->w - pBitmap->w) {
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
					masked_blit(pBitmap, pTargetBitmap, 0, 0, aDrawPos[i].GetFloorIntX(), aDrawPos[i].GetFloorIntY(), pBitmap->w, pBitmap->h);
					// Draw the spawn radius circle too
					circle(pTargetBitmap, aDrawPos[i].GetFloorIntX() + (pBitmap->w / 2), aDrawPos[i].GetFloorIntY() + (pBitmap->h / 2), m_SpawnRadius, c_GUIColorGray);
				} else if (mode == g_DrawTrans) {
					draw_trans_sprite(pTargetBitmap, pBitmap, aDrawPos[i].GetFloorIntX(), aDrawPos[i].GetFloorIntY());
					// Draw the spawn radius circle too
					circle(pTargetBitmap, aDrawPos[i].GetFloorIntX() + (pBitmap->w / 2), aDrawPos[i].GetFloorIntY() + (pBitmap->h / 2), m_SpawnRadius, c_GUIColorGray);
				}
			}
		}

		{
			// Draw direction arrow
			BITMAP* pBitmap = 0;
			Vector offset;
			if (m_HFlipped) {
				pBitmap = m_apArrowLeftBitmap[0];
				offset = Vector(-14, 0);
			} else {
				pBitmap = m_apArrowRightBitmap[0];
				offset = Vector(14, 0);
			}

			// Take care of wrapping situations
			Vector aDrawPos[4];
			aDrawPos[0] = m_Pos - Vector(pBitmap->w / 2, pBitmap->h / 2) - targetPos + offset;
			int passes = 1;

			// See if need to double draw this across the scene seam if we're being drawn onto a scenewide bitmap
			if (targetPos.IsZero()) {
				if (aDrawPos[0].m_X < pBitmap->w) {
					aDrawPos[passes] = aDrawPos[0];
					aDrawPos[passes].m_X += pTargetBitmap->w;
					passes++;
				} else if (aDrawPos[0].m_X > pTargetBitmap->w - pBitmap->w) {
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
					masked_blit(pBitmap, pTargetBitmap, 0, 0, aDrawPos[i].GetFloorIntX(), aDrawPos[i].GetFloorIntY(), pBitmap->w, pBitmap->h);
				} else if (mode == g_DrawTrans) {
					draw_trans_sprite(pTargetBitmap, pBitmap, aDrawPos[i].GetFloorIntX(), aDrawPos[i].GetFloorIntY());
				}
			}
		}
	}
