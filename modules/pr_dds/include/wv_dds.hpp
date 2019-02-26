#ifndef __WV_DDS_HPP__
#define __WV_DDS_HPP__

namespace Lua
{
	class Interface;
	namespace dds
	{
		void register_lua_library(Lua::Interface &l);
	};
};

#endif
