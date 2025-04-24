/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2025 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/lua/classes/c_lwibase.h"
#include "pragma/lua/libraries/c_lgui.h"
#include "pragma/lua/classes/c_ldef_wgui.h"
#include "pragma/gui/wiconsole.hpp"
#include "pragma/gui/wiluabase.h"
#include "pragma/lua/libraries/c_lsound.h"
#include "pragma/lua/libraries/lsound.h"
#include "pragma/audio/c_lalsound.hpp"
#include "pragma/input/input_binding_layer.hpp"
#include "pragma/lua/libraries/lasset.hpp"
#include "pragma/asset/util_asset.hpp"
#include "pragma/audio/c_laleffect.h"
#include "pragma/lua/converters/gui_element_converter_t.hpp"
#include <pragma/lua/policies/core_policies.hpp>
#include <luabind/copy_policy.hpp>
#include <pragma/lua/policies/default_parameter_policy.hpp>
#include <pragma/lua/util_logging.hpp>
#include <pragma/input/inputhelper.h>
#include <luainterface.hpp>
#include <pragma/lua/custom_constructor.hpp>
#include <pragma/lua/converters/string_view_converter_t.hpp>
#include <pragma/lua/converters/vector_converter_t.hpp>
#include <pragma/lua/converters/pair_converter_t.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/lua/converters/optional_converter_t.hpp>
#include <pragma/lua/converters/thread_pool_converter_t.hpp>
#include <wgui/types/wiroot.h>
#include <wgui/types/witooltip.h>
#include <wgui/types/wicontentwrapper.hpp>
#include <wgui/types/wi9slicerect.hpp>
#include <prosper_window.hpp>
#include <prosper_command_buffer.hpp>
#include <alsoundsystem.hpp>

import pragma.audio.util;
import pragma.platform;
import pragma.string.unicode;

extern DLLCLIENT CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

static int log(lua_State *l, spdlog::level::level_enum logLevel)
{
	auto &el = Lua::Check<::WIBase>(l, 1);
	const char *msg = Lua::CheckString(l, 2);
	std::string loggerName = "ui_" + el.GetClass();
	int32_t argOffset = 2;
	auto n = lua_gettop(l) - argOffset; /* number of arguments */
	switch(n) {
	case 0:
		{
			Lua::logging::log_with_args<0>(loggerName, msg, logLevel, l, argOffset);
			break;
		}
	case 1:
		{
			Lua::logging::log_with_args<1>(loggerName, msg, logLevel, l, argOffset);
			break;
		}
	case 2:
		{
			Lua::logging::log_with_args<2>(loggerName, msg, logLevel, l, argOffset);
			break;
		}
	case 3:
		{
			Lua::logging::log_with_args<3>(loggerName, msg, logLevel, l, argOffset);
			break;
		}
	case 4:
		{
			Lua::logging::log_with_args<4>(loggerName, msg, logLevel, l, argOffset);
			break;
		}
	case 5:
		{
			Lua::logging::log_with_args<5>(loggerName, msg, logLevel, l, argOffset);
			break;
		}
	case 6:
		{
			Lua::logging::log_with_args<6>(loggerName, msg, logLevel, l, argOffset);
			break;
		}
	case 7:
		{
			Lua::logging::log_with_args<7>(loggerName, msg, logLevel, l, argOffset);
			break;
		}
	case 8:
		{
			Lua::logging::log_with_args<8>(loggerName, msg, logLevel, l, argOffset);
			break;
		}
	case 9:
		{
			Lua::logging::log_with_args<9>(loggerName, msg, logLevel, l, argOffset);
			break;
		}
	case 10:
		{
			Lua::logging::log_with_args<10>(loggerName, msg, logLevel, l, argOffset);
			break;
		}
	default:
		Lua::logging::log_with_args<0>(loggerName, msg, logLevel, l, argOffset);
		break;
	}
	return 0;
}

static std::vector<pragma::platform::Key> get_mapped_keys(const std::string &cvarName, uint32_t maxKeys = std::numeric_limits<uint32_t>::max())
{
	std::vector<pragma::platform::Key> mappedKeys;
	c_engine->GetMappedKeys(cvarName, mappedKeys, maxKeys);
	return mappedKeys;
}

static pragma::LuaInputBindingLayerRegister &get_input_binding_layer_register() { return c_game->GetLuaInputBindingLayerRegister(); }
static std::shared_ptr<InputBindingLayer> create_input_binding_layer()
{
	auto layer = std::shared_ptr<InputBindingLayer> {new InputBindingLayer {}, [](InputBindingLayer *layer) {
		                                                 if(!umath::is_flag_set(c_game->GetGameFlags(), Game::GameFlags::ClosingGame))
			                                                 get_input_binding_layer_register().Remove(*layer);
		                                                 delete layer;
	                                                 }};
	get_input_binding_layer_register().Add(*layer);
	return layer;
}

