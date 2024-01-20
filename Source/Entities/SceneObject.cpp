#include "SceneObject.h"
#include "PresetMan.h"
#include "Matrix.h"
#include "ActivityMan.h"
#include "SceneMan.h"

namespace RTE {

	AbstractClassInfo(SceneObject, Entity);
	const std::string SceneObject::SOPlacer::c_ClassName = "SOPlacer";

	void SceneObject::SOPlacer::Clear() {
		m_pObjectReference = 0;
		m_Offset.Reset();
		m_RotAngle = 0;
		m_HFlipped = false;
		m_Team = Activity::NoTeam;
	}

	/*
	//////////////////////////////////////////////////////////////////////////////////////////
	// Virtual method:  Create
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Makes the SOPlacer object ready for use.

	int SceneObject::SOPlacer::Create()
	{
	    if (Serializable::Create() < 0)
	        return -1;

	    return 0;
	}
	*/

	int SceneObject::SOPlacer::Create(const SOPlacer& reference) {
		m_pObjectReference = reference.m_pObjectReference;
		m_Offset = reference.m_Offset;
		m_RotAngle = reference.m_RotAngle;
		m_HFlipped = reference.m_HFlipped;
		m_Team = reference.m_Team;

		return 0;
	}

	int SceneObject::SOPlacer::ReadProperty(const std::string_view& propName, Reader& reader) {
		StartPropertyList(return Serializable::ReadProperty(propName, reader));

		MatchProperty("PlacedObject",
		              {
			              m_pObjectReference = dynamic_cast<const SceneObject*>(g_PresetMan.GetEntityPreset(reader));
			              RTEAssert(m_pObjectReference, "Stream suggests allocating an unallocatable type in SOPlacer::Create!");
		              });
		MatchProperty("Offset", { reader >> m_Offset; });
		MatchProperty("Rotation",
		              {
			              Matrix rot;
			              reader >> rot;
			              m_RotAngle = rot.GetRadAngle();
		              });
		MatchProperty("HFlipped", { reader >> m_HFlipped; });
		MatchProperty("Team", { reader >> m_Team; });

		EndPropertyList;
	}

	int SceneObject::SOPlacer::Save(Writer& writer) const {
		Serializable::Save(writer);

		writer.NewProperty("PlacedObject");
		writer << m_pObjectReference;
		writer.NewProperty("Offset");
		writer << m_Offset;
		// TODO: make generalized way of detecting defaults
		if (m_RotAngle != 0) {
			writer.NewProperty("Rotation");
			Matrix rot;
			rot.SetRadAngle(m_RotAngle);
			writer << rot;
		}
		if (m_HFlipped) {
			writer.NewProperty("HFlipped");
			writer << m_HFlipped;
		}
		if (m_Team >= Activity::TeamOne) {
			writer.NewProperty("Team");
			writer << m_Team;
		}

		return 0;
	}

	SceneObject* SceneObject::SOPlacer::GetPlacedCopy(const SceneObject* pParent) const {
		RTEAssert(m_pObjectReference, "No Object reference to make copy from!");

		SceneObject* pCopy = dynamic_cast<SceneObject*>(m_pObjectReference->Clone());
		if (pCopy) {
			// Make relative to the parent
			if (pParent) {
				// Relative flipping (XOR)
				bool placedFlip = (pParent->IsHFlipped() && m_HFlipped) ? false : (pParent->IsHFlipped() || m_HFlipped);
				pCopy->SetHFlipped(placedFlip);

				// Relative rotation
				Matrix placedRot(pParent->GetRotAngle() + m_RotAngle);
				pCopy->SetRotAngle(placedRot.GetRadAngle());

				// Relative position
				pCopy->SetPos(pParent->GetPos() + m_Offset.GetXFlipped(pParent->IsHFlipped()) * pParent->GetRotMatrix());

				// Relative team (parent overrides, if it has a set team)
				if (pParent->GetTeam() >= 0)
					pCopy->SetTeam(pParent->GetTeam());
				else
					pCopy->SetTeam(m_Team);
			}
			// No parent to make relative from, so apply everything as aboslutes.
			else {
				pCopy->SetHFlipped(m_HFlipped);
				pCopy->SetRotAngle(m_RotAngle); // m_HFlipped ? -m_RotAngle : m_RotAngle);
				pCopy->SetPos(m_Offset);
				pCopy->SetTeam(m_Team);
			}
		}

		// Transfer ownership here
		return pCopy;
	}

	void SceneObject::Clear() {
		m_Pos.Reset();
		m_OzValue = 0;
		m_Buyable = true;
		m_BuyableMode = BuyableMode::NoRestrictions;
		m_Team = Activity::NoTeam;
		m_PlacedByPlayer = Players::NoPlayer;
	}

	/*
	//////////////////////////////////////////////////////////////////////////////////////////
	// Virtual method:  Create
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Makes the SceneObject object ready for use.

	int SceneObject::Create()
	{
	    if (Entity::Create() < 0)
	        return -1;

	    return 0;
	}
	*/
	/*
	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          Create
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Makes the SceneObject object ready for use.

	int SceneObject::Create()
	{

	    return 0;
	}
	*/

