#include "ActivityMan.h"
#include "Activity.h"

#include "CameraMan.h"
#include "ConsoleMan.h"
#include "PresetMan.h"
#include "UInputMan.h"
#include "AudioMan.h"
#include "WindowMan.h"
#include "FrameMan.h"
#include "PerformanceMan.h"
#include "PostProcessMan.h"
#include "MetaMan.h"
#include "ThreadMan.h"

#include "GAScripted.h"
#include "SLTerrain.h"

#include "EditorActivity.h"
#include "SceneEditor.h"
#include "AreaEditor.h"
#include "GibEditor.h"
#include "ActorEditor.h"
#include "AssemblyEditor.h"

#include "MusicMan.h"

#ifdef SYSTEM_MINIZIP
#include <minizip/zip.h>
#include <minizip/unzip.h>
#else
#include "zip.h"
#include "unzip.h"
#endif

#include "tracy/Tracy.hpp"

#include "SDL3/SDL_surface.h"
#include <SDL3_image/SDL_image.h>

#include <array>
#include <execution>

using namespace RTE;

ActivityMan::ActivityMan() {
	Clear();
}

ActivityMan::~ActivityMan() {
	Destroy();
}

void ActivityMan::Clear() {
	m_DefaultActivityType = "GATutorial";
	m_DefaultActivityName = "Tutorial Mission";
	m_Activity = nullptr;
	m_StartActivity = nullptr;
	m_SaveGameTask = std::future<void>();
	m_InActivity = false;
	m_ActivityNeedsRestart = false;
	m_ActivityNeedsResume = false;
	m_ResumingActivityFromPauseMenu = false;
	m_SkipPauseMenuWhenPausingActivity = false;
	m_LaunchIntoActivity = false;
	m_LaunchIntoEditor = false;
}

bool ActivityMan::Initialize() {
	if (IsSetToLaunchIntoEditor()) {
		// Evaluate LaunchIntoEditor before LaunchIntoActivity so it takes priority when both are set, otherwise it is ignored and editor is never launched.
		return SetStartEditorActivitySetToLaunchInto();
	} else if (IsSetToLaunchIntoActivity()) {
		m_ActivityNeedsRestart = true;
		return true;
	}
	return false;
}

bool ActivityMan::ForceAbortSave() {
	// Just a utility function we can call in the debugger quickwatch window to force an abort save to occur (great for force-saving the game when it crashes)
	// Throw ActivityMan::Instance().ForceAbortSave() into a quickwatch window and evaluate :)
	return SaveCurrentGame("AbortSave");
}

// For some reason these aren't defined on Linux/MacOS... so
#define HACK_MZ_COMPRESS_METHOD_STORE 0
#define HACK_MZ_COMPRESS_LEVEL_FAST 2
#define HACK_MZ_COMPRESS_METHOD_DEFLATE 8

