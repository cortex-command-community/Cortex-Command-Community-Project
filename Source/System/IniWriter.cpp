#include "IniWriter.h"
#include "Entity.h"
#include "System.h"

#include <fstream>
#include <unordered_map>

using namespace RTE;

namespace {
	/// Gets the preset an Entity is to be written against, if it has one.
	/// @param entity The Entity about to be written.
	/// @return The preset to write the Entity against, or nullptr if it is to be written against a new object of its class.
	const Entity* GetPresetToWriteAgainst(const Entity& entity) {
		const Entity* preset = entity.GetPreset();

		// An original preset finds its own definition. Written against that, it would be no more than a copy of itself.
		if (preset && entity.IsOriginalPreset() && preset->GetModuleID() == entity.GetModuleID()) {
			return nullptr;
		}

		return preset;
	}

	/// Gets a new object of an Entity's class, for what has no preset to be written against: what reading it starts from.
	/// One is made for each class the first time it's needed, and kept for as long as the game runs.
	/// @param entity The Entity about to be written.
	/// @return A new object of the Entity's class.
	const Entity& GetNewObjectOfClass(const Entity& entity) {
		static std::unordered_map<const Entity::ClassInfo*, const Entity*> newObjects;

		auto [newObjectEntry, isNewClass] = newObjects.try_emplace(&entity.GetClass(), nullptr);
		if (isNewClass) {
			// These are never deleted, as their destructors could otherwise run after the managers they use are gone.
			newObjectEntry->second = entity.GetClass().NewInstance();
		}
		return *newObjectEntry->second;
	}
} // namespace

void IniWriter::Clear() {
	m_Stream = nullptr;
	m_FilePath.clear();
	m_FolderPath.clear();
	m_FileName.clear();
	m_IndentCount = 0;
}

IniWriter::IniWriter(const std::string& fileName, bool append, bool createDir) {
	Clear();
	Create(fileName, append, createDir);
}

IniWriter::IniWriter(std::unique_ptr<std::ostream>&& stream) {
	Clear();
	Create(std::move(stream));
}

int IniWriter::Create(const std::string& fileName, bool append, bool createDir) {
	m_FilePath = fileName;

	// Extract filename and folder path
	size_t slashPos = m_FilePath.find_last_of("/\\");
	m_FileName = m_FilePath.substr(slashPos + 1);
	m_FolderPath = m_FilePath.substr(0, slashPos + 1);

	if (createDir && !std::filesystem::exists(System::GetWorkingDirectory() + m_FolderPath)) {
		System::MakeDirectory(System::GetWorkingDirectory() + m_FolderPath);
	}

	return Create(std::make_unique<std::ofstream>(fileName, append ? (std::ios::out | std::ios::app | std::ios::ate) : (std::ios::out | std::ios::trunc)));
}

int IniWriter::Create(std::unique_ptr<std::ostream>&& stream) {
	m_Stream = std::move(stream);
	if (!m_Stream->good()) {
		return -1;
	}

	return 0;
}

void IniWriter::NewLine(bool toIndent, int lineCount) const {
	for (int lines = 0; lines < lineCount; ++lines) {
		*m_Stream << "\n";
		if (toIndent) {
			*m_Stream << std::string(m_IndentCount, '\t');
		}
	}
}

void IniWriter::WriteIdentity() {
	const Entity& entity = Current<Entity>();

	if (IsWrittenAsCopy()) {
		NewPropertyWithValue("CopyOf", Reference<Entity>()->GetModuleAndPresetName());
	}

	// If our preset name differs from the reference's, somehow, we may state the distinction.
	if (entity.IsOriginalPreset()) {
		NewPropertyWithValue("PresetName", entity.GetPresetName());
	} else {
		NewDistinctProperty("InstanceName", [](const Entity& entity) { return entity.GetPresetName(); });
	}
}

void IniWriter::WriteInto(const Serializable& value, const Serializable& referenceValue) {
	// A copy of a preset is reset to the preset by its CopyOf when it's read, so it's written against that instead.
	if (const Entity* entity = dynamic_cast<const Entity*>(&value)) {
		if (const Entity* preset = GetPresetToWriteAgainst(*entity)) {
			WriteObject(value, preset, true);
			ObjectEnd();
			return;
		}
	}
	WriteObject(value, &referenceValue, false);
	ObjectEnd();
}

std::pair<const Serializable*, bool> IniWriter::ReferenceFor(const Serializable& object) const {
	if (const Entity* entity = dynamic_cast<const Entity*>(&object)) {
		if (const Entity* preset = GetPresetToWriteAgainst(*entity)) {
			return {preset, true};
		}
		return {&GetNewObjectOfClass(*entity), false};
	}
	// A Serializable that isn't an Entity has no class to make a new object of, so all of it is written.
	return {nullptr, false};
}

IniWriter& IniWriter::operator<<(const Serializable& object) {
	const auto [reference, referenceIsPreset] = ReferenceFor(object);
	WriteObject(object, reference, referenceIsPreset);
	ObjectEnd();
	return *this;
}

IniWriter& IniWriter::operator<<(const Serializable* object) {
	if (object) {
		*this << *object;
	} else {
		NoObject();
	}
	return *this;
}
