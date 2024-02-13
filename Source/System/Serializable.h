#pragma once

#include "Reader.h"
#include "Writer.h"

#include <functional>
#include <string>
#include <unordered_map>
#include <filesystem>

namespace RTE {

	/// This base class specifies common creation/destruction patterns associated with reading and writing member data from disk.
	/// Is only intended to be inherited from in one level.
	class Serializable {

	public:
#pragma region Global Macro Definitions
/// Convenience macro to cut down on duplicate ReadProperty and Save methods in classes that extend Serializable.
#define SerializableOverrideMethods \
	int ReadProperty(const std::string_view& propName, Reader& reader) override; \
	int Save(Writer& writer) const override;

/// Convenience macro to cut down on duplicate GetClassName methods in non-poolable classes that extend Serializable.
#define SerializableClassNameGetter \
	const std::string& GetClassName() const override { return c_ClassName; }

#define StartPropertyList(failureCase) \
	static std::unordered_map<std::string, int> sm_Properties; \
	static bool sm_Initialized = false; \
	int jmpAddress = sm_Initialized ? -1 : -2; \
	if (sm_Initialized) { \
		auto itr = sm_Properties.find(std::string(propName)); \
		if (itr == sm_Properties.end()) { \
			failureCase; \
		} else { \
			jmpAddress = itr->second; \
		} \
	} \
	switch (jmpAddress) { \
		case -1: \
			break; \
		case -2:

#define MatchForwards(propertyName) \
	sm_Properties[propertyName] = __COUNTER__ + 1; \
	case __COUNTER__:

#define MatchProperty(propertyName, matchedFunction) \
	sm_Properties[propertyName] = __COUNTER__ + 1; \
	case __COUNTER__: \
		if (sm_Initialized) { \
			{matchedFunction}; \
			break; \
		};

#define EndPropertyList \
	} \
	if (!sm_Initialized) { \
		sm_Initialized = true; \
		/* This was the initialization pass, now properly read the value by calling ourselves again */ \
		return ReadProperty(propName, reader); \
	} \
	return 0;

#pragma endregion

#pragma region Creation
		/// Constructor method used to instantiate a Serializable object in system memory. Create() should be called before using the object.
		Serializable() = default;

		/// Makes the Serializable object ready for use, usually after all necessary properties have been set with Create(Reader).
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		virtual int Create() { return 0; }

		/// Makes the Serializable object ready for use.
		/// @param reader A Reader that the Serializable will create itself from.
		/// @param checkType Whether there is a class name in the stream to check against to make sure the correct type is being read from the stream.
		/// @param doCreate Whether to do any additional initialization of the object after reading in all the properties from the Reader. This is done by calling Create().
		/// @param skipStartingObjec Skips notifying the Reader that a new object is being read in order to associate any subsequent reads with the currently read object. Special edge case for DataModules that use ScanFolderContents.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int CreateSerializable(Reader& reader, bool checkType, bool doCreate, bool skipStartingObject);

		/// Makes the Serializable object ready for use.
		/// @param reader A Reader that the Serializable will create itself from.
		/// @param checkType Whether there is a class name in the stream to check against to make sure the correct type is being read from the stream.
		/// @param doCreate Whether to do any additional initialization of the object after reading in all the properties from the Reader. This is done by calling Create().
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		virtual int Create(Reader& reader, bool checkType = true, bool doCreate = true) { return CreateSerializable(reader, checkType, doCreate, false); }
#pragma endregion

#pragma region Destruction
		/// Destructor method used to clean up a Serializable object before deletion from system memory.
		virtual ~Serializable() = default;

		/// Resets the entire Serializable, including its inherited members, to their default settings or values.
		virtual void Reset() { Clear(); }
#pragma endregion

#pragma region INI Handling
		/// Reads a property value from a Reader stream.
		/// If the name isn't recognized by this class, then ReadProperty of the parent class is called.
		/// If the property isn't recognized by any of the base classes, false is returned, and the Reader's position is untouched.
		/// @param propName The name of the property to be read.
		/// @param reader A Reader lined up to the value of the property to be read.
		/// @return
		/// An error return value signaling whether the property was successfully read or not.
		/// 0 means it was read successfully, and any nonzero indicates that a property of that name could not be found in this or base classes.
		virtual int ReadProperty(const std::string_view& propName, Reader& reader);

		/// Saves the complete state of this Serializable to an output stream for later recreation with Create(istream &stream).
		/// @param writer A Writer that the Serializable will save itself to.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		virtual int Save(Writer& writer) const {
			writer.ObjectStart(GetClassName());
			return 0;
		}

		/// Replaces backslashes with forward slashes in file paths to eliminate issues with cross-platform compatibility or invalid escape sequences.
		/// @param pathToCorrect Reference to the file path string to correct slashes in.
		// TODO: Add a warning log entry if backslashes are found in a data path. Perhaps overwrite them in the ini file itself.
		std::string CorrectBackslashesInPath(const std::string& pathToCorrect) const { return std::filesystem::path(pathToCorrect).generic_string(); }
#pragma endregion

#pragma region Logging
		/// Sets the file and line that are currently being read. Formatted to be used for logging warnings and errors.
		/// Because we're often used as a parent for basic types (i.e, Vector, Matrix, Color), where we don't want to spend any time doing string construction we don't actually store this data here.
		/// This just acts as an abstract base for child classes to implement.
		/// @param newPosition A string containing the currently read file path and the line being read.
		virtual void SetFormattedReaderPosition(const std::string& newPosition) {}
#pragma endregion

#pragma region Operator Overloads
		/// A Reader extraction operator for filling a Serializable from a Reader.
		/// @param reader A Reader reference as the left hand side operand.
		/// @param operand An Serializable reference as the right hand side operand.
		/// @return A Reader reference for further use in an expression.
		friend Reader& operator>>(Reader& reader, Serializable& operand);

		/// A Reader extraction operator for filling an Serializable from a Reader.
		/// @param reader A Reader reference as the left hand side operand.
		/// @param operand An Serializable pointer as the right hand side operand.
		/// @return A Reader reference for further use in an expression.
		friend Reader& operator>>(Reader& reader, Serializable* operand);

		/// A Writer insertion operator for sending a Serializable to a Writer.
		/// @param writer A Writer reference as the left hand side operand.
		/// @param operand A Serializable reference as the right hand side operand.
		/// @return A Writer reference for further use in an expression.
		friend Writer& operator<<(Writer& writer, const Serializable& operand);

		/// A Writer insertion operator for sending a Serializable to a Writer.
		/// @param writer A Writer reference as the left hand side operand.
		/// @param operand A Serializable pointer as the right hand side operand.
		/// @return A Writer reference for further use in an expression.
		friend Writer& operator<<(Writer& writer, const Serializable* operand);
#pragma endregion

#pragma region Class Info
		/// Gets the class name of this Serializable.
		/// @return A string with the friendly-formatted type name of this Serializable.
		virtual const std::string& GetClassName() const = 0;
#pragma endregion

	private:
		/// Clears all the member variables of this Object, effectively resetting the members of this abstraction level only.
		void Clear() {}
	};
} // namespace RTE
