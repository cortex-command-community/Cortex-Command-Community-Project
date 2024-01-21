#include "SoundSet.h"
#include "AudioMan.h"
#include "RTETools.h"
#include "RTEError.h"

namespace RTE {

	const std::string SoundSet::m_sClassName = "SoundSet";

	const std::unordered_map<std::string, SoundSet::SoundSelectionCycleMode> SoundSet::c_SoundSelectionCycleModeMap = {
	    {"random", SoundSelectionCycleMode::RANDOM},
	    {"forwards", SoundSelectionCycleMode::FORWARDS},
	    {"all", SoundSelectionCycleMode::ALL}};

	SoundSet::SoundSet() {
		Clear();
	}

	SoundSet::~SoundSet() {
		Destroy();
	}

	void SoundSet::Clear() {
		m_SoundSelectionCycleMode = SoundSelectionCycleMode::RANDOM;
		m_CurrentSelection = {false, -1};

		m_SoundData.clear();
		m_SubSoundSets.clear();
	}

	int SoundSet::Create(const SoundSet& reference) {
		m_SoundSelectionCycleMode = reference.m_SoundSelectionCycleMode;
		m_CurrentSelection = reference.m_CurrentSelection;
		for (SoundData referenceSoundData: reference.m_SoundData) {
			m_SoundData.push_back(referenceSoundData);
		}
		for (const SoundSet& referenceSoundSet: reference.m_SubSoundSets) {
			SoundSet soundSet;
			soundSet.Create(referenceSoundSet);
			m_SubSoundSets.push_back(soundSet);
		}

		return 0;
	}

	int SoundSet::ReadProperty(const std::string_view& propName, Reader& reader) {
		StartPropertyList(return Serializable::ReadProperty(propName, reader));

		MatchProperty("SoundSelectionCycleMode", { SetSoundSelectionCycleMode(ReadSoundSelectionCycleMode(reader)); });
		MatchProperty("AddSound", { AddSoundData(ReadAndGetSoundData(reader)); });
		MatchProperty("AddSoundSet", {
			SoundSet soundSetToAdd;
			reader >> soundSetToAdd;
			AddSoundSet(soundSetToAdd);
		});

		EndPropertyList;
	}

	SoundData SoundSet::ReadAndGetSoundData(Reader& reader) {
		SoundData soundData;

		/// <summary>
		/// Internal lambda function to load an audio file by path in as a ContentFile, which in turn loads it into FMOD, then returns SoundData for it in the outParam outSoundData.
		/// </summary>
		/// <param name="soundPath">The path to the sound file.</param>
		auto readSoundFromPath = [&soundData, &reader](const std::string& soundPath) {
			ContentFile soundFile(soundPath.c_str());

			/// As Serializable::Create(&reader) isn't being used here, we need to set our formatted reader position manually.
			soundFile.SetFormattedReaderPosition("in file " + reader.GetCurrentFilePath() + " on line " + reader.GetCurrentFileLine());

			FMOD::Sound* soundObject = soundFile.GetAsSound();
			if (g_AudioMan.IsAudioEnabled() && !soundObject) {
				reader.ReportError(std::string("Failed to load the sound from the file"));
			}

			soundData.SoundFile = soundFile;
			soundData.SoundObject = soundObject;
		};

		std::string propValue = reader.ReadPropValue();
		if (propValue != "Sound" && propValue != "ContentFile") {
			readSoundFromPath(propValue);
			return soundData;
		}

		while (reader.NextProperty()) {
			std::string soundSubPropertyName = reader.ReadPropName();
			if (soundSubPropertyName == "FilePath" || soundSubPropertyName == "Path") {
				readSoundFromPath(reader.ReadPropValue());
			} else if (soundSubPropertyName == "Offset") {
				reader >> soundData.Offset;
			} else if (soundSubPropertyName == "MinimumAudibleDistance") {
				reader >> soundData.MinimumAudibleDistance;
			} else if (soundSubPropertyName == "AttenuationStartDistance") {
				reader >> soundData.AttenuationStartDistance;
			}
		}

		return soundData;
	}

