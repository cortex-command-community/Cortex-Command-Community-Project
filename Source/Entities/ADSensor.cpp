#include "ADSensor.h"
#include "Actor.h"
#include "SceneMan.h"

using namespace RTE;

const std::string ADSensor::c_ClassName = "Sensor";

ADSensor::ADSensor() {
	Clear();
}

ADSensor::~ADSensor() {
	Destroy();
}

void ADSensor::Clear() {
	m_StartOffset.Reset();
	m_SensorRay.Reset();
	m_Skip = 3;
}

int ADSensor::Create(const ADSensor& reference) {
	m_StartOffset = reference.m_StartOffset;
	m_SensorRay = reference.m_SensorRay;
	m_Skip = reference.m_Skip;

	return 0;
}

int ADSensor::ReadProperty(const std::string_view& propName, Reader& reader) {
	StartPropertyList(return Serializable::ReadProperty(propName, reader));

	MatchProperty("StartOffset", { reader >> m_StartOffset; });
	MatchProperty("SensorRay", { reader >> m_SensorRay; });
	MatchProperty("SkipPixels", { reader >> m_Skip; });

	EndPropertyList;
}

int ADSensor::Save(Writer& writer) const {
	Serializable::Save(writer);

	if (!m_StartOffset.IsZero())
		writer.NewPropertyWithValue("StartOffset", m_StartOffset);
	if (!m_SensorRay.IsZero())
		writer.NewPropertyWithValue("SensorRay", m_SensorRay);
	if (m_Skip != 3)
		writer.NewPropertyWithValue("SkipPixels", m_Skip);

	return 0;
}

HashingData ADSensor::Hash() const {
	HashingData hashData(Serializable::Hash());
	uint64_t& hash = hashData.m_Hash;

	hash ^= m_StartOffset.Hash().m_Hash << 0;
	hash ^= m_SensorRay.Hash().m_Hash << 1;
	hash ^= static_cast<uint64_t>(m_Skip) << 2;

	return hashData;
}

Actor* ADSensor::SenseActor(const Vector& doorPos, const Matrix& doorRot, bool doorHFlipped, MOID ignoreMOID) {
	Actor* sensedActor = 0;
	MOID foundMOID = g_SceneMan.CastMORay(doorPos + m_StartOffset.GetXFlipped(doorHFlipped) * doorRot, m_SensorRay.GetXFlipped(doorHFlipped) * doorRot, ignoreMOID, Activity::NoTeam, 0, true, m_Skip);

	if (foundMOID) {
		sensedActor = dynamic_cast<Actor*>(g_MovableMan.GetMOFromID(g_MovableMan.GetRootMOID(foundMOID)));

		// Reverse the ray direction if the sensed actor was not valid, to see if we hit anything else relevant.
		if (!sensedActor || !sensedActor->IsControllable()) {
			foundMOID = g_SceneMan.CastMORay(doorPos + (m_StartOffset.GetXFlipped(doorHFlipped) + m_SensorRay.GetXFlipped(doorHFlipped)) * doorRot, (-m_SensorRay.GetXFlipped(doorHFlipped)) * doorRot, ignoreMOID, Activity::NoTeam, 0, true, m_Skip);

			if (foundMOID) {
				sensedActor = dynamic_cast<Actor*>(g_MovableMan.GetMOFromID(g_MovableMan.GetRootMOID(foundMOID)));
			}
		}
	}
	return sensedActor;
}
