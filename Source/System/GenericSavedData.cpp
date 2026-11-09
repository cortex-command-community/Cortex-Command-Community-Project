#include "GenericSavedData.h"

#include "Base64/base64.h"

using namespace RTE;

const std::string GenericSavedData::c_ClassName = "GenericSavedData";
const std::string GenericSavedData::GenericSavedEncodedStrings::c_ClassName = "GenericSavedEncodedStrings";
const std::string GenericSavedData::GenericSavedStrings::c_ClassName = "GenericSavedStrings";
const std::string GenericSavedData::GenericSavedNumbers::c_ClassName = "GenericSavedNumbers";

int GenericSavedData::ReadProperty(const std::string_view& propName, Reader& reader) {
	StartPropertyList(return Serializable::ReadProperty(propName, reader));

	MatchProperty("EncodedStringValues", { reader >> m_SavedEncodedStrings; });
	MatchProperty("StringValues", { reader >> m_SavedStrings; });
	MatchProperty("NumberValues", { reader >> m_SavedNumbers; });

	EndPropertyList;
}

int GenericSavedData::Save(Writer& writer) const {
	Serializable::Save(writer);

	if (!m_SavedEncodedStrings.m_Data.empty())
		writer.NewPropertyWithValue("EncodedStringValues", m_SavedEncodedStrings);
	if (!m_SavedStrings.m_Data.empty())
		writer.NewPropertyWithValue("StringValues", m_SavedStrings);
	if (!m_SavedNumbers.m_Data.empty())
		writer.NewPropertyWithValue("NumberValues", m_SavedNumbers);

	return 0;
}

HashingData GenericSavedData::Hash() const {
	HashingData hashData(Serializable::Hash());

	hashData.Add(m_SavedEncodedStrings);
	hashData.Add(m_SavedStrings);
	hashData.Add(m_SavedNumbers);

	return hashData;
}

template <typename WriterType> int GenericSavedData::WriteTo(WriterType& writer) const {
	Serializable::WriteTo(writer);

	if (!m_SavedEncodedStrings.m_Data.empty())
		writer.NewPropertyWithValue("EncodedStringValues", m_SavedEncodedStrings);
	if (!m_SavedStrings.m_Data.empty())
		writer.NewPropertyWithValue("StringValues", m_SavedStrings);
	if (!m_SavedNumbers.m_Data.empty())
		writer.NewPropertyWithValue("NumberValues", m_SavedNumbers);

	return 0;
}
template int GenericSavedData::WriteTo(IniWriter& writer) const;
template int GenericSavedData::WriteTo(HashWriter& writer) const;

void GenericSavedData::SaveString(const std::string& key, const std::string& value) {
	if (value.length() == 0) {
		m_SavedEncodedStrings.m_Data[key] = value;
		m_SavedStrings.m_Data[key] = value;
		return;
	}

	// Probably a bit anal, but the ini parser removes all leading spaces between the = and value, so check for first character being a space
	// Also we're never encoding the key: if somebody uses a keyname with these characters they frankly deserve the crash
	bool stringRequiresEncoding = value[0] == ' ';
	if (!stringRequiresEncoding) {
		for (int i = 0; i < value.length(); i++) {
			switch (value[i]) {
				case '\r':
				case '\n':
				case '\t':
				case '=':
					stringRequiresEncoding = true;
					break;
				default:
					continue;
			}
		}
	}

	if (stringRequiresEncoding) {
		m_SavedEncodedStrings.m_Data[key] = value;
		m_SavedStrings.m_Data[key] = "";
	} else {
		m_SavedEncodedStrings.m_Data[key] = "";
		m_SavedStrings.m_Data[key] = value;
	}
}

const std::string& GenericSavedData::LoadString(const std::string& key) {
	const std::string* loadString = &m_SavedStrings.m_Data[key];
	if (*loadString == "") {
		loadString = &m_SavedEncodedStrings.m_Data[key];
	}
	return *loadString;
}

