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
#include <luabind/detail/conversion_policies/value_converter.hpp> // for default_converter, etc
#include <luabind/get_main_thread.hpp>                            // for get_main_thread
#include <luabind/handle.hpp>                                     // for handle
#include <luabind/detail/decorate_type.hpp>                       // for decorated_type
#include <sharedutils/util_shared_handle.hpp>
#include <memory>

namespace util {
	template<typename T>
	class TWeakSharedHandle;
	template<typename T>
	class TSharedHandle;
	template<typename T>
	class WeakHandle;
};

// Note: get_const_holder and get_pointer have to be defined
// in the same namespace as the type!! (luabind namespace will *not* work
// unless they're defined before *any* luabind includes)
#define LUA_DEFINE_PTR_TYPE(NAMESPACE, TPTR, BASENAME)                                                                                                                                                                                                                                           \
	namespace NAMESPACE {                                                                                                                                                                                                                                                                        \
		template<class T>                                                                                                                                                                                                                                                                        \
		TPTR<T const> *get_const_holder(TPTR<T> *)                                                                                                                                                                                                                                               \
		{                                                                                                                                                                                                                                                                                        \
			return 0;                                                                                                                                                                                                                                                                            \
		}                                                                                                                                                                                                                                                                                        \
		template<typename T>                                                                                                                                                                                                                                                                     \
		T *get_pointer(const TPTR<T> &pointer)                                                                                                                                                                                                                                                   \
		{                                                                                                                                                                                                                                                                                        \
			return const_cast<T *>(pointer.get());                                                                                                                                                                                                                                               \
		}                                                                                                                                                                                                                                                                                        \
	};

LUA_DEFINE_PTR_TYPE(util, util::TWeakSharedHandle, weak_shared_handle);
LUA_DEFINE_PTR_TYPE(util, util::TSharedHandle, shared_handle);
LUA_DEFINE_PTR_TYPE(util, util::WeakHandle, weak_handle);

// Implementation similar to shared_ptr_converter.hpp
#define LUA_DEFINE_PTR_TYPE_CONVERTER(TClass, CastFunc, enableToCpp)                                                                                                                                                                                                                             \
	namespace luabind {                                                                                                                                                                                                                                                                          \
		namespace detail {                                                                                                                                                                                                                                                                       \
			template<typename T>                                                                                                                                                                                                                                                                 \
			struct pointer_traits<TClass<T>> {                                                                                                                                                                                                                                                   \
				enum { is_pointer = true };                                                                                                                                                                                                                                                      \
				using value_type = T;                                                                                                                                                                                                                                                            \
			};                                                                                                                                                                                                                                                                                   \
		};                                                                                                                                                                                                                                                                                       \
		template<class T>                                                                                                                                                                                                                                                                        \
		struct default_converter<TClass<T>> : default_converter<T *> {                                                                                                                                                                                                                           \
			using is_native = std::false_type;                                                                                                                                                                                                                                                   \
			template<class U>                                                                                                                                                                                                                                                                    \
			int match(lua_State *L, U, int index)                                                                                                                                                                                                                                                \
			{                                                                                                                                                                                                                                                                                    \
				return default_converter<T *>::match(L, decorate_type_t<T *>(), index);                                                                                                                                                                                                          \
			}                                                                                                                                                                                                                                                                                    \
			template<class U>                                                                                                                                                                                                                                                                    \
			TClass<T> to_cpp(lua_State *L, U, int index)                                                                                                                                                                                                                                         \
			{                                                                                                                                                                                                                                                                                    \
				if constexpr(!enableToCpp)                                                                                                                                                                                                                                                       \
					return TClass<T>();                                                                                                                                                                                                                                                          \
				else {                                                                                                                                                                                                                                                                           \
					T *raw_ptr = default_converter<T *>::to_cpp(L, decorate_type_t<T *>(), index);                                                                                                                                                                                               \
					if(!raw_ptr) {                                                                                                                                                                                                                                                               \
						return TClass<T>();                                                                                                                                                                                                                                                      \
					}                                                                                                                                                                                                                                                                            \
					else {                                                                                                                                                                                                                                                                       \
						auto h = raw_ptr->GetHandle();                                                                                                                                                                                                                                           \
						return util::CastFunc<typename decltype(h)::value_type, T>(h);                                                                                                                                                                                                           \
					}                                                                                                                                                                                                                                                                            \
				}                                                                                                                                                                                                                                                                                \
			}                                                                                                                                                                                                                                                                                    \
			void to_lua(lua_State *L, TClass<T> const &p)                                                                                                                                                                                                                                        \
			{                                                                                                                                                                                                                                                                                    \
				default_converter<T *>().to_lua(L, const_cast<TClass<T> &>(p).get());                                                                                                                                                                                                            \
			}                                                                                                                                                                                                                                                                                    \
			template<class U>                                                                                                                                                                                                                                                                    \
			void converter_postcall(lua_State *, U const &, int)                                                                                                                                                                                                                                 \
			{                                                                                                                                                                                                                                                                                    \
			}                                                                                                                                                                                                                                                                                    \
		};                                                                                                                                                                                                                                                                                       \
		template<class T>                                                                                                                                                                                                                                                                        \
		struct default_converter<TClass<T> const &> : default_converter<TClass<T>> {};                                                                                                                                                                                                           \
	}

LUA_DEFINE_PTR_TYPE_CONVERTER(util::TWeakSharedHandle, weak_shared_handle_cast, true);
LUA_DEFINE_PTR_TYPE_CONVERTER(util::TSharedHandle, shared_handle_cast, false);

#endif
