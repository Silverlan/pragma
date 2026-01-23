// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:scripting.lua.converters.gui;

import pragma.gui;
export import pragma.lua;
export import :gui.lua_interface;

export namespace luabind {
	template<typename T, typename TConverter>
	struct gui_element_converter {
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

	template<typename T>
	concept is_gui_element = std::derived_from<pragma::util::base_type<T>, pragma::gui::types::WIBase> || std::is_same_v<pragma::util::base_type<T>, pragma::gui::WIHandle>;

	template<typename T>
	    requires(is_gui_element<T> && std::is_pointer_v<T> && !std::is_const_v<std::remove_pointer_t<T>>)
	struct default_converter<T> : gui_element_converter<T, detail::pointer_converter> {};
	template<typename T>
	    requires(is_gui_element<T> && std::is_pointer_v<T> && std::is_const_v<std::remove_pointer_t<T>>)
	struct default_converter<T> : gui_element_converter<T, detail::const_pointer_converter> {};

	template<typename T>
	    requires(is_gui_element<T> && std::is_reference_v<T> && !std::is_const_v<std::remove_reference_t<T>>)
	struct default_converter<T> : gui_element_converter<T, detail::ref_converter> {};
	template<typename T>
	    requires(is_gui_element<T> && std::is_reference_v<T> && std::is_const_v<std::remove_reference_t<T>>)
	struct default_converter<T> : gui_element_converter<T, detail::const_ref_converter> {};

	// Implementation
	template<typename T, typename TConverter>
	template<class U>
	T gui_element_converter<T, TConverter>::to_cpp(lua::State *L, U u, int index)
	{
		return m_converter.to_cpp(L, u, index);
	}

	template<typename T, typename TConverter>
	void gui_element_converter<T, TConverter>::to_lua(lua::State *L, T x)
	{
		if constexpr(std::is_same_v<T, pragma::gui::WIHandle>) {
			if(x.expired())
				lua::push_nil(L);
			else {
				auto &el = *x;
				auto o = pragma::gui::WGUILuaInterface::GetLuaObject(L, const_cast<std::remove_cvref_t<decltype(el)> &>(el));
				o.push(L);
			}
		}
		else if constexpr(std::is_pointer_v<T>) {
			if(!x)
				lua::push_nil(L);
			else {
				auto o = pragma::gui::WGUILuaInterface::GetLuaObject(L, *const_cast<T>(x));
				o.push(L);
			}
		}
		else {
			auto o = pragma::gui::WGUILuaInterface::GetLuaObject(L, const_cast<T &>(x));
			o.push(L);
		}
	}

	template<typename T, typename TConverter>
	template<class U>
	int gui_element_converter<T, TConverter>::match(lua::State *L, U u, int index)
	{
		return m_converter.match(L, u, index);
	}
};
