#include "stdafx_client.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/lua/libraries/c_lgui.h"
#include <wgui/wgui.h>
#include <wgui/wibase.h>
#include "pragma/game/c_game.h"
#include "luasystem.h"
#include "pragma/lua/classes/c_lwibase.h"
#include "pragma/lua/classes/c_ldef_wgui.h"
#include "pragma/gui/wiluahandlewrapper.h"
#include "pragma/gui/wiluaskin.h"
#include "pragma/gui/wgui_luainterface.h"
#include <pragma/lua/classes/ldef_vector.h>

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;
int Lua::gui::create(lua_State *l)
{
	std::string name = Lua::CheckString(l,1);
	WIHandle *hParent = NULL;
	if(Lua::IsSet(l,2))
		hParent = Lua::CheckGUIElementHandle(l,2);
	::WIBase *p;
	if(c_game != NULL)
		p = c_game->CreateGUIElement(name,hParent);
	else
	{
		::WIBase *parent = NULL;
		if(hParent != NULL)
			parent = hParent->get();
		p = WGUI::GetInstance().Create(name,parent);
	}
	if(p == NULL)
		return 0;
	auto data = p->GetUserData();
	if(data != nullptr)
	{
		auto wrapper = std::static_pointer_cast<WILuaHandleWrapper>(data);
		wrapper->lua = true;
	}
	auto obj = WGUILuaInterface::GetLuaObject(l,*p);
	obj.push(l);
	return 1;
}

int Lua::gui::create_button(lua_State *l)
{
	int32_t arg = 1;
	auto *str = Lua::CheckString(l,arg++);
	WIHandle *hParent = nullptr;
	if(Lua::IsSet(l,arg))// && !Lua::IsNumber(l,arg))
		hParent = Lua::CheckGUIElementHandle(l,arg++);
	auto *parent = (hParent != nullptr && hParent->IsValid()) ? hParent->get() : nullptr;
	auto *pButton = WGUI::GetInstance().Create<::WIButton>(parent);
	if(pButton == nullptr)
		return 0;
	pButton->SetText(str);
	pButton->SizeToContents();
	if(Lua::IsNumber(l,arg))
	{
		auto x = Lua::CheckInt(l,arg++);
		auto y = Lua::CheckInt(l,arg++);
		pButton->SetPos(x,y);
	}
	auto obj = WGUILuaInterface::GetLuaObject(l,*pButton);
	obj.push(l);
	return 1;
}

int Lua::gui::create_checkbox(lua_State *l)
{
	std::string label = Lua::CheckString(l,1);

	WIHandle *hParent = nullptr;
	if(Lua::IsSet(l,2))
		hParent = Lua::CheckGUIElementHandle(l,2);
	auto *parent = (hParent != nullptr && hParent->IsValid()) ? hParent->get() : nullptr;

	auto &wgui = WGUI::GetInstance();
	auto *pContainer = wgui.Create<::WIBase>(parent);
	if(pContainer == nullptr)
		return 0;
	auto *pCheckbox = wgui.Create<::WICheckbox>(pContainer);
	if(pCheckbox == nullptr)
	{
		pContainer->Remove();
		return 0;
	}

	auto *pText = WGUI::GetInstance().Create<::WIText>(pContainer);
	if(pText == nullptr)
	{
		pContainer->Remove();
		pCheckbox->Remove();
		return 0;
	}

	pText->SetText(label);
	pText->SizeToContents();
	pText->SetPos(pCheckbox->GetRight() +5,pCheckbox->GetHeight() *0.5f -pText->GetHeight() *0.5f);

	pContainer->SizeToContents();

	auto oContainer = WGUILuaInterface::GetLuaObject(l,*pContainer);
	oContainer.push(l);

	auto oCheckbox = WGUILuaInterface::GetLuaObject(l,*pCheckbox);
	oCheckbox.push(l);

	auto oText = WGUILuaInterface::GetLuaObject(l,*pText);
	oText.push(l);
	return 3;
}

