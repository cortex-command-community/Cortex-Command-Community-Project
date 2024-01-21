#pragma once

#include "TimerMan.h"

namespace RTE {

	/// A precise timer for FPS sync etc.
	class Timer {

	public:
#pragma region Creation
		/// Constructor method used to instantiate a Timer object.
		Timer() {
			Clear();
			Create();
		}

		/// Constructor method used to instantiate a Timer object with a set sim time elapsed.
		/// @param simTimeLimit A unsigned long defining this Timer's sim time limit in ms.
		Timer(double simTimeLimit) {
			Clear();
			Create(simTimeLimit);
		}

		/// Constructor method used to instantiate a Timer object with a set sim time elapsed.
		/// @param simTimeLimit A unsigned long defining this Timer's sim time limit in ms.
		/// @param elapsedSimTime A unsigned long defining the amount of time (in ms) that this Timer should start with elapsed.
		Timer(double simTimeLimit, double elapsedSimTime) {
			Clear();
			Create(simTimeLimit, elapsedSimTime);
		}

		/// Copy constructor method used to instantiate a Timer object identical to an already existing one.
		/// @param reference A Timer object which is passed in by reference.
		Timer(const Timer& reference) {
			Clear();
			Create(reference);
		}

		/// Makes the Timer object ready for use.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create();

		/// Makes the Timer object ready for use.
		/// @param simTimeLimit A unsigned long defining this Timer's sim time limit in ms.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(double simTimeLimit) { return Create(simTimeLimit, 0); }

		/// Makes the Timer object ready for use.
		/// @param simTimeLimit A unsigned long defining this Timer's sim time limit in ms.
		/// @param elapsedSimTime A unsigned long defining the amount of time (in ms) that this Timer should start with elapsed.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(double simTimeLimit, double elapsedSimTime);

		/// Creates a Timer to be identical to another, by deep copy.
		/// @param reference A reference to the Timer to deep copy.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(const Timer& reference);
#pragma endregion

#pragma region Destruction
		/// Resets the timer so that the elapsed time is 0 ms.
		// TODO: Figure out why calling Clear() here breaks time.
		void Reset() {
			m_StartRealTime = g_TimerMan.GetRealTickCount();
			m_StartSimTime = g_TimerMan.GetSimTickCount();
		}
#pragma endregion

#pragma region Real Time
		/// Gets the start real time value of this Timer.
		/// @return An int64 value that represents the amount of real time in ms from when windows was started to when Reset() of this Timer was called.
		int64_t GetStartRealTimeMS() const { return m_StartRealTime; }

		/// Sets the start real time value of this Timer.
		/// @param newStartTime An int64 with the new real time value (ms since the OS was started).
		void SetStartRealTimeMS(const int64_t newStartTime) { m_StartRealTime = newStartTime * m_TicksPerMS; }

		/// Gets the real time limit value of this Timer, RELATVE to the start time.
		/// This is when the timer is supposed to show that it has 'expired' or reached whatever time limit it is supposed to keep track of.
		/// @return A positive double with the real time limit relative to the start time.
		double GetRealTimeLimitMS() const { return m_RealTimeLimit / m_TicksPerMS; }

		/// Sets the real time limit value of this Timer, RELATVE to the start time.
		/// This is when the timer is supposed to show that it has 'expired' or reached whatever time limit it is supposed to keep track of.
		/// @param newTimeLimit A positive double with the new real time limit relative to the start time.
		void SetRealTimeLimitMS(double newTimeLimit) { m_RealTimeLimit = newTimeLimit * m_TicksPerMS; }

		/// Gets the real time limit value of this Timer, RELATVE to the start time.
		/// This is when the timer is supposed to show that it has 'expired' or reached whatever time limit it is supposed to keep track of.
		/// @return A positive double with the real time limit relative to the start time.
		double GetRealTimeLimitS() const { return m_RealTimeLimit / static_cast<double>(g_TimerMan.GetTicksPerSecond()); }

		/// Sets the real time limit value of this Timer, RELATVE to the start time.
		/// This is when the timer is supposed to show that it has 'expired' or reached whatever time limit it is supposed to keep track of.
		/// @param newTimeLimit A positive double with the new real time limit relative to the start time.
		void SetRealTimeLimitS(double newTimeLimit) { m_RealTimeLimit = newTimeLimit * static_cast<double>(g_TimerMan.GetTicksPerSecond()); }

		/// Gets the elapsed real time in ms since this Timer was Reset().
		/// @return A unsigned long value that represents the elapsed real time since Reset() in ms.
		double GetElapsedRealTimeMS() const { return static_cast<double>(g_TimerMan.GetRealTickCount() - m_StartRealTime) / m_TicksPerMS; }

		/// Sets the start real time value of this Timer, in seconds.
		/// @param newElapsedRealTime An int64 with the new elapsed time value.
		void SetElapsedRealTimeMS(const double newElapsedRealTime) { m_StartRealTime = g_TimerMan.GetRealTickCount() - (newElapsedRealTime * m_TicksPerMS); }

