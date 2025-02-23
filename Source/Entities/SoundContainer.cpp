#include "SoundContainer.h"

#include "SoundSet.h"
#include "SettingsMan.h"

using namespace RTE;

ConcreteClassInfo(SoundContainer, Entity, 50);

const std::unordered_map<std::string, SoundContainer::SoundOverlapMode> SoundContainer::c_SoundOverlapModeMap = {
    {"Overlap", SoundContainer::SoundOverlapMode::OVERLAP},
    {"Restart", SoundContainer::SoundOverlapMode::RESTART},
    {"Ignore Play", SoundContainer::SoundOverlapMode::IGNORE_PLAY}};

const std::unordered_map<std::string, SoundContainer::BusRouting> SoundContainer::c_BusRoutingMap = {
    {"SFX", SoundContainer::BusRouting::SFX},
    {"UI", SoundContainer::BusRouting::UI},
    {"Music", SoundContainer::BusRouting::MUSIC}};

SoundContainer::SoundContainer() {
	Clear();
}

SoundContainer::SoundContainer(const SoundContainer& reference) {
	Clear();
	Create(reference);
}

SoundContainer::~SoundContainer() {
	Destroy(true);
}

void SoundContainer::Clear() {
	m_TopLevelSoundSet = std::make_shared<SoundSet>();
	m_TopLevelSoundSet->Destroy();

	m_PlayingChannels.clear();
	m_SoundOverlapMode = SoundOverlapMode::OVERLAP;

	m_BusRouting = BusRouting::SFX;
	m_Immobile = false;
	m_AttenuationStartDistance = c_DefaultAttenuationStartDistance;
	m_CustomPanValue = 0.0f;
	m_PanningStrengthMultiplier = 1.0F;
	m_Loops = 0;
	m_SoundPropertiesUpToDate = false;

	m_Priority = AudioMan::PRIORITY_NORMAL;
	m_AffectedByGlobalPitch = true;

	m_Pos = Vector();
	m_Volume = 1.0F;
	m_Pitch = 1.0F;
	m_PitchVariation = 0;

	m_WasFadedOut = false;
	m_Paused = false;
	m_MusicPreEntryTime = 0.0F;
	m_MusicExitTime = 0.0F;
}

int SoundContainer::Create(const SoundContainer& reference) {
	Entity::Create(reference);

	m_TopLevelSoundSet->Create(*reference.m_TopLevelSoundSet);

	m_PlayingChannels.clear();
	m_SoundOverlapMode = reference.m_SoundOverlapMode;

	m_BusRouting = reference.m_BusRouting;
	m_Immobile = reference.m_Immobile;
	m_AttenuationStartDistance = reference.m_AttenuationStartDistance;
	m_CustomPanValue = reference.m_CustomPanValue;
	m_PanningStrengthMultiplier = reference.m_PanningStrengthMultiplier;
	m_Loops = reference.m_Loops;

	m_Priority = reference.m_Priority;
	m_AffectedByGlobalPitch = reference.m_AffectedByGlobalPitch;

	m_Pos = reference.m_Pos;
	m_Volume = reference.m_Volume;
	m_Pitch = reference.m_Pitch;
	m_PitchVariation = reference.m_PitchVariation;

	m_WasFadedOut = reference.m_WasFadedOut;
	m_Paused = reference.m_Paused;
	m_MusicPreEntryTime = reference.m_MusicPreEntryTime;
	m_MusicExitTime = reference.m_MusicExitTime;

	return 0;
}

int SoundContainer::Create(const std::string& soundFilePath, bool immobile, bool affectedByGlobalPitch, BusRouting busRouting) {
	m_TopLevelSoundSet->AddSound(soundFilePath, true);
	SetImmobile(immobile);
	SetAffectedByGlobalPitch(affectedByGlobalPitch);
	SetBusRouting(busRouting);
	return 0;
}

