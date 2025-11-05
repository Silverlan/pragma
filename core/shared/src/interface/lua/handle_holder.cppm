// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

export module pragma.shared:scripting.lua.handle_holder;

export import luabind;
export import pragma.util;

export {
	namespace pragma::LuaCore {
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

		template<typename T>
		HandleHolder<T>::HandleHolder(T *ptr) : m_ptr {ptr}
		{
		}
		template<typename T>
		T *HandleHolder<T>::get() const
		{
			if(m_hasHandle)
				return m_handle.template get<T>();
			return m_ptr;
		}
		template<typename T>
		void HandleHolder<T>::SetHandle(const util::TWeakSharedHandle<T> &handle)
		{
			m_hasHandle = true;
			m_handle = handle;
		}
	}

	namespace luabind {
		namespace detail {
			template<typename T>
			struct pointer_traits<pragma::LuaCore::HandleHolder<T>> {
				enum { is_pointer = true };
				using value_type = T;
			};
		};
	};
};
