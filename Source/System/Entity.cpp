#include "Entity.h"

#include "RTETools.h"
#include "PresetMan.h"
#include "DataModule.h"

#include <sstream>

namespace RTE {

	Entity::ClassInfo Entity::m_sClass("Entity");
	Entity::ClassInfo* Entity::ClassInfo::s_ClassHead = 0;

	Entity::Entity() {
		Clear();
	}

	Entity::~Entity() {
		Destroy(true);
	}

	void Entity::Clear() {
		m_PresetName = "None";
		m_IsOriginalPreset = false;
		m_DefinedInModule = -1;
		m_PresetDescription.clear();
		m_Groups.clear();
		m_RandomWeight = 100;
	}

	int Entity::Create() {
		return 0;
	}

	int Entity::Create(const Entity& reference) {
		m_PresetName = reference.m_PresetName;
		// Note how m_IsOriginalPreset is NOT assigned, automatically indicating that the copy is not an original Preset!
		m_DefinedInModule = reference.m_DefinedInModule;
		m_PresetDescription = reference.m_PresetDescription;

		for (const std::string& group: reference.m_Groups) {
			m_Groups.emplace(group);
		}
		m_RandomWeight = reference.m_RandomWeight;
		return 0;
	}

	int Entity::ReadProperty(const std::string_view& propName, Reader& reader) {
		StartPropertyList(
		    // Search for a property name match failed!
		    // TODO: write this out to some log file
		    return Serializable::ReadProperty(propName, reader););

		MatchProperty("CopyOf", {
			std::string refName = reader.ReadPropValue();
			if (refName != "None") {
				std::string className = GetClassName();
				const Entity* preset = g_PresetMan.GetEntityPreset(className, refName, reader.GetReadModuleID());
				if (preset) {
					preset->Clone(this);
				} else {
					reader.ReportError("Couldn't find the preset \"" + refName + "\" of type \"" + className + "\" when trying to do CopyOf.");
				}
			}
		});
		MatchProperty("PresetName", {
			SetPresetName(reader.ReadPropValue());

			// Indicate where this was read from.
			m_DefinedInModule = reader.GetReadModuleID();

			// Preset name might have "[ModuleName]/" preceding it. It's deliberately ignored: a module can't define presets in another one, tempting as it is to let modules patch each other.
			size_t slashPos = m_PresetName.find_first_of('/');
			if (slashPos != std::string::npos) {
				m_PresetName = m_PresetName.substr(slashPos + 1);
			}
		});
		MatchProperty("InstanceName", {
			// Set this directly so that SetPresetName doesn't confuse that we're an orignal preset.
			// This names something that is a copy of a preset, without making it a preset or needing one to refer to.
			m_PresetName = reader.ReadPropValue();

			// Indicate where this was read from
			m_DefinedInModule = reader.GetReadModuleID();

			// Preset name might have "[ModuleName]/" preceding it. It's deliberately ignored: a module can't define presets in another one, tempting as it is to let modules patch each other.
			int slashPos = m_PresetName.find_first_of('/');
			if (slashPos != std::string::npos) {
				m_PresetName = m_PresetName.substr(slashPos + 1);
			}
		});
		MatchProperty("Description", {
			std::string descriptionValue = reader.ReadPropValue();
			if (descriptionValue == "MultiLineText") {
				m_PresetDescription.clear();
				while (reader.NextProperty() && reader.ReadPropName() == "AddLine") {
					m_PresetDescription += reader.ReadPropValue() + "\n\n";
				}
				if (!m_PresetDescription.empty()) {
					m_PresetDescription.resize(m_PresetDescription.size() - 2);
				}
			} else {
				m_PresetDescription = descriptionValue;
			}
		});
		MatchProperty("RandomWeight", {
			reader >> m_RandomWeight;
			m_RandomWeight = Limit(m_RandomWeight, 100, 0);
		});
		MatchProperty("_ClearGroups", {
			reader.ReadPropValue();
			m_Groups.clear();
		});
		MatchProperty("AddToGroup", {
			std::string newGroup;
			reader >> newGroup;
			AddToGroup(newGroup);
			// Do this in AddToGroup instead?
			g_PresetMan.RegisterGroup(newGroup, reader.GetReadModuleID());
		});
		MatchProperty("_RemoveFromGroup", {
			std::string oldGroup;
			reader >> oldGroup;
			RemoveFromGroup(oldGroup);
			// Don't need to unregister group or anything.
		});

		EndPropertyList;
	}

	/// Writes a description in the form it's read. A property's value can only be one line, so anything longer is written as MultiLineText.
	/// @param writer The writer to write the description with.
	/// @param description The description to write.
	template <typename WriterType> static void WriteDescription(WriterType& writer, const std::string& description) {
		if (description.find('\n') == std::string::npos) {
			writer.NewPropertyWithValue("Description", description);
			return;
		}

		writer.NewProperty("Description");
		writer.ObjectStart("MultiLineText");

		std::istringstream lines(description);
		std::string line;

		while (std::getline(lines, line)) {
			if (!line.empty()) {
				writer.NewPropertyWithValue("AddLine", line);
			}
		}

		writer.ObjectEnd();
	}

