/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */
#if 0
#include <luabind/class.hpp>
#include <sharedutils/util_weak_handle.hpp>
#include "pragma/entities/baseentity.h"

/*template<typename T>
	class TClass
	{};*/
template<typename T>
	using TClass = util::WeakHandle<T>;

#define LUA_DEFINE_PTR_TYPE(TPTR, BASENAME)                                                                                                                                                                                                                                                      \
	namespace luabind {                                                                                                                                                                                                                                                                          \
		template<typename T>                                                                                                                                                                                                                                                                     \
		T *get_pointer(const TPTR &pointer)                                                                                                                                                                                                                                                      \
		{                                                                                                                                                                                                                                                                                        \
			return const_cast<T *>(pointer.Get());                                                                                                                                                                                                                                               \
		}                                                                                                                                                                                                                                                                                        \
	};                                                                                                                                                                                                                                                                                           \
	namespace luabind::detail::has_get_pointer_ {                                                                                                                                                                                                                                                \
		template<typename T>                                                                                                                                                                                                                                                                     \
		T *get_pointer(const TPTR &pointer)                                                                                                                                                                                                                                                      \
		{                                                                                                                                                                                                                                                                                        \
			return const_cast<T *>(pointer.Get());                                                                                                                                                                                                                                               \
		}                                                                                                                                                                                                                                                                                        \
	};

//LUA_DEFINE_PTR_TYPE(util::WeakHandle<T>,weak_handle);
#include "pragma/lua/lua_handles.hpp"

namespace luabind {

	namespace detail
	{

		struct tclass_deleter
		{
			tclass_deleter(lua_State* L, int index)
				: life_support(get_main_thread(L), L, index)
			{}

			void operator()(void const*)
			{
				handle().swap(life_support);
			}

			handle life_support;
		};

	} // namespace detail

	template <class T>
	struct default_converter<TClass<T> >
		: default_converter<T*>
	{
		using is_native = std::false_type;

		template <class U>
		int match(lua_State* L, U, int index)
		{
			return default_converter<T*>::match(L, decorate_type_t<T*>(), index);
		}

		template <class U>
		TClass<T> to_cpp(lua_State* L, U, int index)
		{
			T* raw_ptr = default_converter<T*>::to_cpp(L, decorate_type_t<T*>(), index);

			if(!raw_ptr) {
				return TClass<T>();
			} else {
				return TClass<T>();//raw_ptr, detail::tclass_deleter(L, index));
			}
		}

		void to_lua(lua_State* L, TClass<T> const& p)
		{
			/*if(detail::tclass_deleter* d = std::get_deleter<detail::tclass_deleter>(p))
			{
				d->life_support.push(L);
			} else {
				detail::value_converter().to_lua(L, p);
			}*/
		}

		template <class U>
		void converter_postcall(lua_State*, U const&, int)
		{}
	};

	template <class T>
	struct default_converter<TClass<T> const&>
		: default_converter<TClass<T> >
	{};

} // namespace luabind


struct BE {};
static void test()
{
	struct Test
	{
		//util::WeakHandle<BaseEntity> m0;
		TClass<BaseEntity> m0;
	};
	auto tdeef = luabind::class_<Test>("Test");
	//tdeef.def_readwrite("attacker",&Test::m0,luabind::weak_handle_policy<0>{},luabind::weak_handle_policy<0>{});
	tdeef.def_readwrite("attacker",&Test::m0);
}
#endif

#include "pragma/lua/ldefinitions.h"
#include "pragma/entities/baseentity_handle.h"
#include "pragma/entities/baseentity.h"
#include "pragma/entities/components/base_entity_component.hpp"
#include "pragma/util/bulletinfo.h"
#include <sharedutils/util_shared_handle.hpp>
namespace Lua {
	// These don't like to be in core/shared/src/lua/class.cpp for some reason (Something to do with EntityHandle hAttacker/hInflictor)
	void register_bullet_info(luabind::module_ &gameMod)
	{
		auto bulletInfo = luabind::class_<BulletInfo>("BulletInfo");
		bulletInfo.def(luabind::constructor<>());
		bulletInfo.def(luabind::tostring(luabind::self));
		bulletInfo.def_readwrite("spread", &BulletInfo::spread);
		bulletInfo.def_readwrite("force", &BulletInfo::force);
		bulletInfo.def_readwrite("distance", &BulletInfo::distance);
		bulletInfo.def_readwrite("damageType", reinterpret_cast<std::underlying_type_t<decltype(BulletInfo::damageType)> BulletInfo::*>(&BulletInfo::damageType));
		bulletInfo.def_readwrite("bulletCount", &BulletInfo::bulletCount);
		bulletInfo.def_readwrite("attacker", &BulletInfo::hAttacker);
		bulletInfo.def_readwrite("inflictor", &BulletInfo::hInflictor);
		bulletInfo.def_readwrite("tracerCount", &BulletInfo::tracerCount);
		bulletInfo.def_readwrite("tracerRadius", &BulletInfo::tracerRadius);
		bulletInfo.def_readwrite("tracerColor", &BulletInfo::tracerColor);
		bulletInfo.def_readwrite("tracerLength", &BulletInfo::tracerLength);
		bulletInfo.def_readwrite("tracerSpeed", &BulletInfo::tracerSpeed);
		bulletInfo.def_readwrite("tracerMaterial", &BulletInfo::tracerMaterial);
		bulletInfo.def_readwrite("tracerBloom", &BulletInfo::tracerBloom);
		bulletInfo.def_readwrite("ammoType", &BulletInfo::ammoType);
		bulletInfo.def_readwrite("direction", &BulletInfo::direction);
		bulletInfo.def_readwrite("effectOrigin", &BulletInfo::effectOrigin);
		bulletInfo.def_readwrite("damage", &BulletInfo::damage);
		gameMod[bulletInfo];
	}
};
