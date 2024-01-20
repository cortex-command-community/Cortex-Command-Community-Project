#ifndef _RTESolObjectWrapper_
#define _RTESolObjectWrapper_

#include "sol/forward.hpp"

struct lua_State;

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
	class SolObjectWrapper {

	public:
#pragma region Creation
		/// Constructor method used for SolObjectWrapper.
		SolObjectWrapper() = default;

		/// Constructor method used to instantiate a SolObjectWrapper object in system memory.
		explicit SolObjectWrapper(sol::object* solObject, const std::string_view& filePath, bool ownsObject = true) :
		    m_SolObject(solObject), m_FilePath(filePath), m_OwnsObject(ownsObject) {}
#pragma endregion

#pragma region Destruction
		static void ApplyQueuedDeletions();

		/// Destructor method used to clean up a SolObjectWrapper object before deletion from system memory.
		~SolObjectWrapper();
#pragma endregion

		/// Attempts to copy a luabind object into another state.
		SolObjectWrapper GetCopyForState(lua_State& newState) const;

#pragma region Getters
		/// Gets the SolObjectWrapper's luabind object. Ownership is NOT transferred!
		/// @return The SolObjectWrapper's luabind object.
		sol::object* GetSolObject() const { return m_SolObject; }

		/// Gets the SolObjectWrapper's file path.
		/// @return The SolObjectWrapper's file path.
		const std::string& GetFilePath() const { return m_FilePath; }
#pragma endregion

	private:
		bool m_OwnsObject; //!< Whether or not we own the sol object this is wrapping.
		sol::object* m_SolObject; //!< The sol object this is wrapping.
		std::string m_FilePath; //!< The filepath the wrapped luabind object represents, if it's a function.

		// Disallow the use of some implicit methods.
		SolObjectWrapper(const SolObjectWrapper& reference) = delete;
		SolObjectWrapper& operator=(const SolObjectWrapper& rhs) = delete;
	};
} // namespace RTE
#endif
