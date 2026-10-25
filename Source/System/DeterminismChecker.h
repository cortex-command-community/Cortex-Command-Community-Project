#pragma once

#include "Vector.h"

#include <cstdint>
#include <fstream>
#include <string>
#include <string_view>
#include <thread>
#include <type_traits>

namespace RTE {

	/// Accumulates a hash of the values added to it. Adding the same values in the same order always results in the same hash.
	class StateHasher {

	public:
		/// Adds a block of memory to the hash.
		/// @param data The start of the block.
		/// @param size The size of the block, in bytes.
		void AddBytes(const void* data, size_t size) {
			const unsigned char* bytes = static_cast<const unsigned char*>(data);
			for (size_t i = 0; i < size; ++i) {
				m_Hash = (m_Hash ^ bytes[i]) * c_FNVPrime;
			}
		}

		/// Adds a number or enum value to the hash. Pointers are deliberately not accepted, since they differ from run to run.
		/// @param value The value to add.
		template <typename ValueType>
		    requires std::is_arithmetic_v<ValueType> || std::is_enum_v<ValueType>
		void Add(ValueType value) { AddBytes(&value, sizeof(value)); }

		/// Adds a Vector to the hash.
		/// @param value The Vector to add.
		void Add(const Vector& value) {
			Add(value.m_X);
			Add(value.m_Y);
		}

		/// Adds a string to the hash.
		/// @param value The string to add.
		void Add(std::string_view value) { AddBytes(value.data(), value.size()); }

		/// Gets the hash of everything added so far.
		/// @return The hash.
		uint64_t GetHash() const { return m_Hash; }

	private:
		// This is the 64-bit FNV-1a hash.
		static constexpr uint64_t c_FNVOffsetBasis = 14695981039346656037ULL;
		static constexpr uint64_t c_FNVPrime = 1099511628211ULL;

		uint64_t m_Hash = c_FNVOffsetBasis; //!< The hash of everything added so far.
	};

	/// Finds out whether, and where, two runs of the simulation diverge.
	/// The simulation makes checkpoints as it goes, each a label and a hash of some of its state. One run records these to a trace file, which later runs verify their own checkpoints against.
	class DeterminismChecker {

	public:
		/// Starts recording all checkpoints to a trace file.
		/// @param traceFilePath The path of the trace file to write.
		static void StartRecording(const std::string& traceFilePath);

		/// Starts verifying all checkpoints against a previously recorded trace file. The first checkpoint that doesn't match gets reported, and quits the program.
		/// @param traceFilePath The path of the trace file to verify against.
		static void StartVerifying(const std::string& traceFilePath);

		/// Makes a last checkpoint of the simulation state, and stops recording or verifying.
		static void Stop();

		/// Gets whether checkpoints are being recorded or verified.
		/// @return Whether checkpoints are being recorded or verified.
		static bool IsActive() { return s_Mode != Mode::Inactive; }

		/// Gets whether a checkpoint that didn't match the trace being verified against was found.
		/// @return Whether this run diverged from the recorded one.
		static bool HasDiverged() { return s_Diverged; }

		/// Makes a checkpoint out of the passed in values. Can only be done from the main thread, since the checkpoints have to be made in the same order each run.
		/// @param label A description of what is being checked, to tell the checkpoints apart.
		/// @param values The values to check. See the StateHasher for which types can be used.
		template <typename... ValueTypes>
		static void Checkpoint(std::string_view label, const ValueTypes&... values) {
			StateHasher hasher;
			(hasher.Add(values), ...);
			SubmitCheckpoint(label, hasher.GetHash());
		}

		/// Makes checkpoints out of the state of the simulation as a whole. The layers of the Scene are big, so they are only included every once in a while.
		/// @param label A description of the point in the simulation update this is done at.
		static void CheckpointSimState(std::string_view label);

	private:
		enum class Mode {
			Inactive,
			Recording,
			Verifying
		};

		static constexpr int c_SceneCheckInterval = 60; //!< How many sim updates to go between checking the layers of the Scene.

		static Mode s_Mode; //!< Whether checkpoints are being recorded, verified or neither.
		static bool s_Diverged; //!< Whether a checkpoint that didn't match the trace being verified against was found.
		static std::string s_TraceFilePath; //!< The path of the trace file being recorded to or verified against.
		static std::fstream s_TraceFile; //!< The trace file being recorded to or verified against.
		static std::thread::id s_MainThreadID; //!< The thread that checkpoints are allowed to be made from.

		/// Opens the trace file, and quits the program if that fails.
		/// @param traceFilePath The path of the trace file.
		/// @param openMode Whether to open the file for reading or writing.
		static void OpenTraceFile(const std::string& traceFilePath, std::ios::openmode openMode);

		/// Records the checkpoint, or verifies it against the next one in the trace file.
		/// @param label A description of what is being checked.
		/// @param hash The hash of the values being checked.
		static void SubmitCheckpoint(std::string_view label, uint64_t hash);

		/// Makes checkpoints out of the state of the simulation as a whole.
		/// @param label A description of the point in the simulation update this is done at.
		/// @param checkSceneLayers Whether to include the layers of the Scene.
		static void CheckpointSimState(std::string_view label, bool checkSceneLayers);

		/// Makes checkpoints out of the layers of the Scene that the simulation depends on.
		/// @param label A description of the point in the simulation update this is done at.
		static void CheckpointSceneLayers(std::string_view label);

		/// Reports that this run doesn't match the trace being verified against, and quits the program.
		/// @param expectedCheckpoint The checkpoint from the trace file.
		/// @param actualCheckpoint The checkpoint that this run made instead.
		static void ReportDivergence(const std::string& expectedCheckpoint, const std::string& actualCheckpoint);
	};
} // namespace RTE

// Use these to make checkpoints rather than calling the DeterminismChecker directly, so that nothing gets evaluated unless it is active.
#define DeterminismCheck(label, ...) \
	if (RTE::DeterminismChecker::IsActive()) { \
		RTE::DeterminismChecker::Checkpoint(label, __VA_ARGS__); \
	}

#define DeterminismCheckSimState(label) \
	if (RTE::DeterminismChecker::IsActive()) { \
		RTE::DeterminismChecker::CheckpointSimState(label); \
	}
