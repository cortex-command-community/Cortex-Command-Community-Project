#pragma once

#include "PropertyWriter.h"

#include <format>
#include <memory>
#include <ostream>
#include <string>
#include <utility>

namespace RTE {

	/// Writes RTE objects to std::ostreams as INI, each written only as far as it differs from its reference.
	/// An Entity with a preset is written as a CopyOf it, one without against a new object of its class, and a Serializable held by value against what it'll be read into.
	class IniWriter : public PropertyWriter<IniWriter> {

	public:
#pragma region Creation
		/// Constructor method used to instantiate an IniWriter object in system memory. Create() should be called before using the object.
		IniWriter() { Clear(); }

		/// Constructor method used to instantiate an IniWriter object in system memory and make it ready for writing to the passed in file path.
		/// @param filename Path to the file to open for writing. If the directory doesn't exist the stream will fail to open.
		/// @param append Whether to append to the file if it exists, or to overwrite it.
		/// @param createDir Whether to create the directory path to the file name before attempting to open the stream, in case it doesn't exist.
		IniWriter(const std::string& fileName, bool append = false, bool createDir = false);

		/// Constructor method used to instantiate an IniWriter object in system memory and make it ready for writing to the passed in file path.
		/// @param stream Stream to write to.
		IniWriter(std::unique_ptr<std::ostream>&& stream);

		/// Makes the IniWriter object ready for use.
		/// @param filename Path to the file to open for writing. If the directory doesn't exist the stream will fail to open.
		/// @param append Whether to append to the file if it exists, or to overwrite it.
		/// @param createDir Whether to create the directory path to the file name before attempting to open the stream, in case it doesn't exist.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(const std::string& fileName, bool append = false, bool createDir = false);

		/// Makes the IniWriter object ready for use.
		/// @param stream Stream to write to.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(std::unique_ptr<std::ostream>&& stream);
#pragma endregion

#pragma region Getters
		/// Gets the path to the file being written.
		/// @return The full path to the file being written.
		const std::string& GetFilePath() const { return m_FilePath; }

		/// Gets the name (without path) of the file being written.
		/// @return The name of file being written.
		const std::string& GetFileName() const { return m_FileName; }

		/// Gets the folder path (without filename) to where the file is being written.
		/// @return The name of folder being written in.
		const std::string& GetFolderPath() const { return m_FolderPath; }
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
		void NewProperty(PropName propName) const {
			NewLine();
			*m_Stream << propName.m_Text << " = ";
		}

		/// Marks that there is a null reference to an object here.
		void NoObject() const { *m_Stream << "None"; }

		/// Writes an Entity's CopyOf, if it's written as a copy of a preset, and its name.
		void WriteIdentity();

		/// Writes a Serializable held by value, against the value it'll be read into.
		/// @param value The Serializable to write.
		/// @param referenceValue The reference's value of the same property, which this will be read into.
		void WriteInto(const Serializable& value, const Serializable& referenceValue);

		/// Writes an object only as far as one of the classes it derives from writes it, against what it would be written against as a whole, for writing just part of a preset.
		/// @param object The object to write.
		template <typename Class> void WriteAs(const Class& object) {
			const auto [reference, referenceIsPreset] = ReferenceFor(object);
			m_Frames.push_back({&object, reference, referenceIsPreset});
			object.Class::WriteTo(*this);
			m_Frames.pop_back();
		}

		/// Writes every item of a collection whose order means nothing.
		/// @param items The collection.
		/// @param writeItem Writes one item.
		template <typename Collection, typename WriteItem> void WriteUnordered(const Collection& items, WriteItem&& writeItem) {
			for (const auto& item: items) {
				writeItem(item);
			}
		}
#pragma endregion

#pragma region Writer Status
		/// Shows whether the writer is ready to start accepting data streamed to it.
		/// @return Whether the writer is ready to start accepting data streamed to it or not.
		bool WriterOK() const { return m_Stream.get() && m_Stream->good(); }

