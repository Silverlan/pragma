/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __LUA_HANDLES_HPP__
#define __LUA_HANDLES_HPP__

#include <luabind/pointer_traits.hpp>
#include <luabind/detail/policy.hpp>
#include <luabind/detail/conversion_policies/value_converter.hpp>    // for default_converter, etc
#include <luabind/get_main_thread.hpp>  // for get_main_thread
#include <luabind/handle.hpp>           // for handle
#include <luabind/detail/decorate_type.hpp>  // for decorated_type
#include <memory>

namespace util
{
	template<typename T>
		class TWeakSharedHandle;
	template<typename T>
		class TSharedHandle;
	template<typename T>
		class WeakHandle;
};

#define LUA_DEFINE_PTR_TYPE(TPTR,BASENAME) \
	namespace luabind \
	{ \
		template<typename T> \
		T* get_pointer(const TPTR& pointer) \
		{ \
			return const_cast<T*>(pointer.Get()); \
		} \
	}; \
	namespace luabind::detail::has_get_pointer_ \
	{ \
		template<typename T> \
			T* get_pointer(const TPTR &pointer) \
		{ \
			return const_cast<T*>(pointer.Get()); \
		} \
	};

LUA_DEFINE_PTR_TYPE(util::TWeakSharedHandle<T>,weak_shared_handle);
LUA_DEFINE_PTR_TYPE(util::TSharedHandle<T>,shared_handle);
LUA_DEFINE_PTR_TYPE(util::WeakHandle<T>,weak_handle);

/*
	namespace luabind { \
		namespace detail \
		{ \
			struct BASENAME##_deleter \
			{ \
				BASENAME##_deleter(lua_State* L, int index) \
					: life_support(get_main_thread(L), L, index) \
				{} \
				void operator()(void const*) \
				{ \
					handle().swap(life_support); \
				} \
				handle life_support; \
			}; \
		} \
		template <class T> \
		struct default_converter<TPTR > \
			: default_converter<T*> \
		{ \
			using is_native = std::false_type; \
			template <class U> \
			int match(lua_State* L, U, int index) \
			{ \
				return default_converter<T*>::match(L, decorate_type_t<T*>(), index); \
			} \
			template <class U> \
			TPTR to_cpp(lua_State* L, U, int index) \
			{ \
				T* raw_ptr = default_converter<T*>::to_cpp(L, decorate_type_t<T*>(), index); \
				if(!raw_ptr) { \
					return TPTR(); \
				} else { \
					return TPTR(raw_ptr, detail::BASENAME##_deleter(L, index)); \
				} \
			} \
			void to_lua(lua_State* L, TPTR const& p) \
			{ \
				if(detail::BASENAME##_deleter* d = std::get_deleter<detail::BASENAME##_deleter>(p)) \
				{ \
					d->life_support.push(L); \
				} else { \
					detail::value_converter().to_lua(L, p); \
				} \
			} \
			template <class U> \
			void converter_postcall(lua_State*, U const&, int) \
			{} \
		}; \
		template <class T> \
		struct default_converter<TPTR const&> \
			: default_converter<TPTR > \
		{}; \
	}
*/

#endif
