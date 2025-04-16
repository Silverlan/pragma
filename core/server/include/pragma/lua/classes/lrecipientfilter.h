/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __LRECIPIENTFILTER_H__
#define __LRECIPIENTFILTER_H__
#include "pragma/serverdefinitions.h"
#include "pragma/networkdefinitions.h"
#include <pragma/lua/ldefinitions.h>

namespace pragma::networking {
	class TargetRecipientFilter;
};
namespace Lua {
	namespace RecipientFilter {
		DLLSERVER luabind::tableT<pragma::SPlayerComponent> GetRecipients(lua_State *l, pragma::networking::TargetRecipientFilter &rp);
		DLLSERVER void AddRecipient(lua_State *l, pragma::networking::TargetRecipientFilter &rp, pragma::SPlayerComponent &hPl);
		DLLSERVER void RemoveRecipient(lua_State *l, pragma::networking::TargetRecipientFilter &rp, pragma::SPlayerComponent &hPl);
		DLLSERVER bool HasRecipient(lua_State *l, pragma::networking::TargetRecipientFilter &rp, pragma::SPlayerComponent &hPl);
	};
};

#endif
