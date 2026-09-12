// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :gui.lua_interface;

import :client_state;
import :game;
import :scripting.lua;
import pragma.gui;

CallbackHandle pragma::gui::WGUILuaInterface::m_cbGameStart;
CallbackHandle pragma::gui::WGUILuaInterface::m_cbLuaReleased;
lua::State *pragma::gui::WGUILuaInterface::m_guiLuaState = nullptr;

static std::optional<pragma::util::EventReply> GUI_Callback_OnMouseEvent(pragma::gui::types::WIBase &p, pragma::platform::MouseButton button, pragma::platform::KeyState state, pragma::platform::Modifier mods)
{
	lua::State *luaStates[2] = {pragma::get_client_state()->GetGUILuaState(), nullptr};
	if(pragma::get_cgame() != nullptr)
		luaStates[1] = pragma::get_cgame()->GetLuaState();
	std::optional<pragma::util::EventReply> reply {};
	for(char i = 0; i < 2; i++) {
		if(luaStates[i] != nullptr) {
			lua::State *lua = luaStates[i];
			auto o = pragma::gui::WGUILuaInterface::GetLuaObject(lua, p);

			o.push(lua);                          /* 1 */
			Lua::PushString(lua, "OnMouseEvent"); /* 2 */
			Lua::GetTableValue(lua, -2);          /* 2 */
			if(Lua::IsFunction(lua, -1)) {
				auto functionIdx = Lua::GetStackTop(lua);
				if(Lua::CallFunction(
				     lua,
				     [functionIdx, &o, button, state, mods](lua::State *l) {
					     Lua::PushValue(l, functionIdx);
					     o.push(l);
					     Lua::PushInt(l, button);
					     Lua::PushInt(l, state);
					     Lua::PushInt(l, mods);
					     return Lua::StatusCode::Ok;
				     },
				     1)
				  == Lua::StatusCode::Ok) {
					reply = static_cast<pragma::util::EventReply>(Lua::CheckInt(lua, -1));
					Lua::Pop(lua, 1);
				}
				Lua::Pop(lua, 1); /* 1 */
			}
			else
				Lua::Pop(lua, 1); /* 1 */
			Lua::Pop(lua, 1);     /* 0 */
			if(reply.has_value())
				return reply;
		}
	}
	return reply;
}

static std::optional<pragma::util::EventReply> GUI_Callback_OnKeyEvent(pragma::gui::types::WIBase &p, pragma::platform::Key key, int scanCode, pragma::platform::KeyState state, pragma::platform::Modifier mods)
{
	lua::State *luaStates[2] = {pragma::get_client_state()->GetGUILuaState(), nullptr};
	if(pragma::get_cgame() != nullptr)
		luaStates[1] = pragma::get_cgame()->GetLuaState();
	std::optional<pragma::util::EventReply> reply {};
	for(char i = 0; i < 2; i++) {
		if(luaStates[i] != nullptr) {
			lua::State *lua = luaStates[i];
			auto o = pragma::gui::WGUILuaInterface::GetLuaObject(lua, p);
			o.push(lua);                        /* 1 */
			Lua::PushString(lua, "OnKeyEvent"); /* 2 */
			Lua::GetTableValue(lua, -2);        /* 2 */
			if(Lua::IsFunction(lua, -1)) {
				auto functionIdx = Lua::GetStackTop(lua);
				if(Lua::CallFunction(
				     lua,
				     [functionIdx, &o, key, scanCode, state, mods](lua::State *l) {
					     Lua::PushValue(l, functionIdx);
					     o.push(l);
					     Lua::PushInt(l, key);
					     Lua::PushInt(l, scanCode);
					     Lua::PushInt(l, state);
					     Lua::PushInt(l, mods);
					     return Lua::StatusCode::Ok;
				     },
				     1)
				  == Lua::StatusCode::Ok) {
					reply = static_cast<pragma::util::EventReply>(Lua::CheckInt(lua, -1));
					Lua::Pop(lua, 1);
				}
				Lua::Pop(lua, 1); /* 1 */
			}
			else
				Lua::Pop(lua, 1); /* 1 */
			Lua::Pop(lua, 1);     /* 0 */
			if(reply.has_value())
				return reply;
		}
	}
	return reply;
}

