/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*
* Copyright (c) 2021 Silverlan
*/

#include "stdafx_shared.h"
#include "pragma/lua/libraries/lutil.hpp"

template<typename T>
	static void register_vector(lua_State *l,const char *name)
{
	auto classDefVec = luabind::class_<std::vector<T>>(name);
	classDefVec.def(luabind::constructor<>());
	classDefVec.def("__len",static_cast<void(*)(lua_State*,const std::vector<T>&)>([](lua_State *l,const std::vector<T> &v) {
		Lua::PushInt(l,v.size());
	}));
	classDefVec.def("PushBack",static_cast<void(*)(lua_State*,const std::vector<T>&,const T&)>([](lua_State *l,const std::vector<T> &v,const T &value) {
		const_cast<std::vector<T>&>(v).push_back(value);
	}));
	classDefVec.def("Insert",static_cast<void(*)(lua_State*,const std::vector<T>&,size_t,const T&)>([](lua_State *l,const std::vector<T> &v,size_t pos,const T &value) {
		const_cast<std::vector<T>&>(v).insert(v.begin() +pos,value);
	}));
	classDefVec.def("Erase",static_cast<void(*)(lua_State*,const std::vector<T>&,size_t)>([](lua_State *l,const std::vector<T> &v,size_t pos) {
		const_cast<std::vector<T>&>(v).erase(v.begin() +pos);
	}));
	classDefVec.def("At",static_cast<void(*)(lua_State*,const std::vector<T>&,size_t)>([](lua_State *l,const std::vector<T> &v,size_t pos) {
		auto &value = const_cast<std::vector<T>&>(v).at(pos);
		Lua::Push<T>(l,value);
	}));
	classDefVec.def("Set",static_cast<void(*)(lua_State*,const std::vector<T>&,size_t,const T&)>([](lua_State *l,const std::vector<T> &v,size_t pos,const T &value) {
		const_cast<std::vector<T>&>(v).at(pos) = value;
	}));
	classDefVec.def("Back",static_cast<void(*)(lua_State*,const std::vector<T>&)>([](lua_State *l,const std::vector<T> &v) {
		Lua::Push<T>(l,v.back());
	}));
	classDefVec.def("Front",static_cast<void(*)(lua_State*,const std::vector<T>&)>([](lua_State *l,const std::vector<T> &v) {
		Lua::Push<T>(l,v.front());
	}));
	classDefVec.def("Size",static_cast<void(*)(lua_State*,const std::vector<T>&)>([](lua_State *l,const std::vector<T> &v) {
		Lua::PushInt(l,v.size());
	}));
	classDefVec.def("Capacity",static_cast<void(*)(lua_State*,const std::vector<T>&)>([](lua_State *l,const std::vector<T> &v) {
		Lua::PushInt(l,v.capacity());
	}));
	classDefVec.def("Resize",static_cast<void(*)(lua_State*,const std::vector<T>&,size_t)>([](lua_State *l,const std::vector<T> &v,size_t size) {
		const_cast<std::vector<T>&>(v).resize(size);
	}));
	classDefVec.def("Resize",static_cast<void(*)(lua_State*,const std::vector<T>&,size_t,const T&)>([](lua_State *l,const std::vector<T> &v,size_t size,const T &value) {
		const_cast<std::vector<T>&>(v).resize(size,value);
	}));
	classDefVec.def("Reserve",static_cast<void(*)(lua_State*,const std::vector<T>&,size_t)>([](lua_State *l,const std::vector<T> &v,size_t size) {
		const_cast<std::vector<T>&>(v).reserve(size);
	}));
	classDefVec.def("ShrinkToFit",static_cast<void(*)(lua_State*,const std::vector<T>&,size_t)>([](lua_State *l,const std::vector<T> &v,size_t size) {
		const_cast<std::vector<T>&>(v).shrink_to_fit();
	}));
	classDefVec.def("Clear",static_cast<void(*)(lua_State*,const std::vector<T>&)>([](lua_State *l,const std::vector<T> &v) {
		const_cast<std::vector<T>&>(v).clear();
	}));
	auto utilMod = luabind::module(l,"util");
	utilMod[classDefVec];
}

void Lua::util::register_std_vector_types(lua_State *l)
{
	register_vector<char>(l,"CharVector");
	register_vector<bool>(l,"BoolVector");
	register_vector<int>(l,"IntVector");
	register_vector<short>(l,"ShortVector");
	register_vector<float>(l,"FloatVector");
	register_vector<double>(l,"DoubleVector");
	register_vector<long long>(l,"LongLongVector");
	register_vector<long double>(l,"LongDoubleVector");
	register_vector<Vector3>(l,"Vector3Vector");
	register_vector<Vector2>(l,"Vector2Vector");
	register_vector<Vector4>(l,"Vector4Vector");
	register_vector<EulerAngles>(l,"EulerAnglesVector");
	register_vector<Quat>(l,"QuaternionVector");
	register_vector<Mat2>(l,"Mat2Vector");
	register_vector<Mat2x3>(l,"Mat2x3Vector");
	register_vector<Mat2x4>(l,"Mat2x4Vector");
	register_vector<Mat3x2>(l,"Mat3x2Vector");
	register_vector<Mat3>(l,"Mat3Vector");
	register_vector<Mat3x4>(l,"Mat3x4Vector");
	register_vector<Mat4x2>(l,"Mat4x2Vector");
	register_vector<Mat4x3>(l,"Mat4x3Vector");
	register_vector<Mat4>(l,"Mat4Vector");
	register_vector<Vector2i>(l,"Vector2iVector");
	register_vector<Vector3i>(l,"Vector3iVector");
	register_vector<Vector4i>(l,"Vector4iVector");
	register_vector<Color>(l,"ColorVector");
	register_vector<std::string>(l,"StringVector");
}
