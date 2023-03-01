/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "pragma/lua/converters/alias_types.hpp"
#include "pragma/lua/converters/alias_converter_t.hpp"

void luabind::detail::AliasTypeConverter<Vector3, Vector2>::convert(const Vector2 &srcValue, Vector3 &outValue) { outValue = {srcValue.x, srcValue.y, 0.f}; }
void luabind::detail::AliasTypeConverter<Vector3i, Vector2>::convert(const Vector2 &srcValue, Vector3i &outValue) { outValue = {srcValue.x, srcValue.y, 0.f}; }
void luabind::detail::AliasTypeConverter<Vector3, Vector2i>::convert(const Vector2i &srcValue, Vector3 &outValue) { outValue = {srcValue.x, srcValue.y, 0.f}; }
void luabind::detail::AliasTypeConverter<Vector3i, Vector2i>::convert(const Vector2i &srcValue, Vector3i &outValue) { outValue = {srcValue.x, srcValue.y, 0.f}; }
void luabind::detail::AliasTypeConverter<Vector4, Vector2>::convert(const Vector2 &srcValue, Vector4 &outValue) { outValue = {srcValue.x, srcValue.y, 0.f, 0.f}; }
void luabind::detail::AliasTypeConverter<Vector4i, Vector2>::convert(const Vector2 &srcValue, Vector4i &outValue) { outValue = {srcValue.x, srcValue.y, 0.f, 0.f}; }
void luabind::detail::AliasTypeConverter<Vector4, Vector2i>::convert(const Vector2i &srcValue, Vector4 &outValue) { outValue = {srcValue.x, srcValue.y, 0.f, 0.f}; }
void luabind::detail::AliasTypeConverter<Vector4i, Vector2i>::convert(const Vector2i &srcValue, Vector4i &outValue) { outValue = {srcValue.x, srcValue.y, 0.f, 0.f}; }
void luabind::detail::AliasTypeConverter<Vector4, Vector3>::convert(const Vector3 &srcValue, Vector4 &outValue) { outValue = {srcValue.x, srcValue.y, srcValue.z, 0.f}; }
void luabind::detail::AliasTypeConverter<Vector4i, Vector3>::convert(const Vector3 &srcValue, Vector4i &outValue) { outValue = {srcValue.x, srcValue.y, srcValue.z, 0.f}; }
void luabind::detail::AliasTypeConverter<Vector4, Vector3i>::convert(const Vector3i &srcValue, Vector4 &outValue) { outValue = {srcValue.x, srcValue.y, srcValue.z, 0.f}; }
void luabind::detail::AliasTypeConverter<Vector4i, Vector3i>::convert(const Vector3i &srcValue, Vector4i &outValue) { outValue = {srcValue.x, srcValue.y, srcValue.z, 0.f}; }
void luabind::detail::AliasTypeConverter<EulerAngles, Quat>::convert(const Quat &srcValue, EulerAngles &outValue) { outValue = {srcValue}; }
void luabind::detail::AliasTypeConverter<Quat, EulerAngles>::convert(const EulerAngles &srcValue, Quat &outValue) { outValue = uquat::create(srcValue); }
void luabind::detail::AliasTypeConverter<util::Path, std::string>::convert(const std::string &srcValue, util::Path &outValue) { outValue = {srcValue}; }
void luabind::detail::AliasTypeConverter<std::string, util::Path>::convert(const util::Path &srcValue, std::string &outValue) { outValue = srcValue.GetString(); }

namespace luabind {
	// Instantiations
	template struct DLLNETWORK luabind::default_converter<Vector2>;
	template struct DLLNETWORK luabind::default_converter<Vector2i>;
	template struct DLLNETWORK luabind::default_converter<Vector3>;
	template struct DLLNETWORK luabind::default_converter<Vector3i>;
	template struct DLLNETWORK luabind::default_converter<Vector4>;
	template struct DLLNETWORK luabind::default_converter<Vector4i>;
	template struct DLLNETWORK luabind::default_converter<EulerAngles>;
	template struct DLLNETWORK luabind::default_converter<Quat>;
	template struct DLLNETWORK luabind::default_converter<std::string>;
	template struct DLLNETWORK luabind::default_converter<util::Path>;

	template struct DLLNETWORK alias_converter<Vector2, Vector2, Vector2i, Vector3, Vector3i, Vector4, Vector4i>;
	template struct DLLNETWORK alias_converter<Vector2i, Vector2, Vector2i, Vector3, Vector3i, Vector4, Vector4i>;
	template struct DLLNETWORK alias_converter<Vector3, Vector2, Vector2i, Vector3, Vector3i, Vector4, Vector4i>;
	template struct DLLNETWORK alias_converter<Vector3i, Vector2, Vector2i, Vector3, Vector3i, Vector4, Vector4i>;
	template struct DLLNETWORK alias_converter<Vector4, Vector2, Vector2i, Vector3, Vector3i, Vector4, Vector4i>;
	template struct DLLNETWORK alias_converter<Vector4i, Vector2, Vector2i, Vector3, Vector3i, Vector4, Vector4i>;
	template struct DLLNETWORK alias_converter<EulerAngles, EulerAngles, Quat>;
	template struct DLLNETWORK alias_converter<Quat, EulerAngles, Quat>;
	template struct DLLNETWORK alias_converter<std::string, std::string, util::Path>;
	template struct DLLNETWORK alias_converter<util::Path, std::string, util::Path>;