int SoundContainer::ReadProperty(const std::string_view& propName, Reader& reader) {
	StartPropertyList(return Entity::ReadProperty(propName, reader));

	MatchProperty("TopLevelSoundSet", { reader >> *m_TopLevelSoundSet; });
	MatchProperty("AddSound", { m_TopLevelSoundSet->AddSoundData(SoundSet::ReadAndGetSoundData(reader)); });
	MatchProperty("AddSoundSet", {
		SoundSet soundSetToAdd;
		reader >> soundSetToAdd;
		m_TopLevelSoundSet->AddSoundSet(soundSetToAdd);
	});
	MatchForwards("SoundSelectionCycleMode") MatchProperty("CycleMode", { m_TopLevelSoundSet->SetSoundSelectionCycleMode(SoundSet::ReadSoundSelectionCycleMode(reader)); });
	MatchProperty("SoundOverlapMode", {
		std::string soundOverlapModeString = reader.ReadPropValue();
		if (c_SoundOverlapModeMap.find(soundOverlapModeString) != c_SoundOverlapModeMap.end()) {
			m_SoundOverlapMode = c_SoundOverlapModeMap.find(soundOverlapModeString)->second;
		} else {
			try {
				m_SoundOverlapMode = static_cast<SoundOverlapMode>(std::stoi(soundOverlapModeString));
			} catch (const std::exception&) {
				reader.ReportError("Cycle mode " + soundOverlapModeString + " is invalid.");
			}
		}
	});
	MatchProperty("BusRouting", {
		std::string busRoutingString = reader.ReadPropValue();
		if (c_BusRoutingMap.find(busRoutingString) != c_BusRoutingMap.end()) {
			m_BusRouting = c_BusRoutingMap.find(busRoutingString)->second;
		} else {
			try {
				m_BusRouting = static_cast<BusRouting>(std::stoi(busRoutingString));
			} catch (const std::exception&) {
				reader.ReportError("Tried to route to non-existent sound bus " + busRoutingString);
			}
		}
	});
	MatchProperty("Immobile", { reader >> m_Immobile; });
	MatchProperty("AttenuationStartDistance", { reader >> m_AttenuationStartDistance; });
	MatchProperty("CustomPanValue", {
		reader >> m_CustomPanValue;
		if (m_CustomPanValue < -1.0f || m_CustomPanValue > 1.0f) {
			reader.ReportError("SoundContainer CustomPanValue must be between -1 and 1.");
		}
	});
	MatchProperty("PanningStrengthMultiplier", { reader >> m_PanningStrengthMultiplier; });
	MatchProperty("LoopSetting", { reader >> m_Loops; });
	MatchProperty("Priority", {
		reader >> m_Priority;
		if (m_Priority < 0 || m_Priority > 256) {
			reader.ReportError("SoundContainer priority must be between 256 (lowest priority) and 0 (highest priority).");
		}
	});
	MatchProperty("AffectedByGlobalPitch", { reader >> m_AffectedByGlobalPitch; });
	MatchProperty("Position", { reader >> m_Pos; });
	MatchProperty("Volume", { reader >> m_Volume; });
	MatchProperty("Pitch", { reader >> m_Pitch; });
	MatchProperty("PitchVariation", { reader >> m_PitchVariation; });

	MatchProperty("MusicPreEntryTime", { reader >> m_MusicPreEntryTime; });
	MatchProperty("MusicExitTime", { reader >> m_MusicExitTime; });

	EndPropertyList;
}

int SoundContainer::Save(Writer& writer) const {
	Entity::Save(writer);

	// Due to writer limitations, the top level SoundSet has to be explicitly written out, even though SoundContainer standard behaviour is to hide it in INI and just have properties be part of the SoundContainer.
	writer.NewPropertyWithValue("TopLevelSoundSet", *m_TopLevelSoundSet);

	writer.NewProperty("SoundSelectionCycleMode");
	SoundSet::SaveSoundSelectionCycleMode(writer, m_TopLevelSoundSet->GetSoundSelectionCycleMode());

	writer.NewProperty("SoundOverlapMode");
	auto overlapModeMapEntry = std::find_if(c_SoundOverlapModeMap.begin(), c_SoundOverlapModeMap.end(), [&soundOverlapMode = m_SoundOverlapMode](auto element) { return element.second == soundOverlapMode; });
	if (overlapModeMapEntry != c_SoundOverlapModeMap.end()) {
		writer << overlapModeMapEntry->first;
	} else {
		RTEAbort("Tried to write invalid SoundOverlapMode when saving SoundContainer.");
	}

	writer.NewPropertyWithValue("BusRouting", m_BusRouting);
	writer.NewPropertyWithValue("Immobile", m_Immobile);
	writer.NewPropertyWithValue("AttenuationStartDistance", m_AttenuationStartDistance);
	writer.NewPropertyWithValue("CustomPanValue", m_CustomPanValue);
	writer.NewPropertyWithValue("PanningStrengthMultiplier", m_PanningStrengthMultiplier);
	writer.NewPropertyWithValue("LoopSetting", m_Loops);

	writer.NewPropertyWithValue("Priority", m_Priority);
	writer.NewPropertyWithValue("AffectedByGlobalPitch", m_AffectedByGlobalPitch);

	writer.NewPropertyWithValue("Position", m_Pos);
	writer.NewPropertyWithValue("Volume", m_Volume);
	writer.NewPropertyWithValue("Pitch", m_Pitch);
	writer.NewPropertyWithValue("PitchVariation", m_PitchVariation);

	writer.NewPropertyWithValue("WasFadedOut", m_WasFadedOut);
	writer.NewPropertyWithValue("Paused", m_Paused);
	writer.NewPropertyWithValue("MusicPreEntryTime", m_MusicPreEntryTime);
	writer.NewPropertyWithValue("MusicExitTime", m_MusicExitTime);

	return 0;
}

