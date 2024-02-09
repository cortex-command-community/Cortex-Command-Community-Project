#include "LimbPath.h"

#include "PresetMan.h"
#include "SLTerrain.h"

#include "PrimitiveMan.h"

using namespace RTE;

ConcreteClassInfo(LimbPath, Entity, 20);

LimbPath::LimbPath() {
	Clear();
}

LimbPath::~LimbPath() {
	Destroy(true);
}

void LimbPath::Clear() {
	m_Start.Reset();
	m_StartSegCount = 0;
	m_Segments.clear();
	//    m_CurrentSegment = 0;
	m_FootCollisionsDisabledSegment = -1;
	m_SegProgress = 0.0;
	for (int i = 0; i < SPEEDCOUNT; ++i) {
		m_TravelSpeed[i] = 0.0;
	}
	m_TravelSpeedMultiplier = 1.0F;
	m_WhichSpeed = NORMAL;
	m_PushForce = 0.0;
	m_JointPos.Reset();
	m_JointVel.Reset();
	m_Rotation.Reset();
	m_RotationOffset.Reset();
	m_PositionOffset.Reset();
	m_TimeLeft = 0.0;
	m_PathTimer.Reset();
	m_SegTimer.Reset();
	m_TotalLength = 0.0;
	m_RegularLength = 0.0;
	m_SegmentDone = false;
	m_Ended = true;
	m_HFlipped = false;
}

int LimbPath::Create() {
	// Read all the properties
	if (Entity::Create() < 0)
		return -1;

	if (m_Segments.size() > 0)
		m_CurrentSegment = m_Segments.begin();
	else
		m_CurrentSegment = m_Segments.end();

	Terminate();

	return 0;
}

/*
int LimbPath::Create(const Vector &startPoint,
                     const unsigned int segCount,
                     const Vector *aSegArray,
                     const float travelSpeed)
{
    m_StartPoint = startPoint;
    m_SegCount = segCount;
    m_TravelSpeed = travelSpeed;

    m_Segments = new Vector[m_SegCount];

    if (aSegArray)
    {
        for (int i = 0; i < m_SegCount; ++i)
            m_Segments[i] = aSegArray[i];
    }

    return 0;
}
*/

int LimbPath::Create(const LimbPath& reference) {
	Entity::Create(reference);

	m_Start = reference.m_Start;
	m_StartSegCount = reference.m_StartSegCount;

	std::deque<Vector>::const_iterator itr;
	for (itr = reference.m_Segments.begin(); itr != reference.m_Segments.end(); ++itr)
		m_Segments.push_back(*itr);

	if (m_Segments.size() > 0)
		m_CurrentSegment = m_Segments.begin();
	else
		m_CurrentSegment = m_Segments.end();

	m_FootCollisionsDisabledSegment = reference.m_FootCollisionsDisabledSegment;

	m_SegProgress = reference.m_SegProgress;
	for (int i = 0; i < SPEEDCOUNT; ++i) {
		m_TravelSpeed[i] = reference.m_TravelSpeed[i];
	}
	m_TravelSpeedMultiplier = reference.m_TravelSpeedMultiplier;
	m_PushForce = reference.m_PushForce;
	m_TimeLeft = reference.m_TimeLeft;
	m_TotalLength = reference.m_TotalLength;
	m_RegularLength = reference.m_RegularLength;
	m_SegmentDone = reference.m_SegmentDone;
	m_HFlipped = reference.m_HFlipped;

	Terminate();

	return 0;
}

int LimbPath::ReadProperty(const std::string_view& propName, Reader& reader) {
	StartPropertyList(Entity::ReadProperty(propName, reader));

	MatchProperty("StartOffset", { reader >> m_Start; });
	MatchProperty("StartSegCount", { reader >> m_StartSegCount; });
	MatchProperty("AddSegment",
	              {
		              Vector segment;
		              reader >> segment;
		              m_Segments.push_back(segment);
		              m_TotalLength += segment.GetMagnitude();
		              if (m_Segments.size() >= m_StartSegCount) {
			              m_RegularLength += segment.GetMagnitude();
		              }
	              });
	MatchProperty("EndSegCount", { reader >> m_FootCollisionsDisabledSegment; });

	MatchProperty("SlowTravelSpeed", {
		reader >> m_TravelSpeed[SLOW];
		// m_TravelSpeed[SLOW] = m_TravelSpeed[SLOW] * 2;
	});
	MatchProperty("NormalTravelSpeed", {
		reader >> m_TravelSpeed[NORMAL];
		// m_TravelSpeed[NORMAL] = m_TravelSpeed[NORMAL] * 2;
	});
	MatchProperty("FastTravelSpeed", {
		reader >> m_TravelSpeed[FAST];
		// m_TravelSpeed[FAST] = m_TravelSpeed[FAST] * 2;
	});
	MatchProperty("TravelSpeedMultiplier", { reader >> m_TravelSpeedMultiplier; });
	MatchProperty("PushForce", {
		reader >> m_PushForce;
		// m_PushForce = m_PushForce / 1.5;
	});

	EndPropertyList;
}

