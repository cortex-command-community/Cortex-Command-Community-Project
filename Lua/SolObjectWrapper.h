#ifndef _RTESolObjectWrapper_
#define _RTESolObjectWrapper_

#include "sol/forward.hpp"

namespace RTE {

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
		explicit SolObjectWrapper(sol::object *solObject, const std::string_view &filePath) : m_SolObject(solObject), m_FilePath(filePath) {}
#pragma endregion

#pragma region Destruction
		static void ApplyQueuedDeletions();

		/// <summary>
		/// Destructor method used to clean up a SolObjectWrapper object before deletion from system memory.
		/// </summary>
		~SolObjectWrapper();
#pragma endregion

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

		sol::object *m_SolObject; //!< The sol object this is wrapping.
		std::string m_FilePath; //!< The filepath the wrapped luabind object represents, if it's a function.

		// Disallow the use of some implicit methods.
		SolObjectWrapper(const SolObjectWrapper &reference) = delete;
		SolObjectWrapper &operator=(const SolObjectWrapper &rhs) = delete;
	};
}
#endif