#pragma once

#include <string>
#include <memory>
#include <ostream>

#include "Hash.h"

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

		/// Writes a collection of pointers where order matters, written to assume it has corresponding data in the hash structure provided.
		/// @param clearPhrase The the phrase used to wipe the contents of the collection, if the referent's is incompatible.
		/// @param insertionPhrase The the phrase used to push an element to the back of the collection.
		/// @param propValue The sequential collection of pointers in question.
		/// @param hashData A modifiable reference to the HashingData relevant to writing this object, which should correspond to it's preset's.
		template <typename Type>
		void NewPointerSequence(const std::string& clearPhrase, const std::string& insertionPhrase, const Type& propValue, HashingData& hashData) {
			// Taking the optimistic assumption that the items in this list can simply be pasted after the reference's at the point that they diverge, start an iterator at the beginning.
			bool areItemsConcatenated = true;
			auto itemItr = propValue.begin();

			// Check the assumption and find where the reference's list diverges.
			for (size_t refIndex = 0; refIndex < hashData.m_ParseValues.front(); refIndex++) {
				if (areItemsConcatenated) {
					if (itemItr != propValue.end()) {
						if ((*itemItr)->Hash().m_Hash != hashData.m_Constituents.front()) {
							areItemsConcatenated = false;
						}
						itemItr++;
					} else
						areItemsConcatenated = false;
				}

				// Iterate through the rest of the list regardless to consume the indicated number of constituent hashes.
				hashData.m_Constituents.pop_front();
			}

			// Assumption was wrong: clear the collection out, and start from the beginning again.
			if (!areItemsConcatenated) {
				if (hashData.m_ParseValues.front() > 0) {
					NewPropertyWithValue(clearPhrase, 1);
				}

				itemItr = propValue.begin();
			}
			
			// For all items determined to need specification, do so.
			while (itemItr != propValue.end()) {
				NewPropertyWithValue(insertionPhrase, **(itemItr++));
			}

			// Value was used for parsing, eject it.
			hashData.m_ParseValues.pop_front();
		}

		/// Writes a collection of items where order matters, written to assume it has corresponding data in the hash structure provided.
		/// @param clearPhrase The the phrase used to wipe the contents of the collection, if the referent's is incompatible.
		/// @param insertionPhrase The the phrase used to push an element to the back of the collection.
		/// @param propValue The sequential collection of items in question.
		/// @param hashData A modifiable reference to the HashingData relevant to writing this object, which should correspond to it's preset's.
		template <typename Type>
		void NewSequence(const std::string& clearPhrase, const std::string& insertionPhrase, const Type& propValue, HashingData& hashData) {
			// Taking the optimistic assumption that the items in this list can simply be pasted after the reference's at the point that they diverge, start an iterator at the beginning.
			bool areItemsConcatenated = true;
			auto itemItr = propValue.begin();

			// Check the assumption and find where the reference's list diverges.
			for (size_t refIndex = 0; refIndex < hashData.m_ParseValues.front(); refIndex++) {
				if (areItemsConcatenated) {
					if (itemItr != propValue.end()) {
						if (itemItr->Hash().m_Hash != hashData.m_Constituents.front()) {
							areItemsConcatenated = false;
						}
						itemItr++;
					} else
						areItemsConcatenated = false;
				}

				// Iterate through the rest of the list regardless to consume the indicated number of constituent hashes.
				hashData.m_Constituents.pop_front();
			}

			// Assumption was wrong: clear the collection out, and start from the beginning again.
			if (!areItemsConcatenated) {
				if (hashData.m_ParseValues.front() > 0) {
					NewPropertyWithValue(clearPhrase, 1);
				}

				itemItr = propValue.begin();
			}

			// For all items determined to need specification, do so.
			while (itemItr != propValue.end()) {
				NewPropertyWithValue(insertionPhrase, *(itemItr++));
			}

			// Value was used for parsing, eject it.
			hashData.m_ParseValues.pop_front();
		}

		/// Writes a collection of preset references where order matters, written to assume it has corresponding data in the hash structure provided.
		/// @param clearPhrase The the phrase used to wipe the contents of the collection, if the referent's is incompatible.
		/// @param insertionPhrase The the phrase used to push an element to the back of the collection.
		/// @param propValue The sequential collection of items in question.
		/// @param hashData A modifiable reference to the HashingData relevant to writing this object, which should correspond to it's preset's.
		template <typename Type>
		void NewPresetReferenceSequence(const std::string& clearPhrase, const std::string& insertionPhrase, const Type& propValue, HashingData& hashData) {
			// Taking the optimistic assumption that the items in this list can simply be pasted after the reference's at the point that they diverge, start an iterator at the beginning.
			bool areItemsConcatenated = true;
			auto itemItr = propValue.begin();

			// Check the assumption and find where the reference's list diverges.
			for (size_t refIndex = 0; refIndex < hashData.m_ParseValues.front(); refIndex++) {
				if (areItemsConcatenated) {
					if (itemItr != propValue.end()) {
						if (RTE::Hash((*itemItr)->GetEntityCharacteristic()) != hashData.m_Constituents.front()) {
							areItemsConcatenated = false;
						}
						itemItr++;
					} else
						areItemsConcatenated = false;
				}

				// Iterate through the rest of the list regardless to consume the indicated number of constituent hashes.
				hashData.m_Constituents.pop_front();
			}

			// Assumption was wrong: clear the collection out, and start from the beginning again.
			if (!areItemsConcatenated) {
				if (hashData.m_ParseValues.front() > 0) {
					NewPropertyWithValue(clearPhrase, 1);
				}

				itemItr = propValue.begin();
			}

			// For all items determined to need specification, do so.
			while (itemItr != propValue.end()) {
				NewPropertyWithValue(insertionPhrase, (*itemItr)->GetEntityCharacteristic());
				itemItr++;
			}

			// Value was used for parsing, eject it.
			hashData.m_ParseValues.pop_front();
		}

		/// Writes a collection of pointers where order matters, written to assume it has corresponding data in the hash structure provided, and respecting the conditional for consideration.
		/// @param clearPhrase The the phrase used to wipe the contents of the collection, if the referent's is incompatible.
		/// @param insertionPhrase The the phrase used to push an element to the back of the collection.
		/// @param propValue The sequential collection of pointers in question.
		/// @param hashData A modifiable reference to the HashingData relevant to writing this object, which should correspond to it's preset's.
		/// @param conditional A conditional lambda to determine if an extant value should be considered at all.
		template <typename Type, typename Lambda>
		void NewPointerSequenceConditional(const std::string& clearPhrase, const std::string& insertionPhrase, const Type& propValue, HashingData& hashData, Lambda&& conditional) {
			// Taking the optimistic assumption that the items in this list can simply be pasted after the reference's at the point that they diverge, start an iterator at the beginning.
			bool areItemsConcatenated = true;
			auto itemItr = propValue.begin();

			// Check the assumption and find where the reference's list diverges.
			for (size_t refIndex = 0; refIndex < hashData.m_ParseValues.front(); refIndex++) {
				if (areItemsConcatenated) {
					if (itemItr != propValue.end()) {
						if (conditional(*itemItr)) {
							if ((*itemItr)->Hash().m_Hash != hashData.m_Constituents.front()) {
								areItemsConcatenated = false;
							}
						}
						itemItr++;
					} else
						areItemsConcatenated = false;
				}

				// Iterate through the rest of the list regardless to consume the indicated number of constituent hashes.
				hashData.m_Constituents.pop_front();
			}

			// Assumption was wrong: clear the collection out, and start from the beginning again.
			if (!areItemsConcatenated) {
				if (hashData.m_ParseValues.front() > 0) {
					NewPropertyWithValue(clearPhrase, 1);
				}

				itemItr = propValue.begin();
			}

			// For all items determined to need specification, do so.
			while (itemItr != propValue.end()) {
				if (conditional(*itemItr)) {
					NewPropertyWithValue(insertionPhrase, **itemItr);
				}
				itemItr++;
			}

			// Value was used for parsing, eject it.
			hashData.m_ParseValues.pop_front();
		}

		/// If the hash data indicates that anything needs to be written, creates a new line and writes the name of the specified property, followed by its set value.
		/// @param propName The name of the property to be written.
		/// @param propValue The value of the property.
		/// @param hashData A modifiable reference to the HashingData relevant to writing this object, which should correspond to it's preset's.
		template <typename Type>
		void NewOptionalEntityPointerProperty(const std::string& propName, const Type& propValue, HashingData& hashData) {
			if (propValue != nullptr) {
				uint64_t thingHash = propValue->Hash().m_Hash;
				// If we have the thing, but the reference didn't, we have to specify it.
				bool isToRespecify = hashData.m_ParseValues.front() == 0;

				if (!isToRespecify) {
					// If both actually had the thing, then we still have to respecify if they're at all different.
					isToRespecify = thingHash != hashData.m_Constituents.front();
					hashData.m_Constituents.pop_front();
				}

				if (isToRespecify) {
					// This property is, in any case, not identical to that of the reference, so write what it is.
					NewProperty(propName);
					*this << propValue;
				}
			} else if (hashData.m_ParseValues.front() != 0) {
				// This property is null, but the reference's property was not, so clear it.
				NewProperty(propName);
				NoObject();
			}

			// Info for parsing has been used, eject it.
			hashData.m_ParseValues.pop_front();
		}

		/// If the hash data indicates that anything needs to be written, creates a new line and writes the name of the specified property, followed by its set value.
		/// @param propName The name of the property to be written.
		/// @param propValue The value of the property.
		/// @param hashData A modifiable reference to the HashingData relevant to writing this object, which should correspond to it's preset's.
		template <typename Type>
		void NewEntityPointerProperty(const std::string& propName, const Type& propValue, HashingData& hashData) {
			// Specify the property if it's different from that of the reference, taking as given that both are non-null.
			if (propValue->Hash().m_Hash != hashData.m_Constituents.front()) {
				NewPropertyWithValue(propName, *propValue);
			}

			hashData.m_Constituents.pop_front();
		}

		/// If the comparison indicates that anything needs to be written, creates a new line and writes the name of the specified property, followed by its set value.
		/// @param propName The name of the property to be written.
		/// @param propValue The value of the property.
		/// @param propReference The value of the property of the reference
		template <typename Type>
		void NewPresetReferenceProperty(const std::string& propName, const Type& propValue, const Type& propReference) {
			// Preset references are straight forward, if they point to different things, write what ours point to.
			if (propValue != propReference) {
				NewProperty(propName);

				if (propValue == nullptr) {
					NoObject();
				} else {
					*this << propValue->GetEntityCharacteristic();
				}
			}
		}

		/// If the comparison indicates that anything needs to be written, creates a new line and writes the name of the specified property, followed by its set value.
		/// @param propName The name of the property to be written.
		/// @param propValue The value of the property.
		/// @param propReference The value of the property of the reference.
		template <typename Type>
		void NewDistinctProperty(const std::string& propName, const Type& propValue, const Type& propReference) {
			// Goes without saying, really.
			if (propValue != propReference) {
				NewPropertyWithValue(propName, propValue);
			}
		}

		/// If the hash data indicates that anything needs to be written, creates a new line and writes the name of the specified property, followed by its set value.
		/// @param propName The name of the property to be written.
		/// @param propValue The value of the property.
		/// @param hashData A modifiable reference to the HashingData relevant to writing this object, which should correspond to it's preset's.
		template <typename Type>
		void NewDistinctHashedProperty(const std::string& propName, const Type& propValue, HashingData& hashData) {
			// Specify the property if it's different from that of the reference.
			if (propValue.Hash().m_Hash != hashData.m_Constituents.front()) {
				NewPropertyWithValue(propName, propValue);
			}
			hashData.m_Constituents.pop_front();
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
