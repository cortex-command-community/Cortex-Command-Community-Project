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

	/// <summary>
	/// A wrapper for luabind objects, to avoid include problems with luabind.
	/// </summary>
	class SolObjectWrapper {

	public:

#pragma region Creation
		/// <summary>
		/// Constructor method used for SolObjectWrapper.
		/// </summary>
		SolObjectWrapper() = default;

		/// <summary>
		/// Constructor method used to instantiate a SolObjectWrapper object in system memory.
		/// </summary>
		explicit SolObjectWrapper(sol::object *solObject, const std::string_view &filePath, bool ownsObject = true) : m_SolObject(solObject), m_FilePath(filePath), m_OwnsObject(ownsObject) {}
#pragma endregion

#pragma region Destruction
		static void ApplyQueuedDeletions();

		/// <summary>
		/// Destructor method used to clean up a SolObjectWrapper object before deletion from system memory.
		/// </summary>
		~SolObjectWrapper();
#pragma endregion

		/// <summary>
		/// Attempts to copy a luabind object into another state.
		/// </summary>
		SolObjectWrapper GetCopyForState(lua_State& newState) const;

#pragma region Getters
		/// <summary>
		/// Gets the SolObjectWrapper's luabind object. Ownership is NOT transferred!
		/// </summary>
		/// <returns>The SolObjectWrapper's luabind object.</returns>
		sol::object * GetSolObject() const { return m_SolObject; }

		/// <summary>
		/// Gets the SolObjectWrapper's file path.
		/// </summary>
		/// <returns>The SolObjectWrapper's file path.</returns>
		const std::string & GetFilePath() const { return m_FilePath; }
#pragma endregion

	private:

		bool m_OwnsObject; //!< Whether or not we own the sol object this is wrapping.
		sol::object *m_SolObject; //!< The sol object this is wrapping.
		std::string m_FilePath; //!< The filepath the wrapped luabind object represents, if it's a function.

		// Disallow the use of some implicit methods.
		SolObjectWrapper(const SolObjectWrapper &reference) = delete;
		SolObjectWrapper &operator=(const SolObjectWrapper &rhs) = delete;
	};
}
#endif