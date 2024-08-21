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

	m_InterruptingMusicSoundContainer = nullptr;

	m_CurrentSong = nullptr;
	m_NextSongSectionType = "Default";
	m_NextSongSection = nullptr;

	m_PreviousSoundContainer = nullptr;
	m_CurrentSoundContainer = nullptr;
	m_NextSoundContainer = nullptr;

	m_MusicFadeTimer.Reset();
	m_MusicFadeTimer.SetRealTimeLimitMS(0);
	m_PreviousSoundContainerSetToFade = false;
	m_MusicTimer.Reset();
	m_MusicTimer.SetRealTimeLimitMS(0);
	m_MusicPausedTime = 0.0;
	m_ReturnToDynamicMusic = false;
}

bool MusicMan::Initialize() {
	return true;
}

void MusicMan::Destroy() {
	// AudioMan will stop any music playing on its Destroy since they're just SoundContainers, so we don't need to worry
	Clear();
}

void MusicMan::Update() {
	if (m_IsPlayingDynamicMusic) {
		if (m_MusicTimer.IsPastRealTimeLimit()) {
			CyclePlayingSoundContainers(false);
		}
		if (m_PreviousSoundContainerSetToFade && m_MusicFadeTimer.IsPastRealTimeLimit()) {
			const int musicFadeOutTimeMs = 250;
			m_PreviousSoundContainer->FadeOut(musicFadeOutTimeMs);
			m_PreviousSoundContainerSetToFade = false;
		}
	} else {
		if (m_PreviousSoundContainer && m_PreviousSoundContainer->GetAudibleVolume() == 0.0F) {
			m_PreviousSoundContainer = nullptr;
		}
		if (m_CurrentSoundContainer && m_CurrentSoundContainer->GetAudibleVolume() == 0.0F) {
			m_CurrentSoundContainer = nullptr;
		}
	}
}

bool MusicMan::IsMusicPlaying() const {
	bool interruptingMusicSoundContainerPlaying = m_InterruptingMusicSoundContainer != nullptr && m_InterruptingMusicSoundContainer->GetAudibleVolume() > 0.0F;
	bool previousSoundContainerPlaying = m_PreviousSoundContainer != nullptr && m_PreviousSoundContainer->GetAudibleVolume() > 0.0F;
	bool currentSoundContainerPlaying = m_CurrentSoundContainer != nullptr && m_CurrentSoundContainer->GetAudibleVolume() > 0.0F;
	if (interruptingMusicSoundContainerPlaying || previousSoundContainerPlaying || currentSoundContainerPlaying) {
		return true;
	}
	return false;
}

void MusicMan::ResetMusicState() {
	if (m_InterruptingMusicSoundContainer != nullptr) {
		m_InterruptingMusicSoundContainer->Stop();
	}

	if (m_PreviousSoundContainer != nullptr) {
		m_PreviousSoundContainer->Stop();
	}

	if (m_CurrentSoundContainer != nullptr) {
		m_CurrentSoundContainer->Stop();
	}

	if (m_NextSoundContainer != nullptr) {
		m_NextSoundContainer->Stop();
	}

	Clear();
}

bool MusicMan::PlayDynamicSong(const std::string& songName, const std::string& songSectionType, bool playImmediately, bool playTransition, bool smoothFade) {
	if (const DynamicSong* dynamicSongToPlay = dynamic_cast<const DynamicSong*>(g_PresetMan.GetEntityPreset("DynamicSong", songName))) {
		m_CurrentSong = std::unique_ptr<DynamicSong>(dynamic_cast<DynamicSong*>(dynamicSongToPlay->Clone()));
		SetNextSongSectionType(songSectionType);
		SelectNextSongSection();
		SelectNextSoundContainer(playTransition);
		// If this isn't the case, then the MusicTimer's existing setup should make it play properly anyway, even if it's just instant
		if (playImmediately) {
			if (m_IsPlayingDynamicMusic) {
				if (m_PreviousSoundContainer) {
					m_PreviousSoundContainer->Stop();
					m_PreviousSoundContainer = nullptr;
				}
			}
			CyclePlayingSoundContainers(smoothFade);
		}
		m_IsPlayingDynamicMusic = true;

		return true;
	}

	return false;
}

