/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __LUA_HANDLE_HOLDER_HPP__
#define __LUA_HANDLE_HOLDER_HPP__

#include <sharedutils/util_shared_handle.hpp>
#include <luabind/pointer_traits.hpp>

namespace pragma::lua
{
	template<typename T>
		struct HandleHolder
	{
	public:
		HandleHolder(T *ptr);
		T *get() const;
		void SetHandle(const util::TWeakSharedHandle<T> &handle);
	private:
		mutable T *m_ptr = nullptr;
		mutable util::TWeakSharedHandle<T> m_handle {};
		bool m_hasHandle = false;
	};

	template<typename T>
		HandleHolder<T>* get_const_holder(HandleHolder<T>*) {return 0;}
	template<typename T>
		T* get_pointer(const HandleHolder<T>& pointer) {return pointer.get();}
};

template<typename T>
	pragma::lua::HandleHolder<T>::HandleHolder(T *ptr)
		: m_ptr{ptr}
{}
template<typename T>
	T *pragma::lua::HandleHolder<T>::get() const
{
	if(m_hasHandle)
		return m_handle.get<T>();
	return m_ptr;
}
template<typename T>
	void pragma::lua::HandleHolder<T>::SetHandle(const util::TWeakSharedHandle<T> &handle)
{
	m_hasHandle = true;
	m_handle = handle;
}

namespace luabind {
	namespace detail {
		template<typename T>
		struct pointer_traits<pragma::lua::HandleHolder<T>>
		{
			enum { is_pointer = true };
			using value_type = T;
		};
	};
};

#endif
