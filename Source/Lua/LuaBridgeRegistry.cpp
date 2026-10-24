// Sets up the metatables of the classes registered with LuaBridge so that they behave the way they did with luabind.
//
// LuaBridge's own lookups are replaced, because Cortex Command's scripts rely on a few luabind behaviours that LuaBridge does differently:
//  - Every object can have values of its own set on it from Lua, like `self.Timer = Timer()`, that are separate for each object (or rather, each userdata), the way luabind's object tables were.
//    LuaBridge's extensible classes store such values on the class instead, which would share them between every object of the class.
//  - Methods registered more than once under the same name are overloads, including across base and derived classes, and calls pick the best matching overload the way luabind did.
//  - When a base and derived class both have a property of the same name, the base class's getter and setter win, like they did with luabind.
//  - Enum values are constants on the class table, and are inherited by derived classes.
//  - Lookups go to the object's own values first, then methods, then properties, and each is a single table lookup since everything is flattened into the class.
//
// Properties don't use LuaBridge's property support at all. Their getters and setters are PropertyAccessors, which __index and __newindex call directly with the object's pointer already
// adjusted for the class the property is on, so a property access costs no more than a table lookup and the call itself. Objects of registered classes also carry the list of classes
// they can be used as (ClassTypeInfo, which starts with LuaBridge's AncestryInfo), so passing them to a function that takes one of their base classes is quick too.

#include "LuaBridgeDefinitions.h"

#include <algorithm>
#include <climits>
#include <cstdarg>
#include <cstdio>
#include <map>
#include <unordered_map>

using namespace RTE;
using namespace RTE::LuaBinding;

namespace {
	// Light userdata keys are fixed values like LuaBridge's own, rather than the addresses of something, see RawGetP.
	const void* const c_InstanceTableMarkerKey = reinterpret_cast<void*>(0x1257ab); //!< Registry key of the metatable that marks a userdata environment as the object's own value table.
	const void* const c_ClassNameKey = reinterpret_cast<void*>(0xc1a55a); //!< Key the class name string is stored under in metatables, so the name in ClassTypeInfo stays valid.

	/// One entry in an overload dispatcher.
	struct DispatchEntry {
		int Arity; //!< How many arguments the overload takes.
		MatchFunction Matcher; //!< Scores how well arguments match the overload.
	};

	/// The overloads a dispatcher picks between. The closures that call them are in a table upvalue, in the same order.
	struct DispatchInfo {
		int Count; //!< How many overloads there are.
		int FirstArgumentIndex; //!< Stack index of the first argument that's matched, which is 2 for constructors called through the class table.
		DispatchEntry Entries[1]; //!< The overloads, actually Count long.
	};

	/// Calls the overload that best matches the arguments, like luabind did: the arity has to match exactly, the overload with the lowest total score wins, and a tie between the best is an error.
	/// Upvalues: 1 light userdata pointing to the DispatchInfo, 2 the table of overload closures, 3 the name for error messages.
	int DispatchOverload(lua_State* luaState) {
		const DispatchInfo* info = static_cast<const DispatchInfo*>(lua_touserdata(luaState, lua_upvalueindex(1)));
		const int top = lua_gettop(luaState);
		const int argumentCount = top - info->FirstArgumentIndex + 1;

		int bestOverload = -1;
		int bestScore = INT_MAX;
		bool ambiguous = false;
		for (int i = 0; i < info->Count; ++i) {
			const DispatchEntry& entry = info->Entries[i];
			if (entry.Arity != argumentCount) {
				continue;
			}
			int score = entry.Matcher(luaState, info->FirstArgumentIndex);
			if (score < 0) {
				continue;
			}
			if (score < bestScore) {
				bestScore = score;
				bestOverload = i;
				ambiguous = false;
			} else if (score == bestScore) {
				ambiguous = true;
			}
		}
		if (bestOverload < 0) {
			RaiseError(luaState, "No matching overload of %s found for the given %d argument(s)!", lua_tostring(luaState, lua_upvalueindex(3)), argumentCount);
		}
		if (ambiguous) {
			RaiseError(luaState, "Ambiguous call to %s, more than one overload matches the given arguments equally well!", lua_tostring(luaState, lua_upvalueindex(3)));
		}
		lua_rawgeti(luaState, lua_upvalueindex(2), bestOverload + 1);
		lua_insert(luaState, 1);
		lua_call(luaState, top, LUA_MULTRET);
		return lua_gettop(luaState);
	}

