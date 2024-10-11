/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/game/c_game.h"
#include "pragma/util/c_util.hpp"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/lua/libraries/c_ldebugoverlay.h"
#include "pragma/lua/libraries/c_lgui.h"
#include "pragma/lua/libraries/c_lsound.h"
#include "pragma/lua/classes/c_lshader.h"
#include "pragma/lua/libraries/c_lutil.h"
#include "pragma/lua/libraries/c_linput.h"
#include "pragma/lua/libraries/lasset.hpp"
#include "pragma/lua/converters/gui_element_converter_t.hpp"
#include "pragma/entities/environment/c_env_camera.h"
#include "pragma/entities/environment/effects/c_env_particle_system.h"
#include "pragma/entities/components/lightmap_data_cache.hpp"
#include "pragma/input/input_binding_layer.hpp"
#include "pragma/audio/c_laleffect.h"
#include "pragma/audio/c_lalsound.hpp"
#include "pragma/gui/wiluabase.h"
#include "pragma/gui/wiconsole.hpp"
#include "pragma/lua/classes/c_lwibase.h"
#include "pragma/lua/classes/c_ldef_wgui.h"
#include "pragma/lua/libraries/c_limport.hpp"
#include "pragma/lua/policies/gui_element_policy.hpp"
#include "pragma/ai/c_lai.hpp"
#include "pragma/rendering/raytracing/cycles.hpp"
#include "pragma/rendering/shaders/c_shader_cubemap_to_equirectangular.hpp"
#include "pragma/rendering/shaders/c_shader_equirectangular_to_cubemap.hpp"
#include "pragma/asset/c_util_model.hpp"
#include <pragma/debug/debug_render_info.hpp>
#include <pragma/game/game_resources.hpp>
#include <pragma/util/giblet_create_info.hpp>
#include <pragma/lua/lua_entity_component.hpp>
#include <pragma/lua/classes/ldef_entity.h>
#include <pragma/lua/classes/thread_pool.hpp>
#include <pragma/lua/libraries/lfile.h>
#include <pragma/lua/libraries/lutil.hpp>
#include <pragma/lua/policies/default_parameter_policy.hpp>
#include <pragma/lua/policies/vector_policy.hpp>
#include <pragma/lua/policies/optional_policy.hpp>
#include <pragma/lua/converters/string_view_converter_t.hpp>
#include <pragma/lua/converters/vector_converter_t.hpp>
#include <pragma/lua/converters/pair_converter_t.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/lua/converters/optional_converter_t.hpp>
#include <pragma/lua/converters/thread_pool_converter_t.hpp>
#include <pragma/lua/policies/core_policies.hpp>
#include <pragma/lua/custom_constructor.hpp>
#include <pragma/lua/util_logging.hpp>
#include <pragma/model/modelmanager.h>
#include <pragma/input/inputhelper.h>
#include <sharedutils/util_file.h>
#include <sharedutils/util_path.hpp>
#include <util_image.hpp>
#include <util_image_buffer.hpp>
#include <util_texture_info.hpp>
#include <alsoundsystem.hpp>
#include <luainterface.hpp>
#include <cmaterialmanager.h>
#include <cmaterial_manager2.hpp>
#include <impl_texture_formats.h>
#include <prosper_window.hpp>
#include <prosper_command_buffer.hpp>
#include <prosper_prepared_command_buffer.hpp>
#include <luabind/copy_policy.hpp>
#include <fsys/ifile.hpp>
#include <wgui/types/witooltip.h>
#include <wgui/types/wiroot.h>

import pragma.string.unicode;
import pragma.audio.util;

extern DLLCLIENT CGame *c_game;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CEngine *c_engine;

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
	    "get_primary_monitor", +[]() -> GLFW::Monitor { return GLFW::get_primary_monitor(); }),
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
	  luabind::def("inject_mouse_input", static_cast<bool (*)(GLFW::MouseButton, GLFW::KeyState, GLFW::Modifier, const Vector2i &)>(&Lua::gui::inject_mouse_input)),
	  luabind::def("inject_mouse_input", static_cast<bool (*)(GLFW::MouseButton, GLFW::KeyState, GLFW::Modifier)>(&Lua::gui::inject_mouse_input)), luabind::def("inject_keyboard_input", Lua::gui::inject_keyboard_input), luabind::def("inject_char_input", Lua::gui::inject_char_input),
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
		    auto primaryMonitor = GLFW::get_primary_monitor();
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
	auto videoModeDef = luabind::class_<GLFW::Monitor::VideoMode>("VideoMode");
	videoModeDef.def(
	  "__tostring", +[](const GLFW::Monitor::VideoMode &videoMode) -> std::string {
		  std::stringstream ss;
		  ss << "VideoMode[resolution:";
		  ss << videoMode.width << "x" << videoMode.height << "][";
		  ss << "refreshRate:" << videoMode.refreshRate << "]";
		  return ss.str();
	  });
	videoModeDef.def_readonly("width", &GLFW::Monitor::VideoMode::width);
	videoModeDef.def_readonly("height", &GLFW::Monitor::VideoMode::height);
	videoModeDef.def_readonly("redBits", &GLFW::Monitor::VideoMode::redBits);
	videoModeDef.def_readonly("greenBits", &GLFW::Monitor::VideoMode::greenBits);
	videoModeDef.def_readonly("blueBits", &GLFW::Monitor::VideoMode::blueBits);
	videoModeDef.def_readonly("refreshRate", &GLFW::Monitor::VideoMode::refreshRate);
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
	wiTooltipClassDef.def(
	  "SetText", +[](WITooltip &elTooltip, const std::string &text) { elTooltip.SetText(text); });
	wiTooltipClassDef.def(
	  "GetText", +[](const WITooltip &elTooltip) { return elTooltip.GetText().cpp_str(); });
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

	auto wiRoundedRectClassDef = luabind::class_<WIRoundedRect, luabind::bases<WIShape, ::WIBase>>("RoundedRect");
	Lua::WIRoundedRect::register_class(wiRoundedRectClassDef);
	guiMod[wiRoundedRectClassDef];

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
	wiConsoleClassDef.def(
	  "GetText", +[](const ::WIConsole &console) { return console.GetText().cpp_str(); });
	wiConsoleClassDef.def("SetText", &WIConsole::SetText);
	wiConsoleClassDef.def("AppendText", &WIConsole::AppendText);
	wiConsoleClassDef.def("Clear", &WIConsole::Clear);
	wiConsoleClassDef.def("SetMaxLogLineCount", &WIConsole::SetMaxLogLineCount);
	wiConsoleClassDef.def("GetMaxLogLineCount", &WIConsole::GetMaxLogLineCount);
	guiMod[wiConsoleClassDef];

	guiMod[wiOutlinedRectClassDef];
	guiMod[wiLineClassDef];
	guiMod[wiScrollBarClassDef];
}

