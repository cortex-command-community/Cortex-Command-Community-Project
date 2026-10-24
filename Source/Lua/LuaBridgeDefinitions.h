#pragma once

// LuaBridge must only ever be included through this header, so that every translation unit sees LuaBridge with the same configuration.
// Stack checks are turned off because Lua always guarantees LUA_MINSTACK free slots to C functions, which is all our bindings ever use.
#define LUABRIDGE_SAFE_STACK_CHECKS 0
// Lua numbers are truncated when passed to integer parameters, the same way luabind did it, rather than rejected when they have a fractional part.
#define LUABRIDGE_LUABIND_COMPATIBLE_NUMBERS 1

#include "lua.hpp"
#include "LuaBridge/LuaBridge.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <typeinfo>
#include <unordered_map>
#include <utility>
#include <vector>

namespace RTE {
	class Entity;
	class GraphicalPrimitive;
} // namespace RTE

#pragma region Enum Stack Specialization
namespace luabridge {
	/// Enums go to and from Lua as plain numbers, like they did with luabind. Any Lua number (or numeric string) is accepted and truncated, without any range checks.
	template <class T>
	struct Stack<T, std::enable_if_t<std::is_enum_v<T>>> {
		[[nodiscard]] static Result push(lua_State* luaState, T value) {
			lua_pushnumber(luaState, static_cast<lua_Number>(static_cast<std::underlying_type_t<T>>(value)));
			return {};
		}

		[[nodiscard]] static TypeResult<T> get(lua_State* luaState, int index) {
			if (!lua_isnumber(luaState, index)) {
				return makeErrorCode(ErrorCode::InvalidTypeCast);
			}
			return static_cast<T>(static_cast<int>(lua_tonumber(luaState, index)));
		}

		[[nodiscard]] static bool isInstance(lua_State* luaState, int index) { return lua_isnumber(luaState, index) != 0; }
	};
} // namespace luabridge
#pragma endregion

namespace RTE::LuaBinding {

#pragma region Raw Lua Helpers
	/// Pushes the value stored under a light userdata key in the table at the given index. Lua 5.1 has no lua_rawgetp, and LuaBridge's version checks the stack on every call.
	inline void RawGetP(lua_State* luaState, int tableIndex, const void* key) {
		tableIndex = tableIndex < 0 && tableIndex > LUA_REGISTRYINDEX ? lua_gettop(luaState) + tableIndex + 1 : tableIndex;
		lua_pushlightuserdata(luaState, const_cast<void*>(key));
		lua_rawget(luaState, tableIndex);
	}

	/// Sets the value at the top of the stack under a light userdata key in the table at the given index, popping the value.
	inline void RawSetP(lua_State* luaState, int tableIndex, const void* key) {
		tableIndex = tableIndex < 0 && tableIndex > LUA_REGISTRYINDEX ? lua_gettop(luaState) + tableIndex + 1 : tableIndex;
		lua_pushlightuserdata(luaState, const_cast<void*>(key));
		lua_insert(luaState, -2);
		lua_rawset(luaState, tableIndex);
	}

	/// Raises a Lua error with a printf style message. Never returns.
	[[noreturn]] void RaiseError(lua_State* luaState, const char* format, ...);
#pragma endregion

#pragma region Class Type Info
	/// Function that pushes a raw, non-owned pointer to an object of a registered class, given as a void pointer to the exact registered type, into a Lua state.
	using PushPointerFunction = void (*)(lua_State* luaState, void* object);

	/// Information about a registered class that's stored in each of its metatables, used for working out overload scores, casting objects to their base classes quickly, and copying objects between Lua states.
	/// It's plain data inside a userdata, so it lives exactly as long as the metatable it's stored in without needing a finalizer.
	/// It starts with LuaBridge's AncestryInfo, which LuaBridge reads to cast objects to their base classes, so it has to stay standard layout.
	struct ClassTypeInfo {
		luabridge::detail::AncestryInfo Ancestry; //!< The class registry keys of the class and its bases, nearest first, so the index in there is the luabind cast distance, with the pointer offsets to them. Also says whether the objects are const.
		const char* Name; //!< The class name, as registered in Lua.
		PushPointerFunction PushPointer; //!< Pushes a non-owned pointer to an object of this class into another Lua state, or null if it's not a pointer-pushable type.

		/// Gets whether objects with this metatable are const.
		bool IsConst() const { return Ancestry.IsConst; }
	};
	static_assert(std::is_standard_layout_v<ClassTypeInfo>, "LuaBridge reads ClassTypeInfo as the AncestryInfo it starts with!");

	/// Gets the key ClassTypeInfo userdata are stored under in metatables.
	inline const void* GetClassTypeInfoKey() {
		return luabridge::detail::getAncestryInfoKey();
	}

	/// Gets the ClassTypeInfo of the userdata at the given stack index, or null if it isn't an object of a registered class.
	inline const ClassTypeInfo* GetClassTypeInfo(lua_State* luaState, int index) {
		if (lua_type(luaState, index) != LUA_TUSERDATA || !lua_getmetatable(luaState, index)) {
			return nullptr;
		}
		RawGetP(luaState, -1, GetClassTypeInfoKey());
		const ClassTypeInfo* info = static_cast<const ClassTypeInfo*>(lua_touserdata(luaState, -1));
		lua_pop(luaState, 2);
		return info;
	}

	/// Works out how many inheritance steps away the registered class identified by its class registry key is from the class of the object at the given stack index.
	/// @return The number of steps, or -1 if the object isn't of that class or a class derived from it.
	inline int GetClassDistance(const ClassTypeInfo* info, const void* classKey) {
		if (info) {
			for (int i = 0; i < info->Ancestry.Count; ++i) {
				if (info->Ancestry.ClassKeys[i] == classKey) {
					return i;
				}
			}
		}
		return -1;
	}

	/// Gets how many bytes have to be added to a pointer to an object of class T to get a pointer to its Base, which is only ever not 0 with multiple inheritance.
	template <class T, class Base>
	std::ptrdiff_t GetBaseOffset() {
		static_assert(std::is_base_of_v<Base, T>, "Base has to be a base class of T!");
		// Any non-null, suitably aligned address works for this, since casting to a non-virtual base only adds a fixed offset without reading anything.
		constexpr std::uintptr_t probeAddress = 0x10000;
		return static_cast<std::ptrdiff_t>(reinterpret_cast<std::uintptr_t>(static_cast<Base*>(reinterpret_cast<T*>(probeAddress))) - probeAddress);
	}
#pragma endregion

#pragma region Owned Objects
	/// Userdata for objects that Lua owns, and deletes when they're garbage collected, unless C++ takes the ownership over first.
	/// These are what luabind called adopted objects. Unlike LuaBridge's own value userdata, the object lives on the heap, so C++ can take it over and keep using the same object.
	class OwnedUserdataBase : public luabridge::detail::Userdata {
	public:
		/// Gets whether Lua still owns the object.
		bool IsOwned() const { return m_Owned; }

