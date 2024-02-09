#pragma once

#include "Singleton.h"
#include "Timer.h"

#include <array>
#include <atomic>
#include <unordered_map>
#include <memory>
#include <vector>
#include <string>

#define g_PerformanceMan PerformanceMan::Instance()

namespace RTE {

	class AllegroBitmap;

	/// Singleton manager responsible for all performance stats counting and drawing.
	class PerformanceMan : public Singleton<PerformanceMan> {
		friend class SettingsMan;

	public:
		/// Enumeration of all available performance counters.
		enum PerformanceCounters {
			SimTotal = 0,
			ActorsAI,
			ActorsTravel,
			ActorsUpdate,
			ParticlesTravel,
			ParticlesUpdate,
			ActivityUpdate,
			ScriptsUpdate,
			PerfCounterCount
		};

		/// Used to store Lua script execution timing information.
		struct ScriptTiming {
			long long m_Time;
			int m_CallCount;
		};

#pragma region Creation
		///  Constructor method used to instantiate a PerformanceMan object in system memory. Create() should be called before using the object.
		PerformanceMan();

		/// Makes the PerformanceMan object ready for use.
		void Initialize();
#pragma endregion

#pragma region Destruction
		/// Destructor method used to clean up a PerformanceMan object before deletion from system memory.
		~PerformanceMan();

		/// Destroys and resets (through Clear()) the PerformanceMan object.
		void Destroy() { Clear(); }
#pragma endregion

#pragma region Getters and Setters
		/// Tells whether to display the performance stats on-screen or not.
		/// @return Whether to show the performance stats or not.
		bool IsShowingPerformanceStats() const { return m_ShowPerfStats; }

		/// Sets whether to display the performance stats on-screen or not.
		/// @param showStats Whether to show the performance stats or not.
		void ShowPerformanceStats(bool showStats = true) { m_ShowPerfStats = showStats; }

		/// Tells whether to display the performance graphs on-screen or not.
		/// @return Whether to show the performance graphs or not.
		bool AdvancedPerformanceStatsEnabled() const { return m_AdvancedPerfStats; }

		/// Sets whether to display the performance graphs on-screen or not.
		/// @param showGraphs Whether to show the performance graphs or not.
		void ShowAdvancedPerformanceStats(bool showGraphs = true) { m_AdvancedPerfStats = showGraphs; }

		/// Gets the average of the MSPU reading buffer, calculated each update.
		/// @return The average value of the MSPU reading buffer.
		float GetMSPSUAverage() const { return m_MSPSUAverage; }

		/// Gets the average of the MSPF reading buffer, calculated each frame.
		/// @return The average value of the MSPF reading buffer.
		float GetMSPFAverage() const { return m_MSPFAverage; }
#pragma endregion

#pragma region Performance Counter Handling
		/// Moves sample counter to next sample and clears it's values.
		void NewPerformanceSample();

		/// Saves current absolute time in microseconds as a start of performance measurement.
		/// @param counter Counter to start measurement for.
		void StartPerformanceMeasurement(PerformanceCounters counter);

		/// Saves current absolute time in microseconds as an end of performance measurement. The difference is added to the value of current performance sample.
		/// @param counter Counter to stop and updated measurement for.
		void StopPerformanceMeasurement(PerformanceCounters counter);

		/// Sets the current ping value to display.
		/// @param ping Ping value to display.
		void SetCurrentPing(int ping) { m_CurrentPing = ping; }
#pragma endregion

#pragma region Concrete Methods
		/// Clears current performance timings.
		void ResetPerformanceTimings() {
			m_MSPSUs.clear();
			m_MSPFs.clear();
			m_MSPUs.clear();
			m_MSPDs.clear();
		}

		/// Resets the sim update timer.
		void ResetSimUpdateTimer() const { m_SimUpdateTimer->Reset(); }

		/// Updates the frame time measurements and recalculates the averages. Supposed to be done every game loop iteration.
		/// @param measuredUpdateTime The total sim update time measured in the game loop iteration.
		/// @param measuredDrawTime The total draw time measured in the game loop iteration.
		void UpdateMSPF(long long measuredUpdateTime, long long measuredDrawTime);

		/// Updates the individual sim update time measurements and recalculates the average. Supposed to be done every sim update.
		void UpdateMSPSU() {
			CalculateTimeAverage(m_MSPSUs, m_MSPSUAverage, static_cast<float>(m_SimUpdateTimer->GetElapsedRealTimeMS()));
			m_SimUpdateTimer->Reset();
		}

		/// Draws the performance stats to the screen.
		/// @param bitmapToDrawTo The BITMAP to draw the performance stats to.
		void Draw(BITMAP* bitmapToDrawTo);

