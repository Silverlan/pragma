// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;


export module pragma.shared:scripting.lua.converters.property;

export import :scripting.lua.classes.property;
export import pragma.util;

export namespace luabind {
	template<typename T>
	using base_type = typename std::remove_cv_t<std::remove_pointer_t<std::remove_reference_t<T>>>;

	// Note: Method definitions for these should be moved to "property_converter_t.hpp", but due to a msvc compile error (21-08-08)
	// this is currently not possible

	template<typename T>
	void push_property(lua::State *l, T &prop);

	template<typename T>
	    requires(std::derived_from<base_type<T>, util::BaseProperty>)
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

		void to_lua(lua::State *L, T const &x) { push_property(L, const_cast<T &>(x)); }
		void to_lua(lua::State *L, std::remove_reference_t<T> *x)
		{
			if(!x)
				Lua::PushNil(L);
			else
				to_lua(L, *x);
		}
	};

	template<typename TProp>
	    requires(std::derived_from<base_type<TProp>, util::BaseProperty>)
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
	struct default_converter<std::shared_ptr<util::ColorProperty>> : property_converter<util::ColorProperty> {};

	template<>
	struct default_converter<std::shared_ptr<util::ColorProperty> const &> : property_converter<util::ColorProperty> {};
	//

	//
	template<>
	struct default_converter<std::shared_ptr<util::BoolProperty>> : property_converter<util::BoolProperty> {};

	template<>
	struct default_converter<std::shared_ptr<util::BoolProperty> const &> : property_converter<util::BoolProperty> {};
	//

	//
	template<>
	struct default_converter<std::shared_ptr<util::Int8Property>> : property_converter<util::Int8Property> {};

	template<>
	struct default_converter<std::shared_ptr<util::Int8Property> const &> : property_converter<util::Int8Property> {};
	//

	//
	template<>
	struct default_converter<std::shared_ptr<util::UInt8Property>> : property_converter<util::UInt8Property> {};

	template<>
	struct default_converter<std::shared_ptr<util::UInt8Property> const &> : property_converter<util::UInt8Property> {};
	//

	//
	template<>
	struct default_converter<std::shared_ptr<util::Int16Property>> : property_converter<util::Int16Property> {};

	template<>
	struct default_converter<std::shared_ptr<util::Int16Property> const &> : property_converter<util::Int16Property> {};
	//

	//
	template<>
	struct default_converter<std::shared_ptr<util::UInt16Property>> : property_converter<util::UInt16Property> {};

	template<>
	struct default_converter<std::shared_ptr<util::UInt16Property> const &> : property_converter<util::UInt16Property> {};
	//

	//
	template<>
	struct default_converter<std::shared_ptr<util::Int32Property>> : property_converter<util::Int32Property> {};

	template<>
	struct default_converter<std::shared_ptr<util::Int32Property> const &> : property_converter<util::Int32Property> {};
	//

	//
	template<>
	struct default_converter<std::shared_ptr<util::UInt32Property>> : property_converter<util::UInt32Property> {};

	template<>
	struct default_converter<std::shared_ptr<util::UInt32Property> const &> : property_converter<util::UInt32Property> {};
	//

	//
	template<>
	struct default_converter<std::shared_ptr<util::Int64Property>> : property_converter<util::Int64Property> {};

	template<>
	struct default_converter<std::shared_ptr<util::Int64Property> const &> : property_converter<util::Int64Property> {};
	//

	//
	template<>
	struct default_converter<std::shared_ptr<util::UInt64Property>> : property_converter<util::UInt64Property> {};

	template<>
	struct default_converter<std::shared_ptr<util::UInt64Property> const &> : property_converter<util::UInt64Property> {};
	//

	//
	template<>
	struct default_converter<std::shared_ptr<util::FloatProperty>> : property_converter<util::FloatProperty> {};

	template<>
	struct default_converter<std::shared_ptr<util::FloatProperty> const &> : property_converter<util::FloatProperty> {};
	//

	//
	template<>
	struct default_converter<std::shared_ptr<util::DoubleProperty>> : property_converter<util::DoubleProperty> {};

	template<>
	struct default_converter<std::shared_ptr<util::DoubleProperty> const &> : property_converter<util::DoubleProperty> {};
	//

	//
	template<>
	struct default_converter<std::shared_ptr<util::LongDoubleProperty>> : property_converter<util::LongDoubleProperty> {};

	template<>
	struct default_converter<std::shared_ptr<util::LongDoubleProperty> const &> : property_converter<util::LongDoubleProperty> {};
	//

	//
	template<>
	struct default_converter<std::shared_ptr<util::EulerAnglesProperty>> : property_converter<util::EulerAnglesProperty> {};

	template<>
	struct default_converter<std::shared_ptr<util::EulerAnglesProperty> const &> : property_converter<util::EulerAnglesProperty> {};
	//

	//
	template<>
	struct default_converter<std::shared_ptr<util::QuatProperty>> : property_converter<util::QuatProperty> {};

	template<>
	struct default_converter<std::shared_ptr<util::QuatProperty> const &> : property_converter<util::QuatProperty> {};
	//

	//
	template<>
	struct default_converter<std::shared_ptr<util::Matrix2Property>> : property_converter<util::Matrix2Property> {};

	template<>
	struct default_converter<std::shared_ptr<util::Matrix2Property> const &> : property_converter<util::Matrix2Property> {};
	//

	//
	template<>
	struct default_converter<std::shared_ptr<util::Matrix2x3Property>> : property_converter<util::Matrix2x3Property> {};

	template<>
	struct default_converter<std::shared_ptr<util::Matrix2x3Property> const &> : property_converter<util::Matrix2x3Property> {};
	//

	//
	template<>
	struct default_converter<std::shared_ptr<util::Matrix2x4Property>> : property_converter<util::Matrix2x4Property> {};

	template<>
	struct default_converter<std::shared_ptr<util::Matrix2x4Property> const &> : property_converter<util::Matrix2x4Property> {};
	//

	//
	template<>
	struct default_converter<std::shared_ptr<util::Matrix3Property>> : property_converter<util::Matrix3Property> {};

	template<>
	struct default_converter<std::shared_ptr<util::Matrix3Property> const &> : property_converter<util::Matrix3Property> {};
	//

	//
	template<>
	struct default_converter<std::shared_ptr<util::Matrix3x2Property>> : property_converter<util::Matrix3x2Property> {};

	template<>
	struct default_converter<std::shared_ptr<util::Matrix3x2Property> const &> : property_converter<util::Matrix3x2Property> {};
	//

	//
	template<>
	struct default_converter<std::shared_ptr<util::Matrix3x4Property>> : property_converter<util::Matrix3x4Property> {};

	template<>
	struct default_converter<std::shared_ptr<util::Matrix3x4Property> const &> : property_converter<util::Matrix3x4Property> {};
	//

	//
	template<>
	struct default_converter<std::shared_ptr<util::Matrix4x2Property>> : property_converter<util::Matrix4x2Property> {};

	template<>
	struct default_converter<std::shared_ptr<util::Matrix4x2Property> const &> : property_converter<util::Matrix4x2Property> {};
	//

	//
	template<>
	struct default_converter<std::shared_ptr<util::Matrix4x3Property>> : property_converter<util::Matrix4x3Property> {};

	template<>
	struct default_converter<std::shared_ptr<util::Matrix4x3Property> const &> : property_converter<util::Matrix4x3Property> {};
	//

	//
	template<>
	struct default_converter<std::shared_ptr<util::Matrix4Property>> : property_converter<util::Matrix4Property> {};

	template<>
	struct default_converter<std::shared_ptr<util::Matrix4Property> const &> : property_converter<util::Matrix4Property> {};
	//

	//
	template<>
	struct default_converter<std::shared_ptr<util::Vector2Property>> : property_converter<util::Vector2Property> {};

	template<>
	struct default_converter<std::shared_ptr<util::Vector2Property> const &> : property_converter<util::Vector2Property> {};
	//

	//
	template<>
	struct default_converter<std::shared_ptr<util::Vector2iProperty>> : property_converter<util::Vector2iProperty> {};

	template<>
	struct default_converter<std::shared_ptr<util::Vector2iProperty> const &> : property_converter<util::Vector2iProperty> {};
	//

	//
	template<>
	struct default_converter<std::shared_ptr<util::Vector3Property>> : property_converter<util::Vector3Property> {};

	template<>
	struct default_converter<std::shared_ptr<util::Vector3Property> const &> : property_converter<util::Vector3Property> {};
	//

	//
	template<>
	struct default_converter<std::shared_ptr<util::Vector3iProperty>> : property_converter<util::Vector3iProperty> {};

	template<>
	struct default_converter<std::shared_ptr<util::Vector3iProperty> const &> : property_converter<util::Vector3iProperty> {};
	//

	//
	template<>
	struct default_converter<std::shared_ptr<util::Vector4Property>> : property_converter<util::Vector4Property> {};

	template<>
	struct default_converter<std::shared_ptr<util::Vector4Property> const &> : property_converter<util::Vector4Property> {};
	//

	//
	template<>
	struct default_converter<std::shared_ptr<util::Vector4iProperty>> : property_converter<util::Vector4iProperty> {};

	template<>
	struct default_converter<std::shared_ptr<util::Vector4iProperty> const &> : property_converter<util::Vector4iProperty> {};
	//
}

export {
	template<typename T>
	void luabind::push_property(lua::State *l, T &prop)
	{
		Lua::Property::push(l, prop);
	}
}
