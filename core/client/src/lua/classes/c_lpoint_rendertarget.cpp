/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/lua/classes/c_lpoint_rendertarget.h"
#include "luasystem.h"
#include <pragma/lua/ldefinitions.h>
#include "pragma/entities/point/c_point_rendertarget.h"
#include "pragma/lua/classes/ldef_entity.h"

DLLCLIENT void Lua_PointRenderTarget_GetTextureBuffer(lua_State *l,PointRenderTargetHandle &hEnt)
{
	// COMPONENT TODO
	//LUA_CHECK_ENTITY(l,hEnt);
	//CPointRenderTarget *rt = hEnt.get<CPointRenderTarget>();
	//unsigned int bufTexture = rt->GetTextureBuffer();
	//Lua::PushInt(l,bufTexture);
}
DLLCLIENT void Lua_PointRenderTarget_SetRenderSize(lua_State *l,PointRenderTargetHandle &hEnt,Vector2 &renderSize)
{
	// COMPONENT TODO
	//LUA_CHECK_ENTITY(l,hEnt);
	//CPointRenderTarget *rt = hEnt.get<CPointRenderTarget>();
	//rt->SetRenderSize(renderSize);
}
DLLCLIENT void Lua_PointRenderTarget_GetRenderSize(lua_State *l,PointRenderTargetHandle &hEnt)
{
	// COMPONENT TODO
	//LUA_CHECK_ENTITY(l,hEnt);
	//CPointRenderTarget *rt = hEnt.get<CPointRenderTarget>();
	//auto sz = rt->GetRenderSize();
	//luabind::object(l,sz).push(l);
}
DLLCLIENT void Lua_PointRenderTarget_SetRenderMaterial(lua_State *l,PointRenderTargetHandle &hEnt,Material *mat)
{
	// COMPONENT TODO
	//LUA_CHECK_ENTITY(l,hEnt);
	//CPointRenderTarget *rt = hEnt.get<CPointRenderTarget>();
	//rt->SetRenderMaterial(mat);
}
DLLCLIENT void Lua_PointRenderTarget_SetRenderMaterial(lua_State *l,PointRenderTargetHandle &hEnt,std::string mat)
{
	// COMPONENT TODO
	//LUA_CHECK_ENTITY(l,hEnt);
	//CPointRenderTarget *rt = hEnt.get<CPointRenderTarget>();
	//rt->SetRenderMaterial(mat);
}
DLLCLIENT void Lua_PointRenderTarget_SetRenderMaterial(lua_State *l,PointRenderTargetHandle &hEnt)
{
	// COMPONENT TODO
	//LUA_CHECK_ENTITY(l,hEnt);
	//CPointRenderTarget *rt = hEnt.get<CPointRenderTarget>();
	//rt->SetRenderMaterial(NULL);
}
DLLCLIENT void Lua_PointRenderTarget_GetRenderMaterial(lua_State *l,PointRenderTargetHandle &hEnt)
{
	// COMPONENT TODO
	//LUA_CHECK_ENTITY(l,hEnt);
	//CPointRenderTarget *rt = hEnt.get<CPointRenderTarget>();
	//Material *mat = rt->GetRenderMaterial();
	//luabind::object(l,mat).push(l);
}
DLLCLIENT void Lua_PointRenderTarget_SetRefreshRate(lua_State *l,PointRenderTargetHandle &hEnt,float rate)
{
	// COMPONENT TODO
	//LUA_CHECK_ENTITY(l,hEnt);
	//CPointRenderTarget *rt = hEnt.get<CPointRenderTarget>();
	//rt->SetRefreshRate(rate);
}
DLLCLIENT void Lua_PointRenderTarget_GetRefreshRate(lua_State *l,PointRenderTargetHandle &hEnt)
{
	// COMPONENT TODO
	//LUA_CHECK_ENTITY(l,hEnt);
	//CPointRenderTarget *rt = hEnt.get<CPointRenderTarget>();
	//Lua::PushNumber(l,rt->GetRefreshRate());
}
DLLCLIENT void Lua_PointRenderTarget_SetRenderDepth(lua_State *l,PointRenderTargetHandle &hEnt,unsigned int depth)
{
	// COMPONENT TODO
	//LUA_CHECK_ENTITY(l,hEnt);
	//CPointRenderTarget *rt = hEnt.get<CPointRenderTarget>();
	//rt->SetRenderDepth(depth);
}
DLLCLIENT void Lua_PointRenderTarget_GetRenderDepth(lua_State *l,PointRenderTargetHandle &hEnt)
{
	// COMPONENT TODO
	//LUA_CHECK_ENTITY(l,hEnt);
	//CPointRenderTarget *rt = hEnt.get<CPointRenderTarget>();
	//Lua::PushInt(l,rt->GetRenderDepth());
}
DLLCLIENT void Lua_PointRenderTarget_SetRenderFOV(lua_State *l,PointRenderTargetHandle &hEnt,float fov)
{
	// COMPONENT TODO
	//LUA_CHECK_ENTITY(l,hEnt);
	//hEnt.get<CPointRenderTarget>()->SetRenderFOV(fov);
}
DLLCLIENT void Lua_PointRenderTarget_GetRenderFOV(lua_State *l,PointRenderTargetHandle &hEnt)
{
	// COMPONENT TODO
	//LUA_CHECK_ENTITY(l,hEnt);
	//float fov = hEnt.get<CPointRenderTarget>()->GetRenderFOV();
	//Lua::PushNumber(l,fov);
}