Vector LimbPath::RotatePoint(const Vector& point) const {
	Vector offset = (m_RotationOffset).GetXFlipped(m_HFlipped);
	return (((point - offset) * m_Rotation) + offset) + m_PositionOffset;
}

int LimbPath::Save(Writer& writer) const {
	Entity::Save(writer);

	writer.NewProperty("StartOffset");
	writer << m_Start;
	writer.NewProperty("StartSegCount");
	writer << m_StartSegCount;
	for (std::deque<Vector>::const_iterator itr = m_Segments.begin(); itr != m_Segments.end(); ++itr) {
		writer.NewProperty("AddSegment");
		writer << *itr;
	}
	writer.NewProperty("SlowTravelSpeed");
	writer << m_TravelSpeed[SLOW];
	writer.NewProperty("NormalTravelSpeed");
	writer << m_TravelSpeed[NORMAL];
	writer.NewProperty("FastTravelSpeed");
	writer << m_TravelSpeed[FAST];
	writer.NewProperty("TravelSpeedMultiplier");
	writer << m_TravelSpeedMultiplier;
	writer.NewProperty("PushForce");
	writer << m_PushForce;

	return 0;
}

void LimbPath::Destroy(bool notInherited) {

	if (!notInherited)
		Entity::Destroy();
	Clear();
}

Vector LimbPath::GetProgressPos() {
	Vector returnVec(m_Start);
	if (IsStaticPoint()) {
		return m_JointPos + RotatePoint(returnVec);
	}

	// Add all the segments before the current one
	std::deque<Vector>::const_iterator itr;
	for (itr = m_Segments.begin(); itr != m_CurrentSegment; ++itr) {
		returnVec += *itr;
	}

	// Add any from the progress made on the current one
	if (itr != m_Segments.end()) {
		returnVec += *m_CurrentSegment * m_SegProgress;
	}

	return m_JointPos + RotatePoint(returnVec);
}

Vector LimbPath::GetCurrentSegTarget() {
	Vector returnVec(m_Start);
	if (IsStaticPoint()) {
		return m_JointPos + RotatePoint(returnVec);
	}

	std::deque<Vector>::const_iterator itr;

	for (itr = m_Segments.begin(); itr != m_CurrentSegment; ++itr) {
		returnVec += *itr;
	}

	if (itr != m_Segments.end()) {
		returnVec += *m_CurrentSegment;
	}

	return m_JointPos + RotatePoint(returnVec);
}

Vector LimbPath::GetCurrentVel(const Vector& limbPos) {
	Vector returnVel;
	Vector distVect = g_SceneMan.ShortestDistance(limbPos, GetCurrentSegTarget());
	float adjustedTravelSpeed = (m_TravelSpeed[m_WhichSpeed] / (1.0F + std::abs(m_JointVel.GetY()) * 0.1F)) * m_TravelSpeedMultiplier;

	if (IsStaticPoint()) {
		returnVel = distVect * c_MPP / 0.020 /* + m_JointVel*/;
		returnVel.CapMagnitude(adjustedTravelSpeed);
		returnVel += m_JointVel;

		//        if (distVect.MagnitudeIsLessThan(0.5F))
		//            returnVel *= 0.1;
	} else {
		returnVel.SetXY(adjustedTravelSpeed, 0);

		if (!distVect.IsZero())
			returnVel.AbsRotateTo(distVect);

		returnVel += m_JointVel;
	}

	return returnVel;
}

float LimbPath::GetNextTimeChunk(const Vector& limbPos) {
	float timeChunk;

	if (IsStaticPoint()) {
		// Use all the time to get to the target point.
		timeChunk = m_TimeLeft;
		m_TimeLeft = 0.0;
	} else {
		Vector distance;
		// Figure out the distance, in meters, between the limb position and the target.
		distance = g_SceneMan.ShortestDistance(limbPos, GetCurrentSegTarget()) * c_MPP;
		// Add the distance needed to be traveled due to the joint velocity.
		//        distance += m_JointVel * m_TimeLeft;

		// Figure out the time needed to get to the target at the current speed, if
		// there are no obstacles.
		timeChunk = distance.GetMagnitude() / (GetSpeed() + m_JointVel.GetMagnitude());
		// Cap the time segment off to what we have left, if needed.
		timeChunk = timeChunk > m_TimeLeft ? m_TimeLeft : timeChunk;
		// Deduct the time used to pushtravel from the total time left.
		m_TimeLeft -= timeChunk;
	}

	return timeChunk;
}