		/// Gives up Lua's ownership of the object, so it won't be deleted when this userdata is garbage collected.
		void ReleaseOwnership() { m_Owned = false; }

	protected:
		bool m_Owned = true; //!< Whether Lua owns the object.
	};

	/// Owned userdata for a specific class.
	template <class T>
	class OwnedUserdata final : public OwnedUserdataBase {
		static_assert(!std::is_const_v<T>, "Lua can only own non-const objects.");

	public:
		~OwnedUserdata() override {
			if (m_Owned && m_p) {
				delete static_cast<T*>(m_p);
			}
		}

		/// Pushes an object onto the Lua stack, giving Lua the ownership of it. Pushes nil if the object is null.
		static void Push(lua_State* luaState, T* object) {
			if (!object) {
				lua_pushnil(luaState);
				return;
			}
			OwnedUserdata<T>* userdata = new (lua_newuserdata(luaState, sizeof(OwnedUserdata<T>))) OwnedUserdata<T>(object);
			RawGetP(luaState, LUA_REGISTRYINDEX, luabridge::detail::getClassRegistryKey<T>());
			if (!lua_istable(luaState, -1)) {
				lua_pop(luaState, 1);
				userdata->m_Owned = false;
				userdata->~OwnedUserdata<T>();
				RaiseError(luaState, "Tried to push an object of an unregistered class to Lua!");
			}
			lua_setmetatable(luaState, -2);
		}

	private:
		explicit OwnedUserdata(T* object) { m_p = object; }
	};

	/// Gets the owned userdata at the given stack index, or null if the value there isn't an owned object.
	inline OwnedUserdataBase* GetOwnedUserdata(lua_State* luaState, int index) {
		if (lua_type(luaState, index) != LUA_TUSERDATA || !GetClassTypeInfo(luaState, index)) {
			return nullptr;
		}
		return dynamic_cast<OwnedUserdataBase*>(static_cast<luabridge::detail::Userdata*>(lua_touserdata(luaState, index)));
	}

	/// Whether objects of a class constructed from Lua are put on the heap as owned objects, so C++ can take them over later, rather than being stored inside their userdata.
	/// Everything that C++ can take ownership of, i.e. Entities and the primitives that PrimitiveMan schedules, has to be.
	template <class T>
	inline constexpr bool c_ConstructOnHeap = std::is_base_of_v<Entity, T> || std::is_base_of_v<GraphicalPrimitive, T>;
#pragma endregion

#pragma region Policy Types
	/// Return type wrapper for pointers whose ownership is handed to Lua (luabind's adopt(result)).
	template <class T>
	struct Owned {
		T* Object;
	};

	/// Parameter type wrapper for pointers whose ownership is taken away from Lua (luabind's adopt(_N)). Only objects that Lua owns can be passed.
	template <class T>
	struct Adopted {
		T* Object;
	};

	/// Return type for functions that return the object they were called on, i.e. the very same userdata (luabind's return_reference_to(_1)).
	struct SelfReference {};

	/// Return type for an iterator over a container that's owned by C++ (luabind's return_stl_iterator). The iterator function is what's returned to Lua.
	template <class Container>
	struct StlIterator {
		const Container* TheContainer;
	};

	/// Return type for an iterator over a heap allocated container that the iterator takes ownership of, and deletes once it's garbage collected.
	template <class Container>
	struct OwnedStlIterator {
		Container* TheContainer;
	};

	/// Policy tags, given after the function or property getter when registering it. They work like the luabind policies of the same names did.
	struct AdoptResultPolicy {};
	struct ReturnStlIteratorPolicy {};
	struct ReturnSelfPolicy {};
	template <int N> struct AdoptArgPolicy {};

	inline constexpr AdoptResultPolicy AdoptResult{}; //!< Lua takes ownership of the returned pointer. luabind's adopt(result)/adopt(return_value).
	inline constexpr ReturnStlIteratorPolicy ReturnStlIterator{}; //!< The returned container is returned as an iterator function. luabind's return_stl_iterator.
	inline constexpr ReturnSelfPolicy ReturnSelf{}; //!< The function returns the object it was called on. luabind's return_reference_to(_1).
	/// Lua gives up ownership of the Nth argument. Arguments are counted the way luabind did, so the object a member function is called on is 1 and the first actual argument is 2. luabind's adopt(_N).
	template <int N> inline constexpr AdoptArgPolicy<N> AdoptArg{};
#pragma endregion

#pragma region Iterator Implementation
	/// Pushes an element of a container the way luabind's iterators did: pointers as pointers, and everything else as a copy.
	template <class Element>
	void PushIteratorElement(lua_State* luaState, const Element& element) {
		if (auto result = luabridge::Stack<Element>::push(luaState, element); !result) {
			RaiseError(luaState, "%s", result.error_cstr());
		}
	}

	/// The state of an iterator over a container, stored in a userdata that's the upvalue of the iterator function.
	template <class Container>
	struct StlIteratorState {
		using Iterator = typename Container::const_iterator;

		Container* OwnedContainer; //!< The container, if the iterator owns it, otherwise null.
		Iterator Current; //!< The next element to return.
		Iterator End; //!< The end of the container.

		static int Next(lua_State* luaState) {
			StlIteratorState* state = static_cast<StlIteratorState*>(lua_touserdata(luaState, lua_upvalueindex(1)));
			if (state->Current == state->End) {
				lua_pushnil(luaState);
			} else {
				PushIteratorElement(luaState, *state->Current);
				++state->Current;
			}
			return 1;
		}

		static int GarbageCollect(lua_State* luaState) {
			static_cast<StlIteratorState*>(lua_touserdata(luaState, 1))->~StlIteratorState();
			return 0;
		}

		~StlIteratorState() { delete OwnedContainer; }

		/// Pushes an iterator function over the container.
		static void Push(lua_State* luaState, const Container* container, Container* ownedContainer) {
			if (!container) {
				lua_pushnil(luaState);
				return;
			}
			new (lua_newuserdata(luaState, sizeof(StlIteratorState))) StlIteratorState{ownedContainer, container->begin(), container->end()};

			static const char c_MetatableKey = 0;
			RawGetP(luaState, LUA_REGISTRYINDEX, &c_MetatableKey);
			if (lua_isnil(luaState, -1)) {
				lua_pop(luaState, 1);
				lua_createtable(luaState, 0, 1);
				lua_pushcfunction(luaState, &StlIteratorState::GarbageCollect);
				lua_setfield(luaState, -2, "__gc");
				lua_pushvalue(luaState, -1);
				RawSetP(luaState, LUA_REGISTRYINDEX, &c_MetatableKey);
			}
			lua_setmetatable(luaState, -2);
			lua_pushcclosure(luaState, &StlIteratorState::Next, 1);
		}
	};
#pragma endregion

} // namespace RTE::LuaBinding

