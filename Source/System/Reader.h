#pragma once

#include <functional>
#include <string>
#include <memory>
#include <stack>
#include <istream>

namespace RTE {

	using ProgressCallback = std::function<void(std::string, bool)>; //!< Convenient name definition for the progress report callback function.

	/// Reads RTE objects from std::istreams.
	class Reader {

	public:
#pragma region Creation
		/// Constructor method used to instantiate a Reader object in system memory. Create() should be called before using the object.
		Reader() { Clear(); }

		/// Constructor method used to instantiate a Reader object in system memory and make it ready for reading from the passed in file path.
		/// @param fileName Path to the file to open for reading. If the file doesn't exist the stream will fail to open.
		/// @param overwrites Whether object definitions read here overwrite existing ones with the same names.
		/// @param progressCallback A function pointer to a function that will be called and sent a string with information about the progress of this Reader's reading.
		/// @param failOK Whether it's ok for the file to not be there, ie we're only trying to open, and if it's not there, then fail silently.
		/// @param nonModulePath Whether this Reader is reading from path that is not a DataModule and should just read it as provided.
		Reader(const std::string& fileName, bool overwrites = false, const ProgressCallback& progressCallback = nullptr, bool failOK = false, bool nonModulePath = false);

		/// Constructor method used to instantiate a Reader object in system memory and make it ready for reading from the passed in file path.
		/// @param stream Stream to read from.
		/// @param overwrites Whether object definitions read here overwrite existing ones with the same names.
		/// @param progressCallback A function pointer to a function that will be called and sent a string with information about the progress of this Reader's reading.
		/// @param failOK Whether it's ok for the file to not be there, ie we're only trying to open, and if it's not there, then fail silently.
		Reader(std::unique_ptr<std::istream>&& stream, bool overwrites = false, const ProgressCallback& progressCallback = nullptr, bool failOK = false);

		/// Makes the Reader object ready for use.
		/// @param fileName Path to the file to open for reading. If the file doesn't exist the stream will fail to open.
		/// @param overwrites Whether object definitions read here overwrite existing ones with the same names.
		/// @param progressCallback A function pointer to a function that will be called and sent a string with information about the progress of this Reader's reading.
		/// @param failOK Whether it's ok for the file to not be there, ie we're only trying to open, and if it's not there, then fail silently.
		/// @return An error return value signaling success or any particular failure.  Anything below 0 is an error signal.
		int Create(const std::string& fileName, bool overwrites = false, const ProgressCallback& progressCallback = nullptr, bool failOK = false);

		/// Makes the Reader object ready for use.
		/// @param stream Stream to read from.
		/// @param overwrites Whether object definitions read here overwrite existing ones with the same names.
		/// @param progressCallback A function pointer to a function that will be called and sent a string with information about the progress of this Reader's reading.
		/// @param failOK Whether it's ok for the file to not be there, ie we're only trying to open, and if it's not there, then fail silently.
		/// @return An error return value signaling success or any particular failure.  Anything below 0 is an error signal.
		int Create(std::unique_ptr<std::istream>&& stream, bool overwrites = false, const ProgressCallback& progressCallback = nullptr, bool failOK = false);
#pragma endregion

#pragma region Getters and Setters
		/// Gets the name of the DataModule this Reader is reading from.
		/// @return The name of the DataModule this reader is reading from.
		const std::string& GetReadModuleName() const { return m_DataModuleName; }

		/// Gets the ID of the DataModule this Reader is reading from. If the ID is invalid, attempts to get a valid ID using the DataModule name.
		/// @return The DataModule ID that this Reader is reading from.
		int GetReadModuleID() const;

		/// Gets a pointer to the istream of this reader.
		/// @return A pointer to the istream object for this reader.
		std::istream* GetStream() const { return m_Stream.get(); }

		/// Gets the path of the current file this reader is reading from.
		/// @return A string with the path, relative from the working directory.
		std::string GetCurrentFilePath() const { return m_FilePath; }

		/// Gets the line of the current file line this reader is reading from.
		/// @return A string with the line number that will be read from next.
		std::string GetCurrentFileLine() const { return std::to_string(m_CurrentLine); }

		/// Shows whether objects read from this will be overwriting any existing ones with the same names.
		/// @return Whether this overwrites or not.
		bool GetPresetOverwriting() const { return m_OverwriteExisting; }

		/// Sets whether objects read from this will be overwriting any existing ones with the same names.
		/// @param overwrites Whether this should overwrite existing definitions or not.
		void SetPresetOverwriting(bool overwrites = true) { m_OverwriteExisting = overwrites; }

		/// Returns true if reader was told to skip InlcudeFile statements
		/// @return Returns whether reader was told to skip included files.
		bool GetSkipIncludes() const { return m_SkipIncludes; };

		/// Set whether this reader should skip included files.
		/// @param skip To make reader skip included files pass true, pass false otherwise.
		void SetSkipIncludes(bool skip) { m_SkipIncludes = skip; };
#pragma endregion

#pragma region Reading Operations
		/// Reads a file and constructs a string from all its contents.
		/// @return A string containing the whole file contents.
		std::string WholeFileAsString() const;

		/// Reads the rest of the line from the context object Reader's stream current location.
		/// @return The std::string that will hold the line's contents.
		std::string ReadLine();

		/// Reads the next property name from the context object Reader's stream after eating all whitespace including newlines up till the first newline char.
		/// Basically gets anything between the last newline before text to the next "=" after that.
		/// @return The whitespace-trimmed std::string that will hold the next property's name.
		std::string ReadPropName();

