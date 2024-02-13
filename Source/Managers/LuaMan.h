#pragma once

#include "Singleton.h"
#include "Entity.h"
#include "RTETools.h"
#include "PerformanceMan.h"

#include "BS_thread_pool.hpp"

#define g_LuaMan LuaMan::Instance()

struct lua_State;

namespace RTE {

	class LuabindObjectWrapper;
	class MovableObject;

	/// A single lua state. Multiple of these can exist at once for multithreaded scripting.
	class LuaStateWrapper {
	public:
#pragma region Creation
		/// Constructor method used to instantiate a LuaStateWrapper object in system memory. Initialize() should be called before using the object.
		LuaStateWrapper();

		/// Makes the LuaStateWrapper object ready for use.
		void Initialize();
#pragma endregion

#pragma region Destruction
		/// Destructor method used to clean up a LuaStateWrapper object before deletion from system memory.
		~LuaStateWrapper();

		/// Destroys and resets (through Clear()) the LuaStateWrapper object.
		void Destroy();
#pragma endregion

#pragma region Getters and Setters
		/// Gets a temporary Entity that can be accessed in the Lua state.
		/// @return The temporary entity. Ownership is NOT transferred!
		Entity* GetTempEntity() const;

		/// Sets a temporary Entity that can be accessed in the Lua state.
		/// @param entity The temporary entity. Ownership is NOT transferred!
		void SetTempEntity(Entity* entity);

		/// Gets the temporary vector of Entities that can be accessed in the Lua state.
		/// @return The temporary vector of entities. Ownership is NOT transferred!
		const std::vector<Entity*>& GetTempEntityVector() const;

		/// Sets a temporary vector of Entities that can be accessed in the Lua state. These Entities are const_cast so they're non-const, for ease-of-use in Lua.
		/// @param entityVector The temporary vector of entities. Ownership is NOT transferred!
		void SetTempEntityVector(const std::vector<const Entity*>& entityVector);

		/// Sets the proper package.path for the script to run.
		/// @param filePath The path to the file to load and run.
		void SetLuaPath(const std::string& filePath);

		/// Gets this LuaStateWrapper's internal lua state.
		/// @return This LuaStateWrapper's internal lua state.
		lua_State* GetLuaState() { return m_State; };

		/// Gets m_ScriptTimings.
		/// @return m_ScriptTimings.
		const std::unordered_map<std::string, PerformanceMan::ScriptTiming>& GetScriptTimings() const;

		/// Gets the currently running script filepath, if applicable.
		/// @return The currently running script filepath. May return inaccurate values for non-MO scripts due to weirdness in setup.
		std::string_view GetCurrentlyRunningScriptFilePath() const { return m_CurrentlyRunningScriptPath; }
#pragma endregion

#pragma region Script Responsibility Handling
		/// Registers an MO as using us.
		/// @param moToRegister The MO to register with us. Ownership is NOT transferred!
		void RegisterMO(MovableObject* moToRegister) { m_AddedRegisteredMOs.insert(moToRegister); }

		/// Unregisters an MO as using us.
		/// @param moToUnregister The MO to unregister as using us. Ownership is NOT transferred!
		void UnregisterMO(MovableObject* moToUnregister) {
			m_RegisteredMOs.erase(moToUnregister);
			m_AddedRegisteredMOs.erase(moToUnregister);
		}

		/// Gets a list of the MOs registed as using us.
		/// @return The MOs registed as using us.
		const std::unordered_set<MovableObject*>& GetRegisteredMOs() const { return m_RegisteredMOs; }
#pragma endregion

#pragma region Script Execution Handling
		/// Runs the given Lua function with optional safety checks and arguments. The first argument to the function will always be the self object.
		/// If either argument list has entries, they will be passed into the function in order, with entity arguments first.
		/// @param functionName The name that gives access to the function in the global Lua namespace.
		/// @param selfObjectName The name that gives access to the self object in the global Lua namespace.
		/// @param variablesToSafetyCheck Optional vector of strings that should be safety checked in order before running the Lua function. Defaults to empty.
		/// @param functionEntityArguments Optional vector of entity pointers that should be passed into the Lua function. Their internal Lua states will not be accessible. Defaults to empty.
		/// @param functionLiteralArguments Optional vector of strings that should be passed into the Lua function. Entries must be surrounded with escaped quotes (i.e.`\"`) they'll be passed in as-is, allowing them to act as booleans, etc. Defaults to empty.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int RunScriptFunctionString(const std::string& functionName, const std::string& selfObjectName, const std::vector<std::string_view>& variablesToSafetyCheck = std::vector<std::string_view>(), const std::vector<const Entity*>& functionEntityArguments = std::vector<const Entity*>(), const std::vector<std::string_view>& functionLiteralArguments = std::vector<std::string_view>());