void LimbPath::ReportProgress(const Vector& limbPos) {
	if (IsStaticPoint()) {
		const float staticPointEndedThreshold = 1.0F;
		m_Ended = g_SceneMan.ShortestDistance(limbPos, GetCurrentSegTarget()).MagnitudeIsLessThan(staticPointEndedThreshold);
	} else {
		// Check if we are sufficiently close to the target to start going after the next one.
		Vector distVec = g_SceneMan.ShortestDistance(limbPos, GetCurrentSegTarget());
		float distance = distVec.GetMagnitude();
		float segMag = (*(m_CurrentSegment)).GetMagnitude();

		// TODO: Don't hardcode this!")
		const float segmentEndedThreshold = 1.5F;
		if (distance < segmentEndedThreshold) {
			if (++(m_CurrentSegment) == m_Segments.end()) {
				--(m_CurrentSegment);
				// Get normalized progress measure toward the target.
				m_SegProgress = distance > segMag ? 0.0F : (1.0F - (distance / segMag));
				m_Ended = true;
			}
			// Next segment!
			else {
				m_SegProgress = 0.0;
				m_SegTimer.Reset();
				m_Ended = false;
			}
		} else {
			m_SegProgress = distance > segMag ? 0.0F : (1.0F - (distance / segMag));
			m_Ended = false;
		}
	}
}

float LimbPath::GetTotalProgress() const {
	if (m_Ended || IsStaticPoint())
		return 0.0;

	float prog = 0;
	for (std::deque<Vector>::const_iterator itr = m_Segments.begin(); itr != m_CurrentSegment; ++itr)
		prog += itr->GetMagnitude();

	prog += (*(m_CurrentSegment)).GetMagnitude() * m_SegProgress;
	return prog / m_TotalLength;
}

float LimbPath::GetRegularProgress() const {
	if (m_Ended || IsStaticPoint())
		return 0.0;

	float prog = m_RegularLength - m_TotalLength;
	for (std::deque<Vector>::const_iterator itr = m_Segments.begin(); itr != m_CurrentSegment; ++itr)
		prog += itr->GetMagnitude();
	prog += (*(m_CurrentSegment)).GetMagnitude() * m_SegProgress;

	return prog / m_RegularLength;
}

int LimbPath::GetCurrentSegmentNumber() const {
	int progress = 0;
	if (!m_Ended && !IsStaticPoint()) {
		for (std::deque<Vector>::const_iterator itr = m_Segments.begin(); itr != m_CurrentSegment; ++itr) {
			progress++;
		}
	}
	return progress;
}

void LimbPath::SetSpeed(int newSpeed) {
	if (newSpeed <= SLOW)
		m_WhichSpeed = SLOW;
	else if (newSpeed >= FAST)
		m_WhichSpeed = FAST;
	else
		m_WhichSpeed = NORMAL;
}

void LimbPath::OverrideSpeed(int speedPreset, float newSpeed) {
	if (speedPreset == SLOW || speedPreset == FAST || speedPreset == NORMAL) {
		m_TravelSpeed[m_WhichSpeed] = newSpeed;
	}
}

void LimbPath::Terminate() {
	if (IsStaticPoint()) {
		m_Ended = true;
	} else {
		m_CurrentSegment = --(m_Segments.end());
		m_SegProgress = 1.0;
		m_Ended = true;
	}
}

void LimbPath::Restart() {
	m_CurrentSegment = m_Segments.begin();
	m_PathTimer.Reset();
	m_SegTimer.Reset();
	m_SegProgress = 0;
	m_Ended = false;
}