	/// Converts a numeric key to a string in place, like luabind's lookups did with lua_tostring.
	/// @return Whether the key is (now) a string that can be looked up.
	inline bool NormalizeKey(lua_State* luaState) {
		switch (lua_type(luaState, 2)) {
			case LUA_TSTRING:
				return true;
			case LUA_TNUMBER:
				lua_tostring(luaState, 2);
				return true;
			default:
				return false;
		}
	}

	/// Pushes the object's own value table if it has one, and returns whether it does. Pushes nothing if it doesn't.
	/// The table is the userdata's environment, which is marked as the object's own by its metatable. Upvalue 3 is that marker.
	inline bool PushInstanceTable(lua_State* luaState) {
		lua_getfenv(luaState, 1);
		// Objects that were never given values of their own still have the globals they were made with as their environment, which is the common case, so check for that first.
		if (lua_rawequal(luaState, -1, LUA_GLOBALSINDEX)) {
			lua_pop(luaState, 1);
			return false;
		}
		if (lua_getmetatable(luaState, -1)) {
			bool isInstanceTable = lua_rawequal(luaState, -1, lua_upvalueindex(3));
			lua_pop(luaState, 1);
			if (isInstanceTable) {
				return true;
			}
		}
		lua_pop(luaState, 1);
		return false;
	}

	/// Gets the pointer to the object at stack index 1 with the given offset added, raising an error if it's not an object.
	inline void* GetObjectPointer(lua_State* luaState, std::ptrdiff_t offset) {
		void* object = lua_type(luaState, 1) == LUA_TUSERDATA ? static_cast<luabridge::detail::Userdata*>(lua_touserdata(luaState, 1))->getPointer() : nullptr;
		if (!object) {
			RaiseError(luaState, "Tried to access a property of something that isn't an object!");
		}
		return static_cast<char*>(object) + offset;
	}

	/// __index of objects. Looks in the object's own values, then the class's methods, then its property getters, like luabind did.
	/// Upvalues: 1 methods table, 2 property getters table (of light userdata pointing to PropertyAccessors), 3 instance table marker, 4 the string "__ok".
	int InstanceIndex(lua_State* luaState) {
		if (!NormalizeKey(luaState)) {
			lua_pushnil(luaState);
			return 1;
		}
		if (lua_rawequal(luaState, 2, lua_upvalueindex(4))) {
			// luabind's special member for checking whether the object's pointer is valid.
			lua_pushboolean(luaState, static_cast<luabridge::detail::Userdata*>(lua_touserdata(luaState, 1))->getPointer() != nullptr);
			return 1;
		}
		if (PushInstanceTable(luaState)) {
			lua_pushvalue(luaState, 2);
			lua_rawget(luaState, -2);
			if (!lua_isnil(luaState, -1)) {
				return 1;
			}
			lua_pop(luaState, 2);
		}
		lua_pushvalue(luaState, 2);
		lua_rawget(luaState, lua_upvalueindex(1));
		if (!lua_isnil(luaState, -1)) {
			return 1;
		}
		lua_pop(luaState, 1);
		lua_pushvalue(luaState, 2);
		lua_rawget(luaState, lua_upvalueindex(2));
		if (!lua_isnil(luaState, -1)) {
			const PropertyAccessor* getter = static_cast<const PropertyAccessor*>(lua_touserdata(luaState, -1));
			return getter->Call(luaState, GetObjectPointer(luaState, getter->Offset), getter->Callable);
		}
		return 1;
	}