	SoundSet::SoundSelectionCycleMode SoundSet::ReadSoundSelectionCycleMode(Reader& reader) {
		SoundSelectionCycleMode soundSelectionCycleModeToReturn;
		std::string soundSelectionCycleModeString = reader.ReadPropValue();
		std::locale locale;
		for (char& character: soundSelectionCycleModeString) {
			character = std::tolower(character, locale);
		}

		std::unordered_map<std::string, SoundSelectionCycleMode>::const_iterator soundSelectionCycleMode = c_SoundSelectionCycleModeMap.find(soundSelectionCycleModeString);
		if (soundSelectionCycleMode != c_SoundSelectionCycleModeMap.end()) {
			soundSelectionCycleModeToReturn = soundSelectionCycleMode->second;
		} else {
			try {
				soundSelectionCycleModeToReturn = static_cast<SoundSelectionCycleMode>(std::stoi(soundSelectionCycleModeString));
			} catch (const std::exception&) {
				reader.ReportError("Sound selection cycle mode " + soundSelectionCycleModeString + " is invalid.");
			}
		}

		return soundSelectionCycleModeToReturn;
	}

	int SoundSet::Save(Writer& writer) const {
		Serializable::Save(writer);

		writer.NewProperty("SoundSelectionCycleMode");
		SaveSoundSelectionCycleMode(writer, m_SoundSelectionCycleMode);

		for (const SoundData& soundData: m_SoundData) {
			writer.NewProperty("AddSound");
			writer.ObjectStart("ContentFile");

			writer.NewProperty("FilePath");
			writer << soundData.SoundFile.GetDataPath();
			writer.NewProperty("Offset");
			writer << soundData.Offset;
			writer.NewProperty("MinimumAudibleDistance");
			writer << soundData.MinimumAudibleDistance;
			writer.NewProperty("AttenuationStartDistance");
			writer << soundData.AttenuationStartDistance;

			writer.ObjectEnd();
		}

		for (const SoundSet& subSoundSet: m_SubSoundSets) {
			writer.NewProperty("AddSoundSet");
			writer.ObjectStart("SoundSet");
			writer << subSoundSet;
			writer.ObjectEnd();
		}

		return 0;
	}

	void SoundSet::SaveSoundSelectionCycleMode(Writer& writer, SoundSelectionCycleMode soundSelectionCycleMode) {
		auto cycleModeMapEntry = std::find_if(c_SoundSelectionCycleModeMap.begin(), c_SoundSelectionCycleModeMap.end(), [&soundSelectionCycleMode = soundSelectionCycleMode](auto element) { return element.second == soundSelectionCycleMode; });
		if (cycleModeMapEntry != c_SoundSelectionCycleModeMap.end()) {
			writer << cycleModeMapEntry->first;
		} else {
			RTEAbort("Tried to write invalid SoundSelectionCycleMode when saving SoundContainer/SoundSet.");
		}
	}

	void SoundSet::AddSound(const std::string& soundFilePath, const Vector& offset, float minimumAudibleDistance, float attenuationStartDistance, bool abortGameForInvalidSound) {
		ContentFile soundFile(soundFilePath.c_str());
		FMOD::Sound* soundObject = soundFile.GetAsSound(abortGameForInvalidSound, false);
		if (!soundObject) {
			return;
		}

		m_SoundData.push_back({soundFile, soundObject, offset, minimumAudibleDistance, attenuationStartDistance});
	}

	bool SoundSet::RemoveSound(const std::string& soundFilePath, bool removeFromSubSoundSets) {
		auto soundsToRemove = std::remove_if(m_SoundData.begin(), m_SoundData.end(), [&soundFilePath](const SoundData& soundData) { return soundData.SoundFile.GetDataPath() == soundFilePath; });
		bool anySoundsToRemove = soundsToRemove != m_SoundData.end();
		if (anySoundsToRemove) {
			m_SoundData.erase(soundsToRemove, m_SoundData.end());
		}
		if (removeFromSubSoundSets) {
			for (SoundSet subSoundSet: m_SubSoundSets) {
				anySoundsToRemove |= RemoveSound(soundFilePath, removeFromSubSoundSets);
			}
		}
		return anySoundsToRemove;
	}

	bool SoundSet::HasAnySounds(bool includeSubSoundSets) const {
		bool hasAnySounds = !m_SoundData.empty();
		if (!hasAnySounds && includeSubSoundSets) {
			for (const SoundSet& subSoundSet: m_SubSoundSets) {
				hasAnySounds = subSoundSet.HasAnySounds();
				if (hasAnySounds) {
					break;
				}
			}
		}
		return hasAnySounds;
	}

