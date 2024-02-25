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
	m_IsPlayingDynamicMusic = false;
	m_HardcodedSoundContainersInitialized = false;

	m_IntroMusicSoundContainer = nullptr;
	m_MainMenuMusicSoundContainer = nullptr;
	m_ScenarioMenuMusicSoundContainer = nullptr;

	m_InterruptingMusicSoundContainer = nullptr;
	
	m_CurrentSong = nullptr;
	m_CurrentSongSectionType = "Default";
	m_CurrentSongSection = nullptr;

	m_OldSoundContainer = nullptr;
	m_CurrentSoundContainer = nullptr;
	m_NextSoundContainer = nullptr;

	m_MusicTimer.Reset();
	m_MusicTimer.SetRealTimeLimitMS(-1);
	m_MusicPausedTime = 0.0;
	m_ReturnToDynamicMusic = false;
}

bool MusicMan::Initialize() {
	return true;
}

bool MusicMan::InitializeHardcodedSoundContainers() {
	// Have to do it here so these are read in at all..
	if (const SoundContainer* introMusicSoundContainer = dynamic_cast<const SoundContainer*>(g_PresetMan.GetEntityPreset("SoundContainer", "Intro Music"))) {
		m_IntroMusicSoundContainer = dynamic_cast<SoundContainer*>(introMusicSoundContainer->Clone());
	}
	
	if (const SoundContainer* mainMenuMusicSoundContainer = dynamic_cast<const SoundContainer*>(g_PresetMan.GetEntityPreset("SoundContainer", "Main Menu Music"))) {
		m_MainMenuMusicSoundContainer = dynamic_cast<SoundContainer*>(mainMenuMusicSoundContainer->Clone());
	}

	if (const SoundContainer* mainMenuMusicSoundContainer = dynamic_cast<const SoundContainer*>(g_PresetMan.GetEntityPreset("SoundContainer", "Scenario Menu Music"))) {
		m_ScenarioMenuMusicSoundContainer = dynamic_cast<SoundContainer*>(mainMenuMusicSoundContainer->Clone());
	}

	return true;
}

void MusicMan::Destroy() {
	// AudioMan will stop any music playing on its Destroy since they're just SoundContainers, so we don't need to worry
	Clear();
}

void MusicMan::Update() {
	if (!m_HardcodedSoundContainersInitialized) {
		m_HardcodedSoundContainersInitialized = InitializeHardcodedSoundContainers();
	}
	
	if (m_IsPlayingDynamicMusic) {
		if (m_MusicTimer.IsPastRealTimeLimit()) {
			CyclePlayingSoundContainers(false);
		}
	}
}

void MusicMan::ResetMusicState() {
	if (m_InterruptingMusicSoundContainer != nullptr) {
		m_InterruptingMusicSoundContainer->Stop();
	}
	
	if (m_OldSoundContainer != nullptr) {
		m_OldSoundContainer->Stop();
	}

	if (m_CurrentSoundContainer != nullptr) {
		m_CurrentSoundContainer->Stop();
	}

	if (m_NextSoundContainer != nullptr) {
		m_NextSoundContainer->Stop();
	}

	m_IsPlayingDynamicMusic = false;
	m_InterruptingMusicSoundContainer = nullptr;
	
	m_CurrentSong = nullptr;
	m_CurrentSongSectionType = "Default";
	m_CurrentSongSection = nullptr;

	m_OldSoundContainer = nullptr;
	m_CurrentSoundContainer = nullptr;
	m_NextSoundContainer = nullptr;
}


bool MusicMan::PlayDynamicSong(std::string songName, std::string songSectionType, bool playImmediately, bool playTransition){
	g_AudioMan.StopMusic();

	if (const DynamicSong* dynamicSongToPlay = dynamic_cast<const DynamicSong*>(g_PresetMan.GetEntityPreset("DynamicSong", std::move(songName)))) {
		m_CurrentSong = std::unique_ptr<DynamicSong>(dynamic_cast<DynamicSong*>(dynamicSongToPlay->Clone()));
		SetCurrentSongSectionType(std::move(songSectionType));
		SelectNextSongSection();
		SelectNextSoundContainer(playTransition);
		if (!m_CurrentSoundContainer || playImmediately) {
			// If we don't have a current sound container, we're not playing any song, so move the Next container appropriately and start it up
			CyclePlayingSoundContainers();
		}
		m_IsPlayingDynamicMusic = true;
		
		return true;
	}

	return false;
}

bool MusicMan::SetNextDynamicSongSection(std::string newSongSectionType, bool playImmediately, bool playTransition){
	if (newSongSectionType == m_CurrentSongSectionType) {
		return false;
	}
	
	SetCurrentSongSectionType(std::move(newSongSectionType));
	SelectNextSongSection();
	SelectNextSoundContainer(playTransition);
	if (playImmediately) {
		CyclePlayingSoundContainers(true);
	}
	
	return true;
}

bool MusicMan::CyclePlayingSoundContainers(bool fadeOutCurrent) {
	if (m_CurrentSoundContainer && m_CurrentSoundContainer->IsBeingPlayed()) {
		if (fadeOutCurrent) {
			m_CurrentSoundContainer->FadeOut(static_cast<int>(m_NextSoundContainer->GetMusicPreEntryTime()));
		}

		m_OldSoundContainer = m_CurrentSoundContainer;
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

	if (!m_IsPlayingDynamicMusic) {
		return false;
	}

	m_IsPlayingDynamicMusic = false;
	return true;
}

void MusicMan::PlayInterruptingMusic(SoundContainer* soundContainer) {
	if (m_InterruptingMusicSoundContainer != nullptr) {
		m_InterruptingMusicSoundContainer->Stop();
	}
	
	if (m_OldSoundContainer != nullptr) {
		m_OldSoundContainer->SetPaused(true);
	}

	if (m_CurrentSoundContainer != nullptr) {
		m_CurrentSoundContainer->SetPaused(true);
	}

	if (m_NextSoundContainer != nullptr) {
		m_NextSoundContainer->SetPaused(true);
	}
	
	m_InterruptingMusicSoundContainer = std::unique_ptr<SoundContainer>(dynamic_cast<SoundContainer*>(soundContainer->Clone()));
	m_InterruptingMusicSoundContainer->Play();
	if (m_IsPlayingDynamicMusic) {
		m_ReturnToDynamicMusic = true;
		m_IsPlayingDynamicMusic = false;
		m_MusicPausedTime = m_MusicTimer.GetElapsedRealTimeMS();
	}
}

void MusicMan::EndInterruptingMusic() {
	if (m_InterruptingMusicSoundContainer && m_InterruptingMusicSoundContainer->IsBeingPlayed()) {
		m_InterruptingMusicSoundContainer->Stop();
        
        if (m_OldSoundContainer != nullptr) {
        	m_OldSoundContainer->SetPaused(false);
        }

        if (m_CurrentSoundContainer != nullptr) {
        	m_CurrentSoundContainer->SetPaused(false);
        }

        if (m_NextSoundContainer != nullptr) {
        	m_NextSoundContainer->SetPaused(false);
        }
		
		if (m_ReturnToDynamicMusic) {
			m_ReturnToDynamicMusic = false;
			m_IsPlayingDynamicMusic = true;
			double elapsedPausedTime = m_MusicTimer.GetElapsedRealTimeMS() - m_MusicPausedTime;
			m_MusicTimer.SetRealTimeLimitMS(m_MusicTimer.GetRealTimeLimitMS() + elapsedPausedTime);
		}
	}
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
