// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.converters.property;

export import :scripting.lua.classes.property;
export import pragma.util;

export namespace luabind {
	// Note: Method definitions for these should be moved to "property_converter_t.hpp", but due to a msvc compile error (21-08-08)
	// this is currently not possible

	template<typename T>
	void push_property(lua::State *l, T &prop);

	template<typename T>
	    requires(std::derived_from<pragma::util::base_type<T>, pragma::util::BaseProperty>)
	struct default_converter<T> : detail::default_converter_generator<T>::type {
		enum { consumed_args = 1 };

		template<typename U>
		T to_cpp(lua::State *L, U u, int index)
		{
			return detail::default_converter_generator<T>::type::to_cpp(L, u, index);
		}

		template<class U>
		int match(lua::State *l, U u, int index)
		{
			return detail::default_converter_generator<T>::type::match(l, u, index);
		}

		template<class U>
		void converter_postcall(lua::State *, U u, int)
		{
		}

		void to_lua(lua::State *L, T const &x)
		{
			if constexpr(std::is_pointer_v<T>) {
				if(!x)
					Lua::PushNil(L);
				else
					push_property(L, *const_cast<T &>(x));
			}
			else
				push_property(L, const_cast<T &>(x));
		}
		void to_lua(lua::State *L, std::remove_reference_t<T> *x)
		{
			if(!x)
				Lua::PushNil(L);
			else
				to_lua(L, *x);
		}
	};

	template<typename TProp>
	    requires(std::derived_from<pragma::util::base_type<TProp>, pragma::util::BaseProperty>)
	struct property_converter : default_converter<TProp *> {
		using is_native = std::false_type;

		template<class U>
		int match(lua::State *L, U, int index)
		{
			return default_converter<TProp *>::match(L, decorate_type_t<TProp *>(), index);
		}

		template<class U>
		std::shared_ptr<TProp> to_cpp(lua::State *L, U, int index)
		{
			TProp *raw_ptr = default_converter<TProp *>::to_cpp(L, decorate_type_t<TProp *>(), index);
			return std::static_pointer_cast<TProp>(raw_ptr->shared_from_this());
		}

		void to_lua(lua::State *L, std::shared_ptr<TProp> const &p) { default_converter<TProp>().to_lua(L, p.get()); }

		template<class U>
		void converter_postcall(lua::State *, U const &, int)
		{
		}
	};

	//
	template<>
	struct DLLNETWORK default_converter<std::shared_ptr<pragma::util::ColorProperty>> : property_converter<pragma::util::ColorProperty> {};

	template<>
	struct DLLNETWORK default_converter<std::shared_ptr<pragma::util::ColorProperty> const &> : property_converter<pragma::util::ColorProperty> {};
	//

	//
	template<>
	struct DLLNETWORK default_converter<std::shared_ptr<pragma::util::BoolProperty>> : property_converter<pragma::util::BoolProperty> {};

	template<>
	struct DLLNETWORK default_converter<std::shared_ptr<pragma::util::BoolProperty> const &> : property_converter<pragma::util::BoolProperty> {};
	//

	//
	template<>
	struct DLLNETWORK default_converter<std::shared_ptr<pragma::util::Int8Property>> : property_converter<pragma::util::Int8Property> {};

	template<>
	struct DLLNETWORK default_converter<std::shared_ptr<pragma::util::Int8Property> const &> : property_converter<pragma::util::Int8Property> {};
	//

	//
	template<>
	struct DLLNETWORK default_converter<std::shared_ptr<pragma::util::UInt8Property>> : property_converter<pragma::util::UInt8Property> {};

	template<>
	struct DLLNETWORK default_converter<std::shared_ptr<pragma::util::UInt8Property> const &> : property_converter<pragma::util::UInt8Property> {};
	//

	//
	template<>
	struct DLLNETWORK default_converter<std::shared_ptr<pragma::util::Int16Property>> : property_converter<pragma::util::Int16Property> {};

	template<>
	struct DLLNETWORK default_converter<std::shared_ptr<pragma::util::Int16Property> const &> : property_converter<pragma::util::Int16Property> {};
	//

	//
	template<>
	struct DLLNETWORK default_converter<std::shared_ptr<pragma::util::UInt16Property>> : property_converter<pragma::util::UInt16Property> {};

	template<>
	struct DLLNETWORK default_converter<std::shared_ptr<pragma::util::UInt16Property> const &> : property_converter<pragma::util::UInt16Property> {};
	//

