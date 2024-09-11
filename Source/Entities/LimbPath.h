#pragma once

/// Header file for the LimbPath class.
/// @author Daniel Tabar
/// data@datarealms.com
/// http://www.datarealms.com
/// Inclusions of header files
#include "Entity.h"
#include "Vector.h"
#include "ActivityMan.h"
#include "Atom.h"

namespace RTE {

#define SPEEDCOUNT 3

	enum Speed {
		SLOW = 0,
		NORMAL,
		FAST
	};

	/// A set of Vector:s making up a motion path for a AtomGroup's limb. The
	/// path is continuous.
	class LimbPath : public Entity {

		/// Public member variable, method and friend function declarations
	public:
		// Concrete allocation and cloning definitions
		EntityAllocation(LimbPath);
		SerializableOverrideMethods;
		ClassInfoGetters;

		/// Constructor method used to instantiate a LimbPath object in system
		/// memory. Create() should be called before using the object.
		LimbPath();

		/// Destructor method used to clean up a LimbPath object before deletion
		/// from system memory.
		~LimbPath() override;

		/// Makes the LimbPath object ready for use.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create() override;

		/*
		/// Makes the LimbPath object ready for use.
		/// @param startPoint A Vector specifying starting point of this LimbPath, relative
		/// to the owning RTEActor's origin.
		/// @param segCount An int specifying how many segments there are in the following (default: 1)
		/// segment array. This MUST match the actual size of the array!
		/// @param aSegArray An array of Vectors that hold the desired path segments to use. (default: new Vector)
		/// @param travelSpeed A float specifying the constant travel speed the limb traveling this (default: 1.0)
		/// LimbPath should have, in m/s.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		    int Create(const Vector &startPoint,
		                       const unsigned int segCount = 1,
		                       const Vector *aSegArray = new Vector,
		                       const float travelSpeed = 1.0);
		*/

		/// Creates a LimbPath to be identical to another, by deep copy.
		/// @param reference A reference to the LimbPath to deep copy.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create(const LimbPath& reference);

		/// Resets the entire LimbPath, including its inherited members, to their
		/// default settings or values.
		void Reset() override {
			Clear();
			Entity::Reset();
		}

		/// Destroys and resets (through Clear()) the LimbPath object.
		/// @param notInherited Whether to only destroy the members defined in this derived class, or (default: false)
		/// to destroy all inherited members also.
		void Destroy(bool notInherited = false) override;

		/// Gets the coordinates where the limb should start at the start of the LimbPath cycle, relative to the owning AtomGroup's local origin.
		/// @return A Vector with the start position.
		const Vector& GetStartOffset() const { return m_Start; }

		/// Sets the coordinates where the limb should start at the start of the LimbPath cycle, relative to the owning AtomGroup's local origin.
		/// @param newStartOffset A Vector with the new start offset.
		void SetStartOffset(const Vector& newStartOffset) { m_Start = newStartOffset; }

		/// Gets the number of Vector:s the internal array of 'waypoints' or
		/// segments of this LimbPath.
		/// @return An int with he count.
		int GetSegCount() const { return m_Segments.size(); }

		/// Gets a pointer to the segment at the given index. Ownership is NOT transferred.
		/// @param segmentIndex The index of the segment to get.
		/// @return A pointer to the segment at the given index. Ownership is NOT transferred.
		Vector* GetSegment(int segmentIndex) {
			if (segmentIndex < static_cast<int>(m_Segments.size())) {
				return &m_Segments.at(segmentIndex);
			}
			return nullptr;
		}

		/// Gets whether or not foot collisions should be disabled, i.e. the limbpath's progress is greater than the FootCollisionsDisabledSegment value.
		/// @return Whether or not foot collisions should be disabled for this limbpath at its current progress.
		bool FootCollisionsShouldBeDisabled() const { return m_FootCollisionsDisabledSegment >= 0 && GetSegCount() - GetCurrentSegmentNumber() <= m_FootCollisionsDisabledSegment; }

		/// Gets how far the limb was last reported to be away form the current
		/// segment target/waypoint.
		/// @return A normalized float describing the progress made toward the current
		/// segment last frame. 0.5 means it was half the length of the current
		/// segment away from it.
		float GetSegProgress() const { return m_SegProgress; }

		/// Gets the APPROXIMATE scene position that the limb was reported to be
		/// last frame. This really shouldn't be used by external clients.
		/// @return A Vector with the APPROXIAMTE scene/world coordinates of the limb as
		/// reported last.
		Vector GetProgressPos();

		/// Gets the scene/world position target that the current segment represents.
		/// @return A vector with the scene position of the current segment target.
		Vector GetCurrentSegTarget();