	/// __newindex of objects. Calls the property setter if there is one, raises an error for read only properties, and otherwise stores the value in the object's own value table.
	/// Upvalues: 1 property setters table, 2 property getters table (both of light userdata pointing to PropertyAccessors), 3 instance table marker, 4 class name.
	int InstanceNewIndex(lua_State* luaState) {
		if (!NormalizeKey(luaState)) {
			RaiseError(luaState, "Can't set a value with a %s key on a %s!", luaL_typename(luaState, 2), lua_tostring(luaState, lua_upvalueindex(4)));
		}
		lua_pushvalue(luaState, 2);
		lua_rawget(luaState, lua_upvalueindex(1));
		if (!lua_isnil(luaState, -1)) {
			const PropertyAccessor* setter = static_cast<const PropertyAccessor*>(lua_touserdata(luaState, -1));
			setter->Call(luaState, GetObjectPointer(luaState, setter->Offset), setter->Callable);
			return 0;
		}
		lua_pop(luaState, 1);
		lua_pushvalue(luaState, 2);
		lua_rawget(luaState, lua_upvalueindex(2));
		if (!lua_isnil(luaState, -1)) {
			RaiseError(luaState, "the attribute '%s.%s' is read only", lua_tostring(luaState, lua_upvalueindex(4)), lua_tostring(luaState, 2));
		}
		lua_pop(luaState, 1);
		if (!PushInstanceTable(luaState)) {
			lua_createtable(luaState, 0, 4);
			lua_pushvalue(luaState, lua_upvalueindex(3));
			lua_setmetatable(luaState, -2);
			lua_pushvalue(luaState, -1);
			lua_setfenv(luaState, 1);
		}
		lua_pushvalue(luaState, 2);
		lua_pushvalue(luaState, 3);
		lua_rawset(luaState, -3);
		return 0;
	}

	/// __index of class tables, for getting methods through the class, like Vector.Normalize. Constants are stored in the class table itself, so they never get here.
	/// Upvalues: 1 methods table.
	int StaticIndex(lua_State* luaState) {
		lua_pushvalue(luaState, 2);
		lua_rawget(luaState, lua_upvalueindex(1));
		return 1;
	}

	/// __newindex of class tables. Lets classes be extended from Lua: the value is stored in the class table, and functions also become methods of the class's objects, and of the objects of derived classes that don't have a method of that name already.
	/// Upvalues: 1 array of methods tables, with the class's own (non-const and const) ones first, followed by those of every derived class.
	int StaticNewIndex(lua_State* luaState) {
		lua_pushvalue(luaState, 2);
		lua_pushvalue(luaState, 3);
		lua_rawset(luaState, 1);

		if (lua_isfunction(luaState, 3)) {
			const int tableCount = static_cast<int>(lua_objlen(luaState, lua_upvalueindex(1)));
			for (int i = 1; i <= tableCount; ++i) {
				lua_rawgeti(luaState, lua_upvalueindex(1), i);
				bool isOwnTable = i <= 2;
				bool shouldSet = isOwnTable;
				if (!isOwnTable) {
					lua_pushvalue(luaState, 2);
					lua_rawget(luaState, -2);
					shouldSet = lua_isnil(luaState, -1);
					lua_pop(luaState, 1);
				}
				if (shouldSet) {
					lua_pushvalue(luaState, 2);
					lua_pushvalue(luaState, 3);
					lua_rawset(luaState, -3);
				}
				lua_pop(luaState, 1);
			}
		}
		return 0;
	}

	/// An overload being considered for a class's flattened method table, with how far up the class hierarchy it was registered.
	struct Candidate {
		const OverloadRecord* Record;
		int Depth;
	};