		/// Draws the current ping value to the screen.
		void DrawCurrentPing() const;
#pragma endregion

		/// Updates m_SortedScriptTimings so PerformanceMan::Draw() can list how long scripts took.
		void UpdateSortedScriptTimings(const std::unordered_map<std::string, ScriptTiming>& scriptTimings);

	protected:
		static constexpr int c_MSPAverageSampleSize = 10; //!< How many samples to use to calculate average milliseconds-per-something value.
		static constexpr int c_MaxSamples = 120; //!< How many performance samples to store, directly affects graph size.
		static constexpr int c_Average = 10; //!< How many samples to use to calculate average value displayed on screen.
		static const std::array<std::string, PerformanceCounters::PerfCounterCount> m_PerfCounterNames; //!< Performance counter names displayed on screen.

		const int c_StatsOffsetX = 17; //!< Offset of the stat text from the left edge of the screen.
		const int c_StatsHeight = 14; //!< Height of each stat text line.
		const int c_GraphsOffsetX = 14; //!< Offset of the graph from the left edge of the screen.
		const int c_GraphsStartOffsetY = 174; //!< Position the first graph block will be drawn from the top edge of the screen.
		const int c_GraphHeight = 20; //!< Height of the performance graph.
		const int c_GraphBlockHeight = 34; //!< Height of the whole graph block (text height and graph height combined).

		bool m_ShowPerfStats; //!< Whether to show performance stats on screen or not.
		bool m_AdvancedPerfStats; //!< Whether to show performance graphs on screen or not.

		int m_Sample; //!< Sample counter.

		std::unique_ptr<Timer> m_SimUpdateTimer; //!< Timer for measuring milliseconds per sim update for performance stats readings.

		std::deque<float> m_MSPSUs; //!< History log of single update time measurements in milliseconds, for averaging the results. In milliseconds.
		std::deque<float> m_MSPFs; //!< History log total frame time measurements in milliseconds, for averaging the results.
		std::deque<float> m_MSPUs; //!< History log of frame update time measurements in milliseconds, for averaging the results. In milliseconds.
		std::deque<float> m_MSPDs; //!< History log of frame draw time measurements in milliseconds, for averaging the results.

		float m_MSPSUAverage; //!< The average of the MSPSU reading buffer, calculated each sim update.
		float m_MSPFAverage; //!< The average of the MSPF reading buffer, calculated each game loop iteration.
		float m_MSPUAverage; //!< The average of the MSPU reading buffer, calculated each game loop iteration.
		float m_MSPDAverage; //!< The average of the MSPD reading buffer, calculated each game loop iteration.

		int m_CurrentPing; //!< Current ping value to display on screen.

		std::array<std::array<int, c_MaxSamples>, PerformanceCounters::PerfCounterCount> m_PerfPercentages; //!< Array to store percentages from SimTotal.
		std::array<std::array<std::atomic_uint64_t, c_MaxSamples>, PerformanceCounters::PerfCounterCount> m_PerfData; //!< Array to store performance measurements in microseconds.

		std::vector<std::pair<std::string, ScriptTiming>> m_SortedScriptTimings; //!< Sorted vector storing how long scripts took to execute.

	private:
#pragma region Performance Counter Handling
		/// Adds provided value to current sample of specified performance counter.
		/// @param counter Counter to update.
		/// @param value Value to add to this counter.
		void AddPerformanceSample(PerformanceCounters counter, uint64_t value) { m_PerfData[counter].at(m_Sample) += value; }

		/// Calculates current sample's percentages from SIM_TOTAL for all performance counters and stores them to m_PerfPercenrages.
		void CalculateSamplePercentages();

		/// Returns an average value of c_Average last samples for specified performance counter.
		/// @param counter Counter to get average value from.
		/// @return An average value for specified counter.
		uint64_t GetPerformanceCounterAverage(PerformanceCounters counter) const;
#pragma endregion

		/// Stores the new time measurement into the specified deque, recalculates the average and stores it in the specified variable.
		/// @param timeMeasurements The deque of time measurements to store the new measurement in and to recalculate the average with.
		/// @param avgResult The variable the recalculated average should be stored in.
		/// @param newTimeMeasurement The new time measurement to store.
		void CalculateTimeAverage(std::deque<float>& timeMeasurements, float& avgResult, float newTimeMeasurement) const;

		/// Draws the performance graphs to the screen. This will be called by Draw() if advanced performance stats are enabled.
		void DrawPeformanceGraphs(AllegroBitmap& bitmapToDrawTo);

		/// Clears all the member variables of this PerformanceMan, effectively resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		PerformanceMan(const PerformanceMan& reference) = delete;
		PerformanceMan& operator=(const PerformanceMan& rhs) = delete;
	};
} // namespace RTE
