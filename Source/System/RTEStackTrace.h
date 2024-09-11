#pragma once

#include "StackWalker/StackWalker.h"

#include <string>
#include <sstream>

namespace RTE {

	/// Class for dumping stack traces on Windows.
	/// Wraps the StackWalker class which uses WinAPI Debug Help Library routines.
	class RTEStackTrace : public StackWalker {

	public:
		/// Constructor method used to instantiate an RTEStackTrace object in system memory and make it ready for use.
		/// @param options
		RTEStackTrace() :
		    StackWalker() {}

		/// Destructor method used to clean up a RTEStackTrace object before deletion from system memory.
		~RTEStackTrace() override = default;

		/// Gets the current call stack as a string.
		/// @param handle Handle to the current process. If none provided will get the current thread handle.
		/// @param context Register data. If none provided will get it from the caller.
		/// @return A string with the call stack.
		std::string GetCallStackAsString(const HANDLE& handle = nullptr, const CONTEXT* context = nullptr);

	protected:
		/// Redirects the output string to the member string stream.
		/// @param text The output string. Provided by the base class method.
		void OnOutput(LPCSTR text) override;

	private:
		std::stringstream m_CallstackStream; //!< Call stack output stream.

		// Disallow the use of some implicit methods.
		RTEStackTrace(const RTEStackTrace& reference) = delete;
		RTEStackTrace& operator=(const RTEStackTrace& rhs) = delete;
	};
} // namespace RTE
