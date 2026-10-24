#pragma once

#include <string>
#include <string_view>
#include <vector>

struct lua_State;

namespace luabridge {
	class LuaRef;
}

namespace RTE {

#pragma region Global Macro Definitions
#define ScriptFunctionNames(...) \
	virtual std::vector<std::string> GetSupportedScriptFunctionNames() const { return {__VA_ARGS__}; }

#define AddScriptFunctionNames(PARENT, ...) \
	std::vector<std::string> GetSupportedScriptFunctionNames() const override { \
		std::vector<std::string> functionNames = PARENT::GetSupportedScriptFunctionNames(); \
		functionNames.insert(functionNames.end(), {__VA_ARGS__}); \
		return functionNames; \
	}
#pragma endregion

	/// A wrapper for references to Lua objects, so that LuaBridge doesn't have to be included everywhere they're stored.
	class LuaObjectWrapper {

	public:
#pragma region Creation
		/// Constructor method used for LuaObjectWrapper.
		LuaObjectWrapper() = default;

		/// Constructor method used to instantiate a LuaObjectWrapper object in system memory.
		explicit LuaObjectWrapper(luabridge::LuaRef* luaObject, const std::string_view& filePath, bool ownsObject = true) :
		    m_OwnsObject(ownsObject), m_LuaObject(luaObject), m_FilePath(filePath) {}
#pragma endregion

#pragma region Destruction
		/// Deletes the Lua object references whose deletion was put off until it was safe, see the destructor.
		static void ApplyQueuedDeletions();

		/// Destructor method used to clean up a LuaObjectWrapper object before deletion from system memory.
		~LuaObjectWrapper();
#pragma endregion

		/// Attempts to copy a Lua object into another state.
		LuaObjectWrapper GetCopyForState(lua_State& newState) const;

#pragma region Getters
		/// Gets the LuaObjectWrapper's Lua object reference. Ownership is NOT transferred!
		/// @return The LuaObjectWrapper's Lua object reference.
		luabridge::LuaRef* GetLuaObject() const { return m_LuaObject; }

		/// Gets the LuaObjectWrapper's file path.
		/// @return The LuaObjectWrapper's file path.
		const std::string& GetFilePath() const { return m_FilePath; }
#pragma endregion

	private:
		bool m_OwnsObject; //!< Whether or not we own the Lua object reference this is wrapping.
		luabridge::LuaRef* m_LuaObject; //!< The Lua object reference this is wrapping.
		std::string m_FilePath; //!< The filepath the wrapped Lua object represents, if it's a function.

		// Disallow the use of some implicit methods.
		LuaObjectWrapper(const LuaObjectWrapper& reference) = delete;
		LuaObjectWrapper& operator=(const LuaObjectWrapper& rhs) = delete;
	};
} // namespace RTE