size_t SoundContainer::Write(Writer& writer, const Entity& entityReference, const HashingData& hashData) const {
	size_t constituentsConsumed = Entity::Write(writer, entityReference, hashData);

	const SoundContainer& reference = static_cast<const SoundContainer&>(entityReference);

	// Due to writer limitations, the top level SoundSet has to be explicitly written out, even though SoundContainer standard behaviour is to hide it in INI and just have properties be part of the SoundContainer.
	if (m_TopLevelSoundSet->Hash().m_Hash != hashData.m_Constituents.at(0)) {
		writer.NewPropertyWithValue("TopLevelSoundSet", *m_TopLevelSoundSet);
	}

	if (m_SoundOverlapMode != reference.m_SoundOverlapMode) {
		writer.NewProperty("SoundOverlapMode");
		auto overlapModeMapEntry = std::find_if(c_SoundOverlapModeMap.begin(), c_SoundOverlapModeMap.end(), [&soundOverlapMode = m_SoundOverlapMode](auto element) { return element.second == soundOverlapMode; });
		if (overlapModeMapEntry != c_SoundOverlapModeMap.end()) {
			writer << overlapModeMapEntry->first;
		} else {
			RTEAbort("Tried to write invalid SoundOverlapMode when saving SoundContainer.");
		}
	}

	if (m_BusRouting != reference.m_BusRouting)
		writer.NewPropertyWithValue("BusRouting", m_BusRouting);
	if (m_Immobile != reference.m_Immobile)
		writer.NewPropertyWithValue("Immobile", m_Immobile);
	if (m_AttenuationStartDistance != reference.m_AttenuationStartDistance)
		writer.NewPropertyWithValue("AttenuationStartDistance", m_AttenuationStartDistance);
	if (m_CustomPanValue != reference.m_CustomPanValue)
		writer.NewPropertyWithValue("CustomPanValue", m_CustomPanValue);
	if (m_PanningStrengthMultiplier != reference.m_PanningStrengthMultiplier)
		writer.NewPropertyWithValue("PanningStrengthMultiplier", m_PanningStrengthMultiplier);
	if (m_Loops != reference.m_Loops)
		writer.NewPropertyWithValue("LoopSetting", m_Loops);

	if (m_Priority != reference.m_Priority)
		writer.NewPropertyWithValue("Priority", m_Priority);
	if (m_AffectedByGlobalPitch != reference.m_AffectedByGlobalPitch)
		writer.NewPropertyWithValue("AffectedByGlobalPitch", m_AffectedByGlobalPitch);

	if (m_Pos != reference.m_Pos)
		writer.NewPropertyWithValue("Position", m_Pos);
	if (m_Volume != reference.m_Volume)
		writer.NewPropertyWithValue("Volume", m_Volume);
	if (m_Pitch != reference.m_Pitch)
		writer.NewPropertyWithValue("Pitch", m_Pitch);
	if (m_PitchVariation != reference.m_PitchVariation)
		writer.NewPropertyWithValue("PitchVariation", m_PitchVariation);

	if (m_WasFadedOut != reference.m_WasFadedOut)
		writer.NewPropertyWithValue("WasFadedOut", m_WasFadedOut);
	if (m_Paused != reference.m_Paused)
		writer.NewPropertyWithValue("Paused", m_Paused);
	if (m_MusicPreEntryTime != reference.m_MusicPreEntryTime)
		writer.NewPropertyWithValue("MusicPreEntryTime", m_MusicPreEntryTime);
	if (m_MusicExitTime != reference.m_MusicExitTime)
		writer.NewPropertyWithValue("MusicExitTime", m_MusicExitTime);

	return constituentsConsumed;
}