	int SceneObject::Create(const SceneObject& reference) {
		Entity::Create(reference);

		m_Pos = reference.m_Pos;
		m_OzValue = reference.m_OzValue;
		m_Buyable = reference.m_Buyable;
		m_BuyableMode = reference.m_BuyableMode;
		m_Team = reference.m_Team;
		m_PlacedByPlayer = reference.m_PlacedByPlayer;

		return 0;
	}

	int SceneObject::ReadProperty(const std::string_view& propName, Reader& reader) {
		StartPropertyList(return Entity::ReadProperty(propName, reader));

		MatchProperty("Position", { reader >> m_Pos; });
		MatchForwards("GoldValue") MatchProperty("GoldCost", { reader >> m_OzValue; });
		MatchProperty("Buyable", { reader >> m_Buyable; });
		MatchProperty("BuyableMode", { m_BuyableMode = static_cast<BuyableMode>(std::stoi(reader.ReadPropValue())); });

		MatchProperty("Team",
		              {
			              reader >> m_Team;
			              // Necessary to properly init (flag icons) some derived classes
			              // (actually, this rarely matters since tehre won't be an activity going when this is read!)
			              SetTeam(m_Team);
		              });
		MatchProperty("PlacedByPlayer", { reader >> m_PlacedByPlayer; });

		EndPropertyList;
	}

	int SceneObject::Save(Writer& writer) const {
		Entity::Save(writer);
		// TODO: Make proper save system that knows not to save redundant data!
		/*
		    writer.NewProperty("Position");
		    writer << m_Pos;
		    writer.NewProperty("GoldValue");
		    writer << m_OzValue;
		    writer.NewProperty("Buyable");
		    writer << m_Buyable;
		    writer.NewProperty("BuyableMode");
		    writer << static_cast<int>(m_BuyableMode);
		    writer.NewProperty("Team");
		    writer << m_Team;
		    writer.NewProperty("PlacedByPlayer");
		    writer << m_PlacedByPlayer;
		*/
		return 0;
	}

	void SceneObject::Destroy(bool notInherited) {

		if (!notInherited)
			Entity::Destroy();
		Clear();
	}

	float SceneObject::GetGoldValue(int nativeModule, float foreignMult, float nativeMult) const {
		// Multiply the value of this according to whether its Tech is native or not to the specified DataModule
		return m_OzValue * ((m_DefinedInModule > 0 && nativeModule > 0 && m_DefinedInModule != nativeModule) ? foreignMult : nativeMult);
	}

	std::string SceneObject::GetGoldValueString(int nativeModule, float foreignMult, float nativeMult) const {
		float subjValue = GetGoldValue(nativeModule, foreignMult, nativeMult);

		char returnString[64];
		if (subjValue != 0) {
			// Just show number since adding oz at the end takes up too much space
			std::snprintf(returnString, sizeof(returnString), "%.0f", subjValue);
		} else
			return "FREE";

		return returnString;
	}

	void SceneObject::DrawTeamMark(BITMAP* pTargetBitmap, const Vector& targetPos) const {
		// Only do HUD if on a team
		if (m_Team < 0)
			return;

		Vector drawPos = m_Pos - targetPos;

		// Adjust the draw position to work if drawn to a target screen bitmap that is straddling a scene seam
		if (!targetPos.IsZero()) {
			// Spans vertical scene seam
			int sceneWidth = g_SceneMan.GetSceneWidth();
			if (g_SceneMan.SceneWrapsX() && pTargetBitmap->w < sceneWidth) {
				if ((targetPos.m_X < 0) && (m_Pos.m_X > (sceneWidth - pTargetBitmap->w)))
					drawPos.m_X -= sceneWidth;
				else if (((targetPos.m_X + pTargetBitmap->w) > sceneWidth) && (m_Pos.m_X < pTargetBitmap->w))
					drawPos.m_X += sceneWidth;
			}
			// Spans horizontal scene seam
			int sceneHeight = g_SceneMan.GetSceneHeight();
			if (g_SceneMan.SceneWrapsY() && pTargetBitmap->h < sceneHeight) {
				if ((targetPos.m_Y < 0) && (m_Pos.m_Y > (sceneHeight - pTargetBitmap->h)))
					drawPos.m_Y -= sceneHeight;
				else if (((targetPos.m_Y + pTargetBitmap->h) > sceneHeight) && (m_Pos.m_Y < pTargetBitmap->h))
					drawPos.m_Y += sceneHeight;
			}
		}

		// Get the Icon bitmaps of this Actor's team, if any
		BITMAP* teamIcon = g_ActivityMan.GetActivity()->GetTeamIcon(m_Team)->GetBitmaps8()[0];

		// Now draw the Icon if we can
		if (teamIcon) {
			// Make team icon blink faster as the health goes down
			masked_blit(teamIcon, pTargetBitmap, 0, 0, drawPos.m_X - teamIcon->h / 2, drawPos.m_Y - teamIcon->h * 2, teamIcon->w, teamIcon->h);
		}
	}

} // namespace RTE
