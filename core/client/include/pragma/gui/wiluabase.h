/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __WILUABASE_H__
#define __WILUABASE_H__
#include "pragma/clientdefinitions.h"
#include <wgui/wibase.h>
#include <pragma/lua/luaapi.h>
#include "pragma/gui/wgui_luainterface.h"
#include "pragma/lua/luaobjectbase.h"
#include <pragma/lua/lua_call.hpp>

#define LUAGUI_CALL_MEMBER(name,numargs,numret,args,ret) \
	{ \
		luabind::object *obj = WGUILuaInterface::GetLuaObject(m_stateLua,this); \
		obj->push(m_stateLua); \
		Lua::PushString(m_stateLua,name); \
		Lua::GetTableValue(m_stateLua,-2); \
		if(Lua::IsSet(m_stateLua,-1) && Lua::IsFunction(m_stateLua,-1)) \
		{ \
			int cargs = numargs +1; \
			obj->push(m_stateLua); \
			args; \
			int s = Lua::CallFunction(m_stateLua,cargs,numret,0); \
			if(s == 0) \
			{ \
				ret; \
			} \
			Error(s); \
		} \
		else \
			Lua::Pop(m_stateLua,1); \
		Lua::Pop(m_stateLua,1); \
	}

struct lua_State;
struct WILuaWrapper;
class DLLCLIENT WILuaBase
	: public WIBase,public LuaObjectBase
{
public:
	WILuaBase(luabind::object &o,std::string &className);
	virtual ~WILuaBase() override;
	virtual void Initialize() override;
	virtual void Think() override;
	virtual void OnFirstThink() override;
	virtual util::EventReply MouseCallback(GLFW::MouseButton button,GLFW::KeyState state,GLFW::Modifier mods) override;
	virtual util::EventReply KeyboardCallback(GLFW::Key key,int scanCode,GLFW::KeyState state,GLFW::Modifier mods) override;
	virtual util::EventReply CharCallback(unsigned int c,GLFW::Modifier mods=GLFW::Modifier::None) override;
	virtual util::EventReply ScrollCallback(Vector2 offset) override;
	virtual void SetSize(int x,int y) override;
	virtual void OnVisibilityChanged(bool bVisible) override;
	virtual void SetColor(float r,float g,float b,float a=1.f) override;
	virtual void SetAlpha(float alpha) override;
	virtual void Render(const DrawInfo &drawInfo,const Mat4 &matDraw,const Vector2 &scale) override;
	virtual void OnCursorEntered() override;
	virtual void OnCursorExited() override;
	virtual void OnFocusGained() override;
	virtual void OnFocusKilled() override;
	virtual void OnRemove() override;
protected:
	virtual void DoUpdate() override;
};

struct DLLCLIENT WILuaWrapper
	: luabind::wrap_base
{
public:
	void OnInitialize();
	static void default_OnInitialize(lua_State *l,WIHandle &hElement);

	void OnThink();
	static void default_OnThink(lua_State *l,WIHandle &hElement);

	void OnFirstThink();
	static void default_OnFirstThink(lua_State *l,WIHandle &hElement);

	void MouseCallback(int button,int action,int mods);
	static void default_MouseCallback(lua_State *l,WIHandle &hElement,int button,int action,int mods);

	void KeyboardCallback(int key,int scancode,int action,int mods);
	static void default_KeyboardCallback(lua_State *l,WIHandle &hElement,int key,int scancode,int action,int mods);

	void CharCallback(unsigned int c,uint32_t mods);
	static void default_CharCallback(lua_State *l,WIHandle &hElement,unsigned int c,uint32_t mods);

	void ScrollCallback(double xoffset,double yoffset);
	static void default_ScrollCallback(lua_State *l,WIHandle &hElement,double xoffset,double yoffset);

	void OnUpdate();
	static void default_OnUpdate(lua_State *l,WIHandle &hElement);

	void OnSetSize(int x,int y);
	static void default_OnSetSize(lua_State *l,WIHandle &hElement,int x,int y);

	void OnSetVisible(bool b);
	static void default_OnSetVisible(lua_State *l,WIHandle &hElement,bool b);

	void OnSetColor(float r,float g,float b,float a=1.f);
	static void default_OnSetColor(lua_State *l,WIHandle &hElement,float r,float g,float b,float a=1.f);

	void OnSetAlpha(float alpha);
	static void default_OnSetAlpha(lua_State *l,WIHandle &hElement,float alpha);

	void Render(const ::WIBase::DrawInfo &drawInfo,const Mat4 &matDraw,const Vector2 &scale);
	static void default_Render(lua_State *l,WIHandle &hElement,const ::WIBase::DrawInfo &drawInfo,const Mat4 &matDraw,const Vector2 &scale);

	void OnCursorEntered();
	static void default_OnCursorEntered(lua_State *l,WIHandle &hElement);

	void OnCursorExited();
	static void default_OnCursorExited(lua_State *l,WIHandle &hElement);

	void OnFocusGained();
	static void default_OnFocusGained(lua_State *l,WIHandle &hElement);

	void OnFocusKilled();
	static void default_OnFocusKilled(lua_State *l,WIHandle &hElement);

	void OnRemove();
	static void default_OnRemove(lua_State *l,WIHandle &hElement);
};

#endif