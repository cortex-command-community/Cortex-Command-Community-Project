#include "LoadingScreen.h"

#include "WindowMan.h"
#include "FrameMan.h"
#include "PresetMan.h"
#include "SceneLayer.h"
#include "Writer.h"

#include "GUI.h"
#include "AllegroScreen.h"
#include "AllegroBitmap.h"
#include "GUIInputWrapper.h"
#include "GUICollectionBox.h"
#include "GUIListBox.h"
#include "System.h"

#include "raylib/raylib.h"
#include "raylib/rlgl.h"
#include "RenderTarget.h"

using namespace RTE;

LoadingScreen::LoadingScreen()  { Clear(); }
void LoadingScreen::Clear() {
	m_LoadingBackground.reset();
	m_ProgressBitmap = nullptr;
}

void LoadingScreen::Create(AllegroScreen* guiScreen, GUIInputWrapper* guiInput, bool progressReportDisabled) {
	g_WindowMan.ClearBackbuffer();

	m_ProgressBitmap = create_bitmap_ex(32, 200, 180);

	// Create loading splash
	m_LoadingBackground = std::make_unique<StaticSceneLayer>();
	m_LoadingBackground->Create(ContentFile("Base.rte/GUIs/Title/LoadingSplash.png").GetAsBitmap(COLORCONV_NONE, false), false, Vector(), true, false, Vector(1.0F, 0));
	CenterLoadingSplash();
}

void LoadingScreen::CenterLoadingSplash() {
	m_LoadingBackground->SetOffset(Vector(static_cast<float>(((m_LoadingBackground->GetBitmap()->w - g_WindowMan.GetResX()) / 2)), 0));
}

void LoadingScreen::Destroy() {
	if (m_ProgressBitmap) {
		destroy_bitmap(m_ProgressBitmap);
	}
	Clear();
}

// gtodo kill logging, System::IsLoggingToCLI(), ::PrintLoadingToCLI
void LoadingScreen::UpdateWithProgressReport() {
	UpdateWithProgressReport_AcquireEntries();
	UpdateWithProgressReport_DrawOntoFramebuffer();
}

//gtodo see how this behaves in loading savefiles
void LoadingScreen::DrawLoadingSplash() {
	Vector corner = Vector(0, static_cast<float>((g_WindowMan.GetResY() - g_LoadingScreen.m_LoadingBackground->GetBitmap()->h) / 2));
	Box loadingSplashTargetBox(corner, static_cast<float>(g_WindowMan.GetResX()), static_cast<float>(g_LoadingScreen.m_LoadingBackground->GetBitmap()->h));
	g_WindowMan.ClearBackbuffer(false);
	g_WindowMan.GetScreenBuffer()->Begin();
	m_LoadingBackground->Draw(loadingSplashTargetBox, loadingSplashTargetBox);
}

