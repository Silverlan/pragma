// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

module pragma.client;

import :gui.lua_interface;

import :client_state;
import :game;
import :scripting.lua;
import pragma.gui;

CallbackHandle pragma::gui::WGUILuaInterface::m_cbGameStart;
CallbackHandle pragma::gui::WGUILuaInterface::m_cbLuaReleased;
lua::State *pragma::gui::WGUILuaInterface::m_guiLuaState = nullptr;

static std::optional<util::EventReply> GUI_Callback_OnMouseEvent(pragma::gui::types::WIBase &p, pragma::platform::MouseButton button, pragma::platform::KeyState state, pragma::platform::Modifier mods)
{
	lua::State *luaStates[2] = {pragma::get_client_state()->GetGUILuaState(), nullptr};
	if(pragma::get_cgame() != nullptr)
		luaStates[1] = pragma::get_cgame()->GetLuaState();
	std::optional<util::EventReply> reply {};
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
					reply = static_cast<util::EventReply>(Lua::CheckInt(lua, -1));
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

static std::optional<util::EventReply> GUI_Callback_OnKeyEvent(pragma::gui::types::WIBase &p, pragma::platform::Key key, int scanCode, pragma::platform::KeyState state, pragma::platform::Modifier mods)
{
	lua::State *luaStates[2] = {pragma::get_client_state()->GetGUILuaState(), nullptr};
	if(pragma::get_cgame() != nullptr)
		luaStates[1] = pragma::get_cgame()->GetLuaState();
	std::optional<util::EventReply> reply {};
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
					reply = static_cast<util::EventReply>(Lua::CheckInt(lua, -1));
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

static std::optional<util::EventReply> GUI_Callback_OnCharEvent(pragma::gui::types::WIBase &p, int c, pragma::platform::Modifier mods)
{
	lua::State *luaStates[2] = {pragma::get_client_state()->GetGUILuaState(), nullptr};
	if(pragma::get_cgame() != nullptr)
		luaStates[1] = pragma::get_cgame()->GetLuaState();
	std::optional<util::EventReply> reply {};
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
					     Lua::PushInt(l, umath::to_integral(mods));
					     return Lua::StatusCode::Ok;
				     },
				     1)
				  == Lua::StatusCode::Ok) {
					reply = static_cast<util::EventReply>(Lua::CheckInt(lua, -1));
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

static std::optional<util::EventReply> GUI_Callback_OnScroll(pragma::gui::types::WIBase &p, Vector2 offset, bool offsetAsPixels)
{
	lua::State *luaStates[2] = {pragma::get_client_state()->GetGUILuaState(), nullptr};
	if(pragma::get_cgame() != nullptr)
		luaStates[1] = pragma::get_cgame()->GetLuaState();
	std::optional<util::EventReply> reply {};
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
					reply = static_cast<util::EventReply>(Lua::CheckInt(lua, -1));
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

void pragma::gui::WGUILuaInterface::OnGameStart() { m_cbLuaReleased = pragma::get_cgame()->AddCallback("OnLuaReleased", FunctionCallback<void, lua::State *>::Create(&pragma::gui::WGUILuaInterface::OnGameLuaReleased)); }

void pragma::gui::WGUILuaInterface::OnGameLuaReleased(lua::State *)
{
	auto *el = pragma::gui::WGUI::GetInstance().GetBaseElement();
	if(el == nullptr)
		return;
	ClearLuaObjects(el);
}

void pragma::gui::WGUILuaInterface::ClearGUILuaObjects(pragma::gui::types::WIBase &el)
{
	el.SetUserData(nullptr);
	std::vector<pragma::gui::WIHandle> *children = el.GetChildren();
	for(unsigned int i = 0; i < children->size(); i++) {
		pragma::gui::WIHandle &hChild = (*children)[i];
		if(hChild.IsValid())
			ClearGUILuaObjects(*hChild.get());
	}
}

void pragma::gui::WGUILuaInterface::ClearLuaObjects(pragma::gui::types::WIBase *el)
{
	el->SetUserData2(nullptr);
	std::vector<pragma::gui::WIHandle> *children = el->GetChildren();
	for(unsigned int i = 0; i < children->size(); i++) {
		pragma::gui::WIHandle &hChild = (*children)[i];
		if(hChild.IsValid())
			ClearLuaObjects(hChild.get());
	}
}

void pragma::gui::WGUILuaInterface::OnGUIDestroy(pragma::gui::types::WIBase &el)
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
	auto *client = pragma::get_client_state();
	m_guiLuaState = client->GetGUILuaState();
	WGUI::GetInstance().SetRemoveCallback(&OnGUIDestroy);
	m_cbGameStart = client->AddCallback("OnGameStart", FunctionCallback<>::Create(&pragma::gui::WGUILuaInterface::OnGameStart));
}

void pragma::gui::WGUILuaInterface::Clear()
{
	if(m_cbGameStart.IsValid())
		m_cbGameStart.Remove();
	if(m_cbLuaReleased.IsValid())
		m_cbLuaReleased.Remove();
}

void pragma::gui::WGUILuaInterface::InitializeGUIElement(pragma::gui::types::WIBase &p)
{
	p.AddCallback("OnMouseEvent",
	  FunctionCallback<util::EventReply, pragma::platform::MouseButton, pragma::platform::KeyState, pragma::platform::Modifier>::CreateWithOptionalReturn(
	    [&p](util::EventReply *reply, pragma::platform::MouseButton button, pragma::platform::KeyState state, pragma::platform::Modifier mods) -> CallbackReturnType {
		    auto r = GUI_Callback_OnMouseEvent(p, button, state, mods);
		    if(r.has_value()) {
			    *reply = *r;
			    return CallbackReturnType::HasReturnValue;
		    }
		    return CallbackReturnType::NoReturnValue;
	    }));
	p.AddCallback("OnKeyEvent",
	  FunctionCallback<util::EventReply, pragma::platform::Key, int, pragma::platform::KeyState, pragma::platform::Modifier>::CreateWithOptionalReturn(
	    [&p](util::EventReply *reply, pragma::platform::Key key, int scanCode, pragma::platform::KeyState state, pragma::platform::Modifier mods) -> CallbackReturnType {
		    auto r = GUI_Callback_OnKeyEvent(p, key, scanCode, state, mods);
		    if(r.has_value()) {
			    *reply = *r;
			    return CallbackReturnType::HasReturnValue;
		    }
		    return CallbackReturnType::NoReturnValue;
	    }));
	p.AddCallback("OnCharEvent", FunctionCallback<util::EventReply, int, pragma::platform::Modifier>::CreateWithOptionalReturn([&p](util::EventReply *reply, int c, pragma::platform::Modifier mods) -> CallbackReturnType {
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

template<typename T>
luabind::object cast_to_type(lua::State *l, pragma::gui::types::WIBase &el)
{
	return pragma::LuaCore::raw_object_to_luabind_object(l, util::weak_shared_handle_cast<pragma::gui::types::WIBase, T>(el.GetHandle()));
}

luabind::object pragma::gui::WGUILuaInterface::CreateLuaObject(lua::State *l, pragma::gui::types::WIBase &p)
{
	for(auto &f : pragma::get_client_state()->GetGUILuaWrapperFactories()) {
		auto r = f(l, p);
		if(r)
			return r;
	}
	if(dynamic_cast<pragma::gui::types::WITextEntry *>(&p) != nullptr) {
		if(dynamic_cast<pragma::gui::types::WINumericEntry *>(&p) != nullptr)
			return cast_to_type<pragma::gui::types::WINumericEntry>(l, p);
		else if(dynamic_cast<pragma::gui::types::WIDropDownMenu *>(&p) != nullptr)
			return cast_to_type<pragma::gui::types::WIDropDownMenu>(l, p);
		else if(dynamic_cast<pragma::gui::types::WICommandLineEntry *>(&p) != nullptr)
			return cast_to_type<pragma::gui::types::WICommandLineEntry>(l, p);
		return cast_to_type<pragma::gui::types::WITextEntry>(l, p);
	}
	else if(dynamic_cast<pragma::gui::types::WIText *>(&p) != nullptr)
		return cast_to_type<pragma::gui::types::WIText>(l, p);
	else if(dynamic_cast<pragma::gui::types::WIOutlinedRect *>(&p) != nullptr)
		return cast_to_type<pragma::gui::types::WIOutlinedRect>(l, p);
	else if(dynamic_cast<pragma::gui::types::WIShape *>(&p) != nullptr) {
		if(dynamic_cast<pragma::gui::types::WITexturedShape *>(&p) != nullptr) {
			if(dynamic_cast<pragma::gui::types::WIRoundedTexturedRect *>(&p) != nullptr)
				return cast_to_type<pragma::gui::types::WIRoundedTexturedRect>(l, p);
			else if(dynamic_cast<pragma::gui::types::WIIcon *>(&p) != nullptr) {
				if(dynamic_cast<pragma::gui::types::WISilkIcon *>(&p) != nullptr)
					return cast_to_type<pragma::gui::types::WISilkIcon>(l, p);
				return cast_to_type<pragma::gui::types::WIIcon>(l, p);
			}
			else if(dynamic_cast<pragma::gui::types::WIDebugSSAO *>(&p) != nullptr)
				return cast_to_type<pragma::gui::types::WIDebugSSAO>(l, p);
			return cast_to_type<pragma::gui::types::WITexturedShape>(l, p);
		}
		else if(dynamic_cast<pragma::gui::types::WIRoundedRect *>(&p) != nullptr)
			return cast_to_type<pragma::gui::types::WIRoundedRect>(l, p);
		else if(dynamic_cast<pragma::gui::types::WICheckbox *>(&p) != nullptr)
			return cast_to_type<pragma::gui::types::WICheckbox>(l, p);
		else if(dynamic_cast<pragma::gui::types::WIArrow *>(&p) != nullptr)
			return cast_to_type<pragma::gui::types::WIArrow>(l, p);
		return cast_to_type<pragma::gui::types::WIShape>(l, p);
	}
	else if(dynamic_cast<pragma::gui::types::WIContainer *>(&p) != nullptr) {
		if(dynamic_cast<pragma::gui::types::WITable *>(&p) != nullptr) {
			if(dynamic_cast<pragma::gui::types::WIGridPanel *>(&p) != nullptr)
				return cast_to_type<pragma::gui::types::WIGridPanel>(l, p);
			else if(dynamic_cast<pragma::gui::types::WITreeList *>(&p) != nullptr)
				return cast_to_type<pragma::gui::types::WITreeList>(l, p);
			return cast_to_type<pragma::gui::types::WITable>(l, p);
		}
		else if(dynamic_cast<pragma::gui::types::WITableRow *>(&p) != nullptr) {
			if(dynamic_cast<pragma::gui::types::WITreeListElement *>(&p) != nullptr)
				return cast_to_type<pragma::gui::types::WITreeListElement>(l, p);
			return cast_to_type<pragma::gui::types::WITableRow>(l, p);
		}
		else if(dynamic_cast<pragma::gui::types::WITableCell *>(&p) != nullptr)
			return cast_to_type<pragma::gui::types::WITableCell>(l, p);
		return cast_to_type<pragma::gui::types::WIContainer>(l, p);
	}
	else if(dynamic_cast<pragma::gui::types::WIScrollBar *>(&p) != nullptr)
		return cast_to_type<pragma::gui::types::WIScrollBar>(l, p);
	else if(dynamic_cast<pragma::gui::types::WISnapArea *>(&p) != nullptr)
		return cast_to_type<pragma::gui::types::WISnapArea>(l, p);
	else if(dynamic_cast<pragma::gui::types::WIButton *>(&p) != nullptr)
		return cast_to_type<pragma::gui::types::WIButton>(l, p);
	else if(dynamic_cast<pragma::gui::types::WILine *>(&p) != nullptr)
		return cast_to_type<pragma::gui::types::WILine>(l, p);
	else if(dynamic_cast<pragma::gui::types::WIScrollContainer *>(&p) != nullptr)
		return cast_to_type<pragma::gui::types::WIScrollContainer>(l, p);
	else if(dynamic_cast<pragma::gui::types::WIConsole *>(&p) != nullptr)
		return cast_to_type<pragma::gui::types::WIConsole>(l, p);
	else if(dynamic_cast<pragma::gui::types::WITransformable *>(&p) != nullptr) {
		if(dynamic_cast<pragma::gui::types::WIFrame *>(&p) != nullptr)
			return cast_to_type<pragma::gui::types::WIFrame>(l, p);
		return cast_to_type<pragma::gui::types::WITransformable>(l, p);
	}
	else if(dynamic_cast<pragma::gui::types::WIDebugDepthTexture *>(&p) != nullptr)
		return cast_to_type<pragma::gui::types::WIDebugDepthTexture>(l, p);
	else if(dynamic_cast<pragma::gui::types::WIDebugShadowMap *>(&p) != nullptr)
		return cast_to_type<pragma::gui::types::WIDebugShadowMap>(l, p);
	else if(dynamic_cast<pragma::gui::types::WIProgressBar *>(&p) != nullptr) {
		if(dynamic_cast<pragma::gui::types::WISlider *>(&p) != nullptr)
			return cast_to_type<pragma::gui::types::WISlider>(l, p);
		return cast_to_type<pragma::gui::types::WIProgressBar>(l, p);
	}
	else if(dynamic_cast<pragma::gui::types::WITooltip *>(&p) != nullptr)
		return cast_to_type<pragma::gui::types::WITooltip>(l, p);
	else if(dynamic_cast<pragma::gui::types::WIRoot *>(&p) != nullptr)
		return cast_to_type<pragma::gui::types::WIRoot>(l, p);
	else if(dynamic_cast<pragma::gui::types::WI9SliceRect *>(&p) != nullptr)
		return cast_to_type<pragma::gui::types::WI9SliceRect>(l, p);
	return pragma::LuaCore::raw_object_to_luabind_object(l, p.GetHandle());
}

luabind::object pragma::gui::WGUILuaInterface::GetLuaObject(lua::State *l, pragma::gui::types::WIBase &p)
{
	luabind::object o {};
	if(l == m_guiLuaState) {
		auto userData = p.GetUserData();
		if(userData == nullptr) {
			o = CreateLuaObject(l, p);
			p.SetUserData(::util::make_shared<luabind::object>(o));
		}
		else
			o = *static_cast<luabind::object *>(userData.get());
	}
	else {
		auto userData2 = std::static_pointer_cast<luabind::object>(p.GetUserData2());
		if(userData2 == nullptr) {
			o = CreateLuaObject(l, p);
			p.SetUserData2(::util::make_shared<luabind::object>(o));
		}
		else
			o = *static_cast<luabind::object *>(userData2.get());
	}
	return o;
}
