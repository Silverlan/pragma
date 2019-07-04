#include "stdafx_shared.h"
#include "pragma/lua/libraries/lmesh.h"
#include "pragma/lua/classes/ldef_vector.h"
#include <mathutil/umesh.h>
#include <pragma/lua/luaapi.h>

int Lua::mesh::generate_convex_hull(lua_State *l)
{
	Lua::CheckTable(l,1);
	int32_t t = 1;
	Lua::PushNil(l);
	std::vector<Vector3> verts;
	auto numVerts = Lua::GetObjectLength(l,t);
	verts.reserve(numVerts);
	while(Lua::GetNextPair(l,t) != 0)
	{
		auto *v = Lua::CheckVector(l,-1);
		verts.push_back(*v);
		Lua::Pop(l,1);
	}

	auto indices = umesh::generate_convex_hull(verts);
	auto tOut = Lua::CreateTable(l);
	for(auto i=decltype(indices.size()){0};i<indices.size();++i)
	{
		auto idx = indices[i];
		Lua::PushInt(l,i +1);
		Lua::PushInt(l,idx);
		Lua::SetTableValue(l,tOut);
	}
	return 1;
}

int Lua::mesh::calc_smallest_enclosing_bbox(lua_State *l)
{
	Lua::CheckTable(l,1);
	int32_t t = 1;
	Lua::PushNil(l);
	std::vector<Vector3> verts;
	auto numVerts = Lua::GetObjectLength(l,t);
	verts.reserve(numVerts);
	while(Lua::GetNextPair(l,t) != 0)
	{
		auto *v = Lua::CheckVector(l,-1);
		verts.push_back(*v);
		Lua::Pop(l,1);
	}
	
	Vector3 center = {};
	Vector3 extents = {};
	Quat rot = uquat::identity();
	umesh::calc_smallest_enclosing_bbox(verts,center,extents,rot);
	Lua::Push<Vector3>(l,center);
	Lua::Push<Vector3>(l,extents);
	Lua::Push<Quat>(l,rot);
	return 3;
}
