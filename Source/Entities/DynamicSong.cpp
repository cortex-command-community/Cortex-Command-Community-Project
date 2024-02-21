#include "DynamicSong.h"

using namespace RTE;

ConcreteClassInfo(DynamicSongSection, Entity, 50);
ConcreteClassInfo(DynamicSong, Entity, 50);

#pragma region DynamicSongSection
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
	m_SoundContainers.clear();
	m_SectionType = "None";
}

int DynamicSongSection::Create(const DynamicSongSection& reference) {
	Entity::Create(reference);

	for (const SoundContainer& referenceSoundContainer: reference.m_TransitionSoundContainers) {
		SoundContainer soundContainer;
		soundContainer.Create(referenceSoundContainer);
		m_TransitionSoundContainers.push_back(soundContainer);
	}
	for (const SoundContainer& referenceSoundContainer: reference.m_SoundContainers) {
		SoundContainer soundContainer;
		soundContainer.Create(referenceSoundContainer);
		m_SoundContainers.push_back(soundContainer);
	}
	m_SectionType = reference.m_SectionType;
	
	return 0;
}

int DynamicSongSection::ReadProperty(const std::string_view& propName, Reader& reader) {
	StartPropertyList(return Entity::ReadProperty(propName, reader));

	MatchProperty("AddTransitionSoundContainer", {
		SoundContainer soundContainerToAdd;
		reader >> soundContainerToAdd;
		AddTransitionSoundContainer(soundContainerToAdd);
	});
	MatchProperty("AddSoundContainer", {
		SoundContainer soundContainerToAdd;
		reader >> soundContainerToAdd;
		AddSoundContainer(soundContainerToAdd);
	});
	MatchProperty("SectionType", { reader >> m_SectionType; });

	EndPropertyList;
}

#pragma endregion 

#pragma region DynamicSong
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
		AddSongSection(songSectionToAdd);
	});

	EndPropertyList;
}

#pragma endregion 