		/// Takes a string containing a script snippet and runs it on the state.
		/// @param scriptString The string with the script snippet.
		/// @param consoleErrors Whether to report any errors to the console immediately.
		/// @return Returns less than zero if any errors encountered when running this script. To get the actual error string, call GetLastError.
		int RunScriptString(const std::string& scriptString, bool consoleErrors = true);

		/// Runs the given Lua function object. The first argument to the function will always be the self object.
		/// If either argument list has entries, they will be passed into the function in order, with entity arguments first.
		/// @param functionObjectWrapper The LuabindObjectWrapper containing the Lua function to be run.
		/// @param selfGlobalTableName The name of the global Lua table that gives access to the self object.
		/// @param selfGlobalTableKey The key for this object in the respective global Lua table.
		/// @param functionEntityArguments Optional vector of entity pointers that should be passed into the Lua function. Their internal Lua states will not be accessible. Defaults to empty.
		/// @param functionLiteralArguments Optional vector of strings that should be passed into the Lua function. Entries must be surrounded with escaped quotes (i.e.`\"`) they'll be passed in as-is, allowing them to act as booleans, etc.. Defaults to empty.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int RunScriptFunctionObject(const LuabindObjectWrapper* functionObjectWrapper, const std::string& selfGlobalTableName, const std::string& selfGlobalTableKey, const std::vector<const Entity*>& functionEntityArguments = std::vector<const Entity*>(), const std::vector<std::string_view>& functionLiteralArguments = std::vector<std::string_view>(), const std::vector<LuabindObjectWrapper*>& functionObjectArguments = std::vector<LuabindObjectWrapper*>());

		/// Opens and loads a file containing a script and runs it on the state.
		/// @param filePath The path to the file to load and run.
		/// @param consoleErrors Whether to report any errors to the console immediately.
		/// @param doInSandboxedEnvironment Whether to do it in a sandboxed environment, or the global environment.
		/// @return Returns less than zero if any errors encountered when running this script. To get the actual error string, call GetLastError.
		int RunScriptFile(const std::string& filePath, bool consoleErrors = true, bool doInSandboxedEnvironment = true);

		/// Retrieves all of the specified functions that exist into the output map, and refreshes the cache.
		/// @param filePath The path to the file to load and run.
		/// @param functionNamesToLookFor The vector of strings defining the function names to be retrieved.
		/// @param outFunctionNamesAndObjects The map of function names to LuabindObjectWrappers to be retrieved from the script that was run.
		/// @return Returns whether functions were successfully retrieved.
		bool RetrieveFunctions(const std::string& functionObjectName, const std::vector<std::string>& functionNamesToLookFor, std::unordered_map<std::string, LuabindObjectWrapper*>& outFunctionNamesAndObjects);

		/// Opens and loads a file containing a script and runs it on the state, then retrieves all of the specified functions that exist into the output map.
		/// @param filePath The path to the file to load and run.
		/// @param functionNamesToLookFor The vector of strings defining the function names to be retrieved.
		/// @param outFunctionNamesAndObjects The map of function names to LuabindObjectWrappers to be retrieved from the script that was run.
		/// @param noCaching Whether caching shouldn't be used.
		/// @return Returns less than zero if any errors encountered when running this script. To get the actual error string, call GetLastError.
		int RunScriptFileAndRetrieveFunctions(const std::string& filePath, const std::vector<std::string>& functionNamesToLookFor, std::unordered_map<std::string, LuabindObjectWrapper*>& outFunctionNamesAndObjects, bool forceReload = false);
#pragma endregion

#pragma region Concrete Methods
		/// Updates this Lua state.
		void Update();

		/// Clears m_ScriptTimings.
		void ClearScriptTimings();
#pragma endregion

#pragma region MultiThreading
		/// Gets the mutex to lock this lua state.
		std::recursive_mutex& GetMutex() { return m_Mutex; };
#pragma endregion

#pragma region
		/// Gets whether the given Lua expression evaluates to true or false.
		/// @param expression The string with the expression to evaluate.
		/// @param consoleErrors Whether to report any errors to the console immediately.
		/// @return Whether the expression was true.
		bool ExpressionIsTrue(const std::string& expression, bool consoleErrors);

