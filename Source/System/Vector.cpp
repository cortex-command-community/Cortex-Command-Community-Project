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

template <typename WriterType> int Vector::WriteTo(WriterType& writer) const {
	Serializable::WriteTo(writer);

	writer.NewPropertyWithValue("X", m_X);
	writer.NewPropertyWithValue("Y", m_Y);

	return 0;
}
template int Vector::WriteTo(IniWriter& writer) const;
template int Vector::WriteTo(HashWriter& writer) const;