HashingData SoundContainer::Hash() const {
	HashingData hashData = Entity::Hash();
	uint64_t& hash = hashData.m_Hash;

	uint64_t topLevelSoundSetHash = m_TopLevelSoundSet->Hash().m_Hash;
	hashData.m_Constituents.push_back(topLevelSoundSetHash);
	hash ^= topLevelSoundSetHash << 0;

	hash ^= std::hash<int>{}(m_SoundOverlapMode) << 1;

	hash ^= std::hash<int>{}(m_BusRouting) << 2;
	hash ^= std::hash<bool>{}(m_Immobile) << 3;
	hash ^= std::hash<float>{}(m_AttenuationStartDistance) << 4;
	hash ^= std::hash<float>{}(m_CustomPanValue) << 5;
	hash ^= std::hash<float>{}(m_PanningStrengthMultiplier) << 6;
	hash ^= std::hash<int>{}(m_Loops) << 7;

	hash ^= std::hash<int>{}(m_Priority) << 8;
	hash ^= std::hash<bool>{}(m_AffectedByGlobalPitch) << 9;
	hash ^= m_Pos.Hash().m_Hash << 10;
	hash ^= std::hash<float>{}(m_Volume) << 11;
	hash ^= std::hash<float>{}(m_Pitch) << 12;
	hash ^= std::hash<float>{}(m_PitchVariation) << 13;

	hash ^= std::hash<bool>{}(m_WasFadedOut) << 14;
	hash ^= std::hash<bool>{}(m_Paused) << 15;
	hash ^= std::hash<float>{}(m_MusicPreEntryTime) << 0;
	hash ^= std::hash<float>{}(m_MusicExitTime) << 1;

	return hashData;
}

bool SoundContainer::HasAnySounds() const {
	return m_TopLevelSoundSet->HasAnySounds();
}

float SoundContainer::GetLength(LengthOfSoundType type) const {
	if (!m_SoundPropertiesUpToDate) {
		// Todo - use a post-load fixup stage instead of lazily initializing shit everywhere... Eugh.
		const_cast<SoundContainer*>(this)->UpdateSoundProperties();
		const_cast<SoundContainer*>(this)->m_TopLevelSoundSet->SelectNextSounds();
	}

	std::vector<const SoundData*> flattenedSoundData;
	m_TopLevelSoundSet->GetFlattenedSoundData(flattenedSoundData, type == LengthOfSoundType::NextPlayed);

	float lengthMilliseconds = 0.0f;
	for (const SoundData* selectedSoundData: flattenedSoundData) {
		unsigned int length;
		selectedSoundData->SoundObject->getLength(&length, FMOD_TIMEUNIT_MS);
		lengthMilliseconds = std::max(lengthMilliseconds, static_cast<float>(length));
	}

	return lengthMilliseconds;
}

void SoundContainer::SetTopLevelSoundSet(const SoundSet& newTopLevelSoundSet) {
	*m_TopLevelSoundSet = newTopLevelSoundSet;
	m_SoundPropertiesUpToDate = false;
}

std::vector<std::size_t> SoundContainer::GetSelectedSoundHashes() const {
	std::vector<size_t> soundHashes;
	std::vector<const SoundData*> flattenedSoundData;
	m_TopLevelSoundSet->GetFlattenedSoundData(flattenedSoundData, false);
	for (const SoundData* selectedSoundData: flattenedSoundData) {
		soundHashes.push_back(selectedSoundData->SoundFile.GetHash());
	}
	return soundHashes;
}

const SoundData* SoundContainer::GetSoundDataForSound(const FMOD::Sound* sound) const {
	std::vector<const SoundData*> flattenedSoundData;
	m_TopLevelSoundSet->GetFlattenedSoundData(flattenedSoundData, false);
	for (const SoundData* soundData: flattenedSoundData) {
		if (sound == soundData->SoundObject) {
			return soundData;
		}
	}
	return nullptr;
}