		/// Gets the velocity of the current position on the path.
		/// @param limbPos The current world coordinate position of the Limb.
		/// @return A Vector with the current move velocity.
		Vector GetCurrentVel(const Vector& limbPos);

		/// Gets the speed that a limb traveling this LimbPath should have.
		/// @return A float describing the speed in m/s.
		float GetSpeed() const { return m_TravelSpeed[m_WhichSpeed] * m_TravelSpeedMultiplier; }

		/// Gets the speed that a limb traveling this LimbPath should have for the specified preset.
		/// @param speedPreset Predefined speed preset to set the value for.
		/// @return A float describing the speed in m/s.
		float GetSpeed(int speedPreset) const {
			if (speedPreset == SLOW || speedPreset == NORMAL || speedPreset == FAST)
				return m_TravelSpeed[speedPreset];
			else
				return 0;
		}

		/// Sets the current travel speed multiplier.
		/// @param newValue The new travel speed multiplier.
		void SetTravelSpeedMultiplier(float newValue) { m_TravelSpeedMultiplier = newValue; }

		/// Gets the current travel speed multiplier.
		/// @return The current travel speed multiplier.
		float GetTravelSpeedMultiplier() const { return m_TravelSpeedMultiplier; }

		/// Gets the force that a limb traveling this LimbPath can push against
		/// stuff in the scene with. It will increase to the double if progress
		/// isn't made on the segment.
		/// @return The currently set force maximum, in kg * m/s^2.
		float GetPushForce() const { return m_PushForce + (m_PushForce * (m_SegTimer.GetElapsedSimTimeMS() / 500)); }

		/// Gets thedefault, unaltered force that a limb traveling this LimbPath can push against
		/// stuff in the scene with.
		/// @return The default set force maximum, in kg * m/s^2.
		float GetDefaultPushForce() const { return m_PushForce; }

		/// Gets the time needed to get to the target waypoint of the current
		/// segment at the current speed, if there are no obstacles. The chunk
		/// will not exceed the remaining time left on the frame, and will deduct
		/// itself from the remaining frame time tally (originally set by
		/// SetFrameTime()).
		/// @param limbPos The current world coordinate position of the Limb.
		/// @return A float describing the time chunk in seconds.
		float GetNextTimeChunk(const Vector& limbPos);

		/// Gets the total time that this entire path should take to travel along
		/// with the current speed setting, including the start segments.
		/// @return The total time (ms) this should take to travel along, if unobstructed.
		float GetTotalPathTime() const { return ((m_TotalLength * c_MPP) / (m_TravelSpeed[m_WhichSpeed] * m_TravelSpeedMultiplier)) * 1000; }

		/// Gets the total time that this path should take to travel along
		/// with the current speed setting, NOT including the start segments.
		/// @return The total time (ms) this should take to travel along, if unobstructed.
		float GetRegularPathTime() const { return ((m_RegularLength * c_MPP) / (m_TravelSpeed[m_WhichSpeed] * m_TravelSpeedMultiplier)) * 1000; }

		/// Gets the ratio of time since the path was restarted and the total time
		/// it should take to travel along the path with the current speed setting,
		/// including the start segments.
		/// @return A positive scalar ratio showing the progress. 0 - 1.0 and beyond.
		/// If the path has ended, but not been reset, 0 is returned.
		float GetTotalTimeProgress() const { return m_Ended ? 0 : (m_PathTimer.GetElapsedSimTimeMS() / GetTotalPathTime()); }

		/// Gets the ratio of time since the path was restarted and the total time
		/// it should take to travel along the path with the current speed setting,
		/// NOT including the start segments.
		/// @return A positive scalar ratio showing the progress. 0 - 1.0 and beyond.
		/// If the path has ended, but not been reset, 0 is returned.
		float GetRegularTimeProgress() const { return m_Ended ? 0 : (m_PathTimer.GetElapsedSimTimeMS() / GetRegularPathTime()); }

		/// Used to report how much progress was made to getting the limb close to
		/// the target (the current segment waypoint).
		/// @param limbPos The new limb position in world coords.
		void ReportProgress(const Vector& limbPos);

		/// Gets a value representing the total progress that has been made on
		/// this entire path. If the path has ended, 0.0 is returned.
		/// @return A float indicating the total progress made on the entire path, from
		/// 0.0 to 1.0. If the path has ended, 0.0 is returned.
		float GetTotalProgress() const;

		/// Gets a value representing the progress that has been made on the
		/// regular part of this path, ie averythign except the starting segments.
		/// If progress has not been made past the starting segments, < 0 will
		/// be returned. If the path has ended, 0.0 is returned.
		/// @return A float indicating the total progress made on the regular path, from
		/// 0.0 to 1.0. If the path has ended, 0.0 is returned.
		float GetRegularProgress() const;

