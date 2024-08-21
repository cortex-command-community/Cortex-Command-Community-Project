#pragma once

#include "Singleton.h"

#include <deque>
#include <chrono>

#define g_TimerMan TimerMan::Instance()

namespace RTE {

	/// The centralized singleton manager of all Timers and overall timekeeping in RTE.
	/// Uses QueryPerformanceCounter for sub-ms resolution timers and the model described in http://www.gaffer.org/game-physics/fix-your-timestep.
	class TimerMan : public Singleton<TimerMan> {

	public:
#pragma region Creation
		/// Constructor method used to instantiate a TimerMan object in system memory. Initialize() should be called before using this object.
		TimerMan() {
			Clear();
			Initialize();
		};

		/// Makes the TimerMan object ready for use.
		void Initialize();
#pragma endregion

#pragma region Destruction
		/// Destroys and resets (through Clear()) the TimerMan object.
		void Destroy() { Clear(); }
#pragma endregion

#pragma region Getters and Setters
		/// Gets the current time stamp in microseconds unrelated to TimerMan updates. Can be used to measure time intervals during a single frame update.
		/// @return Current time stamp in microseconds.
		long long GetAbsoluteTime() const;

		/// Sets the sim to be paused, ie no real time ticks will be transferred to the sim accumulator while this is set to true.
		/// This also clears the accumulator, to avoid the case where the sim may update while paused when behind schedule.
		/// @param pause Whether the sim should be paused or not.
		void PauseSim(bool pause = false) {
			m_SimPaused = pause;
			if (pause)
				m_SimAccumulator = 0.0F;
		}

		/// Tells whether there is enough sim time accumulated to do at least one physics update.
		/// @return Whether there is enough sim time to do a physics update.
		bool TimeForSimUpdate() const { return m_SimAccumulator >= m_DeltaTime; }

		/// Tells whether the current simulation update will be drawn in a frame. Use this to check if it is necessary to draw purely graphical things during the sim update.
		/// @return Whether this is the last sim update before a frame with its results will appear.
		bool DrawnSimUpdate() const { return m_DrawnSimUpdate; }

		/// Tells how many sim updates have been performed since the last one that ended up being a drawn frame.
		/// If negative, it means no sim updates have happened, and a same frame will be drawn again.
		/// @return The number of pure sim updates that have happened since the last drawn.
		int SimUpdatesSinceDrawn() const { return m_SimUpdatesSinceDrawn; }

		/// Gets the simulation speed over real time.
		/// @return The value of the simulation speed over real time.
		float GetSimSpeed() const { return m_SimSpeed; }

		/// Gets a time scale factor which will be used to speed up or slow down the progress of the simulation time in relation to the real world time.
		/// @return A factor between the real world time, and the simulation time.
		float GetTimeScale() const { return m_TimeScale; }

		/// Sets a time scale factor which will be used to speed up or slow down the progress of the simulation time in relation to the real world time.
		/// @param timeScale A factor between the real world time, and the simulation time. A value of 2.0 means simulation runs twice as fast as normal.
		void SetTimeScale(float timeScale = 1.0F) { m_TimeScale = timeScale; }

		/// Gets the cap of the amount of seconds which can be transferred from the real time to the simulated time in one update.
		/// @return A float describing the current cap in seconds.
		float GetRealToSimCap() const;

		/// Gets the number of ticks per second (the resolution of the timer).
		/// @return The number of ticks per second.
		long long GetTicksPerSecond() const { return m_TicksPerSecond; }

		/// Gets a current global real time measured in ticks from the start of the simulation up to the last Update of this TimerMan. Use TickFrequency to determine how many ticks go in a second.
		/// @return The number of ticks passed since the simulation started.
		long long GetRealTickCount() const { return m_RealTimeTicks; }

		/// Gets a current global simulation time measured in ticks from the start of the simulation up to the last Update of this TimerMan. Use TickFrequency to determine how many ticks go in a second.
		/// @return The number of ticks passed since the simulation started.
		long long GetSimTickCount() const { return m_SimTimeTicks; }

		/// Gets a current global simulation time, measured in sim updates, from the start of the simulation up to the last Update of this TimerMan.
		/// @return The number of simulation updates that have occurred since the simulation started.
		long long GetSimUpdateCount() const { return m_SimUpdateCount; }

