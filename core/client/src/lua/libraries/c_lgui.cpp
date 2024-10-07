/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/lua/libraries/c_lgui.h"
#include <wgui/wgui.h>
#include <wgui/wibase.h>
#include <wgui/types/wiroot.h>
#include "pragma/game/c_game.h"
#include "luasystem.h"
#include "pragma/lua/classes/c_lwibase.h"
#include "pragma/lua/classes/c_ldef_wgui.h"
#include "pragma/gui/wiluahandlewrapper.h"
#include "pragma/gui/wiluaskin.h"
#include "pragma/gui/wgui_luainterface.h"
#include <pragma/lua/lua_call.hpp>
#include <pragma/lua/util.hpp>
#include <pragma/lua/classes/ldef_vector.h>
#include <pragma/logging.hpp>
#include <prosper_window.hpp>
#include <prosper_render_pass.hpp>

import pragma.string.unicode;

extern DLLCLIENT CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

static void initialize_element(::WIBase &p)
{
	auto data = p.GetUserData();
	if(data != nullptr) {
		auto wrapper = std::static_pointer_cast<WILuaHandleWrapper>(data);
		wrapper->lua = true;
	}
}
static bool check_element(const std::string &name, ::WIBase *el)
{
	if(el)
		return true;
	Con::cwar << "Failed to create GUI element of type '" << name << "'!" << Con::endl;
	return false;
}
::WIBase *Lua::gui::create(CGame *game, const std::string &name, ::WIBase &parent, int32_t x, int32_t y, uint32_t w, uint32_t h, float left, float top, float right, float bottom)
{
	auto *el = game ? game->CreateGUIElement(name, &parent) : WGUI::GetInstance().Create(name, &parent);
	if(!check_element(name, el))
		return nullptr;
	el->SetPos(x, y);
	el->SetSize(w, h);
	el->SetAnchor(left, top, right, bottom);
	initialize_element(*el);
	return el;
}
::WIBase *Lua::gui::create(CGame *game, const std::string &name, ::WIBase &parent, int32_t x, int32_t y, uint32_t w, uint32_t h)
{
	auto *el = game ? game->CreateGUIElement(name, &parent) : WGUI::GetInstance().Create(name, &parent);
	if(!check_element(name, el))
		return nullptr;
	el->SetPos(x, y);
	el->SetSize(w, h);
	initialize_element(*el);
	return el;
}
::WIBase *Lua::gui::create(CGame *game, const std::string &name, ::WIBase &parent, int32_t x, int32_t y)
{
	auto *el = game ? game->CreateGUIElement(name, &parent) : WGUI::GetInstance().Create(name, &parent);
	if(!check_element(name, el))
		return nullptr;
	el->SetPos(x, y);
	initialize_element(*el);
	return el;
}
::WIBase *Lua::gui::create(CGame *game, const std::string &name, ::WIBase *parent)
{
	auto *el = game ? game->CreateGUIElement(name, parent) : WGUI::GetInstance().Create(name, parent);
	if(!check_element(name, el))
		return nullptr;
	initialize_element(*el);
	return el;
}
::WIBase *Lua::gui::create(CGame *game, const std::string &name)
{
	auto *el = game ? game->CreateGUIElement(name) : WGUI::GetInstance().Create(name);
	if(!check_element(name, el))
		return nullptr;
	initialize_element(*el);
	return el;
}

::WIBase *Lua::gui::create_button(lua_State *l, const std::string &text, ::WIBase &parent, int32_t x, int32_t y)
{
	auto *pButton = WGUI::GetInstance().Create<::WIButton>(&parent);
	if(!pButton)
		return nullptr;
	pButton->SetText(text);
	pButton->SizeToContents();
	pButton->SetPos(x, y);
	initialize_element(*pButton);
	return pButton;
}
::WIBase *Lua::gui::create_button(lua_State *l, const std::string &text, ::WIBase &parent)
{
	auto *pButton = WGUI::GetInstance().Create<::WIButton>(&parent);
	if(!pButton)
		return nullptr;
	pButton->SetText(text);
	pButton->SizeToContents();
	initialize_element(*pButton);
	return pButton;
}
::WIBase *Lua::gui::create_button(lua_State *l, const std::string &text)
{
	auto *pButton = WGUI::GetInstance().Create<::WIButton>();
	if(!pButton)
		return nullptr;
	pButton->SetText(text);
	pButton->SizeToContents();
	initialize_element(*pButton);
	return pButton;
}