bool LimbPath::RestartFree(Vector& limbPos, MOID MOIDToIgnore, int ignoreTeam) {
	std::deque<Vector>::iterator prevSeg = m_CurrentSegment;
	float prevProg = m_SegProgress;
	m_SegProgress = 0;
	bool found = false;
	float result = 0;

	if (IsStaticPoint()) {
		Vector notUsed;
		Vector targetPos = m_JointPos + RotatePoint(m_Start);
		Vector beginPos = targetPos;
		// TODO: don't hardcode the beginpos
		beginPos.m_Y -= 24;

		result = g_SceneMan.CastObstacleRay(beginPos, targetPos - beginPos, notUsed, limbPos, MOIDToIgnore, ignoreTeam, g_MaterialGrass);

		// Only indicate that we found free position if there were any free pixels encountered
		if (result < 0 || result > 0)
			found = true;
	} else {
		Vector notUsed;

		// Start at the very beginning of the path
		m_CurrentSegment = m_Segments.begin();

		// Find the first start segment that has an obstacle on it
		int i = 0;
		for (; i < m_StartSegCount; ++i) {
			Vector offsetSegment = (*m_CurrentSegment);
			result = g_SceneMan.CastObstacleRay(GetProgressPos(), RotatePoint(offsetSegment), notUsed, limbPos, MOIDToIgnore, ignoreTeam, g_MaterialGrass);

			// If we found an obstacle after the first pixel, report the current segment as the starting one and that there is free space here
			if (result > 0) {
				// Set accurate segment progress
				// TODO: See if this is a good idea, or if we should just set it to 0 and set limbPos to the start of current segment
				m_SegProgress = g_SceneMan.ShortestDistance(GetProgressPos(), limbPos).GetMagnitude() / offsetSegment.GetMagnitude();
				limbPos = GetProgressPos();
				//                m_SegProgress = 0;
				m_Ended = false;
				found = true;
				break;
			}
			// If obstacle was found on first pixel, report last segment as restarting pos, if there was a last segment
			else if (result == 0 && m_CurrentSegment != m_Segments.begin()) {
				// Use last segment
				--(m_CurrentSegment);
				limbPos = GetProgressPos();
				m_SegProgress = 0;
				m_Ended = false;
				found = true;
				break;
			}
			// If obstacle was found on the first pixel of the first segment, then just report that we couldn't find any free space
			else if (result == 0 && m_CurrentSegment == m_Segments.begin()) {
				found = false;
				break;
			}

			// Check next segment, and quit if it's the end
			if (++(m_CurrentSegment) == m_Segments.end()) {
				found = false;
				break;
			}
		}

		// If we couldn't find any obstacles on the starting segments, then set it to the first non-starting seg and report success
		if (!found && i == m_StartSegCount && m_CurrentSegment != m_Segments.end()) {
			limbPos = GetProgressPos();
			m_SegProgress = 0;
			m_Ended = false;
			found = true;
		}
	}

	if (found) {
		m_PathTimer.Reset();
		m_SegTimer.Reset();
		return true;
	}

	// Failed to find free space, so set back to old state
	m_CurrentSegment = prevSeg;
	m_SegProgress = prevProg;
	return false;
}

// Todo - cache this instead of recalculating each time!
float LimbPath::GetLowestY() const {
	float lowestY = m_Start.GetY();
	for (auto itr = m_Segments.begin(); itr != m_Segments.end(); ++itr) {
		lowestY = std::max(itr->GetY(), lowestY);
	}
	return lowestY;
}

// Todo - cache this instead of recalculating each time!
float LimbPath::GetMiddleX() const {
	float lowestX = m_Start.GetX();
	float highestX = m_Start.GetX();
	for (auto itr = m_Segments.begin(); itr != m_Segments.end(); ++itr) {
		lowestX = std::min(itr->GetX(), lowestX);
		highestX = std::max(itr->GetX(), highestX);
	}
	float result = (lowestX + highestX) * 0.5F;
	return m_HFlipped ? -result : result;
}

void LimbPath::Draw(BITMAP* pTargetBitmap,
                    const Vector& targetPos,
                    unsigned char color) const {
	Vector prevPoint = m_Start;
	Vector nextPoint = prevPoint;
	for (std::deque<Vector>::const_iterator itr = m_Segments.begin(); itr != m_Segments.end(); ++itr) {
		nextPoint += *itr;

		Vector prevWorldPosition = m_JointPos + RotatePoint(prevPoint);
		Vector nextWorldPosition = m_JointPos + RotatePoint(nextPoint);
		line(pTargetBitmap, prevWorldPosition.m_X, prevWorldPosition.m_Y, nextWorldPosition.m_X, nextWorldPosition.m_Y, color);

		Vector min(std::min(prevWorldPosition.m_X, nextWorldPosition.m_X), std::min(prevWorldPosition.m_Y, nextWorldPosition.m_Y));
		Vector max(std::max(prevWorldPosition.m_X, nextWorldPosition.m_X), std::max(prevWorldPosition.m_Y, nextWorldPosition.m_Y));
		g_SceneMan.RegisterDrawing(pTargetBitmap, g_NoMOID, min.m_X, max.m_Y, max.m_X, min.m_Y);

		prevPoint += *itr;
	}
}