bool ActivityMan::SaveCurrentGame(const std::string& fileName) {
	if (m_SaveGameTask.valid()) {
		m_SaveGameTask.wait();
	}

	ZoneScopedN("Save Game");

	Scene* scene = g_SceneMan.GetScene();
	GAScripted* activity = dynamic_cast<GAScripted*>(GetActivity());

	if (!scene || !activity || (activity && activity->GetActivityState() == Activity::ActivityState::Over)) {
		g_ConsoleMan.PrintString("ERROR: Cannot save when there's no game running, or the game is finished!");
		return false;
	}

	// Get BITMAPS so save into our zip, do this async so we can copy the scene info at the same time
	std::vector<SceneLayerInfo>* sceneLayerInfos = new std::vector<SceneLayerInfo>();
	std::future<void> copyBitmaps = g_ThreadMan.GetBackgroundThreadPool().submit([&]() {
		*sceneLayerInfos = std::move(scene->GetCopiedSceneLayerBitmaps());
	});

	// We need a copy of our scene, because we have to do some fixup to remove PLACEONLOAD items and only keep the current MovableMan state.
	std::unique_ptr<Scene> modifiableScene(dynamic_cast<Scene*>(scene->Clone()));

	// Delete any existing objects from our scene - we don't want to replace broken doors or repair any stuff when we load.
	modifiableScene->ClearPlacedObjectSet(Scene::PlacedObjectSets::PLACEONLOAD, true);

	// Become our own original preset, instead of being a copy of the Scene we got cloned from, so we don't still pick up the PlacedObjectSets from our parent when loading.
	modifiableScene->SetPresetName(fileName);
	modifiableScene->MigrateToModule(g_PresetMan.GetModuleID(c_UserScriptedSavesModuleName));
	modifiableScene->SetSavedGameInternal(true);

	// Make sure the terrain is also treated as an original preset, otherwise it will screw up if we save then load then save again, since it'll try to be a CopyOf of itself.
	modifiableScene->GetTerrain()->SetPresetName(fileName);
	modifiableScene->GetTerrain()->MigrateToModule(g_PresetMan.GetModuleID(c_UserScriptedSavesModuleName));

	// See our content files to point to our save game location. This won't actually save a file here- but it allows us to set these up as in-memory ContentFiles on load
	// Meaning that our loading code doesn't need to care about whether it's loading a savegame or a file- it just sees it as an already loaded, cached bitmap
	modifiableScene->GetTerrain()->GetContentFile().SetIsMemoryFile(true);
	modifiableScene->GetTerrain()->GetFGSceneLayer()->GetContentFile().SetIsMemoryFile(true);
	modifiableScene->GetTerrain()->GetBGSceneLayer()->GetContentFile().SetIsMemoryFile(true);

	modifiableScene->GetTerrain()->GetContentFile().SetDataPath(g_PresetMan.GetFullModulePath(c_UserScriptedSavesModuleName) + "/Save Mat.png");
	modifiableScene->GetTerrain()->GetFGSceneLayer()->GetContentFile().SetDataPath(g_PresetMan.GetFullModulePath(c_UserScriptedSavesModuleName) + "/Save FG.png");
	modifiableScene->GetTerrain()->GetBGSceneLayer()->GetContentFile().SetDataPath(g_PresetMan.GetFullModulePath(c_UserScriptedSavesModuleName) + "/Save BG.png");

	for (int i = 0; i < Activity::MaxTeamCount; ++i) {
		SceneLayer* unseenLayer = modifiableScene->GetUnseenLayer(i);
		if (unseenLayer) {
			unseenLayer->GetContentFile().SetIsMemoryFile(true);
			unseenLayer->GetContentFile().SetDataPath(g_PresetMan.GetFullModulePath(c_UserScriptedSavesModuleName) + std::format("/Save UST{}.png", i));
		}
	}

	std::unique_ptr<std::stringstream> iniStream = std::make_unique<std::stringstream>();

	// Block the main thread for a bit to let the Writer access the relevant data.
	std::unique_ptr<Writer> writer(std::make_unique<Writer>(std::move(iniStream)));
	writer->NewPropertyWithValue("Activity", activity);

	// Pull all stuff from MovableMan into the Scene for saving, so existing Actors/ADoors are saved, without transferring ownership, so the game can continue.
	// This is done after the activity is saved, in case the activity wants to add anything to the scene while saving.
	// TODO- copying may be faster, and lets us move all this actual writing into async
	modifiableScene->RetrieveSceneObjects(false);
	for (SceneObject* objectToSave: *modifiableScene->GetPlacedObjects(Scene::PlacedObjectSets::PLACEONLOAD)) {
		if (MovableObject* objectToSaveAsMovableObject = dynamic_cast<MovableObject*>(objectToSave)) {
			objectToSaveAsMovableObject->OnSave();
		}
	}

	writer->NewPropertyWithValue("OriginalScenePresetName", scene->GetPresetName());
	writer->NewPropertyWithValue("PlaceObjectsIfSceneIsRestarted", g_SceneMan.GetPlaceObjectsOnLoad());
	writer->NewPropertyWithValue("PlaceUnitsIfSceneIsRestarted", g_SceneMan.GetPlaceUnitsOnLoad());
	writer->NewPropertyWithValue("Scene", modifiableScene.get());

	// Save a small little file with index info (activity and original scene name) so we can display info in the samegame menu without needing to decompress and read through the entire zip
	std::unique_ptr<std::stringstream> indexStream = std::make_unique<std::stringstream>();
	Writer* indexWriter = new Writer(std::move(indexStream));
	indexWriter->NewPropertyWithValue("ActivityName", activity->GetPresetName());
	indexWriter->NewPropertyWithValue("OriginalScenePresetName", scene->GetPresetName());

	auto saveWriterData = [fileName, sceneLayerInfos, indexWriter](Writer* mainWriter) {
		// Create zip sav file
		zipFile zippedSaveFile = zipOpen((g_PresetMan.GetFullModulePath(c_UserScriptedSavesModuleName) + "/" + fileName + ".ccsave").c_str(), APPEND_STATUS_CREATE);
		if (!zippedSaveFile) {
			g_ConsoleMan.PrintString("ERROR: Couldn't create zip save file!");
			delete mainWriter;
			delete indexWriter;
			delete sceneLayerInfos;
			return;
		}

		std::stringstream* mainStream = static_cast<std::stringstream*>(mainWriter->GetStream());
		std::stringstream* indexStream = static_cast<std::stringstream*>(indexWriter->GetStream());
		mainStream->flush();
		indexStream->flush();

		std::string_view mainStreamView = mainStream->view();
		std::string_view indexStreamView = indexStream->view();

		zip_fileinfo zfi = {0};

		zipOpenNewFileInZip(zippedSaveFile, "Index.ini", &zfi, nullptr, 0, nullptr, 0, nullptr, HACK_MZ_COMPRESS_METHOD_STORE, HACK_MZ_COMPRESS_LEVEL_FAST);
		zipWriteInFileInZip(zippedSaveFile, indexStreamView.data(), indexStreamView.size());
		zipCloseFileInZip(zippedSaveFile);

		zipOpenNewFileInZip(zippedSaveFile, "Save.ini", &zfi, nullptr, 0, nullptr, 0, nullptr, HACK_MZ_COMPRESS_METHOD_DEFLATE, HACK_MZ_COMPRESS_LEVEL_FAST);
		zipWriteInFileInZip(zippedSaveFile, mainStreamView.data(), mainStreamView.size());
		zipCloseFileInZip(zippedSaveFile);

		std::vector<SDL_IOStream*> pngStreams;
		pngStreams.resize(sceneLayerInfos->size());

		// Generates PNGs (this is thread-safe)
		std::for_each(std::execution::par_unseq,
		              sceneLayerInfos->begin(), sceneLayerInfos->end(),
		              [&](const SceneLayerInfo& layerInfo) {
			              // Save png into a memory buffer
			              SDL_IOStream* stream = SDL_IOFromDynamicMem();
			              SDL_Surface* image = SDL_CreateSurfaceFrom(layerInfo.bitmap->w, layerInfo.bitmap->h, SDL_PIXELFORMAT_INDEX8, layerInfo.bitmap->dat, layerInfo.bitmap->w);

			              SDL_Palette* palette = ContentFile::DefaultPaletteToSDL();
			              SDL_SetSurfacePalette(image, palette);
						  SDL_SetSurfaceColorKey(image, false, 0);

			              bool result = IMG_SavePNG_IO(image, stream, false);
			              SDL_FlushIO(stream);

			              SDL_DestroyPalette(palette);
			              SDL_DestroySurface(image);

			              if (!result) {
				              g_ConsoleMan.PrintString("ERROR: Failed to save scenelayers to PNG!");
				              return;
			              }

						  size_t i = &layerInfo - &(*sceneLayerInfos->begin());
			              pngStreams[i] = stream;
		              });

		// Actually save to the zip (this bit isn't thread-safe)
		for (int i = 0; i < pngStreams.size(); ++i) {
			SDL_IOStream* stream = pngStreams[i];
			
			// Actually get the memory
			void* buffer = SDL_GetPointerProperty(SDL_GetIOProperties(stream), SDL_PROP_IOSTREAM_DYNAMIC_MEMORY_POINTER, nullptr);
			size_t size = static_cast<size_t>(SDL_GetIOSize(stream));
			if (!stream || size < 0) {
				g_ConsoleMan.PrintString("ERROR: Failed to save scenelayers to PNG!");
				continue;
			}

			zipOpenNewFileInZip(zippedSaveFile, ("Save " + (*sceneLayerInfos)[i].name + ".png").c_str(), &zfi, nullptr, 0, nullptr, 0, nullptr, HACK_MZ_COMPRESS_METHOD_STORE, HACK_MZ_COMPRESS_LEVEL_FAST);
			zipWriteInFileInZip(zippedSaveFile, static_cast<const char*>(buffer), size);
			zipCloseFileInZip(zippedSaveFile);

			SDL_CloseIO(stream);
		}

		zipClose(zippedSaveFile, fileName.c_str());

		delete mainWriter;
		delete indexWriter;
		delete sceneLayerInfos;
	};

	copyBitmaps.wait();

	// For some reason I can't std::move a unique ptr in, so just releasing and deleting manually...
	m_SaveGameTask = g_ThreadMan.GetBackgroundThreadPool().submit(saveWriterData, writer.release());

	// We didn't transfer ownership, so we must be very careful that sceneAltered's deletion doesn't touch the stuff we got from MovableMan.
	modifiableScene->ClearPlacedObjectSet(Scene::PlacedObjectSets::PLACEONLOAD, false);

	g_ConsoleMan.PrintString("SYSTEM: Game saved to \"" + fileName + "\"!");
	return true;
}

