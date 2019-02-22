#include "stdafx_client.h"
#include "pragma/c_engine.h"

extern DLLCENGINE CEngine *c_engine;

REGISTER_CONVAR_CALLBACK_CL(cl_render_present_mode,[](NetworkState *state,ConVar*,int32_t,int32_t val) {
	c_engine->SetPresentMode(static_cast<Anvil::PresentModeKHR>(val));
});
