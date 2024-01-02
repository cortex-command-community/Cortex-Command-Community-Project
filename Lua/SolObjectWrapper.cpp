// Make sure that this wrapper file is always set to NOT use pre-compiled headers and conformance mode (/permissive) otherwise everything will be on fire cause luabind is a nightmare!

#include "SolObjectWrapper.h"

#include "sol/sol.hpp"

#include "LuaBindingRegisterDefinitions.h"

namespace RTE {

// With multithreaded Lua, objects can be destructed from multiple threads at once
// This is okay, but Sol wants to do some management on the lua state when one of it's objects is deleted
// This means that potentially an object being deleted by one lua state actually exists in another lua state
// And upon deletion, it's unsafe for Sol to poke at the state until we're out the multithreaded context
// As such, we don't actually delete the object until we're in a safe environment outside the multithreaded parts
// Note - this is required even though we force objects in multithreaded environments to be within our Sol state
// This is because we may assign an object to another state in a singlethreaded context, before the GC runs in the multithreaded context
static std::vector<sol::object *> s_QueuedDeletions;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SolObjectWrapper::ApplyQueuedDeletions() {
	for (sol::object *obj : s_QueuedDeletions) {
		delete obj;
	}

	s_QueuedDeletions.clear();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SolObjectWrapper::~SolObjectWrapper() {
	if (m_OwnsObject) {
		static std::mutex mut;
		std::lock_guard<std::mutex> guard(mut);

		s_QueuedDeletions.push_back(m_SolObject);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

sol::object GetCopyForStateInternal(const sol::object& obj, lua_State& targetState) {
	// Sol TODO - make this work!
	/*if (obj.is_valid()) {
		int type = luabind::type(obj);
		if (type == LUA_TNUMBER) {
			return luabind::adl::object(&targetState, luabind::object_cast<double>(obj));
		}
		else if (type == LUA_TBOOLEAN) {
			return luabind::adl::object(&targetState, luabind::object_cast<bool>(obj));
		}
		else if (type == LUA_TSTRING) {
			return luabind::adl::object(&targetState, luabind::object_cast<std::string>(obj));
		}
		else if (type == LUA_TTABLE) {
			luabind::object table = luabind::newtable(&targetState);
			for (luabind::iterator itr(obj), itrEnd; itr != itrEnd; ++itr) {
				table[GetCopyForStateInternal(itr.key(), targetState)] = GetCopyForStateInternal(*itr, targetState);
			}
			return table;
		}
		else if (type == LUA_TUSERDATA) {
#define PER_LUA_BINDING(Type)																		\
			if (boost::optional<Type*> boundObject = luabind::object_cast_nothrow<Type*>(obj)) {	\
				return luabind::adl::object(&targetState, boundObject.get());						\
			}

			LIST_OF_LUABOUND_OBJECTS
#undef PER_LUA_BINDING
		}
	}*/

	return sol::lua_nil;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SolObjectWrapper SolObjectWrapper::GetCopyForState(lua_State& targetState) const {
	sol::object* copy = new sol::object(GetCopyForStateInternal(*m_SolObject, targetState));
	return SolObjectWrapper(copy, m_FilePath, true);
}

}