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
		m_TravelSpeed = 0.0;
	}
	m_SegmentEndedThreshold = 2.5F;
	m_BaseTravelSpeedMultiplier = 1.0F;
	m_CurrentTravelSpeedMultiplier = 1.0F;
	m_BaseScaleMultiplier = Vector(1.0F, 1.0F);
	m_CurrentScaleMultiplier = Vector(1.0F, 1.0F);
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
		m_TravelSpeed = reference.m_TravelSpeed;
	}
	m_SegmentEndedThreshold = reference.m_SegmentEndedThreshold;
	m_BaseTravelSpeedMultiplier = reference.m_BaseTravelSpeedMultiplier;
	m_CurrentTravelSpeedMultiplier = reference.m_CurrentTravelSpeedMultiplier;
	m_BaseScaleMultiplier = reference.m_BaseScaleMultiplier;
	m_CurrentScaleMultiplier = reference.m_CurrentScaleMultiplier;
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
	MatchProperty("SegmentEndedThreshold", { reader >> m_SegmentEndedThreshold; });

	// Deprecated, here for backwards compat to avoid crashes
	MatchProperty("SlowTravelSpeed", {
		// We have to put it somewhere or the reader goes wonky
		float discard;
		reader >> discard;
	});
	MatchProperty("NormalTravelSpeed", {
		reader >> m_TravelSpeed;
	});
	MatchProperty("FastTravelSpeed", {
		float discard;
		reader >> discard;
	});
	MatchProperty("TravelSpeed", {
		reader >> m_TravelSpeed;
	});
	MatchProperty("BaseTravelSpeedMultiplier", { reader >> m_BaseTravelSpeedMultiplier; });
	MatchProperty("BaseScaleMultiplier", { reader >> m_BaseScaleMultiplier; });
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

Vector LimbPath::InverseRotatePoint(const Vector& point) const {
	Vector offset = (m_RotationOffset).GetXFlipped(m_HFlipped);
	return (((point - m_PositionOffset) - offset) / m_Rotation) + offset;
}

Vector LimbPath::ToLocalSpace(const Vector& position) const {
	// The position might be on one side of a border of a wrapping scene while the joint is on another.
	// Account for that.
	Vector posWrapped = m_JointPos + g_SceneMan.ShortestDistance(m_JointPos, position);

	return InverseRotatePoint(posWrapped - m_JointPos) / GetTotalScaleMultiplier();
}

Vector LimbPath::ToWorldSpace(const Vector& position) const {
	return m_JointPos + (RotatePoint(position * GetTotalScaleMultiplier()));
}

int LimbPath::Save(Writer& writer) const {
	Entity::Save(writer);

	writer.NewPropertyWithValue("StartOffset", m_Start);
	writer.NewPropertyWithValue("StartSegCount", m_StartSegCount);
	for (std::deque<Vector>::const_iterator itr = m_Segments.begin(); itr != m_Segments.end(); ++itr) {
		writer.NewProperty("AddSegment");
		writer << *itr;
	}
	writer.NewPropertyWithValue("EndSegCount", m_FootCollisionsDisabledSegment);
	writer.NewPropertyWithValue("SegmentEndedThreshold", m_SegmentEndedThreshold);
	
	writer.NewPropertyWithValue("TravelSpeed", m_TravelSpeed);
	writer.NewPropertyWithValue("BaseTravelSpeedMultiplier", m_BaseTravelSpeedMultiplier);
	writer.NewPropertyWithValue("BaseScaleMultiplier", m_BaseScaleMultiplier);
	writer.NewPropertyWithValue("PushForce", m_PushForce);

	return 0;
}

void LimbPath::Destroy(bool notInherited) {

	if (!notInherited)
		Entity::Destroy();
	Clear();
}

Vector LimbPath::GetCurrentSegStartLocal() const {
	Vector returnVec(m_Start);

	if (!IsStaticPoint()) {
		// Add all the segments before the current one
		std::deque<Vector>::const_iterator itr;
		for (itr = m_Segments.begin(); itr != m_CurrentSegment; ++itr) {
			returnVec += *itr;
		}
	}

	return returnVec;
}

Vector LimbPath::GetProgressPos() {
	Vector returnVec = GetCurrentSegStartLocal();

	if (!IsStaticPoint()) {
		if (m_CurrentSegment != m_Segments.end()) {
			// Add approximation based on progress.
			returnVec += *m_CurrentSegment * m_SegProgress;
		}
	}

	return ToWorldSpace(returnVec);
}

Vector LimbPath::GetCurrentSegTarget() {
	Vector returnVec = GetCurrentSegStartLocal();

	if (!IsStaticPoint()) {
		if (m_CurrentSegment != m_Segments.end()) {
			// Add the current one as well.
			returnVec += *m_CurrentSegment;
		}
	}

	return ToWorldSpace(returnVec);
}