static void register_gui(Lua::Interface &lua)
{
	auto *l = lua.GetState();
	auto guiMod = luabind::module(l, "gui");
	guiMod[luabind::def("create", static_cast<::WIBase *(*)(CGame *, const std::string &, ::WIBase &, int32_t, int32_t, uint32_t, uint32_t, float, float, float, float)>(&Lua::gui::create)),
	  luabind::def("create", static_cast<::WIBase *(*)(CGame *, const std::string &, ::WIBase &, int32_t, int32_t, uint32_t, uint32_t)>(&Lua::gui::create)), luabind::def("create", static_cast<::WIBase *(*)(CGame *, const std::string &, ::WIBase &, int32_t, int32_t)>(&Lua::gui::create)),
	  luabind::def("create", static_cast<::WIBase *(*)(CGame *, const std::string &, ::WIBase *)>(&Lua::gui::create)), luabind::def("create", static_cast<::WIBase *(*)(CGame *, const std::string &)>(&Lua::gui::create)),

	  luabind::def("create_label", static_cast<::WIBase *(*)(lua_State *, const std::string &, ::WIBase &, int32_t, int32_t)>(&Lua::gui::create_label)), luabind::def("create_label", static_cast<::WIBase *(*)(lua_State *, const std::string &, ::WIBase &)>(&Lua::gui::create_label)),
	  luabind::def("create_label", static_cast<::WIBase *(*)(lua_State *, const std::string &)>(&Lua::gui::create_label)),

	  luabind::def("create_button", static_cast<::WIBase *(*)(lua_State *, const std::string &, ::WIBase &, int32_t, int32_t)>(&Lua::gui::create_button)), luabind::def("create_button", static_cast<::WIBase *(*)(lua_State *, const std::string &, ::WIBase &)>(&Lua::gui::create_button)),
	  luabind::def("create_button", static_cast<::WIBase *(*)(lua_State *, const std::string &)>(&Lua::gui::create_button)),

	  luabind::def("create_checkbox", static_cast<Lua::opt<Lua::mult<Lua::type<::WIBase>, Lua::type<::WIBase>, Lua::type<::WIBase>>> (*)(lua_State *, const std::string &, ::WIBase &)>(&Lua::gui::create_checkbox)),
	  luabind::def("create_checkbox", static_cast<Lua::opt<Lua::mult<Lua::type<::WIBase>, Lua::type<::WIBase>, Lua::type<::WIBase>>> (*)(lua_State *, const std::string &)>(&Lua::gui::create_checkbox)),

	  luabind::def("register", &Lua::gui::register_element), luabind::def("get_base_element", static_cast<::WIBase *(*)(const prosper::Window &)>(&Lua::gui::get_base_element)), luabind::def("get_base_element", static_cast<::WIBase *(*)()>(&Lua::gui::get_base_element)),

	  luabind::def("create_render_target", &Lua::gui::create_render_target), luabind::def("create_color_image", &Lua::gui::create_color_image),

	  luabind::def("get_element_at_position", static_cast<::WIBase *(*)(lua_State *)>(&Lua::gui::get_element_at_position)), luabind::def("get_element_at_position", static_cast<::WIBase *(*)(lua_State *, prosper::Window *)>(&Lua::gui::get_element_at_position)),
	  luabind::def("get_element_at_position", static_cast<::WIBase *(*)(lua_State *, prosper::Window *, ::WIBase *)>(&Lua::gui::get_element_at_position)),
	  luabind::def("get_element_at_position", static_cast<::WIBase *(*)(lua_State *, prosper::Window *, ::WIBase *, int32_t, int32_t)>(&Lua::gui::get_element_at_position)),
	  luabind::def("get_element_at_position", static_cast<::WIBase *(*)(lua_State *, prosper::Window *, ::WIBase *, int32_t, int32_t, const Lua::func<bool, ::WIBase> &)>(&Lua::gui::get_element_at_position)),

	  luabind::def("get_element_under_cursor", static_cast<::WIBase *(*)(lua_State *, const Lua::func<bool, ::WIBase> &)>(&Lua::gui::get_element_under_cursor)),
	  luabind::def("get_element_under_cursor", static_cast<::WIBase *(*)(lua_State *, const prosper::Window *, const Lua::func<bool, ::WIBase> &)>(&Lua::gui::get_element_under_cursor)),
	  luabind::def("get_element_under_cursor", static_cast<::WIBase *(*)(lua_State *, const prosper::Window *)>(&Lua::gui::get_element_under_cursor)),
	  luabind::def("get_element_under_cursor", static_cast<::WIBase *(*)(lua_State *, const prosper::Window *)>(&Lua::gui::get_element_under_cursor), luabind::default_parameter_policy<2, static_cast<const prosper::Window *>(nullptr)> {}),
	  luabind::def("get_element_under_cursor", static_cast<::WIBase *(*)(lua_State *, ::WIBase &)>(&Lua::gui::get_element_under_cursor)),

	  luabind::def("find_focused_window", static_cast<prosper::Window *(*)()>([]() -> prosper::Window * { return WGUI::GetInstance().FindFocusedWindow(); }), luabind::pointer_policy<0> {}),
	  luabind::def("get_primary_window", static_cast<prosper::Window *(*)()>([]() -> prosper::Window * { return &c_engine->GetRenderContext().GetWindow(); }), luabind::pointer_policy<0> {}),
	  luabind::def(
	    "get_primary_monitor", +[]() -> pragma::platform::Monitor { return pragma::platform::get_primary_monitor(); }),
	  luabind::def("find_window_under_cursor", static_cast<prosper::Window *(*)()>([]() -> prosper::Window * { return WGUI::GetInstance().FindWindowUnderCursor(); }), luabind::pointer_policy<0> {}),

	  luabind::def("get_focused_element", static_cast<::WIBase *(*)(lua_State *)>(&Lua::gui::get_focused_element)), luabind::def("get_focused_element", static_cast<::WIBase *(*)(lua_State *, prosper::Window &)>(&Lua::gui::get_focused_element)),
	  luabind::def("register_skin", static_cast<bool (*)(lua_State *, const std::string &, const luabind::tableT<void> &, const luabind::tableT<void> &)>(&Lua::gui::register_skin)),
	  luabind::def("register_skin", static_cast<bool (*)(lua_State *, const std::string &, const luabind::tableT<void> &, const luabind::tableT<void> &, const std::string &)>(&Lua::gui::register_skin)), luabind::def("register_default_skin", &Lua::gui::register_default_skin),
	  luabind::def(
	    "set_focus_enabled", +[](const prosper::Window &window, bool focusEnabled) { WGUI::GetInstance().SetFocusEnabled(window, focusEnabled); }),
	  luabind::def(
	    "is_focus_enabled", +[](const prosper::Window &window) { return WGUI::GetInstance().IsFocusEnabled(window); }),

	  luabind::def("load_skin", &Lua::gui::load_skin), luabind::def("set_skin", Lua::gui::set_skin), luabind::def("skin_exists", Lua::gui::skin_exists), luabind::def("get_cursor", Lua::gui::get_cursor), luabind::def("set_cursor", Lua::gui::set_cursor),
	  luabind::def("get_cursor_input_mode", Lua::gui::get_cursor_input_mode), luabind::def("set_cursor_input_mode", Lua::gui::set_cursor_input_mode), luabind::def("get_window_size", Lua::gui::get_window_size),
	  luabind::def("inject_mouse_input", static_cast<bool (*)(pragma::platform::MouseButton, pragma::platform::KeyState, pragma::platform::Modifier, const Vector2i &)>(&Lua::gui::inject_mouse_input)),
	  luabind::def("inject_mouse_input", static_cast<bool (*)(pragma::platform::MouseButton, pragma::platform::KeyState, pragma::platform::Modifier)>(&Lua::gui::inject_mouse_input)), luabind::def("inject_keyboard_input", Lua::gui::inject_keyboard_input), luabind::def("inject_char_input", Lua::gui::inject_char_input),
	  luabind::def("inject_scroll_input", static_cast<bool (*)(lua_State *, const Vector2 &, const ::Vector2i &)>(&Lua::gui::inject_scroll_input)), luabind::def("inject_scroll_input", static_cast<bool (*)(lua_State *, const Vector2 &)>(&Lua::gui::inject_scroll_input)),
	  luabind::def("find_element_by_name", static_cast<::WIBase *(*)(const std::string &)>([](const std::string &name) -> ::WIBase * {
		  auto *p = WGUI::GetInstance().GetBaseElement();
		  if(p == nullptr)
			  return nullptr;
		  return p->FindDescendantByName(name);
	  })),
	  luabind::def("find_element_by_index", static_cast<::WIBase *(*)(uint64_t)>([](uint64_t idx) -> ::WIBase * { return WGUI::GetInstance().FindByIndex(idx); })),
	  luabind::def("find_elements_by_name", static_cast<luabind::tableT<WIBase> (*)(lua_State *, const std::string &)>([](lua_State *l, const std::string &name) -> luabind::tableT<WIBase> {
		  std::vector<WIHandle> results {};
		  auto t = luabind::newtable(l);
		  auto *p = WGUI::GetInstance().GetBaseElement();
		  if(p != nullptr)
			  p->FindDescendantsByName(name, results);
		  auto idx = 1;
		  for(auto &hEl : results) {
			  if(hEl.IsValid() == false)
				  continue;
			  t[idx++] = WGUILuaInterface::GetLuaObject(l, *hEl.get());
		  }
		  return t;
	  })),
	  luabind::def("find_elements_by_class", static_cast<luabind::tableT<WIBase> (*)(lua_State *, const std::string &)>([](lua_State *l, const std::string &className) -> luabind::tableT<WIBase> {
		  auto t = luabind::newtable(l);
		  auto *p = WGUI::GetInstance().GetBaseElement();
		  if(p == nullptr)
			  return t;
		  uint32_t idx = 1;
		  std::function<void(WIBase &)> fIterateChildren = nullptr;
		  fIterateChildren = [l, &fIterateChildren, &className, &t, &idx](WIBase &el) mutable {
			  if(ustring::compare(el.GetClass(), className, false))
				  t[idx++] = WGUILuaInterface::GetLuaObject(l, el);
			  for(auto &hChild : *el.GetChildren()) {
				  if(hChild.IsValid() == false)
					  continue;
				  fIterateChildren(*hChild.get());
			  }
		  };
		  fIterateChildren(*p);
		  return t;
	  })),
	  luabind::def("get_console", &WIConsole::GetConsole), luabind::def("open_console", &WIConsole::Open), luabind::def("close_console", &WIConsole::Close), luabind::def("is_console_open", static_cast<bool (*)(lua_State *)>([](lua_State *l) -> bool {
		  auto *pConsole = WIConsole::GetConsole();
		  auto *pFrame = pConsole ? pConsole->GetFrame() : nullptr;
		  return pFrame ? pFrame->IsVisible() : false;
	  })),
	  luabind::def("get_delta_time", static_cast<float (*)(lua_State *)>([](lua_State *l) -> float { return WGUI::GetInstance().GetDeltaTime(); })), luabind::def("get_base_elements", static_cast<luabind::tableT<WIBase> (*)(lua_State *)>([](lua_State *l) -> luabind::tableT<WIBase> {
		  auto &els = WGUI::GetInstance().GetBaseElements();
		  auto t = luabind::newtable(l);
		  int32_t idx = 1;
		  for(auto &hEl : els) {
			  if(hEl.IsValid() == false)
				  continue;
			  auto o = WGUILuaInterface::GetLuaObject(l, const_cast<::WIBase &>(*hEl.get()));
			  t[idx++] = o;
		  }
		  return t;
	  })),
	  luabind::def("get_delta_time", static_cast<float (*)(lua_State *)>([](lua_State *l) -> float { return WGUI::GetInstance().GetDeltaTime(); })),
	  luabind::def("get_next_gui_element_index", static_cast<uint64_t (*)(lua_State *)>([](lua_State *l) -> uint64_t { return WGUI::GetInstance().GetNextGuiElementIndex(); })),
	  luabind::def("add_base_element", static_cast<::WIBase *(*)(const prosper::Window &)>([](const prosper::Window &window) -> ::WIBase * { return WGUI::GetInstance().AddBaseElement(&window); })),
	  luabind::def("add_base_element", static_cast<::WIBase *(*)()>([]() -> ::WIBase * { return WGUI::GetInstance().AddBaseElement(); })),
	  luabind::def(
	    "get_supported_video_modes", +[]() -> auto{
		    auto &context = WGUI::GetInstance().GetContext();
		    auto &window = context.GetWindow();
		    auto *monitor = window->GetMonitor();
		    auto primaryMonitor = pragma::platform::get_primary_monitor();
		    if(monitor == nullptr)
			    monitor = &primaryMonitor;
		    return monitor->GetSupportedVideoModes();
	    }),
		  luabind::def("open_main_menu",+[](ClientState *cl) { cl->OpenMainMenu();
			}),
		  luabind::def("close_main_menu",+[](ClientState *cl) { cl->CloseMainMenu();
			}),
		  luabind::def("is_main_menu_open",+[](ClientState *cl) -> bool { return cl->IsMainMenuOpen();
		})
		];

	//
	auto videoModeDef = luabind::class_<pragma::platform::Monitor::VideoMode>("VideoMode");
	videoModeDef.def(
	  "__tostring", +[](const pragma::platform::Monitor::VideoMode &videoMode) -> std::string {
		  std::stringstream ss;
		  ss << "VideoMode[resolution:";
		  ss << videoMode.width << "x" << videoMode.height << "][";
		  ss << "refreshRate:" << videoMode.refreshRate << "]";
		  return ss.str();
	  });
	videoModeDef.def_readonly("width", &pragma::platform::Monitor::VideoMode::width);
	videoModeDef.def_readonly("height", &pragma::platform::Monitor::VideoMode::height);
	videoModeDef.def_readonly("redBits", &pragma::platform::Monitor::VideoMode::redBits);
	videoModeDef.def_readonly("greenBits", &pragma::platform::Monitor::VideoMode::greenBits);
	videoModeDef.def_readonly("blueBits", &pragma::platform::Monitor::VideoMode::blueBits);
	videoModeDef.def_readonly("refreshRate", &pragma::platform::Monitor::VideoMode::refreshRate);
	guiMod[videoModeDef];

	auto wiElementClassDef = luabind::class_<::WIBase>("Element");
	Lua::WIBase::register_class(wiElementClassDef);
	guiMod[wiElementClassDef];

	luabind::object oLogger = luabind::globals(l)["gui"];
	oLogger = oLogger["Element"];
	Lua::logging::add_log_func<spdlog::level::trace, &log>(l, oLogger, "LogTrace");
	Lua::logging::add_log_func<spdlog::level::debug, &log>(l, oLogger, "LogDebug");
	Lua::logging::add_log_func<spdlog::level::info, &log>(l, oLogger, "LogInfo");
	Lua::logging::add_log_func<spdlog::level::warn, &log>(l, oLogger, "LogWarn");
	Lua::logging::add_log_func<spdlog::level::err, &log>(l, oLogger, "LogErr");
	Lua::logging::add_log_func<spdlog::level::critical, &log>(l, oLogger, "LogCritical");

	auto defDrawToTex = luabind::class_<Lua::gui::DrawToTextureInfo>("DrawToTextureInfo");
	defDrawToTex.def(luabind::constructor<>());
	defDrawToTex.def_readwrite("enableMsaa", &Lua::gui::DrawToTextureInfo::enableMsaa);
	defDrawToTex.def_readwrite("useStencil", &Lua::gui::DrawToTextureInfo::useStencil);
	defDrawToTex.def_readwrite("width", &Lua::gui::DrawToTextureInfo::width);
	defDrawToTex.def_readwrite("height", &Lua::gui::DrawToTextureInfo::height);
	defDrawToTex.def_readwrite("clearColor", &Lua::gui::DrawToTextureInfo::clearColor);
	defDrawToTex.def_readwrite("resolvedImage", &Lua::gui::DrawToTextureInfo::resolvedImage);
	defDrawToTex.def_readwrite("commandBuffer", &Lua::gui::DrawToTextureInfo::commandBuffer);
	guiMod[defDrawToTex];

	// Custom Classes
	auto wiBaseWIElement = luabind::class_<WILuaBase, luabind::bases<WIBase>, pragma::lua::WILuaBaseHolder>("Base");
	wiBaseWIElement.def(luabind::constructor<>());
	wiBaseWIElement.def("OnInitialize", &WILuaBase::Lua_OnInitialize, &WILuaBase::default_OnInitialize);
	wiBaseWIElement.def("OnThink", &WILuaBase::Lua_OnThink, &WILuaBase::default_OnThink);
	wiBaseWIElement.def("OnInitialThink", &WILuaBase::Lua_OnFirstThink, &WILuaBase::default_OnFirstThink);
	wiBaseWIElement.def("MouseCallback", &WILuaBase::Lua_MouseCallback, &WILuaBase::default_MouseCallback);
	wiBaseWIElement.def("KeyboardCallback", &WILuaBase::Lua_KeyboardCallback, &WILuaBase::default_KeyboardCallback);
	wiBaseWIElement.def("CharCallback", &WILuaBase::Lua_CharCallback, &WILuaBase::default_CharCallback);
	wiBaseWIElement.def("ScrollCallback", &WILuaBase::Lua_ScrollCallback, &WILuaBase::default_ScrollCallback);
	wiBaseWIElement.def("OnUpdate", &WILuaBase::Lua_OnUpdate, &WILuaBase::default_OnUpdate);
	wiBaseWIElement.def("OnVisibilityChanged", &WILuaBase::Lua_OnSetVisible, &WILuaBase::default_OnSetVisible);
	wiBaseWIElement.def("OnSizeChanged", &WILuaBase::Lua_OnSetSize, &WILuaBase::default_OnSetSize);
	wiBaseWIElement.def("OnColorChanged", &WILuaBase::Lua_OnSetColor, &WILuaBase::default_OnSetColor);
	wiBaseWIElement.def("OnAlphaChanged", &WILuaBase::Lua_OnSetAlpha, &WILuaBase::default_OnSetAlpha);
	wiBaseWIElement.def("CheckPosInBounds", &WILuaBase::Lua_CheckPosInBounds, &WILuaBase::default_CheckPosInBounds);
	wiBaseWIElement.def("OnDraw", &WILuaBase::Lua_Render, &WILuaBase::default_Render);
	wiBaseWIElement.def("OnCursorEntered", &WILuaBase::Lua_OnCursorEntered, &WILuaBase::default_OnCursorEntered);
	wiBaseWIElement.def("OnCursorExited", &WILuaBase::Lua_OnCursorExited, &WILuaBase::default_OnCursorExited);
	wiBaseWIElement.def("OnFileDragEntered", &WILuaBase::Lua_OnFileDragEntered, &WILuaBase::default_OnFileDragEntered);
	wiBaseWIElement.def("OnFileDragExited", &WILuaBase::Lua_OnFileDragExited, &WILuaBase::default_OnFileDragExited);
	wiBaseWIElement.def("OnFilesDropped", &WILuaBase::Lua_OnFilesDropped, &WILuaBase::default_OnFilesDropped);
	wiBaseWIElement.def("OnFocusGained", &WILuaBase::Lua_OnFocusGained, &WILuaBase::default_OnFocusGained);
	wiBaseWIElement.def("OnFocusKilled", &WILuaBase::Lua_OnFocusKilled, &WILuaBase::default_OnFocusKilled);
	wiBaseWIElement.def("OnRemove", &WILuaBase::Lua_OnRemove, &WILuaBase::default_OnRemove);
	wiBaseWIElement.def("SetRenderCommandBuffer", &WILuaBase::SetRenderCommandBuffer);
	guiMod[wiBaseWIElement];
	//

	// Class specific handles have to also be defined in CGame::InitializeGUIElement and WGUIHandleFactory!
	auto wiShapeClassDef = luabind::class_<::WIShape, ::WIBase>("Shape");
	Lua::WIShape::register_class(wiShapeClassDef);
	guiMod[wiShapeClassDef];

	auto wiButtonClassDef = luabind::class_<::WIButton, ::WIBase>("Button");
	Lua::WIButton::register_class(wiButtonClassDef);
	guiMod[wiButtonClassDef];

	auto wiTexturedShapeClassDef = luabind::class_<::WITexturedShape, luabind::bases<::WIShape, ::WIBase>>("TexturedShape");
	Lua::WITexturedShape::register_class(wiTexturedShapeClassDef);
	guiMod[wiTexturedShapeClassDef];

	auto wiIconClassDef = luabind::class_<::WIIcon, luabind::bases<::WITexturedShape, ::WIShape, ::WIBase>>("Icon");
	Lua::WIIcon::register_class(wiIconClassDef);
	guiMod[wiIconClassDef];

	auto wiSilkIconClassDef = luabind::class_<::WISilkIcon, luabind::bases<::WIIcon, ::WITexturedShape, ::WIShape, ::WIBase>>("SilkIcon");
	Lua::WISilkIcon::register_class(wiSilkIconClassDef);
	guiMod[wiSilkIconClassDef];

	auto wiArrowClassDef = luabind::class_<::WIArrow, luabind::bases<::WIShape, ::WIBase>>("Arrow");
	Lua::WIArrow::register_class(wiArrowClassDef);
	guiMod[wiArrowClassDef];

	auto wiCheckboxClassDef = luabind::class_<::WICheckbox, luabind::bases<::WIShape, ::WIBase>>("Checkbox");
	Lua::WICheckbox::register_class(wiCheckboxClassDef);
	guiMod[wiCheckboxClassDef];

	auto wiTransformableClassDef = luabind::class_<WITransformable, ::WIBase>("Transformable");
	Lua::WITransformable::register_class(wiTransformableClassDef);
	guiMod[wiTransformableClassDef];

	auto wiSnapAreaClassDef = luabind::class_<WISnapArea, ::WIBase>("SnapArea");
	wiSnapAreaClassDef.def("GetTriggerArea", &WISnapArea::GetTriggerArea);
	guiMod[wiSnapAreaClassDef];

	auto wiDebugDepthTextureClassDef = luabind::class_<WIDebugDepthTexture, ::WIBase>("DebugDepthTexture");
	wiDebugDepthTextureClassDef.def("SetContrastFactor", &WIDebugDepthTexture::SetContrastFactor);
	wiDebugDepthTextureClassDef.def("GetContrastFactor", &WIDebugDepthTexture::GetContrastFactor);
	wiDebugDepthTextureClassDef.def("SetTexture", static_cast<void (WIDebugDepthTexture::*)(prosper::Texture &, bool)>(&WIDebugDepthTexture::SetTexture));
	wiDebugDepthTextureClassDef.def("SetTexture", static_cast<void (WIDebugDepthTexture::*)(prosper::Texture &, bool)>(&WIDebugDepthTexture::SetTexture), luabind::default_parameter_policy<3, false> {});
	guiMod[wiDebugDepthTextureClassDef];

	auto wiDebugShadowMapClassDef = luabind::class_<WIDebugShadowMap, ::WIBase>("DebugShadowMap");
	wiDebugShadowMapClassDef.def("SetContrastFactor", &WIDebugShadowMap::SetContrastFactor);
	wiDebugShadowMapClassDef.def("GetContrastFactor", &WIDebugShadowMap::GetContrastFactor);
	wiDebugShadowMapClassDef.def("SetShadowMapSize", &WIDebugShadowMap::SetShadowMapSize);
	wiDebugShadowMapClassDef.def("SetLightSource", &WIDebugShadowMap::SetLightSource);
	guiMod[wiDebugShadowMapClassDef];

	auto wiDebugSsaoClassDef = luabind::class_<WIDebugSSAO, luabind::bases<::WITexturedShape, WIShape, ::WIBase>>("DebugSSAO");
	wiDebugSsaoClassDef.def("SetUseBlurredSSAOImage", &WIDebugSSAO::SetUseBlurredSSAOImage);
	guiMod[wiDebugSsaoClassDef];

	auto wiProgressBarClassDef = luabind::class_<WIProgressBar, ::WIBase>("ProgressBar");
	Lua::WIProgressBar::register_class(wiProgressBarClassDef);
	guiMod[wiProgressBarClassDef];

	auto wiTooltipClassDef = luabind::class_<WITooltip, ::WIBase>("Tooltip");
	wiTooltipClassDef.def("SetText", +[](WITooltip &elTooltip, const std::string &text) { elTooltip.SetText(text); });
	wiTooltipClassDef.def("GetText", +[](const WITooltip &elTooltip) { return elTooltip.GetText().cpp_str(); });
	guiMod[wiTooltipClassDef];

	auto wiSliderClassDef = luabind::class_<WISlider, luabind::bases<WIProgressBar, ::WIBase>>("Slider");
	Lua::WISlider::register_class(wiSliderClassDef);
	guiMod[wiSliderClassDef];

	auto wiScrollContainerClassDef = luabind::class_<WIScrollContainer, ::WIBase>("ScrollContainer");
	wiScrollContainerClassDef.def("SetAutoStickToBottom", &WIScrollContainer::SetAutoStickToBottom);
	wiScrollContainerClassDef.def("ShouldAutoStickToBottom", &WIScrollContainer::ShouldAutoStickToBottom);
	wiScrollContainerClassDef.def("IsContentsWidthFixed", &WIScrollContainer::IsContentsWidthFixed);
	wiScrollContainerClassDef.def("IsContentsHeightFixed", &WIScrollContainer::IsContentsHeightFixed);
	wiScrollContainerClassDef.def("SetContentsWidthFixed", &WIScrollContainer::SetContentsWidthFixed);
	wiScrollContainerClassDef.def("SetContentsHeightFixed", &WIScrollContainer::SetContentsHeightFixed);
	wiScrollContainerClassDef.def("GetVerticalScrollBar", &WIScrollContainer::GetVerticalScrollBar);
	wiScrollContainerClassDef.def("GetHorizontalScrollBar", &WIScrollContainer::GetHorizontalScrollBar);
	wiScrollContainerClassDef.def("SetScrollAmount", &WIScrollContainer::SetScrollAmount);
	wiScrollContainerClassDef.def("SetScrollAmountX", &WIScrollContainer::SetScrollAmountX);
	wiScrollContainerClassDef.def("SetScrollAmountY", &WIScrollContainer::SetScrollAmountY);
	wiScrollContainerClassDef.def("GetWrapperElement", &WIScrollContainer::GetWrapperElement);
	wiScrollContainerClassDef.def("ScrollToElement", &WIScrollContainer::ScrollToElement);
	wiScrollContainerClassDef.def("ScrollToElementX", &WIScrollContainer::ScrollToElementX);
	wiScrollContainerClassDef.def("ScrollToElementY", &WIScrollContainer::ScrollToElementY);
	guiMod[wiScrollContainerClassDef];

	auto wiContainerClassDef = luabind::class_<WIContainer, ::WIBase>("Container");
	Lua::WIContainer::register_class(wiContainerClassDef);
	guiMod[wiContainerClassDef];

	auto wiTableClassDef = luabind::class_<WITable, luabind::bases<WIContainer, ::WIBase>>("Table");
	Lua::WITable::register_class(wiTableClassDef);

	auto wiTableRowClassDef = luabind::class_<WITableRow, luabind::bases<WIContainer, ::WIBase>>("Row");
	Lua::WITableRow::register_class(wiTableRowClassDef);

	auto wiTableCellClassDef = luabind::class_<WITableCell, luabind::bases<WIContainer, ::WIBase>>("Cell");
	Lua::WITableCell::register_class(wiTableCellClassDef);
	wiTableClassDef.scope[wiTableCellClassDef];
	wiTableClassDef.scope[wiTableRowClassDef];
	guiMod[wiTableClassDef];

	auto wiGridPanelClassDef = luabind::class_<WIGridPanel, luabind::bases<WITable, ::WIBase>>("GridPanel");
	Lua::WIGridPanel::register_class(wiGridPanelClassDef);
	guiMod[wiGridPanelClassDef];

	auto wiTreeListClassDef = luabind::class_<WITreeList, luabind::bases<WITable, ::WIBase>>("TreeList");
	Lua::WITreeList::register_class(wiTreeListClassDef);

	auto wiTreeListElementClassDef = luabind::class_<WITreeListElement, luabind::bases<WITableRow, ::WIBase>>("Element");
	Lua::WITreeListElement::register_class(wiTreeListElementClassDef);
	wiTreeListClassDef.scope[wiTreeListElementClassDef];
	guiMod[wiTreeListClassDef];

	auto wiFrameClassDef = luabind::class_<WIFrame, luabind::bases<WITransformable, ::WIBase>>("Frame");
	Lua::WIFrame::register_class(wiFrameClassDef);
	guiMod[wiFrameClassDef];

	auto wiTextClassDef = luabind::class_<WIText, ::WIBase>("Text");
	Lua::WIText::register_class(wiTextClassDef);
	guiMod[wiTextClassDef];

	auto wiTextEntryClassDef = luabind::class_<WITextEntry, ::WIBase>("TextEntry");
	Lua::WITextEntry::register_class(wiTextEntryClassDef);
	wiTextEntryClassDef.def("GetCaretElement", &WITextEntry::GetCaretElement);
	guiMod[wiTextEntryClassDef];

	auto wiCommandLineEntryClassDef = luabind::class_<WICommandLineEntry, luabind::bases<WITextEntry, ::WIBase>>("CommandLineEntry");
	wiCommandLineEntryClassDef.def("SetAutoCompleteEntryLimit", &WICommandLineEntry::SetAutoCompleteEntryLimit);
	wiCommandLineEntryClassDef.def("GetAutoCompleteEntryLimit", &WICommandLineEntry::GetAutoCompleteEntryLimit);
	guiMod[wiCommandLineEntryClassDef];

	auto wiOutlinedRectClassDef = luabind::class_<WIOutlinedRect, ::WIBase>("OutlinedRect");
	Lua::WIOutlinedRect::register_class(wiOutlinedRectClassDef);

	auto wiLineClassDef = luabind::class_<WILine, ::WIBase>("Line");
	Lua::WILine::register_class(wiLineClassDef);

	auto wiWIContentWrapper = luabind::class_<WIContentWrapper, ::WIBase>("ContentWrapper");
	wiWIContentWrapper.def("SetPadding", static_cast<void (WIContentWrapper::*)(int32_t, int32_t, int32_t, int32_t)>(&WIContentWrapper::SetPadding));
	wiWIContentWrapper.def("ClearPadding", &WIContentWrapper::ClearPadding);
	wiWIContentWrapper.def(
	  "GetPadding", +[](WIContentWrapper &el) -> std::tuple<int32_t, int32_t, int32_t, int32_t> {
		  auto padding = el.GetPadding();
		  return std::make_tuple(padding.left, padding.right, padding.top, padding.bottom);
	  });
	wiWIContentWrapper.def("SetPaddingLeft", &WIContentWrapper::SetPaddingLeft);
	wiWIContentWrapper.def("SetPaddingRight", &WIContentWrapper::SetPaddingRight);
	wiWIContentWrapper.def("SetPaddingTop", &WIContentWrapper::SetPaddingTop);
	wiWIContentWrapper.def("SetPaddingBottom", &WIContentWrapper::SetPaddingBottom);
	wiWIContentWrapper.def("SetPaddingLeftRight", &WIContentWrapper::SetPaddingLeftRight);
	wiWIContentWrapper.def("SetPaddingTopBottom", &WIContentWrapper::SetPaddingTopBottom);
	guiMod[wiWIContentWrapper];

	auto wiNineSliceRect = luabind::class_<wgui::WI9SliceRect, ::WIBase>("NineSliceRect");
	wiNineSliceRect.def("SetMaterial", &wgui::WI9SliceRect::SetMaterial);
	wiNineSliceRect.def("GetMaterial", &wgui::WI9SliceRect::GetMaterial);
	guiMod[wiNineSliceRect];

	auto wiRoundedTexturedRect = luabind::class_<WIRoundedTexturedRect, luabind::bases<WITexturedShape, WIShape, ::WIBase>>("RoundedTexturedRect");
	Lua::WIRoundedTexturedRect::register_class(wiRoundedTexturedRect);
	guiMod[wiRoundedTexturedRect];

	auto wiScrollBarClassDef = luabind::class_<WIScrollBar, ::WIBase>("ScrollBar");
	Lua::WIScrollBar::register_class(wiScrollBarClassDef);

	auto wiNumericEntryClassDef = luabind::class_<WINumericEntry, luabind::bases<WITextEntry, ::WIBase>>("NumericTextEntry");
	Lua::WINumericEntry::register_class(wiNumericEntryClassDef);
	guiMod[wiNumericEntryClassDef];

	auto wiDropDownMenuClassDef = luabind::class_<WIDropDownMenu, luabind::bases<WITextEntry, ::WIBase>>("DropDownMenu");
	Lua::WIDropDownMenu::register_class(wiDropDownMenuClassDef);
	guiMod[wiDropDownMenuClassDef];

	auto wiRootDef = luabind::class_<WIRoot, ::WIBase>("Root");
	wiRootDef.def("GetRootCursorPos", &::WIRoot::GetCursorPos);
	wiRootDef.def("SetRootCursorPosOverride", &::WIRoot::SetCursorPosOverride);
	wiRootDef.def("GetRootCursorPosOverride", &::WIRoot::GetCursorPosOverride);
	wiRootDef.def("ClearRootCursorPosOverride", &::WIRoot::ClearCursorPosOverride);
	wiRootDef.def("GetAssociatedWindow", static_cast<prosper::Window *(::WIRoot::*)()>(&::WIRoot::GetWindow));
	guiMod[wiRootDef];

	auto wiConsoleClassDef = luabind::class_<WIConsole, ::WIBase>("Console");
	wiConsoleClassDef.def("GetCommandLineEntryElement", &WIConsole::GetCommandLineEntryElement);
	wiConsoleClassDef.def("GetTextLogElement", &WIConsole::GetTextLogElement);
	wiConsoleClassDef.def("SetExternallyOwned", &WIConsole::SetExternallyOwned);
	wiConsoleClassDef.def("IsExternallyOwned", &WIConsole::IsExternallyOwned);
	wiConsoleClassDef.def("GetFrame", &WIConsole::GetFrame);
	wiConsoleClassDef.def("GetText", +[](const ::WIConsole &console) { return console.GetText().cpp_str(); });
	wiConsoleClassDef.def("SetText", &WIConsole::SetText);
	wiConsoleClassDef.def("AppendText", &WIConsole::AppendText);
	wiConsoleClassDef.def("Clear", &WIConsole::Clear);
	wiConsoleClassDef.def("SetMaxLogLineCount", &WIConsole::SetMaxLogLineCount);
	wiConsoleClassDef.def("GetMaxLogLineCount", &WIConsole::GetMaxLogLineCount);
	guiMod[wiConsoleClassDef];

	auto wiRoundedRectClassDef = luabind::class_<WIRoundedRect, luabind::bases<WIShape, ::WIBase>>("RoundedRect");
	Lua::WIRoundedRect::register_class(wiRoundedRectClassDef);
	guiMod[wiRoundedRectClassDef];

	guiMod[wiOutlinedRectClassDef];
	guiMod[wiLineClassDef];
	guiMod[wiScrollBarClassDef];
}

