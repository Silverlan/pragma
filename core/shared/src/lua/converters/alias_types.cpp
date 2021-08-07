/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "pragma/lua/converters/alias_types.hpp"
#include "pragma/lua/converters/alias_converter_t.hpp"

void luabind::detail::AliasTypeConverter<Vector3,Vector2>::convert(const Vector2 &srcValue,Vector3 &outValue) {outValue = {srcValue.x,srcValue.y,0.f};}
void luabind::detail::AliasTypeConverter<Vector3i,Vector2>::convert(const Vector2 &srcValue,Vector3i &outValue) {outValue = {srcValue.x,srcValue.y,0.f};}
void luabind::detail::AliasTypeConverter<Vector3,Vector2i>::convert(const Vector2i &srcValue,Vector3 &outValue) {outValue = {srcValue.x,srcValue.y,0.f};}
void luabind::detail::AliasTypeConverter<Vector3i,Vector2i>::convert(const Vector2i &srcValue,Vector3i &outValue) {outValue = {srcValue.x,srcValue.y,0.f};}
void luabind::detail::AliasTypeConverter<Vector4,Vector2>::convert(const Vector2 &srcValue,Vector4 &outValue) {outValue = {srcValue.x,srcValue.y,0.f,0.f};}
void luabind::detail::AliasTypeConverter<Vector4i,Vector2>::convert(const Vector2 &srcValue,Vector4i &outValue) {outValue = {srcValue.x,srcValue.y,0.f,0.f};}
void luabind::detail::AliasTypeConverter<Vector4,Vector2i>::convert(const Vector2i &srcValue,Vector4 &outValue) {outValue = {srcValue.x,srcValue.y,0.f,0.f};}
void luabind::detail::AliasTypeConverter<Vector4i,Vector2i>::convert(const Vector2i &srcValue,Vector4i &outValue) {outValue = {srcValue.x,srcValue.y,0.f,0.f};}
void luabind::detail::AliasTypeConverter<Vector4,Vector3>::convert(const Vector3 &srcValue,Vector4 &outValue) {outValue = {srcValue.x,srcValue.y,srcValue.z,0.f};}
void luabind::detail::AliasTypeConverter<Vector4i,Vector3>::convert(const Vector3 &srcValue,Vector4i &outValue) {outValue = {srcValue.x,srcValue.y,srcValue.z,0.f};}
void luabind::detail::AliasTypeConverter<Vector4,Vector3i>::convert(const Vector3i &srcValue,Vector4 &outValue) {outValue = {srcValue.x,srcValue.y,srcValue.z,0.f};}
void luabind::detail::AliasTypeConverter<Vector4i,Vector3i>::convert(const Vector3i &srcValue,Vector4i &outValue) {outValue = {srcValue.x,srcValue.y,srcValue.z,0.f};}
void luabind::detail::AliasTypeConverter<EulerAngles,Quat>::convert(const Quat &srcValue,EulerAngles &outValue) {outValue = {srcValue};}
void luabind::detail::AliasTypeConverter<Quat,EulerAngles>::convert(const EulerAngles &srcValue,Quat &outValue) {outValue = uquat::create(srcValue);}

namespace luabind
{
	// Instantiations
	template struct DLLNETWORK luabind::default_converter<Vector2>;
	template struct DLLNETWORK luabind::default_converter<Vector2i>;
	template struct DLLNETWORK luabind::default_converter<Vector3>;
	template struct DLLNETWORK luabind::default_converter<Vector3i>;
	template struct DLLNETWORK luabind::default_converter<Vector4>;
	template struct DLLNETWORK luabind::default_converter<Vector4i>;
	template struct DLLNETWORK luabind::default_converter<EulerAngles>;
	template struct DLLNETWORK luabind::default_converter<Quat>;

	template struct DLLNETWORK alias_converter<Vector2,Vector2,Vector2i,Vector3,Vector3i,Vector4,Vector4i>;
	template struct DLLNETWORK alias_converter<Vector2i,Vector2,Vector2i,Vector3,Vector3i,Vector4,Vector4i>;
	template struct DLLNETWORK alias_converter<Vector3,Vector2,Vector2i,Vector3,Vector3i,Vector4,Vector4i>;
	template struct DLLNETWORK alias_converter<Vector3i,Vector2,Vector2i,Vector3,Vector3i,Vector4,Vector4i>;
	template struct DLLNETWORK alias_converter<Vector4,Vector2,Vector2i,Vector3,Vector3i,Vector4,Vector4i>;
	template struct DLLNETWORK alias_converter<Vector4i,Vector2,Vector2i,Vector3,Vector3i,Vector4,Vector4i>;
	template struct DLLNETWORK alias_converter<EulerAngles,EulerAngles,Quat>;
	template struct DLLNETWORK alias_converter<Quat,EulerAngles,Quat>;

