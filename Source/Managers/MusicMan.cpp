#include "MusicMan.h"

#include "NetworkClient.h"
#include "AudioMan.h"
#include "PresetMan.h"

using namespace RTE;

MusicMan::MusicMan() {
	Clear();
}

MusicMan::~MusicMan() {
	Destroy();
}

void MusicMan::Clear() {
	m_IsSetToNotPlayMusic = true;
	
	m_CurrentSong = nullptr;
	m_CurrentSongSectionType = "Default";
	m_CurrentSongSection = nullptr;

	m_CurrentSoundContainer = nullptr;
	m_NextSoundContainer = nullptr;

	m_MusicTimer.Reset();
	m_MusicTimer.SetRealTimeLimitMS(-1);
}

bool MusicMan::Initialize() {
	return true;
}

void MusicMan::Destroy() {
	// AudioMan will stop any music playing on its Destroy since they're just SoundContainers, so we don't need to worry
	Clear();
}

void MusicMan::Update() {
	if (!m_IsSetToNotPlayMusic) {
		if (m_MusicTimer.IsPastRealTimeLimit()) {
			CyclePlayingSoundContainers(false);
		}
	}
}

#pragma region Music Handling

bool MusicMan::PlayDynamicSong(std::string songName, std::string songSectionType, bool playImmediately){
	g_AudioMan.StopMusic();
	if (const DynamicSong* dynamicSongToPlay = dynamic_cast<const DynamicSong*>(g_PresetMan.GetEntityPreset("DynamicSong", std::move(songName)))) {
		m_IsSetToNotPlayMusic = false;
		m_CurrentSong = dynamic_cast<DynamicSong*>(dynamicSongToPlay->Clone());
		SetCurrentSongSectionType(std::move(songSectionType));
		SelectNextSongSection();
		SelectNextSoundContainer(true);
		if (!m_CurrentSoundContainer || playImmediately) {
			// If we don't have a current sound container, we're not playing any song, so move the Next container appropriately and start it up
			CyclePlayingSoundContainers();
		}
		return true;
	}
	return false;
}

bool MusicMan::SetNextDynamicSongSection(std::string newSongSectionType, bool playImmediately){
	if (newSongSectionType == m_CurrentSongSectionType) {
		return false;
	}
	
	SetCurrentSongSectionType(std::move(newSongSectionType));
	SelectNextSongSection();
	SelectNextSoundContainer(true);
	if (playImmediately) {
		CyclePlayingSoundContainers(true);
	}
	
	return true;
}

bool MusicMan::CyclePlayingSoundContainers(bool fadeOutCurrent) {
	if (m_CurrentSoundContainer && m_CurrentSoundContainer->IsBeingPlayed() && fadeOutCurrent) {
		m_CurrentSoundContainer->FadeOut(static_cast<int>(m_NextSoundContainer->GetMusicPreEntryTime()));
	}
	m_CurrentSoundContainer = m_NextSoundContainer;
	SelectNextSoundContainer();
	m_MusicTimer.Reset();
	double timeUntilNextShouldBePlayed = m_CurrentSoundContainer->GetLength(SoundContainer::LengthOfSoundType::NextPlayed) - m_CurrentSoundContainer->GetMusicPostExitTime() - m_NextSoundContainer->GetMusicPreEntryTime();
	m_MusicTimer.SetRealTimeLimitMS(timeUntilNextShouldBePlayed);
	m_CurrentSoundContainer->Play();

	return true;
}

bool MusicMan::EndMusic(bool fadeOutCurrent) {
	if (fadeOutCurrent && m_CurrentSoundContainer && m_CurrentSoundContainer->IsBeingPlayed()) {
		m_CurrentSoundContainer->FadeOut(2000);
	}
	if (m_IsSetToNotPlayMusic) return false;
	m_IsSetToNotPlayMusic = true;
	return true;
}

void MusicMan::SelectNextSongSection() {
	if (!m_CurrentSongSection || m_CurrentSongSection->GetSectionType() != m_CurrentSongSectionType) {
		for (DynamicSongSection& dynamicSongSection: m_CurrentSong->GetSongSections()) {
			if (dynamicSongSection.GetSectionType() == m_CurrentSongSectionType) {
				m_CurrentSongSection = &dynamicSongSection;
				break;
			}
		}
	}
}

void MusicMan::SelectNextSoundContainer(bool playTransition) {
	if (playTransition) {
		m_NextSoundContainer = &m_CurrentSongSection->SelectTransitionSoundContainer();
	} else {
		m_NextSoundContainer = &m_CurrentSongSection->SelectSoundContainer();
	}
}

#pragma endregion