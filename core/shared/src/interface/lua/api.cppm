// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.api;

export import pragma.lua;
import :scripting.lua.handles;

export namespace luabind {
	// Additional types for overload resolution
	namespace adl {
		template<typename T>
		struct optional : object {
			optional(from_stack const &stack_reference) : object(stack_reference) {}
			optional(const object &o) : object(o) {}
			optional(lua::State *l, const T &t) : object(l, t) {}
			using value_type = T;
		};

		template<typename T = object>
		struct userData : object {
			userData(from_stack const &stack_reference) : object(stack_reference) {}
			userData(const object &o) : object(o) {}
		};

		template<typename T = object>
		struct classObject : object {
			classObject(from_stack const &stack_reference) : object(stack_reference) {}
			classObject(const object &o) : object(o) {}
		};

		template<typename T>
		struct tableT : object {
			tableT(from_stack const &stack_reference) : object(stack_reference) {}
			tableT(const object &o) : object(o) {}
			using value_type = T;
		};

		template<typename TKey, typename TVal>
		struct map : object {
			map(from_stack const &stack_reference) : object(stack_reference) {}
			map(const object &o) : object(o) {}
			using key_type = TKey;
			using value_type = TVal;
		};

		template<typename T, typename T2>
		struct tableTT : object {
			tableTT(from_stack const &stack_reference) : object(stack_reference) {}
			tableTT(const object &o) : object(o) {}
			using value_type = T;
			using sub_value_type = T2;
		};

		template<typename... T>
		struct variant : object {
			variant(from_stack const &stack_reference) : object(stack_reference) {}
			variant(const object &o) : object(o) {}
		};

		template<typename... T>
		struct variadic : object {
			variadic(from_stack const &stack_reference) : object(stack_reference) {}
			variadic(const object &o) : object(o) {}
		};

		template<typename... T>
		struct mult : object {
			mult(from_stack const &stack_reference) : object(stack_reference) {}
			mult(const object &o) : object(o) {}
			mult(lua::State *l, T... args) : object()
			{
				([&](auto &input) { Lua::Push(l, input); }(args), ...);
			}
		};

		template<typename T>
		struct typehint : object {
			typehint(from_stack const &stack_reference) : object(stack_reference) {}
			typehint(const object &o) : object(o) {}
			typehint(lua::State *l, const T &t) : object(l, t) {}
			using value_type = T;
		};

		template<typename TRet, typename... T>
		struct functype : object {
			functype(from_stack const &stack_reference) : object(stack_reference) {}
			functype(const object &o) : object(o) {}
			using return_type = TRet;
		};

	} // namespace adl

	namespace detail {
		template<typename T>
		struct pseudo_traits<adl::optional<T>> {
			enum { is_pseudo_type = true };
			enum { is_variadic = false };
			using value_type = T;
		};

		template<typename T>
		struct pseudo_traits<adl::tableT<T>> {
			enum { is_pseudo_type = true };
			enum { is_variadic = false };
			using value_type = T;
		};

		template<typename TKey, typename TVal>
		struct pseudo_traits<adl::map<TKey, TVal>> {
			enum { is_pseudo_type = true };
			enum { is_variadic = true };
		};

		template<typename T, typename T2>
		struct pseudo_traits<adl::tableTT<T, T2>> {
			enum { is_pseudo_type = true };
			enum { is_variadic = true };
		};

		template<typename... T>
		struct pseudo_traits<adl::variant<T...>> {
			enum { is_pseudo_type = true };
			enum { is_variadic = true };
		};

		template<typename... T>
		struct pseudo_traits<adl::variadic<T...>> {
			enum { is_pseudo_type = true };
			enum { is_variadic = true };
		};

		template<typename... T>
		struct pseudo_traits<adl::mult<T...>> {
			enum { is_pseudo_type = true };
			enum { is_variadic = true };
		};

		template<typename TRet, typename... T>
		struct pseudo_traits<adl::functype<TRet, T...>> {
			enum { is_pseudo_type = true };
			enum { is_variadic = true };
		};
	};

	using adl::classObject;
	using adl::functype;
	using adl::map;
	using adl::mult;
	using adl::optional;
	using adl::tableT;
	using adl::tableTT;
	using adl::typehint;
	using adl::userData;
	using adl::variadic;
	using adl::variant;

