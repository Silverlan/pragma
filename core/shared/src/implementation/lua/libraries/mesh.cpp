// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include <ostream>

#include "pragma/lua/core.hpp"

#include <string>
#include <vector>

module pragma.shared;

import :scripting.lua.libraries.mesh;

int Lua::mesh::generate_convex_hull(lua_State *l)
{
	Lua::CheckTable(l, 1);
	int32_t t = 1;
	Lua::PushNil(l);
	std::vector<Vector3> verts;
	auto numVerts = Lua::GetObjectLength(l, t);
	verts.reserve(numVerts);
	while(Lua::GetNextPair(l, t) != 0) {
		auto &v = Lua::Check<Vector3>(l, -1);
		verts.push_back(v);
		Lua::Pop(l, 1);
	}

	auto indices = umesh::generate_convex_hull(verts);
	auto tOut = Lua::CreateTable(l);
	for(auto i = decltype(indices.size()) {0}; i < indices.size(); ++i) {
		auto idx = indices[i];
		Lua::PushInt(l, i + 1);
		Lua::PushInt(l, idx);
		Lua::SetTableValue(l, tOut);
	}
	return 1;
}

int Lua::mesh::calc_smallest_enclosing_bbox(lua_State *l)
{
	Lua::CheckTable(l, 1);
	int32_t t = 1;
	Lua::PushNil(l);
	std::vector<Vector3> verts;
	auto numVerts = Lua::GetObjectLength(l, t);
	verts.reserve(numVerts);
	while(Lua::GetNextPair(l, t) != 0) {
		auto &v = Lua::Check<Vector3>(l, -1);
		verts.push_back(v);
		Lua::Pop(l, 1);
	}

	Vector3 center = {};
	Vector3 extents = {};
	Quat rot = uquat::identity();
	umesh::calc_smallest_enclosing_bbox(verts, center, extents, rot);
	Lua::Push<Vector3>(l, center);
	Lua::Push<Vector3>(l, extents);
	Lua::Push<Quat>(l, rot);
	return 3;
}