#pragma region Policy Type Stack Specializations
namespace luabridge {
	template <class T>
	struct Stack<RTE::LuaBinding::Owned<T>> {
		[[nodiscard]] static Result push(lua_State* luaState, const RTE::LuaBinding::Owned<T>& value) {
			RTE::LuaBinding::OwnedUserdata<T>::Push(luaState, value.Object);
			return {};
		}
	};

	template <class T>
	struct Stack<RTE::LuaBinding::Adopted<T>> {
		[[nodiscard]] static TypeResult<RTE::LuaBinding::Adopted<T>> get(lua_State* luaState, int index) {
			RTE::LuaBinding::OwnedUserdataBase* owned = RTE::LuaBinding::GetOwnedUserdata(luaState, index);
			if (!owned || !owned->IsOwned()) {
				RTE::LuaBinding::RaiseError(luaState, "Argument %d has to be an object that Lua owns, since its ownership is being handed over!", index);
			}
			auto object = detail::Userdata::get<T>(luaState, index, false);
			if (!object) {
				return object.error();
			}
			owned->ReleaseOwnership();
			return RTE::LuaBinding::Adopted<T>{*object};
		}

		[[nodiscard]] static bool isInstance(lua_State* luaState, int index) { return detail::Userdata::isInstance<T>(luaState, index); }
	};

	template <>
	struct Stack<RTE::LuaBinding::SelfReference> {
		[[nodiscard]] static Result push(lua_State* luaState, const RTE::LuaBinding::SelfReference&) {
			lua_pushvalue(luaState, 1);
			return {};
		}
	};

	template <class Container>
	struct Stack<RTE::LuaBinding::StlIterator<Container>> {
		[[nodiscard]] static Result push(lua_State* luaState, const RTE::LuaBinding::StlIterator<Container>& value) {
			RTE::LuaBinding::StlIteratorState<Container>::Push(luaState, value.TheContainer, nullptr);
			return {};
		}
	};

	template <class Container>
	struct Stack<RTE::LuaBinding::OwnedStlIterator<Container>> {
		[[nodiscard]] static Result push(lua_State* luaState, const RTE::LuaBinding::OwnedStlIterator<Container>& value) {
			RTE::LuaBinding::StlIteratorState<Container>::Push(luaState, value.TheContainer, value.TheContainer);
			return {};
		}
	};
} // namespace luabridge
#pragma endregion

namespace RTE::LuaBinding {

#pragma region Function Signatures
	/// Breaks a function or member function pointer type down into its parts.
	template <class F>
	struct FunctionSignature;

	template <class R, class C, class... A>
	struct FunctionSignature<R (C::*)(A...)> {
		using Return = R;
		using Class = C;
		using Args = std::tuple<A...>;
		static constexpr bool c_IsMember = true;
		static constexpr bool c_IsConst = false;
	};
	template <class R, class C, class... A>
	struct FunctionSignature<R (C::*)(A...) const> : FunctionSignature<R (C::*)(A...)> {
		static constexpr bool c_IsConst = true;
	};
	template <class R, class C, class... A>
	struct FunctionSignature<R (C::*)(A...) noexcept> : FunctionSignature<R (C::*)(A...)> {};
	template <class R, class C, class... A>
	struct FunctionSignature<R (C::*)(A...) const noexcept> : FunctionSignature<R (C::*)(A...) const> {};

	template <class R, class... A>
	struct FunctionSignature<R (*)(A...)> {
		using Return = R;
		using Class = void;
		using Args = std::tuple<A...>;
		static constexpr bool c_IsMember = false;
		static constexpr bool c_IsConst = false;
	};
	template <class R, class... A>
	struct FunctionSignature<R (*)(A...) noexcept> : FunctionSignature<R (*)(A...)> {};

	/// Gets the parameter types of a callable as LuaBridge sees them, i.e. including the object for proxy functions and lambdas, and as T* or const T* for member functions of class T.
	template <class T, class F, class = void>
	struct LuaParameters {
		using Type = typename luabridge::detail::function_traits<F>::argument_types;
	};
	template <class T, class F>
	struct LuaParameters<T, F, std::enable_if_t<std::is_member_function_pointer_v<F>>> {
		using Signature = FunctionSignature<F>;
		using Type = decltype(std::tuple_cat(std::declval<std::tuple<std::conditional_t<Signature::c_IsConst, const T*, T*>>>(), std::declval<typename Signature::Args>()));
	};
#pragma endregion

#pragma region Overload Matching
	/// Score luabind gave to luabind::object parameters, which accept anything but are the least preferred.
	inline constexpr int c_AnyValueMatchScore = 100000;

	/// Scores how well the Lua value at the given stack index matches a C++ parameter type, the way luabind did it, so that overloads get picked the same way.
	/// @return The score, lower being a better match, or -1 if the value can't be passed as that type.
	template <class P, class = void>
	struct ArgumentMatcher {
		using Decayed = std::remove_cv_t<std::remove_reference_t<P>>;

		static int Match(lua_State* luaState, int index) {
			if constexpr (std::is_same_v<Decayed, bool>) {
				return lua_type(luaState, index) == LUA_TBOOLEAN ? 0 : -1;
			} else if constexpr (std::is_enum_v<Decayed>) {
				return lua_isnumber(luaState, index) ? 0 : -1;
			} else if constexpr (std::is_arithmetic_v<Decayed>) {
				return lua_type(luaState, index) == LUA_TNUMBER ? 0 : -1;
			} else if constexpr (std::is_same_v<Decayed, std::string> || std::is_same_v<Decayed, std::string_view>) {
				return lua_type(luaState, index) == LUA_TSTRING ? 0 : -1;
			} else if constexpr (std::is_same_v<Decayed, const char*> || std::is_same_v<Decayed, char*>) {
				return (lua_type(luaState, index) == LUA_TSTRING || lua_isnil(luaState, index)) ? 0 : -1;
			} else if constexpr (std::is_same_v<Decayed, luabridge::LuaRef>) {
				return c_AnyValueMatchScore;
			} else if constexpr (std::is_pointer_v<Decayed>) {
				using Pointee = std::remove_pointer_t<Decayed>;
				if (lua_isnil(luaState, index)) {
					return 0;
				}
				const ClassTypeInfo* info = GetClassTypeInfo(luaState, index);
				int distance = GetClassDistance(info, luabridge::detail::getClassRegistryKey<std::remove_cv_t<Pointee>>());
				if (distance < 0) {
					return -1;
				}
				if constexpr (std::is_const_v<Pointee>) {
					return distance + (info->IsConst() ? 0 : 1);
				} else {
					return info->IsConst() ? -1 : distance;
				}
			} else if constexpr (std::is_class_v<Decayed>) {
				// Registered class passed by value or reference.
				if (lua_isnil(luaState, index)) {
					return -1;
				}
				const ClassTypeInfo* info = GetClassTypeInfo(luaState, index);
				int distance = GetClassDistance(info, luabridge::detail::getClassRegistryKey<Decayed>());
				if (distance < 0) {
					return -1;
				}
				if constexpr (std::is_lvalue_reference_v<P> && !std::is_const_v<std::remove_reference_t<P>>) {
					return info->IsConst() ? -1 : distance;
				} else if constexpr (std::is_lvalue_reference_v<P>) {
					return distance + (info->IsConst() ? 0 : 1);
				} else {
					return distance;
				}
			} else {
				static_assert(sizeof(P) == 0, "Unsupported parameter type for overload matching!");
				return -1;
			}
		}
	};

