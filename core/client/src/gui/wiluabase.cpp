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
DEFINE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WI,WILuaBase,WILua);
void WILuaHandle::Reset(PtrWI *e)
{
	m_basePointer.reset(e);
	m_bEmpty = false;
}

extern ClientState *client;
WILuaBase::WILuaBase(luabind::object &o,std::string &className)
	: WIBase(),LuaObjectBase(o)
{
	m_class = className;
	SetUserData2(m_baseLuaObj);
}

WILuaBase::~WILuaBase()
{}

void WILuaBase::InitializeHandle()
{
	auto &hElement = *luabind::object_cast_nothrow<WILuaHandle*>(*m_baseLuaObj,static_cast<WILuaHandle*>(nullptr));
	hElement.Reset(new PtrWI(this));
	m_handle = std::shared_ptr<WILuaHandle>(&hElement,[](WILuaHandle*) {}); // Empty deleter, Lua will take care of its deletion!
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
void WILuaBase::Render(const DrawInfo &drawInfo,const Mat4 &matDraw,const Vector2 &scale)
{
	WIBase::Render(drawInfo,matDraw);
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

void WILuaWrapper::OnInitialize() {}
void WILuaWrapper::default_OnInitialize(lua_State*,WIHandle&) {}

void WILuaWrapper::OnThink() {}
void WILuaWrapper::default_OnThink(lua_State*,WIHandle&) {}

void WILuaWrapper::OnFirstThink() {}
void WILuaWrapper::default_OnFirstThink(lua_State*,WIHandle&) {}

void WILuaWrapper::MouseCallback(int,int,int) {}
void WILuaWrapper::default_MouseCallback(lua_State*,WIHandle&,int,int,int) {}

void WILuaWrapper::KeyboardCallback(int,int,int,int) {}
void WILuaWrapper::default_KeyboardCallback(lua_State*,WIHandle&,int,int,int,int) {}

void WILuaWrapper::CharCallback(unsigned int,uint32_t) {}
void WILuaWrapper::default_CharCallback(lua_State*,WIHandle&,unsigned int,uint32_t) {}

void WILuaWrapper::ScrollCallback(double,double) {}
void WILuaWrapper::default_ScrollCallback(lua_State*,WIHandle&,double,double) {}

void WILuaWrapper::OnUpdate() {}
void WILuaWrapper::default_OnUpdate(lua_State *l,WIHandle &hElement) {}

void WILuaWrapper::OnSetSize(int,int) {}
void WILuaWrapper::default_OnSetSize(lua_State*,WIHandle&,int,int) {}

void WILuaWrapper::OnSetVisible(bool) {}
void WILuaWrapper::default_OnSetVisible(lua_State*,WIHandle&,bool) {}

void WILuaWrapper::OnSetColor(float,float,float,float) {}
void WILuaWrapper::default_OnSetColor(lua_State*,WIHandle&,float,float,float,float) {}

void WILuaWrapper::OnSetAlpha(float) {}
void WILuaWrapper::default_OnSetAlpha(lua_State*,WIHandle&,float) {}

void WILuaWrapper::Render(const ::WIBase::DrawInfo &drawInfo,const Mat4 &matDraw,const Vector2 &scale) {}
void WILuaWrapper::default_Render(lua_State*,WIHandle&,const ::WIBase::DrawInfo &drawInfo,const Mat4 &matDraw,const Vector2 &scale) {}

void WILuaWrapper::OnCursorEntered() {}
void WILuaWrapper::default_OnCursorEntered(lua_State*,WIHandle&) {}

void WILuaWrapper::OnCursorExited() {}
void WILuaWrapper::default_OnCursorExited(lua_State*,WIHandle&) {}

void WILuaWrapper::OnFocusGained() {}
void WILuaWrapper::default_OnFocusGained(lua_State*,WIHandle&) {}

void WILuaWrapper::OnFocusKilled() {}
void WILuaWrapper::default_OnFocusKilled(lua_State*,WIHandle&) {}

void WILuaWrapper::OnRemove() {}
void WILuaWrapper::default_OnRemove(lua_State*,WIHandle&) {}