	//
	template<>
	struct DLLNETWORK default_converter<std::shared_ptr<pragma::util::Int32Property>> : property_converter<pragma::util::Int32Property> {};

	template<>
	struct DLLNETWORK default_converter<std::shared_ptr<pragma::util::Int32Property> const &> : property_converter<pragma::util::Int32Property> {};
	//

	//
	template<>
	struct DLLNETWORK default_converter<std::shared_ptr<pragma::util::UInt32Property>> : property_converter<pragma::util::UInt32Property> {};

	template<>
	struct DLLNETWORK default_converter<std::shared_ptr<pragma::util::UInt32Property> const &> : property_converter<pragma::util::UInt32Property> {};
	//

	//
	template<>
	struct DLLNETWORK default_converter<std::shared_ptr<pragma::util::Int64Property>> : property_converter<pragma::util::Int64Property> {};

	template<>
	struct DLLNETWORK default_converter<std::shared_ptr<pragma::util::Int64Property> const &> : property_converter<pragma::util::Int64Property> {};
	//

	//
	template<>
	struct DLLNETWORK default_converter<std::shared_ptr<pragma::util::UInt64Property>> : property_converter<pragma::util::UInt64Property> {};

	template<>
	struct DLLNETWORK default_converter<std::shared_ptr<pragma::util::UInt64Property> const &> : property_converter<pragma::util::UInt64Property> {};
	//

	//
	template<>
	struct DLLNETWORK default_converter<std::shared_ptr<pragma::util::FloatProperty>> : property_converter<pragma::util::FloatProperty> {};

	template<>
	struct DLLNETWORK default_converter<std::shared_ptr<pragma::util::FloatProperty> const &> : property_converter<pragma::util::FloatProperty> {};
	//

	//
	template<>
	struct DLLNETWORK default_converter<std::shared_ptr<pragma::util::DoubleProperty>> : property_converter<pragma::util::DoubleProperty> {};

	template<>
	struct DLLNETWORK default_converter<std::shared_ptr<pragma::util::DoubleProperty> const &> : property_converter<pragma::util::DoubleProperty> {};
	//

	//
	template<>
	struct DLLNETWORK default_converter<std::shared_ptr<pragma::util::LongDoubleProperty>> : property_converter<pragma::util::LongDoubleProperty> {};

	template<>
	struct DLLNETWORK default_converter<std::shared_ptr<pragma::util::LongDoubleProperty> const &> : property_converter<pragma::util::LongDoubleProperty> {};
	//

	//
	template<>
	struct DLLNETWORK default_converter<std::shared_ptr<pragma::util::EulerAnglesProperty>> : property_converter<pragma::util::EulerAnglesProperty> {};

	template<>
	struct DLLNETWORK default_converter<std::shared_ptr<pragma::util::EulerAnglesProperty> const &> : property_converter<pragma::util::EulerAnglesProperty> {};
	//

	//
	template<>
	struct DLLNETWORK default_converter<std::shared_ptr<pragma::util::QuatProperty>> : property_converter<pragma::util::QuatProperty> {};

	template<>
	struct DLLNETWORK default_converter<std::shared_ptr<pragma::util::QuatProperty> const &> : property_converter<pragma::util::QuatProperty> {};
	//

	//
	template<>
	struct DLLNETWORK default_converter<std::shared_ptr<pragma::util::Matrix2Property>> : property_converter<pragma::util::Matrix2Property> {};

	template<>
	struct DLLNETWORK default_converter<std::shared_ptr<pragma::util::Matrix2Property> const &> : property_converter<pragma::util::Matrix2Property> {};
	//

	//
	template<>
	struct DLLNETWORK default_converter<std::shared_ptr<pragma::util::Matrix2x3Property>> : property_converter<pragma::util::Matrix2x3Property> {};

	template<>
	struct DLLNETWORK default_converter<std::shared_ptr<pragma::util::Matrix2x3Property> const &> : property_converter<pragma::util::Matrix2x3Property> {};
	//

	//
	template<>
	struct DLLNETWORK default_converter<std::shared_ptr<pragma::util::Matrix2x4Property>> : property_converter<pragma::util::Matrix2x4Property> {};

	template<>
	struct DLLNETWORK default_converter<std::shared_ptr<pragma::util::Matrix2x4Property> const &> : property_converter<pragma::util::Matrix2x4Property> {};
	//

	//
	template<>
	struct DLLNETWORK default_converter<std::shared_ptr<pragma::util::Matrix3Property>> : property_converter<pragma::util::Matrix3Property> {};