static std::vector<GLFW::Key> get_mapped_keys(const std::string &cvarName, uint32_t maxKeys = std::numeric_limits<uint32_t>::max())
{
	std::vector<GLFW::Key> mappedKeys;
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

void ClientState::RegisterSharedLuaLibraries(Lua::Interface &lua, bool bGUI)
{
	register_gui(lua);

	auto inputMod = luabind::module(lua.GetState(), "input");
	inputMod[luabind::def(
	           "get_mouse_button_state", +[](GLFW::MouseButton mouseButton) -> GLFW::KeyState { return c_engine->GetWindow()->GetMouseButtonState(mouseButton); }),
	  luabind::def(
	    "get_key_state", +[](GLFW::Key key) -> GLFW::KeyState { return c_engine->GetWindow()->GetKeyState(key); }),
	  luabind::def(
	    "get_cursor_pos", +[]() -> Vector2 { return c_engine->GetWindow()->GetCursorPos(); }),
	  luabind::def(
	    "set_cursor_pos", +[](const Vector2 &pos) { c_engine->GetWindow()->SetCursorPos(pos); }),
	  luabind::def(
	    "is_ctrl_key_down", +[]() -> bool { return c_engine->GetWindow()->GetKeyState(GLFW::Key::LeftControl) != GLFW::KeyState::Release || c_engine->GetWindow()->GetKeyState(GLFW::Key::RightControl) != GLFW::KeyState::Release; }),
	  luabind::def(
	    "is_alt_key_down", +[]() -> bool { return c_engine->GetWindow()->GetKeyState(GLFW::Key::LeftAlt) != GLFW::KeyState::Release || c_engine->GetWindow()->GetKeyState(GLFW::Key::RightAlt) != GLFW::KeyState::Release; }),
	  luabind::def(
	    "is_shift_key_down", +[]() -> bool { return c_engine->GetWindow()->GetKeyState(GLFW::Key::LeftShift) != GLFW::KeyState::Release || c_engine->GetWindow()->GetKeyState(GLFW::Key::RightShift) != GLFW::KeyState::Release; }),
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
	    "get_controller_count", +[]() -> uint32_t { return GLFW::get_joysticks().size(); }),
	  luabind::def("get_controller_name", &GLFW::get_joystick_name), luabind::def("get_controller_axes", &GLFW::get_joystick_axes), luabind::def("get_controller_buttons", &GLFW::get_joystick_buttons),
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
	  [](const std::string &name) -> std::shared_ptr<InputBindingLayer> {
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
static std::optional<std::string> find_asset_file(const std::string &name, pragma::asset::Type type)
{
	if(type == pragma::asset::Type::Texture) {
		TextureType type;
		auto found = false;
		auto filePath = translate_image_path(name, type, nullptr, &found);
		if(found == false)
			return {};
		::util::Path path {filePath};
		path.PopFront();
		return path.GetString();
	}
	return find_file(name, type);
}
static bool is_asset_loaded(NetworkState &nw, const std::string &name, pragma::asset::Type type)
{
	switch(type) {
	case pragma::asset::Type::Texture:
		{
			auto *asset = static_cast<msys::CMaterialManager &>(nw.GetMaterialManager()).GetTextureManager().FindCachedAsset(name);
			return asset != nullptr;
		}
	case pragma::asset::Type::ParticleSystem:
		{
			return pragma::CParticleSystemComponent::IsParticleFilePrecached(name);
		}
	}
	return is_loaded(nw, name, type);
}

static std::optional<uint32_t> save_image(lua_State *l, uimg::ImageBuffer &imgBuffer, std::string fileName, uimg::TextureInfo &imgWriteInfo, bool cubemap, const pragma::lua::LuaThreadWrapper &tw)
{
	if(Lua::file::validate_write_operation(l, fileName) == false)
		return {};
	auto pImgBuffer = imgBuffer.shared_from_this();
	auto task = [pImgBuffer, fileName = std::move(fileName), imgWriteInfo, cubemap]() -> pragma::lua::LuaThreadPool::ResultHandler {
		auto result = c_game->SaveImage(*pImgBuffer, fileName, imgWriteInfo, cubemap);
		return [result](lua_State *l) { luabind::object {l, result}.push(l); };
	};
	if(tw.IsPool())
		return tw.GetPool().AddTask(task);
	tw.GetTask()->AddSubTask(task);
	return {};
}

static bool save_image(lua_State *l, uimg::ImageBuffer &imgBuffer, std::string fileName, uimg::TextureInfo &imgWriteInfo, bool cubemap)
{
	if(Lua::file::validate_write_operation(l, fileName) == false)
		return false;
	return c_game->SaveImage(imgBuffer, fileName, imgWriteInfo, cubemap);
}

static std::vector<std::string> &get_image_file_extensions()
{
	static std::vector<std::string> exts;
	if(exts.empty()) {
		exts.reserve(umath::to_integral(uimg::ImageFormat::Count));
		auto n = umath::to_integral(uimg::ImageFormat::Count);
		for(auto i = decltype(n) {0u}; i < n; ++i)
			exts.push_back(uimg::get_file_extension(static_cast<uimg::ImageFormat>(i)));
	}
	return exts;
}

static std::pair<bool, std::optional<std::string>> save_image(lua_State *l, uimg::ImageBuffer &imgBuffer, std::string fileName, uimg::ImageFormat format, float quality, pragma::lua::LuaThreadWrapper *tw = nullptr)
{
	if(Lua::file::validate_write_operation(l, fileName) == false)
		return std::pair<bool, std::optional<std::string>> {false, {}};
	ufile::remove_extension_from_filename(fileName, get_image_file_extensions());
	fileName += '.' + uimg::get_file_extension(format);
	auto fp = filemanager::open_file<VFilePtrReal>(fileName, filemanager::FileMode::Write | filemanager::FileMode::Binary);
	if(!fp)
		return std::pair<bool, std::optional<std::string>> {false, {}};
	if(tw) {
		auto pImgBuffer = imgBuffer.shared_from_this();
		auto task = [fp, pImgBuffer, format, quality]() -> pragma::lua::LuaThreadPool::ResultHandler {
			fsys::File f {fp};
			auto result = uimg::save_image(f, *pImgBuffer, format, quality);
			return [result](lua_State *l) { luabind::object {l, result}.push(l); };
		};
		if(tw->IsPool())
			tw->GetPool().AddTask(task);
		else
			tw->GetTask()->AddSubTask(task);
		return std::pair<bool, std::optional<std::string>> {true, fileName};
	}
	fsys::File f {fp};
	return std::pair<bool, std::optional<std::string>> {true, fileName};
}
static std::pair<bool, std::optional<std::string>> save_image(lua_State *l, uimg::ImageBuffer &imgBuffer, std::string fileName, uimg::ImageFormat format) { return save_image(l, imgBuffer, fileName, format, 1.f); }
static bool save_image(lua_State *l, luabind::table<> t, std::string fileName, uimg::TextureInfo &texInfo, bool cubemap)
{
	auto n = Lua::GetObjectLength(l, t);
	std::vector<std::shared_ptr<uimg::ImageBuffer>> imgBufs;
	imgBufs.reserve(n);
	auto o = luabind::object {luabind::from_stack(l, 1)};
	uint32_t maxWidth = 0;
	uint32_t maxHeight = 0;
	for(luabind::iterator it {o}, end; it != end; ++it) {
		auto val = *it;
		auto *imgBuf = luabind::object_cast<uimg::ImageBuffer *>(val);
		imgBufs.push_back(imgBuf->shared_from_this());
		maxWidth = umath::max(maxWidth, imgBuf->GetWidth());
		maxHeight = umath::max(maxHeight, imgBuf->GetHeight());
	}
	for(auto &imgBuf : imgBufs)
		imgBuf->Resize(maxWidth, maxHeight);
	if(imgBufs.empty())
		return false;
	auto &imgBuf = imgBufs.front();
	uimg::TextureSaveInfo saveInfo {};
	saveInfo.texInfo = texInfo;
	saveInfo.width = imgBuf->GetWidth();
	saveInfo.height = imgBuf->GetHeight();
	saveInfo.szPerPixel = imgBuf->GetPixelSize();
	saveInfo.numLayers = imgBufs.size();
	saveInfo.numMipmaps = 0;
	saveInfo.cubemap = cubemap;
	return uimg::save_texture(
	  fileName,
	  [&imgBufs](uint32_t iLayer, uint32_t iMipmap, std::function<void(void)> &outDeleter) -> const uint8_t * {
		  if(iMipmap > 0)
			  return nullptr;
		  return static_cast<uint8_t *>(imgBufs.at(iLayer)->GetData());
	  },
	  saveInfo);
}
static bool save_image(lua_State *l, luabind::table<> t, std::string fileName, uimg::TextureInfo &texInfo) { return save_image(l, t, fileName, texInfo, false); }

static bool save_image(lua_State *l, prosper::IImage &img, std::string fileName, uimg::TextureInfo &imgWriteInfo)
{
	if(Lua::file::validate_write_operation(l, fileName) == false)
		return false;
	return c_game->SaveImage(img, fileName, imgWriteInfo);
}

static luabind::object load_image(lua_State *l, const std::string &fileName, bool loadAsynch, const std::optional<uimg::Format> &targetFormat)
{
	std::string ext;
	if(ufile::get_extension(fileName, &ext) == false)
		return {};
	auto fp = FileManager::OpenFile<VFilePtrReal>(fileName.c_str(), "rb");
	if(fp == nullptr)
		return {};
	auto pixelFormat = uimg::PixelFormat::LDR;
	if(ustring::compare<std::string>(ext, "hdr"))
		pixelFormat = uimg::PixelFormat::Float;

	if(loadAsynch) {
		class ImageLoadJob : public util::ParallelWorker<std::shared_ptr<uimg::ImageBuffer>> {
		  public:
			ImageLoadJob(VFilePtr fp, uimg::PixelFormat pixelFormat, std::optional<uimg::Format> targetFormat)
			{
				AddThread([this, fp, pixelFormat, targetFormat]() {
					fsys::File f {fp};
					m_imgBuffer = uimg::load_image(f, pixelFormat);
					if(m_imgBuffer == nullptr) {
						SetStatus(util::JobStatus::Failed, "Unable to open image!");
						UpdateProgress(1.f);
						return;
					}
					if(targetFormat.has_value()) {
						if(IsCancelled())
							return;
						UpdateProgress(0.9f);
						m_imgBuffer->Convert(*targetFormat);
					}
					UpdateProgress(1.f);
				});
			}

			virtual std::shared_ptr<uimg::ImageBuffer> GetResult() override { return m_imgBuffer; }
		  private:
			std::shared_ptr<uimg::ImageBuffer> m_imgBuffer = nullptr;
		};
		return {l, util::create_parallel_job<ImageLoadJob>(fp, pixelFormat, targetFormat)};
	}
	fsys::File f {fp};
	auto imgBuffer = uimg::load_image(f, pixelFormat);
	if(imgBuffer == nullptr)
		return {};
	if(targetFormat.has_value())
		imgBuffer->Convert(*targetFormat);
	return {l, imgBuffer};
}

static luabind::object load_image(lua_State *l, const std::string &fileName, bool loadAsynch, uimg::Format targetFormat) { return load_image(l, fileName, loadAsynch, std::optional<uimg::Format> {targetFormat}); }

static luabind::object load_image(lua_State *l, const std::string &fileName, bool loadAsynch) { return load_image(l, fileName, loadAsynch, std::optional<uimg::Format> {}); }

static luabind::object load_image(lua_State *l, const std::string &fileName) { return load_image(l, fileName, false); }

static util::ParallelJob<uimg::ImageLayerSet> capture_raytraced_screenshot(lua_State *l, uint32_t width, uint32_t height, uint32_t samples, bool hdrOutput, bool denoise)
{
	pragma::rendering::cycles::RenderImageInfo renderImgInfo {};
	auto *pCam = c_game->GetRenderCamera();
	if(pCam) {
		renderImgInfo.camPose = pCam->GetEntity().GetPose();
		renderImgInfo.viewProjectionMatrix = pCam->GetProjectionMatrix() * pCam->GetViewMatrix();
		renderImgInfo.nearZ = pCam->GetNearZ();
		renderImgInfo.farZ = pCam->GetFarZ();
		renderImgInfo.fov = pCam->GetFOV();
	}
	pragma::rendering::cycles::SceneInfo sceneInfo {};
	sceneInfo.width = width;
	sceneInfo.height = height;
	sceneInfo.samples = samples;
	sceneInfo.denoise = denoise;
	sceneInfo.hdrOutput = hdrOutput;
	return pragma::rendering::cycles::render_image(*client, sceneInfo, renderImgInfo);
}
static util::ParallelJob<uimg::ImageLayerSet> capture_raytraced_screenshot(lua_State *l, uint32_t width, uint32_t height, uint32_t samples, bool hdrOutput) { return capture_raytraced_screenshot(l, width, height, samples, hdrOutput, true); }
static util::ParallelJob<uimg::ImageLayerSet> capture_raytraced_screenshot(lua_State *l, uint32_t width, uint32_t height, uint32_t samples) { return capture_raytraced_screenshot(l, width, height, samples, false, true); }
static util::ParallelJob<uimg::ImageLayerSet> capture_raytraced_screenshot(lua_State *l, uint32_t width, uint32_t height) { return capture_raytraced_screenshot(l, width, height, 1'024, false, true); }

static bool asset_import(NetworkState &nw, const std::string &name, const std::string &outputName, pragma::asset::Type type)
{
	if(type == pragma::asset::Type::Map)
		return util::port_hl2_map(&nw, name);
	auto *manager = nw.GetAssetManager(type);
	if(!manager)
		return false;
	return manager->Import(name, outputName);
}

void CGame::RegisterLuaLibraries()
{
	Lua::util::register_library(GetLuaState());

	auto osMod = luabind::module(GetLuaState(), "os");
	Lua::util::register_os(GetLuaState(), osMod);

	auto utilMod = luabind::module(GetLuaState(), "util");
	Lua::util::register_shared(GetLuaState(), utilMod);
	utilMod[luabind::def("calc_world_direction_from_2d_coordinates", Lua::util::calc_world_direction_from_2d_coordinates), luabind::def("calc_world_direction_from_2d_coordinates", Lua::util::Client::calc_world_direction_from_2d_coordinates),
	  luabind::def("create_particle_tracer", Lua::util::Client::create_particle_tracer), luabind::def("create_muzzle_flash", Lua::util::Client::create_muzzle_flash), luabind::def("fire_bullets", static_cast<luabind::object (*)(lua_State *, BulletInfo &)>(Lua::util::fire_bullets)),
	  luabind::def("save_image", static_cast<bool (*)(lua_State *, uimg::ImageBuffer &, std::string, uimg::TextureInfo &, bool)>(save_image)),
	  luabind::def("save_image", static_cast<bool (*)(lua_State *, uimg::ImageBuffer &, std::string, uimg::TextureInfo &, bool)>(save_image), luabind::default_parameter_policy<5, false> {}),
	  luabind::def(
	    "save_image", +[](lua_State *l, uimg::ImageBuffer &imgBuffer, std::string fileName, uimg::TextureInfo &imgWriteInfo, const pragma::lua::LuaThreadWrapper &tw) { return save_image(l, imgBuffer, fileName, imgWriteInfo, false, tw); }),
	  luabind::def(
	    "save_image", +[](lua_State *l, uimg::ImageBuffer &imgBuffer, std::string fileName, uimg::ImageFormat format, float quality) { return save_image(l, imgBuffer, fileName, format, quality); }),
	  luabind::def(
	    "save_image", +[](lua_State *l, uimg::ImageBuffer &imgBuffer, std::string fileName, uimg::ImageFormat format, float quality, const pragma::lua::LuaThreadWrapper &tw) { return save_image(l, imgBuffer, fileName, format, quality, const_cast<pragma::lua::LuaThreadWrapper *>(&tw)); }),
	  luabind::def("save_image", static_cast<std::pair<bool, std::optional<std::string>> (*)(lua_State *, uimg::ImageBuffer &, std::string, uimg::ImageFormat)>(save_image)),
	  luabind::def("save_image", static_cast<bool (*)(lua_State *, luabind::table<>, std::string, uimg::TextureInfo &, bool)>(save_image)), luabind::def("save_image", static_cast<bool (*)(lua_State *, luabind::table<>, std::string, uimg::TextureInfo &)>(save_image)),
	  luabind::def("save_image", static_cast<bool (*)(lua_State *, prosper::IImage &, std::string, uimg::TextureInfo &)>(save_image)), luabind::def("load_image", static_cast<luabind::object (*)(lua_State *, const std::string &, bool, uimg::Format)>(load_image)),
	  luabind::def("load_image", static_cast<luabind::object (*)(lua_State *, const std::string &, bool)>(load_image)), luabind::def("load_image", static_cast<luabind::object (*)(lua_State *, const std::string &)>(load_image)), luabind::def("screenshot", ::util::screenshot),
	  luabind::def("capture_raytraced_screenshot", static_cast<util::ParallelJob<uimg::ImageLayerSet> (*)(lua_State *, uint32_t, uint32_t, uint32_t, bool, bool)>(capture_raytraced_screenshot)),
	  luabind::def("capture_raytraced_screenshot", static_cast<util::ParallelJob<uimg::ImageLayerSet> (*)(lua_State *, uint32_t, uint32_t, uint32_t, bool)>(capture_raytraced_screenshot)),
	  luabind::def("capture_raytraced_screenshot", static_cast<util::ParallelJob<uimg::ImageLayerSet> (*)(lua_State *, uint32_t, uint32_t, uint32_t)>(capture_raytraced_screenshot)),
	  luabind::def("capture_raytraced_screenshot", static_cast<util::ParallelJob<uimg::ImageLayerSet> (*)(lua_State *, uint32_t, uint32_t)>(capture_raytraced_screenshot)),
	  luabind::def(
	    "cubemap_to_equirectangular_texture",
	    +[](lua_State *l, prosper::Texture &cubemap) -> luabind::object {
		    auto *shader = static_cast<pragma::ShaderCubemapToEquirectangular *>(c_engine->GetShader("cubemap_to_equirectangular").get());
		    if(shader == nullptr)
			    return {};
		    auto equiRect = shader->CubemapToEquirectangularTexture(cubemap);
		    if(equiRect == nullptr)
			    return {};
		    return {l, equiRect};
	    }),
	  luabind::def(
	    "equirectangular_to_cubemap_texture", +[](lua_State *l, prosper::Texture &equiRect, uint32_t resolution) -> luabind::object {
		    auto *shader = static_cast<pragma::ShaderEquirectangularToCubemap *>(c_engine->GetShader("equirectangular_to_cubemap").get());
		    if(shader == nullptr)
			    return {};
		    auto tex = shader->EquirectangularTextureToCubemap(equiRect, resolution);
		    if(tex == nullptr)
			    return {};
		    return {l, tex};
	    })];
	utilMod[
	  // luabind::def("fire_bullets",static_cast<int32_t(*)(lua_State*)>(Lua::util::fire_bullets)),
	  luabind::def("get_clipboard_string", Lua::util::Client::get_clipboard_string), luabind::def("set_clipboard_string", Lua::util::Client::set_clipboard_string), luabind::def("create_giblet", Lua::util::Client::create_giblet),
	  luabind::def("get_image_format_file_extension", uimg::get_file_extension), luabind::def("bake_directional_lightmap_atlas", Lua::util::Client::bake_directional_lightmap_atlas)];

	auto imgWriteInfoDef = luabind::class_<uimg::TextureInfo>("TextureInfo");
	imgWriteInfoDef.def(luabind::constructor<>());
	imgWriteInfoDef.add_static_constant("INPUT_FORMAT_KEEP_INPUT_IMAGE_FORMAT", umath::to_integral(uimg::TextureInfo::InputFormat::KeepInputImageFormat));
	imgWriteInfoDef.add_static_constant("INPUT_FORMAT_R16G16B16A16_FLOAT", umath::to_integral(uimg::TextureInfo::InputFormat::R16G16B16A16_Float));
	imgWriteInfoDef.add_static_constant("INPUT_FORMAT_R32G32B32A32_FLOAT", umath::to_integral(uimg::TextureInfo::InputFormat::R32G32B32A32_Float));
	imgWriteInfoDef.add_static_constant("INPUT_FORMAT_R32_FLOAT", umath::to_integral(uimg::TextureInfo::InputFormat::R32_Float));
	imgWriteInfoDef.add_static_constant("INPUT_FORMAT_R8G8B8A8_UINT", umath::to_integral(uimg::TextureInfo::InputFormat::R8G8B8A8_UInt));

	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_KEEP_INPUT_IMAGE_FORMAT", umath::to_integral(uimg::TextureInfo::OutputFormat::KeepInputImageFormat));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_RGB", umath::to_integral(uimg::TextureInfo::OutputFormat::RGB));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_RGBA", umath::to_integral(uimg::TextureInfo::OutputFormat::RGBA));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_DXT1", umath::to_integral(uimg::TextureInfo::OutputFormat::DXT1));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_DXT1A", umath::to_integral(uimg::TextureInfo::OutputFormat::DXT1a));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_DXT3", umath::to_integral(uimg::TextureInfo::OutputFormat::DXT3));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_DXT5", umath::to_integral(uimg::TextureInfo::OutputFormat::DXT5));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_DXT5N", umath::to_integral(uimg::TextureInfo::OutputFormat::DXT5n));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_BC1", umath::to_integral(uimg::TextureInfo::OutputFormat::BC1));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_BC1A", umath::to_integral(uimg::TextureInfo::OutputFormat::BC1a));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_BC2", umath::to_integral(uimg::TextureInfo::OutputFormat::BC2));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_BC3", umath::to_integral(uimg::TextureInfo::OutputFormat::BC3));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_BC3N", umath::to_integral(uimg::TextureInfo::OutputFormat::BC3n));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_BC4", umath::to_integral(uimg::TextureInfo::OutputFormat::BC4));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_BC5", umath::to_integral(uimg::TextureInfo::OutputFormat::BC5));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_DXT1N", umath::to_integral(uimg::TextureInfo::OutputFormat::DXT1n));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_CTX1", umath::to_integral(uimg::TextureInfo::OutputFormat::CTX1));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_BC6", umath::to_integral(uimg::TextureInfo::OutputFormat::BC6));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_BC7", umath::to_integral(uimg::TextureInfo::OutputFormat::BC7));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_BC3_RGBM", umath::to_integral(uimg::TextureInfo::OutputFormat::BC3_RGBM));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_ETC1", umath::to_integral(uimg::TextureInfo::OutputFormat::ETC1));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_ETC2_R", umath::to_integral(uimg::TextureInfo::OutputFormat::ETC2_R));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_ETC2_RG", umath::to_integral(uimg::TextureInfo::OutputFormat::ETC2_RG));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_ETC2_RGB", umath::to_integral(uimg::TextureInfo::OutputFormat::ETC2_RGB));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_ETC2_RGBA", umath::to_integral(uimg::TextureInfo::OutputFormat::ETC2_RGBA));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_ETC2_RGB_A1", umath::to_integral(uimg::TextureInfo::OutputFormat::ETC2_RGB_A1));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_ETC2_RGBM", umath::to_integral(uimg::TextureInfo::OutputFormat::ETC2_RGBM));

	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_COLOR_MAP", umath::to_integral(uimg::TextureInfo::OutputFormat::ColorMap));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_COLOR_MAP_1BIT_ALPHA", umath::to_integral(uimg::TextureInfo::OutputFormat::ColorMap1BitAlpha));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_COLOR_MAP_SHARP_ALPHA", umath::to_integral(uimg::TextureInfo::OutputFormat::ColorMapSharpAlpha));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_COLOR_MAP_SMOOTH_ALPHA", umath::to_integral(uimg::TextureInfo::OutputFormat::ColorMapSmoothAlpha));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_NORMAL_MAP", umath::to_integral(uimg::TextureInfo::OutputFormat::NormalMap));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_HDR_COLOR_MAP", umath::to_integral(uimg::TextureInfo::OutputFormat::HDRColorMap));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_GRADIENT_MAP", umath::to_integral(uimg::TextureInfo::OutputFormat::GradientMap));

	imgWriteInfoDef.add_static_constant("CONTAINER_FORMAT_DDS", umath::to_integral(uimg::TextureInfo::ContainerFormat::DDS));
	imgWriteInfoDef.add_static_constant("CONTAINER_FORMAT_KTX", umath::to_integral(uimg::TextureInfo::ContainerFormat::KTX));

	imgWriteInfoDef.add_static_constant("FLAG_NONE", umath::to_integral(uimg::TextureInfo::Flags::None));
	imgWriteInfoDef.add_static_constant("FLAG_BIT_CONVERT_TO_NORMAL_MAP", umath::to_integral(uimg::TextureInfo::Flags::ConvertToNormalMap));
	imgWriteInfoDef.add_static_constant("FLAG_BIT_SRGB", umath::to_integral(uimg::TextureInfo::Flags::SRGB));
	imgWriteInfoDef.add_static_constant("FLAG_BIT_GENERATE_MIPMAPS", umath::to_integral(uimg::TextureInfo::Flags::GenerateMipmaps));

	imgWriteInfoDef.add_static_constant("MIPMAP_FILTER_BOX", umath::to_integral(uimg::TextureInfo::MipmapFilter::Box));
	imgWriteInfoDef.add_static_constant("MIPMAP_FILTER_KAISER", umath::to_integral(uimg::TextureInfo::MipmapFilter::Kaiser));

	imgWriteInfoDef.add_static_constant("WRAP_MODE_CLAMP", umath::to_integral(uimg::TextureInfo::WrapMode::Clamp));
	imgWriteInfoDef.add_static_constant("WRAP_MODE_REPEAT", umath::to_integral(uimg::TextureInfo::WrapMode::Repeat));
	imgWriteInfoDef.add_static_constant("WRAP_MODE_MIRROR", umath::to_integral(uimg::TextureInfo::WrapMode::Mirror));

	imgWriteInfoDef.def_readwrite("inputFormat", reinterpret_cast<std::underlying_type_t<decltype(uimg::TextureInfo::inputFormat)> uimg::TextureInfo::*>(&uimg::TextureInfo::inputFormat));
	imgWriteInfoDef.def_readwrite("outputFormat", reinterpret_cast<std::underlying_type_t<decltype(uimg::TextureInfo::outputFormat)> uimg::TextureInfo::*>(&uimg::TextureInfo::outputFormat));
	imgWriteInfoDef.def_readwrite("containerFormat", reinterpret_cast<std::underlying_type_t<decltype(uimg::TextureInfo::containerFormat)> uimg::TextureInfo::*>(&uimg::TextureInfo::containerFormat));
	imgWriteInfoDef.def_readwrite("flags", reinterpret_cast<std::underlying_type_t<decltype(uimg::TextureInfo::flags)> uimg::TextureInfo::*>(&uimg::TextureInfo::flags));
	imgWriteInfoDef.def_readwrite("mipMapFilter", reinterpret_cast<std::underlying_type_t<decltype(uimg::TextureInfo::mipMapFilter)> uimg::TextureInfo::*>(&uimg::TextureInfo::mipMapFilter));
	imgWriteInfoDef.def_readwrite("wrapMode", reinterpret_cast<std::underlying_type_t<decltype(uimg::TextureInfo::wrapMode)> uimg::TextureInfo::*>(&uimg::TextureInfo::wrapMode));

	imgWriteInfoDef.def("SetNormalMap", static_cast<void (*)(lua_State *, uimg::TextureInfo &)>([](lua_State *l, uimg::TextureInfo &writeInfo) { writeInfo.SetNormalMap(); }));
	utilMod[imgWriteInfoDef];

	Lua::RegisterLibraryEnums(GetLuaState(), "util",
	  {{"IMAGE_FORMAT_PNG", umath::to_integral(uimg::ImageFormat::PNG)}, {"IMAGE_FORMAT_BMP", umath::to_integral(uimg::ImageFormat::BMP)}, {"IMAGE_FORMAT_TGA", umath::to_integral(uimg::ImageFormat::TGA)}, {"IMAGE_FORMAT_JPG", umath::to_integral(uimg::ImageFormat::JPG)},
	    {"IMAGE_FORMAT_HDR", umath::to_integral(uimg::ImageFormat::HDR)}, {"IMAGE_FORMAT_COUNT", umath::to_integral(uimg::ImageFormat::Count)},

	    {"PIXEL_FORMAT_LDR", umath::to_integral(uimg::PixelFormat::LDR)}, {"PIXEL_FORMAT_HDR", umath::to_integral(uimg::PixelFormat::HDR)}, {"PIXEL_FORMAT_FLOAT", umath::to_integral(uimg::PixelFormat::Float)}});

	Lua::ai::client::register_library(GetLuaInterface());

	Game::RegisterLuaLibraries();
	ClientState::RegisterSharedLuaLibraries(GetLuaInterface());

	auto consoleMod = luabind::module(GetLuaState(), "console");
	consoleMod[luabind::def(
	  "save_config", +[](CEngine &engine) { engine.SaveClientConfig(); })];

	GetLuaInterface().RegisterLibrary("asset",
	  {{"export_map", Lua::util::Client::export_map}, {"import_model", Lua::util::Client::import_model}, {"import_gltf", Lua::util::Client::import_gltf}, {"export_texture", Lua::util::Client::export_texture}, {"export_material", Lua::util::Client::export_material},
	    {"export_texture_as_vtf", static_cast<int32_t (*)(lua_State *)>([](lua_State *l) -> int32_t {
		     std::string fileName = Lua::CheckString(l, 1);
		     if(Lua::file::validate_write_operation(l, fileName) == false || FileManager::CreatePath(ufile::get_path_from_filename(fileName).c_str()) == false) {
			     Lua::PushBool(l, false);
			     return 1;
		     }

		     auto &img = Lua::Check<prosper::IImage>(l, 2);
		     auto vtfOutputFormat = pragma::asset::prosper_format_to_vtf(img.GetFormat());
		     auto srgb = true;
		     auto normalMap = false;
		     auto generateMipmaps = false;
		     int32_t arg = 3;

		     if(Lua::IsSet(l, arg))
			     srgb = Lua::CheckBool(l, arg);
		     ++arg;

		     if(Lua::IsSet(l, arg))
			     normalMap = Lua::CheckBool(l, arg);
		     ++arg;

		     if(Lua::IsSet(l, arg))
			     generateMipmaps = Lua::CheckBool(l, arg);
		     ++arg;

		     if(Lua::IsSet(l, arg))
			     vtfOutputFormat = pragma::asset::prosper_format_to_vtf(static_cast<prosper::Format>(Lua::CheckInt(l, arg)));
		     ++arg;
		     if(vtfOutputFormat.has_value() == false) {
			     Lua::PushBool(l, false);
			     return 1;
		     }

		     pragma::asset::VtfInfo vtfInfo {};
		     vtfInfo.outputFormat = *vtfOutputFormat;
		     umath::set_flag(vtfInfo.flags, pragma::asset::VtfInfo::Flags::Srgb, srgb);
		     umath::set_flag(vtfInfo.flags, pragma::asset::VtfInfo::Flags::NormalMap, normalMap);
		     umath::set_flag(vtfInfo.flags, pragma::asset::VtfInfo::Flags::GenerateMipmaps, generateMipmaps);
		     auto result = pragma::asset::export_texture_as_vtf(fileName, img, vtfInfo, nullptr, false);
		     Lua::PushBool(l, result);
		     return 1;
	     })},
	    {"exists", static_cast<int32_t (*)(lua_State *)>([](lua_State *l) -> int32_t {
		     std::string name = Lua::CheckString(l, 1);
		     auto type = static_cast<pragma::asset::Type>(Lua::CheckInt(l, 2));
		     auto *nw = c_engine->GetNetworkState(l);
		     auto fileName = find_asset_file(name, type);
		     Lua::PushBool(l, fileName.has_value());
		     return 1;
	     })},
	    {"find_file", static_cast<int32_t (*)(lua_State *)>([](lua_State *l) -> int32_t {
		     std::string name = Lua::CheckString(l, 1);
		     auto type = static_cast<pragma::asset::Type>(Lua::CheckInt(l, 2));
		     auto *nw = c_engine->GetNetworkState(l);
		     auto fileName = find_asset_file(name, type);
		     if(fileName.has_value() == false)
			     return 0;
		     Lua::PushString(l, *fileName);
		     return 1;
	     })},
	    {"is_loaded", static_cast<int32_t (*)(lua_State *)>([](lua_State *l) -> int32_t {
		     std::string name = Lua::CheckString(l, 1);
		     auto type = static_cast<pragma::asset::Type>(Lua::CheckInt(l, 2));
		     auto *nw = c_engine->GetNetworkState(l);
		     Lua::PushBool(l, is_asset_loaded(*nw, name, type));
		     return 1;
	     })},
	    {"import_texture", static_cast<int32_t (*)(lua_State *)>([](lua_State *l) -> int32_t {
		     auto &texImportInfo = Lua::Check<::pragma::asset::TextureImportInfo>(l, 2);
		     std::string outputPath = Lua::CheckString(l, 3);
		     auto result = false;
		     std::string errMsg;
		     if(Lua::IsString(l, 1)) {
			     std::string name = Lua::CheckString(l, 1);
			     result = pragma::asset::import_texture(name, texImportInfo, outputPath, errMsg);
		     }
		     else if(Lua::IsFile(l, 1)) {
			     auto f = Lua::CheckFile(l, 1);
			     auto fp = std::make_unique<ufile::FileWrapper>(f->GetHandle());
			     result = pragma::asset::import_texture(std::move(fp), texImportInfo, outputPath, errMsg);
		     }
		     else {
			     auto &img = Lua::Check<prosper::IImage>(l, 1);
			     result = pragma::asset::import_texture(img, texImportInfo, outputPath, errMsg);
		     }
		     Lua::PushBool(l, result);
		     if(result == false) {
			     Lua::PushString(l, errMsg);
			     return 2;
		     }
		     return 1;
	     })}});

	auto modAsset = luabind::module_(GetLuaState(), "asset");
	modAsset[luabind::def("clear_unused_textures", static_cast<uint32_t (*)()>([]() -> uint32_t { return static_cast<msys::CMaterialManager &>(client->GetMaterialManager()).GetTextureManager().ClearUnused(); })),

	  luabind::def(
	    "load",
	    +[](lua_State *l, LFile &f, pragma::asset::Type type) -> Lua::var<bool, luabind::object> {
		    // See also core/server/src/lua/library.cpp
		    auto *manager = pragma::get_engine()->GetNetworkState(l)->GetAssetManager(type);
		    if(!manager)
			    return luabind::object {l, false};
		    auto fh = f.GetHandle();
		    if(!fh)
			    return luabind::object {l, false};
		    auto fp = std::make_unique<ufile::FileWrapper>(fh);
		    auto fileName = fp->GetFileName();
		    if(!fileName.has_value())
			    return luabind::object {l, false};
		    std::string ext;
		    if(ufile::get_extension(*fileName, &ext) == false)
			    return luabind::object {l, false};
		    auto loadInfo = manager->CreateDefaultLoadInfo();
		    loadInfo->flags |= util::AssetLoadFlags::DontCache | util::AssetLoadFlags::IgnoreCache;
		    auto asset = manager->LoadAsset(ufile::get_file_from_filename(*fileName), std::move(fp), ext, std::move(loadInfo));
		    switch(type) {
		    case pragma::asset::Type::Model:
			    return luabind::object {l, std::static_pointer_cast<Model>(asset)};
		    case pragma::asset::Type::Material:
			    return luabind::object {l, std::static_pointer_cast<Material>(asset)};
		    case pragma::asset::Type::Texture:
			    return luabind::object {l, std::static_pointer_cast<Texture>(asset)};
		    }
		    return luabind::object {};
	    }),
	  luabind::def(
	    "load",
	    +[](lua_State *l, const std::string &name, pragma::asset::Type type) -> Lua::var<bool, luabind::object> {
		    // See also core/server/src/lua/library.cpp
		    auto *manager = pragma::get_engine()->GetNetworkState(l)->GetAssetManager(type);
		    if(!manager)
			    return luabind::object {l, false};
		    auto asset = manager->LoadAsset(name);
		    switch(type) {
		    case pragma::asset::Type::Model:
			    return luabind::object {l, std::static_pointer_cast<Model>(asset)};
		    case pragma::asset::Type::Material:
			    return luabind::object {l, std::static_pointer_cast<Material>(asset)};
		    case pragma::asset::Type::Texture:
			    return luabind::object {l, std::static_pointer_cast<Texture>(asset)};
		    }
		    return luabind::object {};
	    }),
	  luabind::def(
	    "reload",
	    +[](lua_State *l, const std::string &name, pragma::asset::Type type) -> Lua::var<bool, luabind::object> {
		    auto *manager = c_engine->GetNetworkState(l)->GetAssetManager(type);
		    if(!manager)
			    return luabind::object {l, false};
		    auto asset = manager->ReloadAsset(name);
		    switch(type) {
		    case pragma::asset::Type::Model:
			    return luabind::object {l, std::static_pointer_cast<Model>(asset)};
		    case pragma::asset::Type::Material:
			    return luabind::object {l, std::static_pointer_cast<Material>(asset)};
		    case pragma::asset::Type::Texture:
			    return luabind::object {l, std::static_pointer_cast<Texture>(asset)};
		    }
		    return luabind::object {};
	    }),
	  luabind::def(
	    "import", +[](NetworkState &nw, const std::string &name, pragma::asset::Type type) -> bool { return asset_import(nw, name, name, type); }),
	  luabind::def(
	    "import", +[](NetworkState &nw, const std::string &name, const std::string &outputName, pragma::asset::Type type) -> bool { return asset_import(nw, name, outputName, type); })];
	auto defMapExportInfo = luabind::class_<pragma::asset::MapExportInfo>("MapExportInfo");
	defMapExportInfo.def(luabind::constructor<>());
	defMapExportInfo.def_readwrite("includeMapLightSources", &pragma::asset::MapExportInfo::includeMapLightSources);
	defMapExportInfo.def("AddCamera", &pragma::asset::MapExportInfo::AddCamera);
	defMapExportInfo.def("AddLightSource", &pragma::asset::MapExportInfo::AddLightSource);
	modAsset[defMapExportInfo];

	Lua::asset::register_library(GetLuaInterface(), false);

	auto defTexImportInfo = luabind::class_<pragma::asset::TextureImportInfo>("TextureImportInfo");
	defTexImportInfo.def(luabind::constructor<>());
	defTexImportInfo.def_readwrite("srgb", &pragma::asset::TextureImportInfo::srgb);
	defTexImportInfo.def_readwrite("normalMap", &pragma::asset::TextureImportInfo::normalMap);
	GetLuaInterface().RegisterLibrary("asset");
	modAsset[defTexImportInfo];

	Lua::RegisterLibraryEnums(GetLuaState(), "asset",
	  {{"TEXTURE_LOAD_FLAG_NONE", umath::to_integral(TextureLoadFlags::None)}, {"TEXTURE_LOAD_FLAG_LOAD_INSTANTLY_BIT", umath::to_integral(TextureLoadFlags::LoadInstantly)}, {"TEXTURE_LOAD_FLAG_RELOAD_BIT", umath::to_integral(TextureLoadFlags::Reload)},
	    {"TEXTURE_LOAD_FLAG_DONT_CACHE_BIT", umath::to_integral(TextureLoadFlags::DontCache)}});

	auto &utilImport = GetLuaInterface().RegisterLibrary("import", {{"export_scene", static_cast<int32_t (*)(lua_State *)>(Lua::lib_export::export_scene)}});

	auto modDebug = luabind::module_(GetLuaState(), "debug");
	modDebug[luabind::def("draw_points", &Lua::DebugRenderer::Client::DrawPoints), luabind::def("draw_lines", &Lua::DebugRenderer::Client::DrawLines), luabind::def("draw_point", &Lua::DebugRenderer::Client::DrawPoint),
	  luabind::def("draw_line", static_cast<std::shared_ptr<::DebugRenderer::BaseObject> (*)(const Vector3 &, const Vector3 &, const DebugRenderInfo &)>(&Lua::DebugRenderer::Client::DrawLine)),
	  luabind::def("draw_line", static_cast<std::shared_ptr<::DebugRenderer::BaseObject> (*)(const Vector3 &, const Vector3 &)>(&Lua::DebugRenderer::Client::DrawLine)), luabind::def("draw_box", &Lua::DebugRenderer::Client::DrawBox),
	  luabind::def("draw_mesh", &Lua::DebugRenderer::Client::DrawMeshes), luabind::def("draw_mesh", &Lua::DebugRenderer::Client::DrawMesh), luabind::def("draw_sphere", &Lua::DebugRenderer::Client::DrawSphere, luabind::default_parameter_policy<3, 1> {}),
	  luabind::def("draw_sphere", &Lua::DebugRenderer::Client::DrawSphere), luabind::def("draw_truncated_cone", &Lua::DebugRenderer::Client::DrawTruncatedCone, luabind::default_parameter_policy<6, 12u> {}),
	  luabind::def("draw_truncated_cone", &Lua::DebugRenderer::Client::DrawTruncatedCone), luabind::def("draw_cylinder", &Lua::DebugRenderer::Client::DrawCylinder, luabind::default_parameter_policy<5, 12u> {}), luabind::def("draw_cylinder", &Lua::DebugRenderer::Client::DrawCylinder),
	  luabind::def("draw_cone", &Lua::DebugRenderer::Client::DrawCone, luabind::default_parameter_policy<5, 12u> {}), luabind::def("draw_cone", &Lua::DebugRenderer::Client::DrawCone), luabind::def("draw_pose", &Lua::DebugRenderer::Client::DrawAxis),
	  luabind::def("draw_text", static_cast<std::shared_ptr<::DebugRenderer::BaseObject> (*)(const std::string &, const Vector2 &, const DebugRenderInfo &)>(&Lua::DebugRenderer::Client::DrawText)),
	  luabind::def("draw_text", static_cast<std::shared_ptr<::DebugRenderer::BaseObject> (*)(const std::string &, float, const DebugRenderInfo &)>(&Lua::DebugRenderer::Client::DrawText)),
	  luabind::def("draw_text", static_cast<std::shared_ptr<::DebugRenderer::BaseObject> (*)(const std::string &, const DebugRenderInfo &)>(&Lua::DebugRenderer::Client::DrawText)), luabind::def("draw_path", &Lua::DebugRenderer::Client::DrawPath),
#ifdef __clang__
	  luabind::def("draw_spline", &Lua::DebugRenderer::Client::DrawSpline),
	  luabind::def(
	    "draw_spline", +[](const std::vector<Vector3> &path, uint32_t numSegments, const DebugRenderInfo &renderInfo) { return Lua::DebugRenderer::Client::DrawSpline(path, numSegments, renderInfo, 1.f); }),
#else
	  luabind::def("draw_spline", &Lua::DebugRenderer::Client::DrawSpline, luabind::default_parameter_policy<4, 1.f> {}),
#endif
	  luabind::def("draw_plane", static_cast<std::shared_ptr<::DebugRenderer::BaseObject> (*)(const umath::Plane &, const DebugRenderInfo &)>(&Lua::DebugRenderer::Client::DrawPlane)),
	  luabind::def("draw_plane", static_cast<std::shared_ptr<::DebugRenderer::BaseObject> (*)(const Vector3 &, float, const DebugRenderInfo &)>(&Lua::DebugRenderer::Client::DrawPlane)),
	  luabind::def("draw_frustum", static_cast<std::shared_ptr<::DebugRenderer::BaseObject> (*)(pragma::CCameraComponent &, const DebugRenderInfo &)>(&Lua::DebugRenderer::Client::DrawFrustum)),
	  luabind::def("draw_frustum", static_cast<std::shared_ptr<::DebugRenderer::BaseObject> (*)(const std::vector<Vector3> &, const DebugRenderInfo &)>(&Lua::DebugRenderer::Client::DrawFrustum)),
	  luabind::def(
	    "create_collection", +[](const std::vector<std::shared_ptr<::DebugRenderer::BaseObject>> &objects) -> std::shared_ptr<::DebugRenderer::BaseObject> { return std::make_shared<::DebugRenderer::CollectionObject>(objects); })];
}
