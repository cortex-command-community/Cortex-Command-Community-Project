#pragma once

#include "Serializable.h"

namespace RTE {

	/// Helper class to save generic data.
	class GenericSavedData : public Serializable {

		/// Helper class to save generic encoded string data, that can safely include newlines and = and other characters INI doesn't like.
		class GenericSavedEncodedStrings : public Serializable {

		public:
			SerializableClassNameGetter;
			SerializableOverrideMethods;

			/// Constructor method used to instantiate a GenericSavedStrings object in system memory and make it ready for use.
			GenericSavedEncodedStrings() = default;

			///	Constructor method used to instantiate a GenericSavedEncodedStrings object to be identical to another, by deep copy, and make it ready for use.
			/// @param reference A reference to the GenericSavedEncodedStrings to deep copy.
			GenericSavedEncodedStrings(const GenericSavedEncodedStrings& reference) = default;

			std::unordered_map<std::string, std::string> m_Data; //!< Stored string data.

		private:
			static const std::string c_ClassName; //!< A string with the friendly formatted type name of this object.
		};

		/// Helper class to save generic string data.
		class GenericSavedStrings : public Serializable {

		public:
			SerializableClassNameGetter;
			SerializableOverrideMethods;

			/// Constructor method used to instantiate a GenericSavedStrings object in system memory and make it ready for use.
			GenericSavedStrings() = default;

			///	Constructor method used to instantiate a GenericSavedStrings object to be identical to another, by deep copy, and make it ready for use.
			/// @param reference A reference to the GenericSavedStrings to deep copy.
			GenericSavedStrings(const GenericSavedStrings& reference) = default;

			std::unordered_map<std::string, std::string> m_Data; //!< Stored string data.

		private:
			static const std::string c_ClassName; //!< A string with the friendly formatted type name of this object.
		};

		/// Helper class to save generic number data.
		class GenericSavedNumbers : public Serializable {

		public:
			SerializableClassNameGetter;
			SerializableOverrideMethods;

			/// Constructor method used to instantiate a GenericSavedNumbers object in system memory and make it ready for use.
			GenericSavedNumbers() = default;

			///	Constructor method used to instantiate a GenericSavedNumbers object to be identical to another, by deep copy, and make it ready for use.
			/// @param reference A reference to the GenericSavedNumbers to deep copy.
			GenericSavedNumbers(const GenericSavedNumbers& reference) = default;

			std::unordered_map<std::string, float> m_Data; //!< Stored number data.

		private:
			static const std::string c_ClassName; //!< A string with the friendly formatted type name of this object.
		};

	public:
		SerializableClassNameGetter;
		SerializableOverrideMethods;

		/// Constructor method used to instantiate a GenericSavedData object in system memory and make it ready for use.
		GenericSavedData() = default;

		///	Constructor method used to instantiate a GenericSavedData object to be identical to another, by deep copy, and make it ready for use.
		/// @param reference A reference to the GenericSavedData to deep copy.
		GenericSavedData(const GenericSavedData& reference) = default;

		void SaveString(const std::string& key, const std::string& value);
		const std::string& LoadString(const std::string& key);

		void SaveNumber(const std::string& key, float value) { m_SavedNumbers.m_Data[key] = value; };
		float LoadNumber(const std::string& key) { return m_SavedNumbers.m_Data[key]; };

		GenericSavedEncodedStrings m_SavedEncodedStrings; //!< Stored encoded string data.
		GenericSavedStrings m_SavedStrings; //!< Stored string data.
		GenericSavedNumbers m_SavedNumbers; //!< Stored number data.

	private:
		static const std::string c_ClassName; //!< A string with the friendly formatted type name of this object.
	};
} // namespace RTE
