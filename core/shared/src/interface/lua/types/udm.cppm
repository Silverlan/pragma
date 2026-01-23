// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

export module pragma.shared:scripting.lua.types.udm;

import :scripting.lua.core;
export import pragma.lua;
export import pragma.udm;

export namespace luabind {
	// Additional types for overload resolution
	namespace adl {
		template<typename T, bool ENABLE_NUMERIC, bool ENABLE_GENERIC, bool ENABLE_NON_TRIVIAL>
		struct udm_type : object {
			udm_type(from_stack const &stack_reference) : object(stack_reference) {}
			udm_type(const object &o) : object(o) {}
			udm_type(lua::State *l, const T &t) : object(l, t) {}
			using value_type = T;
		};
	} // namespace adl

	namespace detail {
		template<typename T, bool ENABLE_NUMERIC, bool ENABLE_GENERIC, bool ENABLE_NON_TRIVIAL>
		struct pseudo_traits<adl::udm_type<T, ENABLE_NUMERIC, ENABLE_GENERIC, ENABLE_NON_TRIVIAL>> {
			enum { is_variadic = false };
			using value_type = T;
		};
	};

	using adl::udm_type;

	template<bool ENABLE_NUMERIC, bool ENABLE_GENERIC, bool ENABLE_NON_TRIVIAL>
	bool check_udm(lua::State *L, int idx)
	{
		if(!lua_proxy_traits<object>::check(L, idx))
			return false;
		auto fCheck = [L, idx]<bool LENABLE_NUMERIC, bool LENABLE_GENERIC, bool LENABLE_NON_TRIVIAL>(const auto &types) {
			for(auto type : types) {
				auto r = udm::visit<LENABLE_NUMERIC, LENABLE_GENERIC, LENABLE_NON_TRIVIAL>(type, [L, idx](auto tag) {
					using T = typename decltype(tag)::type;
					return Lua::IsType<T>(L, idx);
				});
				if(r)
					return true;
			}
			return false;
		};
		if constexpr(ENABLE_NUMERIC) {
			if(fCheck.template operator()<true, false, false>(udm::NUMERIC_TYPES))
				return true;
		}
		if constexpr(ENABLE_GENERIC) {
			if(fCheck.template operator()<false, true, false>(udm::GENERIC_TYPES))
				return true;
		}
		if constexpr(ENABLE_NON_TRIVIAL) {
			if(fCheck.template operator()<false, false, true>(udm::NON_TRIVIAL_TYPES))
				return true;
		}
		return false;
	}

	template<typename T>
	struct lua_proxy_traits<udm_type<T, true, false, false>> : lua_proxy_traits<object> {
		static bool check(lua::State *L, int idx) { return check_udm<true, false, false>(L, idx); }
	};

	template<typename T>
	struct lua_proxy_traits<udm_type<T, false, true, false>> : lua_proxy_traits<object> {
		static bool check(lua::State *L, int idx) { return check_udm<false, true, false>(L, idx); }
	};

	template<typename T>
	struct lua_proxy_traits<udm_type<T, true, true, false>> : lua_proxy_traits<object> {
		static bool check(lua::State *L, int idx) { return check_udm<true, true, false>(L, idx); }
	};

	template<typename T>
	struct lua_proxy_traits<udm_type<T, false, false, true>> : lua_proxy_traits<object> {
		static bool check(lua::State *L, int idx) { return check_udm<false, false, true>(L, idx); }
	};

	template<typename T>
	struct lua_proxy_traits<udm_type<T, true, true, true>> : lua_proxy_traits<object> {
		static bool check(lua::State *L, int idx) { return check_udm<true, true, true>(L, idx); }
	};
};

export namespace Lua {
	using udm_numeric = luabind::udm_type<luabind::object, true, false, false>;
	using udm_generic = luabind::udm_type<luabind::object, false, true, false>;
	using udm_non_trivial = luabind::udm_type<luabind::object, false, false, true>;
	using udm_ng = luabind::udm_type<luabind::object, true, true, false>;
	using udm_type = luabind::udm_type<luabind::object, true, true, true>;
};