	int Entity::Save(Writer& writer) const {
		Serializable::Save(writer);

		// This writes everything that differs from a new object, for what has no preset to compare against. See Write for what does.

		if (m_IsOriginalPreset) {
			writer.NewPropertyWithValue("PresetName", m_PresetName);
		} else if (!m_PresetName.empty() && m_PresetName != "None") {
			writer.NewPropertyWithValue("InstanceName", m_PresetName);
		}

		if (!m_PresetDescription.empty()) {
			WriteDescription(writer, m_PresetDescription);
		}

		for (auto itr = m_Groups.begin(); itr != m_Groups.end(); ++itr) {
		    writer.NewPropertyWithValue("AddToGroup", *itr);
		}

		writer.NewDistinctProperty("RandomWeight", m_RandomWeight, 100);

		return 0;
	}

	int Entity::Write(Writer& writer, const Entity& entityReference, HashingData& hashData) const {
		writer.ObjectStart(GetClassName());

		writer.NewPropertyWithValue("CopyOf", entityReference.GetModuleAndPresetName());

		// If our preset name differs from the reference's, somehow, we may state the distinction.
		if (m_IsOriginalPreset) {
			writer.NewPropertyWithValue("PresetName", m_PresetName);
		} else {
			writer.NewDistinctProperty("InstanceName", m_PresetName, entityReference.m_PresetName);
		}

		if (m_PresetDescription != entityReference.m_PresetDescription) {
			WriteDescription(writer, m_PresetDescription);
		}
		
		writer.NewDistinctCollection("_ClearGroups", "AddToGroup", m_Groups, entityReference.m_Groups);

		writer.NewDistinctProperty("RandomWeight", m_RandomWeight, entityReference.m_RandomWeight);

		return 0;
	}

	HashingData Entity::Hash() const {
		HashingData hashData(Serializable::Hash());

		hashData.Add(m_PresetName);
		hashData.Add(m_DefinedInModule);
		hashData.Add(m_PresetDescription);
		hashData.AddUnordered(m_Groups);
		hashData.Add(m_RandomWeight);

		return hashData;
	}

	template <typename WriterType> int Entity::WriteTo(WriterType& writer) const {
		Serializable::WriteTo(writer);

		writer.WriteIdentity();

		if (writer.IsDistinct(&Entity::m_PresetDescription)) {
			WriteDescription(writer, m_PresetDescription);
		}

		writer.NewDistinctCollection("_ClearGroups", "AddToGroup", &Entity::m_Groups);

		writer.NewDistinctProperty("RandomWeight", &Entity::m_RandomWeight);

		return 0;
	}
	template int Entity::WriteTo(IniWriter& writer) const;
	template int Entity::WriteTo(HashWriter& writer) const;

	int Entity::SavePresetCopy(Writer& writer) const {
		// Can only save out copies with this
		if (m_IsOriginalPreset) {
			RTEAbort("Tried to save out a pure Preset Copy Reference from an original Preset!");
			return -1;
		}
		writer.ObjectStart(GetClassName());
		writer.NewPropertyWithValue("CopyOf", GetModuleAndPresetName());
		writer.ObjectEnd();

		return 0;
	}

	const Entity* Entity::GetPreset() const {
		return g_PresetMan.GetEntityPreset(GetClassName(), GetPresetName(), m_DefinedInModule);
	}

	std::string Entity::GetEntityCharacteristic() const {
		return GetClassName() + "/" + GetModuleAndPresetName();
	}

	std::string Entity::GetModuleAndPresetName() const {
		if (m_DefinedInModule < 0) {
			return GetPresetName();
		}
		const DataModule* dataModule = g_PresetMan.GetDataModule(m_DefinedInModule);

		if (!dataModule) {
			return GetPresetName();
		}
		return dataModule->GetFileName() + "/" + GetPresetName();
	}

	std::string Entity::GetModuleName() const {
		if (m_DefinedInModule >= 0) {
			if (const DataModule* dataModule = g_PresetMan.GetDataModule(m_DefinedInModule)) {
				return dataModule->GetFileName();
			}
		}
		return "";
	}

	bool Entity::MigrateToModule(int whichModule) {
		if (m_DefinedInModule == whichModule) {
			return false;
		}
		m_IsOriginalPreset = true; // This now a unique snowflake
		m_DefinedInModule = whichModule;
		return true;
	}

	Reader& operator>>(Reader& reader, Entity& operand) {
		// Get this before reading Entity, since if it's the last one in its datafile, the stream will show the parent file instead
		std::string objectFilePath = reader.GetCurrentFilePath();
		// Read the Entity from the file and try to add it to PresetMan
		operand.Create(reader);
		g_PresetMan.AddEntityPreset(&operand, reader.GetReadModuleID(), reader.GetPresetOverwriting(), objectFilePath);

		return reader;
	}

