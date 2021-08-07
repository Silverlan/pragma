/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "pragma/lua/converters/alias_types.hpp"

void luabind::detail::AliasTypeConverter<Vector3,Vector2>::convert(const Vector2 &srcValue,Vector3 &outValue) {outValue = {srcValue.x,srcValue.y,0.f};}
void luabind::detail::AliasTypeConverter<Vector3,Vector2i>::convert(const Vector2i &srcValue,Vector3 &outValue) {outValue = {srcValue.x,srcValue.y,0.f};}
void luabind::detail::AliasTypeConverter<Vector4,Vector2>::convert(const Vector2 &srcValue,Vector4 &outValue) {outValue = {srcValue.x,srcValue.y,0.f,0.f};}
void luabind::detail::AliasTypeConverter<Vector4,Vector2i>::convert(const Vector2i &srcValue,Vector4 &outValue) {outValue = {srcValue.x,srcValue.y,0.f,0.f};}
void luabind::detail::AliasTypeConverter<Vector4,Vector3>::convert(const Vector3 &srcValue,Vector4 &outValue) {outValue = {srcValue.x,srcValue.y,srcValue.z,0.f};}
void luabind::detail::AliasTypeConverter<Vector4,Vector3i>::convert(const Vector3i &srcValue,Vector4 &outValue) {outValue = {srcValue.x,srcValue.y,srcValue.z,0.f};}
void luabind::detail::AliasTypeConverter<EulerAngles,Quat>::convert(const Quat &srcValue,EulerAngles &outValue) {outValue = {srcValue};}
void luabind::detail::AliasTypeConverter<Quat,EulerAngles>::convert(const EulerAngles &srcValue,Quat &outValue) {outValue = uquat::create(srcValue);}