static Lua::opt<Lua::mult<Lua::type<::WIBase>, Lua::type<::WIBase>, Lua::type<::WIBase>>> create_checkbox(lua_State *l, const std::string &label, ::WIBase *parent)
{
	auto &wgui = WGUI::GetInstance();
	auto *pContainer = wgui.Create<::WIBase>(parent);
	if(pContainer == nullptr)
		return Lua::nil;
	auto *pCheckbox = wgui.Create<::WICheckbox>(pContainer);
	if(pCheckbox == nullptr) {
		pContainer->Remove();
		return Lua::nil;
	}

	auto *pText = WGUI::GetInstance().Create<::WIText>(pContainer);
	if(pText == nullptr) {
		pContainer->Remove();
		pCheckbox->Remove();
		return Lua::nil;
	}

	pText->SetText(label);
	pText->SizeToContents();
	pText->SetPos(pCheckbox->GetRight() + 5, pCheckbox->GetHeight() * 0.5f - pText->GetHeight() * 0.5f);

	pContainer->SizeToContents();

	return Lua::mult<Lua::type<::WIBase>, Lua::type<::WIBase>, Lua::type<::WIBase>> {l, WGUILuaInterface::GetLuaObject(l, *pContainer), WGUILuaInterface::GetLuaObject(l, *pCheckbox), WGUILuaInterface::GetLuaObject(l, *pText)};
}
Lua::opt<Lua::mult<Lua::type<::WIBase>, Lua::type<::WIBase>, Lua::type<::WIBase>>> Lua::gui::create_checkbox(lua_State *l, const std::string &label, ::WIBase &parent) { return ::create_checkbox(l, label, &parent); }
Lua::opt<Lua::mult<Lua::type<::WIBase>, Lua::type<::WIBase>, Lua::type<::WIBase>>> Lua::gui::create_checkbox(lua_State *l, const std::string &label) { return ::create_checkbox(l, label, nullptr); }

::WIBase *Lua::gui::create_label(lua_State *l, const std::string &str, ::WIBase &parent, int32_t x, int32_t y)
{
	auto *pText = WGUI::GetInstance().Create<::WIText>(&parent);
	if(pText == nullptr)
		return nullptr;
	pText->SetText(str);
	pText->SizeToContents();
	pText->SetPos(x, y);
	initialize_element(*pText);
	return pText;
}
::WIBase *Lua::gui::create_label(lua_State *l, const std::string &str, ::WIBase &parent)
{
	auto *pText = WGUI::GetInstance().Create<::WIText>(&parent);
	if(pText == nullptr)
		return nullptr;
	pText->SetText(str);
	pText->SizeToContents();
	initialize_element(*pText);
	return pText;
}
::WIBase *Lua::gui::create_label(lua_State *l, const std::string &str)
{
	auto *pText = WGUI::GetInstance().Create<::WIText>();
	if(pText == nullptr)
		return nullptr;
	pText->SetText(str);
	pText->SizeToContents();
	initialize_element(*pText);
	return pText;
}

void Lua::gui::register_element(const std::string &className, const Lua::classObject &classData)
{
	auto &manager = c_game->GetLuaGUIManager();
	manager.RegisterGUIElement(className, const_cast<Lua::classObject &>(classData));
}

::WIBase *Lua::gui::get_base_element(const prosper::Window &window) { return WGUI::GetInstance().GetBaseElement(&window); }
::WIBase *Lua::gui::get_base_element() { return WGUI::GetInstance().GetBaseElement(); }

