#include "LuaMan.h"

#include "SolObjectWrapper.h"
#include "LuaBindingRegisterDefinitions.h"
#include "ThreadMan.h"

#include "tracy/Tracy.hpp"
#include "tracy/TracyLua.hpp"
#include "LuaAdapterDefinitions.h"

// We need to include this crap because std::unique_ptr needs the full implementation to know how to delete things...
#include "Entity.h"
#include "ACDropShip.h"
#include "ACrab.h"
#include "ACraft.h"
#include "ACRocket.h"
#include "Actor.h"
#include "ADoor.h"
#include "AEmitter.h"
#include "AEJetpack.h"
#include "AHuman.h"
#include "Arm.h"
#include "AtomGroup.h"
#include "Attachable.h"
#include "Deployment.h"
#include "Emission.h"
#include "Gib.h"
#include "GlobalScript.h"
#include "HDFirearm.h"
#include "HeldDevice.h"
#include "Leg.h"
#include "LimbPath.h"
#include "Magazine.h"
#include "Material.h"
#include "MetaPlayer.h"
#include "MOSParticle.h"
#include "MOPixel.h"
#include "MOSprite.h"
#include "MOSRotating.h"
#include "MovableObject.h"
#include "PEmitter.h"
#include "Round.h"
#include "Scene.h"
#include "SceneObject.h"
#include "SLBackground.h"
#include "SLTerrain.h"
#include "SoundContainer.h"
#include "TerrainObject.h"
#include "TDExplosive.h"
#include "ThrownDevice.h"
#include "Turret.h"
#include "PieMenu.h"
#include "PieSlice.h"

#include "ActivityMan.h"
#include "AudioMan.h"
#include "CameraMan.h"
#include "ConsoleMan.h"
#include "FrameMan.h"
#include "MetaMan.h"
#include "MovableMan.h"
#include "PostProcessMan.h"
#include "PresetMan.h"
#include "PrimitiveMan.h"
#include "SceneMan.h"
#include "SettingsMan.h"
#include "TimerMan.h"
#include "UInputMan.h"

#include "sol/sol.hpp"

namespace RTE {

