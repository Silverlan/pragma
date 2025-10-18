// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <luabind/pointer_traits.hpp>

export module pragma.shared:scripting.lua.handle_holder;

export import pragma.util;

export {
	namespace pragma::lua {
		template<typename T>
		struct HandleHolder {
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
		HandleHolder<T> *get_const_holder(HandleHolder<T> *)
		{
			return 0;
		}
		template<typename T>
		T *get_pointer(const HandleHolder<T> &pointer)
		{
			return pointer.get();
		}
	};

	template<typename T>
	pragma::lua::HandleHolder<T>::HandleHolder(T *ptr) : m_ptr {ptr}
	{
	}
	template<typename T>
	T *pragma::lua::HandleHolder<T>::get() const
	{
		if(m_hasHandle)
			return m_handle.template get<T>();
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
			struct pointer_traits<pragma::lua::HandleHolder<T>> {
				enum { is_pointer = true };
				using value_type = T;
			};
		};
	};
};