	template <class T>
	struct ArgumentMatcher<Adopted<T>> {
		static int Match(lua_State* luaState, int index) {
			OwnedUserdataBase* owned = GetOwnedUserdata(luaState, index);
			if (!owned || !owned->IsOwned()) {
				return -1;
			}
			const ClassTypeInfo* info = GetClassTypeInfo(luaState, index);
			return info->IsConst() ? -1 : GetClassDistance(info, luabridge::detail::getClassRegistryKey<T>());
		}
	};

	/// Function that scores how well the arguments on the stack, starting at the given index, match an overload. Returns -1 if they don't.
	using MatchFunction = int (*)(lua_State* luaState, int firstArgumentIndex);

	template <class Tuple>
	struct TupleMatcher;

	template <class... P>
	struct TupleMatcher<std::tuple<P...>> {
		static constexpr int c_Arity = static_cast<int>(sizeof...(P));

		static int Match(lua_State* luaState, int firstArgumentIndex) {
			return MatchImpl(luaState, firstArgumentIndex, std::index_sequence_for<P...>{});
		}

	private:
		template <std::size_t... I>
		static int MatchImpl(lua_State* luaState, int firstArgumentIndex, std::index_sequence<I...>) {
			int total = 0;
			bool matched = true;
			((matched = matched && Accumulate(ArgumentMatcher<P>::Match(luaState, firstArgumentIndex + static_cast<int>(I)), total)), ...);
			return matched ? total : -1;
		}

		static bool Accumulate(int score, int& total) {
			if (score < 0) {
				return false;
			}
			total += score;
			return true;
		}
	};
	/// Scores the arguments of a method, where the first parameter is the object it's called on, which can't be nil.
	template <class Tuple>
	struct MethodMatcher {
		static int Match(lua_State* luaState, int firstArgumentIndex) {
			return lua_isnil(luaState, firstArgumentIndex) ? -1 : TupleMatcher<Tuple>::Match(luaState, firstArgumentIndex);
		}
	};
#pragma endregion

#pragma region Policy Application
	template <class Policy, class... Policies>
	inline constexpr bool c_HasPolicy = (std::is_same_v<Policy, Policies> || ...);

	template <int N, class... Policies>
	inline constexpr bool c_AdoptsArgument = (std::is_same_v<AdoptArgPolicy<N>, Policies> || ...);

	/// Gets whether a parameter is a non-const reference to something that isn't an object of a registered class, like std::string&.
	/// LuaBridge can't pass those, so they're taken by value in a wrapper and passed on as a reference to that.
	template <class P>
	inline constexpr bool c_IsNonConstReferenceToValue = std::is_lvalue_reference_v<P> && !std::is_const_v<std::remove_reference_t<P>> && !luabridge::detail::IsUserdata<std::remove_reference_t<P>>::value;

	/// The type a parameter is declared as in a policy wrapper, given its luabind argument index.
	template <class P, int LuabindIndex, class... Policies>
	struct WrappedParameter {
		using Type = std::conditional_t<c_IsNonConstReferenceToValue<P>, std::remove_reference_t<P>, P>;
	};
	template <class T, int LuabindIndex, class... Policies>
	struct WrappedParameter<T*, LuabindIndex, Policies...> {
		using Type = std::conditional_t<c_AdoptsArgument<LuabindIndex, Policies...>, Adopted<T>, T*>;
	};

	/// Passes a wrapper's argument on to the wrapped function, which takes it as the parameter type P.
	template <class P, class A>
	decltype(auto) UnwrapArgument(A& argument) {
		if constexpr (std::is_lvalue_reference_v<P>) {
			return static_cast<P>(argument);
		} else {
			return static_cast<std::remove_reference_t<A>&&>(argument);
		}
	}
	template <class P, class T>
	T* UnwrapArgument(Adopted<T>& argument) {
		return argument.Object;
	}

	/// Gets whether a function has to be wrapped for LuaBridge to be able to call it, because it returns a const value, or takes a non-const reference to something that isn't an object.
	template <class R, class Args>
	struct NeedsWrappingImpl;
	template <class R, class... A>
	struct NeedsWrappingImpl<R, std::tuple<A...>> {
		static constexpr bool c_Value = (std::is_const_v<R> && !std::is_reference_v<R>) || (c_IsNonConstReferenceToValue<A> || ...);
	};
	template <class F>
	inline constexpr bool c_NeedsWrapping = NeedsWrappingImpl<typename FunctionSignature<F>::Return, typename FunctionSignature<F>::Args>::c_Value;

	/// Gets whether a parameter type is a pointer or reference to const.
	template <class P>
	inline constexpr bool c_IsConstIndirection = std::is_const_v<std::remove_pointer_t<std::remove_reference_t<P>>>;

	/// Converts the object a wrapped method is called on to the type the wrapped free function takes it as, which may be a base class, a pointer or a reference.
	template <class P, class Self>
	decltype(auto) AdaptSelf(Self self) {
		if constexpr (std::is_pointer_v<P>) {
			return static_cast<P>(self);
		} else {
			static_assert(std::is_reference_v<P>, "Free functions registered as methods have to take the object as a pointer or reference!");
			return static_cast<P>(*self);
		}
	}

	/// Works out the return type of a policy wrapper, and converts the wrapped function's return value to it.
	template <class R, class... Policies>
	struct WrappedReturn {
		static constexpr bool c_ReturnsSelf = c_HasPolicy<ReturnSelfPolicy, Policies...>;
		static constexpr bool c_Adopts = c_HasPolicy<AdoptResultPolicy, Policies...>;
		static constexpr bool c_Iterates = c_HasPolicy<ReturnStlIteratorPolicy, Policies...>;

		using Bare = std::remove_cv_t<std::remove_reference_t<R>>;
		using Container = std::remove_cv_t<std::remove_pointer_t<Bare>>;

		/// The function's return type, minus any top level const on a value, which LuaBridge can't push.
		using Plain = std::conditional_t<std::is_reference_v<R>, R, std::remove_const_t<R>>;

		using Type = std::conditional_t<c_ReturnsSelf, SelfReference,
		                                std::conditional_t<c_Iterates,
		                                                   std::conditional_t<c_Adopts || (!std::is_pointer_v<Bare> && !std::is_reference_v<R>), OwnedStlIterator<Container>, StlIterator<Container>>,
		                                                   std::conditional_t<c_Adopts, Owned<std::remove_pointer_t<Bare>>, Plain>>>;

