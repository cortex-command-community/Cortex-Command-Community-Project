#include "WindowMan.h"
#include "RTEError.h"
#include "SDL3/SDL.h"
#include "SettingsMan.h"
#include "FrameMan.h"
#include "ActivityMan.h"
#include "UInputMan.h"
#include "ConsoleMan.h"
#include "PresetMan.h"
#include "PostProcessMan.h"
#include "RenderTarget.h"
#include "GLResourceMan.h"

#include "GLCheck.h"
#include <SDL3/SDL.h>
#include "glad/gl.h"
#include "raylib/raylib.h"
#include "raylib/rlgl.h"
#include "Shader.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/epsilon.hpp"
#include "tracy/Tracy.hpp"
#include "tracy/TracyOpenGL.hpp"

#include "GUI/imgui/imgui.h"
#include "GUI/imgui/backends/imgui_impl_sdl3.h"
#include "GUI/imgui/backends/imgui_impl_opengl3.h"

#ifdef __linux__
#include "Resources/cccp.xpm"
#include <SDL3_image/SDL_image.h>
#endif

using namespace RTE;

void SDLWindowDeleter::operator()(SDL_Window* window) const { SDL_DestroyWindow(window); }
void SDLRendererDeleter::operator()(SDL_Renderer* renderer) const { SDL_DestroyRenderer(renderer); }
void SDLTextureDeleter::operator()(SDL_Texture* texture) const { SDL_DestroyTexture(texture); }

void SDLContextDeleter::operator()(SDL_GLContext context) const { SDL_GL_DestroyContext(context); }

void WindowMan::Clear() {
	m_EventQueue.clear();
	m_FocusEventsDispatchedByMovingBetweenWindows = false;
	m_FocusEventsDispatchedByDisplaySwitchIn = false;

	m_PrimaryWindow.reset();
	m_BackBuffer32Texture = 0;
	m_ScreenVAO = 0;
	m_ScreenVBO = 0;
	ClearMultiDisplayData();

	m_AnyWindowHasFocus = false;
	m_ResolutionChanged = false;

	m_NumDisplays = 0;
	m_MaxResX = 0;
	m_MaxResY = 0;
	m_ValidDisplayIndicesAndBoundsForMultiDisplayFullscreen.clear();
	m_CanMultiDisplayFullscreen = false;
	m_DisplayArrangmentLeftMostDisplayIndex = -1;
	m_DisplayArrangementLeftMostOffset = 0;
	m_DisplayArrangementLeftMostOffset = 0;

	m_PrimaryWindowDisplayIndex = 0;
	m_PrimaryWindowDisplayWidth = 0;
	m_PrimaryWindowDisplayHeight = 0;
	m_ResX = c_DefaultResX;
	m_ResY = c_DefaultResY;
	m_ResMultiplier = 1;
	m_Fullscreen = false;
	m_EnableVSync = true;
	m_UseMultiDisplays = false;
}

void WindowMan::ClearMultiDisplayData() {
	m_MultiDisplayTextureOffsets.clear();
	m_MultiDisplayProjections.clear();
	m_MultiDisplayWindows.clear();
}

WindowMan::WindowMan() {
	Clear();
}

WindowMan::~WindowMan() = default;

void WindowMan::Destroy() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();
	GL_CHECK(glDeleteTextures(1, &m_BackBuffer32Texture));
	GL_CHECK(glDeleteBuffers(1, &m_ScreenVBO));
	GL_CHECK(glDeleteVertexArrays(1, &m_ScreenVAO));
}

void WindowMan::Initialize() {
	SDL_free(SDL_GetDisplays(&m_NumDisplays));

	m_PrimaryWindowDisplayIndex = SDL_GetPrimaryDisplay();
	if (m_PrimaryWindowDisplayIndex == 0) {
		g_ConsoleMan.PrintString("ERROR: Failed to get primary display!" + std::string(SDL_GetError()));
		int count{0};
		SDL_DisplayID* displays = SDL_GetDisplays(&count);
		if (displays) {
			m_PrimaryWindowDisplayIndex = displays[0];
		} else {
			RTEAbort("No displays detetected somehow! " + std::string(SDL_GetError()));
		}
		SDL_free(displays);
	}

	SDL_Rect currentDisplayBounds{};
	SDL_GetDisplayBounds(m_PrimaryWindowDisplayIndex, &currentDisplayBounds);

	m_PrimaryWindowDisplayWidth = currentDisplayBounds.w;
	m_PrimaryWindowDisplayHeight = currentDisplayBounds.h;

	MapDisplays(false);

	ValidateResolution(m_ResX, m_ResY, m_ResMultiplier);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	CreatePrimaryWindow();
	InitializeOpenGL();

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
	io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

	ImGui::StyleColorsDark();
	ImGui_ImplSDL3_InitForOpenGL(m_PrimaryWindow.get(), m_GLContext.get());
	ImGui_ImplOpenGL3_Init("#version 330 core");
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();

	CreateBackBufferTexture();
	m_ScreenBlitShader = std::make_unique<Shader>(g_PresetMan.GetFullModulePath("Base.rte/Shaders/ScreenBlit.vert"), g_PresetMan.GetFullModulePath("Base.rte/Shaders/ScreenBlit.frag"));

	// SDL is kinda dumb about the taskbar icon so we need to poll after creating the window for it to show up, otherwise there's no icon till it starts polling in the main menu loop.
	SDL_PollEvent(nullptr);

	m_PrimaryWindowDisplayIndex = SDL_GetDisplayForWindow(m_PrimaryWindow.get());

	if (FullyCoversAllDisplays()) {
		ChangeResolutionToMultiDisplayFullscreen(m_ResMultiplier);
	} else {
		SetViewportLetterboxed();
	}

	SDL_AddEventWatch((SDL_EventFilter)WindowMan::HandleWindowExposedEvent, nullptr);
}