int Lua::gui::create_label(lua_State *l)
{
	int32_t arg = 1;
	auto *str = Lua::CheckString(l,arg++);
	WIHandle *hParent = nullptr;
	if(Lua::IsSet(l,arg))// && !Lua::IsNumber(l,arg))
		hParent = Lua::CheckGUIElementHandle(l,arg++);
	auto *parent = (hParent != nullptr && hParent->IsValid()) ? hParent->get() : nullptr;
	auto *pText = WGUI::GetInstance().Create<::WIText>(parent);
	if(pText == nullptr)
		return 0;
	pText->SetText(str);
	pText->SizeToContents();
	if(Lua::IsNumber(l,arg))
	{
		auto x = Lua::CheckInt(l,arg++);
		auto y = Lua::CheckInt(l,arg++);
		pText->SetPos(x,y);
	}
	auto obj = WGUILuaInterface::GetLuaObject(l,*pText);
	obj.push(l);
	return 1;
}

int Lua::gui::register_element(lua_State *l)
{
	auto className = Lua::CheckString(l,1);
	luaL_checkuserdata(l,2);
	auto o = luabind::object(luabind::from_stack(l,2));
	if(o)
	{
		auto &manager = c_game->GetLuaGUIManager();
		manager.RegisterGUIElement(className,o);
	}
	return 0;
}

int Lua::gui::get_base_element(lua_State *l)
{
	auto *el = WGUI::GetInstance().GetBaseElement();
	if(el == NULL)
		return 0;
	auto o = WGUILuaInterface::GetLuaObject(l,*el);
	o.push(l);
	return 1;
}

int Lua::gui::get_focused_element(lua_State *l)
{
	auto *el = WGUI::GetInstance().GetFocusedElement();
	if(el == NULL)
		return 0;
	auto o = WGUILuaInterface::GetLuaObject(l,*el);
	o.push(l);
	return 1;
}

int Lua::gui::register_skin(lua_State *l)
{
	std::string skin = Lua::CheckString(l,1);
	Lua::CheckTable(l,2);
	Lua::CheckTable(l,3);
	if(Lua::IsSet(l,4))
		Lua::CheckString(l,4);
	WILuaSkin *s = WGUI::GetInstance().RegisterSkin<WILuaSkin>(skin,true);
	if(s == nullptr)
	{
		Lua::PushBool(l,false);
		return 1;
	}
	WILuaSkin::Settings settings;

	Lua::PushValue(l,2);
	settings.vars = Lua::CreateReference(l);

	Lua::PushValue(l,3);
	settings.skin = Lua::CreateReference(l);
	if(Lua::IsSet(l,4))
	{
		std::string baseName = Lua::CheckString(l,4);
		settings.base = dynamic_cast<WILuaSkin*>(WGUI::GetInstance().GetSkin(baseName));
	}
	s->Initialize(l,settings);
	Lua::PushBool(l,true);
	return 1;
}

int Lua::gui::set_skin(lua_State *l)
{
	std::string skin = Lua::CheckString(l,1);
	WGUI::GetInstance().SetSkin(skin);
	return 0;
}

int Lua::gui::skin_exists(lua_State *l)
{
	std::string name = Lua::CheckString(l,1);
	WISkin *skin = WGUI::GetInstance().GetSkin(name);
	Lua::PushBool(l,(skin != nullptr) ? true : false);
	return 1;
}

