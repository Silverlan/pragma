/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/serverstate/serverutil.h"
#include "pragma/game/s_game.h"
#include "pragma/lua/classes/lentity.h"
#include "pragma/lua/classes/lmodelmesh.h"
#include "pragma/lua/classes/s_lentity.h"
#include "pragma/lua/classes/s_lvhcwheel.h"
#include "pragma/lua/classes/lshaderinfo.h"
#include "pragma/lua/classes/lmaterial.h"
#include "pragma/entities/environment/lights/env_light.h"
#include <pragma/entities/environment/lights/env_light_spot.h>
#include "pragma/entities/environment/lights/env_light_point.h"
#include "pragma/entities/environment/lights/env_light_directional.h"
#include "pragma/entities/environment/effects/env_particle_system.h"
#include "pragma/lua/classes/lmodel.h"
#include "pragma/lua/classes/s_lmodel.h"
#include "pragma/ai/ai_task.h"
#include "pragma/lua/classes/s_lai_schedule.h"
#include "pragma/lua/classes/s_lai_behavior.h"
#include <pragma/game/damageinfo.h>
#include "pragma/lua/classes/s_lua_entity.h"
#include "pragma/physics/collision_object.hpp"
#include "pragma/lua/classes/s_lmodelmesh.h"
#include <pragma/entities/func/basefuncwater.h>
#include <pragma/model/modelmesh.h>
#include <luainterface.hpp>
#pragma optimize("",off)
struct ThisIsATestClass
{
	ThisIsATestClass()=default;
};

template<typename T>
	struct TestTemplateClass
{
	TestTemplateClass()=default;
};

/*namespace luabind
{
	template <>
	struct default_converter<TestTemplateClass<ThisIsATestClass>>
		: native_converter_base<TestTemplateClass<ThisIsATestClass>>
	{
		static int compute_score(lua_State* L, int index)
		{
			return lua_type(L, index) == LUA_TNUMBER ? 0 : -1;
		}

		enum { consumed_args = 1 };

		template <typename U>
		TestTemplateClass<ThisIsATestClass> to_cpp(lua_State* L, U u, int index)
		{
			std::cout<<"CONVERT!"<<std::endl;
			return TestTemplateClass<ThisIsATestClass>{};
		}
	};

	template <>
	struct default_converter<TestTemplateClass<ThisIsATestClass> const&>
		: default_converter<TestTemplateClass<ThisIsATestClass>>
	{};
}
*/

#if 0
	template <>
	struct default_converter<bool>
		: native_converter_base<bool>
	{
		static int compute_score(lua_State* L, int index)
		{
			return lua_type(L, index) == LUA_TBOOLEAN ? 0 : no_match;
		}

		static bool to_cpp_deferred(lua_State* L, int index)
		{
			return lua_toboolean(L, index) == 1;
		}

		static void to_lua_deferred(lua_State* L, bool value)
		{
			lua_pushboolean(L, value);
		}
	};
#endif
namespace luabind {
	template<typename T0,typename T1>
	struct default_converter<std::pair<T0,T1>>
		: native_converter_base<std::pair<T0,T1>>
	{
		enum { consumed_args = 2 };

		template <typename U>
		std::pair<T0,T1> to_cpp(lua_State* L, U u, int index)
		{
			std::pair<T0,T1> pair {};
			pair.first = c0.to_cpp(L, decorate_type_t<T0>(), index);
			pair.second = c1.to_cpp(L, decorate_type_t<T1>(), index +1);
			return pair;
		}

		template <class U>
		int match(lua_State *l, U u, int index)
		{
			return (c0.match(l,decorate_type_t<T0>(),index) == 0 && c1.match(l,decorate_type_t<T1>(),index +1) == 0) ? 0 : no_match;
		}

		template <class U>
		void converter_postcall(lua_State*, U u, int) {}

		void to_lua(lua_State* L, std::pair<T0,T1> const& x)
		{
			c0.to_lua(L,x.first);
			c1.to_lua(L,x.second);
		}

		void to_lua(lua_State* L, std::pair<T0,T1>* x)
		{
			if(!x)
				lua_pushnil(L);
			else
				to_lua(L,*x);
		}
	private:
		default_converter<T0> c0;
		default_converter<T1> c1;
	};

	template<typename T0,typename T1>
	struct default_converter< const std::pair<T0,T1> >
		: default_converter< std::pair<T0,T1> >
	{ };

	template<typename T0,typename T1>
	struct default_converter<std::pair<T0,T1> const&>
		: default_converter<std::pair<T0,T1>>
	{};
}

namespace luabind {
	template <class ...T>
	struct default_converter<std::tuple<T...>>
		: native_converter_base<std::tuple<T...>>
	{
		enum { consumed_args = sizeof...(T) };

		template<size_t I = 0, typename... Tp>
		int match_all(lua_State *L,int index,std::tuple<default_converter<Tp>...>&) { // tuple parameter is unused but required for overload resolution for some reason
			using T = std::tuple_element<I, std::tuple<Tp...> >::type;
				if(std::get<I>(m_converters).match(L,decorate_type_t<base_type<T>>(),index) != 0)
					return no_match;
			 if constexpr(I+1 != sizeof...(Tp))
				return match_all<I +1>(L,index +1,m_converters);
			 return 0;
		}

		template<size_t I = 0, typename... Tp>
		void to_lua_all(lua_State *L,const std::tuple<Tp...>& t) {
			 std::get<I>(m_converters).to_lua(L,std::get<I>(t));
			if constexpr(I+1 != sizeof...(Tp))
				to_lua_all<I+1>(L,t);
		}

		template<size_t I = 0, typename... Tp>
		void to_cpp_all(lua_State *L,int index,std::tuple<Tp...>& t) {
			auto &v = std::get<I>(t);
			v = std::get<I>(m_converters).to_cpp(L,decorate_type_t<base_type<decltype(v)>>(),index++);
			if constexpr(I+1 != sizeof...(Tp))
				to_cpp_all<I+1>(L,index,t);
		}

		template <typename U>
		std::tuple<T...> to_cpp(lua_State* L, U u, int index)
		{
			std::tuple<T...> tuple {};
			to_cpp_all(L,index,tuple);
			return tuple;
		}

		template <class U>
		int match(lua_State *l, U u, int index)
		{
			return match_all<0,T...>(l,index,m_converters);
		}

		template <class U>
		void converter_postcall(lua_State*, U u, int) {}

		void to_lua(lua_State* L, std::tuple<T...> const& x)
		{
			to_lua_all(L,x);
		}

		void to_lua(lua_State* L, std::tuple<T...>* x)
		{
			if(!x)
				lua_pushnil(L);
			else
				to_lua(L,*x);
		}
	private:
		std::tuple<default_converter<T>...> m_converters;
	};

