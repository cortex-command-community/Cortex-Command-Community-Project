#ifndef _RTELUABINDDEFINITIONS_
#define _RTELUABINDDEFINITIONS_

#include "lua.hpp"

#include "sol/sol.hpp"

namespace RTE {
	/// <summary>
	/// Special callback function for adding file name and line number to error messages when calling functions incorrectly.
	/// </summary>
	/// <param name="luaState">The Lua master state.</param>
	/// <returns>An error signal, 1, so Lua correctly reports that there's been an error.</returns>
	static int AddFileAndLineToError(lua_State *luaState) {
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

	/// <summary>
	/// Converts a Lua table to a C++ vector of the specified type. Ownership of the objects in the Lua table is transferred!
	/// </summary>
	/// <param name="luaTable">The Lua table object to convert to vector.</param>
	/// <returns>A C++ vector containing all the objects from the Lua table. Ownership is transferred!</returns>
	/// <remarks>In case of type mismatch (by specifying wrong type or a mix of types in the Lua table) object_cast will print an error to the console and throw, so no need to check what it returns before emplacing.</remarks>
	template <typename Type> static std::vector<Type> ConvertLuaTableToVectorOfType(const sol::table &luaObject) {
		std::vector<Type> outVector = {};
		if (luaObject.valid()) {
			for (auto object : luaObject.pairs()) {
				outVector.emplace_back(object.second.as<Type>()); //, luabind::adopt(luabind::result)));
			}
		}
		return outVector;
	}
}
#endif
