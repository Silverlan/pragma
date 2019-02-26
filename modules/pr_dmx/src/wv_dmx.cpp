#include "ldmx.hpp"
#include <luainterface.hpp>
#include <pragma/pragma_module.hpp>

#pragma comment(lib,"lua51.lib")
#pragma comment(lib,"luasystem.lib")
#pragma comment(lib,"luabind.lib")
#pragma comment(lib,"sharedutils.lib")
#pragma comment(lib,"mathutil.lib")
#pragma comment(lib,"vfilesystem.lib")
#pragma comment(lib,"util_dmx.lib")
#pragma comment(lib,"shared.lib")

extern "C"
{
	void PRAGMA_EXPORT pragma_initialize_lua(Lua::Interface &l)
	{
		Lua::dmx::register_lua_library(l);
	}
};
