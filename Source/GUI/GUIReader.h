#pragma once

#include <stack>

namespace RTE {

	/// Reads GUI objects from std::istreams.
	class GUIReader {

	public:
#pragma region Creation
		/// Constructor method used to instantiate a GUIReader object in system memory. Create() should be called before using the object.
		GUIReader();

		/// Makes the GUIReader object ready for use.
		/// @param fileName Path to the file to open for reading. If the file doesn't exist the stream will fail to open.
		/// @return An error return value signaling success or any particular failure.  Anything below 0 is an error signal.
		int Create(const std::string& fileName);
#pragma endregion

#pragma region Getters and Setters
		/// Gets a pointer to the istream of this reader.
		/// @return A pointer to the istream object for this reader.
		std::istream* GetStream() const;

		/// Gets the path of the current file this reader is reading from.
		/// @return A string with the path, relative from the working directory.
		std::string GetCurrentFilePath() const;

		/// Gets the line of the current file line this reader is reading from.
		/// @return A string with the line number that will be read from next.
		std::string GetCurrentFileLine() const;

		/// Returns true if reader was told to skip InlcudeFile statements
		/// @return Returns whether reader was told to skip included files.
		bool GetSkipIncludes() const;

		/// Set whether this reader should skip included files.
		/// @param ski To make reader skip included files pass true, pass false otherwise.
		void SetSkipIncludes(bool skip);
#pragma endregion

#pragma region Reading Operations
		/// Reads the rest of the line from the context object GUIReader's stream current location.
		/// @return The std::string that will hold the line's contents.
		std::string ReadLine();

		/// Reads the next property name from the context object GUIReader's stream after eating all whitespace including newlines up till the first newline char.
		/// Basically gets anything between the last newline before text to the next "=" after that.
		/// @return The whitespace-trimmed std::string that will hold the next property's name.
		std::string ReadPropName();

		/// Reads the next property value from the context object GUIReader's stream after eating all whitespace including newlines up till the first newline char.
		/// Basically gets anything after the last "=" and up to the next newline after that.
		/// @return The whitespace-trimmed std::string that will hold the next property value.
		std::string ReadPropValue();

		/// Lines up the reader with the next property of the current object.
		/// @return Whether there are any more properties to be read by the current object.
		bool NextProperty();

		/// Takes out whitespace from the beginning and the end of a string.
		/// @param stringToTrim String to remove whitespace from.
		/// @return The string that was passed in, sans whitespace in the front and end.
		std::string TrimString(const std::string& stringToTrim) const;

		/// Discards all whitespace, newlines and comment lines (which start with '//') so that the next thing to be read will be actual data.
		/// @return Whether there is more data to read from the file streams after this eat.
		bool DiscardEmptySpace();
#pragma endregion

#pragma region Reader Status
		/// Shows whether this is still OK to read from. If file isn't present, etc, this will return false.
		/// @return Whether this GUIReader's stream is OK or not.
		bool ReaderOK() const;

		/// Makes an error message box pop up for the user that tells them something went wrong with the reading, and where.
		/// @param errorDesc The message describing what's wrong.
		void ReportError(const std::string& errorDesc) const;
#pragma endregion

#pragma region Operator Overloads
		/// Stream extraction operator overloads for all the elemental types.
		/// @param var A reference to the variable that will be filled by the extracted data.
		/// @return A GUIReader reference for further use in an expression.
		GUIReader& operator>>(bool& var);
		GUIReader& operator>>(char& var);
		GUIReader& operator>>(unsigned char& var);
		GUIReader& operator>>(short& var);
		GUIReader& operator>>(unsigned short& var);
		GUIReader& operator>>(int& var);
		GUIReader& operator>>(unsigned int& var);
		GUIReader& operator>>(long& var);
		GUIReader& operator>>(unsigned long& var);
		GUIReader& operator>>(float& var);
		GUIReader& operator>>(double& var);
		GUIReader& operator>>(std::string& var);
#pragma endregion

	protected:
		/// A struct containing information from the currently used stream.
		struct StreamInfo {
			/// Constructor method used to instantiate a StreamInfo object in system memory.
			StreamInfo(std::ifstream* stream, const std::string& filePath, int currentLine, int prevIndent);

			// NOTE: These members are owned by the reader that owns this struct, so are not deleted when this is destroyed.
			std::ifstream* Stream; //!< Currently used stream, is not on the StreamStack until a new stream is opened.
			std::string FilePath; //!< Currently used stream's filepath.
			int CurrentLine; //!< The line number the stream is on.
			int PreviousIndent; //!< Count of tabs encountered on the last line DiscardEmptySpace() discarded.
		};

		std::unique_ptr<std::ifstream> m_Stream; //!< Currently used stream, is not on the StreamStack until a new stream is opened.
		std::stack<StreamInfo> m_StreamStack; //!< Stack of open streams in this GUIReader, each one representing a file opened to read from within another.
		bool m_EndOfStreams; //!< All streams have been depleted.

		std::string m_FilePath; //!< Currently used stream's filepath.
		std::string m_FileName; //!< Only the name of the currently read file, excluding the path.

		int m_PreviousIndent; //!< Count of tabs encountered on the last line DiscardEmptySpace() discarded.
		int m_IndentDifference; //!< Difference in indentation from the last line to the current line.
		std::string m_ReportTabs; //!< String containing the proper amount of tabs for the report.

		int m_CurrentLine; //!< The line number the stream is on.
		bool m_SkipIncludes; //!< Indicates whether reader should skip included files.

		/// When NextProperty() has returned false, indicating that there were no more properties to read on that object,
		/// this is incremented until it matches -m_IndentDifference, and then NextProperty will start returning true again.
		int m_ObjectEndings;

	private:
#pragma region Reading Operations
		/// When ReadPropName encounters the property name "IncludeFile", it will automatically call this function to get started reading on that file.
		/// This will create a new stream to the include file.
		/// @return Whether the include file was found and opened ok or not.
		bool StartIncludeFile();

		/// This should be called when end-of-file is detected in an included file stream.
		/// It will destroy the current stream pop the top stream off the stream stack to resume reading from it instead.
		/// @return Whether there were any stream on the stack to resume.
		bool EndIncludeFile();
#pragma endregion

		/// Clears all the member variables of this GUIReader, effectively resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		GUIReader(const GUIReader& reference) = delete;
		GUIReader& operator=(const GUIReader& rhs) = delete;
	};
} // namespace RTE
