#pragma once

#include <string>
#include <memory>
#include <fstream>

namespace RTE {

	/// Writes GUI objects to std::ostreams.
	class GUIWriter {

	public:
#pragma region Creation
		/// Constructor method used to instantiate a GUIWriter object in system memory. Create() should be called before using the object.
		GUIWriter();

		/// Makes the GUIWriter object ready for use.
		/// @param filename Path to the file to open for writing. If the directory doesn't exist the stream will fail to open.
		/// @param append Whether to append to the file if it exists, or to overwrite it.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(const std::string& fileName, bool append = false);
#pragma endregion

#pragma region Getters
		/// Gets the path to the file being written.
		/// @return The full path to the file being written.
		std::string GetFilePath() const;

		/// Gets the name (without path) of the file being written.
		/// @return The name of file being written.
		std::string GetFileName() const;

		/// Gets the folder path (without filename) to where the file is being written.
		/// @return The name of folder being written in.
		std::string GetFolderPath() const;
#pragma endregion

#pragma region Writing Operations
		/// Used to specify the start of an object to be written.
		/// @param className The class name of the object about to be written.
		void ObjectStart(const std::string& className);

		/// Used to specify the end of an object that has just been written.
		void ObjectEnd();

		/// Creates a new line that can be properly indented.
		/// @param toIndent Whether to indent the new line or not.
		/// @param lineCount How many new lines to create.
		void NewLine(bool toIndent = true, int lineCount = 1) const;

		/// Creates a new line and writes the specified string to it.
		/// @param textString The text string to write to the new line.
		/// @param toIndent Whether to indent the new line or not.
		void NewLineString(const std::string& textString, bool toIndent = true) const;

		/// Creates a new line and fills it with slashes to create a divider line for INI.
		/// @param toIndent Whether to indent the new line or not.
		/// @param dividerLength The length of the divider (number of slashes).
		void NewDivider(bool toIndent = true, int dividerLength = 72) const;

		/// Creates a new line and writes the name of the property in preparation to writing it's value.
		/// @param propName The name of the property to be written.
		void NewProperty(const std::string& propName) const;
#pragma endregion

#pragma region Writer Status
		/// Shows whether the writer is ready to start accepting data streamed to it.
		/// @return Whether the writer is ready to start accepting data streamed to it or not.
		bool WriterOK() const;

		/// Flushes and closes the output stream of this GUIWriter. This happens automatically at destruction but needs to be called manually if a written file must be read from in the same scope.
		void EndWrite() const;
#pragma endregion

#pragma region Operator Overloads
		/// Elemental types stream insertions. Stream insertion operator overloads for all the elemental types.
		/// @param var A reference to the variable that will be written to the ostream.
		/// @return A GUIWriter reference for further use in an expression.
		GUIWriter& operator<<(const bool& var);
		GUIWriter& operator<<(const char& var);
		GUIWriter& operator<<(const unsigned char& var);
		GUIWriter& operator<<(const short& var);
		GUIWriter& operator<<(const unsigned short& var);
		GUIWriter& operator<<(const int& var);
		GUIWriter& operator<<(const unsigned int& var);
		GUIWriter& operator<<(const long& var);
		GUIWriter& operator<<(const long long& var);
		GUIWriter& operator<<(const unsigned long& var);
		GUIWriter& operator<<(const unsigned long long& var);
		GUIWriter& operator<<(const float& var);
		GUIWriter& operator<<(const double& var);
		GUIWriter& operator<<(const char* var);
		GUIWriter& operator<<(const std::string& var);
#pragma endregion

	protected:
		std::unique_ptr<std::ofstream> m_Stream; //!< Stream used for writing to files.
		std::string m_FilePath; //!< Currently used stream's filepath.
		std::string m_FolderPath; //!< Only the path to the folder that we are writing a file in, excluding the filename.
		std::string m_FileName; //!< Only the name of the currently read file, excluding the path.
		int m_IndentCount; //!< Indentation counter.

	private:
		/// Clears all the member variables of this GUIWriter, effectively resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		GUIWriter(const GUIWriter& reference) = delete;
		GUIWriter& operator=(const GUIWriter& rhs) = delete;
	};
} // namespace RTE
