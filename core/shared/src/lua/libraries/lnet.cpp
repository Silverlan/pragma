// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_shared.h"
#include "pragma/engine.h"
#include "pragma/lua/libraries/lnet.hpp"
#include "pragma/networking/enums.hpp"
#include <networkmanager/interface/nwm_manager.hpp>

extern DLLNETWORK Engine *engine;

void Lua::net::RegisterLibraryEnums(lua_State *l)
{
	Lua::RegisterLibraryEnums(l, "net",
	  {{"DROP_REASON_DISCONNECTED", umath::to_integral(pragma::networking::DropReason::Disconnected)}, {"DROP_REASON_TIMEOUT", umath::to_integral(pragma::networking::DropReason::Timeout)}, {"DROP_REASON_KICKED", umath::to_integral(pragma::networking::DropReason::Kicked)},
	    {"DROP_REASON_SHUTDOWN", umath::to_integral(pragma::networking::DropReason::Shutdown)}, {"DROP_REASON_ERROR", umath::to_integral(pragma::networking::DropReason::Error)},

	    {"PROTOCOL_FAST_UNRELIABLE", umath::to_integral(pragma::networking::Protocol::FastUnreliable)}, {"PROTOCOL_SLOW_RELIABLE", umath::to_integral(pragma::networking::Protocol::SlowReliable)}});
}

pragma::NetEventId Lua::net::register_event(lua_State *l, const std::string &name)
{
	auto *nw = engine->GetNetworkState(l);
	auto *game = nw->GetGameState();
	return game->SetupNetEvent(name);
}