void WindowMan::CreatePrimaryWindow() {
	std::string windowTitle = "Cortex Command Community Project";

#ifdef DEBUG_BUILD
	windowTitle += " (Full Debug)";
#elif MIN_DEBUG_BUILD
	windowTitle += " (Min Debug)";
#elif DEBUG_RELEASE_BUILD
	windowTitle += " (Debug Release)";
#elif PROFILING_BUILD
	windowTitle += " (Profiling)";
#endif

#ifdef TARGET_MACHINE_X86
	windowTitle += " (x86)";
#endif

	int windowPosX = (m_ResX * m_ResMultiplier <= m_PrimaryWindowDisplayWidth) ? SDL_WINDOWPOS_CENTERED : (m_MaxResX - (m_ResX * m_ResMultiplier)) / 2;
	int windowPosY = SDL_WINDOWPOS_CENTERED;

	SDL_PropertiesID windowProps = SDL_CreateProperties();
	RTEAssert(windowProps, "Unable to create window properties! " + std::string(SDL_GetError()));
	SDL_SetStringProperty(windowProps, SDL_PROP_WINDOW_CREATE_TITLE_STRING, windowTitle.c_str());
	SDL_SetBooleanProperty(windowProps, SDL_PROP_WINDOW_CREATE_RESIZABLE_BOOLEAN, true);
	SDL_SetBooleanProperty(windowProps, SDL_PROP_WINDOW_CREATE_OPENGL_BOOLEAN, true);
	SDL_SetBooleanProperty(windowProps, SDL_PROP_WINDOW_CREATE_FULLSCREEN_BOOLEAN, m_Fullscreen);
	SDL_SetNumberProperty(windowProps, SDL_PROP_WINDOW_CREATE_X_NUMBER, windowPosX);
	SDL_SetNumberProperty(windowProps, SDL_PROP_WINDOW_CREATE_Y_NUMBER, windowPosY);
	SDL_SetNumberProperty(windowProps, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, m_ResX * m_ResMultiplier);
	SDL_SetNumberProperty(windowProps, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, m_ResY * m_ResMultiplier);
	m_PrimaryWindow = std::shared_ptr<SDL_Window>(SDL_CreateWindowWithProperties(windowProps), SDLWindowDeleter());
	if (!m_PrimaryWindow) {
		RTEError::ShowMessageBox("Unable to create window because:\n" + std::string(SDL_GetError()) + "!\n\nTrying to revert to defaults!");

		m_ResX = c_DefaultResX;
		m_ResY = c_DefaultResY;
		m_ResMultiplier = 1;
		g_SettingsMan.SetSettingsNeedOverwrite();

		m_PrimaryWindow = std::shared_ptr<SDL_Window>(SDL_CreateWindow(windowTitle.c_str(), m_ResX * m_ResMultiplier, m_ResY * m_ResMultiplier, SDL_WINDOW_OPENGL), SDLWindowDeleter());
		if (!m_PrimaryWindow) {
			RTEAbort("Failed to create window because:\n" + std::string(SDL_GetError()));
		}
	}

	SDL_SetWindowMinimumSize(m_PrimaryWindow.get(), c_MinResX, c_MinResY);
	SDL_GL_SwapWindow(m_PrimaryWindow.get());
	SDL_SetCursor(NULL);

	if (!m_Fullscreen && IsResolutionMaximized(m_ResX, m_ResY, m_ResMultiplier)) {
		SDL_MaximizeWindow(m_PrimaryWindow.get());
	}

#ifdef __linux__
	SDL_Surface* iconSurface = IMG_ReadXPMFromArray(ccicon);
	if (iconSurface) {
		SDL_SetWindowIcon(m_PrimaryWindow.get(), iconSurface);
		SDL_DestroySurface(iconSurface);
	}
#endif
}

void WindowMan::InitializeOpenGL() {
	m_GLContext = std::unique_ptr<SDL_GLContextState, SDLContextDeleter>(SDL_GL_CreateContext(m_PrimaryWindow.get()));

	if (!m_GLContext) {
		RTEAbort("Failed to create OpenGL context because:\n" + std::string(SDL_GetError()));
	}

	if (!gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress)) {
		RTEAbort("Failed to load GL functions!");
	}

