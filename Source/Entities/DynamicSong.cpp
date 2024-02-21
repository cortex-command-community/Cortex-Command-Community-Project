#include "DynamicSong.h"

using namespace RTE;

ConcreteClassInfo(DynamicSongSectionType, Entity, 50);
ConcreteClassInfo(DynamicSongSection, Entity, 50);
ConcreteClassInfo(DynamicSong, Entity, 50);

#pragma region DynamicSongSectionType
DynamicSongSectionType::DynamicSongSectionType() {
	Clear();
}

DynamicSongSectionType::DynamicSongSectionType(const DynamicSongSectionType& reference) {
	Clear();
	Create(reference);
}

DynamicSongSectionType::~DynamicSongSectionType() {
	Destroy(true);
}

void DynamicSongSectionType::Clear() {

}

int DynamicSongSectionType::Create(const DynamicSongSectionType& reference) {
	Entity::Create(reference);

	return 0;
}

#pragma endregion 

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

}

int DynamicSongSection::Create(const DynamicSongSection& reference) {
	Entity::Create(reference);

	return 0;
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

}

int DynamicSong::Create(const DynamicSong& reference) {
	Entity::Create(reference);

	return 0;
}

#pragma endregion 