	template struct DLLNETWORK alias_converter<const Vector2&,Vector2,Vector2i,Vector3,Vector3i,Vector4,Vector4i>;
	template struct DLLNETWORK alias_converter<const Vector2i&,Vector2,Vector2i,Vector3,Vector3i,Vector4,Vector4i>;
	template struct DLLNETWORK alias_converter<const Vector3&,Vector2,Vector2i,Vector3,Vector3i,Vector4,Vector4i>;
	template struct DLLNETWORK alias_converter<const Vector3i&,Vector2,Vector2i,Vector3,Vector3i,Vector4,Vector4i>;
	template struct DLLNETWORK alias_converter<const Vector4&,Vector2,Vector2i,Vector3,Vector3i,Vector4,Vector4i>;
	template struct DLLNETWORK alias_converter<const Vector4i&,Vector2,Vector2i,Vector3,Vector3i,Vector4,Vector4i>;
	template struct DLLNETWORK alias_converter<const EulerAngles&,EulerAngles,Quat>;
	template struct DLLNETWORK alias_converter<const Quat&,EulerAngles,Quat>;

#define INSTANTIATE_TO_CPP_LB_TYPE_VEC(T,LBT) \
	template DLLNETWORK T luabind::alias_converter<T,Vector2,Vector2i,Vector3,Vector3i,Vector4,Vector4i>::to_cpp<luabind::LBT<Vector2>>(lua_State*,luabind::LBT<Vector2>,int); \
	template DLLNETWORK T luabind::alias_converter<T,Vector2,Vector2i,Vector3,Vector3i,Vector4,Vector4i>::to_cpp<luabind::LBT<Vector2i>>(lua_State*,luabind::LBT<Vector2i>,int); \
	template DLLNETWORK T luabind::alias_converter<T,Vector2,Vector2i,Vector3,Vector3i,Vector4,Vector4i>::to_cpp<luabind::LBT<Vector3>>(lua_State*,luabind::LBT<Vector3>,int); \
	template DLLNETWORK T luabind::alias_converter<T,Vector2,Vector2i,Vector3,Vector3i,Vector4,Vector4i>::to_cpp<luabind::LBT<Vector3i>>(lua_State*,luabind::LBT<Vector3i>,int); \
	template DLLNETWORK T luabind::alias_converter<T,Vector2,Vector2i,Vector3,Vector3i,Vector4,Vector4i>::to_cpp<luabind::LBT<Vector4>>(lua_State*,luabind::LBT<Vector4>,int); \
	template DLLNETWORK T luabind::alias_converter<T,Vector2,Vector2i,Vector3,Vector3i,Vector4,Vector4i>::to_cpp<luabind::LBT<Vector4i>>(lua_State*,luabind::LBT<Vector4i>,int); \
	template DLLNETWORK int luabind::alias_converter<T,Vector2,Vector2i,Vector3,Vector3i,Vector4,Vector4i>::match<luabind::LBT<Vector2>>(lua_State*,luabind::LBT<Vector2>,int); \
	template DLLNETWORK int luabind::alias_converter<T,Vector2,Vector2i,Vector3,Vector3i,Vector4,Vector4i>::match<luabind::LBT<Vector2i>>(lua_State*,luabind::LBT<Vector2i>,int); \
	template DLLNETWORK int luabind::alias_converter<T,Vector2,Vector2i,Vector3,Vector3i,Vector4,Vector4i>::match<luabind::LBT<Vector3>>(lua_State*,luabind::LBT<Vector3>,int); \
	template DLLNETWORK int luabind::alias_converter<T,Vector2,Vector2i,Vector3,Vector3i,Vector4,Vector4i>::match<luabind::LBT<Vector3i>>(lua_State*,luabind::LBT<Vector3i>,int); \
	template DLLNETWORK int luabind::alias_converter<T,Vector2,Vector2i,Vector3,Vector3i,Vector4,Vector4i>::match<luabind::LBT<Vector4>>(lua_State*,luabind::LBT<Vector4>,int); \
	template DLLNETWORK int luabind::alias_converter<T,Vector2,Vector2i,Vector3,Vector3i,Vector4,Vector4i>::match<luabind::LBT<Vector4i>>(lua_State*,luabind::LBT<Vector4i>,int);

#define INSTANTIATE_TO_CPP_VEC(T) \
	INSTANTIATE_TO_CPP_LB_TYPE_VEC(T,by_value) \
	INSTANTIATE_TO_CPP_LB_TYPE_VEC(const T&,by_const_reference)
	