		/// Gets the elapsed real time in seconds since this Timer was Reset().
		/// @return A double value that represents the elapsed real time since Reset() in s.
		double GetElapsedRealTimeS() const { return static_cast<double>(g_TimerMan.GetRealTickCount() - m_StartRealTime) / static_cast<double>(g_TimerMan.GetTicksPerSecond()); }

		/// Sets the start real time value of this Timer.
		/// @param newElapsedRealTime An int64 with the new elapsed time value in seconds.
		void SetElapsedRealTimeS(const double newElapsedRealTime) { m_StartRealTime = g_TimerMan.GetRealTickCount() - (newElapsedRealTime * static_cast<double>(g_TimerMan.GetTicksPerSecond())); }

		/// Returns how much time in ms that there is left till this Timer reaches a certain time limit.
		/// @param when A unsigned long specifying till when there is time.
		/// @return A unsigned long with the time left till the passed in value, or negative if this Timer is already past that point in time.
		unsigned long LeftTillRealMS(int64_t when) { return when - GetElapsedRealTimeMS(); }

		/// Returns true if the elapsed real time is past a certain amount of time relative to this' start.
		/// @param limit A long specifying the threshold amount of real time in ms.
		/// @return A bool only yielding true if the elapsed real time is greater than the passed in value.
		bool IsPastRealMS(long limit) { return GetElapsedRealTimeMS() > limit; }

		/// Returns how much time in ms that there is left till this Timer reaches a certain time limit previously set by SetRealTimeLimitMS.
		/// @return How many MS left till the real time limit, or negative if this Timer is already past that point in time.
		double LeftTillRealTimeLimitMS() { return GetRealTimeLimitMS() - GetElapsedRealTimeMS(); }

		/// Returns how much time in ms that there is left till this Timer reaches a certain time limit previously set by SetRealTimeLimitS.
		/// @return How many S left till the real time limit, or negative if this Timer is already past that point in time.
		double LeftTillRealTimeLimitS() { return GetRealTimeLimitS() - GetElapsedRealTimeS(); }

		/// Returns true if the elapsed real time is past a certain amount of time after the start previously set by SetRealTimeLimit.
		/// @return A bool only yielding true if the elapsed real time is greater than the set limit value. If no limit has been set, this returns false.
		bool IsPastRealTimeLimit() const { return (m_RealTimeLimit == 0) ? true : (m_RealTimeLimit > 0 && (g_TimerMan.GetRealTickCount() - m_StartRealTime) > m_RealTimeLimit); }

		/// Returns how much progress has been made toward the set time limit previously set by SetRealTimeLimitMS.
		/// 0 means no progress, 1.0 means the timer has reached, or is beyond the limit.
		/// @return A normalized scalar between 0.0 - 1.0 showing the progress toward the limit.
		double RealTimeLimitProgress() const { return (m_RealTimeLimit == 0) ? 1.0 : (std::min(1.0, GetElapsedRealTimeMS() / (m_RealTimeLimit / m_TicksPerMS))); }

		/// Returns true or false, depending on whether the elapsed time falls in one of two repeating intervals which divide it.
		/// This is useful for blink animations etc.
		/// @param period An int with the alternating period in ms. The time specified here is how long it will take for the switch to alternate.
		/// @return Whether the elapsed time is in the first state or not.
		bool AlternateReal(int period) const { return (static_cast<int>(GetElapsedRealTimeMS()) % (period * 2)) > period; }
#pragma endregion

#pragma region Simulation Time
		/// Gets the start time value of this Timer.
		/// @return An int64 value that represents the amount of time in ticks from when windows was started to when Reset() of this Timer was called.
		int64_t GetStartSimTimeMS() const { return m_StartSimTime; }

		/// Sets the start time value of this Timer, in ticks
		/// @param newStartTime An int64 with the new time value (ms since windows was started).
		void SetStartSimTimeMS(const int64_t newStartTime) { m_StartSimTime = newStartTime * m_TicksPerMS; }

		/// Sets the sim time limit value of this Timer, RELATVE to the start time.
		/// This is when the timer is supposed to show that it has 'expired' or reached whatever time limit it is supposed to keep track of.
		/// @return A positive double with the sim time limit relative to the start time.
		double GetSimTimeLimitMS() const { return m_SimTimeLimit / m_TicksPerMS; }

		/// Sets the sim time limit value of this Timer, RELATVE to the start time.
		/// This is when the timer is supposed to show that it has 'expired' or reached whatever time limit it is supposed to keep track of.
		/// @param newTimeLimit A positive double with the new sim time limit relative to the start time.
		void SetSimTimeLimitMS(double newTimeLimit) { m_SimTimeLimit = newTimeLimit * m_TicksPerMS; }

		/// Sets the sim time limit value of this Timer, RELATVE to the start time.
		/// This is when the timer is supposed to show that it has 'expired' or reached whatever time limit it is supposed to keep track of.
		/// @return A positive double with the sim time limit relative to the start time.
		double GetSimTimeLimitS() const { return m_SimTimeLimit / static_cast<double>(g_TimerMan.GetTicksPerSecond()); }