		/// Gets the current segment as a number, rather than an iterator.
		/// @return The current segment as a number.
		int GetCurrentSegmentNumber() const;

		/// Sets a new array of 'waypoints' or segments of this LimbPath.
		/// @param newSpeed An int specifying how many segments there are in the following
		/// segment array. This MUST match the actual size of the array!
		/// A pointer to the new Vector array.
		//    void SetSegments(const unsigned int segCount, const Vector *newSegments);

		/// Sets the current seg pointer to whichever segment in the segment deque.
		/// @param newSpeed An int that is an index to a valid element of the internal segment array.
		//    void SetCurrentSeg(unsigned int currentSeg) { m_CurrentSegment = currentSeg; }

		/// Sets the speed that a limb traveling this LimbPath should have to one
		/// of the three predefined speed settings.
		/// @param newSpeed An int specifying which discrete speed setting to use from the Speed
		/// enumeration.
		void SetSpeed(int newSpeed);

		/// Sets the speed that a limb traveling this LimbPath with the specified preset should have.
		/// @param speedPreset An int specifying which discrete speed setting to use from the Speed
		/// enumeration. New limb travel speed value.
		void OverrideSpeed(int speedPreset, float newSpeed);

		/// Sets the force that a limb traveling this LimbPath can push against
		/// stuff in the scene with.
		/// @param newForce The new push force maximum, in kg * m/s^2.
		void OverridePushForce(float newForce) { m_PushForce = newForce; };

		/// Sets the amount of time that will be used by the limb to travel every
		/// frame. Defined in seconds.
		/// @param newFrameTime A float describing the time in s.
		void SetFrameTime(float newFrameTime) { m_TimeLeft = newFrameTime; }

		/// Sets whether this path is flipped horizontally or not. If being
		/// flipped the path automatically restarts.
		/// @param hflipped A bool telling this path to be flipped or not.
		void SetHFlip(bool hflipped) { m_HFlipped = hflipped; }

		/// Gets the h flip.
		/// @return The h flip.
		bool GetHFlip() { return m_HFlipped; }

		/// Informs this LimbPath of the absolute world coordinates of its owning
		/// Actor's limb's joint for this frame. Needs to be done before
		/// travelling anyhting along this path each frame.
		/// @param jointPos A Vector with the updated joint position info.
		void SetJointPos(const Vector& jointPos) { m_JointPos = jointPos; }

		/// Informs this LimbPath of the current velocity  of its owning Actor's
		/// limb's joint for this frame. Needs to be done before travelling
		/// anyhting along this path each frame.
		/// @param jointVel A Vector with the updated joint velocity info.
		void SetJointVel(const Vector& jointVel) { m_JointVel = jointVel; }

		/// Informs this LimbPath of the current rotation of its owning Actor's
		/// for this frame. Needs to be done before travelling
		/// anything along this path each frame.
		/// @param rotation A Matrix with the updated rotation info.
		void SetRotation(const Matrix& rotation) {
			m_Rotation = rotation;
			m_Rotation.SetXFlipped(m_HFlipped);
		}

		/// Sets the new rotation offset.
		/// @param rotationOffset The new rotation offset, in local space.
		void SetRotationOffset(const Vector& rotationOffset) { m_RotationOffset = rotationOffset; }

		/// Sets the new position offset.
		/// @param rotationOffset The new position offset, in local space.
		void SetPositionOffset(const Vector& positionOffset) { m_PositionOffset = positionOffset; }

		/// Returns if GetNextMoveVec() have to be called again or not on this
		/// frame. If the last call didn't use up all the time moving on the
		/// current segment because it ended, this will return false. Then
		/// GetNextMoveVec() needs to be called at least one more time this frame.
		/// @return A bool with the answer.
		bool FrameDone() const { return m_TimeLeft <= 0; }

		/// Indicates whether the last call to ProgressMade() completed the
		/// entire path. Use Restart() to start the path over.
		/// @return A bool with the answer.
		bool PathEnded() const { return m_Ended; }

		/// Indicates whether the path has been restarted without making any
		/// progress yet.
		/// @return A bool with the answer.
		bool PathIsAtStart() const { return m_CurrentSegment == m_Segments.begin() && m_SegProgress == 0; }

		/// Sets this LimbPath's progress to its end.
		void Terminate();

		/// Restarts the position tracking of the limb that travels along this
		/// LimbPath.
		void Restart();

