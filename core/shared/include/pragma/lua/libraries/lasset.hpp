#ifndef __LASSET_HPP__
#define __LASSET_HPP__

#include "pragma/networkdefinitions.h"

namespace Lua
{
	class Interface;
	namespace asset
	{
		DLLNETWORK void register_library(Lua::Interface &lua,bool extended);

		DLLNETWORK int32_t exists(lua_State *l);
		DLLNETWORK int32_t find_file(lua_State *l);
		DLLNETWORK int32_t is_loaded(lua_State *l);
	};
};

#endif