	template struct DLLNETWORK alias_converter<const Vector2 &, Vector2, Vector2i, Vector3, Vector3i, Vector4, Vector4i>;
	template struct DLLNETWORK alias_converter<const Vector2i &, Vector2, Vector2i, Vector3, Vector3i, Vector4, Vector4i>;
	template struct DLLNETWORK alias_converter<const Vector3 &, Vector2, Vector2i, Vector3, Vector3i, Vector4, Vector4i>;
	template struct DLLNETWORK alias_converter<const Vector3i &, Vector2, Vector2i, Vector3, Vector3i, Vector4, Vector4i>;
	template struct DLLNETWORK alias_converter<const Vector4 &, Vector2, Vector2i, Vector3, Vector3i, Vector4, Vector4i>;
	template struct DLLNETWORK alias_converter<const Vector4i &, Vector2, Vector2i, Vector3, Vector3i, Vector4, Vector4i>;
	template struct DLLNETWORK alias_converter<const EulerAngles &, EulerAngles, Quat>;
	template struct DLLNETWORK alias_converter<const Quat &, EulerAngles, Quat>;
	template struct DLLNETWORK alias_converter<const std::string &, std::string, util::Path>;
	template struct DLLNETWORK alias_converter<const util::Path &, std::string, util::Path>;

#define INSTANTIATE_TO_CPP_LB_TYPE_VEC(T, LBT)                                                                                                                                                                                                                                                   \
	template DLLNETWORK T luabind::alias_converter<T, Vector2, Vector2i, Vector3, Vector3i, Vector4, Vector4i>::to_cpp<luabind::LBT<Vector2>>(lua_State *, luabind::LBT<Vector2>, int);                                                                                                          \
	template DLLNETWORK T luabind::alias_converter<T, Vector2, Vector2i, Vector3, Vector3i, Vector4, Vector4i>::to_cpp<luabind::LBT<Vector2i>>(lua_State *, luabind::LBT<Vector2i>, int);                                                                                                        \
	template DLLNETWORK T luabind::alias_converter<T, Vector2, Vector2i, Vector3, Vector3i, Vector4, Vector4i>::to_cpp<luabind::LBT<Vector3>>(lua_State *, luabind::LBT<Vector3>, int);                                                                                                          \
	template DLLNETWORK T luabind::alias_converter<T, Vector2, Vector2i, Vector3, Vector3i, Vector4, Vector4i>::to_cpp<luabind::LBT<Vector3i>>(lua_State *, luabind::LBT<Vector3i>, int);                                                                                                        \
	template DLLNETWORK T luabind::alias_converter<T, Vector2, Vector2i, Vector3, Vector3i, Vector4, Vector4i>::to_cpp<luabind::LBT<Vector4>>(lua_State *, luabind::LBT<Vector4>, int);                                                                                                          \
	template DLLNETWORK T luabind::alias_converter<T, Vector2, Vector2i, Vector3, Vector3i, Vector4, Vector4i>::to_cpp<luabind::LBT<Vector4i>>(lua_State *, luabind::LBT<Vector4i>, int);                                                                                                        \
	template DLLNETWORK int luabind::alias_converter<T, Vector2, Vector2i, Vector3, Vector3i, Vector4, Vector4i>::match<luabind::LBT<Vector2>>(lua_State *, luabind::LBT<Vector2>, int);                                                                                                         \
	template DLLNETWORK int luabind::alias_converter<T, Vector2, Vector2i, Vector3, Vector3i, Vector4, Vector4i>::match<luabind::LBT<Vector2i>>(lua_State *, luabind::LBT<Vector2i>, int);                                                                                                       \
	template DLLNETWORK int luabind::alias_converter<T, Vector2, Vector2i, Vector3, Vector3i, Vector4, Vector4i>::match<luabind::LBT<Vector3>>(lua_State *, luabind::LBT<Vector3>, int);                                                                                                         \
	template DLLNETWORK int luabind::alias_converter<T, Vector2, Vector2i, Vector3, Vector3i, Vector4, Vector4i>::match<luabind::LBT<Vector3i>>(lua_State *, luabind::LBT<Vector3i>, int);                                                                                                       \
	template DLLNETWORK int luabind::alias_converter<T, Vector2, Vector2i, Vector3, Vector3i, Vector4, Vector4i>::match<luabind::LBT<Vector4>>(lua_State *, luabind::LBT<Vector4>, int);                                                                                                         \
	template DLLNETWORK int luabind::alias_converter<T, Vector2, Vector2i, Vector3, Vector3i, Vector4, Vector4i>::match<luabind::LBT<Vector4i>>(lua_State *, luabind::LBT<Vector4i>, int);

#define INSTANTIATE_TO_CPP_VEC(T)                                                                                                                                                                                                                                                                \
	INSTANTIATE_TO_CPP_LB_TYPE_VEC(T, by_value)                                                                                                                                                                                                                                                  \
	INSTANTIATE_TO_CPP_LB_TYPE_VEC(const T &, by_const_reference)