		/// Returns the underlying stream.
		std::ostream* GetStream() { return m_Stream.get(); }

		/// Flushes and closes the output stream of this IniWriter. This happens automatically at destruction but needs to be called manually if a written file must be read from in the same scope.
		void EndWrite() {
			m_Stream->flush();
			m_Stream.reset();
		}
#pragma endregion

#pragma region Operator Overloads
		/// Elemental types stream insertions. Stream insertion operator overloads for all the elemental types.
		/// @param var A reference to the variable that will be written to the ostream.
		/// @return An IniWriter reference for further use in an expression.
		IniWriter& operator<<(const bool& var) {
			*m_Stream << var;
			return *this;
		}
		IniWriter& operator<<(const char& var) {
			*m_Stream << var;
			return *this;
		}
		IniWriter& operator<<(const unsigned char& var) {
			int temp = var;
			*m_Stream << temp;
			return *this;
		}
		IniWriter& operator<<(const short& var) {
			*m_Stream << var;
			return *this;
		}
		IniWriter& operator<<(const unsigned short& var) {
			*m_Stream << var;
			return *this;
		}
		IniWriter& operator<<(const int& var) {
			*m_Stream << var;
			return *this;
		}
		IniWriter& operator<<(const unsigned int& var) {
			*m_Stream << var;
			return *this;
		}
		IniWriter& operator<<(const long& var) {
			*m_Stream << var;
			return *this;
		}
		IniWriter& operator<<(const long long& var) {
			*m_Stream << var;
			return *this;
		}
		IniWriter& operator<<(const unsigned long& var) {
			*m_Stream << var;
			return *this;
		}
		IniWriter& operator<<(const unsigned long long& var) {
			*m_Stream << var;
			return *this;
		}
		// Floating point numbers get written with as many digits as it takes for them to be read back as exactly the same number, and no more.
		IniWriter& operator<<(const float& var) {
			*m_Stream << std::format("{}", var);
			return *this;
		}
		IniWriter& operator<<(const double& var) {
			*m_Stream << std::format("{}", var);
			return *this;
		}
		IniWriter& operator<<(const char* var) {
			*m_Stream << var;
			return *this;
		}
		IniWriter& operator<<(const std::string& var) {
			*m_Stream << var;
			return *this;
		}

		/// Writes a Serializable that's read as a new object: an Entity as a CopyOf its preset, or against a new object of its class if it has none, and anything else in full.
		/// @param object The Serializable to write.
		/// @return An IniWriter reference for further use in an expression.
		IniWriter& operator<<(const Serializable& object);

		/// Writes a Serializable that's read as a new object, or None if there isn't one.
		/// @param object A pointer to the Serializable to write, or nullptr.
		/// @return An IniWriter reference for further use in an expression.
		IniWriter& operator<<(const Serializable* object);
#pragma endregion

	protected:
		std::unique_ptr<std::ostream> m_Stream; //!< Stream used for writing.
		std::string m_FilePath; //!< Currently used stream's filepath.
		std::string m_FolderPath; //!< Only the path to the folder that we are writing a file in, excluding the filename.
		std::string m_FileName; //!< Only the name of the currently read file, excluding the path.
		int m_IndentCount; //!< Indentation counter.

	private:
		/// Gets what a Serializable that's read as a new object is written against: an Entity's preset, or a new object of its class if it has none, and nothing for anything else.
		/// @param object The Serializable about to be written.
		/// @return What to write it against, or nullptr to write all of it, and whether that's a preset it's written as a CopyOf.
		std::pair<const Serializable*, bool> ReferenceFor(const Serializable& object) const;

		/// Clears all the member variables of this IniWriter, effectively resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		IniWriter(const IniWriter& reference) = delete;
		IniWriter& operator=(const IniWriter& rhs) = delete;
	};
} // namespace RTE