#ifndef _WIN32
	SDL_GL_SetSwapInterval(m_EnableVSync ? 1 : 0);
#else
	SDL_GL_SetSwapInterval(m_Fullscreen && m_EnableVSync ? 1 : 0);
#endif

	rlLoadExtensions((void*)SDL_GL_GetProcAddress);
	rlglInit(m_ResX, m_ResY);

	GL_CHECK(glEnable(GL_BLEND));
	GL_CHECK(glEnable(GL_DEPTH_TEST));
	GL_CHECK(glGenBuffers(1, &m_ScreenVBO));
	GL_CHECK(glGenVertexArrays(1, &m_ScreenVAO));
	GL_CHECK(glBindVertexArray(m_ScreenVAO));
	GL_CHECK(glGenTextures(1, &m_BackBuffer32Texture));
	TracyGpuContext;
	Texture2D shapesTexture = {rlGetTextureIdDefault(), 1, 1, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
	SetShapesTexture(shapesTexture, {0.0f, 0.0f, 1.0f, 1.0f});
}

void WindowMan::CreateBackBufferTexture() {
	m_ScreenBuffer = std::make_unique<RenderTarget>(FloatRect(0, 0, m_ResX, m_ResY), FloatRect(0, 0, m_ResX, m_ResY));
	GL_CHECK(glBindTexture(GL_TEXTURE_2D, m_BackBuffer32Texture));
	GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_ResX, m_ResY, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));
	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
}

int WindowMan::GetWindowResX() {
	int w, h;
	SDL_GetWindowSizeInPixels(m_PrimaryWindow.get(), &w, &h);
	return w;
}

int WindowMan::GetWindowResY() {
	int w, h;
	SDL_GetWindowSizeInPixels(m_PrimaryWindow.get(), &w, &h);
	return h;
}

void WindowMan::SetVSyncEnabled(bool enable) {
	m_EnableVSync = enable;

	// Workaround for DWM frame stutter
	// See https://github.com/libsdl-org/SDL/issues/5797
#ifndef _WIN32
	int sdlEnableVSync = m_EnableVSync ? 1 : 0;
#else
	int sdlEnableVSync = m_Fullscreen && m_EnableVSync ? 1 : 0;
#endif

	SDL_GL_SetSwapInterval(sdlEnableVSync);
}

void WindowMan::RefocusWindow() const {
	SDL_RaiseWindow(m_PrimaryWindow.get());
}

void WindowMan::UpdatePrimaryDisplayInfo() {
	m_PrimaryWindowDisplayIndex = SDL_GetDisplayForWindow(m_PrimaryWindow.get());

	SDL_Rect currentDisplayBounds;
	SDL_GetDisplayBounds(m_PrimaryWindowDisplayIndex, &currentDisplayBounds);

	m_PrimaryWindowDisplayWidth = currentDisplayBounds.w;
	m_PrimaryWindowDisplayHeight = currentDisplayBounds.h;
}

SDL_Rect WindowMan::GetUsableBoundsWithDecorations(int display) {
	if (m_Fullscreen) {
		SDL_Rect displayBounds;
		SDL_GetDisplayBounds(display, &displayBounds);
		return displayBounds;
	}
	SDL_Rect displayBounds;
	SDL_GetDisplayUsableBounds(display, &displayBounds);

	int top, left, bottom, right;
	SDL_GetWindowBordersSize(m_PrimaryWindow.get(), &top, &left, &bottom, &right);
	displayBounds.x += left;
	displayBounds.y += top;
	displayBounds.w -= left + right;
	displayBounds.h -= top + bottom;
	return displayBounds;
}

bool WindowMan::IsResolutionMaximized(int resX, int resY, float resMultiplier) {
	SDL_Rect displayBounds = GetUsableBoundsWithDecorations(m_PrimaryWindowDisplayIndex);
	if (resMultiplier == 1) {
		return (resX == displayBounds.w) && (resY == displayBounds.h);
	} else {
		return glm::epsilonEqual<float>(resX * resMultiplier, displayBounds.w, resMultiplier) && glm::epsilonEqual<float>(resY * resMultiplier, displayBounds.h, resMultiplier);
	}
}

