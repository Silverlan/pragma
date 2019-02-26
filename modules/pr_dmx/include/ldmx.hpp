#ifndef __LDMX_HPP__
#define __LDMX_HPP__

namespace Lua
{
	class Interface;
	namespace dmx
	{
		void register_lua_library(Lua::Interface &l);
	};
};

#endif
