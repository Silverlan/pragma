// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __LUA_GAME_TYPE_CONVERTERS_HPP__
#define __LUA_GAME_TYPE_CONVERTERS_HPP__

#include "pragma/networkdefinitions.h"
#include <fsys/filesystem.h>
#include <luabind/detail/conversion_policies/native_converter.hpp>
#include <vector>
#include <map>
#include <array>
#include <unordered_map>

import pragma.shared;

namespace luabind {
	template<typename T, T (*FUNCTION)(lua_State *)>
	struct parameter_emplacement_converter {
		enum { consumed_args = 0 };

		template<class U>
		T to_cpp(lua_State *L, U, int /*index*/);

		template<class U>
		static int match(lua_State *, U, int /*index*/);

		template<class U>
		void converter_postcall(lua_State *, U, int)
		{
		}
	};

	// Game
	namespace detail {
		DLLNETWORK Game *get_game(lua_State *l);
		template<typename T>
		T get_game(lua_State *l)
		{
			if constexpr(std::is_pointer_v<T>)
				return static_cast<T>(get_game(l));
			else
				return static_cast<T>(*get_game(l));
		}
	};
	template<typename T>
	    requires(is_type_or_derived<T, Game>)
	struct default_converter<T> : parameter_emplacement_converter<T, &detail::get_game<T>> {};

	// NetworkState
	namespace detail {
		DLLNETWORK NetworkState *get_network_state(lua_State *l);
		template<typename T>
		T get_network_state(lua_State *l)
		{
			if constexpr(std::is_pointer_v<T>)
				return static_cast<T>(get_network_state(l));
			else
				return static_cast<T>(*get_network_state(l));
		}
	};
	template<typename T>
	    requires(is_type_or_derived<T, NetworkState>)
	struct default_converter<T> : parameter_emplacement_converter<T, detail::get_network_state<T>> {};

	// Engine
	namespace detail {
		DLLNETWORK Engine *get_engine(lua_State *l);
		template<typename T>
		T get_engine(lua_State *l)
		{
			if constexpr(std::is_pointer_v<T>)
				return static_cast<T>(get_engine(l));
			else
				return static_cast<T>(*get_engine(l));
		}
	};
	template<typename T>
	    requires(is_type_or_derived<T, Engine>)
	struct default_converter<T> : parameter_emplacement_converter<T, detail::get_engine<T>> {};

	// Physics Environment
	namespace detail {
		DLLNETWORK pragma::physics::IEnvironment *get_physics_environment(lua_State *l);
		template<typename T>
		T get_physics_environment(lua_State *l)
		{
			if constexpr(std::is_pointer_v<T>)
				return static_cast<T>(get_physics_environment(l));
			else
				return static_cast<T>(*get_physics_environment(l));
		}
	};
	template<typename T>
	    requires(is_type_or_derived<T, pragma::physics::IEnvironment>)
	struct default_converter<T> : parameter_emplacement_converter<T, detail::get_physics_environment<T>> {};
}

namespace pragma {
	class BaseLuaHandle;
};
namespace pragma::physics {
	class IBase;
};
namespace luabind {
	template<typename T, typename TConverter>
	struct game_object_converter {
		enum { consumed_args = 1 };

		template<class U>
		T to_cpp(lua_State *L, U u, int index);
		void to_lua(lua_State *L, T x);

		template<class U>
		int match(lua_State *L, U u, int index);

		template<class U>
		void converter_postcall(lua_State *, U u, int)
		{
		}
	  private:
		TConverter m_converter;
	};

	template<class T>
	concept IsHandleType = std::derived_from<T, pragma::BaseLuaHandle>;

	template<class T>
	concept IsPhysicsType = std::derived_from<T, pragma::physics::IBase>;

	template<class T>
	concept IsGameObjectType = IsHandleType<T> || IsPhysicsType<T>;

	template<class T> // Note: BaseEntity and derived types are already handled by entity_converter.hpp, so we exclude them here
	concept IsGenericGameObjectType = IsGameObjectType<T> && !
	is_type_or_derived<base_type<T>, BaseEntity>; // && !is_type_or_derived<base_type<T>,pragma::BaseEntityComponent>;

	template<typename T>
	    requires(IsGenericGameObjectType<base_type<T>> && std::is_pointer_v<T> && !std::is_const_v<std::remove_pointer_t<T>>)
	struct default_converter<T> : game_object_converter<T, luabind::detail::pointer_converter> {};
	template<typename T>
	    requires(IsGenericGameObjectType<base_type<T>> && std::is_pointer_v<T> && std::is_const_v<std::remove_pointer_t<T>>)
	struct default_converter<T> : game_object_converter<T, luabind::detail::const_pointer_converter> {};

	template<typename T>
	    requires(IsGenericGameObjectType<base_type<T>> && std::is_reference_v<T> && !std::is_const_v<std::remove_reference_t<T>>)
	struct default_converter<T> : game_object_converter<T, luabind::detail::ref_converter> {};
	template<typename T>
	    requires(IsGenericGameObjectType<base_type<T>> && std::is_reference_v<T> && std::is_const_v<std::remove_reference_t<T>>)
	struct default_converter<T> : game_object_converter<T, luabind::detail::const_ref_converter> {};
};

#endif
