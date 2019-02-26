#ifndef __LXML_H__
#define __LXML_H__

struct lua_State;
namespace Lua
{
	class Interface;
	namespace xml
	{
		void register_lua_library(Lua::Interface &l);
		namespace lib
		{
			int parse(lua_State *l);
			int load(lua_State *l);
			int create(lua_State *l);
		};
	}
};

#endif
