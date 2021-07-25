/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __LUA_HANDLES_HPP__
#define __LUA_HANDLES_HPP__

namespace util
{
	template<typename T>
		class TWeakSharedHandle;
	template<typename T>
		class TSharedHandle;
	template<typename T>
		class WeakHandle;
};
namespace luabind
{
	template<typename T>
		T* get_pointer(const util::TWeakSharedHandle<T>& pointer)
	{
		return const_cast<T*>(pointer.Get());
	}
	template<typename T>
		T* get_pointer(const util::TSharedHandle<T>& pointer)
	{
		return const_cast<T*>(pointer.Get());
	}
	template<typename T>
		T* get_pointer(const util::WeakHandle<T>& pointer)
	{
		return const_cast<T*>(pointer.get());
	}
};
namespace luabind::detail::has_get_pointer_
{
	template<typename T>
		T* get_pointer(const util::TWeakSharedHandle<T>& pointer)
	{
		return const_cast<T*>(pointer.Get());
	}
	template<typename T>
		T* get_pointer(const util::TSharedHandle<T>& pointer)
	{
		return const_cast<T*>(pointer.Get());
	}
	template<typename T>
		T* get_pointer(const util::WeakHandle<T>& pointer)
	{
		return const_cast<T*>(pointer.get());
	}
}

#endif



/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */
#if 0
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
namespace luabind
{
	template<typename T>
		T* get_pointer(const util::TWeakSharedHandle<T>& pointer)
	{
		return const_cast<T*>(pointer.Get());
	}
	template<typename T>
		T* get_pointer(const util::TSharedHandle<T>& pointer)
	{
		return const_cast<T*>(pointer.Get());
	}
	template<typename T>
		T* get_pointer(const util::WeakHandle<T>& pointer)
	{
		return const_cast<T*>(pointer.get());
	}
	namespace detail
	{
		template<typename T>
		struct pointer_traits<util::TWeakSharedHandle<T>>
		{
			enum { is_pointer = true };
			using value_type = T;
		};
		template<typename T>
		struct pointer_traits<util::TSharedHandle<T>>
		{
			enum { is_pointer = true };
			using value_type = T;
		};
		template<typename T>
		struct pointer_traits<util::WeakHandle<T>>
		{
			enum { is_pointer = true };
			using value_type = T;
		};
	};
};
namespace luabind::detail::has_get_pointer_
{
	template<typename T>
		T* get_pointer(const util::TWeakSharedHandle<T> &pointer)
	{
		return const_cast<T*>(pointer.Get());
	}
	template<typename T>
		T* get_pointer(const util::TSharedHandle<T> &pointer)
	{
		return const_cast<T*>(pointer.Get());
	}
	template<typename T>
		T* get_pointer(const util::WeakHandle<T> &pointer)
	{
		return const_cast<T*>(pointer.get());
	}
}

namespace luabind {

	namespace detail
	{

		struct weak_shared_handle_deleter
		{
			weak_shared_handle_deleter(lua_State* L, int index)
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
	struct default_converter<util::TWeakSharedHandle<T> >
		: default_converter<T*>
	{
		using is_native = std::false_type;

		template <class U>
		int match(lua_State* L, U, int index)
		{
			return default_converter<T*>::match(L, decorate_type_t<T*>(), index);
		}

		template <class U>
		util::TWeakSharedHandle<T> to_cpp(lua_State* L, U, int index)
		{
			T* raw_ptr = default_converter<T*>::to_cpp(L, decorate_type_t<T*>(), index);

			if(!raw_ptr) {
				return util::TWeakSharedHandle<T>();
			} else {
				return util::TWeakSharedHandle<T>(raw_ptr, detail::weak_shared_handle_deleter(L, index));
			}
		}

		void to_lua(lua_State* L, util::TWeakSharedHandle<T> const& p)
		{
			if(detail::weak_shared_handle_deleter* d = std::get_deleter<detail::weak_shared_handle_deleter>(p))
			{
				d->life_support.push(L);
			} else {
				detail::value_converter().to_lua(L, p);
			}
		}

		template <class U>
		void converter_postcall(lua_State*, U const&, int)
		{}
	};

	template <class T>
	struct default_converter<util::TWeakSharedHandle<T> const&>
		: default_converter<util::TWeakSharedHandle<T> >
	{};

} // namespace luabind

namespace luabind {

	namespace detail
	{

		struct shared_handle_deleter
		{
			shared_handle_deleter(lua_State* L, int index)
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
	struct default_converter<util::TSharedHandle<T> >
		: default_converter<T*>
	{
		using is_native = std::false_type;

		template <class U>
		int match(lua_State* L, U, int index)
		{
			return default_converter<T*>::match(L, decorate_type_t<T*>(), index);
		}

		template <class U>
		util::TSharedHandle<T> to_cpp(lua_State* L, U, int index)
		{
			T* raw_ptr = default_converter<T*>::to_cpp(L, decorate_type_t<T*>(), index);

			if(!raw_ptr) {
				return util::TSharedHandle<T>();
			} else {
				return util::TSharedHandle<T>(raw_ptr, detail::shared_handle_deleter(L, index));
			}
		}

		void to_lua(lua_State* L, util::TSharedHandle<T> const& p)
		{
			if(detail::shared_handle_deleter* d = std::get_deleter<detail::shared_handle_deleter>(p))
			{
				d->life_support.push(L);
			} else {
				detail::value_converter().to_lua(L, p);
			}
		}

		template <class U>
		void converter_postcall(lua_State*, U const&, int)
		{}
	};

	template <class T>
	struct default_converter<util::TSharedHandle<T> const&>
		: default_converter<util::TSharedHandle<T> >
	{};

} // namespace luabind

namespace luabind {

	namespace detail
	{

		struct weak_handle_deleter
		{
			weak_handle_deleter(lua_State* L, int index)
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
	struct default_converter<util::WeakHandle<T> >
		: default_converter<T*>
	{
		using is_native = std::false_type;

		template <class U>
		int match(lua_State* L, U, int index)
		{
			return default_converter<T*>::match(L, decorate_type_t<T*>(), index);
		}

		template <class U>
		util::WeakHandle<T> to_cpp(lua_State* L, U, int index)
		{
			T* raw_ptr = default_converter<T*>::to_cpp(L, decorate_type_t<T*>(), index);

			if(!raw_ptr) {
				return util::WeakHandle<T>();
			} else {
				return util::WeakHandle<T>(raw_ptr, detail::weak_handle_deleter(L, index));
			}
		}

		void to_lua(lua_State* L, util::WeakHandle<T> const& p)
		{
			if(detail::weak_handle_deleter* d = std::get_deleter<detail::weak_handle_deleter>(p))
			{
				d->life_support.push(L);
			} else {
				detail::value_converter().to_lua(L, p);
			}
		}

		template <class U>
		void converter_postcall(lua_State*, U const&, int)
		{}
	};

	template <class T>
	struct default_converter<util::WeakHandle<T> const&>
		: default_converter<util::WeakHandle<T> >
	{};

} // namespace luabind
#endif
#endif
