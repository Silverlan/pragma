#include "stdafx_client.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/lua/libraries/c_lrender.h"
#include "luasystem.h"
#include "pragma/lua/classes/lmaterial.h"
#include "pragma/lua/classes/c_lcamera.h"
#include "pragma/lua/classes/ldef_color.h"
#include "pragma/lua/classes/ldef_material.h"
extern ClientState *client;
DLLCLIENT int Lua_render_SetMaterialOverride(lua_State *l)
{
	CGame *game = client->GetGameState();
	if(Lua::IsSet(l,1))
	{
		Material *mat = Lua::CheckMaterial(l,1);
		game->SetMaterialOverride(mat);
		return 0;
	}
	game->SetMaterialOverride(NULL);
	return 0;
}

DLLCLIENT int Lua_render_SetColorScale(lua_State *l)
{
	Color *col = Lua::CheckColor(l,1);
	CGame *game = client->GetGameState();
	game->SetColorScale(Vector4(col->r,col->g,col->b,col->a));
	return 0;
}

DLLCLIENT int Lua_render_SetAlphaScale(lua_State *l)
{
	float a = Lua::CheckNumber<float>(l,1);
	CGame *game = client->GetGameState();
	game->SetAlphaScale(a);
	return 0;
}