void ClientState::RegisterSharedLuaLibraries(Lua::Interface &lua, bool bGUI)
{
	register_gui(lua);

	auto inputMod = luabind::module(lua.GetState(), "input");
	inputMod[luabind::def(
	           "get_mouse_button_state", +[](pragma::platform::MouseButton mouseButton) -> pragma::platform::KeyState { return c_engine->GetWindow()->GetMouseButtonState(mouseButton); }),
	  luabind::def(
	    "get_key_state", +[](pragma::platform::Key key) -> pragma::platform::KeyState { return c_engine->GetWindow()->GetKeyState(key); }),
	  luabind::def(
	    "get_cursor_pos", +[]() -> Vector2 { return c_engine->GetWindow()->GetCursorPos(); }),
	  luabind::def(
	    "set_cursor_pos", +[](const Vector2 &pos) { c_engine->GetWindow()->SetCursorPos(pos); }),
	  luabind::def(
	    "is_ctrl_key_down", +[]() -> bool { return c_engine->GetWindow()->GetKeyState(pragma::platform::Key::LeftControl) != pragma::platform::KeyState::Release || c_engine->GetWindow()->GetKeyState(pragma::platform::Key::RightControl) != pragma::platform::KeyState::Release; }),
	  luabind::def(
	    "is_alt_key_down", +[]() -> bool { return c_engine->GetWindow()->GetKeyState(pragma::platform::Key::LeftAlt) != pragma::platform::KeyState::Release || c_engine->GetWindow()->GetKeyState(pragma::platform::Key::RightAlt) != pragma::platform::KeyState::Release; }),
	  luabind::def(
	    "is_shift_key_down", +[]() -> bool { return c_engine->GetWindow()->GetKeyState(pragma::platform::Key::LeftShift) != pragma::platform::KeyState::Release || c_engine->GetWindow()->GetKeyState(pragma::platform::Key::RightShift) != pragma::platform::KeyState::Release; }),
	  luabind::def(
	    "center_cursor",
	    +[]() {
		    auto *window = WGUI::GetInstance().FindFocusedWindow();
		    if(!window)
			    window = &c_engine->GetWindow();
		    if(!window || !window->IsValid())
			    return;
		    auto windowSize = (*window)->GetSize();
		    (*window)->SetCursorPos(windowSize / 2);
	    }),
	  luabind::def(
	    "get_controller_count", +[]() -> uint32_t { return pragma::platform::get_joysticks().size(); }),
	  luabind::def("get_controller_name", &pragma::platform::get_joystick_name), luabind::def("get_controller_axes", &pragma::platform::get_joystick_axes), luabind::def("get_controller_buttons", &pragma::platform::get_joystick_buttons),
	  luabind::def(
	    "key_to_string",
	    +[](short key) -> std::optional<std::string> {
		    std::string str;
		    if(!KeyToString(key, &str))
			    return {};
		    return str;
	    }),
	  luabind::def(
	    "key_to_text",
	    +[](short key) -> std::optional<std::string> {
		    std::string str;
		    if(!KeyToText(key, &str))
			    return {};
		    return str;
	    }),
	  luabind::def(
	    "string_to_key",
	    +[](const std::string &str) -> std::optional<short> {
		    short c;
		    if(!StringToKey(str, &c))
			    return {};
		    return c;
	    }),
	  luabind::def("get_mapped_keys", &get_mapped_keys), luabind::def("get_mapped_keys", &get_mapped_keys, luabind::default_parameter_policy<2, std::numeric_limits<uint32_t>::max()> {}),
	  luabind::def(
	    "add_callback",
	    +[](const std::string &identifier, const Lua::func<void> &f) -> CallbackHandle {
		    auto &inputHandler = c_game->GetInputCallbackHandler();
		    return inputHandler.AddLuaCallback(identifier, f);
	    }),
	  luabind::def(
	    "add_event_listener",
	    +[](const std::string &identifier, const Lua::func<void> &f) -> CallbackHandle {
		    auto &inputHandler = c_game->GetInputCallbackHandler();
		    return inputHandler.AddLuaCallback(identifier, f);
	    }),
	  luabind::def(
	    "add_input_binding_layer", +[](CEngine &en, std::shared_ptr<InputBindingLayer> &layer) { en.AddInputBindingLayer(layer); }),
	  luabind::def("get_input_binding_layers", static_cast<std::vector<std::shared_ptr<InputBindingLayer>> (CEngine::*)()>(&CEngine::GetInputBindingLayers)),
	  luabind::def("get_input_binding_layer", static_cast<std::shared_ptr<InputBindingLayer> (CEngine::*)(const std::string &)>(&CEngine::GetInputBindingLayer)), luabind::def("remove_input_binding_layer", &CEngine::RemoveInputBindingLayer),
	  luabind::def("get_core_input_binding_layers", static_cast<std::shared_ptr<InputBindingLayer> (CEngine::*)()>(&CEngine::GetCoreInputBindingLayer)), luabind::def("update_effective_input_bindings", &CEngine::SetInputBindingsDirty),
	  luabind::def("get_effective_input_binding_layer", &CEngine::GetEffectiveInputBindingLayer),
	  luabind::def(
	    "set_binding_layer_enabled",
	    +[](CEngine &en, const std::string &layerName, bool enabled) {
		    auto layer = en.GetInputBindingLayer(layerName);
		    if(!layer)
			    return;
		    layer->enabled = enabled;
	    }),
	  luabind::def(
	    "is_binding_layer_enabled", +[](CEngine &en, const std::string &layerName) -> std::optional<bool> {
		    auto layer = en.GetInputBindingLayer(layerName);
		    if(!layer)
			    return {};
		    return layer->enabled;
	    })];

	Lua::RegisterLibrary(lua.GetState(), "sound",
	  {
	    {"create", Lua::sound::create},
	    LUA_LIB_SOUND_SHARED,
	    {"create_dsp_effect", Lua::sound::register_aux_effect},
	    {"get_dsp_effect", Lua::sound::get_aux_effect},
	    {"set_distance_model", &Lua::sound::set_distance_model},
	    {"get_distance_model", &Lua::sound::get_distance_model},
	    {"is_supported", &Lua::sound::is_supported},
	    {"get_doppler_factor", &Lua::sound::get_doppler_factor},
	    {"set_doppler_factor", &Lua::sound::set_doppler_factor},
	    {"get_speed_of_sound", &Lua::sound::get_speed_of_sound},
	    {"set_speed_of_sound", &Lua::sound::set_speed_of_sound},
	    {"get_device_name", &Lua::sound::get_device_name},
	    {"add_global_effect", &Lua::sound::add_global_effect},
	    {"remove_global_effect", &Lua::sound::remove_global_effect},
	    {"set_global_effect_parameters", &Lua::sound::set_global_effect_parameters},
	    {"get_duration",
	      +[](lua_State *l) -> int32_t {
		      std::string path = Lua::CheckString(l, 1);
		      auto absPath = pragma::asset::find_file(path, pragma::asset::Type::Sound);
		      if(absPath.has_value() == false) {
			      Lua::PushNil(l);
			      return 1;
		      }
		      float duration;
		      auto success = pragma::audio::util::get_duration(std::string {pragma::asset::get_asset_root_directory(pragma::asset::Type::Sound)} + "/" + *absPath, duration);
		      if(success)
			      Lua::PushNumber(l, duration);
		      else
			      Lua::PushNil(l);
		      return 1;
	      }},
	  });
	Lua::RegisterLibraryEnums(lua.GetState(), "sound",
	  {{"GLOBAL_EFFECT_FLAG_NONE", umath::to_integral(al::ISoundSystem::GlobalEffectFlag::None)}, {"GLOBAL_EFFECT_FLAG_BIT_RELATIVE", umath::to_integral(al::ISoundSystem::GlobalEffectFlag::RelativeSounds)},
	    {"GLOBAL_EFFECT_FLAG_BIT_WORLD", umath::to_integral(al::ISoundSystem::GlobalEffectFlag::WorldSounds)}, {"GLOBAL_EFFECT_FLAG_ALL", umath::to_integral(al::ISoundSystem::GlobalEffectFlag::All)},

	    {"DISTANCE_MODEL_NONE", umath::to_integral(al::DistanceModel::None)}, {"DISTANCE_MODEL_INVERSE_CLAMPED", umath::to_integral(al::DistanceModel::InverseClamped)}, {"DISTANCE_MODEL_LINEAR_CLAMPED", umath::to_integral(al::DistanceModel::LinearClamped)},
	    {"DISTANCE_MODEL_EXPONENT_CLAMPED", umath::to_integral(al::DistanceModel::ExponentClamped)}, {"DISTANCE_MODEL_INVERSE", umath::to_integral(al::DistanceModel::Inverse)}, {"DISTANCE_MODEL_LINEAR", umath::to_integral(al::DistanceModel::Linear)},
	    {"DISTANCE_MODEL_EXPONENT", umath::to_integral(al::DistanceModel::Exponent)}});
	Lua::sound::register_enums(lua.GetState());

	auto defInLay = luabind::class_<InputBindingLayer>("InputBindingLayer");
	defInLay.def_readwrite("identifier", &InputBindingLayer::identifier);
	defInLay.def_readwrite("priority", &InputBindingLayer::priority);
	defInLay.def_readwrite("enabled", &InputBindingLayer::enabled);
	defInLay.scope[luabind::def(
	                 "load",
	                 +[](lua_State *l, const udm::AssetData &data) -> Lua::var<bool, std::vector<std::shared_ptr<InputBindingLayer>>> {
		                 std::vector<std::shared_ptr<InputBindingLayer>> layers;
		                 std::string err;
		                 if(!InputBindingLayer::Load(data, layers, err))
			                 return luabind::object {l, false};
		                 return luabind::object {l, layers};
	                 }),
	  luabind::def(
	    "save", +[](lua_State *l, const udm::AssetData &data, const std::vector<std::shared_ptr<InputBindingLayer>> &layers) {
		    std::string err;
		    return InputBindingLayer::Save(layers, data, err);
	    })];
	defInLay.def("ClearKeyMappings", &InputBindingLayer::ClearKeyMappings);
	defInLay.def("ClearLuaKeyMappings", &InputBindingLayer::ClearLuaKeyMappings);
	defInLay.def(
	  "BindKey", +[](InputBindingLayer &layer, const std::string &key, const std::string &cmd) {
		  short c;
		  if(!StringToKey(key, &c))
			  return;
		  layer.MapKey(c, cmd);
		  return;
	  });
	defInLay.def(
	  "BindKey", +[](InputBindingLayer &layer, const std::string &key, luabind::object function) {
		  Lua::CheckType(function, Lua::Type::Function);
		  short c;
		  if(!StringToKey(key, &c))
			  return;
		  layer.MapKey(c, function);
	  });
	defInLay.def(
	  "UnbindKey", +[](InputBindingLayer &layer, const std::string &key) {
		  short c;
		  if(!StringToKey(key, &c))
			  return;
		  layer.UnmapKey(c);
	  });
	defInLay.def(
	  "FindBoundKeys", +[](const InputBindingLayer &layer, const std::string &cmd) -> std::vector<std::string> {
		  std::vector<std::string> boundKeys;
		  for(auto &pair : layer.GetKeyMappings()) {
			  if(pair.second.GetType() != KeyBind::Type::Regular)
				  continue;
			  if(ustring::compare(pair.second.GetBind(), cmd) == false)
				  continue;
			  std::string str;
			  if(!KeyToString(pair.first, &str))
				  continue;
			  boundKeys.push_back(str);
		  }
		  return boundKeys;
	  });
	inputMod[defInLay];
	pragma::lua::define_custom_constructor<InputBindingLayer,
	  +[](const std::string &name) -> std::shared_ptr<InputBindingLayer> {
		  auto layer = create_input_binding_layer();
		  layer->identifier = name;
		  return layer;
	  },
	  const std::string &>(lua.GetState());

	auto classDefAlEffect = luabind::class_<al::PEffect>("Effect");
	classDefAlEffect.def("SetProperties", static_cast<void (*)(lua_State *, al::PEffect &, const al::EfxEaxReverbProperties &)>(&Lua::ALEffect::SetProperties));
	classDefAlEffect.def("SetProperties", static_cast<void (*)(lua_State *, al::PEffect &, const al::EfxChorusProperties &)>(&Lua::ALEffect::SetProperties));
	classDefAlEffect.def("SetProperties", static_cast<void (*)(lua_State *, al::PEffect &, const al::EfxDistortionProperties &)>(&Lua::ALEffect::SetProperties));
	classDefAlEffect.def("SetProperties", static_cast<void (*)(lua_State *, al::PEffect &, const al::EfxEchoProperties &)>(&Lua::ALEffect::SetProperties));
	classDefAlEffect.def("SetProperties", static_cast<void (*)(lua_State *, al::PEffect &, const al::EfxFlangerProperties &)>(&Lua::ALEffect::SetProperties));
	classDefAlEffect.def("SetProperties", static_cast<void (*)(lua_State *, al::PEffect &, const al::EfxFrequencyShifterProperties &)>(&Lua::ALEffect::SetProperties));
	classDefAlEffect.def("SetProperties", static_cast<void (*)(lua_State *, al::PEffect &, const al::EfxVocalMorpherProperties &)>(&Lua::ALEffect::SetProperties));
	classDefAlEffect.def("SetProperties", static_cast<void (*)(lua_State *, al::PEffect &, const al::EfxPitchShifterProperties &)>(&Lua::ALEffect::SetProperties));
	classDefAlEffect.def("SetProperties", static_cast<void (*)(lua_State *, al::PEffect &, const al::EfxRingModulatorProperties &)>(&Lua::ALEffect::SetProperties));
	classDefAlEffect.def("SetProperties", static_cast<void (*)(lua_State *, al::PEffect &, const al::EfxAutoWahProperties &)>(&Lua::ALEffect::SetProperties));
	classDefAlEffect.def("SetProperties", static_cast<void (*)(lua_State *, al::PEffect &, const al::EfxCompressor &)>(&Lua::ALEffect::SetProperties));
	classDefAlEffect.def("SetProperties", static_cast<void (*)(lua_State *, al::PEffect &, const al::EfxEqualizer &)>(&Lua::ALEffect::SetProperties));

	auto classDefEaxReverb = luabind::class_<al::EfxEaxReverbProperties>("ReverbProperties");
	classDefEaxReverb.def(luabind::constructor<>());
	classDefEaxReverb.def_readwrite("density", &al::EfxEaxReverbProperties::flDensity);
	classDefEaxReverb.def_readwrite("diffusion", &al::EfxEaxReverbProperties::flDiffusion);
	classDefEaxReverb.def_readwrite("gain", &al::EfxEaxReverbProperties::flGain);
	classDefEaxReverb.def_readwrite("gainHF", &al::EfxEaxReverbProperties::flGainHF);
	classDefEaxReverb.def_readwrite("gainLF", &al::EfxEaxReverbProperties::flGainLF);
	classDefEaxReverb.def_readwrite("decayTime", &al::EfxEaxReverbProperties::flDecayTime);
	classDefEaxReverb.def_readwrite("decayHFRatio", &al::EfxEaxReverbProperties::flDecayHFRatio);
	classDefEaxReverb.def_readwrite("decayLFRatio", &al::EfxEaxReverbProperties::flDecayLFRatio);
	classDefEaxReverb.def_readwrite("reflectionsGain", &al::EfxEaxReverbProperties::flReflectionsGain);
	classDefEaxReverb.def_readwrite("reflectionsDelay", &al::EfxEaxReverbProperties::flReflectionsDelay);
	classDefEaxReverb.def_readwrite("reflectionsPan", reinterpret_cast<Vector3 al::EfxEaxReverbProperties::*>(&al::EfxEaxReverbProperties::flReflectionsPan));
	classDefEaxReverb.def_readwrite("lateReverbGain", &al::EfxEaxReverbProperties::flLateReverbGain);
	classDefEaxReverb.def_readwrite("lateReverbDelay", &al::EfxEaxReverbProperties::flLateReverbDelay);
	classDefEaxReverb.def_readwrite("lateReverbPan", reinterpret_cast<Vector3 al::EfxEaxReverbProperties::*>(&al::EfxEaxReverbProperties::flLateReverbPan));
	classDefEaxReverb.def_readwrite("echoTime", &al::EfxEaxReverbProperties::flEchoTime);
	classDefEaxReverb.def_readwrite("echoDepth", &al::EfxEaxReverbProperties::flEchoDepth);
	classDefEaxReverb.def_readwrite("modulationTime", &al::EfxEaxReverbProperties::flModulationTime);
	classDefEaxReverb.def_readwrite("modulationDepth", &al::EfxEaxReverbProperties::flModulationDepth);
	classDefEaxReverb.def_readwrite("airAbsorptionGainHF", &al::EfxEaxReverbProperties::flAirAbsorptionGainHF);
	classDefEaxReverb.def_readwrite("hfReference", &al::EfxEaxReverbProperties::flHFReference);
	classDefEaxReverb.def_readwrite("lfReference", &al::EfxEaxReverbProperties::flLFReference);
	classDefEaxReverb.def_readwrite("roomRolloffFactor", &al::EfxEaxReverbProperties::flRoomRolloffFactor);
	classDefEaxReverb.def_readwrite("decayHFLimit", &al::EfxEaxReverbProperties::iDecayHFLimit);
	classDefAlEffect.scope[classDefEaxReverb];

	auto classDefChorus = luabind::class_<al::EfxChorusProperties>("ChorusProperties");
	classDefChorus.def(luabind::constructor<>());
	classDefChorus.def_readwrite("waveform", &al::EfxChorusProperties::iWaveform);
	classDefChorus.def_readwrite("phase", &al::EfxChorusProperties::iPhase);
	classDefChorus.def_readwrite("rate", &al::EfxChorusProperties::flRate);
	classDefChorus.def_readwrite("depth", &al::EfxChorusProperties::flDepth);
	classDefChorus.def_readwrite("feedback", &al::EfxChorusProperties::flFeedback);
	classDefChorus.def_readwrite("delay", &al::EfxChorusProperties::flDelay);
	classDefAlEffect.scope[classDefChorus];

	auto classDefDistortionProperties = luabind::class_<al::EfxDistortionProperties>("DistortionProperties");
	classDefDistortionProperties.def(luabind::constructor<>());
	classDefDistortionProperties.def_readwrite("edge", &al::EfxDistortionProperties::flEdge);
	classDefDistortionProperties.def_readwrite("gain", &al::EfxDistortionProperties::flGain);
	classDefDistortionProperties.def_readwrite("lowpassCutoff", &al::EfxDistortionProperties::flLowpassCutoff);
	classDefDistortionProperties.def_readwrite("eqCenter", &al::EfxDistortionProperties::flEQCenter);
	classDefDistortionProperties.def_readwrite("eqBandwidth", &al::EfxDistortionProperties::flEQBandwidth);
	classDefAlEffect.scope[classDefDistortionProperties];

	auto classDefEchoProperties = luabind::class_<al::EfxEchoProperties>("EchoProperties");
	classDefEchoProperties.def(luabind::constructor<>());
	classDefEchoProperties.def_readwrite("delay", &al::EfxEchoProperties::flDelay);
	classDefEchoProperties.def_readwrite("lrDelay", &al::EfxEchoProperties::flLRDelay);
	classDefEchoProperties.def_readwrite("damping", &al::EfxEchoProperties::flDamping);
	classDefEchoProperties.def_readwrite("feedback", &al::EfxEchoProperties::flFeedback);
	classDefEchoProperties.def_readwrite("spread", &al::EfxEchoProperties::flSpread);
	classDefAlEffect.scope[classDefEchoProperties];

	auto classDefFlangerProperties = luabind::class_<al::EfxFlangerProperties>("FlangerProperties");
	classDefFlangerProperties.def(luabind::constructor<>());
	classDefFlangerProperties.def_readwrite("waveform", &al::EfxFlangerProperties::iWaveform);
	classDefFlangerProperties.def_readwrite("phase", &al::EfxFlangerProperties::iPhase);
	classDefFlangerProperties.def_readwrite("rate", &al::EfxFlangerProperties::flRate);
	classDefFlangerProperties.def_readwrite("depth", &al::EfxFlangerProperties::flDepth);
	classDefFlangerProperties.def_readwrite("feedback", &al::EfxFlangerProperties::flFeedback);
	classDefFlangerProperties.def_readwrite("delay", &al::EfxFlangerProperties::flDelay);
	classDefAlEffect.scope[classDefFlangerProperties];

	auto classDefFrequencyShifterProperties = luabind::class_<al::EfxFrequencyShifterProperties>("FrequencyShifterProperties");
	classDefFrequencyShifterProperties.def(luabind::constructor<>());
	classDefFrequencyShifterProperties.def_readwrite("frequency", &al::EfxFrequencyShifterProperties::flFrequency);
	classDefFrequencyShifterProperties.def_readwrite("leftDirection", &al::EfxFrequencyShifterProperties::iLeftDirection);
	classDefFrequencyShifterProperties.def_readwrite("rightDirection", &al::EfxFrequencyShifterProperties::iRightDirection);
	classDefAlEffect.scope[classDefFrequencyShifterProperties];

	auto classDefVocalMorpherProperties = luabind::class_<al::EfxVocalMorpherProperties>("VocalMorpherProperties");
	classDefVocalMorpherProperties.def(luabind::constructor<>());
	classDefVocalMorpherProperties.def_readwrite("phonemeA", &al::EfxVocalMorpherProperties::iPhonemeA);
	classDefVocalMorpherProperties.def_readwrite("phonemeB", &al::EfxVocalMorpherProperties::iPhonemeB);
	classDefVocalMorpherProperties.def_readwrite("phonemeACoarseTuning", &al::EfxVocalMorpherProperties::iPhonemeACoarseTuning);
	classDefVocalMorpherProperties.def_readwrite("phonemeBCoarseTuning", &al::EfxVocalMorpherProperties::iPhonemeBCoarseTuning);
	classDefVocalMorpherProperties.def_readwrite("waveform", &al::EfxVocalMorpherProperties::iWaveform);
	classDefVocalMorpherProperties.def_readwrite("rate", &al::EfxVocalMorpherProperties::flRate);
	classDefAlEffect.scope[classDefVocalMorpherProperties];

	auto classDefPitchShifterProperties = luabind::class_<al::EfxPitchShifterProperties>("PitchShifterProperties");
	classDefPitchShifterProperties.def(luabind::constructor<>());
	classDefPitchShifterProperties.def_readwrite("coarseTune", &al::EfxPitchShifterProperties::iCoarseTune);
	classDefPitchShifterProperties.def_readwrite("fineTune", &al::EfxPitchShifterProperties::iFineTune);
	classDefAlEffect.scope[classDefPitchShifterProperties];

	auto classDefRingModulatorProperties = luabind::class_<al::EfxRingModulatorProperties>("RingModulatorProperties");
	classDefRingModulatorProperties.def(luabind::constructor<>());
	classDefRingModulatorProperties.def_readwrite("frequency", &al::EfxRingModulatorProperties::flFrequency);
	classDefRingModulatorProperties.def_readwrite("highpassCutoff", &al::EfxRingModulatorProperties::flHighpassCutoff);
	classDefRingModulatorProperties.def_readwrite("waveform", &al::EfxRingModulatorProperties::iWaveform);
	classDefAlEffect.scope[classDefRingModulatorProperties];

	auto classDefAutoWahProperties = luabind::class_<al::EfxAutoWahProperties>("AutoWahProperties");
	classDefAutoWahProperties.def(luabind::constructor<>());
	classDefAutoWahProperties.def_readwrite("attackTime", &al::EfxAutoWahProperties::flAttackTime);
	classDefAutoWahProperties.def_readwrite("releaseTime", &al::EfxAutoWahProperties::flReleaseTime);
	classDefAutoWahProperties.def_readwrite("resonance", &al::EfxAutoWahProperties::flResonance);
	classDefAutoWahProperties.def_readwrite("peakGain", &al::EfxAutoWahProperties::flPeakGain);
	classDefAlEffect.scope[classDefAutoWahProperties];

	auto classDefCompressor = luabind::class_<al::EfxCompressor>("CompressorProperties");
	classDefCompressor.def(luabind::constructor<>());
	classDefCompressor.def_readwrite("onOff", &al::EfxCompressor::iOnOff);
	classDefAlEffect.scope[classDefCompressor];

	auto classDefEqualizer = luabind::class_<al::EfxEqualizer>("EqualizerProperties");
	classDefEqualizer.def(luabind::constructor<>());
	classDefEqualizer.def_readwrite("lowGain", &al::EfxEqualizer::flLowGain);
	classDefEqualizer.def_readwrite("lowCutoff", &al::EfxEqualizer::flLowCutoff);
	classDefEqualizer.def_readwrite("mid1Gain", &al::EfxEqualizer::flMid1Gain);
	classDefEqualizer.def_readwrite("mid1Center", &al::EfxEqualizer::flMid1Center);
	classDefEqualizer.def_readwrite("mid1Width", &al::EfxEqualizer::flMid1Width);
	classDefEqualizer.def_readwrite("mid2Gain", &al::EfxEqualizer::flMid2Gain);
	classDefEqualizer.def_readwrite("mid2Center", &al::EfxEqualizer::flMid2Center);
	classDefEqualizer.def_readwrite("mid2Width", &al::EfxEqualizer::flMid2Width);
	classDefEqualizer.def_readwrite("highGain", &al::EfxEqualizer::flHighGain);
	classDefEqualizer.def_readwrite("highCutoff", &al::EfxEqualizer::flHighCutoff);
	classDefAlEffect.scope[classDefEqualizer];

	auto alSoundClassDef = luabind::class_<ALSound>("Source");
	Lua::ALSound::Client::register_class(alSoundClassDef);

	auto alBufferClassDef = luabind::class_<al::ISoundBuffer>("Source");
	Lua::ALSound::Client::register_buffer(alBufferClassDef);

	auto soundMod = luabind::module(lua.GetState(), "sound");
	soundMod[classDefAlEffect];
	soundMod[alSoundClassDef];
	soundMod[alBufferClassDef];

	RegisterVulkanLuaInterface(lua);
}
