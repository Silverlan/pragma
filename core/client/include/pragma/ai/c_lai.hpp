#ifndef __C_LAI_HPP__
#define __C_LAI_HPP__

#include "pragma/clientdefinitions.h"
#include <pragma/lua/ldefinitions.h>

namespace Lua
{
	namespace ai
	{
		namespace client
		{
			DLLCLIENT void register_library(Lua::Interface &lua);
		};
	};
};

#endif
