// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :scripting.lua.libraries.mesh;

int Lua::mesh::generate_convex_hull(lua::State *l)
{
	CheckTable(l, 1);
	int32_t t = 1;
	PushNil(l);
	std::vector<Vector3> verts;
	auto numVerts = GetObjectLength(l, t);
	verts.reserve(numVerts);
	while(GetNextPair(l, t) != 0) {
		auto &v = Lua::Check<Vector3>(l, -1);
		verts.push_back(v);
		Pop(l, 1);
	}

	auto indices = umesh::generate_convex_hull(verts);
	auto tOut = CreateTable(l);
	for(auto i = decltype(indices.size()) {0}; i < indices.size(); ++i) {
		auto idx = indices[i];
		PushInt(l, i + 1);
		PushInt(l, idx);
		SetTableValue(l, tOut);
	}
	return 1;
}

int Lua::mesh::calc_smallest_enclosing_bbox(lua::State *l)
{
	CheckTable(l, 1);
	int32_t t = 1;
	PushNil(l);
	std::vector<Vector3> verts;
	auto numVerts = GetObjectLength(l, t);
	verts.reserve(numVerts);
	while(GetNextPair(l, t) != 0) {
		auto &v = Lua::Check<Vector3>(l, -1);
		verts.push_back(v);
		Pop(l, 1);
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