::WIBase *Lua::gui::get_element_under_cursor(lua_State *l, ::WIBase &elRoot, const Lua::func<bool, ::WIBase> &condition)
{
	return ::WGUI::GetInstance().GetCursorGUIElement(&elRoot, [l, condition](::WIBase *el) -> bool {
		auto &oFunc = condition;
		auto result = Lua::CallFunction(
		  l,
		  [&oFunc, el](lua_State *l) -> Lua::StatusCode {
			  oFunc.push(l);
			  auto o = WGUILuaInterface::GetLuaObject(l, *el);
			  o.push(l);
			  return Lua::StatusCode::Ok;
		  },
		  1);
		if(result != Lua::StatusCode::Ok)
			return true;
		auto r = false;
		if(Lua::IsSet(l, -1)) {
			r = Lua::CheckBool(l, -1);
			Lua::Pop(l, 1);
		}
		return r;
	});
}
::WIBase *Lua::gui::get_element_under_cursor(lua_State *l, const prosper::Window *window, const Lua::func<bool, ::WIBase> &condition)
{
	auto *el = WGUI::GetInstance().GetBaseElement(window);
	if(!el)
		return nullptr;
	return get_element_under_cursor(l, *el, condition);
}
::WIBase *Lua::gui::get_element_under_cursor(lua_State *l, const Lua::func<bool, ::WIBase> &condition) { return get_element_under_cursor(l, nullptr, condition); }
::WIBase *Lua::gui::get_element_under_cursor(lua_State *l, const prosper::Window *window)
{
	auto *el = WGUI::GetInstance().GetBaseElement(window);
	if(!el)
		return nullptr;
	return ::WGUI::GetInstance().GetCursorGUIElement(
	  el, [l](::WIBase *el) -> bool { return true; }, window);
}
::WIBase *Lua::gui::get_element_under_cursor(lua_State *l, ::WIBase &elRoot)
{
	return ::WGUI::GetInstance().GetCursorGUIElement(&elRoot, [l](::WIBase *el) -> bool { return true; });
}

::WIBase *Lua::gui::get_element_at_position(lua_State *l, prosper::Window *window, ::WIBase *baseElement, int32_t x, int32_t y, const Lua::func<bool, ::WIBase> &condition)
{
	std::function<bool(::WIBase *)> fCondition = nullptr;

	fCondition = [condition, l](::WIBase *el) -> bool {
		auto result = Lua::CallFunction(
		  l,
		  [condition, el](lua_State *l) -> Lua::StatusCode {
			  condition.push(l);
			  auto o = WGUILuaInterface::GetLuaObject(l, *el);
			  o.push(l);
			  return Lua::StatusCode::Ok;
		  },
		  1);
		if(result != Lua::StatusCode::Ok)
			return true;
		if(Lua::IsSet(l, -1) == false)
			return false;
		return Lua::CheckBool(l, -1);
	};

	return WGUI::GetInstance().GetGUIElement(baseElement, x, y, fCondition, window);
}
::WIBase *Lua::gui::get_element_at_position(lua_State *l, prosper::Window *window, ::WIBase *baseElement, int32_t x, int32_t y) { return WGUI::GetInstance().GetGUIElement(baseElement, x, y, nullptr, window); }
::WIBase *Lua::gui::get_element_at_position(lua_State *l, prosper::Window *window, ::WIBase *baseElement)
{
	int32_t x;
	int32_t y;
	WGUI::GetInstance().GetMousePos(x, y);
	return WGUI::GetInstance().GetGUIElement(baseElement, x, y, nullptr, window);
}
::WIBase *Lua::gui::get_element_at_position(lua_State *l, prosper::Window *window)
{
	int32_t x;
	int32_t y;
	WGUI::GetInstance().GetMousePos(x, y);
	return WGUI::GetInstance().GetGUIElement(nullptr, x, y, nullptr, window);
}
::WIBase *Lua::gui::get_element_at_position(lua_State *l)
{
	int32_t x;
	int32_t y;
	WGUI::GetInstance().GetMousePos(x, y);
	return WGUI::GetInstance().GetGUIElement(nullptr, x, y, nullptr);
}

::WIBase *Lua::gui::get_focused_element(lua_State *l) { return WGUI::GetInstance().GetFocusedElement(); }

::WIBase *Lua::gui::get_focused_element(lua_State *l, prosper::Window &window) { return WGUI::GetInstance().GetFocusedElement(&window); }

bool Lua::gui::load_skin(const std::string &skinName)
{
	auto luaPath = Lua::find_script_file("gui/skins/" + skinName);
	return luaPath && c_game->ExecuteLuaFile(*luaPath, client->GetGUILuaState());
}

