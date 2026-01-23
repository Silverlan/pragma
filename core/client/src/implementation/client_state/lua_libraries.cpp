// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :client_state;

import pragma.audio.util;
import pragma.gui;
import pragma.platform;
import pragma.string.unicode;

static int log(lua::State *l, spdlog::level::level_enum logLevel)
{
	auto &el = Lua::Check<pragma::gui::types::WIBase>(l, 1);
	const char *msg = Lua::CheckString(l, 2);
	std::string loggerName = "ui_" + el.GetClass();
	int32_t argOffset = 2;
	auto n = lua::get_top(l) - argOffset; /* number of arguments */
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
	pragma::get_cengine()->GetMappedKeys(cvarName, mappedKeys, maxKeys);
	return mappedKeys;
}

static pragma::LuaInputBindingLayerRegister &get_input_binding_layer_register() { return pragma::get_cgame()->GetLuaInputBindingLayerRegister(); }
static std::shared_ptr<InputBindingLayer> create_input_binding_layer()
{
	auto layer = std::shared_ptr<InputBindingLayer> {new InputBindingLayer {}, [](InputBindingLayer *layer) {
		                                                 if(!pragma::math::is_flag_set(pragma::get_cgame()->GetGameFlags(), pragma::Game::GameFlags::ClosingGame))
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
	guiMod[(luabind::def("create", static_cast<pragma::gui::types::WIBase *(*)(pragma::CGame *, const std::string &, pragma::gui::types::WIBase &, int32_t, int32_t, uint32_t, uint32_t, float, float, float, float)>(&Lua::gui::create)),
	  luabind::def("create", static_cast<pragma::gui::types::WIBase *(*)(pragma::CGame *, const std::string &, pragma::gui::types::WIBase &, int32_t, int32_t, uint32_t, uint32_t)>(&Lua::gui::create)), luabind::def("create", static_cast<pragma::gui::types::WIBase *(*)(pragma::CGame *, const std::string &, pragma::gui::types::WIBase &, int32_t, int32_t)>(&Lua::gui::create)),
	  luabind::def("create", static_cast<pragma::gui::types::WIBase *(*)(pragma::CGame *, const std::string &, pragma::gui::types::WIBase *)>(&Lua::gui::create)), luabind::def("create", static_cast<pragma::gui::types::WIBase *(*)(pragma::CGame *, const std::string &)>(&Lua::gui::create)),

	  luabind::def("create_label", static_cast<pragma::gui::types::WIBase *(*)(lua::State *, const std::string &, pragma::gui::types::WIBase &, int32_t, int32_t)>(&Lua::gui::create_label)), luabind::def("create_label", static_cast<pragma::gui::types::WIBase *(*)(lua::State *, const std::string &, pragma::gui::types::WIBase &)>(&Lua::gui::create_label)),
	  luabind::def("create_label", static_cast<pragma::gui::types::WIBase *(*)(lua::State *, const std::string &)>(&Lua::gui::create_label)),

	  luabind::def("create_button", static_cast<pragma::gui::types::WIBase *(*)(lua::State *, const std::string &, pragma::gui::types::WIBase &, int32_t, int32_t)>(&Lua::gui::create_button)), luabind::def("create_button", static_cast<pragma::gui::types::WIBase *(*)(lua::State *, const std::string &, pragma::gui::types::WIBase &)>(&Lua::gui::create_button)),
	  luabind::def("create_button", static_cast<pragma::gui::types::WIBase *(*)(lua::State *, const std::string &)>(&Lua::gui::create_button)),

	  luabind::def("create_checkbox", static_cast<Lua::opt<Lua::mult<Lua::type<pragma::gui::types::WIBase>, Lua::type<pragma::gui::types::WIBase>, Lua::type<pragma::gui::types::WIBase>>> (*)(lua::State *, const std::string &, pragma::gui::types::WIBase &)>(&Lua::gui::create_checkbox)),
	  luabind::def("create_checkbox", static_cast<Lua::opt<Lua::mult<Lua::type<pragma::gui::types::WIBase>, Lua::type<pragma::gui::types::WIBase>, Lua::type<pragma::gui::types::WIBase>>> (*)(lua::State *, const std::string &)>(&Lua::gui::create_checkbox)),

	  luabind::def("register", &Lua::gui::register_element), luabind::def("get_base_element", static_cast<pragma::gui::types::WIBase *(*)(const prosper::Window &)>(&Lua::gui::get_base_element)), luabind::def("get_base_element", static_cast<pragma::gui::types::WIBase *(*)()>(&Lua::gui::get_base_element)),

	  luabind::def("create_render_target", &Lua::gui::create_render_target), luabind::def("create_color_image", &Lua::gui::create_color_image),

	  luabind::def("get_element_at_position", static_cast<pragma::gui::types::WIBase *(*)(lua::State *)>(&Lua::gui::get_element_at_position)), luabind::def("get_element_at_position", static_cast<pragma::gui::types::WIBase *(*)(lua::State *, prosper::Window *)>(&Lua::gui::get_element_at_position)),
	  luabind::def("get_element_at_position", static_cast<pragma::gui::types::WIBase *(*)(lua::State *, prosper::Window *, pragma::gui::types::WIBase *)>(&Lua::gui::get_element_at_position)),
	  luabind::def("get_element_at_position", static_cast<pragma::gui::types::WIBase *(*)(lua::State *, prosper::Window *, pragma::gui::types::WIBase *, int32_t, int32_t)>(&Lua::gui::get_element_at_position)),
	  luabind::def("get_element_at_position", static_cast<pragma::gui::types::WIBase *(*)(lua::State *, prosper::Window *, pragma::gui::types::WIBase *, int32_t, int32_t, const Lua::func<bool, pragma::gui::types::WIBase> &)>(&Lua::gui::get_element_at_position)),

	  luabind::def("get_element_under_cursor", static_cast<pragma::gui::types::WIBase *(*)(lua::State *, const Lua::func<bool, pragma::gui::types::WIBase> &)>(&Lua::gui::get_element_under_cursor)),
	  luabind::def("get_element_under_cursor", static_cast<pragma::gui::types::WIBase *(*)(lua::State *, const prosper::Window *, const Lua::func<bool, pragma::gui::types::WIBase> &)>(&Lua::gui::get_element_under_cursor)),
	  luabind::def("get_element_under_cursor", static_cast<pragma::gui::types::WIBase *(*)(lua::State *, const prosper::Window *)>(&Lua::gui::get_element_under_cursor)),
	  luabind::def("get_element_under_cursor", static_cast<pragma::gui::types::WIBase *(*)(lua::State *, const prosper::Window *)>(&Lua::gui::get_element_under_cursor), luabind::default_parameter_policy<2, static_cast<const prosper::Window *>(nullptr)> {}),
	  luabind::def("get_element_under_cursor", static_cast<pragma::gui::types::WIBase *(*)(lua::State *, pragma::gui::types::WIBase &)>(&Lua::gui::get_element_under_cursor)),

	  luabind::def("find_focused_window", static_cast<prosper::Window *(*)()>([]() -> prosper::Window * { return pragma::gui::WGUI::GetInstance().FindFocusedWindow(); }), luabind::pointer_policy<0> {}),
	  luabind::def("get_primary_window", static_cast<prosper::Window *(*)()>([]() -> prosper::Window * { return &pragma::get_cengine()->GetRenderContext().GetWindow(); }), luabind::pointer_policy<0> {}),
		luabind::def("get_windows", +[]() -> std::vector<prosper::Window*> {
			auto &windows = pragma::get_cengine()->GetRenderContext().GetWindows();
			std::vector<prosper::Window*> pwindows;
			pwindows.reserve(windows.size());
			for (auto &window : windows)
				pwindows.push_back(window.get());
			return pwindows;
		}),
		luabind::def("find_window_root_element", +[](const prosper::Window &window) -> pragma::gui::WIHandle {
			auto *el = pragma::gui::WGUI::GetInstance().FindWindowRootElement(window);
			if (!el)
				return {};
			return el->GetHandle();
		}),
	  luabind::def(
	    "get_primary_monitor", +[]() -> pragma::platform::Monitor { return pragma::platform::get_primary_monitor(); }),
	  luabind::def("find_window_under_cursor", static_cast<prosper::Window *(*)()>([]() -> prosper::Window * { return pragma::gui::WGUI::GetInstance().FindWindowUnderCursor(); }), luabind::pointer_policy<0> {}),

	  luabind::def("get_focused_element", static_cast<pragma::gui::types::WIBase *(*)(lua::State *)>(&Lua::gui::get_focused_element)), luabind::def("get_focused_element", static_cast<pragma::gui::types::WIBase *(*)(lua::State *, prosper::Window &)>(&Lua::gui::get_focused_element)),
	  luabind::def("register_skin", static_cast<bool (*)(lua::State *, const std::string &, const luabind::tableT<void> &, const luabind::tableT<void> &)>(&Lua::gui::register_skin)),
	  luabind::def("register_skin", static_cast<bool (*)(lua::State *, const std::string &, const luabind::tableT<void> &, const luabind::tableT<void> &, const std::string &)>(&Lua::gui::register_skin)), luabind::def("register_default_skin", &Lua::gui::register_default_skin),
	  luabind::def(
	    "set_focus_enabled", +[](const prosper::Window &window, bool focusEnabled) { pragma::gui::WGUI::GetInstance().SetFocusEnabled(window, focusEnabled); }),
	  luabind::def(
	    "is_focus_enabled", +[](const prosper::Window &window) { return pragma::gui::WGUI::GetInstance().IsFocusEnabled(window); }),

	  luabind::def("load_skin", &load_skin), luabind::def("set_skin", Lua::gui::set_skin), luabind::def("skin_exists", Lua::gui::skin_exists), luabind::def("get_cursor", Lua::gui::get_cursor), luabind::def("set_cursor", Lua::gui::set_cursor),
	  luabind::def("get_cursor_input_mode", Lua::gui::get_cursor_input_mode), luabind::def("set_cursor_input_mode", Lua::gui::set_cursor_input_mode), luabind::def("get_window_size", Lua::gui::get_window_size),
	  luabind::def("inject_mouse_input", static_cast<bool (*)(pragma::platform::MouseButton, pragma::platform::KeyState, pragma::platform::Modifier, const Vector2i &)>(&Lua::gui::inject_mouse_input)),
	  luabind::def("inject_mouse_input", static_cast<bool (*)(pragma::platform::MouseButton, pragma::platform::KeyState, pragma::platform::Modifier)>(&Lua::gui::inject_mouse_input)), luabind::def("inject_keyboard_input", Lua::gui::inject_keyboard_input), luabind::def("inject_char_input", Lua::gui::inject_char_input),
	  luabind::def("inject_scroll_input", static_cast<bool (*)(lua::State *, const Vector2 &, const Vector2i &)>(&Lua::gui::inject_scroll_input)), luabind::def("inject_scroll_input", static_cast<bool (*)(lua::State *, const Vector2 &)>(&Lua::gui::inject_scroll_input)),
	  luabind::def("find_element_by_name", static_cast<pragma::gui::types::WIBase *(*)(const std::string &)>([](const std::string &name) -> pragma::gui::types::WIBase * {
		  auto *p = pragma::gui::WGUI::GetInstance().GetBaseElement();
		  if(p == nullptr)
			  return nullptr;
		  return p->FindDescendantByName(name);
	  })),
	  luabind::def("find_element_by_index", static_cast<pragma::gui::types::WIBase *(*)(uint64_t)>([](uint64_t idx) -> pragma::gui::types::WIBase * { return pragma::gui::WGUI::GetInstance().FindByIndex(idx); })),
	  luabind::def("find_elements_by_name", static_cast<luabind::tableT<pragma::gui::types::WIBase> (*)(lua::State *, const std::string &)>([](lua::State *l, const std::string &name) -> luabind::tableT<pragma::gui::types::WIBase> {
		  std::vector<pragma::gui::WIHandle> results {};
		  auto t = luabind::newtable(l);
		  auto *p = pragma::gui::WGUI::GetInstance().GetBaseElement();
		  if(p != nullptr)
			  p->FindDescendantsByName(name, results);
		  auto idx = 1;
		  for(auto &hEl : results) {
			  if(hEl.IsValid() == false)
				  continue;
			  t[idx++] = pragma::gui::WGUILuaInterface::GetLuaObject(l, *hEl.get());
		  }
		  return t;
	  })),
	  luabind::def("find_elements_by_class", static_cast<luabind::tableT<pragma::gui::types::WIBase> (*)(lua::State *, const std::string &)>([](lua::State *l, const std::string &className) -> luabind::tableT<pragma::gui::types::WIBase> {
		  auto t = luabind::newtable(l);
		  auto *p = pragma::gui::WGUI::GetInstance().GetBaseElement();
		  if(p == nullptr)
			  return t;
		  uint32_t idx = 1;
		  std::function<void(pragma::gui::types::WIBase &)> fIterateChildren = nullptr;
		  fIterateChildren = [l, &fIterateChildren, &className, &t, &idx](pragma::gui::types::WIBase &el) mutable {
			  if(pragma::string::compare(el.GetClass(), className, false))
				  t[idx++] = pragma::gui::WGUILuaInterface::GetLuaObject(l, el);
			  for(auto &hChild : *el.GetChildren()) {
				  if(hChild.IsValid() == false)
					  continue;
				  fIterateChildren(*hChild.get());
			  }
		  };
		  fIterateChildren(*p);
		  return t;
	  })),
	  luabind::def("get_console", &pragma::gui::types::WIConsole::GetConsole), luabind::def("open_console", &pragma::gui::types::WIConsole::Open), luabind::def("close_console", &pragma::gui::types::WIConsole::Close), luabind::def("is_console_open", static_cast<bool (*)(lua::State *)>([](lua::State *l) -> bool {
		  auto *pConsole = pragma::gui::types::WIConsole::GetConsole();
		  auto *pFrame = pConsole ? pConsole->GetFrame() : nullptr;
		  return pFrame ? pFrame->IsVisible() : false;
	  })),
	  luabind::def("get_delta_time", static_cast<float (*)(lua::State *)>([](lua::State *l) -> float { return pragma::gui::WGUI::GetInstance().GetDeltaTime(); })), luabind::def("get_base_elements", static_cast<luabind::tableT<pragma::gui::types::WIBase> (*)(lua::State *)>([](lua::State *l) -> luabind::tableT<pragma::gui::types::WIBase> {
		  auto &els = pragma::gui::WGUI::GetInstance().GetBaseElements();
		  auto t = luabind::newtable(l);
		  int32_t idx = 1;
		  for(auto &hEl : els) {
			  if(hEl.IsValid() == false)
				  continue;
			  auto o = pragma::gui::WGUILuaInterface::GetLuaObject(l, const_cast<pragma::gui::types::WIBase &>(*hEl.get()));
			  t[idx++] = o;
		  }
		  return t;
	  })),
	  luabind::def("get_delta_time", static_cast<float (*)(lua::State *)>([](lua::State *l) -> float { return pragma::gui::WGUI::GetInstance().GetDeltaTime(); })),
	  luabind::def("get_next_gui_element_index", static_cast<uint64_t (*)(lua::State *)>([](lua::State *l) -> uint64_t { return pragma::gui::WGUI::GetInstance().GetNextGuiElementIndex(); })),
	  luabind::def("add_base_element", static_cast<pragma::gui::types::WIBase *(*)(const prosper::Window &)>([](const prosper::Window &window) -> pragma::gui::types::WIBase * { return pragma::gui::WGUI::GetInstance().AddBaseElement(&window); })),
	  luabind::def("add_base_element", static_cast<pragma::gui::types::WIBase *(*)()>([]() -> pragma::gui::types::WIBase * { return pragma::gui::WGUI::GetInstance().AddBaseElement(); })),
	  luabind::def(
	    "get_supported_video_modes", +[]() -> auto{
		    auto &context = pragma::gui::WGUI::GetInstance().GetContext();
		    auto &window = context.GetWindow();
		    auto *monitor = window->GetMonitor();
		    auto primaryMonitor = pragma::platform::get_primary_monitor();
		    if(monitor == nullptr)
			    monitor = &primaryMonitor;
		    return monitor->GetSupportedVideoModes();
	    }),
		  luabind::def("open_main_menu",+[](pragma::ClientState *cl) { cl->OpenMainMenu();
			}),
		  luabind::def("close_main_menu",+[](pragma::ClientState *cl) { cl->CloseMainMenu();
			}),
		  luabind::def("is_main_menu_open",+[](pragma::ClientState *cl) -> bool { return cl->IsMainMenuOpen();
		}),
	  luabind::def(
	    "reload_text_elements", +[]() {
		    auto &baseElements = pragma::gui::WGUI::GetInstance().GetBaseElements();
		    std::function<void(pragma::gui::types::WIBase &)> updateTextElements = nullptr;
		    updateTextElements = [&updateTextElements](pragma::gui::types::WIBase &el) {
			    if(typeid(el) == typeid(pragma::gui::types::WIText)) {
				    static_cast<pragma::gui::types::WIText &>(el).ReloadFont();
				    static_cast<pragma::gui::types::WIText &>(el).SizeToContents();
			    }
			    for(auto &hEl : *el.GetChildren()) {
				    if(!hEl.IsValid())
					    continue;
				    updateTextElements(*hEl.get());
			    }
		    };
		    for(auto &hEl : baseElements) {
			    if(!hEl.IsValid())
				    continue;
			    updateTextElements(const_cast<pragma::gui::types::WIBase &>(*hEl.get()));
		    }
	    })
		)];

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

	auto wiElementClassDef = luabind::class_<pragma::gui::types::WIBase>("Element");
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
	auto wiBaseWIElement = luabind::class_<pragma::gui::types::WILuaBase, luabind::bases<pragma::gui::types::WIBase>, pragma::LuaCore::WILuaBaseHolder>("Base");
	wiBaseWIElement.def(luabind::constructor<>());
	wiBaseWIElement.def("OnInitialize", &pragma::gui::types::WILuaBase::Lua_OnInitialize, &pragma::gui::types::WILuaBase::default_OnInitialize);
	wiBaseWIElement.def("OnThink", &pragma::gui::types::WILuaBase::Lua_OnThink, &pragma::gui::types::WILuaBase::default_OnThink);
	wiBaseWIElement.def("OnInitialThink", &pragma::gui::types::WILuaBase::Lua_OnFirstThink, &pragma::gui::types::WILuaBase::default_OnFirstThink);
	wiBaseWIElement.def("MouseCallback", &pragma::gui::types::WILuaBase::Lua_MouseCallback, &pragma::gui::types::WILuaBase::default_MouseCallback);
	wiBaseWIElement.def("KeyboardCallback", &pragma::gui::types::WILuaBase::Lua_KeyboardCallback, &pragma::gui::types::WILuaBase::default_KeyboardCallback);
	wiBaseWIElement.def("CharCallback", &pragma::gui::types::WILuaBase::Lua_CharCallback, &pragma::gui::types::WILuaBase::default_CharCallback);
	wiBaseWIElement.def("ScrollCallback", &pragma::gui::types::WILuaBase::Lua_ScrollCallback, &pragma::gui::types::WILuaBase::default_ScrollCallback);
	wiBaseWIElement.def("OnUpdate", &pragma::gui::types::WILuaBase::Lua_OnUpdate, &pragma::gui::types::WILuaBase::default_OnUpdate);
	wiBaseWIElement.def("OnVisibilityChanged", &pragma::gui::types::WILuaBase::Lua_OnSetVisible, &pragma::gui::types::WILuaBase::default_OnSetVisible);
	wiBaseWIElement.def("OnSizeChanged", &pragma::gui::types::WILuaBase::Lua_OnSetSize, &pragma::gui::types::WILuaBase::default_OnSetSize);
	wiBaseWIElement.def("OnColorChanged", &pragma::gui::types::WILuaBase::Lua_OnSetColor, &pragma::gui::types::WILuaBase::default_OnSetColor);
	wiBaseWIElement.def("OnAlphaChanged", &pragma::gui::types::WILuaBase::Lua_OnSetAlpha, &pragma::gui::types::WILuaBase::default_OnSetAlpha);
	wiBaseWIElement.def("CheckPosInBounds", &pragma::gui::types::WILuaBase::Lua_CheckPosInBounds, &pragma::gui::types::WILuaBase::default_CheckPosInBounds);
	wiBaseWIElement.def("OnDraw", &pragma::gui::types::WILuaBase::Lua_Render, &pragma::gui::types::WILuaBase::default_Render);
	wiBaseWIElement.def("OnCursorEntered", &pragma::gui::types::WILuaBase::Lua_OnCursorEntered, &pragma::gui::types::WILuaBase::default_OnCursorEntered);
	wiBaseWIElement.def("OnCursorExited", &pragma::gui::types::WILuaBase::Lua_OnCursorExited, &pragma::gui::types::WILuaBase::default_OnCursorExited);
	wiBaseWIElement.def("OnFileDragEntered", &pragma::gui::types::WILuaBase::Lua_OnFileDragEntered, &pragma::gui::types::WILuaBase::default_OnFileDragEntered);
	wiBaseWIElement.def("OnFileDragExited", &pragma::gui::types::WILuaBase::Lua_OnFileDragExited, &pragma::gui::types::WILuaBase::default_OnFileDragExited);
	wiBaseWIElement.def("OnFilesDropped", &pragma::gui::types::WILuaBase::Lua_OnFilesDropped, &pragma::gui::types::WILuaBase::default_OnFilesDropped);
	wiBaseWIElement.def("OnFocusGained", &pragma::gui::types::WILuaBase::Lua_OnFocusGained, &pragma::gui::types::WILuaBase::default_OnFocusGained);
	wiBaseWIElement.def("OnFocusKilled", &pragma::gui::types::WILuaBase::Lua_OnFocusKilled, &pragma::gui::types::WILuaBase::default_OnFocusKilled);
	wiBaseWIElement.def("OnRemove", &pragma::gui::types::WILuaBase::Lua_OnRemove, &pragma::gui::types::WILuaBase::default_OnRemove);
	wiBaseWIElement.def("SetRenderCommandBuffer", &pragma::gui::types::WILuaBase::SetRenderCommandBuffer);
	guiMod[wiBaseWIElement];
	//

	// Class specific handles have to also be defined in CGame::InitializeGUIElement and WGUIHandleFactory!
	auto wiShapeClassDef = luabind::class_<pragma::gui::types::WIShape, pragma::gui::types::WIBase>("Shape");
	Lua::WIShape::register_class(wiShapeClassDef);
	guiMod[wiShapeClassDef];

	auto wiButtonClassDef = luabind::class_<pragma::gui::types::WIButton, pragma::gui::types::WIBase>("Button");
	Lua::WIButton::register_class(wiButtonClassDef);
	guiMod[wiButtonClassDef];

	auto wiTexturedShapeClassDef = luabind::class_<pragma::gui::types::WITexturedShape, luabind::bases<pragma::gui::types::WIShape, pragma::gui::types::WIBase>>("TexturedShape");
	Lua::WITexturedShape::register_class(wiTexturedShapeClassDef);
	guiMod[wiTexturedShapeClassDef];

	auto wiIconClassDef = luabind::class_<pragma::gui::types::WIIcon, luabind::bases<pragma::gui::types::WITexturedShape, pragma::gui::types::WIShape, pragma::gui::types::WIBase>>("Icon");
	Lua::WIIcon::register_class(wiIconClassDef);
	guiMod[wiIconClassDef];

	auto wiSilkIconClassDef = luabind::class_<pragma::gui::types::WISilkIcon, luabind::bases<pragma::gui::types::WIIcon, pragma::gui::types::WITexturedShape, pragma::gui::types::WIShape, pragma::gui::types::WIBase>>("SilkIcon");
	Lua::WISilkIcon::register_class(wiSilkIconClassDef);
	guiMod[wiSilkIconClassDef];

	auto wiArrowClassDef = luabind::class_<pragma::gui::types::WIArrow, luabind::bases<pragma::gui::types::WIShape, pragma::gui::types::WIBase>>("Arrow");
	Lua::WIArrow::register_class(wiArrowClassDef);
	guiMod[wiArrowClassDef];

	auto wiCheckboxClassDef = luabind::class_<pragma::gui::types::WICheckbox, luabind::bases<pragma::gui::types::WIShape, pragma::gui::types::WIBase>>("Checkbox");
	Lua::WICheckbox::register_class(wiCheckboxClassDef);
	guiMod[wiCheckboxClassDef];

	auto wiTransformableClassDef = luabind::class_<pragma::gui::types::WITransformable, pragma::gui::types::WIBase>("Transformable");
	Lua::WITransformable::register_class(wiTransformableClassDef);
	guiMod[wiTransformableClassDef];

	auto wiSnapAreaClassDef = luabind::class_<pragma::gui::types::WISnapArea, pragma::gui::types::WIBase>("SnapArea");
	wiSnapAreaClassDef.def("GetTriggerArea", &pragma::gui::types::WISnapArea::GetTriggerArea);
	guiMod[wiSnapAreaClassDef];

	auto wiDebugDepthTextureClassDef = luabind::class_<pragma::gui::types::WIDebugDepthTexture, pragma::gui::types::WIBase>("DebugDepthTexture");
	wiDebugDepthTextureClassDef.def("SetContrastFactor", &pragma::gui::types::WIDebugDepthTexture::SetContrastFactor);
	wiDebugDepthTextureClassDef.def("GetContrastFactor", &pragma::gui::types::WIDebugDepthTexture::GetContrastFactor);
	wiDebugDepthTextureClassDef.def("SetTexture", static_cast<void (pragma::gui::types::WIDebugDepthTexture::*)(prosper::Texture &, bool)>(&pragma::gui::types::WIDebugDepthTexture::SetTexture));
	wiDebugDepthTextureClassDef.def("SetTexture", static_cast<void (pragma::gui::types::WIDebugDepthTexture::*)(prosper::Texture &, bool)>(&pragma::gui::types::WIDebugDepthTexture::SetTexture), luabind::default_parameter_policy<3, false> {});
	guiMod[wiDebugDepthTextureClassDef];

	auto wiDebugShadowMapClassDef = luabind::class_<pragma::gui::types::WIDebugShadowMap, pragma::gui::types::WIBase>("DebugShadowMap");
	wiDebugShadowMapClassDef.def("SetContrastFactor", &pragma::gui::types::WIDebugShadowMap::SetContrastFactor);
	wiDebugShadowMapClassDef.def("GetContrastFactor", &pragma::gui::types::WIDebugShadowMap::GetContrastFactor);
	wiDebugShadowMapClassDef.def("SetShadowMapSize", &pragma::gui::types::WIDebugShadowMap::SetShadowMapSize);
	wiDebugShadowMapClassDef.def("SetLightSource", &pragma::gui::types::WIDebugShadowMap::SetLightSource);
	guiMod[wiDebugShadowMapClassDef];

	auto wiDebugSsaoClassDef = luabind::class_<pragma::gui::types::WIDebugSSAO, luabind::bases<pragma::gui::types::WITexturedShape, pragma::gui::types::WIShape, pragma::gui::types::WIBase>>("DebugSSAO");
	wiDebugSsaoClassDef.def("SetUseBlurredSSAOImage", &pragma::gui::types::WIDebugSSAO::SetUseBlurredSSAOImage);
	guiMod[wiDebugSsaoClassDef];

	auto wiProgressBarClassDef = luabind::class_<pragma::gui::types::WIProgressBar, pragma::gui::types::WIBase>("ProgressBar");
	Lua::WIProgressBar::register_class(wiProgressBarClassDef);
	guiMod[wiProgressBarClassDef];

	auto wiTooltipClassDef = luabind::class_<pragma::gui::types::WITooltip, pragma::gui::types::WIBase>("Tooltip");
	wiTooltipClassDef.def("SetText", +[](pragma::gui::types::WITooltip &elTooltip, const std::string &text) { elTooltip.SetText(text); });
	wiTooltipClassDef.def("GetText", +[](const pragma::gui::types::WITooltip &elTooltip) { return elTooltip.GetText().cpp_str(); });
	guiMod[wiTooltipClassDef];

	auto wiSliderClassDef = luabind::class_<pragma::gui::types::WISlider, luabind::bases<pragma::gui::types::WIProgressBar, pragma::gui::types::WIBase>>("Slider");
	Lua::WISlider::register_class(wiSliderClassDef);
	guiMod[wiSliderClassDef];

	auto wiScrollContainerClassDef = luabind::class_<pragma::gui::types::WIScrollContainer, pragma::gui::types::WIBase>("ScrollContainer");
	wiScrollContainerClassDef.def("SetAutoStickToBottom", &pragma::gui::types::WIScrollContainer::SetAutoStickToBottom);
	wiScrollContainerClassDef.def("ShouldAutoStickToBottom", &pragma::gui::types::WIScrollContainer::ShouldAutoStickToBottom);
	wiScrollContainerClassDef.def("IsContentsWidthFixed", &pragma::gui::types::WIScrollContainer::IsContentsWidthFixed);
	wiScrollContainerClassDef.def("IsContentsHeightFixed", &pragma::gui::types::WIScrollContainer::IsContentsHeightFixed);
	wiScrollContainerClassDef.def("SetContentsWidthFixed", &pragma::gui::types::WIScrollContainer::SetContentsWidthFixed);
	wiScrollContainerClassDef.def("SetContentsHeightFixed", &pragma::gui::types::WIScrollContainer::SetContentsHeightFixed);
	wiScrollContainerClassDef.def("GetVerticalScrollBar", &pragma::gui::types::WIScrollContainer::GetVerticalScrollBar);
	wiScrollContainerClassDef.def("GetHorizontalScrollBar", &pragma::gui::types::WIScrollContainer::GetHorizontalScrollBar);
	wiScrollContainerClassDef.def("SetScrollAmount", &pragma::gui::types::WIScrollContainer::SetScrollAmount);
	wiScrollContainerClassDef.def("SetScrollAmountX", &pragma::gui::types::WIScrollContainer::SetScrollAmountX);
	wiScrollContainerClassDef.def("SetScrollAmountY", &pragma::gui::types::WIScrollContainer::SetScrollAmountY);
	wiScrollContainerClassDef.def("GetWrapperElement", &pragma::gui::types::WIScrollContainer::GetWrapperElement);
	wiScrollContainerClassDef.def("ScrollToElement", &pragma::gui::types::WIScrollContainer::ScrollToElement);
	wiScrollContainerClassDef.def("ScrollToElementX", &pragma::gui::types::WIScrollContainer::ScrollToElementX);
	wiScrollContainerClassDef.def("ScrollToElementY", &pragma::gui::types::WIScrollContainer::ScrollToElementY);
	guiMod[wiScrollContainerClassDef];

	auto wiContainerClassDef = luabind::class_<pragma::gui::types::WIContainer, pragma::gui::types::WIBase>("Container");
	Lua::WIContainer::register_class(wiContainerClassDef);
	guiMod[wiContainerClassDef];

	auto wiTableClassDef = luabind::class_<pragma::gui::types::WITable, luabind::bases<pragma::gui::types::WIContainer, pragma::gui::types::WIBase>>("Table");
	Lua::WITable::register_class(wiTableClassDef);

	auto wiTableRowClassDef = luabind::class_<pragma::gui::types::WITableRow, luabind::bases<pragma::gui::types::WIContainer, pragma::gui::types::WIBase>>("Row");
	Lua::WITableRow::register_class(wiTableRowClassDef);

	auto wiTableCellClassDef = luabind::class_<pragma::gui::types::WITableCell, luabind::bases<pragma::gui::types::WIContainer, pragma::gui::types::WIBase>>("Cell");
	Lua::WITableCell::register_class(wiTableCellClassDef);
	wiTableClassDef.scope[wiTableCellClassDef];
	wiTableClassDef.scope[wiTableRowClassDef];
	guiMod[wiTableClassDef];

	auto wiGridPanelClassDef = luabind::class_<pragma::gui::types::WIGridPanel, luabind::bases<pragma::gui::types::WITable, pragma::gui::types::WIBase>>("GridPanel");
	Lua::WIGridPanel::register_class(wiGridPanelClassDef);
	guiMod[wiGridPanelClassDef];

	auto wiTreeListClassDef = luabind::class_<pragma::gui::types::WITreeList, luabind::bases<pragma::gui::types::WITable, pragma::gui::types::WIBase>>("TreeList");
	Lua::WITreeList::register_class(wiTreeListClassDef);

	auto wiTreeListElementClassDef = luabind::class_<pragma::gui::types::WITreeListElement, luabind::bases<pragma::gui::types::WITableRow, pragma::gui::types::WIBase>>("Element");
	Lua::WITreeListElement::register_class(wiTreeListElementClassDef);
	wiTreeListClassDef.scope[wiTreeListElementClassDef];
	guiMod[wiTreeListClassDef];

	auto wiFrameClassDef = luabind::class_<pragma::gui::types::WIFrame, luabind::bases<pragma::gui::types::WITransformable, pragma::gui::types::WIBase>>("Frame");
	Lua::WIFrame::register_class(wiFrameClassDef);
	guiMod[wiFrameClassDef];

	auto wiTextClassDef = luabind::class_<pragma::gui::types::WIText, pragma::gui::types::WIBase>("Text");
	Lua::WIText::register_class(wiTextClassDef);
	guiMod[wiTextClassDef];

	auto wiTextEntryClassDef = luabind::class_<pragma::gui::types::WITextEntry, pragma::gui::types::WIBase>("TextEntry");
	Lua::WITextEntry::register_class(wiTextEntryClassDef);
	wiTextEntryClassDef.def("GetCaretElement", &pragma::gui::types::WITextEntry::GetCaretElement);
	guiMod[wiTextEntryClassDef];

	auto wiCommandLineEntryClassDef = luabind::class_<pragma::gui::types::WICommandLineEntry, luabind::bases<pragma::gui::types::WITextEntry, pragma::gui::types::WIBase>>("CommandLineEntry");
	wiCommandLineEntryClassDef.def("SetAutoCompleteEntryLimit", &pragma::gui::types::WICommandLineEntry::SetAutoCompleteEntryLimit);
	wiCommandLineEntryClassDef.def("GetAutoCompleteEntryLimit", &pragma::gui::types::WICommandLineEntry::GetAutoCompleteEntryLimit);
	guiMod[wiCommandLineEntryClassDef];

	auto wiOutlinedRectClassDef = luabind::class_<pragma::gui::types::WIOutlinedRect, pragma::gui::types::WIBase>("OutlinedRect");
	Lua::WIOutlinedRect::register_class(wiOutlinedRectClassDef);

	auto wiLineClassDef = luabind::class_<pragma::gui::types::WILine, pragma::gui::types::WIBase>("Line");
	Lua::WILine::register_class(wiLineClassDef);

	auto wiWIContentWrapper = luabind::class_<pragma::gui::types::WIContentWrapper, pragma::gui::types::WIBase>("ContentWrapper");
	wiWIContentWrapper.def("SetPadding", static_cast<void (pragma::gui::types::WIContentWrapper::*)(int32_t, int32_t, int32_t, int32_t)>(&pragma::gui::types::WIContentWrapper::SetPadding));
	wiWIContentWrapper.def("ClearPadding", &pragma::gui::types::WIContentWrapper::ClearPadding);
	wiWIContentWrapper.def(
	  "GetPadding", +[](pragma::gui::types::WIContentWrapper &el) -> std::tuple<int32_t, int32_t, int32_t, int32_t> {
		  auto padding = el.GetPadding();
		  return std::make_tuple(padding.left, padding.right, padding.top, padding.bottom);
	  });
	wiWIContentWrapper.def("SetPaddingLeft", &pragma::gui::types::WIContentWrapper::SetPaddingLeft);
	wiWIContentWrapper.def("SetPaddingRight", &pragma::gui::types::WIContentWrapper::SetPaddingRight);
	wiWIContentWrapper.def("SetPaddingTop", &pragma::gui::types::WIContentWrapper::SetPaddingTop);
	wiWIContentWrapper.def("SetPaddingBottom", &pragma::gui::types::WIContentWrapper::SetPaddingBottom);
	wiWIContentWrapper.def("SetPaddingLeftRight", &pragma::gui::types::WIContentWrapper::SetPaddingLeftRight);
	wiWIContentWrapper.def("SetPaddingTopBottom", &pragma::gui::types::WIContentWrapper::SetPaddingTopBottom);
	guiMod[wiWIContentWrapper];

	auto wiNineSliceRect = luabind::class_<pragma::gui::types::WI9SliceRect, pragma::gui::types::WIBase>("NineSliceRect");
	wiNineSliceRect.def("SetMaterial", static_cast<void (pragma::gui::types::WI9SliceRect ::*)(const std::string &)>(&pragma::gui::types::WI9SliceRect::SetMaterial));
	wiNineSliceRect.def("SetMaterial", static_cast<void (pragma::gui::types::WI9SliceRect ::*)(pragma::material::Material &)>(&pragma::gui::types::WI9SliceRect::SetMaterial));
	wiNineSliceRect.def("GetMaterial", &pragma::gui::types::WI9SliceRect::GetMaterial);
	guiMod[wiNineSliceRect];

	auto wiRoundedTexturedRect = luabind::class_<pragma::gui::types::WIRoundedTexturedRect, luabind::bases<pragma::gui::types::WITexturedShape, pragma::gui::types::WIShape, pragma::gui::types::WIBase>>("RoundedTexturedRect");
	Lua::WIRoundedTexturedRect::register_class(wiRoundedTexturedRect);
	guiMod[wiRoundedTexturedRect];

	auto wiScrollBarClassDef = luabind::class_<pragma::gui::types::WIScrollBar, pragma::gui::types::WIBase>("ScrollBar");
	Lua::WIScrollBar::register_class(wiScrollBarClassDef);

	auto wiNumericEntryClassDef = luabind::class_<pragma::gui::types::WINumericEntry, luabind::bases<pragma::gui::types::WITextEntry, pragma::gui::types::WIBase>>("NumericTextEntry");
	Lua::WINumericEntry::register_class(wiNumericEntryClassDef);
	guiMod[wiNumericEntryClassDef];

	auto wiDropDownMenuClassDef = luabind::class_<pragma::gui::types::WIDropDownMenu, luabind::bases<pragma::gui::types::WITextEntry, pragma::gui::types::WIBase>>("DropDownMenu");
	Lua::WIDropDownMenu::register_class(wiDropDownMenuClassDef);
	guiMod[wiDropDownMenuClassDef];

	auto wiRootDef = luabind::class_<pragma::gui::types::WIRoot, pragma::gui::types::WIBase>("Root");
	wiRootDef.def("GetRootCursorPos", &pragma::gui::types::WIRoot::GetCursorPos);
	wiRootDef.def("SetRootCursorPosOverride", &pragma::gui::types::WIRoot::SetCursorPosOverride);
	wiRootDef.def("GetRootCursorPosOverride", &pragma::gui::types::WIRoot::GetCursorPosOverride);
	wiRootDef.def("ClearRootCursorPosOverride", &pragma::gui::types::WIRoot::ClearCursorPosOverride);
	wiRootDef.def("GetAssociatedWindow", static_cast<prosper::Window *(pragma::gui::types::WIRoot::*)()>(&pragma::gui::types::WIRoot::GetWindow));
	guiMod[wiRootDef];

	auto wiConsoleClassDef = luabind::class_<pragma::gui::types::WIConsole, pragma::gui::types::WIBase>("Console");
	wiConsoleClassDef.def("GetCommandLineEntryElement", &pragma::gui::types::WIConsole::GetCommandLineEntryElement);
	wiConsoleClassDef.def("GetTextLogElement", &pragma::gui::types::WIConsole::GetTextLogElement);
	wiConsoleClassDef.def("SetExternallyOwned", &pragma::gui::types::WIConsole::SetExternallyOwned);
	wiConsoleClassDef.def("IsExternallyOwned", &pragma::gui::types::WIConsole::IsExternallyOwned);
	wiConsoleClassDef.def("GetFrame", &pragma::gui::types::WIConsole::GetFrame);
	wiConsoleClassDef.def("GetText", +[](const pragma::gui::types::WIConsole &console) { return console.GetText().cpp_str(); });
	wiConsoleClassDef.def("SetText", &pragma::gui::types::WIConsole::SetText);
	wiConsoleClassDef.def("AppendText", &pragma::gui::types::WIConsole::AppendText);
	wiConsoleClassDef.def("Clear", &pragma::gui::types::WIConsole::Clear);
	wiConsoleClassDef.def("SetMaxLogLineCount", &pragma::gui::types::WIConsole::SetMaxLogLineCount);
	wiConsoleClassDef.def("GetMaxLogLineCount", &pragma::gui::types::WIConsole::GetMaxLogLineCount);
	guiMod[wiConsoleClassDef];

	auto wiRoundedRectClassDef = luabind::class_<pragma::gui::types::WIRoundedRect, luabind::bases<pragma::gui::types::WIShape, pragma::gui::types::WIBase>>("RoundedRect");
	Lua::WIRoundedRect::register_class(wiRoundedRectClassDef);
	guiMod[wiRoundedRectClassDef];

	guiMod[wiOutlinedRectClassDef];
	guiMod[wiLineClassDef];
	guiMod[wiScrollBarClassDef];
}

void pragma::ClientState::RegisterSharedLuaLibraries(Lua::Interface &lua, bool bGUI)
{
	register_gui(lua);

	auto inputMod = luabind::module(lua.GetState(), "input");
	inputMod[(luabind::def(
	            "get_mouse_button_state", +[](platform::MouseButton mouseButton) -> platform::KeyState { return get_cengine()->GetWindow()->GetMouseButtonState(mouseButton); }),
	  luabind::def(
	    "get_key_state", +[](platform::Key key) -> platform::KeyState { return get_cengine()->GetWindow()->GetKeyState(key); }),
	  luabind::def(
	    "get_cursor_pos", +[]() -> Vector2 { return get_cengine()->GetWindow()->GetCursorPos(); }),
	  luabind::def(
	    "set_cursor_pos", +[](const Vector2 &pos) { get_cengine()->GetWindow()->SetCursorPos(pos); }),
	  luabind::def(
	    "is_ctrl_key_down", +[]() -> bool { return get_cengine()->GetWindow()->GetKeyState(platform::Key::LeftControl) != platform::KeyState::Release || get_cengine()->GetWindow()->GetKeyState(platform::Key::RightControl) != platform::KeyState::Release; }),
	  luabind::def(
	    "is_alt_key_down", +[]() -> bool { return get_cengine()->GetWindow()->GetKeyState(platform::Key::LeftAlt) != platform::KeyState::Release || get_cengine()->GetWindow()->GetKeyState(platform::Key::RightAlt) != platform::KeyState::Release; }),
	  luabind::def(
	    "is_shift_key_down", +[]() -> bool { return get_cengine()->GetWindow()->GetKeyState(platform::Key::LeftShift) != platform::KeyState::Release || get_cengine()->GetWindow()->GetKeyState(platform::Key::RightShift) != platform::KeyState::Release; }),
	  luabind::def(
	    "center_cursor",
	    +[]() {
		    auto *window = gui::WGUI::GetInstance().FindFocusedWindow();
		    if(!window)
			    window = &get_cengine()->GetWindow();
		    if(!window || !window->IsValid())
			    return;
		    auto windowSize = (*window)->GetSize();
		    (*window)->SetCursorPos(windowSize / 2);
	    }),
	  luabind::def(
	    "get_controller_count", +[]() -> uint32_t { return platform::get_joysticks().size(); }),
	  luabind::def("get_controller_name", &platform::get_joystick_name), luabind::def("get_controller_axes", &platform::get_joystick_axes), luabind::def("get_controller_buttons", &platform::get_joystick_buttons),
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
		    auto &inputHandler = get_cgame()->GetInputCallbackHandler();
		    return inputHandler.AddLuaCallback(identifier, f);
	    }),
	  luabind::def(
	    "add_event_listener",
	    +[](const std::string &identifier, const Lua::func<void> &f) -> CallbackHandle {
		    auto &inputHandler = get_cgame()->GetInputCallbackHandler();
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
	    }))];

	auto soundMod = luabind::module(lua.GetState(), "sound");
	Lua::sound::register_library(soundMod);
	soundMod[luabind::def("register_aux_effect", static_cast<audio::PEffect (*)(const std::string &, const audio::EfxEaxReverbProperties &)>(&Lua::sound::register_aux_effect))];
	soundMod[luabind::def("register_aux_effect", static_cast<audio::PEffect (*)(const std::string &, const audio::EfxChorusProperties &)>(&Lua::sound::register_aux_effect))];
	soundMod[luabind::def("register_aux_effect", static_cast<audio::PEffect (*)(const std::string &, const audio::EfxDistortionProperties &)>(&Lua::sound::register_aux_effect))];
	soundMod[luabind::def("register_aux_effect", static_cast<audio::PEffect (*)(const std::string &, const audio::EfxEchoProperties &)>(&Lua::sound::register_aux_effect))];
	soundMod[luabind::def("register_aux_effect", static_cast<audio::PEffect (*)(const std::string &, const audio::EfxFlangerProperties &)>(&Lua::sound::register_aux_effect))];
	soundMod[luabind::def("register_aux_effect", static_cast<audio::PEffect (*)(const std::string &, const audio::EfxFrequencyShifterProperties &)>(&Lua::sound::register_aux_effect))];
	soundMod[luabind::def("register_aux_effect", static_cast<audio::PEffect (*)(const std::string &, const audio::EfxVocalMorpherProperties &)>(&Lua::sound::register_aux_effect))];
	soundMod[luabind::def("register_aux_effect", static_cast<audio::PEffect (*)(const std::string &, const audio::EfxPitchShifterProperties &)>(&Lua::sound::register_aux_effect))];
	soundMod[luabind::def("register_aux_effect", static_cast<audio::PEffect (*)(const std::string &, const audio::EfxRingModulatorProperties &)>(&Lua::sound::register_aux_effect))];
	soundMod[luabind::def("register_aux_effect", static_cast<audio::PEffect (*)(const std::string &, const audio::EfxAutoWahProperties &)>(&Lua::sound::register_aux_effect))];
	soundMod[luabind::def("register_aux_effect", static_cast<audio::PEffect (*)(const std::string &, const audio::EfxCompressor &)>(&Lua::sound::register_aux_effect))];
	soundMod[luabind::def("register_aux_effect", static_cast<audio::PEffect (*)(const std::string &, const audio::EfxEqualizer &)>(&Lua::sound::register_aux_effect))];
	soundMod[luabind::def("get_aux_effect", &Lua::sound::get_aux_effect)];
	soundMod[luabind::def("set_distance_model", &Lua::sound::set_distance_model)];
	soundMod[luabind::def("get_distance_model", &Lua::sound::get_distance_model)];
	soundMod[luabind::def("is_supported", &Lua::sound::is_supported)];
	soundMod[luabind::def("get_doppler_factor", &Lua::sound::get_doppler_factor)];
	soundMod[luabind::def("set_doppler_factor", &Lua::sound::set_doppler_factor)];
	soundMod[luabind::def("get_speed_of_sound", &Lua::sound::get_speed_of_sound)];
	soundMod[luabind::def("set_speed_of_sound", &Lua::sound::set_speed_of_sound)];
	soundMod[luabind::def("get_device_name", &Lua::sound::get_device_name)];
	soundMod[luabind::def("add_global_effect", static_cast<bool (*)(const std::string &)>(&Lua::sound::add_global_effect))];
	soundMod[luabind::def("add_global_effect", static_cast<bool (*)(const std::string &, audio::ISoundSystem::GlobalEffectFlag, const audio::EffectParams &)>(&Lua::sound::add_global_effect))];
	soundMod[luabind::def("remove_global_effect", &Lua::sound::remove_global_effect)];
	soundMod[luabind::def("set_global_effect_parameters", &Lua::sound::set_global_effect_parameters)];
	soundMod[luabind::def(
	  "get_duration", +[](const std::string &path) -> std::optional<float> {
		  auto absPath = pragma::asset::find_file(path, asset::Type::Sound);
		  if(absPath.has_value() == false)
			  return {};
		  float duration;
		  auto success = audio::util::get_duration(std::string {pragma::asset::get_asset_root_directory(asset::Type::Sound)} + "/" + *absPath, duration);
		  if(!success)
			  return {};
		  return duration;
	  })];

	Lua::RegisterLibraryEnums(lua.GetState(), "sound",
	  {{"GLOBAL_EFFECT_FLAG_NONE", math::to_integral(audio::ISoundSystem::GlobalEffectFlag::None)}, {"GLOBAL_EFFECT_FLAG_BIT_RELATIVE", math::to_integral(audio::ISoundSystem::GlobalEffectFlag::RelativeSounds)},
	    {"GLOBAL_EFFECT_FLAG_BIT_WORLD", math::to_integral(audio::ISoundSystem::GlobalEffectFlag::WorldSounds)}, {"GLOBAL_EFFECT_FLAG_ALL", math::to_integral(audio::ISoundSystem::GlobalEffectFlag::All)},

	    {"DISTANCE_MODEL_NONE", math::to_integral(audio::DistanceModel::None)}, {"DISTANCE_MODEL_INVERSE_CLAMPED", math::to_integral(audio::DistanceModel::InverseClamped)}, {"DISTANCE_MODEL_LINEAR_CLAMPED", math::to_integral(audio::DistanceModel::LinearClamped)},
	    {"DISTANCE_MODEL_EXPONENT_CLAMPED", math::to_integral(audio::DistanceModel::ExponentClamped)}, {"DISTANCE_MODEL_INVERSE", math::to_integral(audio::DistanceModel::Inverse)}, {"DISTANCE_MODEL_LINEAR", math::to_integral(audio::DistanceModel::Linear)},
	    {"DISTANCE_MODEL_EXPONENT", math::to_integral(audio::DistanceModel::Exponent)}});
	Lua::sound::register_enums(lua.GetState());

	auto defInLay = luabind::class_<InputBindingLayer>("InputBindingLayer");
	defInLay.def_readwrite("identifier", &InputBindingLayer::identifier);
	defInLay.def_readwrite("priority", &InputBindingLayer::priority);
	defInLay.def_readwrite("enabled", &InputBindingLayer::enabled);
	defInLay.scope[(luabind::def(
	                  "load",
	                  +[](lua::State *l, const udm::AssetData &data) -> Lua::var<bool, std::vector<std::shared_ptr<InputBindingLayer>>> {
		                  std::vector<std::shared_ptr<InputBindingLayer>> layers;
		                  std::string err;
		                  if(!InputBindingLayer::Load(data, layers, err))
			                  return luabind::object {l, false};
		                  return luabind::object {l, layers};
	                  }),
	  luabind::def(
	    "save", +[](lua::State *l, const udm::AssetData &data, const std::vector<std::shared_ptr<InputBindingLayer>> &layers) {
		    std::string err;
		    return InputBindingLayer::Save(layers, data, err);
	    }))];
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
			  if(string::compare(pair.second.GetBind(), cmd) == false)
				  continue;
			  std::string str;
			  if(!KeyToString(pair.first, &str))
				  continue;
			  boundKeys.push_back(str);
		  }
		  return boundKeys;
	  });
	inputMod[defInLay];
	pragma::LuaCore::define_custom_constructor<InputBindingLayer,
	  +[](const std::string &name) -> std::shared_ptr<InputBindingLayer> {
		  auto layer = create_input_binding_layer();
		  layer->identifier = name;
		  return layer;
	  },
	  const std::string &>(lua.GetState());

	scripting::lua_core::bindings::register_audio(lua.GetState());

	RegisterVulkanLuaInterface(lua);
}
