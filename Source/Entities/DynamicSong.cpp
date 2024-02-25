#include "DynamicSong.h"

using namespace RTE;

ConcreteClassInfo(DynamicSongSection, Entity, 50);
ConcreteClassInfo(DynamicSong, Entity, 50);

DynamicSongSection::DynamicSongSection() {
	Clear();
}

DynamicSongSection::DynamicSongSection(const DynamicSongSection& reference) {
	Clear();
	Create(reference);
}

DynamicSongSection::~DynamicSongSection() {
	Destroy(true);
}

void DynamicSongSection::Clear() {
	m_TransitionSoundContainers.clear();
	m_LastTransitionSoundContainerIndex = -1;
	m_SoundContainers.clear();
	m_LastSoundContainerIndex = -1;
	m_SectionType = "None";
}

int DynamicSongSection::Create(const DynamicSongSection& reference) {
	Entity::Create(reference);

	for (const SoundContainer& referenceSoundContainer: reference.m_TransitionSoundContainers) {
		SoundContainer soundContainer;
		soundContainer.Create(referenceSoundContainer);
		m_TransitionSoundContainers.push_back(soundContainer);
	}

	m_LastTransitionSoundContainerIndex = reference.m_LastTransitionSoundContainerIndex;

	for (const SoundContainer& referenceSoundContainer: reference.m_SoundContainers) {
		SoundContainer soundContainer;
		soundContainer.Create(referenceSoundContainer);
		m_SoundContainers.push_back(soundContainer);
	}

	m_LastSoundContainerIndex = reference.m_LastSoundContainerIndex;
	m_SectionType = reference.m_SectionType;

	return 0;
}

int DynamicSongSection::ReadProperty(const std::string_view& propName, Reader& reader) {
	StartPropertyList(return Entity::ReadProperty(propName, reader));

	MatchProperty("AddTransitionSoundContainer", {
		SoundContainer soundContainerToAdd;
		reader >> soundContainerToAdd;
		m_TransitionSoundContainers.push_back(soundContainerToAdd);
	});
	MatchProperty("AddSoundContainer", {
		SoundContainer soundContainerToAdd;
		reader >> soundContainerToAdd;
		m_SoundContainers.push_back(soundContainerToAdd);
	});
	MatchProperty("SectionType", { reader >> m_SectionType; });

	EndPropertyList;
}

int DynamicSongSection::Save(Writer& writer) const {
	Entity::Save(writer);

	for (const SoundContainer& soundContainer: m_TransitionSoundContainers) {
		writer.NewProperty("AddTransitionSoundContainer");
		writer.ObjectStart("SoundContainer");
		writer << soundContainer;
		writer.ObjectEnd();
	}
	writer.NewProperty("LastTransitionSoundContainerIndex");
	writer << m_LastTransitionSoundContainerIndex;
	for (const SoundContainer& soundContainer: m_SoundContainers) {
		writer.NewProperty("AddSoundContainer");
		writer.ObjectStart("SoundContainer");
		writer << soundContainer;
		writer.ObjectEnd();
	}
	writer.NewProperty("LastSoundContainerIndex");
	writer << m_LastSoundContainerIndex;
	writer.NewProperty("SectionType");
	writer << m_SectionType;

	return 0;
}

SoundContainer& DynamicSongSection::SelectTransitionSoundContainer() {
	if (m_TransitionSoundContainers.empty()) {
		return SelectSoundContainer();
	}
	if (m_TransitionSoundContainers.size() == 1) {
		return m_TransitionSoundContainers[0];
	}

	std::vector<unsigned int> validIndices;
	for (unsigned int i = 0; i < m_TransitionSoundContainers.size(); i++) {
		if (i != m_LastTransitionSoundContainerIndex) {
			validIndices.push_back(i);
		}
	}

	unsigned int randomIndex = validIndices[RandomNum(0, static_cast<int>(validIndices.size()) - 1)];
	m_LastTransitionSoundContainerIndex = randomIndex;
	return m_TransitionSoundContainers[randomIndex];
}

SoundContainer& DynamicSongSection::SelectSoundContainer() {
	RTEAssert(!m_SoundContainers.empty(), "Tried to get a SoundContainer from a DynamicSongSection with none to choose from!");

	if (m_SoundContainers.size() == 1) {
		return m_SoundContainers[0];
	}

	std::vector<unsigned int> validIndices;
	for (unsigned int i = 0; i < m_SoundContainers.size(); i++) {
		if (i != m_LastSoundContainerIndex) {
			validIndices.push_back(i);
		}
	}

	unsigned int randomIndex = validIndices[RandomNum(0, static_cast<int>(validIndices.size()) - 1)];
	m_LastSoundContainerIndex = randomIndex;
	return m_SoundContainers[randomIndex];
}

DynamicSong::DynamicSong() {
	Clear();
}

DynamicSong::DynamicSong(const DynamicSong& reference) {
	Clear();
	Create(reference);
}

DynamicSong::~DynamicSong() {
	Destroy(true);
}

void DynamicSong::Clear() {
	m_DefaultSongSection.Destroy();
	m_SongSections.clear();
}

int DynamicSong::Create(const DynamicSong& reference) {
	Entity::Create(reference);

	m_DefaultSongSection.Create(reference.m_DefaultSongSection);

	for (const DynamicSongSection& referenceDynamicSongSection: reference.m_SongSections) {
		DynamicSongSection dynamicSongSection;
		dynamicSongSection.Create(referenceDynamicSongSection);
		m_SongSections.push_back(dynamicSongSection);
	}

	return 0;
}

int DynamicSong::ReadProperty(const std::string_view& propName, Reader& reader) {
	StartPropertyList(return Entity::ReadProperty(propName, reader));

	MatchProperty("DefaultSongSection", {
		DynamicSongSection songSection;
		reader >> songSection;
		SetDefaultSongSection(songSection);
	});
	MatchProperty("AddSongSection", {
		DynamicSongSection songSectionToAdd;
		reader >> songSectionToAdd;
		for (DynamicSongSection& songSection: GetSongSections()) {
			if (songSection.GetSectionType() == songSectionToAdd.GetSectionType()) {
				RTEAssert(false, "Tried to add a SongSection with SectionType " + songSection.GetSectionType() + ", which the DynamicSong already had in another SongSection!");
				break;
			}
		}
		m_SongSections.push_back(songSectionToAdd);
	});

	EndPropertyList;
}

int DynamicSong::Save(Writer& writer) const {
	Entity::Save(writer);

	writer.NewProperty("DefaultSongSection");
	writer.ObjectStart("DynamicSongSection");
	writer << m_DefaultSongSection;
	writer.ObjectEnd();

	for (const DynamicSongSection& dynamicSongSection: m_SongSections) {
		writer.NewProperty("AddSongSection");
		writer.ObjectStart("SoundContainer");
		writer << dynamicSongSection;
		writer.ObjectEnd();
	}

	return 0;
}