	template<>
	struct DLLNETWORK default_converter<std::shared_ptr<pragma::util::Matrix3Property> const &> : property_converter<pragma::util::Matrix3Property> {};
	//

	//
	template<>
	struct DLLNETWORK default_converter<std::shared_ptr<pragma::util::Matrix3x2Property>> : property_converter<pragma::util::Matrix3x2Property> {};

	template<>
	struct DLLNETWORK default_converter<std::shared_ptr<pragma::util::Matrix3x2Property> const &> : property_converter<pragma::util::Matrix3x2Property> {};
	//

	//
	template<>
	struct DLLNETWORK default_converter<std::shared_ptr<pragma::util::Matrix3x4Property>> : property_converter<pragma::util::Matrix3x4Property> {};

	template<>
	struct DLLNETWORK default_converter<std::shared_ptr<pragma::util::Matrix3x4Property> const &> : property_converter<pragma::util::Matrix3x4Property> {};
	//

	//
	template<>
	struct DLLNETWORK default_converter<std::shared_ptr<pragma::util::Matrix4x2Property>> : property_converter<pragma::util::Matrix4x2Property> {};

	template<>
	struct DLLNETWORK default_converter<std::shared_ptr<pragma::util::Matrix4x2Property> const &> : property_converter<pragma::util::Matrix4x2Property> {};
	//

	//
	template<>
	struct DLLNETWORK default_converter<std::shared_ptr<pragma::util::Matrix4x3Property>> : property_converter<pragma::util::Matrix4x3Property> {};

	template<>
	struct DLLNETWORK default_converter<std::shared_ptr<pragma::util::Matrix4x3Property> const &> : property_converter<pragma::util::Matrix4x3Property> {};
	//

	//
	template<>
	struct DLLNETWORK default_converter<std::shared_ptr<pragma::util::Matrix4Property>> : property_converter<pragma::util::Matrix4Property> {};

	template<>
	struct DLLNETWORK default_converter<std::shared_ptr<pragma::util::Matrix4Property> const &> : property_converter<pragma::util::Matrix4Property> {};
	//

	//
	template<>
	struct DLLNETWORK default_converter<std::shared_ptr<pragma::util::Vector2Property>> : property_converter<pragma::util::Vector2Property> {};

	template<>
	struct DLLNETWORK default_converter<std::shared_ptr<pragma::util::Vector2Property> const &> : property_converter<pragma::util::Vector2Property> {};
	//

	//
	template<>
	struct DLLNETWORK default_converter<std::shared_ptr<pragma::util::Vector2iProperty>> : property_converter<pragma::util::Vector2iProperty> {};

	template<>
	struct DLLNETWORK default_converter<std::shared_ptr<pragma::util::Vector2iProperty> const &> : property_converter<pragma::util::Vector2iProperty> {};
	//

	//
	template<>
	struct DLLNETWORK default_converter<std::shared_ptr<pragma::util::Vector3Property>> : property_converter<pragma::util::Vector3Property> {};

	template<>
	struct DLLNETWORK default_converter<std::shared_ptr<pragma::util::Vector3Property> const &> : property_converter<pragma::util::Vector3Property> {};
	//

	//
	template<>
	struct DLLNETWORK default_converter<std::shared_ptr<pragma::util::Vector3iProperty>> : property_converter<pragma::util::Vector3iProperty> {};

	template<>
	struct DLLNETWORK default_converter<std::shared_ptr<pragma::util::Vector3iProperty> const &> : property_converter<pragma::util::Vector3iProperty> {};
	//

	//
	template<>
	struct DLLNETWORK default_converter<std::shared_ptr<pragma::util::Vector4Property>> : property_converter<pragma::util::Vector4Property> {};

	template<>
	struct DLLNETWORK default_converter<std::shared_ptr<pragma::util::Vector4Property> const &> : property_converter<pragma::util::Vector4Property> {};
	//

	//
	template<>
	struct DLLNETWORK default_converter<std::shared_ptr<pragma::util::Vector4iProperty>> : property_converter<pragma::util::Vector4iProperty> {};

	template<>
	struct DLLNETWORK default_converter<std::shared_ptr<pragma::util::Vector4iProperty> const &> : property_converter<pragma::util::Vector4iProperty> {};
	//
}

export {
	template<typename T>
	void luabind::push_property(lua::State *l, T &prop)
	{
		Lua::Property::push(l, prop);
	}
}