		/// Takes a pointer to an object and saves it in the Lua state as a global of a specified variable name.
		/// @param objectToSave The pointer to the object to save. Ownership is NOT transferred!
		/// @param globalName The name of the global var in the Lua state to save the pointer to.
		void SavePointerAsGlobal(void* objectToSave, const std::string& globalName);

		/// Checks if there is anything defined on a specific global var in Lua.
		/// @param globalName The name of the global var in the Lua state to check.
		/// @return Whether that global var has been defined yet in the Lua state.
		bool GlobalIsDefined(const std::string& globalName);

		/// Checks if there is anything defined in a specific index of a table.
		/// @param tableName The name of the table to look inside.
		/// @param indexName The name of the index to check inside that table.
		/// @return Whether that table var has been defined yet in the Lua state.
		bool TableEntryIsDefined(const std::string& tableName, const std::string& indexName);

		/// Clears internal Lua package tables from all user-defined modules. Those must be reloaded with ReloadAllScripts().
		void ClearUserModuleCache();

		/// Clears the Lua script cache.
		void ClearLuaScriptCache();
#pragma endregion

#pragma region Error Handling
		/// Tells whether there are any errors reported waiting to be read.
		/// @return Whether errors exist.
		bool ErrorExists() const;

		/// Returns the last error message from executing scripts.
		/// @return The error string with hopefully meaningful info about what went wrong.
		std::string GetLastError() const;

		/// Clears the last error message, so the Lua state will not be considered to have any errors until the next time there's a script error.
		void ClearErrors();
#pragma endregion

	private:
		/// Gets a random integer between minInclusive and maxInclusive.
		/// @return A random integer between minInclusive and maxInclusive.
		int SelectRand(int minInclusive, int maxInclusive);

		/// Gets a random real between minInclusive and maxInclusive.
		/// @return A random real between minInclusive and maxInclusive.
		double RangeRand(double minInclusive, double maxInclusive);

		/// Gets a random number between -1 and 1.
		/// @return A random number between -1 and 1.
		double NormalRand();

		/// Gets a random number between 0 and 1.
		/// @return A random number between 0 and 1.
		double PosRand();

#pragma region Passthrough LuaMan Functions
		const std::vector<std::string>* DirectoryList(const std::string& path);
		const std::vector<std::string>* FileList(const std::string& path);
		bool FileExists(const std::string& path);
		bool DirectoryExists(const std::string& path);
		int FileOpen(const std::string& path, const std::string& accessMode);
		void FileClose(int fileIndex);
		void FileCloseAll();
		bool FileRemove(const std::string& path);
		bool DirectoryCreate1(const std::string& path);
		bool DirectoryCreate2(const std::string& path, bool recursive);
		bool DirectoryRemove1(const std::string& path);
		bool DirectoryRemove2(const std::string& path, bool recursive);
		bool FileRename(const std::string& oldPath, const std::string& newPath);
		bool DirectoryRename(const std::string& oldPath, const std::string& newPath);
		std::string FileReadLine(int fileIndex);
		void FileWriteLine(int fileIndex, const std::string& line);
		bool FileEOF(int fileIndex);
#pragma endregion

		/// Generates a string that describes the current state of the Lua stack, for debugging purposes.
		/// @return A string that describes the current state of the Lua stack.
		std::string DescribeLuaStack();

		/// Clears all the member variables of this LuaStateWrapper, effectively resetting the members of this abstraction level only.
		void Clear();

		std::unordered_set<MovableObject*> m_RegisteredMOs; //!< The objects using our lua state.
		std::unordered_set<MovableObject*> m_AddedRegisteredMOs; //!< The objects using our lua state that were recently added.

		lua_State* m_State;
		Entity* m_TempEntity; //!< Temporary holder for an Entity object that we want to pass into the Lua state without fuss. Lets you export objects to lua easily.
		std::vector<Entity*> m_TempEntityVector; //!< Temporary holder for a vector of Entities that we want to pass into the Lua state without a fuss. Usually used to pass arguments to special Lua functions.
		std::string m_LastError; //!< Description of the last error that occurred in the script execution.
		std::string_view m_CurrentlyRunningScriptPath; //!< The currently running script filepath.

		// This mutex is more for safety, and with new script/AI architecture we shouldn't ever be locking on a mutex. As such we use this primarily to fire asserts.
		std::recursive_mutex m_Mutex; //!< Mutex to ensure multiple threads aren't running something in this lua state simultaneously.