		template <class Call>
		static Type Invoke(Call&& call) {
			if constexpr (c_ReturnsSelf) {
				call();
				return SelfReference{};
			} else if constexpr (c_Iterates) {
				if constexpr (c_Adopts) {
					static_assert(std::is_pointer_v<Bare>, "An adopted iterator container has to be returned as a pointer!");
					return Type{const_cast<Container*>(call())};
				} else if constexpr (std::is_pointer_v<Bare>) {
					return Type{call()};
				} else if constexpr (std::is_reference_v<R>) {
					return Type{&call()};
				} else {
					// Returned by value, so the iterator has to keep its own copy, rather than iterating over a destroyed temporary like luabind did.
					return Type{new Container(call())};
				}
			} else if constexpr (c_Adopts) {
				static_assert(std::is_pointer_v<Bare>, "Only pointers can be adopted!");
				return Type{call()};
			} else {
				return call();
			}
		}
	};

	/// Makes sure the object a wrapped method is called on isn't nil, which LuaBridge passes on as a null pointer.
	/// Throwing makes LuaBridge raise it as a Lua error, like luabind's failed overload match was.
	inline void CheckSelf(const void* self) {
		if (!self) {
			throw std::invalid_argument("Tried to call a method on nil instead of an object, use : rather than . to call methods!");
		}
	}

	template <class T, class F, class... Policies, std::size_t... I>
	auto WrapMemberFunction(F function, std::index_sequence<I...>) {
		using Signature = FunctionSignature<F>;
		using Args = typename Signature::Args;
		using Return = WrappedReturn<typename Signature::Return, Policies...>;
		using Self = std::conditional_t<Signature::c_IsConst, const T*, T*>;

		return [function](Self self, typename WrappedParameter<std::tuple_element_t<I, Args>, static_cast<int>(I) + 2, Policies...>::Type... arguments) -> typename Return::Type {
			CheckSelf(self);
			return Return::Invoke([&]() -> decltype(auto) { return std::invoke(function, self, UnwrapArgument<std::tuple_element_t<I, Args>>(arguments)...); });
		};
	}

	template <class T, class F, class... Policies, std::size_t... I>
	auto WrapFreeFunctionAsMember(F function, std::index_sequence<I...>) {
		using Signature = FunctionSignature<F>;
		using Args = typename Signature::Args;
		using Return = WrappedReturn<typename Signature::Return, Policies...>;
		using SelfParameter = std::tuple_element_t<0, Args>;
		using Self = std::conditional_t<c_IsConstIndirection<SelfParameter>, const T*, T*>;

		return [function](Self self, typename WrappedParameter<std::tuple_element_t<I + 1, Args>, static_cast<int>(I) + 2, Policies...>::Type... arguments) -> typename Return::Type {
			CheckSelf(self);
			return Return::Invoke([&]() -> decltype(auto) { return std::invoke(function, AdaptSelf<SelfParameter>(self), UnwrapArgument<std::tuple_element_t<I + 1, Args>>(arguments)...); });
		};
	}

	template <class F, class... Policies, std::size_t... I>
	auto WrapFreeFunction(F function, std::index_sequence<I...>) {
		using Signature = FunctionSignature<F>;
		using Args = typename Signature::Args;
		using Return = WrappedReturn<typename Signature::Return, Policies...>;

		return [function](typename WrappedParameter<std::tuple_element_t<I, Args>, static_cast<int>(I) + 1, Policies...>::Type... arguments) -> typename Return::Type {
			return Return::Invoke([&]() -> decltype(auto) { return std::invoke(function, UnwrapArgument<std::tuple_element_t<I, Args>>(arguments)...); });
		};
	}

	/// Makes a method of class T out of a member function or a free function that takes the object as its first parameter, applying the given policies.
	/// Member functions without policies are returned as they are, since LuaBridge can call them directly, and everything else is wrapped in a lambda that takes the object as T* or const T*.
	template <class T, bool AlwaysWrap = false, class F, class... Policies>
	auto MakeMethod(F function, Policies...) {
		using Signature = FunctionSignature<F>;
		constexpr std::size_t argumentCount = std::tuple_size_v<typename Signature::Args>;
		if constexpr (Signature::c_IsMember) {
			if constexpr (sizeof...(Policies) == 0 && !AlwaysWrap && !c_NeedsWrapping<F>) {
				return function;
			} else {
				return WrapMemberFunction<T, F, Policies...>(function, std::make_index_sequence<argumentCount>{});
			}
		} else {
			static_assert(argumentCount > 0, "Free functions registered as methods have to take the object as their first parameter!");
			return WrapFreeFunctionAsMember<T, F, Policies...>(function, std::make_index_sequence<argumentCount - 1>{});
		}
	}

	/// Makes a global function out of a free function, applying the given policies.
	template <class F, class... Policies>
	auto MakeFunction(F function, Policies...) {
		if constexpr (sizeof...(Policies) == 0 && !c_NeedsWrapping<F>) {
			return function;
		} else {
			return WrapFreeFunction<F, Policies...>(function, std::make_index_sequence<std::tuple_size_v<typename FunctionSignature<F>::Args>>{});
		}
	}
#pragma endregion

#pragma region Property Accessors
	/// A property getter or setter, stored in a userdata in a class's property tables, which the class's __index and __newindex call directly.
	/// Calling them this way, rather than through Lua closures like LuaBridge's own properties, saves a Lua call and a type check on every property access, which scripts do more of than anything else.
	struct PropertyAccessor {
		/// Pushes the property's value for getters, or sets the property to the value at stack index 3 for setters.
		/// @param object Pointer to the object, already adjusted to point to the class the property is registered on.
		/// @param callable The getter or setter function, member pointer or lambda.
		/// @return How many values were pushed.
		using Function = int (*)(lua_State* luaState, void* object, const void* callable);

		Function Call; //!< Calls the getter or setter.
		const void* Callable; //!< The getter or setter that Call calls, kept alive in a userdata of its own.
		std::ptrdiff_t Offset; //!< What to add to a pointer to an object of the class whose property tables this is in, to get a pointer to the class the property is registered on.
	};

	/// Calls a property getter of class T and pushes what it returns.
	template <class T, class Getter>
	int CallPropertyGetter(lua_State* luaState, void* object, const void* callable) {
		const Getter& getter = *static_cast<const Getter*>(callable);
		luabridge::Result result;
		try {
			// luabind let properties be read from const objects whatever the getter took the object as.
			if constexpr (std::is_invocable_v<const Getter&, const T*>) {
				using Value = decltype(std::invoke(getter, static_cast<const T*>(object)));
				result = luabridge::Stack<Value>::push(luaState, std::invoke(getter, static_cast<const T*>(object)));
			} else {
				using Value = decltype(std::invoke(getter, static_cast<T*>(object)));
				result = luabridge::Stack<Value>::push(luaState, std::invoke(getter, static_cast<T*>(object)));
			}
		} catch (const std::exception& exception) {
			RaiseError(luaState, "%s", exception.what());
		}
		if (!result) {
			RaiseError(luaState, "%s", result.error_cstr());
		}
		return 1;
	}

