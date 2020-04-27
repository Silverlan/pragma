/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/game/c_game.h"
#include "pragma/gui/wgui_luainterface.h"
#include <wgui/wibase.h>
#include "pragma/lua/classes/c_ldef_wguihandles.h"
#include "pragma/gui/wiluahandlewrapper.h"
#include <wgui/types/witextentry.h>
#include "pragma/gui/wiluabase.h"
#include <pragma/lua/lua_call.hpp>

extern ClientState *client;
extern CGame *c_game;

CallbackHandle WGUILuaInterface::m_cbGameStart;
CallbackHandle WGUILuaInterface::m_cbLuaReleased;
lua_State *WGUILuaInterface::m_guiLuaState = nullptr;

static std::optional<util::EventReply> GUI_Callback_OnMouseEvent(WIBase &p,GLFW::MouseButton button,GLFW::KeyState state,GLFW::Modifier mods)
{
	lua_State *luaStates[2] = {client->GetGUILuaState(),NULL};
	if(c_game != NULL)
		luaStates[1] = c_game->GetLuaState();
	std::optional<util::EventReply> reply {};
	for(char i=0;i<2;i++)
	{
		if(luaStates[i] != NULL)
		{
			lua_State *lua = luaStates[i];
			auto o = WGUILuaInterface::GetLuaObject(lua,p);

			o.push(lua); /* 1 */
			Lua::PushString(lua,"OnMouseEvent"); /* 2 */
			Lua::GetTableValue(lua,-2); /* 2 */
			if(Lua::IsFunction(lua,-1))
			{
				auto functionIdx = Lua::GetStackTop(lua);
				if(Lua::CallFunction(lua,[functionIdx,&o,button,state,mods](lua_State *l) {
					Lua::PushValue(l,functionIdx);
					o.push(l);
					Lua::PushInt(l,button);
					Lua::PushInt(l,state);
					Lua::PushInt(l,mods);
					return Lua::StatusCode::Ok;
				},1) == Lua::StatusCode::Ok)
				{
					reply = static_cast<util::EventReply>(Lua::CheckInt(lua,-1));
					Lua::Pop(lua,1);
				}
				Lua::Pop(lua,1); /* 1 */
			}
			else
				Lua::Pop(lua,1); /* 1 */
			Lua::Pop(lua,1); /* 0 */
			if(reply.has_value())
				return reply;
		}
	}
	return reply;
}

static std::optional<util::EventReply> GUI_Callback_OnKeyEvent(WIBase &p,GLFW::Key key,int scanCode,GLFW::KeyState state,GLFW::Modifier mods)
{
	lua_State *luaStates[2] = {client->GetGUILuaState(),NULL};
	if(c_game != NULL)
		luaStates[1] = c_game->GetLuaState();
	std::optional<util::EventReply> reply {};
	for(char i=0;i<2;i++)
	{
		if(luaStates[i] != NULL)
		{
			lua_State *lua = luaStates[i];
			auto o = WGUILuaInterface::GetLuaObject(lua,p);
			o.push(lua); /* 1 */
			Lua::PushString(lua,"OnKeyEvent"); /* 2 */
			Lua::GetTableValue(lua,-2); /* 2 */
			if(Lua::IsFunction(lua,-1))
			{
				auto functionIdx = Lua::GetStackTop(lua);
				if(Lua::CallFunction(lua,[functionIdx,&o,key,scanCode,state,mods](lua_State *l) {
					Lua::PushValue(l,functionIdx);
					o.push(l);
					Lua::PushInt(l,key);
					Lua::PushInt(l,scanCode);
					Lua::PushInt(l,state);
					Lua::PushInt(l,mods);
					return Lua::StatusCode::Ok;
				},1) == Lua::StatusCode::Ok)
				{
					reply = static_cast<util::EventReply>(Lua::CheckInt(lua,-1));
					Lua::Pop(lua,1);
				}
				Lua::Pop(lua,1); /* 1 */
			}
			else
				Lua::Pop(lua,1); /* 1 */
			Lua::Pop(lua,1); /* 0 */
			if(reply.has_value())
				return reply;
		}
	}
	return reply;
}

