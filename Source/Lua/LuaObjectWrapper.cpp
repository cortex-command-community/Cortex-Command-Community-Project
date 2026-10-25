#include "LuaObjectWrapper.h"

#include "LuaBridgeDefinitions.h"

#include <mutex>

using namespace RTE;

// With multithreaded Lua, objects can be destructed from multiple threads at once.
// Releasing a Lua reference pokes at the Lua state it belongs to, and the object being deleted by one thread might belong to a Lua state that another thread is running.
// As such, we don't actually delete the object until we're in a safe environment outside the multithreaded parts.
// Note - this is required even though we force objects in multithreaded environments to be within our Lua state.
// This is because we may assign an object to another state in a singlethreaded context, before the GC runs in the multithreaded context.
static std::vector<luabridge::LuaRef*> s_QueuedDeletions;
static std::mutex s_QueuedDeletionsMutex;

void LuaObjectWrapper::ApplyQueuedDeletions() {
	std::vector<luabridge::LuaRef*> queuedDeletions;
	{
		std::lock_guard<std::mutex> guard(s_QueuedDeletionsMutex);
		queuedDeletions.swap(s_QueuedDeletions);
	}
	for (luabridge::LuaRef* luaObject: queuedDeletions) {
		delete luaObject;
	}
}

LuaObjectWrapper::~LuaObjectWrapper() {
	if (m_OwnsObject) {
		std::lock_guard<std::mutex> guard(s_QueuedDeletionsMutex);
		s_QueuedDeletions.push_back(m_LuaObject);
	}
}

/// Pushes a copy of the value at the given index of one Lua state onto another Lua state.
/// Numbers, booleans and strings are copied, tables are copied deeply, objects of registered classes are pushed as pointers to the same object, and anything else becomes nil.
static void PushCopyForState(lua_State* sourceState, int sourceIndex, lua_State* targetState) {
	sourceIndex = sourceIndex < 0 ? lua_gettop(sourceState) + sourceIndex + 1 : sourceIndex;
	switch (lua_type(sourceState, sourceIndex)) {
		case LUA_TNUMBER:
			lua_pushnumber(targetState, lua_tonumber(sourceState, sourceIndex));
			return;
		case LUA_TBOOLEAN:
			lua_pushboolean(targetState, lua_toboolean(sourceState, sourceIndex));
			return;
		case LUA_TSTRING: {
			size_t length = 0;
			const char* string = lua_tolstring(sourceState, sourceIndex, &length);
			lua_pushlstring(targetState, string, length);
			return;
		}
		case LUA_TTABLE: {
			lua_newtable(targetState);
			const int targetTableIndex = lua_gettop(targetState);
			lua_pushnil(sourceState);
			while (lua_next(sourceState, sourceIndex) != 0) {
				PushCopyForState(sourceState, -2, targetState);
				PushCopyForState(sourceState, -1, targetState);
				lua_settable(targetState, targetTableIndex);
				lua_pop(sourceState, 1);
			}
			return;
		}
		case LUA_TUSERDATA: {
			// Like luabind's copy did, objects become non-owned pointers to the same object, and const objects can't be copied.
			const LuaBinding::ClassTypeInfo* info = LuaBinding::GetClassTypeInfo(sourceState, sourceIndex);
			if (info && !info->IsConst() && info->PushPointer) {
				void* object = static_cast<luabridge::detail::Userdata*>(lua_touserdata(sourceState, sourceIndex))->getPointer();
				if (object) {
					info->PushPointer(targetState, object);
					return;
				}
			}
			break;
		}
		default:
			break;
	}
	lua_pushnil(targetState);
}

LuaObjectWrapper LuaObjectWrapper::GetCopyForState(lua_State& targetState) const {
	luabridge::LuaRef* copy = nullptr;
	if (m_LuaObject && m_LuaObject->isValid()) {
		lua_State* sourceState = m_LuaObject->state();
		m_LuaObject->push(sourceState);
		PushCopyForState(sourceState, -1, &targetState);
		lua_pop(sourceState, 1);
		copy = new luabridge::LuaRef(luabridge::LuaRef::fromStack(&targetState));
	} else {
		copy = new luabridge::LuaRef(&targetState);
	}
	return LuaObjectWrapper(copy, m_FilePath, true);
}