bool ActivityMan::LoadAndLaunchGame(const std::string& fileName) {
	std::string filePath = g_PresetMan.GetFullModulePath(c_UserScriptedSavesModuleName) + "/" + fileName;

	// load zip sav file
	std::string saveFilePath = filePath + ".ccsave";
	unzFile zippedSaveFile = unzOpen(saveFilePath.c_str());
	if (!zippedSaveFile) {
		// Might be trying to open one we're already saving too, wait until we finish saving and try again
		if (m_SaveGameTask.valid()) {
			m_SaveGameTask.wait();
			zippedSaveFile = unzOpen(saveFilePath.c_str());
		}

		if (!zippedSaveFile) {
			// Some other process is stopping us from loading, oh well
			RTEError::ShowMessageBox("Game loading failed! Make sure you have a saved game called \"" + fileName + "\"");
			return false;
		}
	}

	unz_file_info info;
	char* buffer = nullptr;

	auto unzipFileIntoBuffer = [&](const std::string& fullFileName) {
		// These need to use NULL instead of nullptr to compile on Linux/OSX?
		if (unzLocateFile(zippedSaveFile, fullFileName.c_str(), NULL) == UNZ_END_OF_LIST_OF_FILE) {
			return false;
		}

		unzOpenCurrentFile(zippedSaveFile);
		unzGetCurrentFileInfo(zippedSaveFile, &info, nullptr, 0, nullptr, 0, nullptr, 0);

		buffer = (char*)malloc(info.uncompressed_size + 1); // add one so we can add a pretend null terminator on the end
		if (!buffer) {
			// If this ever hits I've lost all faith in modern OSes, but alas when one is writing C, one must dance along
			RTEError::ShowMessageBox("Catastrophic failure! Failed to allocate memory for savegame");
			return false;
		}

		unzReadCurrentFile(zippedSaveFile, buffer, info.uncompressed_size);
		unzCloseCurrentFile(zippedSaveFile);

		return true;
	};

	auto loadMemPng = [](void* buffer, size_t size) {
		SDL_IOStream* stream = SDL_IOFromConstMem(buffer, size);
		SDL_Surface* image = stream ? IMG_LoadPNG_IO(stream) : nullptr;
		SDL_CloseIO(stream);

		int bitDepth = SDL_GetPixelFormatDetails(image->format)->bits_per_pixel;
		SDL_Palette* palette = ContentFile::DefaultPaletteToSDL();
		if (bitDepth != 8) {
			SDL_Surface* newImage = SDL_ConvertSurfaceAndColorspace(image, SDL_PIXELFORMAT_INDEX8, palette, SDL_COLORSPACE_UNKNOWN, 0);
			SDL_DestroySurface(image);
			image = newImage;
		} else {
			SDL_SetSurfacePalette(image, palette);
		}
		SDL_DestroyPalette(palette);

		free(buffer);
		return image;
	};

	// Manually load all our bitmaps into our cache so the activity skips looking for the file and just gets it directly from us
	if (unzipFileIntoBuffer("Save Mat.png")) {
		ContentFile::ManuallyLoadDataPNG(g_PresetMan.GetFullModulePath(c_UserScriptedSavesModuleName) + "/Save Mat.png", loadMemPng(buffer, info.uncompressed_size));
	}

	if (unzipFileIntoBuffer("Save FG.png")) {
		ContentFile::ManuallyLoadDataPNG(g_PresetMan.GetFullModulePath(c_UserScriptedSavesModuleName) + "/Save FG.png", loadMemPng(buffer, info.uncompressed_size));
	}

	if (unzipFileIntoBuffer("Save BG.png")) {
		ContentFile::ManuallyLoadDataPNG(g_PresetMan.GetFullModulePath(c_UserScriptedSavesModuleName) + "/Save BG.png", loadMemPng(buffer, info.uncompressed_size));
	}

	for (int i = 0; i < Activity::MaxTeamCount; ++i) {
		if (unzipFileIntoBuffer(std::format("Save UST{}.png", i))) {
			ContentFile::ManuallyLoadDataPNG(g_PresetMan.GetFullModulePath(c_UserScriptedSavesModuleName) + std::format("/Save UST{}.png", i), loadMemPng(buffer, info.uncompressed_size));
		}
	}

	if (!unzipFileIntoBuffer("Save.ini")) {
		RTEError::ShowMessageBox("Game loading failed! This save looks invalid or corrupted.");
		return false;
	}

	buffer[info.uncompressed_size] = 0; // null terminate

	Reader reader(std::make_unique<std::istringstream>(buffer), filePath + "/Save.ini", true, nullptr, false);

	std::unique_ptr<Scene> scene(std::make_unique<Scene>());
	std::unique_ptr<GAScripted> activity(std::make_unique<GAScripted>());

	std::string originalScenePresetName = fileName;
	bool placeObjectsIfSceneIsRestarted = true;
	bool placeUnitsIfSceneIsRestarted = true;
	while (reader.NextProperty()) {
		std::string propName = reader.ReadPropName();
		if (propName == "Activity") {
			reader >> activity.get();
		} else if (propName == "OriginalScenePresetName") {
			reader >> originalScenePresetName;
		} else if (propName == "PlaceObjectsIfSceneIsRestarted") {
			reader >> placeObjectsIfSceneIsRestarted;
		} else if (propName == "PlaceUnitsIfSceneIsRestarted") {
			reader >> placeUnitsIfSceneIsRestarted;
		} else if (propName == "Scene") {
			reader >> scene.get();
		}
	}

	free(buffer);

	unzClose(zippedSaveFile);

	// SetSceneToLoad() doesn't Clone(), but when the Activity starts, it will eventually call LoadScene(), which does a Clone() of scene internally.
	g_SceneMan.SetSceneToLoad(scene.get(), true, true);
	// Saved Scenes get their presetname set to their filename to ensure they're separate from the preset Scene they're based off of.
	// However, saving a game you've already saved will end up with its OriginalScenePresetName set to the filename, which will screw up restarting the Activity, so we set its PresetName here.
	scene->SetPresetName(originalScenePresetName);
	// For starting Activity, we need to directly clone the Activity we want to start.
	StartActivity(dynamic_cast<GAScripted*>(activity->Clone()));
	// When this method exits, our Scene object will be destroyed, which will cause problems if you try to restart it. To avoid this, set the Scene to load to the preset object with the same name.
	g_SceneMan.SetSceneToLoad(originalScenePresetName, placeObjectsIfSceneIsRestarted, placeUnitsIfSceneIsRestarted);

	g_ConsoleMan.PrintString("SYSTEM: Game \"" + fileName + "\" loaded!");

	return true;
}