static std::optional<util::EventReply> GUI_Callback_OnCharEvent(WIBase &p,int c,GLFW::Modifier mods)
{
	lua_State *luaStates[2] = {client->GetGUILuaState(),NULL};
	if(c_game != NULL)
		luaStates[1] = c_game->GetLuaState();
	std::optional<util::EventReply> reply {};
	for(char i=0;i<2;i++)
	{
		if(luaStates[i] != NULL)
		{
			lua_State *lua = luaStates[i];
			auto o = WGUILuaInterface::GetLuaObject(lua,p);
			o.push(lua); /* 1 */
			Lua::PushString(lua,"OnCharEvent"); /* 2 */
			Lua::GetTableValue(lua,-2); /* 2 */
			if(Lua::IsFunction(lua,-1))
			{
				auto functionIdx = Lua::GetStackTop(lua);
				if(Lua::CallFunction(lua,[functionIdx,&o,c,mods](lua_State *l) {
					Lua::PushValue(l,functionIdx);
					o.push(l);
					char ch = CInt8(c);
					Lua::PushString(l,std::string(&ch,1));
					Lua::PushInt(l,umath::to_integral(mods));
					return Lua::StatusCode::Ok;
				},1) == Lua::StatusCode::Ok)
				{
					reply = static_cast<util::EventReply>(Lua::CheckInt(lua,-1));
					Lua::Pop(lua,1);
				}
				Lua::Pop(lua,1); /* 1 */
			}
			else
				Lua::Pop(lua,1); /* 1 */
			Lua::Pop(lua,1); /* 0 */
			if(reply.has_value())
				return reply;
		}
	}
	return reply;
}

static std::optional<util::EventReply> GUI_Callback_OnScroll(WIBase &p,Vector2 offset)
{
	lua_State *luaStates[2] = {client->GetGUILuaState(),NULL};
	if(c_game != NULL)
		luaStates[1] = c_game->GetLuaState();
	std::optional<util::EventReply> reply {};
	for(char i=0;i<2;i++)
	{
		if(luaStates[i] != NULL)
		{
			lua_State *lua = luaStates[i];
			auto o = WGUILuaInterface::GetLuaObject(lua,p);
			o.push(lua); /* 1 */
			Lua::PushString(lua,"OnScroll"); /* 2 */
			Lua::GetTableValue(lua,-2); /* 2 */
			if(Lua::IsFunction(lua,-1))
			{
				auto functionIdx = Lua::GetStackTop(lua);
				if(Lua::CallFunction(lua,[functionIdx,&o,&offset](lua_State *l) {
					Lua::PushValue(l,functionIdx);
					o.push(l);
					Lua::PushNumber(l,offset.x);
					Lua::PushNumber(l,offset.y);
					return Lua::StatusCode::Ok;
				},1) == Lua::StatusCode::Ok)
				{
					reply = static_cast<util::EventReply>(Lua::CheckInt(lua,-1));
					Lua::Pop(lua,1);
				}
				Lua::Pop(lua,1); /* 1 */
			}
			else
				Lua::Pop(lua,1); /* 1 */
			Lua::Pop(lua,1); /* 0 */
			if(reply.has_value())
				return reply;
		}
	}
	return reply;
}

void WGUILuaInterface::OnGameStart()
{
	m_cbLuaReleased = c_game->AddCallback("OnLuaReleased",FunctionCallback<void,lua_State*>::Create(&WGUILuaInterface::OnGameLuaReleased));
}

void WGUILuaInterface::OnGameLuaReleased(lua_State*)
{
	WIBase *el = WGUI::GetInstance().GetBaseElement();
	if(el == nullptr)
		return;
	ClearLuaObjects(el);
}

void WGUILuaInterface::ClearGUILuaObjects(WIBase &el)
{
	el.SetUserData(nullptr);
	std::vector<WIHandle> *children = el.GetChildren();
	for(unsigned int i=0;i<children->size();i++)
	{
		WIHandle &hChild = (*children)[i];
		if(hChild.IsValid())
			ClearGUILuaObjects(*hChild.get());
	}
}

void WGUILuaInterface::ClearLuaObjects(WIBase *el)
{
	el->SetUserData2(nullptr);
	std::vector<WIHandle> *children = el->GetChildren();
	for(unsigned int i=0;i<children->size();i++)
	{
		WIHandle &hChild = (*children)[i];
		if(hChild.IsValid())
			ClearLuaObjects(hChild.get());
	}
}

void WGUILuaInterface::OnGUIDestroy(WIBase &el)
{
	auto userData = el.GetUserData();
	if(userData != nullptr)
	{
		userData = nullptr;
		el.SetUserData(nullptr);
	}
	auto userData2 = el.GetUserData2();
	if(userData2 != nullptr)
	{
		userData2 = nullptr;
		auto ptr = std::static_pointer_cast<luabind::object>(userData2);
		if(ptr != nullptr)
			el.SetUserData2(nullptr);
	}
}

void WGUILuaInterface::Initialize()
{
	m_guiLuaState = client->GetGUILuaState();
	WGUI::GetInstance().SetRemoveCallback(&OnGUIDestroy);
	m_cbGameStart = client->AddCallback("OnGameStart",FunctionCallback<>::Create(&WGUILuaInterface::OnGameStart));
}