	template <class ...T>
	struct default_converter< const std::tuple<T...> >
		: default_converter< std::tuple<T...> >
	{ };

	template <class ...T>
	struct default_converter<std::tuple<T...> const&>
		: default_converter<std::tuple<T...>>
	{};
}

namespace luabind {
	template<>
	struct default_converter<std::string_view>
		: native_converter_base<std::string_view>
	{
		enum { consumed_args = 1 };

		template <typename U>
		std::string_view to_cpp(lua_State* L, U u, int index)
		{
			return {luaL_checkstring(L,index)};
		}

		template <class U>
		static int match(lua_State *l, U u, int index)
		{
			return lua_isstring(l,index) ? 0 : no_match;
		}

		template <class U>
		void converter_postcall(lua_State*, U u, int) {}
		
		void to_lua(lua_State* L, std::string_view const& x)
		{
			luabind::object{L,std::string{x}}.push(L);
		}

		void to_lua(lua_State* L, std::string_view* x)
		{
			if(!x)
				lua_pushnil(L);
			else
				to_lua(L,*x);
		}
	};

	template<>
	struct default_converter< const std::string_view >
		: default_converter< std::string_view >
	{ };

	template<>
	struct default_converter<std::string_view const&>
		: default_converter<std::string_view>
	{};
}

namespace luabind {
	template<typename T>
	struct default_converter<std::optional<T>>
		: native_converter_base<std::optional<T>>
	{
		enum { consumed_args = 1 };

		template <typename U>
		std::optional<T> to_cpp(lua_State* L, U u, int index)
		{
			if(lua_isnil(L,index))
				return {};
			return m_converter.to_cpp(L,decorate_type_t<T>(),index);
		}

		template <class U>
		int match(lua_State *l, U u, int index)
		{
			if(lua_isnil(l,index))
				return 0;
			return m_converter.match(l,decorate_type_t<T>(),index);
		}

		template <class U>
		void converter_postcall(lua_State*, U u, int) {}
		
		void to_lua(lua_State* L, std::optional<T> const& x)
		{
			if(!x.has_value())
				lua_pushnil(L);
			else
				m_converter.to_lua(L,*x);
		}

		void to_lua(lua_State* L, std::optional<T>* x)
		{
			if(!x || !x->has_value())
				lua_pushnil(L);
			else
				to_lua(L,*x);
		}
	private:
		default_converter<T> m_converter;
	};

	template<typename T>
	struct default_converter< const std::optional<T> >
		: default_converter< std::optional<T> >
	{ };

	template<typename T>
	struct default_converter<std::optional<T> const&>
		: default_converter<std::optional<T>>
	{};
}

namespace luabind {
	template<typename T>
	struct default_converter<std::vector<T>>
		: native_converter_base<std::vector<T>>
	{
		enum { consumed_args = 1 };

		template <typename U>
		std::vector<T> to_cpp(lua_State* L, U u, int index)
		{
			default_converter<T> converter;

			auto o = luabind::object{luabind::from_stack(L,index)};
			auto n = Lua::GetObjectLength(L,o);
			std::vector<T> v;
			v.reserve(n);
			for(luabind::iterator it{o},end;it!=end;++it)
			{
				luabind::object o = *it;
				o.push(L);
				if(converter.match(L,decorate_type_t<T>(),-1) != no_match)
					v.push_back(converter.to_cpp(L, decorate_type_t<T>(), -1));
				lua_pop(L,1);
			}
			return v;
		}

		template <class U>
		static int match(lua_State *l, U u, int index)
		{
			return lua_istable(l,index) ? 0 : no_match;
		}

		template <class U>
		void converter_postcall(lua_State*, U u, int) {}
		
		void to_lua(lua_State* L, std::vector<T> const& x)
		{
			default_converter<T> converter;
			auto t = luabind::newtable(L);
			t.push(L);
			int index = 1;

			for(const auto& element : x)
			{
				if constexpr(std::is_fundamental_v<decltype(element)>)
					t[index] = element;
				else
				{
					converter.to_lua(L, element);
					lua_rawseti(L, -2, index);
				}
				++index;
			}
		}

		void to_lua(lua_State* L, std::vector<T>* x)
		{
			if(!x)
				luabind::newtable(L).push(L);
			else
				to_lua(L,*x);
		}
	};

	template<typename T>
	struct default_converter< const std::vector<T> >
		: default_converter< std::vector<T> >
	{ };

	template<typename T>
	struct default_converter<std::vector<T> const&>
		: default_converter<std::vector<T>>
	{};
}

namespace luabind {
	template<typename T,size_t SIZE>
	struct default_converter<std::array<T,SIZE>>
		: native_converter_base<std::array<T,SIZE>>
	{
		enum { consumed_args = 1 };

		template <typename U>
		std::array<T,SIZE> to_cpp(lua_State* L, U u, int index)
		{
			default_converter<T> converter;

			auto o = luabind::object{luabind::from_stack(L,index)};
			auto n = Lua::GetObjectLength(L,o);
			assert(n == SIZE);
			if(n > SIZE)
				n = SIZE;
			std::array<T,SIZE> v;
			uint32_t i = 0;
			for(luabind::iterator it{o},end;it!=end;++it)
			{
				luabind::object o = *it;
				o.push(L);
				if(converter.match(L,decorate_type_t<T>(),-1) != no_match)
					v[i] = converter.to_cpp(L, decorate_type_t<T>(), -1);
				lua_pop(L,1);

				++i;
			}
			return v;
		}

		template <class U>
		static int match(lua_State *l, U u, int index)
		{
			if(!lua_istable(l,index))
				return no_match;
			auto n = Lua::GetObjectLength(l,index);
			if(n != SIZE)
				return no_match;
			return 0;
		}

		template <class U>
		void converter_postcall(lua_State*, U u, int) {}
		
		void to_lua(lua_State* L, std::array<T,SIZE> const& x)
		{
			default_converter<T> converter;
			auto t = luabind::newtable(L);
			t.push(L);
			int index = 1;

			for(const auto& element : x)
			{
				if constexpr(std::is_fundamental_v<decltype(element)>)
					t[index] = element;
				else
				{
					converter.to_lua(L, element);
					lua_rawseti(L, -2, index);
				}
				++index;
			}
		}

		void to_lua(lua_State* L, std::array<T,SIZE>* x)
		{
			if(!x)
				luabind::newtable(L).push(L);
			else
				to_lua(L,*x);
		}
	};