	/// Removes overloads that are hidden by an overload with exactly the same parameters registered on a class nearer to the object's class. Those could never be picked, since the object would always be a closer match for the nearer one.
	std::vector<const OverloadRecord*> RemoveHiddenOverloads(const std::vector<Candidate>& candidates) {
		std::vector<const OverloadRecord*> result;
		for (const Candidate& candidate: candidates) {
			bool hidden = false;
			for (const Candidate& other: candidates) {
				if (other.Depth < candidate.Depth && *other.Record->Signature == *candidate.Record->Signature && other.Record->ConstCallable == candidate.Record->ConstCallable && other.Record->Arity == candidate.Record->Arity) {
					hidden = true;
					break;
				}
			}
			if (!hidden) {
				result.push_back(candidate.Record);
			}
		}
		return result;
	}

	/// Pushes the function that calls a set of overloads: the overload's own closure if there's only one, otherwise a dispatcher that picks between them.
	void PushOverloadSet(lua_State* luaState, Storage& storage, const std::vector<const OverloadRecord*>& overloads, const std::string& name, int firstArgumentIndex) {
		if (overloads.size() == 1 && firstArgumentIndex == 1) {
			lua_rawgeti(luaState, LUA_REGISTRYINDEX, overloads.front()->ClosureReference);
			return;
		}
		const int count = static_cast<int>(overloads.size());
		DispatchInfo* info = new (storage.Allocate(sizeof(DispatchInfo) + sizeof(DispatchEntry) * (count - 1), alignof(DispatchInfo))) DispatchInfo;
		info->Count = count;
		info->FirstArgumentIndex = firstArgumentIndex;
		lua_pushlightuserdata(luaState, info);
		lua_createtable(luaState, count, 0);
		for (int i = 0; i < count; ++i) {
			info->Entries[i] = {overloads[i]->Arity, overloads[i]->Matcher};
			lua_rawgeti(luaState, LUA_REGISTRYINDEX, overloads[i]->ClosureReference);
			lua_rawseti(luaState, -2, i + 1);
		}
		lua_pushstring(luaState, name.c_str());
		lua_pushcclosure(luaState, &DispatchOverload, 3);
	}

	/// Sets the value at the top of the stack as a field of the table at the given index without invoking metamethods, popping the value.
	/// This is needed for LuaBridge's metatables, which are their own metatables, so lua_setfield on them would call their __newindex.
	void RawSetField(lua_State* luaState, int tableIndex, const char* name) {
		tableIndex = tableIndex < 0 ? lua_gettop(luaState) + tableIndex + 1 : tableIndex;
		lua_pushstring(luaState, name);
		lua_insert(luaState, -2);
		lua_rawset(luaState, tableIndex);
	}

	/// The stored copies of the property accessors that don't need a pointer offset, by the accessor in the registry's records they're copies of. Classes that inherit a property share these.
	using SharedAccessors = std::unordered_map<const PropertyAccessor*, const PropertyAccessor*>;

	/// Pushes a light userdata pointing to a stored property accessor, for a class whose objects need the given offset added to point to the class the property is registered on.
	/// Accessors without an offset are shared between all the classes that inherit them.
	void PushPropertyAccessor(lua_State* luaState, Storage& storage, SharedAccessors& sharedAccessors, const PropertyAccessor& accessor, std::ptrdiff_t offset) {
		const PropertyAccessor* stored;
		if (offset == 0) {
			const PropertyAccessor*& shared = sharedAccessors[&accessor];
			if (!shared) {
				shared = storage.Emplace<PropertyAccessor>(accessor);
			}
			stored = shared;
		} else {
			PropertyAccessor* copy = storage.Emplace<PropertyAccessor>(accessor);
			copy->Offset = offset;
			stored = copy;
		}
		lua_pushlightuserdata(luaState, const_cast<PropertyAccessor*>(stored));
	}
} // namespace

namespace RTE::LuaBinding {
	void* Storage::Allocate(std::size_t size, std::size_t alignment) {
		std::size_t offset = (m_LastBlockUsed + alignment - 1) / alignment * alignment;
		if (m_Blocks.empty() || offset + size > m_LastBlockSize) {
			m_LastBlockSize = std::max(size, c_BlockSize);
			m_Blocks.push_back(std::make_unique<std::byte[]>(m_LastBlockSize));
			offset = 0;
		}
		m_LastBlockUsed = offset + size;
		return m_Blocks.back().get() + offset;
	}