void WGUILuaInterface::Clear()
{
	if(m_cbGameStart.IsValid())
		m_cbGameStart.Remove();
	if(m_cbLuaReleased.IsValid())
		m_cbLuaReleased.Remove();
}

void WGUILuaInterface::InitializeGUIElement(WIBase &p)
{
	p.AddCallback("OnMouseEvent",FunctionCallback<util::EventReply,GLFW::MouseButton,GLFW::KeyState,GLFW::Modifier>::CreateWithOptionalReturn(
		[&p](util::EventReply *reply,GLFW::MouseButton button,GLFW::KeyState state,GLFW::Modifier mods) -> CallbackReturnType {
		auto r = GUI_Callback_OnMouseEvent(p,button,state,mods);
		if(r.has_value())
		{
			*reply = *r;
			return CallbackReturnType::HasReturnValue;
		}
		return CallbackReturnType::NoReturnValue;
	}));
	p.AddCallback("OnKeyEvent",FunctionCallback<util::EventReply,GLFW::Key,int,GLFW::KeyState,GLFW::Modifier>::CreateWithOptionalReturn(
		[&p](util::EventReply *reply,GLFW::Key key,int scanCode,GLFW::KeyState state,GLFW::Modifier mods) -> CallbackReturnType {
		auto r = GUI_Callback_OnKeyEvent(p,key,scanCode,state,mods);
		if(r.has_value())
		{
			*reply = *r;
			return CallbackReturnType::HasReturnValue;
		}
		return CallbackReturnType::NoReturnValue;
	}));
	p.AddCallback("OnCharEvent",FunctionCallback<util::EventReply,int,GLFW::Modifier>::CreateWithOptionalReturn([&p](util::EventReply *reply,int c,GLFW::Modifier mods) -> CallbackReturnType {
		auto r = GUI_Callback_OnCharEvent(p,c,mods);
		if(r.has_value())
		{
			*reply = *r;
			return CallbackReturnType::HasReturnValue;
		}
		return CallbackReturnType::NoReturnValue;
	}));
	p.AddCallback("OnScroll",FunctionCallback<util::EventReply,Vector2>::CreateWithOptionalReturn([&p](util::EventReply *reply,Vector2 offset) -> CallbackReturnType {
		auto r = GUI_Callback_OnScroll(p,offset);
		if(r.has_value())
		{
			*reply = *r;
			return CallbackReturnType::HasReturnValue;
		}
		return CallbackReturnType::NoReturnValue;
	}));
}