void ActivityMan::SetStartActivity(Activity* newActivity) {
	RTEAssert(newActivity, "Trying to replace an activity with a null one!");
	m_StartActivity.reset(newActivity);
}

void ActivityMan::SetStartTutorialActivity() {
	SetStartActivity(dynamic_cast<Activity*>(g_PresetMan.GetEntityPreset("GATutorial", "Tutorial Mission")->Clone()));
	if (GameActivity* gameActivity = dynamic_cast<GameActivity*>(GetStartActivity())) {
		gameActivity->SetStartingGold(10000);
	}
	g_SceneMan.SetSceneToLoad("Tutorial Bunker");
}

void ActivityMan::SetStartEditorActivity(const std::string_view& editorToLaunch) {
	std::unique_ptr<EditorActivity> editorActivityToStart = nullptr;

	if (editorToLaunch == "ActorEditor") {
		editorActivityToStart = std::make_unique<ActorEditor>();
	} else if (editorToLaunch == "GibEditor") {
		editorActivityToStart = std::make_unique<GibEditor>();
	} else if (editorToLaunch == "SceneEditor") {
		editorActivityToStart = std::make_unique<SceneEditor>();
	} else if (editorToLaunch == "AreaEditor") {
		editorActivityToStart = std::make_unique<AreaEditor>();
	} else if (editorToLaunch == "AssemblyEditor") {
		editorActivityToStart = std::make_unique<AssemblyEditor>();
	}
	if (editorActivityToStart) {
		if (g_MetaMan.GameInProgress()) {
			g_MetaMan.EndGame();
		}
		g_SceneMan.SetSceneToLoad("Editor Scene");
		editorActivityToStart->Create();
		editorActivityToStart->SetEditorMode(EditorActivity::LOADDIALOG);
		SetStartActivity(editorActivityToStart.release());
		m_ActivityNeedsRestart = true;
	} else {
		RTEAbort("Failed to instantiate the " + std::string(editorToLaunch) + " Activity!");
	}
}