int GenericSavedData::GenericSavedEncodedStrings::ReadProperty(const std::string_view& propName, Reader& reader) {
	std::string value = reader.ReadPropValue();
	m_Data[std::string(propName)] = base64_decode(value); // until we get P0919R2.
	return 0;
}

int GenericSavedData::GenericSavedEncodedStrings::Save(Writer& writer) const {
	Serializable::Save(writer);

	for (const auto& [propName, value]: m_Data) {
		// Need to encode as URL, so it avoids = character
		if (!value.empty()) {
			writer.NewPropertyWithValue(propName, base64_encode(value, true));
		}
	}
	return 0;
}

HashingData GenericSavedData::GenericSavedEncodedStrings::Hash() const {
	HashingData hashData(Serializable::Hash());

	hashData.AddUnordered(m_Data);

	return hashData;
}

template <typename WriterType> int GenericSavedData::GenericSavedEncodedStrings::WriteTo(WriterType& writer) const {
	Serializable::WriteTo(writer);

	writer.WriteUnordered(m_Data, [&writer](const auto& entry) {
		// Need to encode as URL, so it avoids = character
		if (!entry.second.empty()) {
			writer.NewPropertyWithValue(PropName(entry.first), base64_encode(entry.second, true));
		}
	});
	return 0;
}
template int GenericSavedData::GenericSavedEncodedStrings::WriteTo(IniWriter& writer) const;
template int GenericSavedData::GenericSavedEncodedStrings::WriteTo(HashWriter& writer) const;

int GenericSavedData::GenericSavedStrings::ReadProperty(const std::string_view& propName, Reader& reader) {
	m_Data[std::string(propName)] = reader.ReadPropValue(); // until we get P0919R2.
	return 0;
}

int GenericSavedData::GenericSavedStrings::Save(Writer& writer) const {
	Serializable::Save(writer);

	for (const auto& [propName, value]: m_Data) {
		if (!value.empty()) {
			writer.NewPropertyWithValue(propName, value);
		}
	}
	return 0;
}

HashingData GenericSavedData::GenericSavedStrings::Hash() const {
	HashingData hashData(Serializable::Hash());

	hashData.AddUnordered(m_Data);

	return hashData;
}

template <typename WriterType> int GenericSavedData::GenericSavedStrings::WriteTo(WriterType& writer) const {
	Serializable::WriteTo(writer);

	writer.WriteUnordered(m_Data, [&writer](const auto& entry) {
		if (!entry.second.empty()) {
			writer.NewPropertyWithValue(PropName(entry.first), entry.second);
		}
	});
	return 0;
}
template int GenericSavedData::GenericSavedStrings::WriteTo(IniWriter& writer) const;
template int GenericSavedData::GenericSavedStrings::WriteTo(HashWriter& writer) const;

int GenericSavedData::GenericSavedNumbers::ReadProperty(const std::string_view& propName, Reader& reader) {
	double value;
	reader >> value;
	m_Data[std::string(propName)] = value; // until we get P0919R2.
	return 0;
}

int GenericSavedData::GenericSavedNumbers::Save(Writer& writer) const {
	Serializable::Save(writer);

	for (const auto& [propName, value]: m_Data) {
		writer.NewPropertyWithValue(propName, value);
	}

	return 0;
}

HashingData GenericSavedData::GenericSavedNumbers::Hash() const {
	HashingData hashData(Serializable::Hash());

	hashData.AddUnordered(m_Data);

	return hashData;
}

template <typename WriterType> int GenericSavedData::GenericSavedNumbers::WriteTo(WriterType& writer) const {
	Serializable::WriteTo(writer);

	writer.WriteUnordered(m_Data, [&writer](const auto& entry) { writer.NewPropertyWithValue(PropName(entry.first), entry.second); });

	return 0;
}
template int GenericSavedData::GenericSavedNumbers::WriteTo(IniWriter& writer) const;
template int GenericSavedData::GenericSavedNumbers::WriteTo(HashWriter& writer) const;
