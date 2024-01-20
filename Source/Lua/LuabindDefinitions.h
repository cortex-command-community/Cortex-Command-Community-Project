#ifndef _RTELUABINDDEFINITIONS_
#define _RTELUABINDDEFINITIONS_

#define BOOST_BIND_GLOBAL_PLACEHOLDERS 1

// Todo - figure out luabind error callbacks and blah-de-blah
// #define LUABIND_NO_EXCEPTIONS

#include "lua.hpp"
#include "luabind.hpp"
#include "luabind/object.hpp"
#include "luabind/operator.hpp"
#include "luabind/copy_policy.hpp"
#include "luabind/adopt_policy.hpp"
#include "luabind/out_value_policy.hpp"
#include "luabind/iterator_policy.hpp"
#include "luabind/return_reference_to_policy.hpp"

namespace luabind {
	/// Function that extracts the raw pointer from the smart pointer. This is needed when Lua calls member functions on held types, the 'this' pointer must be a raw pointer, it is also needed to allow the smart_pointer to raw_pointer conversion from Lua to C++.
	/// @param ptr The smart pointer to get raw pointer for.
	/// @return Raw pointer of the passed in smart pointer.
	template <class Type> Type* get_pointer(boost::shared_ptr<Type>& ptr) {
		return ptr.get();
	}

	/// Can't have global enums in the master state so we use this dummy struct as a class and register the enums under it.
	struct enum_wrapper {};
} // namespace luabind

namespace RTE {
	/// Derived structs for each of the input enums because we can't register enum_wrapper multiple times under a different name.
	/// We're doing this so we can access each enum separately by name rather than having all of them accessed from a shared name.
	/// If this proves to be a hassle then we can easily revert to the shared name access by registering everything under enum_wrapper.
	struct input_device : public luabind::enum_wrapper {};
	struct input_elements : public luabind::enum_wrapper {};
	struct mouse_buttons : public luabind::enum_wrapper {};
	struct joy_buttons : public luabind::enum_wrapper {};
	struct joy_directions : public luabind::enum_wrapper {};
	struct key_codes : public luabind::enum_wrapper {};
	struct scan_codes : public luabind::enum_wrapper {};
	struct gamepad_buttons : public luabind::enum_wrapper {};
	struct gamepad_axis : public luabind::enum_wrapper {};
	struct directions : public luabind::enum_wrapper {};
	struct blend_modes : public luabind::enum_wrapper {};

	/// Special callback function for adding file name and line number to error messages when calling functions incorrectly.
	/// @param luaState The Lua master state.
	/// @return An error signal, 1, so Lua correctly reports that there's been an error.
	static int AddFileAndLineToError(lua_State* luaState) {
		lua_Debug luaDebug;
		if (lua_getstack(luaState, 2, &luaDebug) > 0) {
			lua_getinfo(luaState, "Sln", &luaDebug);
			std::string errorString = lua_tostring(luaState, -1);

			if (errorString.find(".lua") != std::string::npos) {
				lua_pushstring(luaState, errorString.c_str());
			} else {
				std::stringstream messageStream;
				messageStream << ((luaDebug.name == nullptr || strstr(luaDebug.name, ".rte") == nullptr) ? luaDebug.short_src : luaDebug.name);
				messageStream << ":" << luaDebug.currentline << ": " << errorString;
				lua_pushstring(luaState, messageStream.str().c_str());
			}
		}
		return 1;
	}

	/// Converts a Lua table to a C++ vector of the specified type. Ownership of the objects in the Lua table is transferred!
	/// @param luaTable The Lua table object to convert to vector.
	/// @return A C++ vector containing all the objects from the Lua table. Ownership is transferred!
	/// @remark In case of type mismatch (by specifying wrong type or a mix of types in the Lua table) object_cast will print an error to the console and throw, so no need to check what it returns before emplacing.
	template <typename Type> static std::vector<Type> ConvertLuaTableToVectorOfType(const luabind::object& luaObject) {
		std::vector<Type> outVector = {};
		if (luaObject.is_valid() && luabind::type(luaObject) == LUA_TTABLE) {
			for (luabind::iterator tableItr(luaObject), tableEnd; tableItr != tableEnd; ++tableItr) {
				outVector.emplace_back(luabind::object_cast<Type>(*tableItr, luabind::adopt(luabind::result)));
			}
		}
		return outVector;
	}
} // namespace RTE
#endif
