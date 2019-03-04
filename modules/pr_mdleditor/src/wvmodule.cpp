#include "stdafx_mdleditor.h"
#include "wvmodule.h"
#include <Windows.h>
#include "lmdleditor.h"
#include <sharedutils/functioncallback.h>
#include <luainterface.hpp>
#include <pragma/pragma_module.hpp>
#ifdef _DEBUG
#include <iostream>
#endif

#pragma comment(lib,"lua51.lib")
#pragma comment(lib,"luasystem.lib")
#pragma comment(lib,"luabind.lib")
#pragma comment(lib,"vfilesystem.lib")
#pragma comment(lib,"sharedutils.lib")
#pragma comment(lib,"mathutil.lib")
#pragma comment(lib,"shared.lib")
#pragma comment(lib,"engine.lib")
#pragma comment(lib,"util_smdmodel.lib")
#pragma comment(lib,"pr_source.lib")
#pragma comment(lib,"materialsystem.lib")

#pragma comment(lib,"niflib_dll.lib")
#pragma comment(lib,"util_archive.lib")

extern "C"
{
	void PRAGMA_EXPORT pragma_initialize_lua(Lua::Interface &l)
	{
		Lua::mde::register_lua_library(l.GetState());
	}
};
/*
#include "fbx.h"
int main(int argc,char *argv[])
{
	//load_fbx("car_01.fbx");
	for(;;);
	return 0;
}

*/