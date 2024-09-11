#pragma once

#include "Serializable.h"
#include "RTEError.h"

#include <list>
#include <unordered_set>

namespace RTE {

	typedef std::function<void*()> MemoryAllocate; //!< Convenient name definition for the memory allocation callback function.
	typedef std::function<void(void*)> MemoryDeallocate; //!< Convenient name definition for the memory deallocation callback function.

#pragma region Global Macro Definitions
#define AbstractClassInfo(TYPE, PARENT) \
	Entity::ClassInfo TYPE::m_sClass(#TYPE, &PARENT::m_sClass);

#define ConcreteClassInfo(TYPE, PARENT, BLOCKCOUNT) \
	Entity::ClassInfo TYPE::m_sClass(#TYPE, &PARENT::m_sClass, TYPE::Allocate, TYPE::Deallocate, TYPE::NewInstance, BLOCKCOUNT);

#define ConcreteSubClassInfo(TYPE, SUPER, PARENT, BLOCKCOUNT) \
	Entity::ClassInfo SUPER::TYPE::m_sClass(#TYPE, &PARENT::m_sClass, SUPER::TYPE::Allocate, SUPER::TYPE::Deallocate, SUPER::TYPE::NewInstance, BLOCKCOUNT);

/// Convenience macro to cut down on duplicate ClassInfo methods in classes that extend Entity.
#define ClassInfoGetters \
	const Entity::ClassInfo& GetClass() const override { return m_sClass; } \
	const std::string& GetClassName() const override { return m_sClass.GetName(); }

/// Static method used in conjunction with ClassInfo to allocate an Entity.
/// This function is passed into the constructor of this Entity's static ClassInfo's constructor, so that it can instantiate MovableObjects.
/// @return A pointer to the newly dynamically allocated Entity. Ownership is transferred as well.
#define EntityAllocation(TYPE) \
	static void* operator new(size_t size) { return TYPE::m_sClass.GetPoolMemory(); } \
	static void operator delete(void* instance) { TYPE::m_sClass.ReturnPoolMemory(instance); } \
	static void* operator new(size_t size, void* p) throw() { return p; } \
	static void operator delete(void*, void*) throw() {} \
	static void* Allocate() { return malloc(sizeof(TYPE)); } \
	static void Deallocate(void* instance) { free(instance); } \
	static Entity* NewInstance() { return new TYPE; } \
	Entity* Clone(Entity* cloneTo = nullptr) const override { \
		TYPE* ent = cloneTo ? dynamic_cast<TYPE*>(cloneTo) : new TYPE(); \
		RTEAssert(ent, "Tried to clone to an incompatible instance!"); \
		if (cloneTo) { \
			ent->Destroy(); \
		} \
		ent->Create(*this); \
		return ent; \
	}
#pragma endregion

	/// Whether to draw the colors, or own material property, or to clear the corresponding non-key-color pixels of the Entity being drawn with key-color pixels on the target.
	enum DrawMode {
		g_DrawColor,
		g_DrawMaterial,
		g_DrawWhite,
		g_DrawMOID,
		g_DrawDoor,
		g_DrawTrans,
		g_DrawAlpha
	};

	/// The base class that specifies certain common creation/destruction patterns and simple reflection support for virtually all RTE classes.
	class Entity : public Serializable {
		friend class DataModule;

	public:
		SerializableOverrideMethods;

#pragma region ClassInfo
		/// The class that describes each subclass of Entity. There should be one ClassInfo static instance for every Entity child.
		class ClassInfo {
			friend class Entity;

		public:
#pragma region Creation
			/// Constructor method used to instantiate a ClassInfo Entity.
			/// @param name A friendly-formatted name of the Entity that is going to be represented by this ClassInfo.
			/// @param parentInfo Pointer to the parent class' info. 0 if this describes a root class.
			/// @param allocFunc Function pointer to the raw allocation function of the derived's size. If the represented Entity subclass isn't concrete, pass in 0.
			/// @param deallocFunc Function pointer to the raw deallocation function of memory. If the represented Entity subclass isn't concrete, pass in 0.
			/// @param newFunc Function pointer to the new instance factory. If the represented Entity subclass isn't concrete, pass in 0.
			/// @param allocBlockCount The number of new instances to fill the pre-allocated pool with when it runs out.
			ClassInfo(const std::string& name, ClassInfo* parentInfo = 0, MemoryAllocate allocFunc = 0, MemoryDeallocate deallocFunc = 0, Entity* (*newFunc)() = 0, int allocBlockCount = 10);
#pragma endregion

#pragma region Getters
			/// Gets the name of this ClassInfo.
			/// @return A string with the friendly-formatted name of this ClassInfo.
			const std::string& GetName() const { return m_Name; }

			/// Gets the names of all ClassInfos in existence.
			/// @return A list of the names.
			static std::list<std::string> GetClassNames();

			/// Gets the ClassInfo of a particular RTE class corresponding to a friendly-formatted string name.
			/// @param name The friendly name of the desired ClassInfo.
			/// @return A pointer to the requested ClassInfo, or 0 if none that matched the name was found. Ownership is NOT transferred!
			static const ClassInfo* GetClass(const std::string& name);

			/// Gets the ClassInfo which describes the parent of this.
			/// @return A pointer to the parent ClassInfo. 0 if this is a root class.
			const ClassInfo* GetParent() const { return m_ParentInfo; }

			/// Gets whether or not this ClassInfo is the same as, or a parent of the ClassInfo corresponding to the given class name.
			/// @param classNameToCheck The name of the class to check for.
			/// @return Whether or not this ClassInfo is the same as, or a parent of corresponding ClassInfo for the given class.
			bool IsClassOrParentClassOf(const std::string& classNameToCheck) const { return GetClass(classNameToCheck)->IsClassOrChildClassOf(this); }

			/// Gets whether or not this ClassInfo is the same as, or a parent of the given ClassInfo.
			/// @param classNameToCheck The name of the class to check for.
			/// @return Whether or not this ClassInfo is the same as, or a parent of the given ClassInfo.
			bool IsClassOrParentClassOf(const ClassInfo* classInfoToCheck) const { return classInfoToCheck->IsClassOrChildClassOf(this); }

			/// Gets whether or not this ClassInfo is the same as, or a child of the ClassInfo corresponding to the given class name.
			/// @param classNameToCheck The name of the class to check for.
			/// @return Whether or not this ClassInfo is the same as, or a child of corresponding ClassInfo for the given class.
			bool IsClassOrChildClassOf(const std::string& classNameToCheck) const { return IsClassOrChildClassOf(GetClass(classNameToCheck)); }

			/// Gets whether or not this ClassInfo is the same as, or a child of the given ClassInfo.
			/// @param classNameToCheck The name of the class to check for.
			/// @return Whether or not this ClassInfo is the same as, or a child of the given ClassInfo.
			bool IsClassOrChildClassOf(const ClassInfo* classInfoToCheck) const;
#pragma endregion

#pragma region Memory Management
			/// Grabs from the pre-allocated pool, an available chunk of memory the exact size of the Entity this ClassInfo represents. OWNERSHIP IS TRANSFERRED!
			/// @return A pointer to the pre-allocated pool memory. OWNERSHIP IS TRANSFERRED!
			void* GetPoolMemory();

			/// Returns a raw chunk of memory back to the pre-allocated available pool.
			/// @param returnedMemory The raw chunk of memory that is being returned. Needs to be the same size as the type this ClassInfo describes. OWNERSHIP IS TRANSFERRED!
			/// @return The count of outstanding memory chunks after this was returned.
			int ReturnPoolMemory(void* returnedMemory);

			/// Writes a bunch of useful debug info about the memory pools to a file.
			/// @param fileWriter The writer to write info to.
			static void DumpPoolMemoryInfo(const Writer& fileWriter);

			/// Adds a certain number of newly allocated instances to this' pool.
			/// @param fillAmount The number of instances to fill the pool with. If 0 is specified, the set refill amount will be used.
			void FillPool(int fillAmount = 0);

			/// Adds a certain number of newly allocated instances to all pools.
			/// @param fillAmount The number of instances to fill the pool with. If 0 is specified, the set refill amount will be used.
			static void FillAllPools(int fillAmount = 0);
#pragma endregion

#pragma region Entity Allocation
			/// Returns whether the represented Entity subclass is concrete or not, that is if it can create new instances through NewInstance().
			/// @return Whether the represented Entity subclass is concrete or not.
			bool IsConcrete() const { return (m_Allocate != 0) ? true : false; }

			/// Dynamically allocates an instance of the Entity subclass that this ClassInfo represents. If the Entity isn't concrete, 0 will be returned.
			/// @return A pointer to the dynamically allocated Entity. Ownership is transferred. If the represented Entity subclass isn't concrete, 0 will be returned.
			virtual Entity* NewInstance() const { return IsConcrete() ? m_NewInstance() : 0; }
#pragma endregion

		protected:
			static ClassInfo* s_ClassHead; //!< Head of unordered linked list of ClassInfos in existence.

			const std::string m_Name; //!< A string with the friendly - formatted name of this ClassInfo.
			const ClassInfo* m_ParentInfo; //!< A pointer to the parent ClassInfo.

			MemoryAllocate m_Allocate; //!< Raw memory allocation for the size of the type this ClassInfo describes.
			MemoryDeallocate m_Deallocate; //!< Raw memory deallocation for the size of the type this ClassInfo describes.
			// TODO: figure out why this doesn't want to work when defined as std::function.
			Entity* (*m_NewInstance)(); //!< Returns an actual new instance of the type that this describes.

			ClassInfo* m_NextClass; //!< Next ClassInfo after this one on aforementioned unordered linked list.

			std::vector<void*> m_AllocatedPool; //!< Pool of pre-allocated objects of the type described by this ClassInfo.
			int m_PoolAllocBlockCount; //!< The number of instances to fill up the pool of this type with each time it runs dry.
			int m_InstancesInUse; //!< The number of allocated instances passed out from the pool.

			std::mutex m_Mutex; //!< Mutex to ensure multiple things aren't grabbing/deallocating memory at the same time

			// Forbidding copying
			ClassInfo(const ClassInfo& reference) = delete;
			ClassInfo& operator=(const ClassInfo& rhs) = delete;
		};
#pragma endregion

#pragma region Creation
		/// Constructor method used to instantiate a Entity in system memory. Create() should be called before using the Entity.
		Entity();

		/// Makes the Entity ready for use.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create() override;

		/// Creates an Entity to be identical to another, by deep copy.
		/// @param reference A reference to the Entity to deep copy.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		virtual int Create(const Entity& reference);

		/// Makes the Serializable ready for use.
		/// @param reader A Reader that the Serializable will create itself from.
		/// @param checkType Whether there is a class name in the stream to check against to make sure the correct type is being read from the stream.
		/// @param doCreate Whether to do any additional initialization of the object after reading in all the properties from the Reader. This is done by calling Create().
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(Reader& reader, bool checkType = true, bool doCreate = true) override { return Serializable::Create(reader, checkType, doCreate); }

		/// Uses a passed-in instance, or creates a new one, and makes it identical to this.
		/// @param cloneTo A pointer to an instance to make identical to this. If 0 is passed in, a new instance is made inside here, and ownership of it IS returned!
		/// @return An Entity pointer to the newly cloned-to instance. Ownership IS transferred!
		virtual Entity* Clone(Entity* cloneTo = nullptr) const {
			RTEAbort("Attempt to clone an abstract or unclonable type!");
			return nullptr;
		}
#pragma endregion

#pragma region Destruction
		/// Destructor method used to clean up a Entity before deletion from system memory.
		virtual ~Entity();

		/// Destroys and resets (through Clear()) the Entity.
		/// @param notInherited Whether to only destroy the members defined in this derived class, or to destroy all inherited members also.
		virtual void Destroy(bool notInherited = false) { Clear(); }

		/// Resets the entire Entity, including its inherited members, to their default settings or values.
		void Reset() override { Clear(); }
#pragma endregion

#pragma region INI Handling
		/// Only saves out a Preset reference of this to the stream.
		/// Is only applicable to objects that are not original presets and haven't been altered since they were copied from their original.
		/// @param writer A Writer that the Entity will save itself to.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int SavePresetCopy(Writer& writer) const;
#pragma endregion

#pragma region Getters and Setters
		/// Shows the ID of the DataModule this Entity has been defined in.
		/// @return The ID of the module, or -1 if it hasn't been defined in any.
		int GetModuleID() const { return m_DefinedInModule; }

		/// Sets the module this Entity was supposed to be defined in.
		/// @param whichModule The ID of the module, or -1 if it hasn't been defined in any.
		void SetModuleID(int whichModule) { m_DefinedInModule = whichModule; }

		/// Gets this Entity's data preset.
		/// @return This Entity's data preset.
		const Entity* GetPreset() const;

		/// Gets the name of this Entity's data Preset.
		/// @return A string reference with the instance name of this Entity.
		const std::string& GetPresetName() const { return m_PresetName; }

		/// Sets the name of this Entity's data Preset.
		/// @param newName A string reference with the instance name of this Entity.
		/// @param calledFromLua Whether this method was called from Lua, in which case this change is cosmetic only and shouldn't affect scripts.
		// TODO: Replace the calledFromLua flag with some DisplayName property
		// TODO: Figure out how to handle if same name was set, still make it wasgivenname = true?
		virtual void SetPresetName(const std::string& newName, bool calledFromLua = false) { /*if (m_PresetName != newName) { m_IsOriginalPreset = true; }*/
			m_IsOriginalPreset = calledFromLua ? m_IsOriginalPreset : true;
			m_PresetName = newName;
		}

		/// Gets the plain text description of this Entity's data Preset.
		/// @return A string reference with the plain text description name of this Preset.
		const std::string& GetDescription() const { return m_PresetDescription; }

		/// Sets the plain text description of this Entity's data Preset. Shouldn't be more than a couple of sentences.
		/// @param newDesc A string reference with the preset description.
		void SetDescription(const std::string& newDesc) { m_PresetDescription = newDesc; }

		/// Gets the name of this Entity's data Preset, preceded by the name of the Data Module it was defined in, separated with a '/'.
		/// @return A string with the module and instance name of this Entity.
		std::string GetModuleAndPresetName() const;

		/// Gets the name of this Entity's Data Module it was defined in.
		/// @return A string with the module of this Entity.
		std::string GetModuleName() const;

		/// Indicates whether this Entity was explicitly given a new instance name upon creation, or if it was just copied/inherited implicitly.
		/// @return Whether this Entity was given a new Preset Name upon creation.
		bool IsOriginalPreset() const { return m_IsOriginalPreset; }

		/// Sets IsOriginalPreset flag to indicate that the object should be saved as CopyOf.
		void ResetOriginalPresetFlag() { m_IsOriginalPreset = false; }
#pragma endregion

#pragma region Logging
		/// Gets the file and line that are currently being read. Formatted to be used for logging warnings and errors.
		/// @return A string containing the currently read file path and the line being read.
		const std::string& GetFormattedReaderPosition() const { return m_FormattedReaderPosition; }

		/// Sets the file and line that are currently being read. Formatted to be used for logging warnings and errors.
		/// @param newPosition A string containing the currently read file path and the line being read.
		void SetFormattedReaderPosition(const std::string& newPosition) override { m_FormattedReaderPosition = newPosition; }
#pragma endregion

#pragma region Virtual Override Methods
		/// Makes this an original Preset in a different module than it was before. It severs ties deeply to the old module it was saved in.
		/// @param whichModule The ID of the new module.
		/// @return Whether the migration was successful. If you tried to migrate to the same module it already was in, this would return false.
		virtual bool MigrateToModule(int whichModule);
#pragma endregion

#pragma region Groups
		/// Gets the set of groups this is member of.
		/// @return A pointer to a list of strings which describes the groups this is added to. Ownership is NOT transferred!
		const std::unordered_set<std::string>* GetGroups() const { return &m_Groups; }

		/// Gets whether this is part of a specific group or not.
		/// @param whichGroup A string which describes the group to check for.
		/// @return Whether this Entity is in the specified group or not.
		bool IsInGroup(const std::string& whichGroup) const { return whichGroup == "None" ? false : (whichGroup == "All" || whichGroup == "Any" || m_Groups.contains(whichGroup)); }

		/// Adds this Entity to a new grouping.
		/// @param newGroup A string which describes the group to add this to. Duplicates will be ignored.
		void AddToGroup(const std::string& newGroup) { m_Groups.emplace(newGroup); }

		/// Removes this Entity from the specified grouping.
		/// @param groupToRemoveFrom A string which describes the group to remove this from.
		void RemoveFromGroup(const std::string& groupToRemoveFrom) { m_Groups.erase(groupToRemoveFrom); }

		/// Returns random weight used in PresetMan::GetRandomBuyableOfGroupFromTech.
		/// @return This item's random weight from 0 to 100.
		int GetRandomWeight() const { return m_RandomWeight; }
#pragma endregion

#pragma region Lua Script Handling
		/// Reloads the Preset scripts of this Entity, from the same script file path as was originally defined.
		/// This will also update the original Preset in the PresetMan with the updated scripts so future objects spawned will use the new scripts.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		virtual int ReloadScripts() { return 0; }
#pragma endregion

#pragma region Operator Overloads
		/// A stream insertion operator for sending a Entity to an output stream.
		/// @param stream An ostream reference as the left hand side operand.
		/// @param operand A Entity reference as the right hand side operand.
		/// @return An ostream reference for further use in an expression.
		friend std::ostream& operator<<(std::ostream& stream, const Entity& operand) {
			stream << operand.GetPresetName() << ", " << operand.GetClassName();
			return stream;
		}

		/// A Reader extraction operator for filling an Entity from a Reader.
		/// @param reader A Reader reference as the left hand side operand.
		/// @param operand An Entity reference as the right hand side operand.
		/// @return A Reader reference for further use in an expression.
		friend Reader& operator>>(Reader& reader, Entity& operand);

		/// A Reader extraction operator for filling an Entity from a Reader.
		/// @param reader A Reader reference as the left hand side operand.
		/// @param operand An Entity pointer as the right hand side operand.
		/// @return A Reader reference for further use in an expression.
		friend Reader& operator>>(Reader& reader, Entity* operand);
#pragma endregion

#pragma region Class Info
		/// Gets the ClassInfo instance of this Entity.
		/// @return A reference to the ClassInfo of this' class.
		virtual const Entity::ClassInfo& GetClass() const { return m_sClass; }

		/// Gets the class name of this Entity.
		/// @return A string with the friendly-formatted type name of this Entity.
		virtual const std::string& GetClassName() const { return m_sClass.GetName(); }
#pragma endregion

	protected:
		static Entity::ClassInfo m_sClass; //!< Type description of this Entity.

		std::string m_PresetName; //!< The name of the Preset data this was cloned from, if any.
		std::string m_PresetDescription; //!< The description of the preset in user friendly plain text that will show up in menus etc.
		std::string m_FormattedReaderPosition; //!< A string containing the file path and the line we were read from. Formatted to be used for logging.

		bool m_IsOriginalPreset; //!< Whether this is to be added to the PresetMan as an original preset instance.
		int m_DefinedInModule; //!< The DataModule ID that this was successfully added to at some point. -1 if not added to anything yet.

		std::unordered_set<std::string> m_Groups; //!< List of all tags associated with this. The groups are used to categorize and organize Entities.

		int m_RandomWeight; //!< Random weight used when picking item using PresetMan::GetRandomBuyableOfGroupFromTech. From 0 to 100. 0 means item won't be ever picked.

		// Forbidding copying
		Entity(const Entity& reference) {}
		Entity& operator=(const Entity& rhs) { return *this; }

	private:
		/// Clears all the member variables of this Entity, effectively resetting the members of this abstraction level only.
		void Clear();
	};
} // namespace RTE
