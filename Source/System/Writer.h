#pragma once

#include <string>
#include <memory>
#include <ostream>

namespace RTE {

	/// Writes RTE objects to std::ostreams.
	class Writer {

	public:
#pragma region Creation
		/// Constructor method used to instantiate a Writer object in system memory. Create() should be called before using the object.
		Writer() { Clear(); }

		/// Constructor method used to instantiate a Writer object in system memory and make it ready for writing to the passed in file path.
		/// @param filename Path to the file to open for writing. If the directory doesn't exist the stream will fail to open.
		/// @param append Whether to append to the file if it exists, or to overwrite it.
		/// @param createDir Whether to create the directory path to the file name before attempting to open the stream, in case it doesn't exist.
		Writer(const std::string& fileName, bool append = false, bool createDir = false);

		/// Constructor method used to instantiate a Writer object in system memory and make it ready for writing to the passed in file path.
		/// @param stream Stream to write to.
		Writer(std::unique_ptr<std::ostream>&& stream);

		/// Makes the Writer object ready for use.
		/// @param filename Path to the file to open for writing. If the directory doesn't exist the stream will fail to open.
		/// @param append Whether to append to the file if it exists, or to overwrite it.
		/// @param createDir Whether to create the directory path to the file name before attempting to open the stream, in case it doesn't exist.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(const std::string& fileName, bool append = false, bool createDir = false);

		/// Makes the Writer object ready for use.
		/// @param stream Stream to write to.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(std::unique_ptr<std::ostream>&& stream);
#pragma endregion

#pragma region Getters
		/// Gets the path to the file being written.
		/// @return The full path to the file being written.
		std::string GetFilePath() const { return m_FilePath; }

		/// Gets the name (without path) of the file being written.
		/// @return The name of file being written.
		std::string GetFileName() const { return m_FileName; }

		/// Gets the folder path (without filename) to where the file is being written.
		/// @return The name of folder being written in.
		std::string GetFolderPath() const { return m_FolderPath; }
#pragma endregion

#pragma region Writing Operations
		/// Used to specify the start of an object to be written.
		/// @param className The class name of the object about to be written.
		void ObjectStart(const std::string& className) {
			*m_Stream << className;
			++m_IndentCount;
		}

		/// Used to specify the end of an object that has just been written.
		void ObjectEnd() {
			--m_IndentCount;
			if (m_IndentCount == 0) {
				NewLine(false, 2);
			}
		}

		/// Creates a new line that can be properly indented.
		/// @param toIndent Whether to indent the new line or not.
		/// @param lineCount How many new lines to create.
		void NewLine(bool toIndent = true, int lineCount = 1) const;

		/// Creates a new line and writes the specified string to it.
		/// @param textString The text string to write to the new line.
		/// @param toIndent Whether to indent the new line or not.
		void NewLineString(const std::string& textString, bool toIndent = true) const {
			NewLine(toIndent);
			*m_Stream << textString;
		}

		/// Creates a new line and fills it with slashes to create a divider line for INI.
		/// @param toIndent Whether to indent the new line or not.
		/// @param dividerLength The length of the divider (number of slashes).
		void NewDivider(bool toIndent = true, int dividerLength = 72) const {
			NewLine(toIndent);
			*m_Stream << std::string(dividerLength, '/');
		}

		/// Creates a new line and writes the name of the property in preparation to writing it's value.
		/// @param propName The name of the property to be written.
		void NewProperty(const std::string& propName) const {
			NewLine();
			*m_Stream << propName + " = ";
		}

		/// Creates a new line and writes the name of the specified property, followed by its set value.
		/// @param propName The name of the property to be written.
		/// @param propValue The value of the property.
		template <typename Type> void NewPropertyWithValue(const std::string& propName, const Type& propValue) {
			NewProperty(propName);
			*this << propValue;
		}

		/// Marks that there is a null reference to an object here.
		void NoObject() const { *m_Stream << "None"; }
#pragma endregion

#pragma region Writer Status
		/// Shows whether the writer is ready to start accepting data streamed to it.
		/// @return Whether the writer is ready to start accepting data streamed to it or not.
		bool WriterOK() const { return m_Stream.get() && m_Stream->good(); }

		/// Flushes and closes the output stream of this Writer. This happens automatically at destruction but needs to be called manually if a written file must be read from in the same scope.
		void EndWrite() {
			m_Stream->flush();
			m_Stream.reset();
		}
#pragma endregion

#pragma region Operator Overloads
		/// Elemental types stream insertions. Stream insertion operator overloads for all the elemental types.
		/// @param var A reference to the variable that will be written to the ostream.
		/// @return A Writer reference for further use in an expression.
		Writer& operator<<(const bool& var) {
			*m_Stream << var;
			return *this;
		}
		Writer& operator<<(const char& var) {
			*m_Stream << var;
			return *this;
		}
		Writer& operator<<(const unsigned char& var) {
			int temp = var;
			*m_Stream << temp;
			return *this;
		}
		Writer& operator<<(const short& var) {
			*m_Stream << var;
			return *this;
		}
		Writer& operator<<(const unsigned short& var) {
			*m_Stream << var;
			return *this;
		}
		Writer& operator<<(const int& var) {
			*m_Stream << var;
			return *this;
		}
		Writer& operator<<(const unsigned int& var) {
			*m_Stream << var;
			return *this;
		}
		Writer& operator<<(const long& var) {
			*m_Stream << var;
			return *this;
		}
		Writer& operator<<(const long long& var) {
			*m_Stream << var;
			return *this;
		}
		Writer& operator<<(const unsigned long& var) {
			*m_Stream << var;
			return *this;
		}
		Writer& operator<<(const unsigned long long& var) {
			*m_Stream << var;
			return *this;
		}
		Writer& operator<<(const float& var) {
			*m_Stream << var;
			return *this;
		}
		Writer& operator<<(const double& var) {
			*m_Stream << var;
			return *this;
		}
		Writer& operator<<(const char* var) {
			*m_Stream << var;
			return *this;
		}
		Writer& operator<<(const std::string& var) {
			*m_Stream << var;
			return *this;
		}
#pragma endregion

	protected:
		std::unique_ptr<std::ostream> m_Stream; //!< Stream used for writing.
		std::string m_FilePath; //!< Currently used stream's filepath.
		std::string m_FolderPath; //!< Only the path to the folder that we are writing a file in, excluding the filename.
		std::string m_FileName; //!< Only the name of the currently read file, excluding the path.
		int m_IndentCount; //!< Indentation counter.

	private:
		/// Clears all the member variables of this Writer, effectively resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		Writer(const Writer& reference) = delete;
		Writer& operator=(const Writer& rhs) = delete;
	};
} // namespace RTE
