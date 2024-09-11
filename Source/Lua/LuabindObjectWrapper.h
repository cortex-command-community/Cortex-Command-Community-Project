#pragma once

#include <string>
#include <string_view>

struct lua_State;

namespace luabind::adl {
	class object;
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

	/// A wrapper for luabind objects, to avoid include problems with luabind.
	class LuabindObjectWrapper {

	public:
#pragma region Creation
		/// Constructor method used for LuabindObjectWrapper.
		LuabindObjectWrapper() = default;

		/// Constructor method used to instantiate a LuabindObjectWrapper object in system memory.
		explicit LuabindObjectWrapper(luabind::adl::object* luabindObject, const std::string_view& filePath, bool ownsObject = true) :
		    m_OwnsObject(ownsObject), m_LuabindObject(luabindObject), m_FilePath(filePath) {}
#pragma endregion

#pragma region Destruction
		static void ApplyQueuedDeletions();

		/// Destructor method used to clean up a LuabindObjectWrapper object before deletion from system memory.
		~LuabindObjectWrapper();
#pragma endregion

		/// Attempts to copy a luabind object into another state.
		LuabindObjectWrapper GetCopyForState(lua_State& newState) const;

#pragma region Getters
		/// Gets the LuabindObjectWrapper's luabind object. Ownership is NOT transferred!
		/// @return The LuabindObjectWrapper's luabind object.
		luabind::adl::object* GetLuabindObject() const { return m_LuabindObject; }

		/// Gets the LuabindObjectWrapper's file path.
		/// @return The LuabindObjectWrapper's file path.
		const std::string& GetFilePath() const { return m_FilePath; }
#pragma endregion

	private:
		bool m_OwnsObject; //!< Whether or not we own the luabind object this is wrapping.
		luabind::adl::object* m_LuabindObject; //!< The luabind object this is wrapping.
		std::string m_FilePath; //!< The filepath the wrapped luabind object represents, if it's a function.

		// Disallow the use of some implicit methods.
		LuabindObjectWrapper(const LuabindObjectWrapper& reference) = delete;
		LuabindObjectWrapper& operator=(const LuabindObjectWrapper& rhs) = delete;
	};
} // namespace RTE