		/// Gets a current global simulation time measured in ms ticks from the start of the simulation up to the last UpdateSim of this TimerMan.
		/// @return The number of ms passed since the simulation started.
		long long GetSimTimeMS() const { return static_cast<long long>((static_cast<float>(m_SimTimeTicks) / static_cast<float>(m_TicksPerSecond)) * 0.001F); }

		/// Gets the current number of ticks that the simulation should be updating with.
		/// @return The current fixed delta time that the simulation should be updating with, in ticks.
		long long GetDeltaTimeTicks() const { return m_DeltaTime; }

		/// Sets the number of ticks that a simulation update delta time should take.
		/// @param newDelta The new delta time in ticks.
		void SetDeltaTimeTicks(int newDelta) {
			m_DeltaTime = newDelta;
			m_DeltaTimeS = static_cast<float>(m_DeltaTime) / static_cast<float>(m_TicksPerSecond);
		}

		/// Gets the current fixed delta time of the simulation updates, in ms.
		/// @return The current fixed delta time that the simulation should be updating with, in ms.
		float GetDeltaTimeMS() const { return m_DeltaTimeS * 1000; }

		/// Gets the current fixed delta time of the simulation updates, in seconds.
		/// @return The current fixed delta time that the simulation should be updating with, in seconds.
		float GetDeltaTimeSecs() const { return m_DeltaTimeS; }

		/// Gets the current fixed delta time of AI updates, in seconds.
		/// @return The current fixed delta time of AI updates, in seconds.
		float GetAIDeltaTimeSecs() const;

		/// Gets the current fixed delta time of AI updates, in ms.
		/// @return The current fixed delta time of AI updates, in ms.
		float GetAIDeltaTimeMS() const { return GetAIDeltaTimeSecs() * 1000; };

		/// Sets the number of seconds that a simulation update delta time should take.
		/// @param newDelta The new delta time in seconds.
		void SetDeltaTimeSecs(float newDelta) {
			m_DeltaTimeS = newDelta;
			m_DeltaTime = static_cast<long long>(m_DeltaTimeS * static_cast<float>(m_TicksPerSecond));
		}
#pragma endregion

#pragma region Concrete Methods
		/// Resets the measured real and simulation times to 0.
		void ResetTime();

		/// Updates the simulation time to represent the current amount of simulation time passed from the start of the simulation up to the last update.
		void UpdateSim();

		/// Updates the real time ticks based on the actual clock time and adds it to the accumulator which the simulation ticks will draw from in whole DeltaTime-sized chunks.
		void Update();
#pragma endregion

#pragma region Network Handling
		/// Gets the duration the thread should be put to sleep. This is used when ServerSimSleepWhenIdle is true to put the thread to sleep if the sim frame is finished faster than it usually should.
		/// @return The duration the thread should be put to sleep.
		long long GetTimeToSleep() const { return (m_DeltaTime - m_SimAccumulator) / 2; };
#pragma endregion

	protected:
		std::chrono::steady_clock::time_point m_StartTime; //!< The point in real time when the simulation (re)started.
		long long m_TicksPerSecond; //!< The frequency of ticks each second, ie the resolution of the timer.
		long long m_RealTimeTicks; //!< The number of actual microseconds counted so far.
		long long m_SimTimeTicks; //!< The number of simulation time ticks counted so far.
		long long m_SimUpdateCount; //!< The number of whole simulation updates have been made since reset.
		long long m_SimAccumulator; //!< Simulation time accumulator keeps track of how much actual time has passed and is chunked into whole DeltaTime:s upon UpdateSim.

		long long m_DeltaTime; //!< The fixed delta time chunk of the simulation update.
		float m_DeltaTimeS; //!< The simulation update step size, in seconds.
		std::deque<float> m_DeltaBuffer; //!< Buffer for measuring the most recent real time differences, used for averaging out the readings.

		int m_SimUpdatesSinceDrawn; //!< How many sim updates have been done since the last drawn one.
		bool m_DrawnSimUpdate; //!< Tells whether the current simulation update will be drawn in a frame.

		float m_SimSpeed; //!< The simulation speed over real time.
		float m_TimeScale; //!< The relationship between the real world actual time and the simulation time. A value of 2.0 means simulation runs twice as fast as normal, as perceived by a player.

		bool m_SimPaused; //!< Simulation paused; no real time ticks will go to the sim accumulator.

	private:
		/// Clears all the member variables of this TimerMan, effectively resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		TimerMan(const TimerMan& reference) = delete;
		TimerMan& operator=(const TimerMan& rhs) = delete;
	};
} // namespace RTE