	void SoundSet::GetFlattenedSoundData(std::vector<SoundData*>& flattenedSoundData, bool onlyGetSelectedSoundData) {
		if (!onlyGetSelectedSoundData || m_SoundSelectionCycleMode == SoundSelectionCycleMode::ALL) {
			for (SoundData& soundData: m_SoundData) {
				flattenedSoundData.push_back(&soundData);
			}
			for (SoundSet& subSoundSet: m_SubSoundSets) {
				subSoundSet.GetFlattenedSoundData(flattenedSoundData, onlyGetSelectedSoundData);
			}
		} else {
			if (m_CurrentSelection.first == false) {
				flattenedSoundData.push_back(&m_SoundData[m_CurrentSelection.second]);
			} else {
				m_SubSoundSets[m_CurrentSelection.second].GetFlattenedSoundData(flattenedSoundData, onlyGetSelectedSoundData);
			}
		}
	}

	void SoundSet::GetFlattenedSoundData(std::vector<const SoundData*>& flattenedSoundData, bool onlyGetSelectedSoundData) const {
		if (!onlyGetSelectedSoundData || m_SoundSelectionCycleMode == SoundSelectionCycleMode::ALL) {
			for (const SoundData& soundData: m_SoundData) {
				flattenedSoundData.push_back(&soundData);
			}
			for (const SoundSet& subSoundSet: m_SubSoundSets) {
				subSoundSet.GetFlattenedSoundData(flattenedSoundData, onlyGetSelectedSoundData);
			}
		} else {
			if (m_CurrentSelection.first == false) {
				flattenedSoundData.push_back(&m_SoundData[m_CurrentSelection.second]);
			} else {
				m_SubSoundSets[m_CurrentSelection.second].GetFlattenedSoundData(flattenedSoundData, onlyGetSelectedSoundData);
			}
		}
	}

	bool SoundSet::SelectNextSounds() {
		if (m_SoundSelectionCycleMode == SoundSelectionCycleMode::ALL) {
			for (SoundSet& subSoundSet: m_SubSoundSets) {
				if (!subSoundSet.SelectNextSounds()) {
					return false;
				}
			}
			return true;
		}
		int selectedVectorSize = m_CurrentSelection.first == false ? m_SoundData.size() : m_SubSoundSets.size();
		int unselectedVectorSize = m_CurrentSelection.first == true ? m_SoundData.size() : m_SubSoundSets.size();
		if (selectedVectorSize == 0 && unselectedVectorSize > 0) {
			m_CurrentSelection.first = !m_CurrentSelection.first;
			std::swap(selectedVectorSize, unselectedVectorSize);
		}

		/// <summary>
		/// Internal lambda function to pick a random sound that's not the previously played sound. Done to avoid scoping issues inside the switch below.
		/// </summary>
		auto selectSoundRandom = [&selectedVectorSize, &unselectedVectorSize, this]() {
			if (unselectedVectorSize > 0 && (selectedVectorSize == 1 || RandomNum(0, 1) == 1)) {
				std::swap(selectedVectorSize, unselectedVectorSize);
				m_CurrentSelection = {!m_CurrentSelection.first, RandomNum(0, selectedVectorSize - 1)};
			} else {
				size_t soundToSelect = RandomNum(0, selectedVectorSize - 1);
				while (soundToSelect == m_CurrentSelection.second) {
					soundToSelect = RandomNum(0, selectedVectorSize - 1);
				}
				m_CurrentSelection.second = soundToSelect;
			}
		};

		/// <summary>
		/// Internal lambda function to pick the next sound in the forwards direction.
		/// </summary>
		auto selectSoundForwards = [&selectedVectorSize, &unselectedVectorSize, this]() {
			m_CurrentSelection.second++;
			if (m_CurrentSelection.second > selectedVectorSize - 1) {
				m_CurrentSelection.second = 0;
				if (unselectedVectorSize > 0) {
					m_CurrentSelection.first = !m_CurrentSelection.first;
					std::swap(selectedVectorSize, unselectedVectorSize);
				}
			}
		};

		switch (selectedVectorSize + unselectedVectorSize) {
			case 0:
				return false;
			case 1:
				m_CurrentSelection.second = 0;
				break;
			default:
				switch (m_SoundSelectionCycleMode) {
					case SoundSelectionCycleMode::RANDOM:
						selectSoundRandom();
						break;
					case SoundSelectionCycleMode::FORWARDS:
						selectSoundForwards();
						break;
					default:
						RTEAbort("Invalid sound selection sound cycle mode. " + m_SoundSelectionCycleMode);
						break;
				}
				RTEAssert(m_CurrentSelection.second >= 0 && m_CurrentSelection.second < selectedVectorSize, "Failed to select next sound, either none was selected or the selected sound was invalid.");
		}

		if (m_CurrentSelection.first == true) {
			return m_SubSoundSets[m_CurrentSelection.second].SelectNextSounds();
		}

		return true;
	}
} // namespace RTE
