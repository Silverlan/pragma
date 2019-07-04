#include "stdafx_server.h"
#include "pragma/serverstate/serverutil.h"
#include "pragma/lua/classes/lrecipientfilter.h"
#include "pragma/entities/player.h"
#include "pragma/networking/wvserverclient.h"
#include "pragma/networking/recipient_filter.hpp"
#include "pragma/lua/classes/ldef_recipientfilter.h"
#include "pragma/lua/classes/ldef_entity.h"
#include "pragma/lua/s_lentity_handles.hpp"
#include "pragma/entities/components/s_player_component.hpp"
#include <pragma/lua/lua_entity_component.hpp>

extern ServerState *server;
void Lua::RecipientFilter::GetRecipients(lua_State *l,pragma::networking::TargetRecipientFilter &rp)
{
	auto &recipients = rp.GetRecipients();
	lua_newtable(l);
	int top = lua_gettop(l);
	int n = 1;
	for(unsigned int i=0;i<recipients.size();i++)
	{
		auto &rp = recipients.at(i);
		if(rp.expired())
			continue;
		auto *pl = server->GetPlayer(*rp);
		if(pl != NULL)
		{
			pl->PushLuaObject(l);
			lua_rawseti(l,top,n);
			n++;
		}
	}
}

void Lua::RecipientFilter::AddRecipient(lua_State *l,pragma::networking::TargetRecipientFilter &rp,SPlayerHandle &hPl)
{
	pragma::Lua::check_component(l,hPl);
	rp.AddRecipient(*hPl->GetClientSession());
}

void Lua::RecipientFilter::RemoveRecipient(lua_State *l,pragma::networking::TargetRecipientFilter &rp,SPlayerHandle &hPl)
{
	pragma::Lua::check_component(l,hPl);
	rp.RemoveRecipient(*hPl->GetClientSession());
}

void Lua::RecipientFilter::HasRecipient(lua_State *l,pragma::networking::TargetRecipientFilter &rp,SPlayerHandle &hPl)
{
	pragma::Lua::check_component(l,hPl);
	lua_pushboolean(l,rp.HasRecipient(*hPl->GetClientSession()));
}