bool ActivityMan::SetStartEditorActivitySetToLaunchInto() {
	std::array<std::string_view, 5> validEditorNames = {"ActorEditor", "GibEditor", "SceneEditor", "AreaEditor", "AssemblyEditor"};

	if (std::find(validEditorNames.begin(), validEditorNames.end(), m_EditorToLaunch) != validEditorNames.end()) {
		// Force mouse + keyboard with default mapping so we won't need to change manually if player 1 is set to keyboard only or gamepad.
		g_UInputMan.GetControlScheme(Players::PlayerOne)->SetDevice(InputDevice::DEVICE_MOUSE_KEYB);
		g_UInputMan.GetControlScheme(Players::PlayerOne)->SetPreset(InputScheme::InputPreset::PresetMouseWASDKeys);
		SetStartEditorActivity(m_EditorToLaunch);
		return true;
	} else {
		g_ConsoleMan.PrintString("ERROR: Invalid editor name passed into \"-editor\" argument!");
		g_ConsoleMan.SetEnabled(true);
		m_LaunchIntoEditor = false;
		return false;
	}
}

int ActivityMan::StartActivity(Activity* activity) {
	RTEAssert(activity, "Trying to start a null activity!");

	g_ThreadMan.GetPriorityThreadPool().wait_for_tasks();
	g_ThreadMan.GetBackgroundThreadPool().wait_for_tasks();

	m_StartActivity.reset(activity);
	m_Activity.reset(dynamic_cast<Activity*>(m_StartActivity->Clone()));

	g_MusicMan.ResetMusicState();

	m_Activity->SetupPlayers();
	int error = m_Activity->Start();

	if (error >= 0)
		g_ConsoleMan.PrintString("SYSTEM: Activity \"" + m_Activity->GetPresetName() + "\" was successfully started");
	else {
		g_ConsoleMan.PrintString("ERROR: Activity \"" + m_Activity->GetPresetName() + "\" was NOT started due to errors!");
		m_Activity->SetActivityState(Activity::HasError);
		return error;
	}

	// Close the console in case it was open by the player or because of a previous Activity error.
	g_ConsoleMan.SetEnabled(false);

	m_ActivityNeedsResume = true;
	m_InActivity = true;

	g_PostProcessMan.ClearScenePostEffects();
	g_FrameMan.ClearScreenText();

	// Reset the mouse input to the center
	g_UInputMan.SetMouseValueMagnitude(0, g_UInputMan.MouseUsedByPlayer());

	g_AudioMan.PauseIngameSounds(false);

	g_PerformanceMan.ResetPerformanceTimings();

	return error;
}