	/// Calls a property setter of class T with the value at stack index 3. Setters take the object as const, see ClassBuilder::MakeSetter.
	template <class T, class Setter>
	int CallPropertySetter(lua_State* luaState, void* object, const void* callable) {
		const Setter& setter = *static_cast<const Setter*>(callable);
		using Value = std::tuple_element_t<1, typename luabridge::detail::function_traits<Setter>::argument_types>;
		try {
			luabridge::detail::invoke_callable_from_stack<std::tuple<Value>, 3>(luaState, [&setter, object](Value value) { std::invoke(setter, static_cast<const T*>(object), std::forward<Value>(value)); });
		} catch (const std::exception& exception) {
			RaiseError(luaState, "%s", exception.what());
		}
		return 0;
	}

	/// A property registered on a class, recorded during registration and put into the property tables of the class and its derived classes once every class is registered.
	struct PropertyRecord {
		std::string Name; //!< The property name.
		PropertyAccessor Getter; //!< The getter.
		PropertyAccessor Setter; //!< The setter, or one with a null Call if the property is read only.
	};
#pragma endregion

#pragma region Registration
	/// An overload of a function, method, operator or constructor, recorded during registration and resolved once every class is registered.
	struct OverloadRecord {
		std::string Name; //!< The name it's registered under.
		int ClosureReference; //!< Lua registry reference to the function closure that calls it.
		int Arity; //!< How many arguments it takes, including the object for methods.
		MatchFunction Matcher; //!< Scores how well arguments match it.
		const std::type_info* Signature; //!< The type of the tuple of its parameters excluding the object, used to find methods hidden by identical ones in derived classes.
		bool ConstCallable; //!< Whether it can be called on const objects.
	};

	/// Everything recorded about a registered class.
	struct ClassRecord {
		std::string Name; //!< The class name.
		const void* ClassKey; //!< LuaBridge registry key of the class metatable.
		const void* ConstKey; //!< LuaBridge registry key of the const metatable.
		const void* StaticKey; //!< LuaBridge registry key of the static (class) table.
		int ParentIndex; //!< Index of the base class's record, or -1 if there isn't one.
		std::ptrdiff_t ParentOffset; //!< What has to be added to a pointer to an object of the class to get a pointer to its base class part.
		PushPointerFunction PushPointer; //!< Pushes a non-owned pointer to an object of the class.
		std::vector<OverloadRecord> Methods; //!< The methods and operator metamethods registered on this class itself.
		std::vector<OverloadRecord> Constructors; //!< The constructors of this class.
		std::vector<std::pair<std::string, int>> Constants; //!< The enum values registered on this class itself.
		std::vector<PropertyRecord> Properties; //!< The properties registered on this class itself.
	};

	template <class T, class Base>
	class ClassBuilder;

	/// Collects everything registered into a Lua state, and sets up the luabind compatible lookups for it once it's all registered.
	class Registry {
		template <class T, class Base>
		friend class ClassBuilder;

	public:
		/// Constructor method used to instantiate a Registry object for a Lua state.
		explicit Registry(lua_State* luaState) :
		    m_LuaState(luaState) {}

		/// Gets the Lua state being registered into.
		lua_State* GetLuaState() const { return m_LuaState; }

		/// Starts registering a class.
		/// @tparam Base The registered base class of the class, if it has one.
		template <class T, class Base = void>
		ClassBuilder<T, Base> Class(const char* name);

		/// Registers a global function. Registering more under the same name adds overloads to it.
		template <class F, class... Policies>
		Registry& Function(const char* name, F function, Policies... policies) {
			auto wrapped = MakeFunction(function, policies...);
			using Parameters = typename luabridge::detail::function_traits<decltype(wrapped)>::argument_types;
			luabridge::detail::push_function(m_LuaState, std::move(wrapped), name);
			m_Functions.push_back({name, luaL_ref(m_LuaState, LUA_REGISTRYINDEX), TupleMatcher<Parameters>::c_Arity, &TupleMatcher<Parameters>::Match, &typeid(Parameters), true});
			return *this;
		}

		/// Sets up the metatables of every registered class, and the global functions, so they behave the way they did with luabind. Has to be called once everything is registered.
		void Finalize();

		/// Stores a copy of a property getter or setter in a userdata that lives as long as the Lua state.
		/// @return Pointer to the stored copy, which never moves.
		template <class Callable>
		const void* StoreCallable(Callable callable) {
			static_assert(std::is_trivially_destructible_v<Callable>, "Property getters and setters are stored without finalizers, so they can't need destructing!");
			if (m_KeepAliveTableReference == LUA_NOREF) {
				lua_newtable(m_LuaState);
				m_KeepAliveTableReference = luaL_ref(m_LuaState, LUA_REGISTRYINDEX);
			}
			lua_rawgeti(m_LuaState, LUA_REGISTRYINDEX, m_KeepAliveTableReference);
			void* storage = lua_newuserdata(m_LuaState, sizeof(Callable));
			new (storage) Callable(std::move(callable));
			lua_rawseti(m_LuaState, -2, static_cast<int>(lua_objlen(m_LuaState, -2)) + 1);
			lua_pop(m_LuaState, 1);
			return storage;
		}

	private:
		lua_State* m_LuaState; //!< The Lua state being registered into.
		int m_KeepAliveTableReference = LUA_NOREF; //!< Lua registry reference to the table that keeps the stored property getters and setters alive.
		std::vector<ClassRecord> m_Classes; //!< Every registered class, bases before their derived classes.
		std::unordered_map<const void*, int> m_ClassIndices; //!< Index of each class's record, by class registry key.
		std::vector<OverloadRecord> m_Functions; //!< The global functions.
	};

	/// Gets a function that pushes a non-owned pointer to an object of class T.
	template <class T>
	PushPointerFunction GetPushPointerFunction() {
		return [](lua_State* luaState, void* object) {
			if (auto result = luabridge::Stack<T*>::push(luaState, static_cast<T*>(object)); !result) {
				lua_pushnil(luaState);
			}
		};
	}

	/// Registers the members of a class, mirroring luabind's class_. Methods registered more than once under the same name become overloads, including across base and derived classes.
	template <class T, class Base>
	class ClassBuilder {
	public:
		ClassBuilder(Registry& registry, const char* name) :
		    m_Registry(&registry), m_Class(Begin(registry.GetLuaState(), name)), m_RecordIndex(static_cast<int>(registry.m_Classes.size())) {
			ClassRecord record;
			record.Name = name;
			record.ClassKey = luabridge::detail::getClassRegistryKey<T>();
			record.ConstKey = luabridge::detail::getConstRegistryKey<T>();
			record.StaticKey = luabridge::detail::getStaticRegistryKey<T>();
			record.ParentIndex = -1;
			record.ParentOffset = 0;
			if constexpr (!std::is_void_v<Base>) {
				record.ParentOffset = GetBaseOffset<T, Base>();
				auto parent = registry.m_ClassIndices.find(luabridge::detail::getClassRegistryKey<Base>());
				if (parent == registry.m_ClassIndices.end()) {
					RaiseError(registry.GetLuaState(), "Class %s was registered before its base class!", name);
				}
				record.ParentIndex = parent->second;
			}
			record.PushPointer = GetPushPointerFunction<T>();
			registry.m_ClassIndices.emplace(record.ClassKey, m_RecordIndex);
			registry.m_Classes.push_back(std::move(record));
		}