void WindowMan::MapDisplays(bool updatePrimaryDisplayInfo) {
	auto setSingleDisplayMode = [this](const std::string& errorMsg = "") {
		m_MaxResX = m_PrimaryWindowDisplayWidth;
		m_MaxResY = m_PrimaryWindowDisplayHeight;
		m_MaxResMultiplier = std::min<float>(m_MaxResX / static_cast<float>(c_MinResX), m_MaxResY / static_cast<float>(c_MinResY));
		m_NumDisplays = 1;
		m_DisplayArrangementLeftMostOffset = -1;
		m_DisplayArrangementTopMostOffset = -1;
		m_ValidDisplayIndicesAndBoundsForMultiDisplayFullscreen.clear();
		m_CanMultiDisplayFullscreen = false;
		m_UseMultiDisplays = false;
		if (!errorMsg.empty()) {
			RTEError::ShowMessageBox("Failed to map displays for multi-display fullscreen because:\n\n" + errorMsg + "!\n\nFullscreen will be limited to the display the window is positioned at!");
		}
	};

	if (updatePrimaryDisplayInfo) {
		UpdatePrimaryDisplayInfo();
	}

	SDL_DisplayID* displays = SDL_GetDisplays(&m_NumDisplays);

	if (!m_UseMultiDisplays || m_NumDisplays == 1) {
		setSingleDisplayMode();
		return;
	}

	m_ValidDisplayIndicesAndBoundsForMultiDisplayFullscreen.clear();

	int leftMostOffset = 0;
	int topMostOffset = std::numeric_limits<int>::max();
	int maxHeight = std::numeric_limits<int>::min();
	int totalWidth = 0;

	for (int i = 0; i < m_NumDisplays; ++i) {
		SDL_Rect displayBounds;
		if (SDL_GetDisplayBounds(displays[i], &displayBounds)) {
			m_ValidDisplayIndicesAndBoundsForMultiDisplayFullscreen.emplace_back(displays[i], displayBounds);

			leftMostOffset = std::min(leftMostOffset, displayBounds.x);
			topMostOffset = std::min(topMostOffset, displayBounds.y);
			maxHeight = std::max(maxHeight, displayBounds.h);

			totalWidth += displayBounds.w;
		} else {
			setSingleDisplayMode("Failed to get resolution of display " + std::to_string(displays[i]) + "!");
			return;
		}
	}

	if (m_ValidDisplayIndicesAndBoundsForMultiDisplayFullscreen.size() == 1) {
		setSingleDisplayMode("Somehow ended up with only one valid display even though " + std::to_string(m_NumDisplays) + " are available!");
		return;
	}

	std::stable_sort(m_ValidDisplayIndicesAndBoundsForMultiDisplayFullscreen.begin(), m_ValidDisplayIndicesAndBoundsForMultiDisplayFullscreen.end(),
	                 [](auto left, auto right) {
		                 return left.second.x < right.second.x;
	                 });

	for (const auto& [displayIndex, displayBounds]: m_ValidDisplayIndicesAndBoundsForMultiDisplayFullscreen) {
		// Translate display offsets to backbuffer offsets, where the top left corner is (0,0) to figure out if the display arrangement is unreasonable garbage, i.e not top or bottom edge aligned.
		// If any of the translated offsets ends up negative, or over-positive for the Y offset, disallow going into multi-display fullscreen
		// because we'll just end up with an access violation when trying to read from the backbuffer pixel array during rendering.
		// In odd display size arrangements this is valid as long as the misaligned display is somewhere between the top and bottom edge of the tallest display, as the translated offset will remain in bounds.
		int translatedOffsetX = (displayBounds.x - leftMostOffset);
		int translatedOffsetY = (displayBounds.y - topMostOffset);
		if (translatedOffsetX < 0 || translatedOffsetY < 0 || translatedOffsetY + displayBounds.h > maxHeight) {
			setSingleDisplayMode("Bad display alignment detected!\nMulti-display fullscreen currently supports only horizontal arrangements where all displays are either top or bottom edge aligned!");
			return;
		}
	}

	for (const auto& [displayIndex, displayBounds]: m_ValidDisplayIndicesAndBoundsForMultiDisplayFullscreen) {
#if SDL_VERSION_ATLEAST(2, 24, 0)
		m_DisplayArrangmentLeftMostDisplayIndex = SDL_GetDisplayForRect(&displayBounds);
		if (m_DisplayArrangmentLeftMostDisplayIndex >= 0) {
#else
		// This doesn't return the nearest display index to the point but should still be reliable enough for reasonable display arrangements.
		SDL_Point testPoint = {leftMostOffset + 1, topMostOffset + 1};
		if (SDL_PointInRect(&testPoint, &displayBounds) == true) {
#endif
			m_DisplayArrangmentLeftMostDisplayIndex = displayIndex;
			break;
		}
	}

	if (m_DisplayArrangmentLeftMostDisplayIndex >= 0) {
		m_MaxResX = totalWidth;
		m_MaxResY = maxHeight;
		m_MaxResMultiplier = std::min<float>(m_MaxResX / static_cast<float>(c_MinResX), m_MaxResY / static_cast<float>(c_MinResY));
		m_DisplayArrangementLeftMostOffset = leftMostOffset;
		m_DisplayArrangementTopMostOffset = topMostOffset;
		m_CanMultiDisplayFullscreen = true;
	} else {
		setSingleDisplayMode("Unable to determine left-most display index!");
	}
}

void WindowMan::ValidateResolution(int& resX, int& resY, float& resMultiplier) const {
	if (resX < c_MinResX || resY < c_MinResY) {
		resX = c_MinResX;
		resY = c_MinResY;
		resMultiplier = 1.0f;
		RTEError::ShowMessageBox("Resolution too low, overriding to fit!");
		g_SettingsMan.SetSettingsNeedOverwrite();
	} else if (resMultiplier > m_MaxResMultiplier) {
		resMultiplier = 1.0f;
		RTEError::ShowMessageBox("Resolution multiplier too high, overriding to fit!");
		g_SettingsMan.SetSettingsNeedOverwrite();
	}
}

void WindowMan::SetViewportLetterboxed() {
	int windowW, windowH;
	SDL_GetWindowSizeInPixels(m_PrimaryWindow.get(), &windowW, &windowH);
	double aspectRatio = m_ResX / static_cast<double>(m_ResY);
	int width = windowW;
	int height = (windowW / aspectRatio) + 0.5F;

	if (height > windowH) {
		height = windowH;
		width = (height * aspectRatio) + 0.5F;
	}

	m_ResMultiplier = width / static_cast<float>(m_ResX);

	int offsetX = (windowW / 2) - (width / 2);
	int offsetY = (windowH / 2) - (height / 2);
	m_PrimaryWindowViewport = std::make_unique<SDL_Rect>(offsetX, windowH - offsetY - height, width, height);
}

void WindowMan::AttemptToRevertToPreviousResolution(bool revertToDefaults) {
	auto setDefaultResSettings = [this]() {
		m_ResX = c_DefaultResX;
		m_ResY = c_DefaultResY;
		m_ResMultiplier = 1;
		g_SettingsMan.UpdateSettingsFile();
	};

	bool fullscreen = true;

	if ((m_ResX * m_ResMultiplier >= m_MaxResX) && (m_ResY * m_ResMultiplier >= m_MaxResY)) {
		setDefaultResSettings();
		fullscreen = false;
	}
	SDL_SetWindowSize(m_PrimaryWindow.get(), m_ResX * m_ResMultiplier, m_ResY * m_ResMultiplier);

	if (!m_Fullscreen) {
		fullscreen = false;
		SDL_SetWindowBordered(m_PrimaryWindow.get(), true);
		SDL_SetWindowPosition(m_PrimaryWindow.get(), SDL_WINDOWPOS_CENTERED_DISPLAY(m_PrimaryWindowDisplayIndex), SDL_WINDOWPOS_CENTERED_DISPLAY(m_PrimaryWindowDisplayIndex));
	}

	bool result = SDL_SetWindowFullscreen(m_PrimaryWindow.get(), fullscreen) == 0;
	if (!result && !revertToDefaults) {
		RTEError::ShowMessageBox("Failed to revert to previous resolution settings!\nAttempting to revert to defaults!");
		setDefaultResSettings();
		AttemptToRevertToPreviousResolution(true);
	} else if (!result) {
		RTEAbort("Failed to revert to previous resolution or defaults because: \n" + std::string(SDL_GetError()) + "!");
	}
}

void WindowMan::ChangeResolution(int newResX, int newResY, float newResMultiplier, bool fullscreen, bool displaysAlreadyMapped) {

	if (m_ResX == newResX && m_ResY == newResY && glm::epsilonEqual(m_ResMultiplier, newResMultiplier, glm::epsilon<float>()) && m_Fullscreen == fullscreen) {
		return;
	}

	bool onlyResMultiplierChange = (m_ResX == newResX) && (m_ResY == newResY);

	SDL_GL_MakeCurrent(m_PrimaryWindow.get(), m_GLContext.get());
	ClearMultiDisplayData();

	if (!displaysAlreadyMapped) {
		MapDisplays();
	}
	ValidateResolution(newResX, newResY, newResMultiplier);

	bool newResFullyCoversAllDisplays = fullscreen && m_UseMultiDisplays && m_CanMultiDisplayFullscreen && (m_NumDisplays > 1);

	bool recoveredToPreviousSettings = false;

	if ((newResFullyCoversAllDisplays && !ChangeResolutionToMultiDisplayFullscreen(newResMultiplier)) || (fullscreen && !SDL_SetWindowFullscreen(m_PrimaryWindow.get(), true))) {
		RTEError::ShowMessageBox("Failed to switch to new resolution!\nAttempting to revert to previous settings!");
		AttemptToRevertToPreviousResolution();
		recoveredToPreviousSettings = true;
	} else if (!fullscreen && !newResFullyCoversAllDisplays) {
		SDL_SetWindowFullscreen(m_PrimaryWindow.get(), 0);
		if (IsResolutionMaximized(newResX, newResY, newResMultiplier)) {
			SDL_MaximizeWindow(m_PrimaryWindow.get());
		} else {
			SDL_RestoreWindow(m_PrimaryWindow.get());
			SDL_GL_SwapWindow(m_PrimaryWindow.get());
		}
		SDL_SetWindowSize(m_PrimaryWindow.get(), newResX * newResMultiplier, newResY * newResMultiplier);
		SDL_SetWindowBordered(m_PrimaryWindow.get(), true);
		SDL_SetWindowPosition(m_PrimaryWindow.get(), SDL_WINDOWPOS_CENTERED_DISPLAY(m_PrimaryWindowDisplayIndex), SDL_WINDOWPOS_CENTERED_DISPLAY(m_PrimaryWindowDisplayIndex));
		SDL_SetWindowMinimumSize(m_PrimaryWindow.get(), c_MinResX, c_MinResY);
	}
	if (!recoveredToPreviousSettings) {
		m_ResX = newResX;
		m_ResY = newResY;
		m_ResMultiplier = newResMultiplier;
		m_Fullscreen = fullscreen;
		g_SettingsMan.UpdateSettingsFile();
	}

	if (onlyResMultiplierChange) {
		if (!newResFullyCoversAllDisplays) {
			SetViewportLetterboxed();
			CreateBackBufferTexture();
		}
		g_ConsoleMan.PrintString("SYSTEM: Switched to different resolution multiplier.");
	} else {
		m_ResolutionChanged = true;
		g_FrameMan.CreateBackBuffers();
		g_PostProcessMan.CreateGLBackBuffers();
		SetViewportLetterboxed();
		CreateBackBufferTexture();
	}
#ifdef _WIN32
	SDL_GL_SetSwapInterval(m_Fullscreen && m_EnableVSync ? 1 : 0);
#endif
	g_ConsoleMan.PrintString("SYSTEM: " + std::string(!recoveredToPreviousSettings ? "Switched to different resolution." : "Failed to switch to different resolution. Reverted to previous settings."));
}

void WindowMan::ToggleFullscreen() {
	bool fullscreen = !m_Fullscreen;

	MapDisplays();

	if (fullscreen && m_UseMultiDisplays && m_CanMultiDisplayFullscreen && (m_NumDisplays > 1)) {
		double aspectRatio = m_ResX / static_cast<double>(m_ResY);
		double maxAspectRatio = m_MaxResX / static_cast<double>(m_MaxResY);
		if (glm::epsilonNotEqual(aspectRatio, maxAspectRatio, glm::epsilon<double>())) {
			RTEError::ShowMessageBox("Switching to multi display fullscreen would result in letterboxing, please disable multiple displays in settings or switch to fullscreen manually!");
			return;
		}
		ChangeResolution(m_ResX, m_ResY, m_ResMultiplier, fullscreen, true);
	}

	if (!fullscreen) {
		SDL_SetWindowFullscreen(m_PrimaryWindow.get(), 0);
		SDL_SetWindowMinimumSize(m_PrimaryWindow.get(), c_MinResX, c_MinResY);
	} else {
		SDL_SetWindowFullscreen(m_PrimaryWindow.get(), true);
	}
	m_Fullscreen = fullscreen;

#ifdef _WIN32
	SDL_GL_SetSwapInterval(m_Fullscreen && m_EnableVSync ? 1 : 0);
#endif
	SetViewportLetterboxed();
}

bool WindowMan::ChangeResolutionToMultiDisplayFullscreen(float resMultiplier) {
	if (!m_CanMultiDisplayFullscreen) {
		return false;
	}
	int windowPrevPositionX = 0;
	int windowPrevPositionY = 0;
	SDL_GetWindowPosition(m_PrimaryWindow.get(), &windowPrevPositionX, &windowPrevPositionY);

	// Move the window to the detected leftmost display to avoid all the headaches.
	if (m_PrimaryWindowDisplayIndex != m_DisplayArrangmentLeftMostDisplayIndex) {
		SDL_SetWindowPosition(m_PrimaryWindow.get(), SDL_WINDOWPOS_CENTERED_DISPLAY(m_DisplayArrangmentLeftMostDisplayIndex), SDL_WINDOWPOS_CENTERED_DISPLAY(m_DisplayArrangmentLeftMostDisplayIndex));
		m_PrimaryWindowDisplayIndex = m_DisplayArrangmentLeftMostDisplayIndex;
	}

	bool errorSettingFullscreen = false;

	for (const auto& [displayIndex, displayBounds]: m_ValidDisplayIndicesAndBoundsForMultiDisplayFullscreen) {
		int displayOffsetX = displayBounds.x;
		int displayOffsetY = displayBounds.y;
		int displayWidth = displayBounds.w;
		int displayHeight = displayBounds.h;

		SDL_PropertiesID windowProps = SDL_CreateProperties();
		RTEAssert(windowProps, "Failed to create properties!" + std::string(SDL_GetError()));
		if (displayIndex == m_PrimaryWindowDisplayIndex) {
			m_MultiDisplayWindows.emplace_back(m_PrimaryWindow);
		} else {
			SDL_SetNumberProperty(windowProps, SDL_PROP_WINDOW_CREATE_X_NUMBER, displayOffsetX);
			SDL_SetNumberProperty(windowProps, SDL_PROP_WINDOW_CREATE_Y_NUMBER, displayOffsetY);
			SDL_SetNumberProperty(windowProps, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, displayHeight);
			SDL_SetNumberProperty(windowProps, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, displayWidth);
			SDL_SetBooleanProperty(windowProps, SDL_PROP_WINDOW_CREATE_OPENGL_BOOLEAN, true);
			SDL_SetBooleanProperty(windowProps, SDL_PROP_WINDOW_CREATE_FULLSCREEN_BOOLEAN, true);
			SDL_SetBooleanProperty(windowProps, SDL_PROP_WINDOW_CREATE_UTILITY_BOOLEAN, true);

			m_MultiDisplayWindows.emplace_back(SDL_CreateWindowWithProperties(windowProps), SDLWindowDeleter());
			if (m_MultiDisplayWindows.back()) {
			} else {
				errorSettingFullscreen = true;
			}
			if (errorSettingFullscreen) {
				break;
			}
		}

		int textureOffsetX = (displayOffsetX - m_DisplayArrangementLeftMostOffset);
		int textureOffsetY = (displayOffsetY - m_DisplayArrangementTopMostOffset);

		glm::mat4 textureOffset = glm::translate(glm::mat4(1), {m_DisplayArrangementLeftMostOffset, m_DisplayArrangementTopMostOffset, 0.0f});
		textureOffset = glm::scale(textureOffset, {m_MaxResX * 0.5f, m_MaxResY * 0.5f, 1.0f});
		textureOffset = glm::translate(textureOffset, {1.0f, 1.0f, 0.0f}); // Shift the quad so we're scaling from top left instead of center.

		m_MultiDisplayTextureOffsets.emplace_back(textureOffset);
		glm::mat4 projection = glm::ortho(static_cast<float>(textureOffsetX), static_cast<float>(textureOffsetX + displayWidth), static_cast<float>(textureOffsetY), static_cast<float>(textureOffsetY + displayHeight), -1.0f, 1.0f);
		m_MultiDisplayProjections.emplace_back(projection);
	}

	if (errorSettingFullscreen) {
		ClearMultiDisplayData();
		SDL_SetWindowPosition(m_PrimaryWindow.get(), windowPrevPositionX, windowPrevPositionY);
		return false;
	}

	SDL_SetWindowFullscreen(m_PrimaryWindow.get(), true);
	return true;
}

void WindowMan::DisplaySwitchIn(SDL_Window* windowThatShouldTakeInputFocus) const {
	g_UInputMan.DisableMouseMoving(false);
	g_UInputMan.DisableKeys(false);

	if (!m_MultiDisplayWindows.empty()) {
		for (const auto& window: m_MultiDisplayWindows) {
			SDL_RaiseWindow(window.get());
		}
		SDL_RaiseWindow(windowThatShouldTakeInputFocus);
	} else {
		SDL_RaiseWindow(m_PrimaryWindow.get());
	}

	SDL_HideCursor();
}

void WindowMan::DisplaySwitchOut() const {
	g_UInputMan.DisableMouseMoving(true);
	g_UInputMan.DisableKeys(true);

	SDL_ShowCursor();
	// Sometimes the cursor will not be visible after disabling relative mode. Setting it to nullptr forces it to redraw, though this doesn't always work either.
	SDL_SetCursor(nullptr);
}

bool WindowMan::HandleWindowExposedEvent(void *userdata, SDL_Event *event) {
	if (event->type == SDL_EVENT_WINDOW_EXPOSED) {
		g_WindowMan.SetViewportLetterboxed();
		g_WindowMan.ClearBackbuffer(false);
		g_WindowMan.UploadFrame();
	}

	return true;
}

void WindowMan::QueueWindowEvent(const SDL_Event& windowEvent) {
	m_EventQueue.emplace_back(windowEvent);
}

void WindowMan::Update() {
	// Some bullshit we have to deal with to correctly focus windows in multi-display fullscreen so mouse binding/unbinding works correctly. Not relevant for single window.
	// This is SDL's fault for not having handling to raise a window so it's top-most without taking focus of it.
	// Don't process any focus events this update if either of these has been set in the previous one.
	// Having two flags is a bit redundant but better be safe than recursively raising windows and taking focus and pretty much locking up your whole shit so the only thing you can do about it is sign out to terminate this.
	// Clearing the queue here is just us not handling the events on our end. Whatever they are and do and wherever they propagate to is handled by SDL_PollEvent earlier.
	if (m_FocusEventsDispatchedByDisplaySwitchIn || m_FocusEventsDispatchedByMovingBetweenWindows) {
		m_EventQueue.clear();

		m_FocusEventsDispatchedByDisplaySwitchIn = false;
		m_FocusEventsDispatchedByMovingBetweenWindows = false;
		return;
	}

	SDL_Event windowEvent;
	for (std::vector<SDL_Event>::const_iterator eventIterator = m_EventQueue.begin(); eventIterator != m_EventQueue.end(); eventIterator++) {
		windowEvent = *eventIterator;
		int windowID = windowEvent.window.windowID;

		switch (windowEvent.type) {
			case SDL_EVENT_WINDOW_MOUSE_ENTER:
				if (SDL_GetWindowID(SDL_GetMouseFocus()) > 0 && m_AnyWindowHasFocus && FullyCoversAllDisplays()) {
					for (const auto& window: m_MultiDisplayWindows) {
						SDL_RaiseWindow(window.get());
					}
					SDL_RaiseWindow(SDL_GetWindowFromID(windowID));
					m_AnyWindowHasFocus = true;
					m_FocusEventsDispatchedByMovingBetweenWindows = true;
				}
				break;
			case SDL_EVENT_WINDOW_FOCUS_GAINED:
				DisplaySwitchIn(SDL_GetWindowFromID(windowID));
				m_AnyWindowHasFocus = true;
				m_FocusEventsDispatchedByDisplaySwitchIn = true;
				break;
			case SDL_EVENT_WINDOW_FOCUS_LOST:
				DisplaySwitchOut();
				m_AnyWindowHasFocus = false;
				m_FocusEventsDispatchedByDisplaySwitchIn = false;
				m_FocusEventsDispatchedByMovingBetweenWindows = false;
				break;
			case SDL_EVENT_WINDOW_RESIZED:
			case SDL_WINDOW_MAXIMIZED:
				SetViewportLetterboxed();
				break;
			default:
				break;
		}
	}
	m_EventQueue.clear();
}

void WindowMan::ClearBackbuffer(bool clearFrameMan) {
	GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
	if (clearFrameMan) {
		g_FrameMan.ClearBackBuffer32();
	}
	GL_CHECK(glActiveTexture(GL_TEXTURE0));
	GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
	GL_CHECK(glActiveTexture(GL_TEXTURE1));
	GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
	m_DrawPostProcessBuffer = false;
}

void WindowMan::UploadFrame() {

	m_ScreenBuffer->Begin(g_ActivityMan.IsInActivity());

	rlDisableDepthTest();
	rlDisableColorBlend();
	//rlSetBlendMode(RL_BLEND_ALPHA);

	GL_CHECK(glBindTexture(GL_TEXTURE_2D, m_BackBuffer32Texture));
	GL_CHECK(glPixelStorei(GL_UNPACK_ALIGNMENT, 4));
	GL_CHECK(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, g_FrameMan.GetBackBuffer32()->w, g_FrameMan.GetBackBuffer32()->h, GL_RGBA, GL_UNSIGNED_BYTE, g_FrameMan.GetBackBuffer32()->line[0]));

	m_ScreenBlitShader->Begin();
	rlSetUniformSampler(m_ScreenBlitShader->GetUniformLocation("rteGUITexture"), m_BackBuffer32Texture);
	if (m_DrawPostProcessBuffer) {
		Texture2D postBuffer = g_PostProcessMan.GetPostProcessColorBuffer()->GetColorTexture();
		DrawTextureRec(postBuffer, Rectangle(0.0f, 0.0f, postBuffer.width, -postBuffer.height), {0.0f, 0.0f}, {255, 255, 255, 255});
	} else {
		Texture2D empty(rlGetTextureIdDefault(), m_ResX, m_ResY, 1, 0);
		//rlSetUniformSampler(m_ScreenBlitShader->GetTextureUniform(), 0);
		DrawTextureRec(m_ScreenBuffer->GetColorTexture(), {0.0f, 0.0f, static_cast<float>(m_ResX), -static_cast<float>(m_ResY)}, {0, 0}, {255, 255, 255, 255});
	}
	m_ScreenBlitShader->End();
	m_ScreenBuffer->End();

	rlDisableColorBlend();
	if (m_MultiDisplayWindows.empty()) {
		rlMatrixMode(RL_PROJECTION);
		rlLoadIdentity();
		rlOrtho(0, m_ResX, m_ResY, 0, 0.0, 1.0);
		rlMatrixMode(RL_MODELVIEW);
		rlLoadIdentity();
		GL_CHECK(glViewport(m_PrimaryWindowViewport->x, m_PrimaryWindowViewport->y, m_PrimaryWindowViewport->w, m_PrimaryWindowViewport->h));
		DrawTextureRec(m_ScreenBuffer->GetColorTexture(), {0.0f, 0.0f, static_cast<float>(m_ResX), static_cast<float>(-m_ResY)}, {0.0f, 0.0f}, {255, 255, 255, 255});
		rlDrawRenderBatchActive();
	} else {
		for (size_t i = 0; i < m_MultiDisplayWindows.size(); ++i) {
			SDL_GL_MakeCurrent(m_MultiDisplayWindows.at(i).get(), m_GLContext.get());
			int windowW, windowH;

			SDL_GetWindowSizeInPixels(m_MultiDisplayWindows.at(i).get(), &windowW, &windowH);
			GL_CHECK(glViewport(0, 0, windowW, windowH));

			rlMatrixMode(RL_PROJECTION);
			rlLoadIdentity();
			rlOrtho(0, windowW, windowH, 0, -1.0, 1.0);

			DrawTexture(m_ScreenBuffer->GetColorTexture(), 0, 0, {255, 255, 255, 255});
			rlDrawRenderBatchActive();
		}
	}
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	Present();
	TracyGpuCollect;
	FrameMark;
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();
}

void WindowMan::Present() {
	if (m_MultiDisplayWindows.empty()) {
		SDL_GL_SwapWindow(m_PrimaryWindow.get());
	} else {
		for (size_t i = 0; i < m_MultiDisplayWindows.size(); ++i) {
			SDL_GL_MakeCurrent(m_MultiDisplayWindows.at(i).get(), m_GLContext.get());
			SDL_GL_SwapWindow(m_MultiDisplayWindows.at(i).get());
		}
	}
}
