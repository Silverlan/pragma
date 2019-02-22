#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/lua/libraries/c_lphysx.h"
#include "luasystem.h"
#include <pragma/physics/physapi.h>
#include <pragma/console/convars.h>
#include "pragma/physics/pxvisualizer.h"
#ifdef PHYS_ENGINE_PHYSX
#include <PxVisualizationParameter.h>
#endif

extern CGame *c_game;
#if PHYS_ENGINE_PHYSX
static void CVAR_CALLBACK_render_debug_physics_enabled(NetworkState *state,ConVar *cvar,bool prev,bool val)
{
	physx::PxScene *scene = c_game->GetPhysXScene();
	scene->setVisualizationParameter(physx::PxVisualizationParameter::eSCALE,(val == true) ? 1.f : 0.f);
	if(val == false)
		PxVisualizer::ClearScene();
}
DLLCLIENT int Lua_physx_SetVisualizationEnabled(lua_State *l)
{
	bool bEnabled = Lua::CheckBool(l,1);
	physx::PxScene *scene = c_game->GetPhysXScene();
	scene->setVisualizationParameter(physx::PxVisualizationParameter::eSCALE,(bEnabled == true) ? 1.f : 0.f);
	return 0;
}

DLLCLIENT int Lua_physx_SetVisualizationParameter(lua_State *l)
{
	unsigned int visParam = Lua::CheckInt(l,1);
	float scale = 1.f;
	if(Lua::IsSet(l,2))
		scale = Lua::CheckNumber(l,2);
	physx::PxScene *scene = c_game->GetPhysXScene();
	scene->setVisualizationParameter(physx::PxVisualizationParameter::Enum(visParam),scale);
	return 0;
}
REGISTER_CONVAR_CALLBACK_CL(render_debug_physics_enabled,CVAR_CALLBACK_render_debug_physics_enabled);
#endif