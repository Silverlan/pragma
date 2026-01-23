// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :scripting.lua.classes.recipient_filter;

import :entities;
import :entities.components;
import :server_state;

luabind::tableT<pragma::SPlayerComponent> Lua::RecipientFilter::GetRecipients(lua::State *l, pragma::networking::TargetRecipientFilter &rp)
{
	auto &recipients = rp.GetRecipients();
	auto t = luabind::newtable(l);
	uint32_t idx = 1;
	for(unsigned int i = 0; i < recipients.size(); i++) {
		auto &rp = recipients.at(i);
		if(rp.expired())
			continue;
		auto *pl = pragma::ServerState::Get()->GetPlayer(*rp);
		if(!pl)
			continue;
		t[idx++] = pl->GetLuaObject();
	}
	return t;
}

void Lua::RecipientFilter::AddRecipient(lua::State *l, pragma::networking::TargetRecipientFilter &rp, pragma::SPlayerComponent &hPl) { rp.AddRecipient(*hPl.GetClientSession()); }

void Lua::RecipientFilter::RemoveRecipient(lua::State *l, pragma::networking::TargetRecipientFilter &rp, pragma::SPlayerComponent &hPl) { rp.RemoveRecipient(*hPl.GetClientSession()); }

bool Lua::RecipientFilter::HasRecipient(lua::State *l, pragma::networking::TargetRecipientFilter &rp, pragma::SPlayerComponent &hPl) { return rp.HasRecipient(*hPl.GetClientSession()); }
