/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __LRECIPIENTFILTER_H__
#define __LRECIPIENTFILTER_H__
#include "pragma/serverdefinitions.h"
#include "pragma/networkdefinitions.h"
#include <pragma/lua/ldefinitions.h>
#include "pragma/lua/s_lentity_handles.hpp"

namespace pragma::networking {class TargetRecipientFilter;};
namespace Lua
{
	namespace RecipientFilter
	{
		DLLSERVER void GetRecipients(lua_State *l,pragma::networking::TargetRecipientFilter &rp);
		DLLSERVER void AddRecipient(lua_State *l,pragma::networking::TargetRecipientFilter &rp,SPlayerHandle &hPl);
		DLLSERVER void RemoveRecipient(lua_State *l,pragma::networking::TargetRecipientFilter &rp,SPlayerHandle &hPl);
		DLLSERVER void HasRecipient(lua_State *l,pragma::networking::TargetRecipientFilter &rp,SPlayerHandle &hPl);
	};
};

#endif