		/// Restarts the position tracking of the limb that travels along this
		/// LimbPath at a point which does not contain terrain. In doing this,
		/// a list of potential starting segments are checked and the first to
		/// yield a starting position that is not in terrain will be picked.
		/// If none of the candidate starting segments are free of terrain,
		/// the last one in the list will be picked and false will be returned
		/// here. The passed in limbPos Vector will be set to teh new position of
		/// the restarted path, if a free spot is found.
		/// @param limbPos Limb scene pos which will be set to the new reset position if a free
		/// spot was found.
		/// @param MOIDToIgnore The root MOID to ignore when looking for a free position. (default: g_NoMOID)
		/// @param ignoreTeam To enable ignoring of all MOIDs associated with an object of a specific (default: Activity::NoTeam)
		/// team which also has team ignoring enabled itself.
		/// @return Whether a starting segment that yielded a starting pos free of terrain
		/// was found or not.
		bool RestartFree(Vector& limbPos, MOID MOIDToIgnore = g_NoMOID, int ignoreTeam = Activity::NoTeam);

		/// Indicated whether this path is has been created and had some data set
		/// yet.
		/// @return Whether this has been Create:ed yet.
		bool IsInitialized() const { return !m_Start.IsZero() || !m_Segments.empty(); }

		/// Indicated whether this path is in fact just a single point to where
		/// the limb will always be ordered to move toward. IE a standing still
		/// type limb movement.
		bool IsStaticPoint() const { return m_Segments.empty(); }

		/// Returns the lowest y position of this LimbPath.
		/// @return The lowest y position of this LimbPath.
		float GetLowestY() const;

		/// Returns the middle x position of this LimbPath.
		/// @return The middle x position of this LimbPath.
		float GetMiddleX() const;

		/// Draws this LimbPath's current graphical debug representation to a
		/// BITMAP of choice.
		/// @param pTargetBitmap A pointer to a BITMAP to draw on.
		/// @param targetPos The absolute position of the target bitmap's upper left corner in the Scene. (default: Vector())
		/// @param color The color to draw the path's pixels as. (default: 34)
		void Draw(BITMAP* pTargetBitmap, const Vector& targetPos = Vector(), unsigned char color = 34) const;

		/// Protected member variable and method declarations
	protected:
		static Entity::ClassInfo m_sClass;

		// The starting point of the path.
		Vector m_Start;

		// The number of starting segments, counting into the path from its beginning,
		// that upon restart of this path will be tried in reverse order till one which
		// yields a starting position that is clear of terrain is found.
		int m_StartSegCount;

		// Array containing the actual 'waypoints' or segments for the path.
		std::deque<Vector> m_Segments;

		// The iterator to the segment of the path that the limb ended up on the end of
		std::deque<Vector>::iterator m_CurrentSegment;

		int m_FootCollisionsDisabledSegment; //!< The segment after which foot collisions will be disabled for this limbpath, if it's for legs.

		// Normalized measure of how far the limb has progressed toward the
		// current segment's target. 0.0 means its farther away than the
		// magnitude of the entire segment. 0.5 means it's half the mag of the segment
		// away from the target.
		float m_SegProgress;

		// The constant speed that the limb traveling this path has in m/s.
		float m_TravelSpeed[SPEEDCOUNT];

		// The current travel speed multiplier
		float m_TravelSpeedMultiplier;

		// The current speed setting.
		int m_WhichSpeed;

		// The max force that a limb travelling along this path can push.
		// In kg * m/(s^2)
		float m_PushForce;

		// The latest known position of the owning actor's joint in world coordinates.
		Vector m_JointPos;
		// The latest known velocity of the owning actor's joint in world coordinates.
		Vector m_JointVel;
		// The rotation applied to this walkpath.
		Matrix m_Rotation;
		// The point we should be rotated around, in local space.
		Vector m_RotationOffset;
		// The offset to apply to our walkpath position, in local space.
		Vector m_PositionOffset;

		// If GetNextTimeSeg() couldn't use up all frame time because the current segment
		// ended,this var stores the remainder of time that should be used to progress
		// on the next segment during the same frame.
		float m_TimeLeft;

		// Times the amount of sim time spent since the last path traversal was started
		Timer m_PathTimer;
		// Times the amount of sim time spent pursuing the current segment's target.
		Timer m_SegTimer;

		// Total length of this LimbPath, including the alternative starting segments, in pixel units
		float m_TotalLength;
		// Length of this LimbPath, excluding the alternative starting segments.
		float m_RegularLength;
		bool m_SegmentDone;
		bool m_Ended;
		bool m_HFlipped;

		/// Private member variable and method declarations
	private:
		/// Clears all the member variables of this LimbPath, effectively
		/// resetting the members of this abstraction level only.
		void Clear();

		/// Rotates a point to match our rotation and rotation offset.
		/// @param point The point to rotate.
		/// @return The rotated point.
		Vector RotatePoint(const Vector& point) const;
	};

} // namespace RTE
