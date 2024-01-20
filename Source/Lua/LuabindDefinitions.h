#ifndef _RTELUABINDDEFINITIONS_
#define _RTELUABINDDEFINITIONS_

#include "lua.hpp"

#include "sol/sol.hpp"

namespace RTE {
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
	template <typename Type> static std::vector<Type> ConvertLuaTableToVectorOfType(const sol::table& luaObject) {
		std::vector<Type> outVector = {};
		if (luaObject.valid()) {
			for (auto object: luaObject.pairs()) {
				outVector.emplace_back(object.second.as<Type>()); //, luabind::adopt(luabind::result)));
			}
		}
		return outVector;
	}
} // namespace RTE
#endif