int ActivityMan::StartActivity(const std::string& className, const std::string& presetName) {
	if (const Entity* entity = g_PresetMan.GetEntityPreset(className, presetName)) {
		Activity* newActivity = dynamic_cast<Activity*>(entity->Clone());
		if (GameActivity* newActivityAsGameActivity = dynamic_cast<GameActivity*>(newActivity)) {
			newActivityAsGameActivity->SetStartingGold(newActivityAsGameActivity->GetDefaultGoldMediumDifficulty());
			if (newActivityAsGameActivity->GetStartingGold() <= 0) {
				newActivityAsGameActivity->SetStartingGold(static_cast<int>(newActivityAsGameActivity->GetTeamFunds(0)));
			} else {
				newActivityAsGameActivity->SetTeamFunds(static_cast<float>(newActivityAsGameActivity->GetStartingGold()), 0);
			}
		}
		return StartActivity(newActivity);
	} else {
		g_ConsoleMan.PrintString("ERROR: Couldn't find the " + className + " named " + presetName + " to start! Has it been defined?");
		return -1;
	}
}

void ActivityMan::PauseActivity(bool pause, bool skipPauseMenu) {
	if (!m_Activity) {
		g_ConsoleMan.PrintString("ERROR: No Activity to pause!");
		return;
	}

	if (pause == m_Activity->IsPaused()) {
		return;
	}

	m_Activity->SetPaused(pause);
	m_InActivity = !pause;
	m_ResumingActivityFromPauseMenu = false;
	m_SkipPauseMenuWhenPausingActivity = skipPauseMenu;

	g_AudioMan.PauseIngameSounds(pause);
	g_AudioMan.SetMusicMuffledState(pause);

	if (!pause) {
		g_MusicMan.EndInterruptingMusic();
	}

	g_ConsoleMan.PrintString("SYSTEM: Activity \"" + m_Activity->GetPresetName() + "\" was " + (pause ? "paused" : "resumed"));
}

