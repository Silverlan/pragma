#include "wvmodule.h"
#include <Windows.h>
#include "lxml.h"
#include "sharedutils/functioncallback.h"
#include <pragma/pragma_module.hpp>
#ifdef _DEBUG
#include <iostream>
#endif

#pragma comment(lib,"lua51.lib")
#pragma comment(lib,"luasystem.lib")
#pragma comment(lib,"luasystem.lib")
#pragma comment(lib,"luabind.lib")
#pragma comment(lib,"sharedutils.lib")
#pragma comment(lib,"mathutil.lib")
#pragma comment(lib,"vfilesystem.lib")
#pragma comment(lib,"ishared.lib")

extern "C"
{
	void PRAGMA_EXPORT pragma_initialize_lua(Lua::Interface &l)
	{
		Lua::xml::register_lua_library(l);
	}
};
