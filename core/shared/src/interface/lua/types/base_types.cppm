// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

export module pragma.shared:scripting.lua.types.base_types;

export import luabind;
export import std;

export namespace luabind {
	template<typename T, typename U>
	struct copy_qualifiers {
	  private:
		using R = std::remove_pointer_t<std::remove_reference_t<T>>;
		using U1 = std::conditional_t<std::is_const<R>::value, std::add_const_t<U>, U>;
		using U2 = std::conditional_t<std::is_volatile<R>::value, std::add_volatile_t<U1>, U1>;
		using U3 = std::conditional_t<std::is_lvalue_reference<T>::value, std::add_lvalue_reference_t<U2>, U2>;
		using U4 = std::conditional_t<std::is_rvalue_reference<T>::value, std::add_rvalue_reference_t<U3>, U3>;
		using U5 = std::conditional_t<std::is_pointer<T>::value, std::add_pointer_t<U4>, U4>;
	  public:
		using type = U5;
	};

	template<typename T, typename U>
	using copy_qualifiers_t = typename copy_qualifiers<T, U>::type;

	template<typename T>
	using base_type = typename std::remove_cv_t<std::remove_pointer_t<std::remove_reference_t<T>>>;

	template<class T, class Test>
	concept is_type_or_derived = std::is_same_v<base_type<T>, Test> || std::derived_from<base_type<T>, Test>;

	template<typename T, typename TTarget>
	concept is_decorated_type_target = std::is_same_v<base_type<typename T::value_type>, TTarget> || std::derived_from<base_type<typename T::value_type>, TTarget>;

	template<typename T>
	concept is_native_type = std::is_arithmetic_v<T> || std::is_same_v<base_type<T>, std::string> || std::is_same_v<T, const char *>;

	template<typename T>
	using type_converter = std::conditional_t<is_native_type<T>, default_converter<T>,
	  std::conditional_t<std::is_pointer_v<T>, std::conditional_t<std::is_const_v<std::remove_pointer_t<T>>, detail::const_pointer_converter, detail::pointer_converter>,
	    std::conditional_t<std::is_reference_v<T>, std::conditional_t<std::is_const_v<std::remove_reference_t<T>>, detail::const_ref_converter, detail::ref_converter>, detail::value_converter>>>;
};