void SoundContainer::SetCustomPanValue(float customPanValue) {
	m_CustomPanValue = std::clamp(customPanValue, -1.0f, 1.0f);
	if (IsBeingPlayed()) {
		g_AudioMan.ChangeSoundContainerPlayingChannelsCustomPanValue(this);
	}
}

void SoundContainer::SetPosition(const Vector& newPosition) {
	if (!m_Immobile && newPosition != m_Pos) {
		m_Pos = newPosition;
		if (IsBeingPlayed()) {
			g_AudioMan.ChangeSoundContainerPlayingChannelsPosition(this);
		}
	}
}

float SoundContainer::GetAudibleVolume() const {
	return g_AudioMan.GetSoundContainerAudibleVolume(this);
}

void SoundContainer::SetVolume(float newVolume) {
	newVolume = std::clamp(newVolume, 0.0F, 10.0F);
	if (IsBeingPlayed()) {
		g_AudioMan.ChangeSoundContainerPlayingChannelsVolume(this, newVolume);
	}
	m_Volume = newVolume;
}

void SoundContainer::SetPitch(float newPitch) {
	m_Pitch = std::clamp(newPitch, 0.125F, 8.0F);
	if (IsBeingPlayed()) {
		g_AudioMan.ChangeSoundContainerPlayingChannelsPitch(this);
	}
}

void SoundContainer::SetPaused(bool paused) {
	if (paused != m_Paused) {
		m_Paused = paused;
		g_AudioMan.SetPausedSoundContainerPlayingChannels(this, paused);
	}
}

bool SoundContainer::Play(int player) {
	if (HasAnySounds()) {
		m_WasFadedOut = false;
		if (IsBeingPlayed()) {
			if (m_SoundOverlapMode == SoundOverlapMode::RESTART) {
				return Restart(player);
			} else if (m_SoundOverlapMode == SoundOverlapMode::IGNORE_PLAY) {
				return false;
			}
		}
		return g_AudioMan.PlaySoundContainer(this, player);
	}
	return false;
}

bool SoundContainer::Stop(int player) {
	return (HasAnySounds() && IsBeingPlayed()) ? g_AudioMan.StopSoundContainerPlayingChannels(this, player) : false;
}

bool SoundContainer::Restart(int player) {
	return (HasAnySounds() && IsBeingPlayed()) ? g_AudioMan.StopSoundContainerPlayingChannels(this, player) && g_AudioMan.PlaySoundContainer(this, player) : false;
}

void SoundContainer::FadeOut(int fadeOutTime) {
	if (!m_WasFadedOut && IsBeingPlayed()) {
		m_WasFadedOut = true;
		return g_AudioMan.FadeOutSoundContainerPlayingChannels(this, fadeOutTime);
	}
}

FMOD_RESULT SoundContainer::UpdateSoundProperties() {
	FMOD_RESULT result = FMOD_OK;

	std::vector<SoundData*> flattenedSoundData;
	m_TopLevelSoundSet->GetFlattenedSoundData(flattenedSoundData, false);
	for (SoundData* soundData: flattenedSoundData) {
		FMOD_MODE soundMode = (m_Loops == 0) ? FMOD_LOOP_OFF : FMOD_LOOP_NORMAL;
		if (m_Immobile) {
			soundMode |= FMOD_2D;
			m_AttenuationStartDistance = c_SoundMaxAudibleDistance;
		} else if (g_AudioMan.GetSoundPanningEffectStrength() == 1.0F) {
			soundMode |= FMOD_3D_INVERSEROLLOFF;
		} else {
			soundMode |= FMOD_3D_CUSTOMROLLOFF;
		}

		result = (result == FMOD_OK) ? soundData->SoundObject->setMode(soundMode) : result;
		result = (result == FMOD_OK) ? soundData->SoundObject->setLoopCount(m_Loops) : result;
		m_AttenuationStartDistance = std::clamp(m_AttenuationStartDistance, 0.0F, static_cast<float>(c_SoundMaxAudibleDistance) - soundData->MinimumAudibleDistance);
		result = (result == FMOD_OK) ? soundData->SoundObject->set3DMinMaxDistance(soundData->MinimumAudibleDistance + m_AttenuationStartDistance, c_SoundMaxAudibleDistance) : result;
	}
	m_SoundPropertiesUpToDate = result == FMOD_OK;

	return result;
}