	void RaiseError(lua_State* luaState, const char* format, ...) {
		char message[1024];
		va_list arguments;
		va_start(arguments, format);
		std::vsnprintf(message, sizeof(message), format, arguments);
		va_end(arguments);
		lua_pushstring(luaState, message);
		lua_error(luaState);
		std::abort(); // lua_error never returns.
	}

	void Registry::Finalize() {
		lua_State* luaState = m_LuaState;
		const int stackTop = lua_gettop(luaState);
		// Stack checks are off for LuaBridge, so make sure there's room for everything below, which isn't called from inside a Lua C function that would guarantee some.
		if (!lua_checkstack(luaState, 64)) {
			RaiseError(luaState, "Not enough Lua stack space to finish registering the bindings!");
		}

		lua_newtable(luaState);
		const int instanceTableMarkerIndex = lua_gettop(luaState);
		lua_pushvalue(luaState, instanceTableMarkerIndex);
		RawSetP(luaState, LUA_REGISTRYINDEX, c_InstanceTableMarkerKey);
		lua_pushstring(luaState, "__ok");
		const int okKeyIndex = lua_gettop(luaState);

		const int classCount = static_cast<int>(m_Classes.size());

		// The flattened methods tables of every class, non-const then const, so class tables can be extended from Lua.
		lua_createtable(luaState, classCount * 2, 0);
		const int allMethodsTablesIndex = lua_gettop(luaState);

		SharedAccessors sharedAccessors;
		std::vector<std::vector<int>> descendants(classCount);

		for (int classIndex = 0; classIndex < classCount; ++classIndex) {
			const ClassRecord& record = m_Classes[classIndex];

			std::vector<int> chain;
			for (int i = classIndex; i >= 0; i = m_Classes[i].ParentIndex) {
				chain.push_back(i);
				if (i != classIndex) {
					descendants[i].push_back(classIndex);
				}
			}
			if (static_cast<int>(chain.size()) > luabridge::detail::AncestryInfo::MaxAncestors) {
				RaiseError(luaState, "Class %s has a deeper hierarchy than is supported!", record.Name.c_str());
			}

			RawGetP(luaState, LUA_REGISTRYINDEX, record.ClassKey);
			const int classMetatableIndex = lua_gettop(luaState);
			RawGetP(luaState, LUA_REGISTRYINDEX, record.ConstKey);
			const int constMetatableIndex = lua_gettop(luaState);
			// LuaBridge's static registry key is for the metatable of the class table, rather than the class table that's visible from Lua, which only the global namespace refers to.
			RawGetP(luaState, LUA_REGISTRYINDEX, record.StaticKey);
			const int staticMetatableIndex = lua_gettop(luaState);
			lua_getglobal(luaState, record.Name.c_str());
			const int staticTableIndex = lua_gettop(luaState);
			if (!lua_istable(luaState, staticTableIndex) || !lua_getmetatable(luaState, staticTableIndex)) {
				RaiseError(luaState, "Class %s isn't registered as a global!", record.Name.c_str());
			}
			if (!lua_rawequal(luaState, -1, staticMetatableIndex)) {
				RaiseError(luaState, "Global %s isn't the class table of the class registered under that name!", record.Name.c_str());
			}
			lua_pop(luaState, 1);

			// Type info, for overload matching and copying objects between Lua states.
			for (int metatableIndex: {classMetatableIndex, constMetatableIndex}) {
				lua_pushstring(luaState, record.Name.c_str());
				const char* persistentName = lua_tostring(luaState, -1);
				RawSetP(luaState, metatableIndex, c_ClassNameKey);

				ClassTypeInfo* info = m_Storage.Emplace<ClassTypeInfo>();
				info->Ancestry.Count = static_cast<int>(chain.size());
				std::ptrdiff_t offset = 0;
				for (int i = 0; i < info->Ancestry.Count; ++i) {
					info->Ancestry.ClassKeys[i] = m_Classes[chain[i]].ClassKey;
					info->Ancestry.Offsets[i] = offset;
					offset += m_Classes[chain[i]].ParentOffset;
				}
				info->Ancestry.IsConst = metatableIndex == constMetatableIndex;
				info->Name = persistentName;
				info->PushPointer = record.PushPointer;
				lua_pushlightuserdata(luaState, info);
				RawSetP(luaState, metatableIndex, GetClassTypeInfoKey());
			}

			// Methods and operators, with overloads merged across the whole hierarchy like luabind did.
			std::map<std::string, std::vector<Candidate>> methodCandidates;
			std::map<std::string, std::vector<Candidate>> constMethodCandidates;
			for (int depth = 0; depth < static_cast<int>(chain.size()); ++depth) {
				for (const OverloadRecord& method: m_Classes[chain[depth]].Methods) {
					methodCandidates[method.Name].push_back({&method, depth});
					if (method.ConstCallable) {
						constMethodCandidates[method.Name].push_back({&method, depth});
					}
				}
			}

			lua_newtable(luaState);
			const int methodsIndex = lua_gettop(luaState);
			lua_newtable(luaState);
			const int constMethodsIndex = lua_gettop(luaState);

			for (const auto& [name, candidates]: methodCandidates) {
				PushOverloadSet(luaState, m_Storage, RemoveHiddenOverloads(candidates), record.Name + "." + name, 1);
				if (name.rfind("__", 0) == 0) {
					// Operators have to be in the metatables themselves, and the same function in both, or Lua won't compare const and non-const objects with each other.
					lua_pushvalue(luaState, -1);
					RawSetField(luaState, classMetatableIndex, name.c_str());
					RawSetField(luaState, constMetatableIndex, name.c_str());
				} else {
					lua_setfield(luaState, methodsIndex, name.c_str());
				}
			}
			for (const auto& [name, candidates]: constMethodCandidates) {
				if (name.rfind("__", 0) != 0) {
					PushOverloadSet(luaState, m_Storage, RemoveHiddenOverloads(candidates), record.Name + "." + name, 1);
					lua_setfield(luaState, constMethodsIndex, name.c_str());
				}
			}
			lua_pushvalue(luaState, methodsIndex);
			lua_rawseti(luaState, allMethodsTablesIndex, classIndex * 2 + 1);
			lua_pushvalue(luaState, constMethodsIndex);
			lua_rawseti(luaState, allMethodsTablesIndex, classIndex * 2 + 2);

			// Property getters and setters, where the base class wins when there are ones of the same name, like luabind did.
			lua_newtable(luaState);
			const int gettersIndex = lua_gettop(luaState);
			lua_newtable(luaState);
			const int settersIndex = lua_gettop(luaState);
			std::ptrdiff_t ancestorOffset = 0;
			for (int chainIndex: chain) {
				for (const PropertyRecord& property: m_Classes[chainIndex].Properties) {
					PushPropertyAccessor(luaState, m_Storage, sharedAccessors, property.Getter, ancestorOffset);
					RawSetField(luaState, gettersIndex, property.Name.c_str());
					if (property.Setter.Call) {
						PushPropertyAccessor(luaState, m_Storage, sharedAccessors, property.Setter, ancestorOffset);
						RawSetField(luaState, settersIndex, property.Name.c_str());
					}
				}
				ancestorOffset += m_Classes[chainIndex].ParentOffset;
			}

			// The lookups for objects.
			for (bool isConst: {false, true}) {
				const int metatableIndex = isConst ? constMetatableIndex : classMetatableIndex;

				lua_pushvalue(luaState, isConst ? constMethodsIndex : methodsIndex);
				lua_pushvalue(luaState, gettersIndex);
				lua_pushvalue(luaState, instanceTableMarkerIndex);
				lua_pushvalue(luaState, okKeyIndex);
				lua_pushcclosure(luaState, &InstanceIndex, 4);
				RawSetField(luaState, metatableIndex, "__index");

				lua_pushvalue(luaState, settersIndex);
				lua_pushvalue(luaState, gettersIndex);
				lua_pushvalue(luaState, instanceTableMarkerIndex);
				lua_pushstring(luaState, record.Name.c_str());
				lua_pushcclosure(luaState, &InstanceNewIndex, 4);
				RawSetField(luaState, metatableIndex, "__newindex");
			}

			// Enum constants, stored in the class table itself, where the base class wins when there are ones of the same name, like luabind did.
			std::map<std::string, int> constants;
			for (int chainIndex: chain) {
				for (const auto& [name, value]: m_Classes[chainIndex].Constants) {
					constants[name] = value;
				}
			}
			for (const auto& [name, value]: constants) {
				lua_pushstring(luaState, name.c_str());
				lua_pushnumber(luaState, static_cast<lua_Number>(value));
				lua_rawset(luaState, staticTableIndex);
			}

			// The class table's lookups, and its constructors.
			lua_pushvalue(luaState, methodsIndex);
			lua_pushcclosure(luaState, &StaticIndex, 1);
			RawSetField(luaState, staticMetatableIndex, "__index");
			if (!record.Constructors.empty()) {
				std::vector<const OverloadRecord*> constructors;
				for (const OverloadRecord& constructor: record.Constructors) {
					constructors.push_back(&constructor);
				}
				PushOverloadSet(luaState, m_Storage, constructors, record.Name, 2);
				RawSetField(luaState, staticMetatableIndex, "__call");
			}

			lua_settop(luaState, allMethodsTablesIndex);
		}

		// Class tables can only be extended once all the derived classes' method tables exist.
		for (int classIndex = 0; classIndex < classCount; ++classIndex) {
			RawGetP(luaState, LUA_REGISTRYINDEX, m_Classes[classIndex].StaticKey);
			lua_createtable(luaState, 2 + static_cast<int>(descendants[classIndex].size()) * 2, 0);
			int tableCount = 0;
			for (int tableClassIndex: {classIndex}) {
				lua_rawgeti(luaState, allMethodsTablesIndex, tableClassIndex * 2 + 1);
				lua_rawseti(luaState, -2, ++tableCount);
				lua_rawgeti(luaState, allMethodsTablesIndex, tableClassIndex * 2 + 2);
				lua_rawseti(luaState, -2, ++tableCount);
			}
			for (int descendantIndex: descendants[classIndex]) {
				lua_rawgeti(luaState, allMethodsTablesIndex, descendantIndex * 2 + 1);
				lua_rawseti(luaState, -2, ++tableCount);
				lua_rawgeti(luaState, allMethodsTablesIndex, descendantIndex * 2 + 2);
				lua_rawseti(luaState, -2, ++tableCount);
			}
			lua_pushcclosure(luaState, &StaticNewIndex, 1);
			RawSetField(luaState, -2, "__newindex");
			lua_pop(luaState, 1);
		}

		// Global functions.
		std::map<std::string, std::vector<const OverloadRecord*>> functions;
		for (const OverloadRecord& function: m_Functions) {
			functions[function.Name].push_back(&function);
		}
		for (const auto& [name, overloads]: functions) {
			PushOverloadSet(luaState, m_Storage, overloads, name, 1);
			lua_setglobal(luaState, name.c_str());
		}

		lua_settop(luaState, stackTop);

		// Everything that needs the overload closures holds onto them by now, so the registry doesn't need to anymore.
		for (ClassRecord& record: m_Classes) {
			for (OverloadRecord& method: record.Methods) {
				luaL_unref(luaState, LUA_REGISTRYINDEX, method.ClosureReference);
			}
			for (OverloadRecord& constructor: record.Constructors) {
				luaL_unref(luaState, LUA_REGISTRYINDEX, constructor.ClosureReference);
			}
		}
		for (OverloadRecord& function: m_Functions) {
			luaL_unref(luaState, LUA_REGISTRYINDEX, function.ClosureReference);
		}
		m_Classes.clear();
		m_ClassIndices.clear();
		m_Functions.clear();
	}
} // namespace RTE::LuaBinding
