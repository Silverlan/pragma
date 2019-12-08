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
	WIBase::SetSize(x,y);
	CallLuaMember<void,int,int>("OnSizeChanged",x,y);
}
void WILuaBase::OnVisibilityChanged(bool bVisible)
{
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
	WIBase::SetColor(r,g,b,a);
	CallLuaMember<void,float,float,float,float>("OnColorChanged",r,g,b,a);
}
void WILuaBase::SetAlpha(float alpha)
{
	WIBase::SetAlpha(alpha);
	CallLuaMember<void,float>("OnAlphaChanged",alpha);
}
void WILuaBase::Render(int w,int h,const Mat4 &mat,const Vector2i &origin,const Mat4 &matParent)
{
	WIBase::Render(w,h,mat,origin,matParent);
	CallLuaMember<void,int,int,Mat4>("OnDraw",w,h,mat);
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

void WILuaWrapper::Render(int,int,Mat4) {}
void WILuaWrapper::default_Render(lua_State*,WIHandle&,int,int,Mat4) {}

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
