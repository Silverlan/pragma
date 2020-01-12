#include "stdafx_pcl.h"
#include "lpcl.h"
#include "wvmodule.h"
#include <pragma/lua/luaapi.h>
#include <pragma/lua/ldefinitions.h>
#include <util_pcl.h>
#include <mathutil/uvec.h>

#pragma comment(lib,"util_pcl.lib")

static std::vector<Vector3> get_point_cloud(lua_State *l)
{
	Lua::CheckTable(l,1);
	std::vector<Vector3> pc;
	auto n = Lua::GetObjectLength(l,1);
	pc.reserve(n);

	int32_t t = 1;
	Lua::PushNil(l);
	while(Lua::GetNextPair(l,t) != 0)
	{
		auto &v = Lua::Check<Vector3>(l,-1);
		pc.push_back(v);
		Lua::Pop(l,1);
	}
	return pc;
}

int Lua::pcl::lib::generate_poly_mesh(lua_State *l)
{
	auto pc = get_point_cloud(l);
	std::vector<Vector3> verts;
	std::vector<uint32_t> triangles;
	util::pcl::build_convex_mesh(pc,verts,triangles);

	auto t = Lua::CreateTable(l);
	for(auto i=decltype(verts.size()){0};i<verts.size();++i)
	{
		Lua::PushInt(l,i +1);
		Lua::Push<Vector3>(l,verts[i]);
		Lua::SetTableValue(l,t);
	}

	t = Lua::CreateTable(l);
	for(auto i=decltype(triangles.size()){0};i<triangles.size();++i)
	{
		Lua::PushInt(l,i +1);
		Lua::PushInt(l,triangles[i]);
		Lua::SetTableValue(l,t);
	}
	return 2;
}

void Lua::pcl::register_lua_library(lua_State *lua)
{
	Lua::RegisterLibrary(lua,"pcl",{
		{"generate_poly_mesh",Lua::pcl::lib::generate_poly_mesh}
	});
}