Vector LimbPath::GetCurrentVel(const Vector& limbPos) {
	Vector returnVel;
	Vector distVect = g_SceneMan.ShortestDistance(limbPos, GetCurrentSegTarget());
	float adjustedTravelSpeed = (m_TravelSpeed / (1.0F + std::abs(m_JointVel.GetY()) * 0.1F)) * GetTotalTravelSpeedMultiplier();

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
		timeChunk = distance.GetMagnitude() / (GetEffectiveTravelSpeed() + m_JointVel.GetMagnitude());
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
	} else if (m_CurrentSegment == m_Segments.end()) {
		// Current path has already come to an end. Compute progress and m_Ended based on last segment's target.
		Vector distVec = g_SceneMan.ShortestDistance(limbPos, GetCurrentSegTarget());
		float distanceSqr = distVec.GetSqrMagnitude();
		float segMagSqr = (*m_CurrentSegment * GetTotalScaleMultiplier()).GetSqrMagnitude();

		// Get normalized progress measure toward the target.

		if (distanceSqr > segMagSqr)
			// We're too far away from this target.
			m_SegProgress = 0.0;
		else
			m_SegProgress = (1.0F - (std::sqrt(distanceSqr) / std::sqrt(segMagSqr)));

		m_Ended = distVec.MagnitudeIsLessThan(m_SegmentEndedThreshold);
	} else {
		// Presume we're not done with all path segments until proven otherwise.
		m_Ended = false;

		// Check if we are sufficiently close to the current target, or any of the next ones,
		// to start going to whatever target is after that one.
		//
		// The limb might have been yanked and is closer to one of the future targets, so check them too.

		// This rest of the code will be working in local space, so convert input limb pos to that.
		Vector limbPosLocal = ToLocalSpace(limbPos);


		// Iterate over all segments and find one whose target is closest to the limb position.


		// Segment positions are accumulative, so keep an accumulator.
		Vector currentSegmentStartPos = GetCurrentSegStartLocal(); // Will be needed later.
		Vector segmentPosAccumulator = currentSegmentStartPos;

		Vector closestSegmentStartPos;
		float closestSegmentTargetDistanceSqr = std::numeric_limits<float>::max();
		std::deque<Vector>::iterator closestSegment = m_CurrentSegment;

		for (std::deque<Vector>::iterator itr = m_CurrentSegment; itr != m_Segments.end(); ++itr) {

			// We want to find a closest segment to work off of, but we don't want to
			// snap from collision-enabled segments to collision-disabled segments,
			// because doing so tends to produce erratic foot behavior.

			// If the current segment of the limbpath is collision-enabled...
			if (!FootCollisionsShouldBeDisabled()) {
				// If the currently looked at segment is collision-disabled...
				if (m_FootCollisionsDisabledSegment >= 0 &&
						m_Segments.size() - (itr - m_Segments.begin()) <= m_FootCollisionsDisabledSegment) {

					// ...Then break.

					// Note: if the first of the above two checks has passed,
					// this means that the current segment is collision-enabled.
					// And, since this iterator starts with it, this means that
					// *at least* the current segment was picked as closest already.
					//
					// In other words, if this break was hit, then closest segment vars have
					// been properly initialized already. Therefore, it's safe to break.

					break;
				}
			}

			Vector thisSegmentStartPos = segmentPosAccumulator;
			segmentPosAccumulator += *itr; // The accumulator's value is now the target pos of this segment.
			float thisSegmentDistanceSqr = (segmentPosAccumulator - limbPosLocal).GetSqrMagnitude();

			if (thisSegmentDistanceSqr < closestSegmentTargetDistanceSqr) {
				// This one's closer.
				closestSegmentStartPos = thisSegmentStartPos;
				closestSegmentTargetDistanceSqr = thisSegmentDistanceSqr;
				closestSegment = itr;
			}
		}


		// Branches below will determine the new current segment and write the distance to it here.
		// We need this distance to compute progress towards it, whatever it ends up being.
		float distanceToCurrentSegmentTargetSqr;

		if (closestSegmentTargetDistanceSqr < m_SegmentEndedThreshold * m_SegmentEndedThreshold) {
			// We're sufficiently close to this segment's target to go on.
			// Either declare this path ended, or continue from the next segment.

			if (closestSegment + 1 == m_Segments.end()) {
				// Closest segment is the last segment and we are at its target. Declare done.
				m_Ended = true;
				m_CurrentSegment = closestSegment;

				distanceToCurrentSegmentTargetSqr = closestSegmentTargetDistanceSqr;

			} else {
				// Time to switch to next segment!
				m_SegTimer.Reset();

				m_CurrentSegment = closestSegment + 1;

				Vector currentSegmentTarget = closestSegmentStartPos + *closestSegment + *m_CurrentSegment;
				distanceToCurrentSegmentTargetSqr = (currentSegmentTarget - limbPosLocal).GetSqrMagnitude();
			}
		} else {
			// We're not close enough to that closest segment's target, but we can still try to do better.

			Vector currentSegmentTargetPos = currentSegmentStartPos + *m_CurrentSegment;
			float currentSegmentDistanceSqr = (currentSegmentTargetPos - limbPosLocal).GetSqrMagnitude();

			if (closestSegmentTargetDistanceSqr < currentSegmentDistanceSqr) {
				// The target for this closest segment is closer than the current segment's.
				// Fast-forward to it.
				m_SegTimer.Reset();

				m_CurrentSegment = closestSegment;

				distanceToCurrentSegmentTargetSqr = closestSegmentTargetDistanceSqr;
			} else {
				// Just get the distance to current segment's target.
				Vector currentSegmentTarget = currentSegmentStartPos + *m_CurrentSegment;
				distanceToCurrentSegmentTargetSqr = (currentSegmentTarget - limbPosLocal).GetSqrMagnitude();
			}
		}

		// Now compute a normalized progress measure towards the current segment.

		float currentSegmentMagnitudeSqr = m_CurrentSegment->GetSqrMagnitude();

		if (distanceToCurrentSegmentTargetSqr > currentSegmentMagnitudeSqr)
			// We're too far away from this target.
			m_SegProgress = 0.0;
		else
			m_SegProgress = (1.0F - (std::sqrt(distanceToCurrentSegmentTargetSqr) / std::sqrt(currentSegmentMagnitudeSqr)));
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
	if (m_Ended || IsStaticPoint()) {
		return 0;
	} else {
		return m_CurrentSegment - m_Segments.begin();
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
	m_SegProgress = 0.0F;
	bool found = false;
	float result = 0;

	if (IsStaticPoint()) {
		Vector notUsed;
		Vector targetPos = ToWorldSpace(m_Start);
		Vector beginPos = targetPos;
		// TODO: don't hardcode the beginpos
		beginPos.m_Y -= 24;

		result = g_SceneMan.CastObstacleRay(beginPos, targetPos - beginPos, notUsed, limbPos, MOIDToIgnore, ignoreTeam, g_MaterialGrass);

		// Only indicate that we found free position if there were any free pixels encountered
		if (result < 0 || result > 0) {
			found = true;
		}
	} else {
		Vector notUsed;

		// Start at the very beginning of the path
		m_CurrentSegment = m_Segments.begin();

		// Find the first start segment that has an obstacle on it
		size_t i = 0;
		for (; i < m_StartSegCount; ++i) {
			Vector segmentStart = GetProgressPos();
			++m_CurrentSegment;
			Vector segmentEnd = GetProgressPos();
			--m_CurrentSegment;
			Vector currentSegment = segmentEnd - segmentStart;
			result = g_SceneMan.CastObstacleRay(segmentStart, currentSegment, notUsed, limbPos, MOIDToIgnore, ignoreTeam, g_MaterialGrass);

			// If we found an obstacle after the first pixel, report the current segment as the starting one and that there is free space here
			if (result > 0) {
				// Set accurate segment progress
				// TODO: See if this is a good idea, or if we should just set it to 0 and set limbPos to the start of current segment
				m_SegProgress = g_SceneMan.ShortestDistance(GetProgressPos(), limbPos).GetMagnitude() / currentSegment.GetMagnitude();
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

// TODO -  these implementations should be more accurate (segments are additive), but they don't seem to work as well
// Investigate!
/*float LimbPath::GetLowestY() const {
	float currentY = m_Start.GetY();
	float lowestY = currentY;
	for (auto itr = m_Segments.begin() + m_StartSegCount; itr != m_Segments.end(); ++itr) {
		currentY += itr->GetY();
		lowestY = std::min(currentY, lowestY);
	}
	return lowestY * GetTotalScaleMultiplier().GetY();
}

float LimbPath::GetMiddleX() const {
	float currentX = m_Start.GetX();
	float lowestX = currentX;
	float highestX = currentX;
	for (auto itr = m_Segments.begin() + m_StartSegCount; itr != m_Segments.end(); ++itr) {
		currentX += itr->GetX();
		lowestX = std::min(currentX, lowestX);
		highestX = std::max(currentX, highestX);
	}
	lowestX  *= GetTotalScaleMultiplier().GetX();
	highestX *= GetTotalScaleMultiplier().GetX();
	float result = (lowestX + highestX) * 0.5F;
	return m_HFlipped ? -result : result;
}*/

void LimbPath::Draw(BITMAP* pTargetBitmap,
                    const Vector& targetPos,
                    unsigned char color) const {
	Vector prevPoint = m_Start;
	Vector nextPoint = prevPoint;
	for (std::deque<Vector>::const_iterator itr = m_Segments.begin(); itr != m_Segments.end(); ++itr) {
		nextPoint += *itr;

		Vector prevWorldPosition = ToWorldSpace(prevPoint) - targetPos;
		Vector nextWorldPosition = ToWorldSpace(nextPoint) - targetPos;
		line(pTargetBitmap, prevWorldPosition.m_X, prevWorldPosition.m_Y, nextWorldPosition.m_X, nextWorldPosition.m_Y, color);

		Vector min(std::min(prevWorldPosition.m_X, nextWorldPosition.m_X), std::min(prevWorldPosition.m_Y, nextWorldPosition.m_Y));
		Vector max(std::max(prevWorldPosition.m_X, nextWorldPosition.m_X), std::max(prevWorldPosition.m_Y, nextWorldPosition.m_Y));
		g_SceneMan.RegisterDrawing(pTargetBitmap, g_NoMOID, min.m_X, max.m_Y, max.m_X, min.m_Y);

		prevPoint += *itr;
	}
}

