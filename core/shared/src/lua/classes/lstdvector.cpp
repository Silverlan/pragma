/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*
* Copyright (c) 2021 Silverlan
*/

#include "stdafx_shared.h"
#include "pragma/lua/libraries/lutil.hpp"
#include "pragma/lua/converters/vector_converter_t.hpp"
#include "pragma/lua/policies/core_policies.hpp"

template<typename T>
static void register_vector(lua_State *l, const char *name)
{
	auto classDefVec = luabind::class_<std::vector<T>>(name);
	classDefVec.def(luabind::constructor<>());
	classDefVec.def("__len", static_cast<size_t (*)(const std::vector<T> &)>([](const std::vector<T> &v) { return v.size(); }), luabind::const_ref_policy<1> {});
	classDefVec.def("PushBack", static_cast<void (*)(std::vector<T> &, const T &)>([](std::vector<T> &v, const T &value) { v.push_back(value); }), luabind::ref_policy<1> {});
	classDefVec.def("Insert", static_cast<void (*)(std::vector<T> &, size_t, const T &)>([](std::vector<T> &v, size_t pos, const T &value) { v.insert(v.begin() + pos, value); }), luabind::ref_policy<1> {});
	classDefVec.def("Erase", static_cast<void (*)(std::vector<T> &, size_t)>([](std::vector<T> &v, size_t pos) { v.erase(v.begin() + pos); }), luabind::ref_policy<1> {});
	classDefVec.def("At", static_cast<T (*)(const std::vector<T> &, size_t)>([](const std::vector<T> &v, size_t pos) -> T { return v.at(pos); }), luabind::const_ref_policy<1> {});
	classDefVec.def("Set", static_cast<void (*)(std::vector<T> &, size_t, const T &)>([](std::vector<T> &v, size_t pos, const T &value) { v.at(pos) = value; }), luabind::ref_policy<1> {});
	classDefVec.def("Back", static_cast<T (*)(const std::vector<T> &)>([](const std::vector<T> &v) { return v.back(); }), luabind::const_ref_policy<1> {});
	classDefVec.def("Front", static_cast<T (*)(const std::vector<T> &)>([](const std::vector<T> &v) { return v.front(); }), luabind::const_ref_policy<1> {});
	classDefVec.def("Size", static_cast<size_t (*)(const std::vector<T> &)>([](const std::vector<T> &v) { return v.size(); }), luabind::const_ref_policy<1> {});
	classDefVec.def("Capacity", static_cast<size_t (*)(const std::vector<T> &)>([](const std::vector<T> &v) { return v.capacity(); }), luabind::const_ref_policy<1> {});
	classDefVec.def("Resize", static_cast<void (*)(std::vector<T> &, size_t)>([](std::vector<T> &v, size_t size) { v.resize(size); }), luabind::ref_policy<1> {});
	classDefVec.def("Resize", static_cast<void (*)(std::vector<T> &, size_t, const T &)>([](std::vector<T> &v, size_t size, const T &value) { v.resize(size, value); }), luabind::ref_policy<1> {});
	classDefVec.def("Reserve", static_cast<void (*)(std::vector<T> &, size_t)>([](std::vector<T> &v, size_t size) { v.reserve(size); }), luabind::ref_policy<1> {});
	classDefVec.def("ShrinkToFit", static_cast<void (*)(std::vector<T> &, size_t)>([](std::vector<T> &v, size_t size) { v.shrink_to_fit(); }), luabind::ref_policy<1> {});
	classDefVec.def("Clear", static_cast<void (*)(std::vector<T> &)>([](std::vector<T> &v) { v.clear(); }), luabind::ref_policy<1> {});
	classDefVec.def("ToTable", static_cast<std::vector<T> (*)(const std::vector<T> &)>([](const std::vector<T> &v) { return v; }), luabind::const_ref_policy<1> {});
	classDefVec.def("SetValues", static_cast<void (*)(std::vector<T> &, std::vector<T> &)>([](std::vector<T> &v, std::vector<T> &newValues) { v = std::move(newValues); }), luabind::ref_policy<1> {});
	auto utilMod = luabind::module(l, "util");
	utilMod[classDefVec];
}

void Lua::util::register_std_vector_types(lua_State *l)
{
	register_vector<char>(l, "CharVector");
	register_vector<bool>(l, "BoolVector");
	register_vector<int>(l, "IntVector");
	register_vector<short>(l, "ShortVector");
	register_vector<float>(l, "FloatVector");
	register_vector<double>(l, "DoubleVector");
	register_vector<long long>(l, "LongLongVector");
	register_vector<long double>(l, "LongDoubleVector");
	register_vector<Vector3>(l, "Vector3Vector");
	register_vector<Vector2>(l, "Vector2Vector");
	register_vector<Vector4>(l, "Vector4Vector");
	register_vector<EulerAngles>(l, "EulerAnglesVector");
	register_vector<Quat>(l, "QuaternionVector");
	register_vector<Mat2>(l, "Mat2Vector");
	register_vector<Mat2x3>(l, "Mat2x3Vector");
	register_vector<Mat2x4>(l, "Mat2x4Vector");
	register_vector<Mat3x2>(l, "Mat3x2Vector");
	register_vector<Mat3>(l, "Mat3Vector");
	register_vector<Mat3x4>(l, "Mat3x4Vector");
	register_vector<Mat4x2>(l, "Mat4x2Vector");
	register_vector<Mat4x3>(l, "Mat4x3Vector");
	register_vector<Mat4>(l, "Mat4Vector");
	register_vector<Vector2i>(l, "Vector2iVector");
	register_vector<Vector3i>(l, "Vector3iVector");
	register_vector<Vector4i>(l, "Vector4iVector");
	register_vector<Color>(l, "ColorVector");
	register_vector<std::string>(l, "StringVector");
}
