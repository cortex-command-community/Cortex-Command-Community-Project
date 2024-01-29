#pragma once

#include <string>

namespace RTE {

	/// A utility class with misc static functions for different things.
	class GUIUtil {

	public:
		/// Removes the preceding and ending spaces from a c type string.
		/// @param String String to trim.
		/// @return Trimmed string.
		static char* TrimString(char* String);

		/// Gets the text from the clipboard.
		/// @param text Pointer to string receiving the text.
		/// @return True if text was available in the clipboard.
		static bool GetClipboardText(std::string* text);

		/// Sets the text in the clipboard.
		/// @param text String to put into the clipboard.
		/// @return True if text was added to the clipboard.
		static bool SetClipboardText(const std::string& text);
	};
} // namespace RTE
