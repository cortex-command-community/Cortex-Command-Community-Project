#pragma once

namespace RTE {

	/// A class containing properties for controls and skins.
	class GUIProperties {

	public:
		/// Constructor method used to instantiate a GUIProperties object in
		/// system memory.
		/// @param Name Name of section.
		explicit GUIProperties(const std::string& Name);

		/// Constructor method used to instantiate a GUIProperties object in
		/// system memory.
		GUIProperties();

		/// Destructor method used to free a GUIProperties object in system
		/// memory.
		~GUIProperties();

		/// Clears the properties.
		void Clear();

		/// Adds a new variable to the properties
		/// @param Variable Variable, Value
		void AddVariable(const std::string& Variable, const std::string& Value);

		/// Adds a new variable to the properties
		/// @param Variable Variable, Value
		void AddVariable(const std::string& Variable, char* Value);

		/// Adds a new variable to the properties
		/// @param Variable Variable, Value
		void AddVariable(const std::string& Variable, int Value);

		/// Adds a new variable to the properties
		/// @param Variable Variable, Value
		void AddVariable(const std::string& Variable, bool Value);

		/// Changes the value of a property.
		/// @param Variable Variable, Value
		/// @return True if the variable was set. Otherwise false.
		bool SetValue(const std::string& Variable, const std::string& Value);

		/// Changes the value of a property.
		/// @param Variable Variable, Value
		/// @return True if the variable was set. Otherwise false.
		bool SetValue(const std::string& Variable, int Value);

		/// Updates the properties with properties from another instance.
		/// @param Props Pointer to a Properties class, whether to add variables.
		void Update(GUIProperties* Props, bool Add = false);

		/// Gets a string value
		/// @param Variable Variable, String pointer
		bool GetValue(const std::string& Variable, std::string* Value);

		/// Gets a string array of values
		/// @param Variable Variable, String array, max size of array
		/// @return Number of elements read
		int GetValue(const std::string& Variable, std::string* Array, int MaxArraySize);

		/// Gets an integer array of values
		/// @param Variable Variable, Integer array, max size of array
		/// @return Number of elements read
		int GetValue(const std::string& Variable, int* Array, int MaxArraySize);

		/// Gets a single interger
		/// @param Variable Variable, Integer pointer
		bool GetValue(const std::string& Variable, int* Value);

		/// Gets a single unsigned interger
		/// @param Variable Variable, Unsigned Integer pointer
		bool GetValue(const std::string& Variable, unsigned long* Value);

		/// Gets a boolean value
		/// @param Variable Variable, Boolean pointer
		bool GetValue(const std::string& Variable, bool* Value);

		/// Gets the property name
		std::string GetName() const;

		/// Converts the properties to a string
		std::string ToString();

		/// Gets the variable count in the properties
		int GetCount() const;

		/// Gets a variable based on index
		bool GetVariable(int Index, std::string* Name, std::string* Value);

		/// Sets a variable based on index
		bool SetVariable(int Index, const std::string& Name, const std::string& Value);

		/// Sorts the list by variable name
		/// @param Ascending True for ascending, False for descending
		void Sort(bool Ascending);

	private:
		// Variable structure
		typedef struct {
			std::string m_Name;
			std::string m_Value;
		} PropVariable;

		std::string m_Name;

		std::vector<PropVariable*> m_VariableList;
	};
} // namespace RTE