	INSTANTIATE_TO_CPP_VEC(Vector2);
	INSTANTIATE_TO_CPP_VEC(Vector2i);
	INSTANTIATE_TO_CPP_VEC(Vector3);
	INSTANTIATE_TO_CPP_VEC(Vector3i);
	INSTANTIATE_TO_CPP_VEC(Vector4);
	INSTANTIATE_TO_CPP_VEC(Vector4i);

	template DLLNETWORK const Quat &luabind::alias_converter<const Quat&,EulerAngles,Quat>::to_cpp<luabind::by_const_reference<Quat>>(lua_State*,luabind::by_const_reference<Quat>,int);
	template DLLNETWORK const Quat &luabind::alias_converter<const Quat&,EulerAngles,Quat>::to_cpp<luabind::by_const_reference<EulerAngles>>(lua_State*,luabind::by_const_reference<EulerAngles>,int);
	template DLLNETWORK const EulerAngles &luabind::alias_converter<const EulerAngles&,EulerAngles,Quat>::to_cpp<luabind::by_const_reference<Quat>>(lua_State*,luabind::by_const_reference<Quat>,int);
	template DLLNETWORK const EulerAngles &luabind::alias_converter<const EulerAngles&,EulerAngles,Quat>::to_cpp<luabind::by_const_reference<EulerAngles>>(lua_State*,luabind::by_const_reference<EulerAngles>,int);
	template DLLNETWORK Quat luabind::alias_converter<Quat,EulerAngles,Quat>::to_cpp<luabind::by_value<Quat>>(lua_State*,luabind::by_value<Quat>,int);
	template DLLNETWORK Quat luabind::alias_converter<Quat,EulerAngles,Quat>::to_cpp<luabind::by_value<EulerAngles>>(lua_State*,luabind::by_value<EulerAngles>,int);
	template DLLNETWORK EulerAngles luabind::alias_converter<EulerAngles,EulerAngles,Quat>::to_cpp<luabind::by_value<Quat>>(lua_State*,luabind::by_value<Quat>,int);
	template DLLNETWORK EulerAngles luabind::alias_converter<EulerAngles,EulerAngles,Quat>::to_cpp<luabind::by_value<EulerAngles>>(lua_State*,luabind::by_value<EulerAngles>,int);

	template DLLNETWORK int luabind::alias_converter<const Quat&,EulerAngles,Quat>::match<luabind::by_const_reference<Quat>>(lua_State*,luabind::by_const_reference<Quat>,int);
	template DLLNETWORK int luabind::alias_converter<const Quat&,EulerAngles,Quat>::match<luabind::by_const_reference<EulerAngles>>(lua_State*,luabind::by_const_reference<EulerAngles>,int);
	template DLLNETWORK int luabind::alias_converter<const EulerAngles&,EulerAngles,Quat>::match<luabind::by_const_reference<Quat>>(lua_State*,luabind::by_const_reference<Quat>,int);
	template DLLNETWORK int luabind::alias_converter<const EulerAngles&,EulerAngles,Quat>::match<luabind::by_const_reference<EulerAngles>>(lua_State*,luabind::by_const_reference<EulerAngles>,int);
	template DLLNETWORK int luabind::alias_converter<Quat,EulerAngles,Quat>::match<luabind::by_value<Quat>>(lua_State*,luabind::by_value<Quat>,int);
	template DLLNETWORK int luabind::alias_converter<Quat,EulerAngles,Quat>::match<luabind::by_value<EulerAngles>>(lua_State*,luabind::by_value<EulerAngles>,int);
	template DLLNETWORK int luabind::alias_converter<EulerAngles,EulerAngles,Quat>::match<luabind::by_value<Quat>>(lua_State*,luabind::by_value<Quat>,int);
	template DLLNETWORK int luabind::alias_converter<EulerAngles,EulerAngles,Quat>::match<luabind::by_value<EulerAngles>>(lua_State*,luabind::by_value<EulerAngles>,int);
};