static bool register_skin(lua_State *l, const std::string &skin, const luabind::tableT<void> &vars, const luabind::tableT<void> &skinData, const std::string *baseName)
{
	if(l != client->GetGUILuaState()) {
		Lua::Error(l, "Attempted to register GUI skin with client lua state! This is not allowed, skins can only be registered with GUI lua state!");
		return false;
	}

	WILuaSkin *s = WGUI::GetInstance().RegisterSkin<WILuaSkin>(skin, true);
	if(s == nullptr)
		return false;
	WILuaSkin::Settings settings;
	settings.vars = vars;
	settings.skin = skinData;
	if(baseName)
		settings.base = dynamic_cast<WILuaSkin *>(WGUI::GetInstance().GetSkin(*baseName));
	s->Initialize(l, settings);
	return true;
}
void Lua::gui::register_default_skin(const std::string &vars, const std::string &skinData)
{
	auto *skin = dynamic_cast<WILuaSkin *>(WGUI::GetInstance().GetSkin("default"));
	if(!skin)
		return;
	auto *l = client->GetGUILuaState();
	auto resVars = Lua::RunString(l, "return " + vars, 1, "register_default_skin", Lua::HandleTracebackError);
	Lua::CheckTable(l, -1);
	if(resVars != Lua::StatusCode::Ok) {
		Lua::Pop(l, 1);
		return;
	}
	auto tVars = luabind::object {luabind::from_stack(l, -1)};
	Lua::Pop(l);
	auto resSkinData = Lua::RunString(l, "return " + skinData, 1, "register_default_skin", Lua::HandleTracebackError);
	if(resSkinData != Lua::StatusCode::Ok) {
		Lua::Pop(l, 2);
		return;
	}
	Lua::CheckTable(l, -1);
	auto tSkinData = luabind::object {luabind::from_stack(l, -1)};
	Lua::Pop(l);

	WILuaSkin::Settings settings;
	settings.vars = tVars;
	settings.skin = tSkinData;
	skin->MergeInto(l, settings);
}
bool Lua::gui::register_skin(lua_State *l, const std::string &skin, const luabind::tableT<void> &vars, const luabind::tableT<void> &skinData) { return ::register_skin(l, skin, vars, skinData, nullptr); }
bool Lua::gui::register_skin(lua_State *l, const std::string &skin, const luabind::tableT<void> &vars, const luabind::tableT<void> &skinData, const std::string &baseName) { return ::register_skin(l, skin, vars, skinData, &baseName); }

void Lua::gui::set_skin(const std::string &skin) { WGUI::GetInstance().SetSkin(skin); }

bool Lua::gui::skin_exists(const std::string &name)
{
	WISkin *skin = WGUI::GetInstance().GetSkin(name);
	return (skin != nullptr) ? true : false;
}

GLFW::Cursor::Shape Lua::gui::get_cursor() { return WGUI::GetInstance().GetCursor(); }
void Lua::gui::set_cursor(GLFW::Cursor::Shape shape) { WGUI::GetInstance().SetCursor(shape); }
GLFW::CursorMode Lua::gui::get_cursor_input_mode() { return WGUI::GetInstance().GetCursorInputMode(); }
void Lua::gui::set_cursor_input_mode(GLFW::CursorMode mode) { WGUI::GetInstance().SetCursorInputMode(mode); }
::Vector2i Lua::gui::get_window_size(lua_State *l)
{
	auto &context = WGUI::GetInstance().GetContext();
	auto &window = context.GetWindow();
	return window->GetSize();
}

std::shared_ptr<prosper::IImage> Lua::gui::create_color_image(uint32_t w, uint32_t h, prosper::ImageUsageFlags usageFlags, prosper::ImageLayout initialLayout, bool msaa)
{
	auto &context = c_engine->GetRenderContext();
	auto &rtStaging = context.GetWindow().GetStagingRenderTarget();
	auto &texCol = rtStaging->GetTexture();
	auto *texStencil = rtStaging->GetTexture(1);
	if(!texStencil)
		return nullptr;
	auto imgCreateInfo = texCol.GetImage().GetCreateInfo();
	imgCreateInfo.width = w;
	imgCreateInfo.height = h;
	imgCreateInfo.usage = usageFlags;
	imgCreateInfo.postCreateLayout = initialLayout;
	if(msaa)
		imgCreateInfo.samples = WGUI::GetInstance().MSAA_SAMPLE_COUNT;
	return context.CreateImage(imgCreateInfo);
}

