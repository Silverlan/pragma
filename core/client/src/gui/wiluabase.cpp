/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/gui/wiluabase.h"
#include "luasystem.h"
#include <wgui/wihandle.h>
#include "pragma/lua/classes/c_ldef_wgui.h"

extern DLLCLIENT CGame *c_game;

extern ClientState *client;
WILuaBase::WILuaBase()
{}

WILuaBase::~WILuaBase()
{}

void WILuaBase::SetupLua(const luabind::object &o,std::string &className)
{
	m_class = className;
	SetLuaObject(o);
	SetUserData2(m_baseLuaObj);
}

void WILuaBase::Initialize()
{
	auto hThis = GetHandle();
	WIBase::Initialize();
	if(hThis.IsValid() == false)
		return;
	CallLuaMember("OnInitialize");
}

void WILuaBase::Think()
{
	auto hThis = GetHandle();
	WIBase::Think();
	if(hThis.IsValid() == false)
		return;
	CallLuaMember("OnThink");
}

void WILuaBase::OnFirstThink()
{
	auto hThis = GetHandle();
	WIBase::OnFirstThink();
	if(hThis.IsValid() == false)
		return;
	CallLuaMember("OnFirstThink");
}

util::EventReply WILuaBase::MouseCallback(GLFW::MouseButton button,GLFW::KeyState state,GLFW::Modifier mods)
{
	if(WIBase::MouseCallback(button,state,mods) == util::EventReply::Handled)
		return util::EventReply::Handled;
	uint32_t reply = umath::to_integral(util::EventReply::Unhandled);
	CallLuaMember<uint32_t,int,int,int>("MouseCallback",&reply,static_cast<int>(button),static_cast<int>(state),static_cast<int>(mods));
	return static_cast<util::EventReply>(reply);
}
util::EventReply WILuaBase::KeyboardCallback(GLFW::Key key,int scanCode,GLFW::KeyState state,GLFW::Modifier mods)
{
	if(WIBase::KeyboardCallback(key,scanCode,state,mods) == util::EventReply::Handled)
		return util::EventReply::Handled;
	uint32_t reply = umath::to_integral(util::EventReply::Unhandled);
	CallLuaMember<uint32_t,int,int,int,int>("KeyboardCallback",&reply,static_cast<int>(key),scanCode,static_cast<int>(state),static_cast<int>(mods));
	return static_cast<util::EventReply>(reply);
}
util::EventReply WILuaBase::CharCallback(unsigned int c,GLFW::Modifier mods)
{
	if(WIBase::CharCallback(c) == util::EventReply::Handled)
		return util::EventReply::Handled;
	uint32_t reply = umath::to_integral(util::EventReply::Unhandled);
	CallLuaMember<uint32_t,unsigned int,uint32_t>("CharCallback",&reply,c,umath::to_integral(mods));
	return static_cast<util::EventReply>(reply);
}
util::EventReply WILuaBase::ScrollCallback(Vector2 offset)
{
	if(WIBase::ScrollCallback(offset) == util::EventReply::Handled)
		return util::EventReply::Handled;
	uint32_t reply = umath::to_integral(util::EventReply::Unhandled);
	CallLuaMember<uint32_t,double,double>("ScrollCallback",&reply,offset.x,offset.y);
	return static_cast<util::EventReply>(reply);
}