	INSTANTIATE_TO_CPP_VEC(Vector2);
	INSTANTIATE_TO_CPP_VEC(Vector2i);
	INSTANTIATE_TO_CPP_VEC(Vector3);
	INSTANTIATE_TO_CPP_VEC(Vector3i);
	INSTANTIATE_TO_CPP_VEC(Vector4);
	INSTANTIATE_TO_CPP_VEC(Vector4i);

#define INSTANTIATE_TWOWAY_value(T0, T1)                                                                                                                                                                                                                                                         \
	template DLLNETWORK T0 luabind::alias_converter<T0, T1, T0>::to_cpp<luabind::by_value<T0>>(lua_State *, luabind::by_value<T0>, int);                                                                                                                                                         \
	template DLLNETWORK T0 luabind::alias_converter<T0, T1, T0>::to_cpp<luabind::by_value<T1>>(lua_State *, luabind::by_value<T1>, int);                                                                                                                                                         \
	template DLLNETWORK T1 luabind::alias_converter<T1, T1, T0>::to_cpp<luabind::by_value<T0>>(lua_State *, luabind::by_value<T0>, int);                                                                                                                                                         \
	template DLLNETWORK T1 luabind::alias_converter<T1, T1, T0>::to_cpp<luabind::by_value<T1>>(lua_State *, luabind::by_value<T1>, int);                                                                                                                                                         \
                                                                                                                                                                                                                                                                                                 \
	template DLLNETWORK int luabind::alias_converter<T0, T1, T0>::match<luabind::by_value<T0>>(lua_State *, luabind::by_value<T0>, int);                                                                                                                                                         \
	template DLLNETWORK int luabind::alias_converter<T0, T1, T0>::match<luabind::by_value<T1>>(lua_State *, luabind::by_value<T1>, int);                                                                                                                                                         \
	template DLLNETWORK int luabind::alias_converter<T1, T1, T0>::match<luabind::by_value<T0>>(lua_State *, luabind::by_value<T0>, int);                                                                                                                                                         \
	template DLLNETWORK int luabind::alias_converter<T1, T1, T0>::match<luabind::by_value<T1>>(lua_State *, luabind::by_value<T1>, int);

#define INSTANTIATE_TWOWAY(T0, T1)                                                                                                                                                                                                                                                               \
	template DLLNETWORK const T0 &luabind::alias_converter<const T0 &, T1, T0>::to_cpp<luabind::by_const_reference<T0>>(lua_State *, luabind::by_const_reference<T0>, int);                                                                                                                      \
	template DLLNETWORK const T0 &luabind::alias_converter<const T0 &, T1, T0>::to_cpp<luabind::by_const_reference<T1>>(lua_State *, luabind::by_const_reference<T1>, int);                                                                                                                      \
	template DLLNETWORK const T1 &luabind::alias_converter<const T1 &, T1, T0>::to_cpp<luabind::by_const_reference<T0>>(lua_State *, luabind::by_const_reference<T0>, int);                                                                                                                      \
	template DLLNETWORK const T1 &luabind::alias_converter<const T1 &, T1, T0>::to_cpp<luabind::by_const_reference<T1>>(lua_State *, luabind::by_const_reference<T1>, int);                                                                                                                      \
                                                                                                                                                                                                                                                                                                 \
	template DLLNETWORK int luabind::alias_converter<const T0 &, T1, T0>::match<luabind::by_const_reference<T0>>(lua_State *, luabind::by_const_reference<T0>, int);                                                                                                                             \
	template DLLNETWORK int luabind::alias_converter<const T0 &, T1, T0>::match<luabind::by_const_reference<T1>>(lua_State *, luabind::by_const_reference<T1>, int);                                                                                                                             \
	template DLLNETWORK int luabind::alias_converter<const T1 &, T1, T0>::match<luabind::by_const_reference<T0>>(lua_State *, luabind::by_const_reference<T0>, int);                                                                                                                             \
	template DLLNETWORK int luabind::alias_converter<const T1 &, T1, T0>::match<luabind::by_const_reference<T1>>(lua_State *, luabind::by_const_reference<T1>, int);                                                                                                                             \
	INSTANTIATE_TWOWAY_value(T0, T1);

	INSTANTIATE_TWOWAY(Quat, EulerAngles);
	//INSTANTIATE_TWOWAY_value(std::string,util::Path);
};