std::shared_ptr<prosper::RenderTarget> Lua::gui::create_render_target(uint32_t w, uint32_t h, bool enableMsaa, bool enableSampling)
{
	auto &context = c_engine->GetRenderContext();
	auto &rtStaging = context.GetWindow().GetStagingRenderTarget();
	auto *texStencil = rtStaging->GetTexture(1);
	if(!texStencil)
		return nullptr;
	auto usageFlags = prosper::ImageUsageFlags::ColorAttachmentBit | prosper::ImageUsageFlags::TransferSrcBit;
	if(enableSampling)
		usageFlags |= prosper::ImageUsageFlags::SampledBit;
	auto img = create_color_image(w, h, usageFlags, prosper::ImageLayout::ColorAttachmentOptimal, enableMsaa);
	if(!img)
		return nullptr;
	auto imgCreateInfo = texStencil->GetImage().GetCreateInfo();
	imgCreateInfo.width = w;
	imgCreateInfo.height = h;
	if(enableMsaa)
		imgCreateInfo.samples = WGUI::GetInstance().MSAA_SAMPLE_COUNT;
	auto depthStencilImg = context.CreateImage(imgCreateInfo);

	auto tex = context.CreateTexture(prosper::util::TextureCreateInfo {}, *img, prosper::util::ImageViewCreateInfo {}, prosper::util::SamplerCreateInfo {});
	if(!tex)
		return nullptr;

	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	imgViewCreateInfo.aspectFlags = prosper::ImageAspectFlags::StencilBit;
	auto depthStencilTex = context.CreateTexture({}, *depthStencilImg, imgViewCreateInfo);

	auto &rp = enableMsaa ? WGUI::GetInstance().GetMsaaRenderPass() : context.GetWindow().GetStagingRenderPass();
	return context.CreateRenderTarget({tex, depthStencilTex}, rp.shared_from_this());
}

bool Lua::gui::inject_mouse_input(GLFW::MouseButton button, GLFW::KeyState state, GLFW::Modifier mods, const Vector2i &pCursorPos)
{
	Vector2i cursorPos = {};
	auto &gui = WGUI::GetInstance();

	gui.GetMousePos(cursorPos.x, cursorPos.y);
	auto &window = c_engine->GetWindow();
	window->SetCursorPos({pCursorPos.x, pCursorPos.y});

	auto b = gui.HandleMouseInput(window, static_cast<GLFW::MouseButton>(button), static_cast<GLFW::KeyState>(state), static_cast<GLFW::Modifier>(mods));
	window->SetCursorPos(cursorPos);
	return b;
}
bool Lua::gui::inject_mouse_input(GLFW::MouseButton button, GLFW::KeyState state, GLFW::Modifier mods)
{
	Vector2i cursorPos = {};
	auto &gui = WGUI::GetInstance();
	auto &window = c_engine->GetWindow();
	return gui.HandleMouseInput(window, static_cast<GLFW::MouseButton>(button), static_cast<GLFW::KeyState>(state), static_cast<GLFW::Modifier>(mods));
}
bool Lua::gui::inject_keyboard_input(GLFW::Key key, GLFW::KeyState state, GLFW::Modifier mods)
{
	auto &window = c_engine->GetWindow();
	return WGUI::GetInstance().HandleKeyboardInput(window, key, 0, state, mods);
}
bool Lua::gui::inject_char_input(const std::string &c)
{
	auto &window = c_engine->GetWindow();
	auto b = false;
	if(!c.empty())
		b = WGUI::GetInstance().HandleCharInput(window, c.front());
	return b;
}
bool Lua::gui::inject_scroll_input(lua_State *l, const Vector2 &offset, const ::Vector2i &pCursorPos)
{
	Vector2i cursorPos = {};
	auto &gui = WGUI::GetInstance();

	gui.GetMousePos(cursorPos.x, cursorPos.y);
	auto &window = c_engine->GetWindow();
	window->SetCursorPos({pCursorPos.x, pCursorPos.y});

	auto b = gui.HandleScrollInput(window, offset);
	window->SetCursorPos(cursorPos);
	return b;
}
bool Lua::gui::inject_scroll_input(lua_State *l, const Vector2 &offset)
{
	Vector2i cursorPos = {};
	auto &gui = WGUI::GetInstance();
	auto &window = c_engine->GetWindow();
	return gui.HandleScrollInput(window, offset);
}

float Lua::gui::RealTime(lua_State *l) { return client->RealTime(); }

float Lua::gui::DeltaTime(lua_State *l) { return client->DeltaTime(); }

float Lua::gui::LastThink(lua_State *l) { return client->LastThink(); }