static std::optional<pragma::util::EventReply> GUI_Callback_OnCharEvent(pragma::gui::types::WIBase &p, int c, pragma::platform::Modifier mods)
{
	lua::State *luaStates[2] = {pragma::get_client_state()->GetGUILuaState(), nullptr};
	if(pragma::get_cgame() != nullptr)
		luaStates[1] = pragma::get_cgame()->GetLuaState();
	std::optional<pragma::util::EventReply> reply {};
	for(char i = 0; i < 2; i++) {
		if(luaStates[i] != nullptr) {
			lua::State *lua = luaStates[i];
			auto o = pragma::gui::WGUILuaInterface::GetLuaObject(lua, p);
			o.push(lua);                         /* 1 */
			Lua::PushString(lua, "OnCharEvent"); /* 2 */
			Lua::GetTableValue(lua, -2);         /* 2 */
			if(Lua::IsFunction(lua, -1)) {
				auto functionIdx = Lua::GetStackTop(lua);
				if(Lua::CallFunction(
				     lua,
				     [functionIdx, &o, c, mods](lua::State *l) {
					     Lua::PushValue(l, functionIdx);
					     o.push(l);
					     char ch = CInt8(c);
					     Lua::PushString(l, std::string(&ch, 1));
					     Lua::PushInt(l, pragma::math::to_integral(mods));
					     return Lua::StatusCode::Ok;
				     },
				     1)
				  == Lua::StatusCode::Ok) {
					reply = static_cast<pragma::util::EventReply>(Lua::CheckInt(lua, -1));
					Lua::Pop(lua, 1);
				}
				Lua::Pop(lua, 1); /* 1 */
			}
			else
				Lua::Pop(lua, 1); /* 1 */
			Lua::Pop(lua, 1);     /* 0 */
			if(reply.has_value())
				return reply;
		}
	}
	return reply;
}

static std::optional<pragma::util::EventReply> GUI_Callback_OnScroll(pragma::gui::types::WIBase &p, Vector2 offset, bool offsetAsPixels)
{
	lua::State *luaStates[2] = {pragma::get_client_state()->GetGUILuaState(), nullptr};
	if(pragma::get_cgame() != nullptr)
		luaStates[1] = pragma::get_cgame()->GetLuaState();
	std::optional<pragma::util::EventReply> reply {};
	for(char i = 0; i < 2; i++) {
		if(luaStates[i] != nullptr) {
			lua::State *lua = luaStates[i];
			auto o = pragma::gui::WGUILuaInterface::GetLuaObject(lua, p);
			o.push(lua);                      /* 1 */
			Lua::PushString(lua, "OnScroll"); /* 2 */
			Lua::GetTableValue(lua, -2);      /* 2 */
			if(Lua::IsFunction(lua, -1)) {
				auto functionIdx = Lua::GetStackTop(lua);
				if(Lua::CallFunction(
				     lua,
				     [functionIdx, &o, &offset, &offsetAsPixels](lua::State *l) {
					     Lua::PushValue(l, functionIdx);
					     o.push(l);
					     Lua::PushNumber(l, offset.x);
					     Lua::PushNumber(l, offset.y);
					     Lua::PushBool(l, offsetAsPixels);
					     return Lua::StatusCode::Ok;
				     },
				     1)
				  == Lua::StatusCode::Ok) {
					reply = static_cast<pragma::util::EventReply>(Lua::CheckInt(lua, -1));
					Lua::Pop(lua, 1);
				}
				Lua::Pop(lua, 1); /* 1 */
			}
			else
				Lua::Pop(lua, 1); /* 1 */
			Lua::Pop(lua, 1);     /* 0 */
			if(reply.has_value())
				return reply;
		}
	}
	return reply;
}

void pragma::gui::WGUILuaInterface::OnGameStart() { m_cbLuaReleased = get_cgame()->AddCallback("OnLuaReleased", FunctionCallback<void, lua::State *>::Create(&WGUILuaInterface::OnGameLuaReleased)); }

void pragma::gui::WGUILuaInterface::OnGameLuaReleased(lua::State *)
{
	auto *el = WGUI::GetInstance().GetBaseElement();
	if(el == nullptr)
		return;
	ClearLuaObjects(el);
}

void pragma::gui::WGUILuaInterface::ClearGUILuaObjects(types::WIBase &el)
{
	el.SetUserData(nullptr);
	std::vector<WIHandle> *children = el.GetChildren();
	for(unsigned int i = 0; i < children->size(); i++) {
		WIHandle &hChild = (*children)[i];
		if(hChild.IsValid())
			ClearGUILuaObjects(*hChild.get());
	}
}

void pragma::gui::WGUILuaInterface::ClearLuaObjects(types::WIBase *el)
{
	el->SetUserData2(nullptr);
	std::vector<WIHandle> *children = el->GetChildren();
	for(unsigned int i = 0; i < children->size(); i++) {
		WIHandle &hChild = (*children)[i];
		if(hChild.IsValid())
			ClearLuaObjects(hChild.get());
	}
}