	Reader& operator>>(Reader& reader, Entity* operand) {
		if (operand) {
			// Get this before reading Entity, since if it's the last one in its datafile, the stream will show the parent file instead
			std::string objectFilePath = reader.GetCurrentFilePath();
			// Read the Entity from the file and try to add it to PresetMan
			operand->Create(reader);
			g_PresetMan.AddEntityPreset(operand, reader.GetReadModuleID(), reader.GetPresetOverwriting(), objectFilePath);
		} else {
			reader.ReportError("Tried to read an .ini file into a null Entity pointer!");
		}
		return reader;
	}

	Entity::ClassInfo::ClassInfo(const std::string& name, ClassInfo* parentInfo, MemoryAllocate allocFunc, MemoryDeallocate deallocFunc, Entity* (*newFunc)(), int allocBlockCount) :
	    m_Name(name),
	    m_ParentInfo(parentInfo),
	    m_Allocate(allocFunc),
	    m_Deallocate(deallocFunc),
	    m_NewInstance(newFunc),
	    m_NextClass(s_ClassHead) {
		s_ClassHead = this;

		m_AllocatedPool.clear();
		m_PoolAllocBlockCount = (allocBlockCount > 0) ? allocBlockCount : 10;
	}

	std::list<std::string> Entity::ClassInfo::GetClassNames() {
		std::list<std::string> retList;
		for (const ClassInfo* itr = s_ClassHead; itr != 0; itr = itr->m_NextClass) {
			retList.push_back(itr->GetName());
		}
		return retList;
	}

	const Entity::ClassInfo* Entity::ClassInfo::GetClass(const std::string& name) {
		if (name.empty() || name == "None") {
			return 0;
		}
		for (const ClassInfo* itr = s_ClassHead; itr != 0; itr = itr->m_NextClass) {
			if (itr->GetName() == name) {
				return itr;
			}
		}
		return 0;
	}

	void Entity::ClassInfo::FillAllPools(int fillAmount) {
		for (ClassInfo* itr = s_ClassHead; itr != 0; itr = itr->m_NextClass) {
			if (itr->IsConcrete()) {
				itr->FillPool(fillAmount);
			}
		}
	}

	void Entity::ClassInfo::FillPool(int fillAmount) {
#ifdef __SANITIZE_ADDRESS__
		// If we have ASan, make this a no-op.
		(void)(fillAmount); // Silence warning about unused variable.
#else

		// Default to the set block allocation size if fillAmount is 0
		if (fillAmount <= 0) {
			fillAmount = m_PoolAllocBlockCount;
		}

		// If concrete class, fill up the pool with pre-allocated memory blocks the size of the type
		if (m_Allocate && fillAmount > 0) {
			for (int i = 0; i < fillAmount; ++i) {
				m_AllocatedPool.push_back(m_Allocate());
			}
		}
#endif
	}

	bool Entity::ClassInfo::IsClassOrChildClassOf(const ClassInfo* classInfoToCheck) const {
		if (GetName() == classInfoToCheck->GetName()) {
			return true;
		} else if (m_ParentInfo) {
			return m_ParentInfo->IsClassOrChildClassOf(classInfoToCheck);
		}
		return false;
	}

	void* Entity::ClassInfo::GetPoolMemory() {
#ifdef __SANITIZE_ADDRESS__
		// If compiled with ASan, sidestep pooling and just use the allocator normally.

		void* foundMemory = m_Allocate();
		RTEAssert(foundMemory, "m_Allocate failed! to make memory!");
#else

		std::lock_guard<std::mutex> guard(m_Mutex);

		RTEAssert(IsConcrete(), "Trying to get pool memory of an abstract Entity class!");

		// If the pool is empty, then fill it up again with as many instances as we are set to
		if (m_AllocatedPool.empty()) {
			FillPool((m_PoolAllocBlockCount > 0) ? m_PoolAllocBlockCount : 10);
		}

		// Get the instance in the top of the pool and pop it off
		void* foundMemory = m_AllocatedPool.back();
		m_AllocatedPool.pop_back();

		RTEAssert(foundMemory, "Could not find an available instance in the pool, even after increasing its size!");
#endif

		// Keep track of the number of instances passed out
		m_InstancesInUse++;

		return foundMemory;
	}

	int Entity::ClassInfo::ReturnPoolMemory(void* returnedMemory) {
		if (!returnedMemory) {
			return 0;
		}

#ifdef __SANITIZE_ADDRESS__
		// If compiled with ASan, sidestep pooling and just use the allocator normally.
		m_Deallocate(returnedMemory);
#else
		std::lock_guard<std::mutex> guard(m_Mutex);
		m_AllocatedPool.push_back(returnedMemory);
#endif

		// Keep track of the number of instances passed in
		m_InstancesInUse--;

		return m_InstancesInUse;
	}

	void Entity::ClassInfo::DumpPoolMemoryInfo(const Writer& fileWriter) {
		for (const ClassInfo* itr = s_ClassHead; itr != nullptr; itr = itr->m_NextClass) {
			if (itr->IsConcrete()) {
				fileWriter.NewLineString(itr->GetName() + ": " + std::to_string(itr->m_InstancesInUse), false);
			}
		}
	}
} // namespace RTE