		ClassBuilder(ClassBuilder&&) = default;

		/// Registers a method, or an overload of one. Free functions are registered as methods by taking the object as their first parameter, like they were with luabind.
		template <class F, class... Policies>
		ClassBuilder&& addFunction(const char* name, F function, Policies... policies) && {
			AddMethod(name, MakeMethod<T>(function, policies...));
			return std::move(*this);
		}

		/// Registers a read only property. Policies can be given for the getter.
		template <class Getter, class... Policies>
		ClassBuilder&& addProperty(const char* name, Getter getter, Policies... policies) && {
			if constexpr (sizeof...(Policies) > 0 && !(IsPolicy<Policies>() && ...)) {
				AddReadWriteProperty(name, getter, policies...);
			} else if constexpr (std::is_member_object_pointer_v<Getter>) {
				AddDataMember(name, getter, policies...);
			} else {
				AddAccessors(name, MakeGetter(getter, policies...));
			}
			return std::move(*this);
		}

		/// Registers a data member that can be read and written to, luabind's def_readwrite.
		/// Members that are objects of registered classes are returned as references to them, so they can be changed in place, like `box.Corner.X = 5`.
		template <class M>
		ClassBuilder&& addReadWriteMember(const char* name, M T::*member) && {
			if constexpr (luabridge::detail::IsUserdata<std::remove_cv_t<M>>::value) {
				AddAccessors(name, [member](const T* self) -> M& { return const_cast<T*>(self)->*member; }, MakeMemberSetter(member));
			} else {
				AddAccessors(name, [member](const T* self) -> M { return self->*member; }, MakeMemberSetter(member));
			}
			return std::move(*this);
		}

		/// Registers a constructor, or an overload of one.
		template <class... Args>
		ClassBuilder&& addConstructor() && {
			lua_State* luaState = m_Registry->GetLuaState();
			lua_pushcfunction(luaState, &Construct<Args...>);
			using Parameters = std::tuple<Args...>;
			Record().Constructors.push_back({"__init", luaL_ref(luaState, LUA_REGISTRYINDEX), TupleMatcher<Parameters>::c_Arity, &TupleMatcher<Parameters>::Match, &typeid(Parameters), true});
			return std::move(*this);
		}

		/// Registers enum values as constants on the class, the way luabind's enum_ did. They're stored as integers, like luabind did it.
		ClassBuilder&& addEnum(std::initializer_list<std::pair<const char*, int>> values) && {
			for (const auto& [valueName, value]: values) {
				Record().Constants.emplace_back(valueName, value);
			}
			return std::move(*this);
		}

		/// Registers the == operator against another type, luabind's self == other<Other>().
		template <class Other>
		ClassBuilder&& addEqualityOperator() && {
			AddMethod("__eq", [](const T* self, Other other) { return *self == other; });
			return std::move(*this);
		}

		/// Registers tostring support using the class's << operator, luabind's tostring(const_self).
		ClassBuilder&& addToString() && {
			AddMethod("__tostring", [](const T* self) {
				std::stringstream stream;
				stream << *self;
				return stream.str();
			});
			return std::move(*this);
		}

	private:
		Registry* m_Registry; //!< The registry this class is registered in.
		using LuaBridgeClass = decltype(std::declval<luabridge::Namespace>().template beginClass<T>("")); //!< LuaBridge's class registration type, which is private, but can be named this way.

		LuaBridgeClass m_Class; //!< The LuaBridge class registration. Ends the registration when it's destroyed.
		int m_RecordIndex; //!< Index of this class's record in the registry.

		static LuaBridgeClass Begin(lua_State* luaState, const char* name) {
			if constexpr (std::is_void_v<Base>) {
				return luabridge::getGlobalNamespace(luaState).beginClass<T>(name, luabridge::extensibleClass);
			} else {
				return luabridge::getGlobalNamespace(luaState).deriveClass<T, Base>(name, luabridge::extensibleClass);
			}
		}

		ClassRecord& Record() { return m_Registry->m_Classes[m_RecordIndex]; }

		template <class P>
		static constexpr bool IsPolicy() {
			return std::is_same_v<P, AdoptResultPolicy> || std::is_same_v<P, ReturnStlIteratorPolicy> || std::is_same_v<P, ReturnSelfPolicy>;
		}

		/// Makes a property getter. Const member functions are used as they are, everything else is wrapped.
		template <class Getter, class... Policies>
		static auto MakeGetter(Getter getter, Policies... policies) {
			if constexpr (sizeof...(Policies) == 0 && std::is_member_function_pointer_v<Getter> && FunctionSignature<Getter>::c_IsConst && !c_NeedsWrapping<Getter>) {
				return getter;
			} else {
				return MakeMethod<T, true>(getter, policies...);
			}
		}

		/// Records a property with the given getter, and setter if there is one, which ends up in the property tables of the class and its derived classes when the registry is finalized.
		template <class Getter>
		void AddAccessors(const char* name, Getter getter) {
			PropertyRecord record{name, {&CallPropertyGetter<T, Getter>, m_Registry->StoreCallable(std::move(getter)), 0}, {nullptr, nullptr, 0}};
			Record().Properties.push_back(std::move(record));
		}
		template <class Getter, class Setter>
		void AddAccessors(const char* name, Getter getter, Setter setter) {
			PropertyRecord record{name, {&CallPropertyGetter<T, Getter>, m_Registry->StoreCallable(std::move(getter)), 0}, {&CallPropertySetter<T, Setter>, m_Registry->StoreCallable(std::move(setter)), 0}};
			Record().Properties.push_back(std::move(record));
		}

		/// Makes a property setter.
		/// luabind let properties be set on const objects too, and scripts rely on that to change things like `actor.Pos.X` in place, so setters take the object as const and cast that away.
		template <class Setter>
		static auto MakeSetter(Setter setter) {
			using Signature = FunctionSignature<Setter>;
			using Args = typename Signature::Args;
			if constexpr (Signature::c_IsMember) {
				using Value = std::tuple_element_t<0, Args>;
				return [setter](const T* self, typename WrappedParameter<Value, 2>::Type value) {
					std::invoke(setter, const_cast<T*>(self), UnwrapArgument<Value>(value));
				};
			} else {
				using SelfParameter = std::tuple_element_t<0, Args>;
				using Value = std::tuple_element_t<1, Args>;
				return [setter](const T* self, typename WrappedParameter<Value, 2>::Type value) {
					std::invoke(setter, AdaptSelf<SelfParameter>(const_cast<T*>(self)), UnwrapArgument<Value>(value));
				};
			}
		}