int Lua::gui::get_cursor(lua_State *l)
{
	Lua::PushInt(l,umath::to_integral(WGUI::GetInstance().GetCursor()));
	return 1;
}
int Lua::gui::set_cursor(lua_State *l)
{
	auto cursor = Lua::CheckInt(l,1);
	WGUI::GetInstance().SetCursor(static_cast<GLFW::Cursor::Shape>(cursor));
	return 0;
}
int Lua::gui::get_cursor_input_mode(lua_State *l)
{
	Lua::PushInt(l,umath::to_integral(WGUI::GetInstance().GetCursorInputMode()));
	return 1;
}
int Lua::gui::set_cursor_input_mode(lua_State *l)
{
	auto mode = Lua::CheckInt(l,1);
	WGUI::GetInstance().SetCursorInputMode(static_cast<GLFW::CursorMode>(mode));
	return 0;
}
int Lua::gui::get_window_size(lua_State *l)
{
	auto &context = WGUI::GetInstance().GetContext();
	auto &window = context.GetWindow();
	Lua::Push<Vector2i>(l,window.GetSize());
	return 1;
}

int Lua::gui::inject_mouse_input(lua_State *l)
{
	auto button = Lua::CheckInt(l,1);
	auto state = Lua::CheckInt(l,2);
	auto mods = Lua::CheckInt(l,3);
	Vector2i cursorPos = {};
	auto bCursorPos = Lua::IsSet(l,4);
	auto &gui = WGUI::GetInstance();
	if(bCursorPos == true)
	{
		gui.GetMousePos(cursorPos.x,cursorPos.y);
		auto *cursorPos = Lua::CheckVector2i(l,4);
		auto &window = c_engine->GetWindow();
		window.SetCursorPos({cursorPos->x,cursorPos->y});
	}
	auto &window = c_engine->GetWindow();
	auto b = gui.HandleMouseInput(window,static_cast<GLFW::MouseButton>(button),static_cast<GLFW::KeyState>(state),static_cast<GLFW::Modifier>(mods));
	if(bCursorPos == true)
		window.SetCursorPos(cursorPos);
	Lua::PushBool(l,b);
	return 1;
}
int Lua::gui::inject_keyboard_input(lua_State *l)
{
	auto key = Lua::CheckInt(l,1);
	auto state = Lua::CheckInt(l,2);
	auto mods = Lua::CheckInt(l,3);
	auto &window = c_engine->GetWindow();
	auto b = WGUI::GetInstance().HandleKeyboardInput(window,static_cast<GLFW::Key>(key),0,static_cast<GLFW::KeyState>(state),static_cast<GLFW::Modifier>(mods));
	Lua::PushBool(l,b);
	return 1;
}
int Lua::gui::inject_char_input(lua_State *l)
{
	std::string c = Lua::CheckString(l,1);
	auto &window = c_engine->GetWindow();
	auto b = false;
	if(!c.empty())
		 b = WGUI::GetInstance().HandleCharInput(window,c.front());
	Lua::PushBool(l,b);
	return 1;
}
int Lua::gui::inject_scroll_input(lua_State *l)
{
	auto *offset = Lua::CheckVector2(l,1);
	Vector2i cursorPos = {};
	auto bCursorPos = Lua::IsSet(l,2);
	auto &gui = WGUI::GetInstance();
	if(bCursorPos == true)
	{
		gui.GetMousePos(cursorPos.x,cursorPos.y);
		auto *cursorPos = Lua::CheckVector2i(l,2);
		auto &window = c_engine->GetWindow();
		window.SetCursorPos({cursorPos->x,cursorPos->y});
	}
	auto &window = c_engine->GetWindow();
	auto b = gui.HandleScrollInput(window,*offset);
	if(bCursorPos == true)
		window.SetCursorPos(cursorPos);
	Lua::PushBool(l,b);
	return 1;
}

DLLCLIENT int Lua_gui_RealTime(lua_State *l)
{
	Lua::PushNumber(l,client->RealTime());
	return 1;
}

DLLCLIENT int Lua_gui_DeltaTime(lua_State *l)
{
	Lua::PushNumber(l,client->DeltaTime());
	return 1;
}

DLLCLIENT int Lua_gui_LastThink(lua_State *l)
{
	Lua::PushNumber(l,client->LastThink());
	return 1;
}
