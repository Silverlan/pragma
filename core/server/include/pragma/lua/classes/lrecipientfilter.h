#ifndef __LRECIPIENTFILTER_H__
#define __LRECIPIENTFILTER_H__
#include "pragma/serverdefinitions.h"
#include "pragma/networkdefinitions.h"
#include <pragma/lua/ldefinitions.h>
#include "pragma/lua/s_lentity_handles.hpp"
namespace nwm {class RecipientFilter;};

namespace Lua
{
	namespace RecipientFilter
	{
		DLLSERVER void GetRecipients(lua_State *l,nwm::RecipientFilter *rp);
		DLLSERVER void AddRecipient(lua_State *l,nwm::RecipientFilter *rp,SPlayerHandle &hPl);
		DLLSERVER void RemoveRecipient(lua_State *l,nwm::RecipientFilter *rp,SPlayerHandle &hPl);
		DLLSERVER void HasRecipient(lua_State *l,nwm::RecipientFilter *rp,SPlayerHandle &hPl);
	};
};

#endif