luabind::object WGUILuaInterface::CreateLuaObject(lua_State *l,WIBase &p)
{
	for(auto &f : client->GetGUILuaWrapperFactories())
	{
		auto r = f(l,p);
		if(r)
			return r;
	}
	if(dynamic_cast<WITextEntry*>(&p) != nullptr)
	{
		if(dynamic_cast<WINumericEntry*>(&p) != nullptr)
			return luabind::object(l,WINumericEntryHandle(WITextEntryHandle(p.GetHandle())));
		else if(dynamic_cast<WIDropDownMenu*>(&p) != nullptr)
			return luabind::object(l,WIDropDownMenuHandle(WITextEntryHandle(p.GetHandle())));
		else if(dynamic_cast<WICommandLineEntry*>(&p) != nullptr)
			return luabind::object(l,WICommandLineEntryHandle(WITextEntryHandle(p.GetHandle())));
		return luabind::object(l,WITextEntryHandle(p.GetHandle()));
	}
	else if(dynamic_cast<WIText*>(&p) != nullptr)
		return luabind::object(l,WITextHandle(p.GetHandle()));
	else if(dynamic_cast<WIOutlinedRect*>(&p) != nullptr)
		return luabind::object(l,WIOutlinedRectHandle(p.GetHandle()));
	else if(dynamic_cast<WIShape*>(&p) != nullptr)
	{
		if(dynamic_cast<WITexturedShape*>(&p) != nullptr)
		{
			if(dynamic_cast<WIRoundedTexturedRect*>(&p) != nullptr)
				return luabind::object(l,WIRoundedTexturedRectHandle(WITexturedShapeHandle(WIShapeHandle(p.GetHandle()))));
			else if(dynamic_cast<WIIcon*>(&p) != nullptr)
			{
				if(dynamic_cast<WISilkIcon*>(&p) != nullptr)
					return luabind::object(l,WISilkIconHandle(WIIconHandle(WITexturedShapeHandle(WIShapeHandle((p.GetHandle()))))));
				return luabind::object(l,WIIconHandle(WITexturedShapeHandle(WIShapeHandle(p.GetHandle()))));
			}
			else if(dynamic_cast<WIDebugSSAO*>(&p) != nullptr)
				return luabind::object(l,WIDebugSSAOHandle(WITexturedShapeHandle(WIShapeHandle(p.GetHandle()))));
			return luabind::object(l,WITexturedShapeHandle(WIShapeHandle(p.GetHandle())));
		}
		else if(dynamic_cast<WIRoundedRect*>(&p) != nullptr)
			return luabind::object(l,WIRoundedRectHandle(WIShapeHandle(p.GetHandle())));
		else if(dynamic_cast<WICheckbox*>(&p) != nullptr)
			return luabind::object(l,WICheckboxHandle(WIShapeHandle(p.GetHandle())));
		else if(dynamic_cast<WIArrow*>(&p) != nullptr)
			return luabind::object(l,WIArrowHandle(WIShapeHandle(p.GetHandle())));
		return luabind::object(l,WIShapeHandle(p.GetHandle()));
	}
	else if(dynamic_cast<WIContainer*>(&p) != nullptr)
	{
		if(dynamic_cast<WITable*>(&p) != nullptr)
		{
			if(dynamic_cast<WIGridPanel*>(&p) != nullptr)
				return luabind::object(l,WIGridPanelHandle(WITableHandle(WIContainerHandle(p.GetHandle()))));
			else if(dynamic_cast<WITreeList*>(&p) != nullptr)
				return luabind::object(l,WITreeListHandle(WITableHandle(WIContainerHandle(p.GetHandle()))));
			return luabind::object(l,WITableHandle(WIContainerHandle(p.GetHandle())));
		}
		else if(dynamic_cast<WITableRow*>(&p) != nullptr)
		{
			if(dynamic_cast<WITreeListElement*>(&p) != nullptr)
				return luabind::object(l,WITreeListElementHandle(WITableRowHandle(WIContainerHandle(p.GetHandle()))));
			return luabind::object(l,WITableRowHandle(WIContainerHandle(p.GetHandle())));
		}
		else if(dynamic_cast<WITableCell*>(&p) != nullptr)
			return luabind::object(l,WITableCellHandle(WIContainerHandle(p.GetHandle())));
		return luabind::object(l,WIContainerHandle(p.GetHandle()));
	}
	else if(dynamic_cast<WIScrollBar*>(&p) != nullptr)
		return luabind::object(l,WIScrollBarHandle(p.GetHandle()));
	else if(dynamic_cast<WISnapArea*>(&p) != nullptr)
		return luabind::object(l,WISnapAreaHandle(p.GetHandle()));
	else if(dynamic_cast<WIButton*>(&p) != nullptr)
		return luabind::object(l,WIButtonHandle(p.GetHandle()));
	else if(dynamic_cast<WILine*>(&p) != nullptr)
		return luabind::object(l,WILineHandle(p.GetHandle()));
	else if(dynamic_cast<WIScrollContainer*>(&p) != nullptr)
		return luabind::object(l,WIScrollContainerHandle(p.GetHandle()));
	else if(dynamic_cast<WIConsole*>(&p) != nullptr)
		return luabind::object(l,WIConsoleHandle(p.GetHandle()));
	else if(dynamic_cast<WITransformable*>(&p) != nullptr)
	{
		if(dynamic_cast<WIFrame*>(&p) != nullptr)
			return luabind::object(l,WIFrameHandle(WITransformableHandle(p.GetHandle())));
		return luabind::object(l,WITransformableHandle(p.GetHandle()));
	}
	else if(dynamic_cast<WIDebugDepthTexture*>(&p) != nullptr)
		return luabind::object(l,WIDebugDepthTextureHandle(p.GetHandle()));
	else if(dynamic_cast<WIDebugShadowMap*>(&p) != nullptr)
		return luabind::object(l,WIDebugShadowMapHandle(p.GetHandle()));
	else if(dynamic_cast<WIProgressBar*>(&p) != nullptr)
	{
		if(dynamic_cast<WISlider*>(&p) != nullptr)
			return luabind::object(l,WISliderHandle(WIProgressBarHandle(p.GetHandle())));
		return luabind::object(l,WIProgressBarHandle(p.GetHandle()));
	}
	return luabind::object(l,p.GetHandle());
}

luabind::object WGUILuaInterface::GetLuaObject(lua_State *l,WIBase &p)
{
	luabind::object o {};
	if(l == m_guiLuaState)
	{
		auto userData = p.GetUserData();
		if(userData == nullptr)
		{
			o = CreateLuaObject(l,p);
			p.SetUserData(std::make_shared<luabind::object>(o));
		}
		else
			o = *static_cast<luabind::object*>(userData.get());
	}
	else
	{
		auto userData2 = std::static_pointer_cast<luabind::object>(p.GetUserData2());
		if(userData2 == nullptr)
		{
			o = CreateLuaObject(l,p);
			p.SetUserData2(std::make_shared<luabind::object>(o));
		}
		else
			o = *static_cast<luabind::object*>(userData2.get());
	}
	return o;
}