void RTE::LoadingScreen::UpdateWithProgressReport_AcquireEntries() {
	if (System::IsSetToQuit()) {
		m_ProgressEntries.emplace_back("Quitting!");
		m_ProgressTextYOffset = 0;
		return;
	}

	static PresetMan::MLTFWorkerStruct::Status loadingStatusPrev = PresetMan::MLTFWorkerStruct::Status::NotYetStartedAnything;
	PresetMan::MLTFWorkerStruct::Status loadingStatusNew = g_PresetMan.m_MLTFWorkerStruct.status;

	switch (loadingStatusPrev) {
	case PresetMan::MLTFWorkerStruct::Status::NotYetStartedAnything:
		if (loadingStatusNew < PresetMan::MLTFWorkerStruct::Status::SetupNotDone_InitBaseRte) {
			break;
		}
		[[fallthrough]];
	case PresetMan::MLTFWorkerStruct::Status::SetupNotDone_InitBaseRte:
		if (loadingStatusNew < PresetMan::MLTFWorkerStruct::Status::SetupNotDone_InitOfficialModules) {
			break;
		}
		m_ProgressEntries.emplace_back("Initializing base modules");
		m_ProgressTextYOffset += m_ProgressTextLineSpacing;

		[[fallthrough]];
	case PresetMan::MLTFWorkerStruct::Status::SetupNotDone_InitOfficialModules:
		if (loadingStatusNew < PresetMan::MLTFWorkerStruct::Status::SetupNotDone_GatheringModFolders) {
			break;
		}
		[[fallthrough]];
	case PresetMan::MLTFWorkerStruct::Status::SetupNotDone_GatheringModFolders:
		if (loadingStatusNew < PresetMan::MLTFWorkerStruct::Status::SetupNotDone_InitModModules) {
			break;
		}
		m_ProgressEntries.emplace_back("Initializing mod modules");
		m_ProgressTextYOffset += m_ProgressTextLineSpacing;
		[[fallthrough]];
	case PresetMan::MLTFWorkerStruct::Status::SetupNotDone_InitModModules:
		if (loadingStatusNew < PresetMan::MLTFWorkerStruct::Status::SetupNotDone_InitUserdataModules) {
			break;
		}
		[[fallthrough]];
	case PresetMan::MLTFWorkerStruct::Status::SetupNotDone_InitUserdataModules:
		if (loadingStatusNew < PresetMan::MLTFWorkerStruct::Status::SetupNotDone_AllInitialized) {
			break;
		}
		m_ProgressEntries.emplace_back("Initializing userdata modules");
		m_ProgressTextYOffset += m_ProgressTextLineSpacing;
		[[fallthrough]];
	case PresetMan::MLTFWorkerStruct::Status::SetupNotDone_AllInitialized:
		if (loadingStatusNew < PresetMan::MLTFWorkerStruct::Status::SetupDoneNotYetFinalizing) {
			break;
		}
		[[fallthrough]];
	case PresetMan::MLTFWorkerStruct::Status::SetupDoneNotYetFinalizing:
		if (loadingStatusNew < PresetMan::MLTFWorkerStruct::Status::FinalizingBaseModules) {
			break;
		}
		[[fallthrough]];
	case PresetMan::MLTFWorkerStruct::Status::FinalizingBaseModules:
		if (loadingStatusNew < PresetMan::MLTFWorkerStruct::Status::FinalizingMissionsRte) {
			break;
		}
		m_ProgressEntries.emplace_back("Finalizing base modules");
		m_ProgressTextYOffset += m_ProgressTextLineSpacing;
		[[fallthrough]];
	case PresetMan::MLTFWorkerStruct::Status::FinalizingMissionsRte:
		if (loadingStatusNew < PresetMan::MLTFWorkerStruct::Status::FinalizingMods) {
			break;
		}
		m_ProgressEntries.emplace_back("Finalizing Missions.rte");
		m_ProgressTextYOffset += m_ProgressTextLineSpacing;
		[[fallthrough]];
	case PresetMan::MLTFWorkerStruct::Status::FinalizingMods:
		if (loadingStatusNew < PresetMan::MLTFWorkerStruct::Status::FinalizingUserdataModules) {
			break;
		}
		m_ProgressEntries.emplace_back("Finalizing mod modules");
		m_ProgressTextYOffset += m_ProgressTextLineSpacing;
		[[fallthrough]];
	case PresetMan::MLTFWorkerStruct::Status::FinalizingUserdataModules:
		if (loadingStatusNew < PresetMan::MLTFWorkerStruct::Status::EverythingDone) {
			break;
		}
		m_ProgressEntries.emplace_back("Finalizing userdata modules");
		m_ProgressTextYOffset += m_ProgressTextLineSpacing;
		[[fallthrough]];
	case PresetMan::MLTFWorkerStruct::Status::EverythingDone:
		m_ProgressEntries.emplace_back("Everything done!"); // Right now this doesn't have a chance to get shown
		m_ProgressTextYOffset += m_ProgressTextLineSpacing;
		break;
	default:
		RTEAbort("Bad MLTFWorkerStruct status!");
	}

	loadingStatusPrev = loadingStatusNew;
}

void RTE::LoadingScreen::UpdateWithProgressReport_DrawOntoFramebuffer() {
	DrawLoadingSplash();

	const float deltaTime = 1.0f / 60.0f;

	const int progressBitmapCenterX = m_ProgressBitmap->w / 2;
	const int progressBitmapCenterY = m_ProgressBitmap->h / 2;

	const int m_ProgressPosX = g_WindowMan.GetResX() * 0.5 - progressBitmapCenterX;
	const int m_ProgressPosY = g_WindowMan.GetResY() * 0.75 - progressBitmapCenterY;

	AllegroBitmap drawBitmap(m_ProgressBitmap); // gtodo put into initialize

	auto drawProgressText = [&drawBitmap, &progressBitmapCenterX](const std::string str, int yPos) {
		g_FrameMan.GetLargeFont(true)->DrawAligned(&drawBitmap, progressBitmapCenterX, yPos, str, GUIFont::Centre, GUIFont::Bottom);
	};

	clear(g_FrameMan.GetBackBuffer32());
	clear(m_ProgressBitmap);

	int yPos = m_ProgressBitmap->h + m_ProgressTextYOffset;
	for (int entryIt = m_ProgressEntries.size() - 1; entryIt >= 0; --entryIt) {
		auto& entry = m_ProgressEntries[entryIt];
		drawProgressText(entry.str, yPos);

		yPos -= m_ProgressTextLineSpacing;
	}

	if (m_ProgressTextYOffset > 0) {
		const float popInSpeedMult = 1.5f;
		m_ProgressTextYOffset = std::max(0.0f, m_ProgressTextYOffset - deltaTime * 60.0f * popInSpeedMult);
	}

	masked_blit(m_ProgressBitmap, g_FrameMan.GetBackBuffer32(), 0, 0, m_ProgressPosX, m_ProgressPosY, m_ProgressBitmap->w, m_ProgressBitmap->h);
}