	template<typename T,size_t SIZE>
	struct default_converter< const std::array<T,SIZE> >
		: default_converter< std::array<T,SIZE> >
	{ };

	template<typename T,size_t SIZE>
	struct default_converter<std::array<T,SIZE> const&>
		: default_converter<std::array<T,SIZE>>
	{};
}

namespace luabind {

	template <typename T>
	using base_type = typename std::remove_cv_t<std::remove_pointer_t<std::remove_reference_t<T>>>;

	template <class T,class Test>
	concept is_type_or_derived = std::is_same_v<base_type<T>,Test> || std::derived_from<base_type<T>,Test>;

	// Game
	template <typename T> requires(is_type_or_derived<T,Game>)
	struct default_converter<T>
	{
		enum { consumed_args = 0 };

		template <class U>
		T to_cpp(lua_State* L, U, int /*index*/)
		{
			auto *game = pragma::get_engine()->GetNetworkState(L)->GetGameState();
			if constexpr(std::is_pointer_v<T>)
				return static_cast<T>(game);
			else
				return static_cast<T>(*game);
		}

		template <class U>
		static int match(lua_State*, U, int /*index*/) {return 0;}

		template <class U>
		void converter_postcall(lua_State*, U, int) {}
	};

	// NetworkState
	template <typename T> requires(is_type_or_derived<T,NetworkState>)
	struct default_converter<T>
	{
		enum { consumed_args = 0 };

		template <class U>
		T to_cpp(lua_State* L, U, int /*index*/)
		{
			auto *nw = pragma::get_engine()->GetNetworkState(L);
			if constexpr(std::is_pointer_v<T>)
				return static_cast<T>(nw);
			else
				return static_cast<T>(*nw);
		}

		template <class U>
		static int match(lua_State*, U, int /*index*/) {return 0;}

		template <class U>
		void converter_postcall(lua_State*, U, int) {}
	};

	// NetworkState
	template <typename T> requires(is_type_or_derived<T,Engine>)
	struct default_converter<T>
	{
		using Test = std::remove_cv_t<std::remove_pointer_t<std::remove_reference_t<T>>>;
		enum { consumed_args = 0 };

		template <class U>
		T to_cpp(lua_State* L, U, int /*index*/)
		{
			auto *en = pragma::get_engine();
			if constexpr(std::is_pointer_v<T>)
				return static_cast<T>(en);
			else
				return static_cast<T>(*en);
		}

		template <class U>
		static int match(lua_State*, U, int /*index*/) {return 0;}

		template <class U>
		void converter_postcall(lua_State*, U, int) {}
	};

	/*namespace detail {

		// This is the one that gets hit, if default_policy doesn't hit one of the specializations defined all over the place
		template< class T >
		struct default_converter_generator
			: public meta::select_ <
			meta::case_< is_lua_proxy_arg<T>, lua_proxy_converter<T> >,
			meta::case_< std::is_enum<typename std::remove_reference<T>::type>, enum_converter >,
			meta::case_< is_nonconst_pointer<T>, pointer_converter >,
			meta::case_< is_const_pointer<T>, const_pointer_converter >,
			meta::case_< is_nonconst_reference<T>, ref_converter >,
			meta::case_< is_const_reference<T>, const_ref_converter >,
			meta::default_< value_converter >
			> ::type
		{
		};

	}

	template <class T, class Enable>
	struct default_converter
		: detail::default_converter_generator<T>::type
	{};*/
}

#include "pragma/lua/libraries/lfile.h"
namespace luabind {
	LFile;
	template<>
	struct default_converter<std::shared_ptr<VFilePtrInternal> >
		: default_converter<VFilePtrInternal*>
	{
		using is_native = std::false_type;

		template <class U>
		int match(lua_State* L, U, int index)
		{
			return Lua::IsFile(L,index);
		}

		template <class U>
		std::shared_ptr<VFilePtrInternal> to_cpp(lua_State* L, U u, int index)
		{
			auto *f = luabind::object_cast<LFile*>(luabind::object{luabind::from_stack(L,index)});
			return f->GetHandle();
		}

		void to_lua(lua_State* L, std::shared_ptr<VFilePtrInternal> const& p)
		{
			auto f = std::make_shared<LFile>();
			f->Construct(p);
			default_converter<std::shared_ptr<LFile>>{}.to_lua(L,f);
		}

		template <class U>
		void converter_postcall(lua_State*, U const&, int)
		{}
	};


	/*template <typename T> requires(std::is_same_v<base_type<T>,VFilePtr> || std::derived_from<base_type<T>,VFilePtr>)
	struct default_converter<T>
	{
		enum { consumed_args = 1 };

		VFilePtr to_cpp(lua_State* L, luabind::by_reference<T> u, int index)
		{
			return const_cast<LFile&>(u).GetHandle();
		}

		template <class U>
		static int match(lua_State *l, U u, int index)
		{
			return Lua::IsType<LFile>(l,index) ? 0 : no_match;
		}

		template <class U>
		void converter_postcall(lua_State*, U u, int) {}
	};*/
};

namespace luabind
{
	template <typename T,typename TConverter>
	struct game_object_converter
	{
		enum { consumed_args = 1 };

		template <class U>
		T to_cpp(lua_State* L, U u, int index)
		{
			return m_converter.to_cpp(L,u,index);
		}

		void to_lua(lua_State* L, T x)
		{
			if constexpr(std::is_pointer_v<T>)
			{
				if(!x)
					lua_pushnil(L);
				else
					const_cast<T>(x)->GetLuaObject().push(L);
			}
			else
				const_cast<T>(x).GetLuaObject().push(L);
		}

		template <class U>
		int match(lua_State* L, U u, int index)
		{
			return m_converter.match(L,u,index);
		}

		template <class U>
		void converter_postcall(lua_State*, U u, int) {}
	private:
		TConverter m_converter;
	};

	template <class T>
		concept IsEntity = is_type_or_derived<T,BaseEntity>;

	template <class T>
		concept IsEntityComponent = is_type_or_derived<T,pragma::BaseEntityComponent>;

	template <class T>
		concept IsGameObjectType = IsEntity<T> || IsEntityComponent<T>;

	template <typename T> requires(IsGameObjectType<T> && std::is_pointer_v<T> && !std::is_const_v<std::remove_pointer_t<T>>)
	struct default_converter<T>
		: game_object_converter<T,luabind::detail::pointer_converter>
	{};
	template <typename T> requires(IsGameObjectType<T> && std::is_pointer_v<T> && std::is_const_v<std::remove_pointer_t<T>>)
	struct default_converter<T>
		: game_object_converter<T,luabind::detail::const_pointer_converter>
	{};

