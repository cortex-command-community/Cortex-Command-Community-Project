#include "Magazine.h"
#include "PresetMan.h"
#include "AEmitter.h"

using namespace RTE;

ConcreteClassInfo(Magazine, Attachable, 50);

Magazine::Magazine() {
	Clear();
}

Magazine::~Magazine() {
	Destroy(true);
}

void Magazine::Clear() {
	m_RoundCount = 0;
	m_FullCapacity = 0;
	m_RTTRatio = 0;
	m_pRegularRound = 0;
	m_pTracerRound = 0;
	m_Discardable = true;
	m_AIAimVel = 100;
	m_AIAimMaxDistance = -1;
	m_AIAimPenetration = 0;
	m_AIBlastRadius = -1;

	// NOTE: This special override of a parent class member variable avoids needing an extra variable to avoid overwriting INI values.
	m_CollidesWithTerrainWhileAttached = false;
}

int Magazine::Create() {
	if (Attachable::Create() < 0)
		return -1;

	// Read projectile properties for AI aim caluculations
	const Round* pNextRound = GetNextRound();
	if (pNextRound) {
		// What muzzle velocity should the AI use when aiming?
		m_AIAimVel = pNextRound->GetAIFireVel() < 0 ? pNextRound->GetFireVel() : pNextRound->GetAIFireVel();

		// How much material can this projectile penetrate?
		m_AIAimPenetration = pNextRound->GetAIPenetration();

		if (pNextRound->GetAIFireVel() < 0) {
			const MovableObject* pBullet = pNextRound->GetNextParticle();
			if (pBullet) {
				// Also get FireVel on emitters from sharpness to assure backwards compability with mods
				const AEmitter* pEmitter = dynamic_cast<const AEmitter*>(pBullet);
				if (pEmitter)
					m_AIAimVel = std::max(m_AIAimVel, pEmitter->GetSharpness());
			}
		}
	}

	return 0;
}

int Magazine::Create(const Magazine& reference) {
	Attachable::Create(reference);

	m_RoundCount = reference.m_RoundCount;
	m_FullCapacity = reference.m_FullCapacity;
	m_RTTRatio = reference.m_RTTRatio;
	m_pRegularRound = reference.m_pRegularRound;
	m_pTracerRound = reference.m_pTracerRound;
	m_Discardable = reference.m_Discardable;
	m_AIBlastRadius = reference.m_AIBlastRadius;
	m_AIAimPenetration = reference.m_AIAimPenetration;
	m_AIAimVel = reference.m_AIAimVel;

	return 0;
}

int Magazine::ReadProperty(const std::string_view& propName, Reader& reader) {
	StartPropertyList(return Attachable::ReadProperty(propName, reader));

	MatchProperty("RoundCount", {
		reader >> m_RoundCount;
		m_FullCapacity = m_RoundCount;
	});
	MatchProperty("RTTRatio", { reader >> m_RTTRatio; });
	MatchProperty("RegularRound", { m_pRegularRound = dynamic_cast<const Round*>(g_PresetMan.GetEntityPreset(reader)); });
	MatchProperty("TracerRound", { m_pTracerRound = dynamic_cast<const Round*>(g_PresetMan.GetEntityPreset(reader)); });
	MatchProperty("Discardable", { reader >> m_Discardable; });
	MatchProperty("AIBlastRadius", { reader >> m_AIBlastRadius; });

	EndPropertyList;
}

int Magazine::Save(Writer& writer) const {
	Attachable::Save(writer);

	writer.NewProperty("RoundCount");
	writer << m_RoundCount;
	writer.NewProperty("RTTRatio");
	writer << m_RTTRatio;
	writer.NewProperty("RegularRound");
	writer << m_pRegularRound;
	writer.NewProperty("TracerRound");
	writer << m_pTracerRound;
	writer.NewProperty("Discardable");
	writer << m_Discardable;
	writer.NewProperty("AIBlastRadius");
	writer << m_AIBlastRadius;

	return 0;
}

void Magazine::Destroy(bool notInherited) {

	if (!notInherited)
		Attachable::Destroy();
	Clear();
}

const Round* Magazine::GetNextRound() const {
	const Round* tempRound = 0;
	if (m_RoundCount != 0) {
		if (m_RTTRatio && m_pTracerRound && m_RoundCount % m_RTTRatio == 0)
			tempRound = m_pTracerRound;
		else
			tempRound = m_pRegularRound;
	}
	return tempRound;
}

Round* Magazine::PopNextRound() {
	Round* tempRound = 0;
	if (m_RoundCount != 0) {
		if (m_RTTRatio && m_pTracerRound && m_RoundCount % m_RTTRatio == 0)
			tempRound = dynamic_cast<Round*>(m_pTracerRound->Clone());
		else
			tempRound = dynamic_cast<Round*>(m_pRegularRound->Clone());
		// Negative roundcount means infinite ammo
		if (m_FullCapacity > 0)
			m_RoundCount--;
	}
	return tempRound;
}

float Magazine::EstimateDigStrength() const {
	float maxPenetration = 1;
	if (m_pTracerRound) {
		// Find the next tracer
		const MovableObject* pBullet = m_pTracerRound->GetNextParticle();
		if (pBullet) {
			if (m_pTracerRound->GetAIFireVel() > 0)
				maxPenetration = std::max(maxPenetration, m_pTracerRound->GetAIFireVel() * abs(pBullet->GetMass()) * std::max(pBullet->GetSharpness(), 0.0f));
			else
				maxPenetration = std::max(maxPenetration, m_pTracerRound->GetFireVel() * abs(pBullet->GetMass()) * std::max(pBullet->GetSharpness(), 0.0f));
		}
	}

	if (m_pRegularRound) {
		// Find the next regular bullet
		const MovableObject* pBullet = m_pRegularRound->GetNextParticle();
		if (pBullet) {
			if (m_pRegularRound->GetAIFireVel() > 0)
				maxPenetration = std::max(maxPenetration, m_pRegularRound->GetAIFireVel() * abs(pBullet->GetMass()) * std::max(pBullet->GetSharpness(), 0.0f));
			else
				maxPenetration = std::max(maxPenetration, m_pRegularRound->GetFireVel() * abs(pBullet->GetMass()) * std::max(pBullet->GetSharpness(), 0.0f));
		}
	}

	return maxPenetration;
}

float Magazine::GetBulletAccScalar() {
	const Round* pRound = GetNextRound();
	if (pRound) {
		const MovableObject* pBullet = pRound->GetNextParticle();
		if (pBullet)
			return pBullet->GetGlobalAccScalar();
	}

	return 1;
}

void Magazine::Update() {
	Attachable::Update();

	/*if (!m_pParent) {

	}
	else {
	    /////////////////////////////////
	    // Update rotations and scale

	    // Taken care of by holder/owner Arm.
//        m_Pos += m_ParentOffset;
// Only apply in Draw().
//        m_aSprite->SetAngle(m_Rotation);
//        m_aSprite->SetScale(m_Scale);
	}*/
}

void Magazine::Draw(BITMAP* pTargetBitmap,
                    const Vector& targetPos,
                    DrawMode mode,
                    bool onlyPhysical) const {
	Attachable::Draw(pTargetBitmap, targetPos, mode, onlyPhysical);
}
