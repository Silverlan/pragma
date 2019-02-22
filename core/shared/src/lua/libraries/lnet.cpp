#include "stdafx_shared.h"
#include "pragma/lua/libraries/lnet.hpp"
#include <networkmanager/interface/nwm_manager.hpp>

extern DLLENGINE Engine *engine;

void Lua::net::RegisterLibraryEnums(lua_State *l)
{
	Lua::RegisterLibraryEnums(l,"net",{
		{"CLIENT_DROPPED_REASON_DISCONNECTED",umath::to_integral(nwm::ClientDropped::Disconnected)},
		{"CLIENT_DROPPED_REASON_TIMEOUT",umath::to_integral(nwm::ClientDropped::Timeout)},
		{"CLIENT_DROPPED_REASON_KICKED",umath::to_integral(nwm::ClientDropped::Kicked)},
		{"CLIENT_DROPPED_REASON_SHUTDOWN",umath::to_integral(nwm::ClientDropped::Shutdown)},
		{"CLIENT_DROPPED_REASON_ERROR",umath::to_integral(nwm::ClientDropped::Error)},

		{"PROTOCOL_TCP",umath::to_integral(nwm::Protocol::TCP)},
		{"PROTOCOL_UDP",umath::to_integral(nwm::Protocol::UDP)}
	});
}

int32_t Lua::net::register_event(lua_State *l)
{
	std::string name = Lua::CheckString(l,1);
	auto *nw = engine->GetNetworkState(l);
	auto *game = nw->GetGameState();
	auto eventId = game->SetupNetEvent(name);
	Lua::PushInt(l,eventId);
	return 1;
}