	template <typename T> requires(IsGameObjectType<T> && std::is_reference_v<T> && !std::is_const_v<std::remove_reference_t<T>>)
	struct default_converter<T>
		: game_object_converter<T,luabind::detail::ref_converter>
	{};
	template <typename T> requires(IsGameObjectType<T> && std::is_reference_v<T> && std::is_const_v<std::remove_reference_t<T>>)
	struct default_converter<T>
		: game_object_converter<T,luabind::detail::const_ref_converter>
	{};
};

#include "pragma/lua/classes/lproperty.hpp"
namespace luabind
{
	template <typename T> requires(std::derived_from<base_type<T>,util::BaseProperty> || (util::is_specialization<T,std::shared_ptr>::value && std::derived_from<base_type<typename T::element_type>,util::BaseProperty>))
	struct default_converter<T>
	{
		enum { consumed_args = 1 };

		void to_lua(lua_State* L, T x)
		{
			if constexpr(std::is_pointer_v<T> || util::is_specialization<T,std::shared_ptr>::value)
			{
				if(!x)
					lua_pushnil(L);
				else
					Lua::Property::push(L,*const_cast<T>(x));
			}
			else
				Lua::Property::push(L,const_cast<T>(x));
		}
	};
};

namespace util
{
	template<typename T>
	class TestHandle
		: public TWeakSharedHandle<T>
	{
	public:
		TestHandle()=default;
		TestHandle(const TWeakSharedHandle<T> &h)
			: TWeakSharedHandle<T>{h}
		{}
	};
};

// TODO: Converter -> SBaseEntity / CBaseEntyity/ BaseENtity -> Then check handles again

	namespace luabind {
		/*namespace detail {
			template<typename T>
			struct pointer_traits<util::TestHandle<T>>
			{
				enum { is_pointer = true };
				using value_type = T;
			};
		};*/
		/*template <class T> requires(util::is_specialization<T,util::TestHandle>::value && is_type_or_derived<base_type<typename T::value_type>,BaseEntity>)
		struct default_converter<T>
		{
			enum { consumed_args = 1 };
			using is_native = std::false_type;
			template <class U>
			int match(lua_State* L, U, int index)
			{
				std::cout<<"Match check..."<<std::endl;
				return m_converter.match(L, decorate_type_t<T*>(), index);
			}
			template <class U>
			T to_cpp(lua_State* L, U, int index)
			{
				T* raw_ptr = m_converter.to_cpp(L, decorate_type_t<T*>(), index);
				if(!raw_ptr) {
					return T{};
				} else {
					//auto h = raw_ptr->GetHandle();
					//return util::weak_shared_handle_cast<decltype(h)::value_type,T>(h);
					return *raw_ptr;
				}
			}
			void to_lua(lua_State* L, T const& p)
			{
				default_converter<T>().to_lua(L, p);
			}
			template <class U>
			void converter_postcall(lua_State*, U const&, int)
			{}
		default_converter<T*> m_converter;
		};*/
		//template <class T>
		//struct default_converter<util::TestHandle<T> const&>
		//	: default_converter<util::TestHandle<T> >
		//{};

	template <typename T> requires(util::is_specialization<base_type<T>,util::TestHandle>::value && IsGameObjectType<typename T::value_type>)
	struct handle_converter
	{
		enum { consumed_args = 1 };

		template <class U>
		T to_cpp(lua_State* L, U u, int index)
		{
			return m_converter.to_cpp(L,u,index);
		}

		void to_lua(lua_State* L, T x)
		{
			if constexpr(std::is_pointer_v<T>)
			{
				if(!x)
					lua_pushnil(L);
				else
					const_cast<T>(x)->GetLuaObject().push(L);
			}
			else
				const_cast<T>(x).GetLuaObject().push(L);
		}

		template <class U>
		int match(lua_State* L, U u, int index)
		{
			return m_converter.match(L,u,index);
		}

		template <class U>
		void converter_postcall(lua_State*, U u, int) {}
	private:
		TConverter m_converter;
	};

	template <class T>
		concept IsEntity = is_type_or_derived<T,BaseEntity>;

	template <class T>
		concept IsEntityComponent = is_type_or_derived<T,pragma::BaseEntityComponent>;

	template <class T>
		concept IsGameObjectType = IsEntity<T> || IsEntityComponent<T>;

	template <typename T> requires(IsGameObjectType<T> && std::is_pointer_v<T> && !std::is_const_v<std::remove_pointer_t<T>>)
	struct default_converter<T>
		: game_object_converter<T,luabind::detail::pointer_converter>
	{};
	template <typename T> requires(IsGameObjectType<T> && std::is_pointer_v<T> && std::is_const_v<std::remove_pointer_t<T>>)
	struct default_converter<T>
		: game_object_converter<T,luabind::detail::const_pointer_converter>
	{};
	}