void pragma::gui::WGUILuaInterface::OnGUIDestroy(types::WIBase &el)
{
	auto userData = el.GetUserData();
	if(userData != nullptr) {
		userData = nullptr;
		el.SetUserData(nullptr);
	}
	auto userData2 = el.GetUserData2();
	if(userData2 != nullptr) {
		userData2 = nullptr;
		auto ptr = std::static_pointer_cast<luabind::object>(userData2);
		if(ptr != nullptr)
			el.SetUserData2(nullptr);
	}
}

void pragma::gui::WGUILuaInterface::Initialize()
{
	auto *client = get_client_state();
	m_guiLuaState = client->GetGUILuaState();
	WGUI::GetInstance().SetRemoveCallback(&OnGUIDestroy);
	m_cbGameStart = client->AddCallback("OnGameStart", FunctionCallback<>::Create(&WGUILuaInterface::OnGameStart));
}

void pragma::gui::WGUILuaInterface::Clear()
{
	if(m_cbGameStart.IsValid())
		m_cbGameStart.Remove();
	if(m_cbLuaReleased.IsValid())
		m_cbLuaReleased.Remove();
}

void pragma::gui::WGUILuaInterface::InitializeGUIElement(types::WIBase &p)
{
	p.AddCallback("OnMouseEvent",
	  FunctionCallback<util::EventReply, platform::MouseButton, platform::KeyState, platform::Modifier>::CreateWithOptionalReturn([&p](util::EventReply *reply, platform::MouseButton button, platform::KeyState state, platform::Modifier mods) -> CallbackReturnType {
		  auto r = GUI_Callback_OnMouseEvent(p, button, state, mods);
		  if(r.has_value()) {
			  *reply = *r;
			  return CallbackReturnType::HasReturnValue;
		  }
		  return CallbackReturnType::NoReturnValue;
	  }));
	p.AddCallback("OnKeyEvent", FunctionCallback<util::EventReply, platform::Key, int, platform::KeyState, platform::Modifier>::CreateWithOptionalReturn([&p](util::EventReply *reply, platform::Key key, int scanCode, platform::KeyState state, platform::Modifier mods) -> CallbackReturnType {
		auto r = GUI_Callback_OnKeyEvent(p, key, scanCode, state, mods);
		if(r.has_value()) {
			*reply = *r;
			return CallbackReturnType::HasReturnValue;
		}
		return CallbackReturnType::NoReturnValue;
	}));
	p.AddCallback("OnCharEvent", FunctionCallback<util::EventReply, int, platform::Modifier>::CreateWithOptionalReturn([&p](util::EventReply *reply, int c, platform::Modifier mods) -> CallbackReturnType {
		auto r = GUI_Callback_OnCharEvent(p, c, mods);
		if(r.has_value()) {
			*reply = *r;
			return CallbackReturnType::HasReturnValue;
		}
		return CallbackReturnType::NoReturnValue;
	}));
	p.AddCallback("OnScroll", FunctionCallback<util::EventReply, Vector2, bool>::CreateWithOptionalReturn([&p](util::EventReply *reply, Vector2 offset, bool offsetAsPixels) -> CallbackReturnType {
		auto r = GUI_Callback_OnScroll(p, offset, offsetAsPixels);
		if(r.has_value()) {
			*reply = *r;
			return CallbackReturnType::HasReturnValue;
		}
		return CallbackReturnType::NoReturnValue;
	}));
}

luabind::object pragma::gui::WGUILuaInterface::CreateLuaObject(lua::State *l, types::WIBase &p)
{
	for(auto &f : get_client_state()->GetGUILuaWrapperFactories()) {
		auto r = f(l, p);
		if(r)
			return r;
	}
	return get_cengine()->CreateGuiElementLuaObject(l, p);
}

luabind::object pragma::gui::WGUILuaInterface::GetLuaObject(lua::State *l, types::WIBase &p)
{
	luabind::object o {};
	if(l == m_guiLuaState) {
		auto userData = p.GetUserData();
		if(userData == nullptr) {
			o = CreateLuaObject(l, p);
			p.SetUserData(pragma::util::make_shared<luabind::object>(o));
		}
		else
			o = *static_cast<luabind::object *>(userData.get());
	}
	else {
		auto userData2 = std::static_pointer_cast<luabind::object>(p.GetUserData2());
		if(userData2 == nullptr) {
			o = CreateLuaObject(l, p);
			p.SetUserData2(pragma::util::make_shared<luabind::object>(o));
		}
		else
			o = *static_cast<luabind::object *>(userData2.get());
	}
	return o;
}
