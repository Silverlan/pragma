// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_LPOINT_RENDERTARGET_H__
#define __C_LPOINT_RENDERTARGET_H__
#include "pragma/clientdefinitions.h"
#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>
#include <mathutil/glmutil.h>
class PointRenderTargetHandle;
class Material;
DLLCLIENT void Lua_PointRenderTarget_GetTextureBuffer(lua_State *l, PointRenderTargetHandle &hEnt);
DLLCLIENT void Lua_PointRenderTarget_SetRenderSize(lua_State *l, PointRenderTargetHandle &hEnt, Vector2 &renderSize);
DLLCLIENT void Lua_PointRenderTarget_GetRenderSize(lua_State *l, PointRenderTargetHandle &hEnt);
DLLCLIENT void Lua_PointRenderTarget_SetRenderMaterial(lua_State *l, PointRenderTargetHandle &hEnt, Material *mat);
DLLCLIENT void Lua_PointRenderTarget_SetRenderMaterial(lua_State *l, PointRenderTargetHandle &hEnt, std::string mat);
DLLCLIENT void Lua_PointRenderTarget_SetRenderMaterial(lua_State *l, PointRenderTargetHandle &hEnt);
DLLCLIENT void Lua_PointRenderTarget_GetRenderMaterial(lua_State *l, PointRenderTargetHandle &hEnt);
DLLCLIENT void Lua_PointRenderTarget_SetRefreshRate(lua_State *l, PointRenderTargetHandle &hEnt, float rate);
DLLCLIENT void Lua_PointRenderTarget_GetRefreshRate(lua_State *l, PointRenderTargetHandle &hEnt);
DLLCLIENT void Lua_PointRenderTarget_SetRenderDepth(lua_State *l, PointRenderTargetHandle &hEnt, unsigned int depth);
DLLCLIENT void Lua_PointRenderTarget_GetRenderDepth(lua_State *l, PointRenderTargetHandle &hEnt);
DLLCLIENT void Lua_PointRenderTarget_SetRenderFOV(lua_State *l, PointRenderTargetHandle &hEnt, float fov);
DLLCLIENT void Lua_PointRenderTarget_GetRenderFOV(lua_State *l, PointRenderTargetHandle &hEnt);

#endif