	template<typename T>
	struct lua_proxy_traits<optional<T>> : lua_proxy_traits<object> {
		static bool check(lua::State *L, int idx)
		{
			if constexpr(std::is_fundamental_v<T>)
				return lua_proxy_traits<object>::check(L, idx) && (!Lua::IsSet(L, idx) || Lua::IsNumber(L, idx));
			else
				return lua_proxy_traits<object>::check(L, idx) && (!Lua::IsSet(L, idx) || luabind::object_cast_nothrow<T *>(from_stack {L, idx}, static_cast<T *>(nullptr)) != nullptr);
		}
	};

	template<typename T>
	struct lua_proxy_traits<userData<T>> : lua_proxy_traits<object> {
		static bool check(lua::State *L, int idx) { return lua_proxy_traits<object>::check(L, idx) && Lua::IsUserData(L, idx); }
	};

	template<typename T>
	struct lua_proxy_traits<classObject<T>> : lua_proxy_traits<object> {
		static bool check(lua::State *L, int idx) { return lua_proxy_traits<object>::check(L, idx) && Lua::IsUserData(L, idx); }
	};

	template<typename T>
	struct lua_proxy_traits<tableT<T>> : lua_proxy_traits<object> {
		static bool check(lua::State *L, int idx) { return lua_proxy_traits<object>::check(L, idx) && Lua::IsTable(L, idx); }
	};

	template<typename TKey, typename TVal>
	struct lua_proxy_traits<map<TKey, TVal>> : lua_proxy_traits<object> {
		static bool check(lua::State *L, int idx) { return lua_proxy_traits<object>::check(L, idx) && Lua::IsTable(L, idx); }
	};

	template<typename T, typename T2>
	struct lua_proxy_traits<tableTT<T, T2>> : lua_proxy_traits<object> {
		static bool check(lua::State *L, int idx) { return lua_proxy_traits<object>::check(L, idx) && Lua::IsTable(L, idx); }
	};

	template<typename... T>
	struct lua_proxy_traits<variant<T...>> : lua_proxy_traits<object> {
		static bool check(lua::State *L, int idx) { return lua_proxy_traits<object>::check(L, idx); }
	};

	template<typename... T>
	struct lua_proxy_traits<variadic<T...>> : lua_proxy_traits<object> {
		static bool check(lua::State *L, int idx) { return lua_proxy_traits<object>::check(L, idx); }
	};

	template<typename... T>
	struct lua_proxy_traits<mult<T...>> : lua_proxy_traits<object> {
		static bool check(lua::State *L, int idx) { return lua_proxy_traits<object>::check(L, idx); }
	};

	template<typename T>
	struct lua_proxy_traits<typehint<T>> : lua_proxy_traits<object> {
		static bool check(lua::State *L, int idx) { return lua_proxy_traits<object>::check(L, idx) && Lua::IsSet(L, idx); }
	};

	template<typename TRet, typename... T>
	struct lua_proxy_traits<functype<TRet, T...>> : lua_proxy_traits<object> {
		static bool check(lua::State *L, int idx) { return lua_proxy_traits<object>::check(L, idx) && Lua::IsFunction(L, idx); }
	};
};

export namespace Lua {
	template<typename T>
	using opt = luabind::optional<T>;
	using userData = luabind::userData<luabind::object>;
	using classObject = luabind::classObject<luabind::object>;
	template<typename... T>
	using var = luabind::variant<T...>;
	template<typename... T>
	using variadic = luabind::variadic<T...>;
	template<typename... T>
	using mult = luabind::mult<T...>;
	template<typename T>
	using tb = luabind::tableT<T>;
	template<typename TKey, typename TVal>
	using map = luabind::map<TKey, TVal>;
	template<typename T, typename T2>
	using tbt = luabind::tableTT<T, T2>;
	template<typename T>
	using type = luabind::typehint<T>;
	template<typename TRet, typename... T>
	using func = luabind::functype<TRet, T...>;

	DLLNETWORK extern const luabind::object nil;
	using object = luabind::object;

	struct Exception : public std::runtime_error {
	  public:
		Exception(const std::string &err) : std::runtime_error {err} {}
	};

	DLLNETWORK void initialize_lua_state(Interface &lua);
	DLLNETWORK void set_extended_lua_modules_enabled(bool b);
	DLLNETWORK bool get_extended_lua_modules_enabled();
};

export namespace pragma::LuaCore {
	template<typename T, typename TPush = T>
	luabind::object raw_object_to_luabind_object(lua::State *l, T v)
	{
		// Using the value_converter will prevent the default_converter from getting triggered, which would cause an infinite recursion in some cases
		luabind::detail::value_converter c;
		c.to_lua<TPush>(l, static_cast<TPush>(v));
		auto o = luabind::object {luabind::from_stack(l, -1)};
		Lua::Pop(l, 1);
		return o;
	}
};
