/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/serverstate/serverutil.h"
#include "pragma/lua/classes/lrecipientfilter.h"
#include "pragma/entities/player.h"
#include "pragma/networking/recipient_filter.hpp"
#include "pragma/lua/classes/ldef_recipientfilter.h"
#include "pragma/lua/classes/ldef_entity.h"
#include "pragma/entities/components/s_player_component.hpp"

extern ServerState *server;
luabind::tableT<pragma::SPlayerComponent> Lua::RecipientFilter::GetRecipients(lua_State *l, pragma::networking::TargetRecipientFilter &rp)
{
	auto &recipients = rp.GetRecipients();
	auto t = luabind::newtable(l);
	uint32_t idx = 1;
	for(unsigned int i = 0; i < recipients.size(); i++) {
		auto &rp = recipients.at(i);
		if(rp.expired())
			continue;
		auto *pl = server->GetPlayer(*rp);
		if(!pl)
			continue;
		t[idx++] = pl->GetLuaObject();
	}
	return t;
}

void Lua::RecipientFilter::AddRecipient(lua_State *l, pragma::networking::TargetRecipientFilter &rp, pragma::SPlayerComponent &hPl) { rp.AddRecipient(*hPl.GetClientSession()); }

void Lua::RecipientFilter::RemoveRecipient(lua_State *l, pragma::networking::TargetRecipientFilter &rp, pragma::SPlayerComponent &hPl) { rp.RemoveRecipient(*hPl.GetClientSession()); }

bool Lua::RecipientFilter::HasRecipient(lua_State *l, pragma::networking::TargetRecipientFilter &rp, pragma::SPlayerComponent &hPl) { return rp.HasRecipient(*hPl.GetClientSession()); }