bool MusicMan::SetNextDynamicSongSection(const std::string& newSongSectionType, bool playImmediately, bool playTransition, bool smoothFade) {
	if (!m_IsPlayingDynamicMusic) {
		return false;
	}
	SetNextSongSectionType(newSongSectionType);
	SelectNextSongSection();
	SelectNextSoundContainer(playTransition);
	if (playImmediately) {
		if (m_PreviousSoundContainerSetToFade) {
			m_PreviousSoundContainerSetToFade = false;
			m_PreviousSoundContainer->Stop();
			m_PreviousSoundContainer = nullptr;
		}
		CyclePlayingSoundContainers(smoothFade);
	}
	return true;
}

bool MusicMan::CyclePlayingSoundContainers(bool smoothFade) {
	if (m_CurrentSoundContainer && m_CurrentSoundContainer->IsBeingPlayed()) {
		if (smoothFade) {
			m_CurrentSoundContainer->FadeOut(static_cast<int>(m_NextSoundContainer->GetMusicPreEntryTime()));
		} else {
			m_PreviousSoundContainerSetToFade = true;
			m_MusicFadeTimer.Reset();
			m_MusicFadeTimer.SetRealTimeLimitMS(static_cast<int>(m_NextSoundContainer->GetMusicPreEntryTime()));
		}
		m_PreviousSoundContainer = std::unique_ptr<SoundContainer>(m_CurrentSoundContainer.release());
	}

	// Clone instead of just point to because we might wanna keep this around even if the DynamicSong is gone
	m_CurrentSoundContainer = std::unique_ptr<SoundContainer>(dynamic_cast<SoundContainer*>(m_NextSoundContainer->Clone()));
	SelectNextSoundContainer();
	m_MusicTimer.Reset();
	double timeUntilNextShouldBePlayed = m_CurrentSoundContainer->GetMusicExitTime() - m_NextSoundContainer->GetMusicPreEntryTime();
	m_MusicTimer.SetRealTimeLimitMS(timeUntilNextShouldBePlayed);
	m_CurrentSoundContainer->Play();
	m_CurrentSongSectionType = m_NextSongSectionType;

	return true;
}

bool MusicMan::EndDynamicMusic(bool fadeOutCurrent) {
	if (!m_IsPlayingDynamicMusic) {
		return false;
	}

	m_CurrentSong = nullptr;

	if (m_PreviousSoundContainer && m_PreviousSoundContainer->IsBeingPlayed()) {
		m_PreviousSoundContainer->FadeOut(500);
	}

	if (fadeOutCurrent && m_CurrentSoundContainer && m_CurrentSoundContainer->IsBeingPlayed()) {
		m_CurrentSoundContainer->FadeOut(2000);
	}

	m_MusicTimer.Reset();
	m_MusicTimer.SetRealTimeLimitMS(0);
	m_MusicPausedTime = 0.0;
	m_ReturnToDynamicMusic = false;
	m_IsPlayingDynamicMusic = false;
	return true;
}

void MusicMan::PlayInterruptingMusic(const SoundContainer* soundContainer) {
	if (m_InterruptingMusicSoundContainer != nullptr) {
		m_InterruptingMusicSoundContainer->Stop();
	}

	if (m_PreviousSoundContainer != nullptr) {
		m_PreviousSoundContainer->SetPaused(true);
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

		if (m_PreviousSoundContainer != nullptr) {
			m_PreviousSoundContainer->SetPaused(false);
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
	if (m_NextSongSection && m_NextSongSection->GetSectionType() == m_NextSongSectionType) {
		// Our current song section is already suitable
		return;
	}

	for (DynamicSongSection& dynamicSongSection: m_CurrentSong->GetSongSections()) {
		if (dynamicSongSection.GetSectionType() == m_NextSongSectionType) {
			m_NextSongSection = &dynamicSongSection;
			return;
		}
	}

	m_NextSongSection = &m_CurrentSong->GetDefaultSongSection();
}

void MusicMan::SelectNextSoundContainer(bool playTransition) {
	if (playTransition) {
		m_NextSoundContainer = &m_NextSongSection->SelectTransitionSoundContainer();
	} else {
		m_NextSoundContainer = &m_NextSongSection->SelectSoundContainer();
	}
}