void WILuaBase::SetSize(int x,int y)
{
	if(x == GetWidth() && y == GetHeight())
		return;
	WIBase::SetSize(x,y);
	CallLuaMember<void,int,int>("OnSizeChanged",x,y);
}
void WILuaBase::OnVisibilityChanged(bool bVisible)
{
	if(bVisible == *GetVisibilityProperty())
		return;
	WIBase::OnVisibilityChanged(bVisible);
	CallLuaMember<void,bool>("OnVisibilityChanged",bVisible);
}
void WILuaBase::DoUpdate()
{
	WIBase::DoUpdate();
	CallLuaMember<void>("OnUpdate");
}
void WILuaBase::SetColor(float r,float g,float b,float a)
{
	// TODO: Check against current values?
	WIBase::SetColor(r,g,b,a);
	CallLuaMember<void,float,float,float,float>("OnColorChanged",r,g,b,a);
}
void WILuaBase::SetAlpha(float alpha)
{
	if(alpha == GetAlpha())
		return;
	WIBase::SetAlpha(alpha);
	CallLuaMember<void,float>("OnAlphaChanged",alpha);
}
void WILuaBase::Render(const DrawInfo &drawInfo,const Mat4 &matDraw,const Vector2 &scale,uint32_t testStencilLevel,StencilPipeline stencilPipeline)
{
	WIBase::Render(drawInfo,matDraw,scale,testStencilLevel,stencilPipeline);
	// No longer supported, since UI rendering is now multi-threaded
	//CallLuaMember<void,std::reference_wrapper<const DrawInfo>,Mat4,Vector2>("OnDraw",std::ref(drawInfo),matDraw,scale);
}
void WILuaBase::OnCursorEntered()
{
	WIBase::OnCursorEntered();
	CallLuaMember("OnCursorEntered");
}
void WILuaBase::OnCursorExited()
{
	WIBase::OnCursorExited();
	CallLuaMember("OnCursorExited");
}
void WILuaBase::OnFocusGained()
{
	WIBase::OnFocusGained();
	CallLuaMember("OnFocusGained");
}
void WILuaBase::OnFocusKilled()
{
	WIBase::OnFocusKilled();
	CallLuaMember("OnFocusKilled");
}
void WILuaBase::OnRemove()
{
	WIBase::OnRemove();
	CallLuaMember("OnRemove");
}

///////////////////////////////////////////

void WILuaBase::Lua_OnInitialize() {}
void WILuaBase::default_OnInitialize(lua_State*,WILuaBase&) {}

void WILuaBase::Lua_OnThink() {}
void WILuaBase::default_OnThink(lua_State*,WILuaBase&) {}

void WILuaBase::Lua_OnFirstThink() {}
void WILuaBase::default_OnFirstThink(lua_State*,WILuaBase&) {}

void WILuaBase::Lua_MouseCallback(int,int,int) {}
void WILuaBase::default_MouseCallback(lua_State*,WILuaBase&,int,int,int) {}

void WILuaBase::Lua_KeyboardCallback(int,int,int,int) {}
void WILuaBase::default_KeyboardCallback(lua_State*,WILuaBase&,int,int,int,int) {}

void WILuaBase::Lua_CharCallback(unsigned int,uint32_t) {}
void WILuaBase::default_CharCallback(lua_State*,WILuaBase&,unsigned int,uint32_t) {}

void WILuaBase::Lua_ScrollCallback(double,double) {}
void WILuaBase::default_ScrollCallback(lua_State*,WILuaBase&,double,double) {}

void WILuaBase::Lua_OnUpdate() {}
void WILuaBase::default_OnUpdate(lua_State *l,WILuaBase &hElement) {}

void WILuaBase::Lua_OnSetSize(int,int) {}
void WILuaBase::default_OnSetSize(lua_State*,WILuaBase&,int,int) {}

void WILuaBase::Lua_OnSetVisible(bool) {}
void WILuaBase::default_OnSetVisible(lua_State*,WILuaBase&,bool) {}

void WILuaBase::Lua_OnSetColor(float,float,float,float) {}
void WILuaBase::default_OnSetColor(lua_State*,WILuaBase&,float,float,float,float) {}

void WILuaBase::Lua_OnSetAlpha(float) {}
void WILuaBase::default_OnSetAlpha(lua_State*,WILuaBase&,float) {}

void WILuaBase::Lua_Render(const ::WIBase::DrawInfo &drawInfo,const Mat4 &matDraw,const Vector2 &scale) {}
void WILuaBase::default_Render(lua_State*,WILuaBase&,const ::WIBase::DrawInfo &drawInfo,const Mat4 &matDraw,const Vector2 &scale) {}

void WILuaBase::Lua_OnCursorEntered() {}
void WILuaBase::default_OnCursorEntered(lua_State*,WILuaBase&) {}

void WILuaBase::Lua_OnCursorExited() {}
void WILuaBase::default_OnCursorExited(lua_State*,WILuaBase&) {}

void WILuaBase::Lua_OnFocusGained() {}
void WILuaBase::default_OnFocusGained(lua_State*,WILuaBase&) {}

void WILuaBase::Lua_OnFocusKilled() {}
void WILuaBase::default_OnFocusKilled(lua_State*,WILuaBase&) {}

void WILuaBase::Lua_OnRemove() {}
void WILuaBase::default_OnRemove(lua_State*,WILuaBase&) {}