		struct LuaScriptFunctionObjects {
			std::unordered_map<std::string, LuabindObjectWrapper*> functionNamesAndObjects;
		};
		std::unordered_map<std::string, LuaScriptFunctionObjects> m_ScriptCache;

		std::unordered_map<std::string, PerformanceMan::ScriptTiming> m_ScriptTimings; //!< Internal map of script timings.

		// For determinism, every Lua state has it's own random number generator.
		RandomGenerator m_RandomGenerator; //!< The random number generator used for this lua state.
	};

	typedef std::vector<LuaStateWrapper> LuaStatesArray;

	/// The singleton manager of each Lua state.
	class LuaMan : public Singleton<LuaMan> {
		friend class SettingsMan;
		friend class LuaStateWrapper;

	public:
#pragma region Creation
		/// Constructor method used to instantiate a LuaMan object in system memory. Initialize() should be called before using the object.
		LuaMan();

		/// Makes the LuaMan object ready for use.
		void Initialize();
#pragma endregion

#pragma region Destruction
		/// Destructor method used to clean up a LuaMan object before deletion from system memory.
		~LuaMan();

		/// Destroys and resets (through Clear()) the LuaMan object.
		void Destroy();
#pragma endregion

#pragma region Lua State Handling
		/// Returns our master script state (where activies, global scripts etc run).
		/// @return The master script state.
		LuaStateWrapper& GetMasterScriptState();

		/// Returns our threaded script states which movable objects use.
		/// @return A list of threaded script states.
		LuaStatesArray& GetThreadedScriptStates();

		/// Gets the current thread lua state override that new objects created will be assigned to.
		/// @return The current lua state to force objects to be assigned to.
		LuaStateWrapper* GetThreadLuaStateOverride() const;

		/// Forces all new MOs created in this thread to be assigned to a particular lua state.
		/// This is to ensure that objects created in threaded Lua environments can be safely used.
		/// @param luaState The lua state to force objects to be assigned to.
		void SetThreadLuaStateOverride(LuaStateWrapper* luaState);

		/// Gets the current thread lua state that is running.
		/// @return The current lua state that is running.
		LuaStateWrapper* GetThreadCurrentLuaState() const;

		/// Returns a free threaded script states to assign a movableobject to.
		/// This will be locked to our thread and safe to use - ensure that it'll be unlocked after use!
		/// @return A script state.
		LuaStateWrapper* GetAndLockFreeScriptState();

		/// Clears internal Lua package tables from all user-defined modules. Those must be reloaded with ReloadAllScripts().
		void ClearUserModuleCache();

		/// Adds a function to be called prior to executing lua scripts. This is used to callback into lua from other threads safely.
		/// @param callback The callback function that will be executed.
		void AddLuaScriptCallback(const std::function<void()>& callback);

		/// Executes and clears all pending script callbacks.
		void ExecuteLuaScriptCallbacks();

		/// Gets m_ScriptTimings.
		/// @return m_ScriptTimings.
		const std::unordered_map<std::string, PerformanceMan::ScriptTiming> GetScriptTimings() const;
#pragma endregion

#pragma region File I / O Handling
		/// Returns a vector of all the directories in path, which is relative to the working directory.
		/// @param path Directory path relative to the working directory.
		/// @return A vector of the directories in path.
		const std::vector<std::string>* DirectoryList(const std::string& path);

		/// Returns a vector of all the files in path, which is relative to the working directory.
		/// @param path Directory path relative to the working directory.
		/// @return A vector of the files in path.
		const std::vector<std::string>* FileList(const std::string& path);

		/// Returns whether or not the specified file exists. You can only check for files inside .rte folders in the working directory.
		/// @param path Path to the file. All paths are made absolute by adding current working directory to the specified path.
		/// @return Whether or not the specified file exists.
		bool FileExists(const std::string& path);

		/// Returns whether or not the specified directory exists. You can only check for directories inside .rte folders in the working directory.
		/// @param path Path to the directory. All paths are made absolute by adding current working directory to the specified path.
		/// @return Whether or not the specified file exists.
		bool DirectoryExists(const std::string& path);

		/// Returns whether or not the path refers to an accessible file or directory. You can only check for files or directories inside .rte directories in the working directory.
		/// @param path Path to the file or directory. All paths are made absolute by adding current working directory to the specified path.
		/// @return Whether or not the specified file exists.
		bool IsValidModulePath(const std::string& path);