		/// Reads the next property value from the context object Reader's stream after eating all whitespace including newlines up till the first newline char.
		/// Basically gets anything after the last "=" and up to the next newline after that.
		/// @return The whitespace-trimmed std::string that will hold the next property value.
		std::string ReadPropValue();

		/// Lines up the reader with the next property of the current object.
		/// @return Whether there are any more properties to be read by the current object.
		bool NextProperty();

		/// Notifies the reader that we're starting reading a new object, by making it expect another level of indentation.
		void StartObject() { --m_ObjectEndings; }

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
		/// @return Whether this Reader's stream is OK or not.
		bool ReaderOK() const { return m_Stream.get() && m_Stream->good(); }

		/// Makes an error message box pop up for the user that tells them something went wrong with the reading, and where.
		/// @param errorDesc The message describing what's wrong.
		void ReportError(const std::string& errorDesc) const;
#pragma endregion

#pragma region Operator Overloads
		/// Stream extraction operator overloads for all the elemental types.
		/// @param var A reference to the variable that will be filled by the extracted data.
		/// @return A Reader reference for further use in an expression.
		Reader& operator>>(bool& var) {
			DiscardEmptySpace();
			*m_Stream >> var;
			return *this;
		}
		Reader& operator>>(char& var) {
			DiscardEmptySpace();
			*m_Stream >> var;
			return *this;
		}
		Reader& operator>>(unsigned char& var) {
			DiscardEmptySpace();
			int temp;
			*m_Stream >> temp;
			var = temp;
			return *this;
		}
		Reader& operator>>(short& var) {
			DiscardEmptySpace();
			*m_Stream >> var;
			return *this;
		}
		Reader& operator>>(unsigned short& var) {
			DiscardEmptySpace();
			*m_Stream >> var;
			return *this;
		}
		Reader& operator>>(int& var) {
			DiscardEmptySpace();
			*m_Stream >> var;
			return *this;
		}
		Reader& operator>>(unsigned int& var) {
			DiscardEmptySpace();
			*m_Stream >> var;
			return *this;
		}
		Reader& operator>>(long& var) {
			DiscardEmptySpace();
			*m_Stream >> var;
			return *this;
		}
		Reader& operator>>(unsigned long& var) {
			DiscardEmptySpace();
			*m_Stream >> var;
			return *this;
		}
		// Yeah, this is dumb - read as double and cast.
		// This is because, for whatever fucking reason, iostream can save out floats at a precision that it's then unable to read...
		Reader& operator>>(float& var) {
			DiscardEmptySpace();
			double var2;
			*m_Stream >> var2;
			var = static_cast<float>(var2);
			return *this;
		}
		Reader& operator>>(double& var) {
			DiscardEmptySpace();
			*m_Stream >> var;
			return *this;
		}
		Reader& operator>>(std::string& var) {
			var.assign(ReadLine());
			return *this;
		}
#pragma endregion

	protected:
		/// A struct containing information from the currently used stream.
		struct StreamInfo {
			/// Constructor method used to instantiate a StreamInfo object in system memory.
			StreamInfo(std::istream* stream, const std::string& filePath, int currentLine, int prevIndent) :
			    Stream(stream), FilePath(filePath), CurrentLine(currentLine), PreviousIndent(prevIndent) {}

			// NOTE: These members are owned by the reader that owns this struct, so are not deleted when this is destroyed.
			std::istream* Stream; //!< Currently used stream, is not on the StreamStack until a new stream is opened.
			std::string FilePath; //!< Currently used stream's filepath.
			int CurrentLine; //!< The line number the stream is on.
			int PreviousIndent; //!< Count of tabs encountered on the last line DiscardEmptySpace() discarded.
		};

		std::unique_ptr<std::istream> m_Stream; //!< Currently used stream, is not on the StreamStack until a new stream is opened.
		std::stack<StreamInfo> m_StreamStack; //!< Stack of open streams in this Reader, each one representing a file opened to read from within another.
		bool m_EndOfStreams; //!< All streams have been depleted.

		ProgressCallback m_ReportProgress; //!< Function pointer to report our reading progress to, by calling it and passing a descriptive string to it.

		std::string m_FilePath; //!< Currently used stream's filepath.
		std::string m_FileName; //!< Only the name of the currently read file, excluding the path.

		std::string m_DataModuleName; //!< The current name of the data module being read from, including the .rte extension.
		int m_DataModuleID; //!< The current ID of the data module being read from.

		int m_PreviousIndent; //!< Count of tabs encountered on the last line DiscardEmptySpace() discarded.
		int m_IndentDifference; //!< Difference in indentation from the last line to the current line.
		std::string m_ReportTabs; //!< String containing the proper amount of tabs for the report.

		int m_CurrentLine; //!< The line number the stream is on.
		bool m_OverwriteExisting; //!< Whether object instances read from this should overwrite any already existing ones with the same names.
		bool m_SkipIncludes; //!< Indicates whether reader should skip included files.
		bool m_CanFail; //!< Whether it's ok for the Reader to fail reading a file and fail silently instead of aborting.
		bool m_NonModulePath; //!< Whether this Reader is reading from path that is not a DataModule and should just read it as provided.

		std::stack<int> m_BlockCommentOpenTagLines; //<! Stores lines on which block comment open tags are encountered. Used for error reporting when a file stream ends with an open block comment.

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

		/// Clears all the member variables of this Reader, effectively resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		Reader(const Reader& reference) = delete;
		Reader& operator=(const Reader& rhs) = delete;
	};
} // namespace RTE
