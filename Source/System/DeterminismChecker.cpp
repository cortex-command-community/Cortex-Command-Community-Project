#include "DeterminismChecker.h"

#include "ActivityMan.h"
#include "ConsoleMan.h"
#include "LuaMan.h"
#include "MovableMan.h"
#include "SceneMan.h"
#include "TimerMan.h"

#include "Activity.h"
#include "Scene.h"
#include "SLTerrain.h"

#include "RTEError.h"
#include "RTETools.h"
#include "System.h"

#include "allegro.h"

#include <format>
#include <iostream>

using namespace RTE;

DeterminismChecker::Mode DeterminismChecker::s_Mode = Mode::Inactive;
bool DeterminismChecker::s_Diverged = false;
std::string DeterminismChecker::s_TraceFilePath;
std::fstream DeterminismChecker::s_TraceFile;
std::thread::id DeterminismChecker::s_MainThreadID;

void DeterminismChecker::StartRecording(const std::string& traceFilePath) {
	OpenTraceFile(traceFilePath, std::ios::out | std::ios::trunc);
	s_Mode = Mode::Recording;
}

void DeterminismChecker::StartVerifying(const std::string& traceFilePath) {
	OpenTraceFile(traceFilePath, std::ios::in);
	s_Mode = Mode::Verifying;
}

void DeterminismChecker::OpenTraceFile(const std::string& traceFilePath, std::ios::openmode openMode) {
	s_TraceFilePath = traceFilePath;
	s_TraceFile.open(traceFilePath, openMode);
	if (!s_TraceFile.is_open()) {
		// This happens before there are any managers around to report through, and carrying on without checking anything would look just like a run that passed.
		RTEError::ShowMessageBox("Failed to open the determinism trace file \"" + traceFilePath + "\"!");
		std::exit(EXIT_FAILURE);
	}
	s_MainThreadID = std::this_thread::get_id();
}

void DeterminismChecker::Stop() {
	if (!IsActive()) {
		return;
	}

	if (g_SceneMan.GetScene()) {
		CheckpointSimState("End of run", true);
	}

	std::string remainingCheckpoint;
	if (s_Mode == Mode::Verifying && !s_Diverged && std::getline(s_TraceFile, remainingCheckpoint)) {
		ReportDivergence(remainingCheckpoint, "Nothing, this run ended before the recorded one did.");
	}

	s_TraceFile.close();
	s_Mode = Mode::Inactive;
}

void DeterminismChecker::SubmitCheckpoint(std::string_view label, uint64_t hash) {
	RTEAssert(std::this_thread::get_id() == s_MainThreadID, "Determinism checkpoints can only be made from the main thread, as there's no telling which order they would get made in otherwise!");

	std::string checkpoint = std::format("{} {:016x} {}", g_TimerMan.GetSimUpdateCount(), hash, label);

	if (s_Mode == Mode::Recording) {
		s_TraceFile << checkpoint << "\n";
	} else if (!s_Diverged) {
		std::string expectedCheckpoint;
		if (!std::getline(s_TraceFile, expectedCheckpoint)) {
			ReportDivergence("Nothing, the recorded run ended before this one did.", checkpoint);
		} else if (checkpoint != expectedCheckpoint) {
			ReportDivergence(expectedCheckpoint, checkpoint);
		}
	}
}

void DeterminismChecker::CheckpointSimState(std::string_view label) {
	CheckpointSimState(label, g_TimerMan.GetSimUpdateCount() % c_SceneCheckInterval == 0);
}

void DeterminismChecker::CheckpointSimState(std::string_view label, bool checkSceneLayers) {
	Checkpoint(std::format("{} - RNG", label), g_RandomGenerator.PeekNext());

	Checkpoint(std::format("{} - Master Lua state RNG", label), g_LuaMan.GetMasterScriptState().GetRandomGenerator().PeekNext());
	const LuaStatesArray& luaStates = g_LuaMan.GetThreadedScriptStates();
	for (size_t i = 0; i < luaStates.size(); ++i) {
		Checkpoint(std::format("{} - Lua state {} RNG", label, i), luaStates[i].GetRandomGenerator().PeekNext());
	}

	StateHasher movableObjectsHasher;
	g_MovableMan.HashState(movableObjectsHasher);
	Checkpoint(std::format("{} - MovableObjects", label), movableObjectsHasher.GetHash());

	if (const Activity* activity = g_ActivityMan.GetActivity()) {
		StateHasher activityHasher;
		activityHasher.Add(activity->GetActivityState());
		for (int team = Activity::Teams::TeamOne; team < Activity::Teams::MaxTeamCount; ++team) {
			activityHasher.Add(activity->GetTeamFunds(team));
		}
		Checkpoint(std::format("{} - Activity", label), activityHasher.GetHash());
	}

	if (checkSceneLayers) {
		CheckpointSceneLayers(label);
	}
}

void DeterminismChecker::CheckpointSceneLayers(std::string_view label) {
	Scene* scene = g_SceneMan.GetScene();
	if (!scene) {
		return;
	}

	auto hashBitmap = [](BITMAP* bitmap) {
		StateHasher hasher;
		size_t bytesPerLine = static_cast<size_t>(bitmap->w) * (bitmap_color_depth(bitmap) / 8);
		for (int y = 0; y < bitmap->h; ++y) {
			hasher.AddBytes(bitmap->line[y], bytesPerLine);
		}
		return hasher.GetHash();
	};

	Checkpoint(std::format("{} - Terrain material", label), hashBitmap(scene->GetTerrain()->GetMaterialBitmap()));

	for (int team = Activity::Teams::TeamOne; team < Activity::Teams::MaxTeamCount; ++team) {
		if (const SceneLayer* unseenLayer = scene->GetUnseenLayer(team)) {
			Checkpoint(std::format("{} - Team {} unseen layer", label, team + 1), hashBitmap(unseenLayer->GetBitmap()));
		}
	}
}

void DeterminismChecker::ReportDivergence(const std::string& expectedCheckpoint, const std::string& actualCheckpoint) {
	s_Diverged = true;

	// Checkpoints are written out as the sim update they were made in, the hash of the checked values, and the label.
	std::string report = "Determinism break! This run diverged from the one recorded in \"" + s_TraceFilePath + "\".\nExpected: " + expectedCheckpoint + "\nGot:      " + actualCheckpoint;

	g_ConsoleMan.PrintString("ERROR: " + report);
	// The console can't be seen from the outside, so also report it in a way that whatever launched this run can pick up.
	std::cerr << report << std::endl;

	System::SetQuit();
}
