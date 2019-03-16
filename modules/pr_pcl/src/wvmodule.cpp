#include "stdafx_pcl.h"
#include "wvmodule.h"
#include <Windows.h>
#include "lpcl.h"
#include <sharedutils/functioncallback.h>
#include <util_pcl.h>
#include <luainterface.hpp>
#include <pragma/pragma_module.hpp>
#ifdef _DEBUG
#include <iostream>
#endif

#pragma comment(lib,"lua51.lib")
#pragma comment(lib,"luasystem.lib")
#pragma comment(lib,"luabind.lib")
#pragma comment(lib,"sharedutils.lib")
#pragma comment(lib,"mathutil.lib")

extern "C"
{
	void PRAGMA_EXPORT pragma_initialize_lua(Lua::Interface &l)
	{
		Lua::pcl::register_lua_library(l.GetState());
	}

	void PRAGMA_EXPORT pcl_build_convex_mesh(const std::vector<Vector3> &pointCloud,std::vector<Vector3> &verts,std::vector<uint32_t> &indices)
	{
		util::pcl::build_convex_mesh(pointCloud,verts,indices);
	}
};
