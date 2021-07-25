/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __HANDLE_POLICY_HPP__
#define __HANDLE_POLICY_HPP__

#include <luabind/detail/policy.hpp>
#if 0
namespace luabind {
	namespace detail {
		struct handle_converter
		{
			template <class T>
			void to_lua(lua_State* L, T const& x)
			{
				luabind::object{L,x.GetHandle()}.push(L);
			}

			template <class T>
			void to_lua(lua_State* L, T* x)
			{
				if(!x)
					lua_pushnil(L);
				else
					luabind::object{L,x->GetHandle()}.push(L);
			}
		};

		struct handle_policy
		{
			template <class T, class Direction>
			struct specialize
			{
				static_assert(std::is_same<Direction, cpp_to_lua>::value, "Handle policy only supports cpp -> lua");
				using type = handle_converter;
			};
		};
	} // namespace detail

	template< unsigned int N >
	using handle_policy = meta::type_list< converter_policy_injector< N, detail::handle_policy > >;

} // namespace luabind
#endif
#include <sharedutils/util_weak_handle.hpp>
namespace luabind {
	namespace detail {
		struct weak_handle2_deleter
		{
			weak_handle2_deleter(lua_State* L, int index)
				: life_support(get_main_thread(L), L, index)
			{}

			void operator()(void const*)
			{
				handle().swap(life_support);
			}

			handle life_support;
		};

		struct weak_handle_converter
		{
#if 0
			template <class T>
			T to_cpp(lua_State* L, by_reference<T>, int index)
			{
				/*T* raw_ptr = default_converter<T*>::to_cpp(L, decorate_type_t<T*>(), index);
				if(!raw_ptr) {
					return util::WeakHandle<T>();
				} else {
					return raw_ptr->GetHandle();
				}*/
				return T{};
			}
			template <class T>
			T to_cpp(lua_State* L, by_const_reference<T>, int index)
			{
				/*T* raw_ptr = default_converter<T*>::to_cpp(L, decorate_type_t<T*>(), index);
				if(!raw_ptr) {
					return util::WeakHandle<T>();
				} else {
					return raw_ptr->GetHandle();
				}*/
				return T{};
			}
			template <class T>
			T to_cpp(lua_State* L, by_value<T>, int index)
			{
				/*T* raw_ptr = default_converter<T*>::to_cpp(L, decorate_type_t<T*>(), index);
				if(!raw_ptr) {
					return util::WeakHandle<T>();
				} else {
					return raw_ptr->GetHandle();
				}*/
				return T{};
			}
			template <class T>
			T to_cpp(lua_State* L, by_pointer<T>, int index)
			{
				/*T* raw_ptr = default_converter<T*>::to_cpp(L, decorate_type_t<T*>(), index);
				if(!raw_ptr) {
					return util::WeakHandle<T>();
				} else {
					return raw_ptr->GetHandle();
				}*/
				return T{};
			}

			template <class T>
			void to_lua(lua_State* L, util::WeakHandle<T> const& x)
			{
				if(x.expired())
					lua_pushnil(L);
				else
					;//default_converter<T*>::to_lua(L,x.get());
			}

			template <class T>
			void to_lua(lua_State* L, util::WeakHandle<T>* x)
			{
				if(!x)
					lua_pushnil(L);
				else
					;//default_converter<T*>::to_lua(L,x->get());
			}
#endif
			using is_native = std::false_type;

			template <class T>
			int match(lua_State* L, T, int index)
			{
				return default_converter<T*>::match(L, decorate_type_t<T*>(), index);
			}

			template <class T>
			util::WeakHandle<T> to_cpp(lua_State* L, T, int index)
			{
				T* raw_ptr = default_converter<T*>::to_cpp(L, decorate_type_t<T*>(), index);

				if(!raw_ptr) {
					return util::WeakHandle<T>();
				} else {
					return raw_ptr->GetHandle();
				}
			}
			
			template <class T>
			void to_lua(lua_State* L, util::WeakHandle<T> const& p)
			{
				/*if(detail::weak_handle2_deleter* d = std::get_deleter<detail::weak_handle2_deleter>(p))
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

		struct weak_handle_policy
		{
			template <class T, class Direction>
			struct specialize
			{
				static_assert(std::is_same<Direction, cpp_to_lua>::value || std::is_same<Direction, lua_to_cpp>::value, "Invalid direction");
				using type = weak_handle_converter;
			};
		};
	} // namespace detail

	template< unsigned int N >
	using weak_handle_policy = meta::type_list< converter_policy_injector< N, detail::weak_handle_policy > >;

} // namespace luabind




/*namespace luabind {

	namespace detail
	{

		struct weak_handle2_deleter
		{
			weak_handle2_deleter(lua_State* L, int index)
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

	};

	template <class T>
	struct default_converter<util::WeakHandle<T> const&>
		: default_converter<util::WeakHandle<T> >
	{};

} // namespace luabind
*/
#if 0
namespace luabind {

	namespace detail
	{
		template<typename T>
		struct pointer_traits<util::WeakHandle<T>>
		{
			enum { is_pointer = true };
			using value_type = T;
		};


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
				return util::WeakHandle<T>();//raw_ptr, detail::weak_handle_deleter(L, index));
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