	const std::unordered_set<std::string> LuaMan::c_FileAccessModes = { "r", "r+", "w", "w+", "a", "a+", "rt", "wt"};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaStateWrapper::Clear() {
		m_State = nullptr;
		m_TempEntity = nullptr;
		m_TempEntityVector.clear();
		m_LastError.clear();
		m_CurrentlyRunningScriptPath = "";
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaStateWrapper::Initialize() {
		m_State = luaL_newstate();
		tracy::LuaRegister(m_State);

		// Disable gc. We do this manually, so we can thread it to occur parallel with non-lua updates
		lua_gc(m_State, LUA_GCSTOP, 0);

		const luaL_Reg libsToLoad[] = {
			{ LUA_COLIBNAME, luaopen_base },
			{ LUA_LOADLIBNAME, luaopen_package },
			{ LUA_TABLIBNAME, luaopen_table },
			{ LUA_STRLIBNAME, luaopen_string },
			{ LUA_MATHLIBNAME, luaopen_math },
			{ LUA_DBLIBNAME, luaopen_debug },
			{ LUA_JITLIBNAME, luaopen_jit },
			{ NULL, NULL } // End of array
		};

		for (const luaL_Reg *lib = libsToLoad; lib->func; lib++) {
			if (g_SettingsMan.DisableLuaJIT() && strcmp(lib->name, LUA_JITLIBNAME) == 0) {
				continue;
			}
			lua_pushcfunction(m_State, lib->func);
			lua_pushstring(m_State, lib->name);
			lua_call(m_State, 1, 0);
		}

		// LuaJIT should start automatically after we load the library (if we loaded it) but we're making sure it did anyway.
		if (!g_SettingsMan.DisableLuaJIT() && !luaJIT_setmode(m_State, 0, LUAJIT_MODE_ENGINE | LUAJIT_MODE_ON)) { RTEAbort("Failed to initialize LuaJIT!\nIf this error persists, please disable LuaJIT with \"Settings.ini\" property \"DisableLuaJIT\"."); }

		// Sol bindings
		sol::state_view solState(m_State);

		// Register our lua manager
		sol::usertype<LuaStateWrapper> usertype = solState.new_usertype<LuaStateWrapper>("LuaManager", sol::no_constructor);
		usertype["TempEntity"] = sol::property(&LuaStateWrapper::GetTempEntity);
		usertype["TempEntities"] = sol::readonly(&LuaStateWrapper::m_TempEntityVector);
		usertype["SelectRand"] = &LuaStateWrapper::SelectRand;
		usertype["RangeRand"] = &LuaStateWrapper::RangeRand;
		usertype["PosRand"] = &LuaStateWrapper::PosRand;
		usertype["NormalRand"] = &LuaStateWrapper::NormalRand;
		usertype["GetDirectoryList"] = &LuaStateWrapper::DirectoryList;
		usertype["GetFileList"] = &LuaStateWrapper::FileList;
		usertype["FileExists"] = &LuaStateWrapper::FileExists;
		usertype["FileOpen"] = &LuaStateWrapper::FileOpen;
		usertype["FileClose"] = &LuaStateWrapper::FileClose;
		usertype["FileReadLine"] = &LuaStateWrapper::FileReadLine;
		usertype["FileWriteLine"] = &LuaStateWrapper::FileWriteLine;
		usertype["FileEOF"] = &LuaStateWrapper::FileEOF;

		// Register all relevant bindings to the state. Note that the order of registration is important, as bindings can't derive from an unregistered type (inheritance and all that).
		RegisterLuaBindingsOfType(solState, SystemLuaBindings, Vector);
		RegisterLuaBindingsOfType(solState, SystemLuaBindings, Box);
		RegisterLuaBindingsOfType(solState, EntityLuaBindings, Entity);
		RegisterLuaBindingsOfConcreteType(solState, EntityLuaBindings, SoundContainer);
		RegisterLuaBindingsOfType(solState, EntityLuaBindings, SoundSet);
		RegisterLuaBindingsOfType(solState, EntityLuaBindings, LimbPath);
		RegisterLuaBindingsOfAbstractType(solState, EntityLuaBindings, SceneObject);
		RegisterLuaBindingsOfAbstractType(solState, EntityLuaBindings, MovableObject);
		RegisterLuaBindingsOfType(solState, EntityLuaBindings, Material);
		RegisterLuaBindingsOfConcreteType(solState, EntityLuaBindings, MOPixel);
		RegisterLuaBindingsOfConcreteType(solState, EntityLuaBindings, TerrainObject);
		RegisterLuaBindingsOfAbstractType(solState, EntityLuaBindings, MOSprite);
		RegisterLuaBindingsOfConcreteType(solState, EntityLuaBindings, MOSParticle);
		RegisterLuaBindingsOfConcreteType(solState, EntityLuaBindings, MOSRotating);
		RegisterLuaBindingsOfConcreteType(solState, EntityLuaBindings, Attachable);
		RegisterLuaBindingsOfAbstractType(solState, EntityLuaBindings, Emission);
		RegisterLuaBindingsOfConcreteType(solState, EntityLuaBindings, AEmitter);
		RegisterLuaBindingsOfConcreteType(solState, EntityLuaBindings, AEJetpack);
		RegisterLuaBindingsOfConcreteType(solState, EntityLuaBindings, PEmitter);
		RegisterLuaBindingsOfConcreteType(solState, EntityLuaBindings, Actor);
		RegisterLuaBindingsOfConcreteType(solState, EntityLuaBindings, ADoor);
		RegisterLuaBindingsOfConcreteType(solState, EntityLuaBindings, Arm);
		RegisterLuaBindingsOfConcreteType(solState, EntityLuaBindings, Leg);
		RegisterLuaBindingsOfConcreteType(solState, EntityLuaBindings, AHuman);
		RegisterLuaBindingsOfConcreteType(solState, EntityLuaBindings, ACrab);
		RegisterLuaBindingsOfConcreteType(solState, EntityLuaBindings, Turret);
		RegisterLuaBindingsOfAbstractType(solState, EntityLuaBindings, ACraft);
		RegisterLuaBindingsOfConcreteType(solState, EntityLuaBindings, ACDropShip);
		RegisterLuaBindingsOfConcreteType(solState, EntityLuaBindings, ACRocket);
		RegisterLuaBindingsOfConcreteType(solState, EntityLuaBindings, HeldDevice);
		RegisterLuaBindingsOfConcreteType(solState, EntityLuaBindings, Magazine);
		RegisterLuaBindingsOfConcreteType(solState, EntityLuaBindings, Round);
		RegisterLuaBindingsOfConcreteType(solState, EntityLuaBindings, HDFirearm);
		RegisterLuaBindingsOfConcreteType(solState, EntityLuaBindings, ThrownDevice);
		RegisterLuaBindingsOfConcreteType(solState, EntityLuaBindings, TDExplosive);
		RegisterLuaBindingsOfConcreteType(solState, EntityLuaBindings, PieSlice);
		RegisterLuaBindingsOfConcreteType(solState, EntityLuaBindings, PieMenu);
		RegisterLuaBindingsOfType(solState, EntityLuaBindings, Gib);
		RegisterLuaBindingsOfType(solState, SystemLuaBindings, Controller);
		RegisterLuaBindingsOfType(solState, SystemLuaBindings, Timer);
		RegisterLuaBindingsOfType(solState, SystemLuaBindings, PathRequest);
		RegisterLuaBindingsOfConcreteType(solState, EntityLuaBindings, Scene);
		RegisterLuaBindingsOfType(solState, EntityLuaBindings, SceneArea);
		RegisterLuaBindingsOfType(solState, EntityLuaBindings, SceneLayer);
		RegisterLuaBindingsOfType(solState, EntityLuaBindings, SLBackground);
		RegisterLuaBindingsOfAbstractType(solState, EntityLuaBindings, Deployment);
		RegisterLuaBindingsOfType(solState, SystemLuaBindings, DataModule);
		RegisterLuaBindingsOfType(solState, ActivityLuaBindings, Activity);
		RegisterLuaBindingsOfAbstractType(solState, ActivityLuaBindings, GameActivity);
		RegisterLuaBindingsOfAbstractType(solState, EntityLuaBindings, GlobalScript);
		RegisterLuaBindingsOfType(solState, EntityLuaBindings, MetaPlayer);
		RegisterLuaBindingsOfType(solState, GUILuaBindings, GUIBanner);
		RegisterLuaBindingsOfType(solState, GUILuaBindings, BuyMenuGUI);
		RegisterLuaBindingsOfType(solState, GUILuaBindings, SceneEditorGUI);
		RegisterLuaBindingsOfType(solState, ManagerLuaBindings, ActivityMan);
		RegisterLuaBindingsOfType(solState, ManagerLuaBindings, AudioMan);
		RegisterLuaBindingsOfType(solState, ManagerLuaBindings, CameraMan);
		RegisterLuaBindingsOfType(solState, ManagerLuaBindings, ConsoleMan);
		RegisterLuaBindingsOfType(solState, ManagerLuaBindings, FrameMan);
		RegisterLuaBindingsOfType(solState, ManagerLuaBindings, MetaMan);
		RegisterLuaBindingsOfType(solState, ManagerLuaBindings, MovableMan);
		RegisterLuaBindingsOfType(solState, ManagerLuaBindings, PerformanceMan);
		RegisterLuaBindingsOfType(solState, ManagerLuaBindings, PostProcessMan);
		RegisterLuaBindingsOfType(solState, ManagerLuaBindings, PresetMan);
		RegisterLuaBindingsOfType(solState, ManagerLuaBindings, PrimitiveMan);
		RegisterLuaBindingsOfType(solState, ManagerLuaBindings, SceneMan);
		RegisterLuaBindingsOfType(solState, ManagerLuaBindings, SettingsMan);
		RegisterLuaBindingsOfType(solState, ManagerLuaBindings, TimerMan);
		RegisterLuaBindingsOfType(solState, ManagerLuaBindings, UInputMan);
		RegisterLuaBindingsOfType(solState, PrimitiveLuaBindings, GraphicalPrimitive);
		RegisterLuaBindingsOfType(solState, PrimitiveLuaBindings, LinePrimitive);
		RegisterLuaBindingsOfType(solState, PrimitiveLuaBindings, ArcPrimitive);
		RegisterLuaBindingsOfType(solState, PrimitiveLuaBindings, SplinePrimitive);
		RegisterLuaBindingsOfType(solState, PrimitiveLuaBindings, BoxPrimitive);
		RegisterLuaBindingsOfType(solState, PrimitiveLuaBindings, BoxFillPrimitive);
		RegisterLuaBindingsOfType(solState, PrimitiveLuaBindings, RoundedBoxPrimitive);
		RegisterLuaBindingsOfType(solState, PrimitiveLuaBindings, RoundedBoxFillPrimitive);
		RegisterLuaBindingsOfType(solState, PrimitiveLuaBindings, CirclePrimitive);
		RegisterLuaBindingsOfType(solState, PrimitiveLuaBindings, CircleFillPrimitive);
		RegisterLuaBindingsOfType(solState, PrimitiveLuaBindings, EllipsePrimitive);
		RegisterLuaBindingsOfType(solState, PrimitiveLuaBindings, EllipseFillPrimitive);
		RegisterLuaBindingsOfType(solState, PrimitiveLuaBindings, TrianglePrimitive);
		RegisterLuaBindingsOfType(solState, PrimitiveLuaBindings, TriangleFillPrimitive);
		RegisterLuaBindingsOfType(solState, PrimitiveLuaBindings, TextPrimitive);
		RegisterLuaBindingsOfType(solState, PrimitiveLuaBindings, BitmapPrimitive);
		RegisterLuaBindingsOfType(solState, InputLuaBindings, InputDevice);
		RegisterLuaBindingsOfType(solState, InputLuaBindings, InputElements);
		RegisterLuaBindingsOfType(solState, InputLuaBindings, JoyButtons);
		RegisterLuaBindingsOfType(solState, InputLuaBindings, JoyDirections);
		RegisterLuaBindingsOfType(solState, InputLuaBindings, MouseButtons);
		RegisterLuaBindingsOfType(solState, InputLuaBindings, SDL_Keycode);
		RegisterLuaBindingsOfType(solState, InputLuaBindings, SDL_Scancode);
		RegisterLuaBindingsOfType(solState, InputLuaBindings, SDL_GameControllerButton);
		RegisterLuaBindingsOfType(solState, InputLuaBindings, SDL_GameControllerAxis);
		RegisterLuaBindingsOfType(solState, MiscLuaBindings, AlarmEvent);
		RegisterLuaBindingsOfType(solState, MiscLuaBindings, Directions);
		RegisterLuaBindingsOfType(solState, MiscLuaBindings, DrawBlendMode);

		// Assign the manager instances to globals in the lua master state
		solState["LuaMan"] = this;
		solState["TimerMan"] = &g_TimerMan;
		solState["FrameMan"] = &g_FrameMan;
		solState["PerformanceMan"] = &g_PerformanceMan;
		solState["PostProcessMan"] = &g_PostProcessMan;
		solState["PrimitiveMan"] = &g_PrimitiveMan;
		solState["PresetMan"] = &g_PresetMan;
		solState["AudioMan"] = &g_AudioMan;
		solState["UInputMan"] = &g_UInputMan;
		solState["SceneMan"] = &g_SceneMan;
		solState["ActivityMan"] = &g_ActivityMan;
		solState["MetaMan"] = &g_MetaMan;
		solState["MovableMan"] = &g_MovableMan;
		solState["CameraMan"] = &g_CameraMan;
		solState["ConsoleMan"] = &g_ConsoleMan;
		solState["SettingsMan"] = &g_SettingsMan;

		// And assign our global functions
		solState["LERP"] = &LERP;
		solState["EaseIn"] = &EaseIn;
		solState["EaseOut"] = &EaseOut;
		solState["EaseInOut"] = &EaseInOut;
		solState["Clamp"] = &Limit;
		solState["NormalizeAngleBetween0And2PI"] = &NormalizeAngleBetween0And2PI;
		solState["NormalizeAngleBetweenNegativePIAndPI"] = &NormalizeAngleBetweenNegativePIAndPI;
		solState["AngleWithinRange"] = &AngleWithinRange;
		solState["ClampAngle"] = &ClampAngle;
		solState["GetPPM"] = &LuaAdaptersUtility::GetPPM;
		solState["GetMPP"] = &LuaAdaptersUtility::GetMPP;
		solState["GetPPL"] = &LuaAdaptersUtility::GetPPL;
		solState["GetLPP"] = &LuaAdaptersUtility::GetLPP;
		solState["GetPathFindingDefaultDigStrength"] = &LuaAdaptersUtility::GetPathFindingDefaultDigStrength;
		solState["RoundFloatToPrecision"] = &RoundFloatToPrecision;
		solState["RoundToNearestMultiple"] = &RoundToNearestMultiple;

		const uint64_t seed = RandomNum<uint64_t>(0, std::numeric_limits<uint64_t>::max());
		m_RandomGenerator.Seed(seed);

		luaL_dostring(m_State,
			// Add cls() as a shortcut to ConsoleMan:Clear().
			"cls = function() ConsoleMan:Clear(); end"
			"\n"
			// Override "print" in the lua state to output to the console.
			"print = function(stringToPrint) ConsoleMan:PrintString(\"PRINT: \" .. tostring(stringToPrint)); end"
			"\n"
			// Override random functions to appear global instead of under LuaMan
			"SelectRand = function(lower, upper) return LuaMan:SelectRand(lower, upper); end;\n"
			"RangeRand = function(lower, upper) return LuaMan:RangeRand(lower, upper); end;\n"
			"PosRand = function() return LuaMan:PosRand(); end;\n"
			"NormalRand = function() return LuaMan:NormalRand(); end;\n"
			// Override "math.random" in the lua state to use RTETools MT19937 implementation. Preserve return types of original to not break all the things.
			"math.random = function(lower, upper) if lower ~= nil and upper ~= nil then return LuaMan:SelectRand(lower, upper); elseif lower ~= nil then return LuaMan:SelectRand(1, lower); else return LuaMan:PosRand(); end end"
			"\n"
			// Override "dofile"/"loadfile" to be able to account for Data/ or Mods/ directory.
			"OriginalDoFile = dofile; dofile = function(filePath) filePath = PresetMan:GetFullModulePath(filePath); if filePath ~= '' then return OriginalDoFile(filePath); end end;"
			"OriginalLoadFile = loadfile; loadfile = function(filePath) filePath = PresetMan:GetFullModulePath(filePath); if filePath ~= '' then return OriginalLoadFile(filePath); end end;"
			// Internal helper functions to add callbacks for async pathing requests
			"_AsyncPathCallbacks = {};"
			"_AddAsyncPathCallback = function(id, callback) _AsyncPathCallbacks[id] = callback; end\n"
			"_TriggerAsyncPathCallback = function(id, param) if _AsyncPathCallbacks[id] ~= nil then _AsyncPathCallbacks[id](param); _AsyncPathCallbacks[id] = nil; end end\n"
		);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaStateWrapper::Destroy() {
		lua_close(m_State);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int LuaStateWrapper::SelectRand(int minInclusive, int maxInclusive) {
		return m_RandomGenerator.RandomNum<int>(minInclusive, maxInclusive);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	double LuaStateWrapper::RangeRand(double minInclusive, double maxInclusive) {
		return m_RandomGenerator.RandomNum<double>(minInclusive, maxInclusive);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	double LuaStateWrapper::NormalRand() {
		return m_RandomGenerator.RandomNormalNum<double>();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	double LuaStateWrapper::PosRand() {
		return m_RandomGenerator.RandomNum<double>();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Passthrough LuaMan Functions
	const std::vector<std::string>& LuaStateWrapper::DirectoryList(const std::string& relativeDirectory) { return g_LuaMan.DirectoryList(relativeDirectory); }
	const std::vector<std::string>& LuaStateWrapper::FileList(const std::string& relativeDirectory) { return g_LuaMan.FileList(relativeDirectory); }
	bool LuaStateWrapper::FileExists(const std::string &fileName) { return g_LuaMan.FileExists(fileName); }
	int LuaStateWrapper::FileOpen(const std::string& fileName, const std::string& accessMode) { return g_LuaMan.FileOpen(fileName, accessMode); }
	void LuaStateWrapper::FileClose(int fileIndex) { return g_LuaMan.FileClose(fileIndex); }
	void LuaStateWrapper::FileCloseAll() { return g_LuaMan.FileCloseAll(); }
	std::string LuaStateWrapper::FileReadLine(int fileIndex) { return g_LuaMan.FileReadLine(fileIndex); }
	void LuaStateWrapper::FileWriteLine(int fileIndex, const std::string& line) { return g_LuaMan.FileWriteLine(fileIndex, line); }
	bool LuaStateWrapper::FileEOF(int fileIndex) { return g_LuaMan.FileEOF(fileIndex); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaMan::Clear() {
		m_OpenedFiles.fill(nullptr);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaMan::Initialize() {
		m_MasterScriptState.Initialize();

		m_ScriptStates = std::vector<LuaStateWrapper>(std::thread::hardware_concurrency());
		for (LuaStateWrapper &luaState : m_ScriptStates) {
			luaState.Initialize();
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaStateWrapper & LuaMan::GetMasterScriptState() {
        return m_MasterScriptState;
    }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    LuaStatesArray & LuaMan::GetThreadedScriptStates() {
		return m_ScriptStates;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	thread_local LuaStateWrapper * s_luaStateOverride = nullptr;
    LuaStateWrapper * LuaMan::GetThreadLuaStateOverride() const {
		return s_luaStateOverride;
    }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaMan::SetThreadLuaStateOverride(LuaStateWrapper * luaState) {
		s_luaStateOverride = luaState;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	thread_local LuaStateWrapper* s_currentLuaState = nullptr;
	LuaStateWrapper* LuaMan::GetThreadCurrentLuaState() const {
		return s_currentLuaState;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaStateWrapper * LuaMan::GetAndLockFreeScriptState() {
		if (s_luaStateOverride) {
			// We're creating this object in a multithreaded environment, ensure that it's assigned to the same script state as us
			bool success = s_luaStateOverride->GetMutex().try_lock();
			RTEAssert(success, "Our lua state override for our thread already belongs to another thread!")
			return s_luaStateOverride;
		}

		// TODO
		// It would be nice to assign to least-saturated state, but that's a bit tricky with MO registering...
		/*auto itr = std::min_element(m_ScriptStates.begin(), m_ScriptStates.end(), 
			[](const LuaStateWrapper& lhs, const LuaStateWrapper& rhs) { return lhs.GetRegisteredMOs().size() < rhs.GetRegisteredMOs().size(); }
		);

		bool success = itr->GetMutex().try_lock();
		RTEAssert(success, "Script mutex was already locked while in a non-multithreaded environment!");

		return &(*itr);*/

		int ourState = m_LastAssignedLuaState;
		m_LastAssignedLuaState = (m_LastAssignedLuaState + 1) % m_ScriptStates.size();

		bool success = m_ScriptStates[ourState].GetMutex().try_lock();
		RTEAssert(success, "Script mutex was already locked while in a non-multithreaded environment!");

		return &m_ScriptStates[ourState];	
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void LuaMan::ClearUserModuleCache() {
		m_GarbageCollectionTask.wait();

		m_MasterScriptState.ClearLuaScriptCache();
		for (LuaStateWrapper& luaState : m_ScriptStates) {
			luaState.ClearLuaScriptCache();
		}

		m_MasterScriptState.ClearUserModuleCache();
		for (LuaStateWrapper& luaState : m_ScriptStates) {
			luaState.ClearUserModuleCache();
		}
    }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void LuaMan::AddLuaScriptCallback(const std::function<void()> &callback) {
		std::scoped_lock lock(m_ScriptCallbacksMutex);
		m_ScriptCallbacks.emplace_back(callback);
    }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void LuaMan::ExecuteLuaScriptCallbacks() {
		std::vector<std::function<void()>> callbacks;
		
		// Move our functions into the local buffer to clear the existing callbacks and to lock for as little time as possible
		{
			std::scoped_lock lock(m_ScriptCallbacksMutex);
			callbacks.swap(m_ScriptCallbacks);
		}
		
		for (const std::function<void()> &callback : callbacks) {
			callback();
		}
    }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	const std::unordered_map<std::string, PerformanceMan::ScriptTiming> LuaMan::GetScriptTimings() const {
		std::unordered_map<std::string, PerformanceMan::ScriptTiming> timings = m_MasterScriptState.GetScriptTimings();
		for (const LuaStateWrapper &luaState : m_ScriptStates) {
			for (auto&& [functionName, timing] : luaState.GetScriptTimings()) {
				auto& existing = timings[functionName];
				existing.m_CallCount += timing.m_CallCount;
				existing.m_Time = std::max(existing.m_Time, timing.m_Time);
			}
		}
		return timings;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaMan::Destroy() {
		for (int i = 0; i < c_MaxOpenFiles; ++i) {
			FileClose(i);
		}
		Clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaStateWrapper::ClearUserModuleCache() {
		luaL_dostring(m_State, "for m, n in pairs(package.loaded) do if type(n) == \"boolean\" then package.loaded[m] = nil; end; end;");
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaStateWrapper::ClearLuaScriptCache() {
		m_ScriptCache.clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Entity *LuaStateWrapper::GetTempEntity() const {
        return m_TempEntity;
    }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void LuaStateWrapper::SetTempEntity(Entity *entity) {
		m_TempEntity = entity;
    }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    const std::vector<Entity *> & LuaStateWrapper::GetTempEntityVector() const {
        return m_TempEntityVector;
    }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaStateWrapper::SetTempEntityVector(const std::vector<const Entity *> &entityVector) {
		m_TempEntityVector.reserve(entityVector.size());
		for (const Entity *entity : entityVector) {
			m_TempEntityVector.push_back(const_cast<Entity *>(entity));
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaStateWrapper::SetLuaPath(const std::string &filePath) {
		const std::string moduleName = g_PresetMan.GetModuleNameFromPath(filePath);
		const std::string moduleFolder = g_PresetMan.IsModuleOfficial(moduleName) ? System::GetDataDirectory() : System::GetModDirectory();
		const std::string scriptPath = moduleFolder + moduleName + "/?.lua";

		lua_getglobal(m_State, "package");
		lua_getfield(m_State, -1, "path"); // get field "path" from table at top of stack (-1).
		std::string currentPath = lua_tostring(m_State, -1); // grab path string from top of stack.

		// check if scriptPath is already in there, if not add it.
		if (currentPath.find(scriptPath) == std::string::npos) {
			currentPath.append(";" + scriptPath);
		}

		lua_pop(m_State, 1); // get rid of the string on the stack we just pushed previously.
		lua_pushstring(m_State, currentPath.c_str()); // push the new one.
		lua_setfield(m_State, -2, "path"); // set the field "path" in table at -2 with value at top of stack.
		lua_pop(m_State, 1); // get rid of package table from top of stack.
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	const std::unordered_map<std::string, PerformanceMan::ScriptTiming> & LuaStateWrapper::GetScriptTimings() const {
		return m_ScriptTimings;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int LuaStateWrapper::RunScriptFunctionString(const std::string &functionName, const std::string &selfObjectName, const std::vector<std::string_view> &variablesToSafetyCheck, const std::vector<const Entity *> &functionEntityArguments, const std::vector<std::string_view> &functionLiteralArguments) {
		std::stringstream scriptString;
		if (!variablesToSafetyCheck.empty()) {
			scriptString << "if ";
			for (const std::string_view &variableToSafetyCheck : variablesToSafetyCheck) {
				if (&variableToSafetyCheck != &variablesToSafetyCheck[0]) { scriptString << " and "; }
				scriptString << variableToSafetyCheck;
			}
			scriptString << " then ";
		}
		if (!functionEntityArguments.empty()) { scriptString << "local entityArguments = LuaMan.TempEntities; "; }

		// Lock here, even though we also lock in RunScriptString(), to ensure that the temp entity vector isn't stomped by separate threads.
		std::lock_guard<std::recursive_mutex> lock(m_Mutex);
		s_currentLuaState = this;

		scriptString << functionName + "(";
		if (!selfObjectName.empty()) { scriptString << selfObjectName; }
		bool isFirstFunctionArgument = selfObjectName.empty();
		if (!functionEntityArguments.empty()) {
			SetTempEntityVector(functionEntityArguments);
			for (const Entity *functionEntityArgument : functionEntityArguments) {
				if (!isFirstFunctionArgument) { scriptString << ", "; }
				scriptString << "(To" + functionEntityArgument->GetClassName() + " and To" + functionEntityArgument->GetClassName() + "(entityArguments()) or entityArguments())";
				isFirstFunctionArgument = false;
			}
		}
		if (!functionLiteralArguments.empty()) {
			for (const std::string_view &functionLiteralArgument : functionLiteralArguments) {
				if (!isFirstFunctionArgument) { scriptString << ", "; }
				scriptString << std::string(functionLiteralArgument);
				isFirstFunctionArgument = false;
			}
		}
		scriptString << ");";

		if (!variablesToSafetyCheck.empty()) { scriptString << " end;"; }

		int result = RunScriptString(scriptString.str());
		m_TempEntityVector.clear();
		return result;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int LuaStateWrapper::RunScriptString(const std::string &scriptString, bool consoleErrors) {
		if (scriptString.empty()) {
			return -1;
		}
		int error = 0;

		std::lock_guard<std::recursive_mutex> lock(m_Mutex);
		s_currentLuaState = this;

		lua_pushcfunction(m_State, &AddFileAndLineToError);
		// Load the script string onto the stack and then execute it with pcall. Pcall will call the file and line error handler if there's an error by pointing 2 up the stack to it.
		if (luaL_loadstring(m_State, scriptString.c_str()) || lua_pcall(m_State, 0, LUA_MULTRET, -2)) {
			// Retrieve the error message then pop it off the stack to clean it up
			m_LastError = lua_tostring(m_State, -1);
			lua_pop(m_State, 1);
			if (consoleErrors) {
				g_ConsoleMan.PrintString("ERROR: " + m_LastError);
				ClearErrors();
			}
			error = -1;
		}
		// Pop the file and line error handler off the stack to clean it up
		lua_pop(m_State, 1);

		return error;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int LuaStateWrapper::RunScriptFunctionObject(const SolObjectWrapper *functionObject, const std::string &selfGlobalTableName, const std::string &selfGlobalTableKey, const std::vector<const Entity*> &functionEntityArguments, const std::vector<std::string_view> &functionLiteralArguments, const std::vector<SolObjectWrapper*> &functionObjectArguments) {
		int status = 0;

		std::lock_guard<std::recursive_mutex> lock(m_Mutex);
		s_currentLuaState = this;
		m_CurrentlyRunningScriptPath = functionObject->GetFilePath();

		lua_pushcfunction(m_State, &AddFileAndLineToError);
		functionObject->GetSolObject()->push(m_State);

		int argumentCount = functionEntityArguments.size() + functionLiteralArguments.size() + functionObjectArguments.size();
		if (!selfGlobalTableName.empty() && TableEntryIsDefined(selfGlobalTableName, selfGlobalTableKey)) {
			lua_getglobal(m_State, selfGlobalTableName.c_str());
			lua_getfield(m_State, -1, selfGlobalTableKey.c_str());
			lua_remove(m_State, -2);
			argumentCount++;
		}

		for (const Entity *functionEntityArgument : functionEntityArguments) {
			std::unique_ptr<SolObjectWrapper> downCastEntityAsSolObjectWrapper(LuaAdaptersEntityCast::s_EntityToSolObjectCastFunctions.at(functionEntityArgument->GetClassName())(const_cast<Entity *>(functionEntityArgument), m_State));
			downCastEntityAsSolObjectWrapper->GetSolObject()->push(m_State);
		}

		for (const std::string_view &functionLiteralArgument : functionLiteralArguments) {
			char *stringToDoubleConversionFailed = nullptr;
			if (functionLiteralArgument == "nil") {
				lua_pushnil(m_State);
			} else if (functionLiteralArgument == "true" || functionLiteralArgument == "false") {
				lua_pushboolean(m_State, functionLiteralArgument == "true" ? 1 : 0);
			} else if (double argumentAsNumber = std::strtod(functionLiteralArgument.data(), &stringToDoubleConversionFailed); !*stringToDoubleConversionFailed) {
				lua_pushnumber(m_State, argumentAsNumber);
			} else {
				lua_pushlstring(m_State, functionLiteralArgument.data(), functionLiteralArgument.size());
			}
		}

		for (const SolObjectWrapper *functionObjectArgument : functionObjectArguments) {
			if (functionObjectArgument->GetSolObject()->lua_state() != m_State) {
				SolObjectWrapper copy = functionObjectArgument->GetCopyForState(*m_State);
				copy.GetSolObject()->push(m_State);
			} else {
				functionObjectArgument->GetSolObject()->push(m_State);
			}
		}

		const std::string& path = functionObject->GetFilePath();
		std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
		{
			ZoneScoped;
			ZoneName(path.c_str(), path.length());

			if (lua_pcall(m_State, argumentCount, LUA_MULTRET, -argumentCount - 2) > 0) {
				m_LastError = lua_tostring(m_State, -1);
				lua_pop(m_State, 1);
				g_ConsoleMan.PrintString("ERROR: " + m_LastError);
				ClearErrors();
				status = -1;
			}
		}
		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

		// only track time in non-MT scripts, for now
		if (&g_LuaMan.GetMasterScriptState() == this) {
			m_ScriptTimings[path].m_Time += std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
			m_ScriptTimings[path].m_CallCount++;
		}

		lua_pop(m_State, 1);

		m_CurrentlyRunningScriptPath = "";
		return status;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int LuaStateWrapper::RunScriptFile(const std::string &filePath, bool consoleErrors, bool doInSandboxedEnvironment) {
		const std::string fullScriptPath = g_PresetMan.GetFullModulePath(filePath);
		if (fullScriptPath.empty()) {
			m_LastError = "Can't run a script file with an empty filepath!";
			return -1;
		}

		if (!System::PathExistsCaseSensitive(fullScriptPath)) {
			m_LastError = "Script file: " + filePath + " doesn't exist!";
			if (consoleErrors) {
				g_ConsoleMan.PrintString("ERROR: " + m_LastError);
				ClearErrors();
			}
			return -1;
		}

		int error = 0;

		std::lock_guard<std::recursive_mutex> lock(m_Mutex);
		s_currentLuaState = this;
		m_CurrentlyRunningScriptPath = filePath;

		const int stackStart = lua_gettop(m_State);

		lua_pushcfunction(m_State, &AddFileAndLineToError);
		SetLuaPath(fullScriptPath);

		// Load the script file's contents onto the stack
		if (luaL_loadfile(m_State, fullScriptPath.c_str())) {
			m_LastError = lua_tostring(m_State, -1);
			lua_pop(m_State, 1);
			if (consoleErrors) {
				g_ConsoleMan.PrintString("ERROR: " + m_LastError);
				ClearErrors();
			}
			error = -1;
		}

		if (error == 0) {
			if (doInSandboxedEnvironment) {
				// create a new environment table
				lua_getglobal(m_State, filePath.c_str());
				if (lua_isnil(m_State, -1)) {
					lua_pop(m_State, 1);
					lua_newtable(m_State);
					lua_newtable(m_State);
					lua_getglobal(m_State, "_G");
					lua_setfield(m_State, -2, "__index");
					lua_setmetatable(m_State, -2);
					lua_setglobal(m_State, filePath.c_str());
					lua_getglobal(m_State, filePath.c_str());
				}

				lua_setfenv(m_State, -2);
			}

			// execute script file with pcall. Pcall will call the file and line error handler if there's an error by pointing 2 up the stack to it.
			if (lua_pcall(m_State, 0, LUA_MULTRET, -2)) {
				m_LastError = lua_tostring(m_State, -1);
				lua_pop(m_State, 1);
				if (consoleErrors) {
					g_ConsoleMan.PrintString("ERROR: " + m_LastError);
					ClearErrors();
				}
				error = -1;
			}
		}

		// Pop the line error handler off the stack to clean it up
		lua_pop(m_State, 1);

		m_CurrentlyRunningScriptPath = "";
		RTEAssert(lua_gettop(m_State) == stackStart, "Malformed lua stack!");
		return error;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool LuaStateWrapper::RetrieveFunctions(const std::string& funcObjectName, const std::vector<std::string>& functionNamesToLookFor, std::unordered_map<std::string, SolObjectWrapper*>& outFunctionNamesAndObjects) {
		std::lock_guard<std::recursive_mutex> lock(m_Mutex);
		s_currentLuaState = this;

		sol::state_view solState(m_State);
		sol::table funcHoldingObject = solState.globals()[funcObjectName.c_str()];
		if (funcHoldingObject == sol::lua_nil) {
			return -1;
		}

		auto& newScript = m_ScriptCache[funcObjectName.c_str()];
		for (auto& pair : newScript.functionNamesAndObjects) {
			delete pair.second;
		}
		newScript.functionNamesAndObjects.clear();
		for (const std::string& functionName : functionNamesToLookFor) {
			sol::function functionObject = funcHoldingObject[functionName];
			if (functionObject.valid()) {
				sol::object* functionObjectCopyForStoring = new sol::object(functionObject);
				newScript.functionNamesAndObjects.try_emplace(functionName, new SolObjectWrapper(functionObjectCopyForStoring, funcObjectName));
			}
		}

		for (auto& pair : newScript.functionNamesAndObjects) {
			sol::object* functionObjectCopyForStoring = new sol::object(*pair.second->GetSolObject());
			outFunctionNamesAndObjects.try_emplace(pair.first, new SolObjectWrapper(functionObjectCopyForStoring, funcObjectName));
		}

		return true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int LuaStateWrapper::RunScriptFileAndRetrieveFunctions(const std::string &filePath, const std::vector<std::string> &functionNamesToLookFor, std::unordered_map<std::string, SolObjectWrapper *> &outFunctionNamesAndObjects, bool forceReload) {
		static bool disableCaching = false;
		forceReload = forceReload || disableCaching;

		// If it's already cached, we don't need to run it again
		// TODO - fix activity restarting needing to force reload
		auto cachedScript = m_ScriptCache.find(filePath);
		if (!forceReload && cachedScript != m_ScriptCache.end()) {
			for (auto& pair : cachedScript->second.functionNamesAndObjects) {
				sol::object* functionObjectCopyForStoring = new sol::object(*pair.second->GetSolObject());
				outFunctionNamesAndObjects.try_emplace(pair.first, new SolObjectWrapper(functionObjectCopyForStoring, filePath));
			}

			return 0;
		}

		std::lock_guard<std::recursive_mutex> lock(m_Mutex);
		s_currentLuaState = this;

		if (int error = RunScriptFile(filePath); error < 0) {
			return error;
		}

		if (!RetrieveFunctions(filePath, functionNamesToLookFor, outFunctionNamesAndObjects)) {
			return -1;
		}

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void LuaStateWrapper::Update() {
		for (MovableObject* mo : m_AddedRegisteredMOs) {
			m_RegisteredMOs.insert(mo);
		}
		m_AddedRegisteredMOs.clear();
    }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaStateWrapper::ClearScriptTimings() {
		m_ScriptTimings.clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    bool LuaStateWrapper::ExpressionIsTrue(const std::string &expression, bool consoleErrors) {
        if (expression.empty()) {
			return false;
		}
		bool result = false;

		std::lock_guard<std::recursive_mutex> lock(m_Mutex);

		// Push the script string onto the stack so we can execute it, and then actually try to run it. Assign the result to a dedicated temp global variable.
		if (luaL_dostring(m_State, std::string("ExpressionResult = " + expression + ";").c_str())) {
			m_LastError = std::string("When evaluating Lua expression: ") + lua_tostring(m_State, -1);
			lua_pop(m_State, 1);
			if (consoleErrors) {
				g_ConsoleMan.PrintString("ERROR: " + m_LastError);
				ClearErrors();
			}
			return false;
		}
		// Put the result of the expression on the lua stack and check its value. Need to pop it off the stack afterwards so it leaves the stack unchanged.
		lua_getglobal(m_State, "ExpressionResult");
		result = lua_toboolean(m_State, -1);
		lua_pop(m_State, 1);

		return result;
    }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaStateWrapper::SavePointerAsGlobal(void *objectToSave, const std::string &globalName) {
		std::lock_guard<std::recursive_mutex> lock(m_Mutex);

		// Push the pointer onto the Lua stack.
		lua_pushlightuserdata(m_State, objectToSave);
		// Pop and assign that pointer to a global var in the Lua state.
		lua_setglobal(m_State, globalName.c_str());
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool LuaStateWrapper::GlobalIsDefined(const std::string &globalName) {
		std::lock_guard<std::recursive_mutex> lock(m_Mutex);

		// Get the var you want onto the stack so we can check it.
		lua_getglobal(m_State, globalName.c_str());
		// Now report if it is nil/null or not.
		bool isDefined = !lua_isnil(m_State, -1);
		// Pop the var so this operation is balanced and leaves the stack as it was.
		lua_pop(m_State, 1);

		return isDefined;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool LuaStateWrapper::TableEntryIsDefined(const std::string &tableName, const std::string &indexName) {
		std::lock_guard<std::recursive_mutex> lock(m_Mutex);
		
		// Push the table onto the stack, checking if it even exists.
		lua_getglobal(m_State, tableName.c_str());
		if (!lua_istable(m_State, -1)) {
			// Clean up and report that there was nothing properly defined here.
			lua_pop(m_State, 1);
			return false;
		}
		// Push the value at the requested index onto the stack so we can check if it's anything.
		lua_getfield(m_State, -1, indexName.c_str());
		// Now report if it is nil/null or not
		bool isDefined = !lua_isnil(m_State, -1);
		// Pop both the var and the table so this operation is balanced and leaves the stack as it was.
		lua_pop(m_State, 2);

		return isDefined;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    bool LuaStateWrapper::ErrorExists() const {
        return !m_LastError.empty();;
    }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    std::string LuaStateWrapper::GetLastError() const {
        return m_LastError;
    }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void LuaStateWrapper::ClearErrors() {
		m_LastError.clear();
    }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::string LuaStateWrapper::DescribeLuaStack() {
		int indexOfTopOfStack = lua_gettop(m_State);
		if (indexOfTopOfStack == 0) {
			return "The Lua stack is empty.";
		}
		std::stringstream stackDescription;
		stackDescription << "The Lua stack contains " + std::to_string(indexOfTopOfStack) + " elements. From top to bottom, they are:\n";

		for (int i = indexOfTopOfStack; i > 0; --i) {
			switch (int type = lua_type(m_State, i)) {
				case LUA_TBOOLEAN:
					stackDescription << (lua_toboolean(m_State, i) ? "true" : "false");
					break;
				case LUA_TNUMBER:
					stackDescription << std::to_string(lua_tonumber(m_State, i));
					break;
				case LUA_TSTRING:
					stackDescription << lua_tostring(m_State, i);
					break;
				default:
					stackDescription << lua_typename(m_State, type);
					break;
			}
			if (i - 1 > 0) { stackDescription << "\n"; }
		}
		return stackDescription.str();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	const std::vector<std::string> & LuaMan::DirectoryList(const std::string &filePath) {
		thread_local std::vector<std::string> directoryPaths;
		directoryPaths.clear();

		for (const std::filesystem::directory_entry &directoryEntry : std::filesystem::directory_iterator(System::GetWorkingDirectory() + filePath)) {
			if (directoryEntry.is_directory()) { directoryPaths.emplace_back(directoryEntry.path().filename().generic_string()); }
		}
		return directoryPaths;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	const std::vector<std::string> & LuaMan::FileList(const std::string &filePath) {
		thread_local std::vector<std::string> filePaths;
		filePaths.clear();

		for (const std::filesystem::directory_entry &directoryEntry : std::filesystem::directory_iterator(System::GetWorkingDirectory() + filePath)) {
			if (directoryEntry.is_regular_file()) { filePaths.emplace_back(directoryEntry.path().filename().generic_string()); }
		}
		return filePaths;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool LuaMan::FileExists(const std::string &fileName) {
		std::string fullPath = System::GetWorkingDirectory() + g_PresetMan.GetFullModulePath(fileName);
		if ((fullPath.find("..") == std::string::npos) && (fullPath.find(System::GetModulePackageExtension()) != std::string::npos)) {
			return std::filesystem::exists(fullPath);
		}

		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int LuaMan::FileOpen(const std::string &fileName, const std::string &accessMode) {
		if (c_FileAccessModes.find(accessMode) == c_FileAccessModes.end()) {
			g_ConsoleMan.PrintString("ERROR: Cannot open file, invalid file access mode specified.");
			return -1;
		}

		int fileIndex = -1;
		for (int i = 0; i < c_MaxOpenFiles; ++i) {
			if (!m_OpenedFiles[i]) {
				fileIndex = i;
				break;
			}
		}
		if (fileIndex == -1) {
			g_ConsoleMan.PrintString("ERROR: Cannot open file, maximum number of files already open.");
			return -1;
		}

		std::string fullPath = System::GetWorkingDirectory() + g_PresetMan.GetFullModulePath(fileName);
		if ((fullPath.find("..") == std::string::npos) && (fullPath.find(System::GetModulePackageExtension()) != std::string::npos)) {

#ifdef _WIN32
			FILE *file = fopen(fullPath.c_str(), accessMode.c_str());
#else
			FILE *file = [&fullPath, &accessMode]() -> FILE* {
				std::filesystem::path inspectedPath = System::GetWorkingDirectory();
				const std::filesystem::path relativeFilePath = std::filesystem::path(fullPath).lexically_relative(inspectedPath);

				for (std::filesystem::path::const_iterator relativeFilePathIterator = relativeFilePath.begin(); relativeFilePathIterator != relativeFilePath.end(); ++relativeFilePathIterator) {
					bool pathPartExists = false;

					// Check if a path part (directory or file) exists in the filesystem.
					for (const std::filesystem::path &filesystemEntryPath : std::filesystem::directory_iterator(inspectedPath)) {
						if (StringsEqualCaseInsensitive(filesystemEntryPath.filename().generic_string(), (*relativeFilePathIterator).generic_string())) {
							inspectedPath = filesystemEntryPath;
							pathPartExists = true;
							break;
						}
					}
					if (!pathPartExists) {
						// If this is the last part, then all directories in relativeFilePath exist, but the file doesn't.
						if (std::next(relativeFilePathIterator) == relativeFilePath.end()) {
							return fopen((inspectedPath / relativeFilePath.filename()).generic_string().c_str(), accessMode.c_str());
						}
						// Some directory in relativeFilePath doesn't exist, so the file can't be created.
						return nullptr;
					}
				}
				// If the file exists, open it.
				return fopen(inspectedPath.generic_string().c_str(), accessMode.c_str());
			}();
#endif
			if (file) {
				m_OpenedFiles[fileIndex] = file;
				return fileIndex;
			}
		}
		g_ConsoleMan.PrintString("ERROR: Failed to open file " + fileName);
		return -1;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaMan::FileClose(int fileIndex) {
		if (fileIndex > -1 && fileIndex < c_MaxOpenFiles && m_OpenedFiles.at(fileIndex)) {
			fclose(m_OpenedFiles[fileIndex]);
			m_OpenedFiles[fileIndex] = nullptr;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaMan::FileCloseAll() {
		for (int file = 0; file < c_MaxOpenFiles; ++file) {
			FileClose(file);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::string LuaMan::FileReadLine(int fileIndex) {
		if (fileIndex > -1 && fileIndex < c_MaxOpenFiles && m_OpenedFiles.at(fileIndex)) {
			char buf[4096];
			if (fgets(buf, sizeof(buf), m_OpenedFiles[fileIndex]) != nullptr) {
				return buf;
			}
#ifndef RELEASE_BUILD
			g_ConsoleMan.PrintString("ERROR: " + std::string(FileEOF(fileIndex) ? "Tried to read past EOF." : "Failed to read from file."));
#endif
		} else {
			g_ConsoleMan.PrintString("ERROR: Tried to read an invalid or closed file.");
		}
		return "";
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaMan::FileWriteLine(int fileIndex, const std::string &line) {
		if (fileIndex > -1 && fileIndex < c_MaxOpenFiles && m_OpenedFiles.at(fileIndex)) {
			if (fputs(line.c_str(), m_OpenedFiles[fileIndex]) == EOF) {
				g_ConsoleMan.PrintString("ERROR: Failed to write to file. File might have been opened without writing permissions or is corrupt.");
			}
		} else {
			g_ConsoleMan.PrintString("ERROR: Tried to write to an invalid or closed file.");
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool LuaMan::FileEOF(int fileIndex) {
		if (fileIndex > -1 && fileIndex < c_MaxOpenFiles && m_OpenedFiles.at(fileIndex)) {
			return feof(m_OpenedFiles[fileIndex]);
		}
		g_ConsoleMan.PrintString("ERROR: Tried to check EOF for an invalid or closed file.");
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaMan::Update() {
		ZoneScoped;

		m_MasterScriptState.Update();
		for (LuaStateWrapper &luaState : m_ScriptStates) {
			luaState.Update();
		}

		// Make sure a GC run isn't happening while we try to apply deletions
		m_GarbageCollectionTask.wait();

		// Apply all deletions queued from lua
    	SolObjectWrapper::ApplyQueuedDeletions();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaMan::StartAsyncGarbageCollection() {
		ZoneScoped;
		
		std::vector<LuaStateWrapper*> allStates;
		allStates.reserve(m_ScriptStates.size() + 1);

		allStates.push_back(&m_MasterScriptState);
		for (LuaStateWrapper& wrapper : m_ScriptStates) {
			allStates.push_back(&wrapper);
		}
		
		m_GarbageCollectionTask = BS::multi_future<void>();
		for (LuaStateWrapper* luaState : allStates) {
			m_GarbageCollectionTask.push_back(
				g_ThreadMan.GetPriorityThreadPool().submit([luaState]() {
					ZoneScopedN("Lua Garbage Collection");
					std::lock_guard<std::recursive_mutex> lock(luaState->GetMutex());
					lua_gc(luaState->GetLuaState(), LUA_GCSTEP, 30);
					lua_gc(luaState->GetLuaState(), LUA_GCSTOP, 0);
				})
			);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaMan::ClearScriptTimings() {
		m_MasterScriptState.ClearScriptTimings();
		for (LuaStateWrapper& luaState : m_ScriptStates) {
			luaState.ClearScriptTimings();
		}
	}

}
