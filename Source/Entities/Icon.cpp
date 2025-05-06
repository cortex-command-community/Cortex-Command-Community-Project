#include "Icon.h"

using namespace RTE;

ConcreteClassInfo(Icon, Entity, 80);

Icon::Icon() {
	Clear();
}

Icon::Icon(const Icon& reference) {
	if (this != &reference) {
		Clear();
		Create(reference);
	}
}

Icon::~Icon() {
	Destroy(true);
}

void Icon::Clear() {
	m_BitmapFile.Reset();
	m_FrameCount = 0;
	m_BitmapsIndexed.clear();
	m_BitmapsTrueColor.clear();
}

int Icon::Create() {
	if (m_BitmapsIndexed.empty() || m_BitmapsTrueColor.empty()) {
		if (m_BitmapFile.GetDataPath().empty()) {
			m_BitmapFile.SetDataPath("Base.rte/GUIs/DefaultIcon.png");
		}

		m_BitmapFile.GetAsAnimation(m_BitmapsIndexed, m_FrameCount, COLORCONV_REDUCE_TO_256);
		m_BitmapFile.GetAsAnimation(m_BitmapsTrueColor, m_FrameCount, COLORCONV_8_TO_32);
	}
	return 0;
}

int Icon::Create(const Icon& reference) {
	Entity::Create(reference);

	m_BitmapFile = reference.m_BitmapFile;
	m_FrameCount = reference.m_FrameCount;
	m_BitmapsIndexed = reference.m_BitmapsIndexed;
	m_BitmapsTrueColor = reference.m_BitmapsTrueColor;

	return 0;
}

int Icon::ReadProperty(const std::string_view& propName, Reader& reader) {
	StartPropertyList(return Entity::ReadProperty(propName, reader));

	MatchProperty("BitmapFile", { reader >> m_BitmapFile; });
	MatchProperty("FrameCount", { reader >> m_FrameCount; });

	EndPropertyList;
}

int Icon::Save(Writer& writer) const {
	Entity::Save(writer);

	writer.NewPropertyWithValue("BitmapFile", m_BitmapFile);
	writer.NewDistinctProperty("FrameCount", m_FrameCount, 0U);

	return 0;
}


int Icon::Write(Writer& writer, const Entity& entityReference, HashingData& hashData) const {
	Entity::Write(writer, entityReference, hashData);

	const Icon& reference = static_cast<const Icon&>(entityReference);

	writer.NewDistinctHashedProperty("BitmapFile", m_BitmapFile, hashData);
	writer.NewDistinctProperty("FrameCount", m_FrameCount, reference.m_FrameCount);

	return 0;
}

HashingData Icon::Hash() const {
	HashingData hashData(std::move(Entity::Hash()));
	uint64_t& hash = hashData.m_Hash;

	uint64_t fileHash = m_BitmapFile.Hash().m_Hash;
	hashData.m_Constituents.push_back(fileHash);
	hash ^= fileHash << 0;

	hash ^= static_cast<uint64_t>(m_FrameCount) << 1;

	return hashData;
}

void Icon::Destroy(bool notInherited) {
	if (!notInherited) {
		Entity::Destroy();
	}
	Clear();
}