		/// Opens a file or creates one if it does not exist, depending on access mode. You can open files only inside .rte folders in the working directory. You can't open more that c_MaxOpenFiles file simultaneously.
		/// On Linux will attempt to open a file case insensitively.
		/// @param path Path to the file. All paths are made absolute by adding current working directory to the specified path.
		/// @param mode File access mode. See 'fopen' for list of modes.
		/// @return File index in the opened files array.
		int FileOpen(const std::string& path, const std::string& accessMode);

		/// Closes a previously opened file.
		/// @param fileIndex File index in the opened files array.
		void FileClose(int fileIndex);

		/// Closes all previously opened files.
		void FileCloseAll();

		/// Removes a file.
		/// @param path Path to the file. All paths are made absolute by adding current working directory to the specified path.
		/// @return Whether or not the file was removed.
		bool FileRemove(const std::string& path);

		/// Creates a directory, optionally recursively.
		/// @param path Path to the directory to be created. All paths are made absolute by adding current working directory to the specified path.
		/// @param recursive Whether to recursively create parent directories.
		/// @return Whether or not the directory was removed.
		bool DirectoryCreate(const std::string& path, bool recursive);

		/// Removes a directory, optionally recursively.
		/// @param path Path to the directory to be removed. All paths are made absolute by adding current working directory to the specified path.
		/// @param recursive Whether to recursively remove files and directories.
		/// @return Whether or not the directory was removed.
		bool DirectoryRemove(const std::string& path, bool recursive);

		/// Moves or renames the file oldPath to newPath.
		/// In order to get consistent behavior across Windows and Linux across all 4 combinations of oldPath and newPath being a directory/file,
		/// the newPath isn't allowed to already exist.
		/// @param oldPath Path to the filesystem object. All paths are made absolute by adding current working directory to the specified path.
		/// @param newPath Path to the filesystem object. All paths are made absolute by adding current working directory to the specified path.
		/// @return Whether or not renaming succeeded.
		bool FileRename(const std::string& oldPath, const std::string& newPath);

		/// Moves or renames the directory oldPath to newPath.
		/// In order to get consistent behavior across Windows and Linux across all 4 combinations of oldPath and newPath being a directory/file,
		/// the newPath isn't allowed to already exist.
		/// @param oldPath Path to the filesystem object. All paths are made absolute by adding current working directory to the specified path.
		/// @param newPath Path to the filesystem object. All paths are made absolute by adding current working directory to the specified path.
		/// @return Whether or not renaming succeeded.
		bool DirectoryRename(const std::string& oldPath, const std::string& newPath);

		/// Reads a line from a file.
		/// @param fileIndex File index in the opened files array.
		/// @return Line from file, or empty string on error.
		std::string FileReadLine(int fileIndex);

		/// Writes a text line to a file.
		/// @param fileIndex File index in the opened files array.
		/// @param line String to write.
		void FileWriteLine(int fileIndex, const std::string& line);

		/// Returns true if end of file was reached.
		/// @param fileIndex File index in the opened files array.
		/// @return Whether or not EOF was reached.
		bool FileEOF(int fileIndex);
#pragma endregion

#pragma region Concrete Methods
		/// Updates the state of this LuaMan.
		void Update();

		/// Asynchronously enforces a GC run to occur.
		void StartAsyncGarbageCollection();
#pragma endregion

		/// Clears Script Timings.
		void ClearScriptTimings();

	private:
		static constexpr int c_MaxOpenFiles = 10; //!< The maximum number of files that can be opened with FileOpen at runtime.
		static const std::unordered_set<std::string> c_FileAccessModes; //!< Valid file access modes when opening files with FileOpen.

		std::array<FILE*, c_MaxOpenFiles> m_OpenedFiles; //!< Internal list of opened files used by File functions.

		LuaStateWrapper m_MasterScriptState;
		LuaStatesArray m_ScriptStates;

		std::vector<std::function<void()>> m_ScriptCallbacks; //!< A list of callback functions we'll trigger before processing lua scripts. This allows other threads (i.e pathing requests) to safely trigger callbacks in lua
		std::mutex m_ScriptCallbacksMutex; //!< Mutex to ensure multiple threads aren't modifying the script callback vector at the same time.

		int m_LastAssignedLuaState = 0;

		BS::multi_future<void> m_GarbageCollectionTask;

		/// Clears all the member variables of this LuaMan, effectively resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		LuaMan(const LuaMan& reference) = delete;
		LuaMan& operator=(const LuaMan& rhs) = delete;
	};
} // namespace RTE