using EntityHandleT = util::TestHandle<BaseEntity>;
#include <pragma/entities/components/velocity_component.hpp>
#include <pragma/entities/entity_iterator.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
void SGame::RegisterLuaClasses()
{
	Game::RegisterLuaClasses();

	auto &modGame = GetLuaInterface().RegisterLibrary("game");
	
	auto &modTestConverters = GetLuaInterface().RegisterLibrary("test_converters");
	modTestConverters[
		luabind::def("test_pair_param",static_cast<void(*)(const std::pair<std::string,float>&)>([](const std::pair<std::string,float> &pair) {
			Con::cout<<"Pair: "<<pair.first<<","<<pair.second<<Con::endl;
		})),
		luabind::def("test_pair_ret",static_cast<std::pair<std::string,float>(*)()>([]() -> std::pair<std::string,float> {
			return {"TestString",1357.f};
		})),

		luabind::def("test_tuple_param",static_cast<void(*)(const std::tuple<std::string,float,Vector3>&)>([](const std::tuple<std::string,float,Vector3> &tuple) {
			Con::cout<<"Tuple: "<<std::get<0>(tuple)<<","<<std::get<1>(tuple)<<","<<std::get<2>(tuple)<<Con::endl;
		})),
		luabind::def("test_tuple_ret",static_cast<std::tuple<std::string,float,Vector3>(*)()>([]() -> std::tuple<std::string,float,Vector3> {
			return {"TestString",1357.f,Vector3{5,87,99}};
		})),

		luabind::def("test_string_view_param",static_cast<void(*)(const std::string_view&)>([](const std::string_view &str) {
			Con::cout<<"string_view: "<<str<<Con::endl;
		})),
		luabind::def("test_string_view_ret",static_cast<std::string_view(*)()>([]() -> std::string_view {
			static std::string str = "TestStringView";
			return str;
		})),

		luabind::def("test_optional_param",static_cast<void(*)(const std::optional<Vector3>&)>([](const std::optional<Vector3> &val) {
			Con::cout<<"optional: ";
			if(!val.has_value())
				Con::cout<<"<no value>";
			else
				Con::cout<<*val;
			Con::cout<<Con::endl;
		})),
		luabind::def("test_optional_ret",static_cast<std::optional<Vector3>(*)(int)>([](int i) -> std::optional<Vector3> {
			if(i == 0)
				return {};
			return Vector3{55,66,77};
		})),

		luabind::def("test_vector_param",static_cast<void(*)(const std::vector<Vector3>&)>([](const std::vector<Vector3> &val) {
			Con::cout<<"values: ";
			for(auto &v : val)
				Con::cout<<v<<Con::endl;
		})),
		luabind::def("test_vector_ret",static_cast<std::vector<Vector3>(*)()>([]() -> std::vector<Vector3> {
			return {Vector3{1,2,3},Vector3{6,5,4},Vector3{8,7,8},Vector3{1,1,1}};
		})),

		luabind::def("test_array_param",static_cast<void(*)(const std::array<Vector3,3>&)>([](const std::array<Vector3,3> &val) {
			Con::cout<<"values: ";
			for(auto &v : val)
				Con::cout<<v<<Con::endl;
		})),
		luabind::def("test_array_ret",static_cast<std::array<Vector3,3>(*)()>([]() -> std::array<Vector3,3> {
			return {Vector3{1,2,3},Vector3{6,5,4},Vector3{8,7,8}};
		})),

		luabind::def("test_entity_param",static_cast<void(*)(BaseEntity*)>([](BaseEntity *ent) {
			Con::cout<<"Entity: "<<*ent<<Con::endl;
		})),
		luabind::def("test_entity_ret",static_cast<BaseEntity*(*)(Game&)>([](Game &game) -> BaseEntity* {
			return &game.GetWorld()->GetEntity();
		})),

		luabind::def("test_entity_handle_test_get",static_cast<EntityHandleT(*)(Game&)>([](Game &game) -> EntityHandleT {
			auto hTest = game.GetWorld()->GetEntity().GetHandle();
			std::cout<<hTest.get()<<std::endl;
			return hTest;
		})),

		luabind::def("test_entity_handle_param",static_cast<void(*)(EntityHandleT)>([](EntityHandleT ent) {

			//luabind::default_converter<EntityHandleT> x;
			//x.to_cpp(nullptr,luabind::by_reference<EntityHandleT>{},0);

			constexpr auto b  = util::is_specialization<EntityHandleT,util::TestHandle>::value;
			constexpr auto c = luabind::is_type_or_derived<luabind::base_type<typename EntityHandleT::value_type>,BaseEntity>;

			Con::cout<<"Entity: "<<ent->GetClass()<<Con::endl;
		})),
		luabind::def("test_entity_handle_const_param",static_cast<void(*)(const EntityHandleT&)>([](const EntityHandleT &ent) {
			Con::cout<<"Entity: "<<ent->GetClass()<<Con::endl;
		})),
		luabind::def("test_entity_handle_const_pointer_param",static_cast<void(*)(const EntityHandleT*)>([](const EntityHandleT *ent) {
			Con::cout<<"Entity: "<<(*ent)->GetClass()<<Con::endl;
		})),
		luabind::def("test_entity_handle_ret",static_cast<EntityHandleT(*)(Game&)>([](Game &game) -> EntityHandleT {
			return game.GetWorld()->GetEntity().GetHandle();
		})),
		luabind::def("test_entity_handle_const_ret",static_cast<const EntityHandleT(*)(Game&)>([](Game &game) -> const EntityHandleT {
			return game.GetWorld()->GetEntity().GetHandle();
		})),

		luabind::def("test_entity_const_param",static_cast<void(*)(const BaseEntity*)>([](const BaseEntity *ent) {
			Con::cout<<"Entity: "<<const_cast<BaseEntity&>(*ent)<<Con::endl;
		})),
		luabind::def("test_entity_const_ret",static_cast<const BaseEntity*(*)(Game&)>([](Game &game) -> const BaseEntity* {
			return &game.GetWorld()->GetEntity();
		})),

		luabind::def("test_entity_null_param",static_cast<void(*)(BaseEntity*)>([](BaseEntity *ent) {
			Con::cout<<"Entity: "<<ent<<Con::endl;
		})),
		luabind::def("test_entity_null_ret",static_cast<BaseEntity*(*)(Game&)>([](Game &game) -> BaseEntity* {
			return nullptr;
		})),

		luabind::def("test_entity_ref_param",static_cast<void(*)(BaseEntity&)>([](BaseEntity &ent) {
			Con::cout<<"Entity: "<<ent<<Con::endl;
		})),
		luabind::def("test_entity_ref_ret",static_cast<BaseEntity&(*)(Game&)>([](Game &game) -> BaseEntity& {
			return game.GetWorld()->GetEntity();
		})),

		luabind::def("test_entity_ref_const_param",static_cast<void(*)(const BaseEntity&)>([](const BaseEntity &ent) {
			Con::cout<<"Entity: "<<const_cast<BaseEntity&>(ent)<<Con::endl;
		})),
		luabind::def("test_entity_ref_const_ret",static_cast<const BaseEntity&(*)(Game&)>([](Game &game) -> const BaseEntity& {
			return game.GetWorld()->GetEntity();
		})),

		luabind::def("test_component_param",static_cast<void(*)(pragma::VelocityComponent*)>([](pragma::VelocityComponent *c) {
			Con::cout<<"Component: "<<c->GetVelocity()<<Con::endl;
		})),
		luabind::def("test_component_ret",static_cast<pragma::VelocityComponent*(*)(Game&)>([](Game &game) -> pragma::VelocityComponent* {
			EntityIterator entIt {game,EntityIterator::FilterFlags::Default | EntityIterator::FilterFlags::Pending};
			entIt.AttachFilter<TEntityIteratorFilterComponent<pragma::VelocityComponent>>();
			auto it = entIt.begin();
			if(it == entIt.end())
				return nullptr;
			return it->GetComponent<pragma::VelocityComponent>().get();
		})),

		luabind::def("test_component_handle_param",static_cast<void(*)(pragma::ComponentHandle<pragma::VelocityComponent>&)>([](pragma::ComponentHandle<pragma::VelocityComponent> &c) {
			Con::cout<<"Component: "<<c->GetVelocity()<<Con::endl;
		})),
		luabind::def("test_component_handle_ret",static_cast<pragma::ComponentHandle<pragma::VelocityComponent>(*)(Game&)>([](Game &game) -> pragma::ComponentHandle<pragma::VelocityComponent> {
			EntityIterator entIt {game,EntityIterator::FilterFlags::Default | EntityIterator::FilterFlags::Pending};
			entIt.AttachFilter<TEntityIteratorFilterComponent<pragma::VelocityComponent>>();
			auto it = entIt.begin();
			if(it == entIt.end())
				return pragma::ComponentHandle<pragma::VelocityComponent>{};
			return it->GetComponent<pragma::VelocityComponent>();
		})),
		luabind::def("test_component_handle_null_ret",static_cast<pragma::ComponentHandle<pragma::VelocityComponent>(*)(Game&)>([](Game &game) -> pragma::ComponentHandle<pragma::VelocityComponent> {
			return {};
		})),

		luabind::def("test_hidden_param",static_cast<void(*)(Game&,NetworkState&,Engine&)>([](Game &game,NetworkState &nw,Engine &en) {
			Con::cout<<"Game Map: "<<game.GetMapName()<<Con::endl;
			Con::cout<<"NetworkState material count: "<<nw.GetMaterialManager().GetMaterials().size()<<Con::endl;
			Con::cout<<"Last engine tick: "<<en.GetLastTick()<<Con::endl;
		})),

		luabind::def("test_file",static_cast<void(*)(VFilePtr)>([](VFilePtr f) {
			Con::cout<<"File size: "<<f->GetSize()<<Con::endl;
		})),

		luabind::def("test_property_ret",static_cast<util::PFloatProperty(*)()>([]() {
			return util::FloatProperty::Create(3.f);
		}))
	];

	auto materialClassDef = luabind::class_<Material>("Material");
	Lua::Material::register_class(materialClassDef);
	materialClassDef.def("SetShader",static_cast<void(*)(lua_State*,::Material&,const std::string&)>([](lua_State *l,::Material &mat,const std::string &shader) {
		auto db = mat.GetDataBlock();
		if(db == nullptr)
			return;
		mat.Initialize(shader,db);
		mat.SetLoaded(true);
	}));
	modGame[materialClassDef];

	auto modelMeshClassDef = luabind::class_<ModelMesh>("Mesh");
	Lua::ModelMesh::register_class(modelMeshClassDef);
	modelMeshClassDef.scope[luabind::def("Create",&Lua::ModelMesh::Server::Create)];

	auto subModelMeshClassDef = luabind::class_<ModelSubMesh>("Sub");
	Lua::ModelSubMesh::register_class(subModelMeshClassDef);
	subModelMeshClassDef.scope[luabind::def("Create",&Lua::ModelSubMesh::Server::Create)];
	subModelMeshClassDef.scope[luabind::def("CreateQuad",&Lua::ModelSubMesh::Server::CreateQuad)];
	subModelMeshClassDef.scope[luabind::def("CreateBox",&Lua::ModelSubMesh::Server::CreateBox)];
	subModelMeshClassDef.scope[luabind::def("CreateSphere",static_cast<void(*)(lua_State*,const Vector3&,float,uint32_t)>(&Lua::ModelSubMesh::Server::CreateSphere))];
	subModelMeshClassDef.scope[luabind::def("CreateSphere",static_cast<void(*)(lua_State*,const Vector3&,float)>(&Lua::ModelSubMesh::Server::CreateSphere))];
	subModelMeshClassDef.scope[luabind::def("CreateCylinder",static_cast<void(*)(lua_State*,float,float,uint32_t)>(&Lua::ModelSubMesh::Server::CreateCylinder))];
	subModelMeshClassDef.scope[luabind::def("CreateCylinder",static_cast<void(*)(lua_State*,float,float)>([](lua_State *l,float startRadius,float length) {
		Lua::ModelSubMesh::Server::CreateCylinder(l,startRadius,length,12);
	}))];
	subModelMeshClassDef.scope[luabind::def("CreateCone",static_cast<void(*)(lua_State*,float,float,float,uint32_t)>(&Lua::ModelSubMesh::Server::CreateCone))];
	subModelMeshClassDef.scope[luabind::def("CreateCone",static_cast<void(*)(lua_State*,float,float,float)>([](lua_State *l,float startRadius,float length,float endRadius) {
		Lua::ModelSubMesh::Server::CreateCone(l,startRadius,length,endRadius,12);
	}))];
	subModelMeshClassDef.scope[luabind::def("CreateCircle",static_cast<void(*)(lua_State*,float,bool,uint32_t)>(&Lua::ModelSubMesh::Server::CreateCircle))];
	subModelMeshClassDef.scope[luabind::def("CreateCircle",static_cast<void(*)(lua_State*,float,bool)>([](lua_State *l,float radius,bool doubleSided) {
		Lua::ModelSubMesh::Server::CreateCircle(l,radius,doubleSided,36);
	}))];
	subModelMeshClassDef.scope[luabind::def("CreateCircle",static_cast<void(*)(lua_State*,float)>([](lua_State *l,float radius) {
		Lua::ModelSubMesh::Server::CreateCircle(l,radius,true,36);
	}))];
	subModelMeshClassDef.scope[luabind::def("CreateRing",static_cast<void(*)(lua_State*,float,float,bool,uint32_t)>(&Lua::ModelSubMesh::Server::CreateRing))];
	subModelMeshClassDef.scope[luabind::def("CreateRing",static_cast<void(*)(lua_State*,float,float,bool)>([](lua_State *l,float innerRadius,float outerRadius,bool doubleSided) {
		Lua::ModelSubMesh::Server::CreateRing(l,innerRadius,outerRadius,doubleSided,36);
	}))];
	subModelMeshClassDef.scope[luabind::def("CreateRing",static_cast<void(*)(lua_State*,float,float)>([](lua_State *l,float innerRadius,float outerRadius) {
		Lua::ModelSubMesh::Server::CreateRing(l,innerRadius,outerRadius,true,36);
	}))];

	auto modelClassDef = luabind::class_<Model>("Model");
	Lua::Model::register_class(GetLuaState(),modelClassDef,modelMeshClassDef,subModelMeshClassDef);
	modelClassDef.def("AddMaterial",&Lua::Model::Server::AddMaterial);
	modelClassDef.def("SetMaterial",&Lua::Model::Server::SetMaterial);
	modGame[modelClassDef];
	auto _G = luabind::globals(GetLuaState());
	_G["Model"] = _G["game"]["Model"];
	_G["Animation"] = _G["game"]["Model"]["Animation"];

	_G["Entity"] = _G["ents"]["Entity"];
	_G["BaseEntity"] = _G["ents"]["BaseEntity"];

	_G["BaseEntityComponent"] = _G["ents"]["BaseEntityComponent"];

	// COMPONENT TODO
	/*auto charClassDef = luabind::class_<pragma::BaseCharacterComponent>("CharacterComponent");
	Lua::BaseCharacter::register_class(charClassDef);
	modGame[charClassDef];

	auto playerClassDef = luabind::class_<PlayerHandle,EntityHandle>("Player");
	Lua::Player::Server::register_class(playerClassDef);
	modGame[playerClassDef];

	auto defWeapon = luabind::class_<WeaponHandle,EntityHandle>("Weapon");
	LUA_CLASS_WEAPON_SHARED(defWeapon);
	modGame[defWeapon];

	auto npcClassDef = luabind::class_<NPCHandle,EntityHandle>("NPC");
	Lua::NPC::Server::register_class(npcClassDef);
	modGame[npcClassDef];

	auto vehicleClassDef = luabind::class_<VehicleHandle,EntityHandle>("Vehicle");
	Lua::Vehicle::Server::register_class(vehicleClassDef);

	auto vhcWheelClassDef = luabind::class_<VHCWheelHandle,EntityHandle>("Wheel")
	LUA_CLASS_VHCWHEEL_SHARED;
	vhcWheelClassDef.def("SetFrontWheel",&Lua_VHCWheel_SetFrontWheel);
	vhcWheelClassDef.def("SetChassisConnectionPoint",&Lua_VHCWheel_SetChassisConnectionPoint);
	vhcWheelClassDef.def("SetWheelAxle",&Lua_VHCWheel_SetWheelAxle);
	vhcWheelClassDef.def("SetWheelDirection",&Lua_VHCWheel_SetWheelDirection);
	vhcWheelClassDef.def("SetMaxSuspensionLength",&Lua_VHCWheel_SetMaxSuspensionLength);
	vhcWheelClassDef.def("SetMaxDampingRelaxation",&Lua_VHCWheel_SetMaxDampingRelaxation);
	vhcWheelClassDef.def("SetMaxSuspensionCompression",&Lua_VHCWheel_SetMaxSuspensionCompression);
	vhcWheelClassDef.def("SetWheelRadius",&Lua_VHCWheel_SetWheelRadius);
	vhcWheelClassDef.def("SetSuspensionStiffness",&Lua_VHCWheel_SetSuspensionStiffness);
	vhcWheelClassDef.def("SetWheelDampingCompression",&Lua_VHCWheel_SetWheelDampingCompression);
	vhcWheelClassDef.def("SetFrictionSlip",&Lua_VHCWheel_SetFrictionSlip);
	vhcWheelClassDef.def("SetSteeringAngle",&Lua_VHCWheel_SetSteeringAngle);
	vhcWheelClassDef.def("SetWheelRotation",&Lua_VHCWheel_SetWheelRotation);
	vhcWheelClassDef.def("SetRollInfluence",&Lua_VHCWheel_SetRollInfluence);
	vehicleClassDef.scope[vhcWheelClassDef];
	modGame[vehicleClassDef];

	// Custom Classes
	auto classDefBase = luabind::class_<SLuaEntityHandle COMMA SLuaEntityWrapper COMMA luabind::bases<EntityHandle>>("BaseEntity");
	LUA_CUSTOM_CLASS_ENTITY_SHARED(classDefBase,SLuaEntityWrapper);
	classDefBase.def("SendData",&SLuaEntityWrapper::SendData,&SLuaBaseEntityWrapper::default_SendData);
	classDefBase.def("ReceiveNetEvent",&SLuaEntityWrapper::ReceiveNetEvent,&SLuaBaseEntityWrapper::default_ReceiveNetEvent);
	modGame[classDefBase];*/

	/*auto classDefWeapon = luabind::class_<SLuaWeaponHandle COMMA SLuaWeaponWrapper COMMA luabind::bases<SLuaEntityHandle COMMA WeaponHandle>>("BaseWeapon");
	LUA_CUSTOM_CLASS_ENTITY_SHARED(classDefWeapon,SLuaWeaponWrapper); // TODO Find a way to derive these from BaseEntity directly
	LUA_CUSTOM_CLASS_WEAPON_SHARED(classDefWeapon,SLuaWeaponWrapper);

	classDefWeapon.def("OnPickedUp",&SLuaWeaponWrapper::OnPickedUp,&SLuaWeaponWrapper::default_OnPickedUp);
	//.def("Initialize",&SLuaEntityWrapper::Initialize<SLuaWeaponWrapper>,&SLuaEntityWrapper::default_Initialize)
	//.def("Initialize",&SLuaWeaponWrapper::Initialize,&SLuaWeaponWrapper::default_Initialize)

	classDefWeapon.def("SendData",&SLuaWeaponWrapper::SendData,&SLuaBaseEntityWrapper::default_SendData);
	classDefWeapon.def("ReceiveNetEvent",&SLuaWeaponWrapper::ReceiveNetEvent,&SLuaBaseEntityWrapper::default_ReceiveNetEvent);
	modGame[classDefWeapon];*/

	// COMPONENT TODO
	/*auto classDefBaseVehicle = luabind::class_<SLuaVehicleHandle COMMA SLuaVehicleWrapper COMMA luabind::bases<SLuaEntityHandle COMMA VehicleHandle>>("BaseVehicle");
	LUA_CUSTOM_CLASS_ENTITY_SHARED(classDefBaseVehicle,SLuaVehicleWrapper);
	LUA_CUSTOM_CLASS_VEHICLE_SHARED(classDefBaseVehicle,SLuaVehicleWrapper);

	classDefBaseVehicle.def("SendData",&SLuaVehicleWrapper::SendData,&SLuaBaseEntityWrapper::default_SendData);
	classDefBaseVehicle.def("ReceiveNetEvent",&SLuaVehicleWrapper::ReceiveNetEvent,&SLuaBaseEntityWrapper::default_ReceiveNetEvent);
	modGame[classDefBaseVehicle];

	auto classDefBaseNPC = luabind::class_<SLuaNPCHandle COMMA SLuaNPCWrapper COMMA luabind::bases<SLuaEntityHandle COMMA NPCHandle>>("BaseNPC");
	LUA_CUSTOM_CLASS_ENTITY_SHARED(classDefBaseNPC,SLuaNPCWrapper);
	LUA_CUSTOM_CLASS_NPC_SHARED(classDefBaseNPC,SLuaNPCWrapper);

	classDefBaseNPC.def("SendData",&SLuaNPCWrapper::SendData,&SLuaBaseEntityWrapper::default_SendData);
	classDefBaseNPC.def("ReceiveNetEvent",&SLuaNPCWrapper::ReceiveNetEvent,&SLuaBaseEntityWrapper::default_ReceiveNetEvent);
	classDefBaseNPC.def("SelectSchedule",&SLuaNPCWrapper::SelectSchedule,&SLuaNPCWrapper::default_SelectSchedule);
	classDefBaseNPC.def("OnPrimaryTargetChanged",&SLuaNPCWrapper::OnPrimaryTargetChanged,&SLuaNPCWrapper::default_OnPrimaryTargetChanged);
	classDefBaseNPC.def("OnNPCStateChanged",&SLuaNPCWrapper::OnNPCStateChanged,&SLuaNPCWrapper::default_OnNPCStateChanged);
	classDefBaseNPC.def("OnTargetVisibilityLost",&SLuaNPCWrapper::OnTargetVisibilityLost,&SLuaNPCWrapper::default_OnTargetVisibilityLost);
	classDefBaseNPC.def("OnTargetVisibilityReacquired",&SLuaNPCWrapper::OnTargetVisibilityReacquired,&SLuaNPCWrapper::default_OnTargetVisibilityReacquired);
	classDefBaseNPC.def("OnMemoryGained",&SLuaNPCWrapper::OnMemoryGained,&SLuaNPCWrapper::default_OnMemoryGained);
	classDefBaseNPC.def("OnMemoryLost",&SLuaNPCWrapper::OnMemoryLost,&SLuaNPCWrapper::default_OnMemoryLost);
	classDefBaseNPC.def("OnTargetAcquired",&SLuaNPCWrapper::OnTargetAcquired,&SLuaNPCWrapper::default_OnTargetAcquired);
	classDefBaseNPC.def("OnScheduleComplete",&SLuaNPCWrapper::OnScheduleComplete,&SLuaNPCWrapper::default_OnScheduleComplete);
	classDefBaseNPC.def("OnSuspiciousSoundHeared",&SLuaNPCWrapper::OnSuspiciousSoundHeared,&SLuaNPCWrapper::default_OnSuspiciousSoundHeared);
	classDefBaseNPC.def("OnControllerActionInput",&SLuaNPCWrapper::OnControllerActionInput,&SLuaNPCWrapper::default_OnControllerActionInput);
	classDefBaseNPC.def("OnStartControl",&SLuaNPCWrapper::OnStartControl,&SLuaNPCWrapper::default_OnStartControl);
	classDefBaseNPC.def("OnEndControl",&SLuaNPCWrapper::OnEndControl,&SLuaNPCWrapper::default_OnEndControl);
	classDefBaseNPC.def("OnDeath",&SLuaNPCWrapper::OnDeath,&SLuaNPCWrapper::default_OnDeath);
	classDefBaseNPC.def("PlayFootStepSound",&SLuaNPCWrapper::PlayFootStepSound,&SLuaNPCWrapper::default_PlayFootStepSound);
	classDefBaseNPC.def("CalcMovementSpeed",&SLuaNPCWrapper::CalcMovementSpeed,&SLuaNPCWrapper::default_CalcMovementSpeed);
	modGame[classDefBaseNPC];
	//
	
	auto defEnvParticleSystem = luabind::class_<EnvParticleSystemHandle,EntityHandle>("EnvParticleSystem");
	modGame[defEnvParticleSystem];

	auto envLightClassDef = luabind::class_<EnvLightHandle,EntityHandle>("EnvLight");
	Lua::EnvLight::register_class(envLightClassDef);
	modGame[envLightClassDef];

	auto envLightSpotClassDef = luabind::class_<EnvLightSpotHandle COMMA EnvLightHandle,EntityHandle>("EnvLightSpot");
	Lua::EnvLightSpot::register_class(envLightSpotClassDef);
	modGame[envLightSpotClassDef];

	auto envLightPointClassDef = luabind::class_<EnvLightPointHandle COMMA EnvLightHandle,EntityHandle>("EnvLightPoint");
	Lua::EnvLightPoint::register_class(envLightPointClassDef);
	modGame[envLightPointClassDef];

	auto envLightDirectionalClassDef = luabind::class_<EnvLightDirectionalHandle COMMA EnvLightHandle,EntityHandle>("EnvLightDirectional");
	Lua::EnvLightDirectional::register_class(envLightDirectionalClassDef);
	modGame[envLightDirectionalClassDef];

	auto funcWaterClassDef = luabind::class_<FuncWaterHandle,EntityHandle>("FuncWater");
	Lua::FuncWater::register_class(funcWaterClassDef);
	modGame[funcWaterClassDef];

	_G["Entity"] = _G["game"]["Entity"];
	_G["BaseEntity"] = _G["game"]["BaseEntity"];
	_G["BaseWeapon"] = _G["game"]["BaseWeapon"];
	_G["BaseVehicle"] = _G["game"]["BaseVehicle"];
	_G["BaseNPC"] = _G["game"]["BaseNPC"];

	auto &modShader = GetLuaInterface().RegisterLibrary("shader");
	auto defShaderInfo = luabind::class_<ShaderInfo>("Info");
	defShaderInfo.def("GetName",&Lua_ShaderInfo_GetName);
	modShader[defShaderInfo];*/
	/*lua_bind(
		luabind::class_<AITask>("AITask")
		.def("AddParameter",(void(AITask::*)(bool))&AITask::AddParameter)
		.def("AddParameter",(void(AITask::*)(float))&AITask::AddParameter)
		.def("AddParameter",(void(AITask::*)(std::string))&AITask::AddParameter)
		.def("AddParameter",(void(AITask::*)(BaseEntity*))&AITask::AddParameter)
		.def("AddParameter",(void(AITask::*)(EntityHandle&))&AITask::AddParameter)
		.def("AddParameter",(void(AITask::*)(Vector3&))&AITask::AddParameter)
		.def("AddParameter",(void(AITask::*)(Quat&))&AITask::AddParameter)
		.def("AddParameter",(void(AITask::*)(EulerAngles&))&AITask::AddParameter)
	);*/ // TODO
}
#pragma optimize("",on)