		/// Sets the sim time limit value of this Timer, RELATVE to the start time.
		/// This is when the timer is supposed to show that it has 'expired' or reached whatever time limit it is supposed to keep track of.
		/// @param newTimeLimit A positive double with the new sim time limit relative to the start time.
		void SetSimTimeLimitS(double newTimeLimit) { m_SimTimeLimit = newTimeLimit * static_cast<double>(g_TimerMan.GetTicksPerSecond()); }

		/// Gets the elapsed time in ms since this Timer was Reset().
		/// @return A unsigned long value that represents the elapsed time since Reset() in ms.
		double GetElapsedSimTimeMS() const { return static_cast<double>(g_TimerMan.GetSimTickCount() - m_StartSimTime) / m_TicksPerMS; }

		/// Sets the start time value of this Timer, in ms.
		/// @param newElapsedSimTime A double with the new elapsed time value.
		void SetElapsedSimTimeMS(const double newElapsedSimTime) { m_StartSimTime = g_TimerMan.GetSimTickCount() - (newElapsedSimTime * m_TicksPerMS); }

		/// Gets the elapsed time in s since this Timer was Reset().
		/// @return A unsigned long value that represents the elapsed time since Reset() in s.
		double GetElapsedSimTimeS() const { return static_cast<double>(g_TimerMan.GetSimTickCount() - m_StartSimTime) / static_cast<double>(g_TimerMan.GetTicksPerSecond()); }

		/// Sets the start time value of this Timer, in seconds.
		/// @param newElapsedSimTime An int64 with the new elapsed time value in seconds.
		void SetElapsedSimTimeS(const double newElapsedSimTime) { m_StartSimTime = g_TimerMan.GetSimTickCount() - (newElapsedSimTime * static_cast<double>(g_TimerMan.GetTicksPerSecond())); }

		/// Returns how much time in ms that there is left till this Timer reaches a certain time limit.a certain time limit.
		/// @param when A unsigned long specifying till when there is time.
		/// @return A unsigned long with the time left till the passed in value, or negative if this Timer is already past that point in time.
		double LeftTillSimMS(double when) const { return when - GetElapsedSimTimeMS(); }

		/// Returns true if the elapsed time is past a certain amount of time.
		/// @param limit A unsigned long specifying the threshold amount of time in ms.
		/// @return A bool only yielding true if the elapsed time is greater than the passed in value.
		bool IsPastSimMS(double limit) const { return GetElapsedSimTimeMS() > limit; }

		/// Returns how much time in ms that there is left till this Timer reaches a certain time limit previously set by SetSimTimeLimitMS.
		/// @return How many MS left till the sim time limit, or negative if this Timer is already past that point in time.
		double LeftTillSimTimeLimitMS() const { return GetSimTimeLimitMS() - GetElapsedSimTimeMS(); }

		/// Returns how much time in ms that there is left till this Timer reaches a certain time limit previously set by SetSimTimeLimitS.
		/// @return How many S left till the real time limit, or negative if this Timer is already past that point in time.
		double LeftTillSimTimeLimitS() const { return GetSimTimeLimitS() - GetElapsedSimTimeS(); }

		/// Returns true if the elapsed sim time is past a certain amount of time after the start previously set by SetSimTimeLimit.
		/// @return A bool only yielding true if the elapsed real time is greater than the set limit value. If no limit has been set, this returns false.
		bool IsPastSimTimeLimit() const { return (m_SimTimeLimit == 0) ? true : (m_SimTimeLimit > 0 && (g_TimerMan.GetSimTickCount() - m_StartSimTime) > m_SimTimeLimit); }

		/// Returns how much progress has been made toward the set time limit previously set by SetSimTimeLimitMS.
		/// 0 means no progress, 1.0 means the timer has reached, or is beyond the limit.
		/// @return A normalized scalar between 0.0 - 1.0 showing the progress toward the limit.
		double SimTimeLimitProgress() const { return (m_SimTimeLimit == 0) ? 1.0 : (std::min(1.0, GetElapsedSimTimeMS() / (m_SimTimeLimit / m_TicksPerMS))); }

		/// Returns true or false, depending on whether the elapsed time falls in one of two repeating intervals which divide it.
		/// This is useful for blink animations etc.
		/// @param period An int with the alternating period in ms. The time specified here is how long it will take for the switch to alternate.
		/// @return Whether the elapsed time is in the first state or not.
		bool AlternateSim(int period) const { return (period == 0) ? true : (static_cast<int>(GetElapsedSimTimeMS()) % (period * 2)) > period; }
#pragma endregion

	protected:
		double m_TicksPerMS; //!< Ticks per MS.

		int64_t m_StartRealTime; //!< Absolute tick count when this was started in real time.
		int64_t m_RealTimeLimit; //!< Tick count, relative to the start time, when this should indicate end or expired in real time.

		int64_t m_StartSimTime; //!< Absolute tick count when this was started in simulation time.
		int64_t m_SimTimeLimit; //!< Tick count, relative to the start time, when this should indicate end or expired in simulation time.

	private:
		/// Clears all the member variables of this Timer, effectively resetting the members of this abstraction level only.
		void Clear();
	};
} // namespace RTE
