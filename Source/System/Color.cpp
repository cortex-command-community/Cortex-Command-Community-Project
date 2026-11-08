#include "Color.h"
#include "allegro/color.h"

using namespace RTE;

const std::string Color::c_ClassName = "Color";

int Color::Create() {
	if (Serializable::Create()) {
		return -1;
	}
	RecalculateIndex();
	return 0;
}

int Color::Create(int inputR, int inputG, int inputB) {
	SetR(inputR);
	SetG(inputG);
	SetB(inputB);
	RecalculateIndex();
	return 0;
}

int Color::ReadProperty(const std::string_view& propName, Reader& reader) {
	StartPropertyList(return Serializable::ReadProperty(propName, reader));

	MatchProperty("Index", { SetRGBWithIndex(std::stoi(reader.ReadPropValue())); });
	MatchProperty("R", { SetR(std::stoi(reader.ReadPropValue())); });
	MatchProperty("G", { SetG(std::stoi(reader.ReadPropValue())); });
	MatchProperty("B", { SetB(std::stoi(reader.ReadPropValue())); });

	EndPropertyList;
}

int Color::Save(Writer& writer) const {
	Serializable::Save(writer);

	// A Color can be read over one that's already set, such as one copied from a preset, so a channel left out wouldn't read back as 0.
	writer.NewPropertyWithValue("R", m_R);
	writer.NewPropertyWithValue("G", m_G);
	writer.NewPropertyWithValue("B", m_B);

	return 0;
}

HashingData Color::Hash() const {
	HashingData hashData(Serializable::Hash());
	uint64_t& hash = hashData.m_Hash;

	AddToHash(hash, m_R);
	AddToHash(hash, m_G);
	AddToHash(hash, m_B);

	return hashData;
}

void Color::SetRGBWithIndex(int index) {
	m_Index = std::clamp(index, 0, 255);

	RGB rgbColor;
	get_color(m_Index, &rgbColor);

	// Multiply by 4 because the Allegro RGB struct elements are in range 0-63, and proper RGB needs 0-255.
	m_R = rgbColor.r;
	m_G = rgbColor.g;
	m_B = rgbColor.b;
}

int Color::RecalculateIndex() {
	return m_Index = makecol8(m_R, m_G, m_B);
}