		/// Makes a setter for a data member that can be set on const objects too, see MakeSetter.
		template <class M>
		static auto MakeMemberSetter(M T::*member) {
			using Value = std::conditional_t<std::is_arithmetic_v<M> || std::is_enum_v<M> || std::is_pointer_v<M>, M, const M&>;
			return [member](const T* self, Value value) { const_cast<T*>(self)->*member = value; };
		}

		template <class Getter, class Setter, class... Policies>
		void AddReadWriteProperty(const char* name, Getter getter, Setter setter, Policies... policies) {
			static_assert((IsPolicy<Policies>() && ...), "Only getters can take policies!");
			AddAccessors(name, MakeGetter(getter, policies...), MakeSetter(setter));
		}

		template <class M, class... Policies>
		void AddDataMember(const char* name, M T::*member, Policies...) {
			if constexpr (sizeof...(Policies) > 0) {
				static_assert((std::is_same_v<Policies, ReturnStlIteratorPolicy> && ...), "Data members can only take the iterator policy!");
				AddAccessors(name, [member](const T* self) { return StlIterator<std::remove_cv_t<M>>{&(self->*member)}; });
			} else if constexpr (luabridge::detail::IsUserdata<std::remove_cv_t<M>>::value) {
				// Read only data members of registered classes are returned as const references, the way luabind did it.
				AddAccessors(name, [member](const T* self) -> const M& { return self->*member; });
			} else {
				AddAccessors(name, [member](const T* self) -> M { return self->*member; });
			}
		}

		template <class F>
		void AddMethod(const char* name, F function) {
			lua_State* luaState = m_Registry->GetLuaState();
			using Parameters = typename LuaParameters<T, F>::Type;
			using Self = std::tuple_element_t<0, Parameters>;
			using SelfPointee = std::remove_reference_t<std::remove_pointer_t<Self>>;
			luabridge::detail::push_member_function<T>(luaState, std::move(function), name);
			Record().Methods.push_back({name, luaL_ref(luaState, LUA_REGISTRYINDEX), TupleMatcher<Parameters>::c_Arity, &MethodMatcher<Parameters>::Match, &typeid(TailOf<Parameters>), std::is_const_v<SelfPointee>});
		}

		template <class Tuple>
		struct Tail;
		template <class Head, class... Rest>
		struct Tail<std::tuple<Head, Rest...>> {
			using Type = std::tuple<Rest...>;
		};
		template <class Tuple>
		using TailOf = typename Tail<Tuple>::Type;

		/// Constructs an object from the arguments after the class table, which is the first argument of __call.
		template <class... Args>
		static int Construct(lua_State* luaState) {
			if constexpr (c_ConstructOnHeap<T>) {
				T* object = luabridge::detail::invoke_callable_from_stack<std::tuple<Args...>, 2>(luaState, [](Args... arguments) { return new T(arguments...); });
				OwnedUserdata<T>::Push(luaState, object);
			} else {
				std::error_code error;
				auto* userdata = luabridge::detail::UserdataValue<T>::place(luaState, error);
				if (!userdata) {
					RaiseError(luaState, "%s", error.message().c_str());
				}
				luabridge::detail::invoke_callable_from_stack<std::tuple<Args...>, 2>(luaState, [userdata](Args... arguments) { new (userdata->getObject()) T(arguments...); });
				userdata->commit();
			}
			return 1;
		}
	};

	template <class T, class Base>
	ClassBuilder<T, Base> Registry::Class(const char* name) {
		return ClassBuilder<T, Base>(*this, name);
	}
#pragma endregion

} // namespace RTE::LuaBinding

namespace RTE {
	/// Can't have global enums in the master state so we use this dummy struct as a class and register the enums under it.
	struct enum_wrapper {};

	/// Derived structs for each of the input enums because we can't register enum_wrapper multiple times under a different name.
	/// We're doing this so we can access each enum separately by name rather than having all of them accessed from a shared name.
	/// If this proves to be a hassle then we can easily revert to the shared name access by registering everything under enum_wrapper.
	struct input_device : public enum_wrapper {};
	struct input_elements : public enum_wrapper {};
	struct mouse_buttons : public enum_wrapper {};
	struct joy_buttons : public enum_wrapper {};
	struct joy_directions : public enum_wrapper {};
	struct key_codes : public enum_wrapper {};
	struct scan_codes : public enum_wrapper {};
	struct gamepad_buttons : public enum_wrapper {};
	struct gamepad_axis : public enum_wrapper {};
	struct directions : public enum_wrapper {};
	struct blend_modes : public enum_wrapper {};
	struct draw_depths : public enum_wrapper {};

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
	/// @remark In case of type mismatch (by specifying wrong type or a mix of types in the Lua table) a Lua error is raised, like luabind's object_cast did.
	template <typename Type>
	static std::vector<Type> ConvertLuaTableToVectorOfType(const luabridge::LuaRef& luaTable) {
		std::vector<Type> outVector = {};
		if (!luaTable.isValid() || !luaTable.isTable()) {
			return outVector;
		}
		lua_State* luaState = luaTable.state();
		luaTable.push(luaState);
		const int tableIndex = lua_gettop(luaState);
		lua_pushnil(luaState);
		while (lua_next(luaState, tableIndex) != 0) {
			if constexpr (std::is_pointer_v<Type>) {
				using Pointee = std::remove_pointer_t<Type>;
				auto object = luabridge::detail::Userdata::get<Pointee>(luaState, -1, true);
				if (!object || !*object) {
					LuaBinding::RaiseError(luaState, "Wrong type of object in table, expected %s!", typeid(Pointee).name());
				}
				if constexpr (LuaBinding::c_ConstructOnHeap<Pointee>) {
					// The object is handed over to C++, so Lua mustn't delete it anymore.
					LuaBinding::OwnedUserdataBase* owned = LuaBinding::GetOwnedUserdata(luaState, -1);
					if (!owned || !owned->IsOwned()) {
						LuaBinding::RaiseError(luaState, "Objects in the table have to be owned by Lua, since their ownership is being handed over!");
					}
					owned->ReleaseOwnership();
					outVector.emplace_back(*object);
				} else {
					// Values are stored inside their userdata, so hand over a copy that C++ owns instead.
					outVector.emplace_back(new Pointee(**object));
				}
			} else {
				auto value = luabridge::Stack<Type>::get(luaState, -1);
				if (!value) {
					LuaBinding::RaiseError(luaState, "Wrong type of value in table: %s", value.error_cstr());
				}
				outVector.emplace_back(*value);
			}
			lua_pop(luaState, 1);
		}
		lua_pop(luaState, 1);
		return outVector;
	}
} // namespace RTE