void ActivityMan::ResumeActivity() {
	if (GetActivity()->GetActivityState() != Activity::NotStarted) {
		m_InActivity = true;
		m_ActivityNeedsResume = false;

		std::vector<int> humanPlayers;
		for (int player = 0; player < MaxPlayerCount; player++) {
			if (m_Activity->PlayerHuman(player)) {
				humanPlayers.push_back(player);
			}
		}
		g_UInputMan.CheckMultiMouseKeyboardEnabled(humanPlayers);
		PauseActivity(false);
		g_TimerMan.PauseSim(false);
		g_PerformanceMan.ResetPerformanceTimings();
	}
}

bool ActivityMan::RestartActivity() {
	m_ActivityNeedsRestart = false;
	g_ConsoleMan.PrintString("SYSTEM: Activity was reset!");

	g_AudioMan.StopAll();
	g_MovableMan.PurgeAllMOs();
	// Have to reset TimerMan before creating anything else because all timers are reset against it.
	g_TimerMan.ResetTime();

	// TODO: Deal with GUI resetting here!$@#") // Figure out what the hell this is about.

	int activityStarted;
	if (m_StartActivity) {
		// Need to pass in a clone of the activity because the original will be deleted and re-set during StartActivity.
		Activity* startActivityToUse = dynamic_cast<Activity*>(m_StartActivity->Clone());
		startActivityToUse->SetActivityState(Activity::ActivityState::NotStarted);
		activityStarted = StartActivity(startActivityToUse);
	} else {
		activityStarted = StartActivity(m_DefaultActivityType, m_DefaultActivityName);
	}
	g_TimerMan.PauseSim(false);
	if (activityStarted >= 0) {
		m_InActivity = true;
		return true;
	} else {
		m_InActivity = false;
		PauseActivity();
		g_ConsoleMan.SetEnabled(true);
		return false;
	}
}

void ActivityMan::EndActivity() const {
	// TODO: Set the activity pointer to nullptr so it doesn't return junk after being destructed. Do it here, or wherever works without crashing.
	if (m_Activity) {
		g_ThreadMan.GetPriorityThreadPool().wait_for_tasks();
		g_ThreadMan.GetBackgroundThreadPool().wait_for_tasks();

		m_Activity->End();
		g_ConsoleMan.PrintString("SYSTEM: Activity \"" + m_Activity->GetPresetName() + "\" was ended");
	} else {
		g_ConsoleMan.PrintString("ERROR: No Activity to end!");
	}
}

void ActivityMan::LateUpdateGlobalScripts() const {
	if (GAScripted* scriptedActivity = dynamic_cast<GAScripted*>(m_Activity.get())) {
		scriptedActivity->UpdateGlobalScripts(true);
	}
}

void ActivityMan::Update() {
	g_PerformanceMan.StartPerformanceMeasurement(PerformanceMan::ActivityUpdate);
	if (m_Activity) {
		m_Activity->Update();
	}
	g_PerformanceMan.StopPerformanceMeasurement(PerformanceMan::ActivityUpdate);
}

void ActivityMan::RenderUpdate() {
	if (m_Activity) {
		m_Activity->RenderUpdate();
	}
}