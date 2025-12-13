// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.converters.game_types;

export import :physics.base;
export import :scripting.lua.base_lua_handle;
export import :scripting.lua.types.base_types;
export import :types;

export namespace luabind {
	template<typename T, T (*FUNCTION)(lua::State *)>
	struct parameter_emplacement_converter {
		enum { consumed_args = 0 };

		template<class U>
		T to_cpp(lua::State *L, U, int /*index*/);

		template<class U>
		static int match(lua::State *, U, int /*index*/);

		template<class U>
		void converter_postcall(lua::State *, U, int)
		{
		}
	};

	// Game
	namespace detail {
		DLLNETWORK pragma::Game *get_game(lua::State *l);
		template<typename T>
		T get_game(lua::State *l)
		{
			if constexpr(std::is_pointer_v<T>)
				return static_cast<T>(get_game(l));
			else
				return static_cast<T>(*get_game(l));
		}
	};
	template<typename T>
	    requires(is_type_or_derived<T, pragma::Game>)
	struct default_converter<T> : parameter_emplacement_converter<T, &detail::get_game<T>> {};

	// NetworkState
	namespace detail {
		DLLNETWORK pragma::NetworkState *get_network_state(lua::State *l);
		template<typename T>
		T get_network_state(lua::State *l)
		{
			if constexpr(std::is_pointer_v<T>)
				return static_cast<T>(get_network_state(l));
			else
				return static_cast<T>(*get_network_state(l));
		}
	};
	template<typename T>
	    requires(is_type_or_derived<T, pragma::NetworkState>)
	struct default_converter<T> : parameter_emplacement_converter<T, detail::get_network_state<T>> {};

	// Engine
	namespace detail {
		DLLNETWORK pragma::Engine *get_engine(lua::State *l);
		template<typename T>
		T get_engine(lua::State *l)
		{
			if constexpr(std::is_pointer_v<T>)
				return static_cast<T>(get_engine(l));
			else
				return static_cast<T>(*get_engine(l));
		}
	};
	template<typename T>
	    requires(is_type_or_derived<T, pragma::Engine>)
	struct default_converter<T> : parameter_emplacement_converter<T, detail::get_engine<T>> {};

	// Physics Environment
	namespace detail {
		DLLNETWORK pragma::physics::IEnvironment *get_physics_environment(lua::State *l);
		template<typename T>
		T get_physics_environment(lua::State *l)
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

export namespace luabind {
	template<typename T, typename TConverter>
	struct game_object_converter {
		enum { consumed_args = 1 };

		template<class U>
		T to_cpp(lua::State *L, U u, int index);
		void to_lua(lua::State *L, T x);

		template<class U>
		int match(lua::State *L, U u, int index);

		template<class U>
		void converter_postcall(lua::State *, U u, int)
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

	template<class T>                                                                                                    // Note: BaseEntity and derived types are already handled by entity_converter.hpp, so we exclude them here
	concept IsGenericGameObjectType = IsGameObjectType<T> && !is_type_or_derived<base_type<T>, pragma::ecs::BaseEntity>; // && !is_type_or_derived<base_type<T>,pragma::BaseEntityComponent>;

	template<typename T>
	    requires(IsGenericGameObjectType<base_type<T>> && std::is_pointer_v<T> && !std::is_const_v<std::remove_pointer_t<T>>)
	struct default_converter<T> : game_object_converter<T, detail::pointer_converter> {};
	template<typename T>
	    requires(IsGenericGameObjectType<base_type<T>> && std::is_pointer_v<T> && std::is_const_v<std::remove_pointer_t<T>>)
	struct default_converter<T> : game_object_converter<T, detail::const_pointer_converter> {};

	template<typename T>
	    requires(IsGenericGameObjectType<base_type<T>> && std::is_reference_v<T> && !std::is_const_v<std::remove_reference_t<T>>)
	struct default_converter<T> : game_object_converter<T, detail::ref_converter> {};
	template<typename T>
	    requires(IsGenericGameObjectType<base_type<T>> && std::is_reference_v<T> && std::is_const_v<std::remove_reference_t<T>>)
	struct default_converter<T> : game_object_converter<T, detail::const_ref_converter> {};
};

export namespace luabind {
	template<typename T, T (*FUNCTION)(lua::State *)>
	template<class U>
	T parameter_emplacement_converter<T, FUNCTION>::to_cpp(lua::State *L, U, int /*index*/)
	{
		T p = FUNCTION(L);
		return static_cast<T>(p);
	}

	template<typename T, T (*FUNCTION)(lua::State *)>
	template<class U>
	int parameter_emplacement_converter<T, FUNCTION>::match(lua::State *, U, int /*index*/)
	{
		return 0;
	}

	/////////////

	template<typename T, typename TConverter>
	template<class U>
	T game_object_converter<T, TConverter>::to_cpp(lua::State *L, U u, int index)
	{
		return m_converter.to_cpp(L, u, index);
	}

	template<typename T, typename TConverter>
	void game_object_converter<T, TConverter>::to_lua(lua::State *L, T x)
	{
		if constexpr(std::is_pointer_v<T>) {
			if(!x)
				Lua::PushNil(L);
			else
				const_cast<T>(x)->GetLuaObject(L).push(L);
		}
		else
			const_cast<T>(x).GetLuaObject(L).push(L);
	}

	template<typename T, typename TConverter>
	template<class U>
	int game_object_converter<T, TConverter>::match(lua::State *L, U u, int index)
	{
		return m_converter.match(L, u, index);
	}
}
