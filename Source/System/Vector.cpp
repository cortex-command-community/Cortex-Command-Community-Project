#include "Vector.h"

#pragma float_control(precise, on)

using namespace RTE;

const std::string Vector::c_ClassName = "Vector";

int Vector::ReadProperty(const std::string_view& propName, Reader& reader) {
	StartPropertyList(return Serializable::ReadProperty(propName, reader));

	MatchProperty("X", { reader >> m_X; });
	MatchProperty("Y", { reader >> m_Y; });

	EndPropertyList;
}

int Vector::Save(Writer& writer) const {
	Serializable::Save(writer);

	writer.NewPropertyWithValue("X", m_X);
	writer.NewPropertyWithValue("Y", m_Y);

	return 0;
}

HashingData Vector::Hash() const {
	HashingData hashData(std::move(Serializable::Hash()));
	uint64_t& hash = hashData.m_Hash;

	hash ^= std::hash<float>{}(m_X) << 0;
	hash ^= std::hash<float>{}(m_Y) << 1;

	return hashData;
}
