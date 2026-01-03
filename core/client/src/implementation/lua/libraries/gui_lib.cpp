// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :scripting.lua.libraries.gui;
import :client_state;
import :engine;
import :game;
import :gui;
import :scripting.lua;
import pragma.string.unicode;
//import pragma.scripting.lua;

static void initialize_element(pragma::gui::types::WIBase &p)
{
	auto data = p.GetUserData();
	if(data != nullptr) {
		auto wrapper = std::static_pointer_cast<pragma::gui::WILuaHandleWrapper>(data);
		wrapper->lua = true;
	}
}
static bool check_element(const std::string &name, pragma::gui::types::WIBase *el)
{
	if(el)
		return true;
	Con::CWAR << "Failed to create GUI element of type '" << name << "'!" << Con::endl;
	return false;
}
pragma::gui::types::WIBase *Lua::gui::create(pragma::CGame *game, const std::string &name, pragma::gui::types::WIBase &parent, int32_t x, int32_t y, uint32_t w, uint32_t h, float left, float top, float right, float bottom)
{
	auto *el = game ? game->CreateGUIElement(name, &parent) : pragma::gui::WGUI::GetInstance().Create(name, &parent);
	if(!check_element(name, el))
		return nullptr;
	el->SetPos(x, y);
	el->SetSize(w, h);
	el->SetAnchor(left, top, right, bottom);
	initialize_element(*el);
	return el;
}
pragma::gui::types::WIBase *Lua::gui::create(pragma::CGame *game, const std::string &name, pragma::gui::types::WIBase &parent, int32_t x, int32_t y, uint32_t w, uint32_t h)
{
	auto *el = game ? game->CreateGUIElement(name, &parent) : pragma::gui::WGUI::GetInstance().Create(name, &parent);
	if(!check_element(name, el))
		return nullptr;
	el->SetPos(x, y);
	el->SetSize(w, h);
	initialize_element(*el);
	return el;
}
pragma::gui::types::WIBase *Lua::gui::create(pragma::CGame *game, const std::string &name, pragma::gui::types::WIBase &parent, int32_t x, int32_t y)
{
	auto *el = game ? game->CreateGUIElement(name, &parent) : pragma::gui::WGUI::GetInstance().Create(name, &parent);
	if(!check_element(name, el))
		return nullptr;
	el->SetPos(x, y);
	initialize_element(*el);
	return el;
}
pragma::gui::types::WIBase *Lua::gui::create(pragma::CGame *game, const std::string &name, pragma::gui::types::WIBase *parent)
{
	auto *el = game ? game->CreateGUIElement(name, parent) : pragma::gui::WGUI::GetInstance().Create(name, parent);
	if(!check_element(name, el))
		return nullptr;
	initialize_element(*el);
	return el;
}
pragma::gui::types::WIBase *Lua::gui::create(pragma::CGame *game, const std::string &name)
{
	auto *el = game ? game->CreateGUIElement(name) : pragma::gui::WGUI::GetInstance().Create(name);
	if(!check_element(name, el))
		return nullptr;
	initialize_element(*el);
	return el;
}

pragma::gui::types::WIBase *Lua::gui::create_button(lua::State *l, const std::string &text, pragma::gui::types::WIBase &parent, int32_t x, int32_t y)
{
	auto *pButton = pragma::gui::WGUI::GetInstance().Create<pragma::gui::types::WIButton>(&parent);
	if(!pButton)
		return nullptr;
	pButton->SetText(text);
	pButton->SizeToContents();
	pButton->SetPos(x, y);
	initialize_element(*pButton);
	return pButton;
}
pragma::gui::types::WIBase *Lua::gui::create_button(lua::State *l, const std::string &text, pragma::gui::types::WIBase &parent)
{
	auto *pButton = pragma::gui::WGUI::GetInstance().Create<pragma::gui::types::WIButton>(&parent);
	if(!pButton)
		return nullptr;
	pButton->SetText(text);
	pButton->SizeToContents();
	initialize_element(*pButton);
	return pButton;
}
pragma::gui::types::WIBase *Lua::gui::create_button(lua::State *l, const std::string &text)
{
	auto *pButton = pragma::gui::WGUI::GetInstance().Create<pragma::gui::types::WIButton>();
	if(!pButton)
		return nullptr;
	pButton->SetText(text);
	pButton->SizeToContents();
	initialize_element(*pButton);
	return pButton;
}

static Lua::opt<Lua::mult<Lua::type<pragma::gui::types::WIBase>, Lua::type<pragma::gui::types::WIBase>, Lua::type<pragma::gui::types::WIBase>>> create_checkbox(lua::State *l, const std::string &label, pragma::gui::types::WIBase *parent)
{
	auto &wgui = pragma::gui::WGUI::GetInstance();
	auto *pContainer = wgui.Create<pragma::gui::types::WIBase>(parent);
	if(pContainer == nullptr)
		return Lua::nil;
	auto *pCheckbox = wgui.Create<pragma::gui::types::WICheckbox>(pContainer);
	if(pCheckbox == nullptr) {
		pContainer->Remove();
		return Lua::nil;
	}

	auto *pText = pragma::gui::WGUI::GetInstance().Create<pragma::gui::types::WIText>(pContainer);
	if(pText == nullptr) {
		pContainer->Remove();
		pCheckbox->Remove();
		return Lua::nil;
	}

	pText->SetText(label);
	pText->SizeToContents();
	pText->SetPos(pCheckbox->GetRight() + 5, pCheckbox->GetHeight() * 0.5f - pText->GetHeight() * 0.5f);

	pContainer->SizeToContents();

	return Lua::mult<Lua::type<pragma::gui::types::WIBase>, Lua::type<pragma::gui::types::WIBase>, Lua::type<pragma::gui::types::WIBase>> {l, pragma::gui::WGUILuaInterface::GetLuaObject(l, *pContainer), pragma::gui::WGUILuaInterface::GetLuaObject(l, *pCheckbox),
	  pragma::gui::WGUILuaInterface::GetLuaObject(l, *pText)};
}
Lua::opt<Lua::mult<Lua::type<pragma::gui::types::WIBase>, Lua::type<pragma::gui::types::WIBase>, Lua::type<pragma::gui::types::WIBase>>> Lua::gui::create_checkbox(lua::State *l, const std::string &label, pragma::gui::types::WIBase &parent) { return ::create_checkbox(l, label, &parent); }
Lua::opt<Lua::mult<Lua::type<pragma::gui::types::WIBase>, Lua::type<pragma::gui::types::WIBase>, Lua::type<pragma::gui::types::WIBase>>> Lua::gui::create_checkbox(lua::State *l, const std::string &label) { return ::create_checkbox(l, label, nullptr); }

pragma::gui::types::WIBase *Lua::gui::create_label(lua::State *l, const std::string &str, pragma::gui::types::WIBase &parent, int32_t x, int32_t y)
{
	auto *pText = pragma::gui::WGUI::GetInstance().Create<pragma::gui::types::WIText>(&parent);
	if(pText == nullptr)
		return nullptr;
	pText->SetText(str);
	pText->SizeToContents();
	pText->SetPos(x, y);
	initialize_element(*pText);
	return pText;
}
pragma::gui::types::WIBase *Lua::gui::create_label(lua::State *l, const std::string &str, pragma::gui::types::WIBase &parent)
{
	auto *pText = pragma::gui::WGUI::GetInstance().Create<pragma::gui::types::WIText>(&parent);
	if(pText == nullptr)
		return nullptr;
	pText->SetText(str);
	pText->SizeToContents();
	initialize_element(*pText);
	return pText;
}
pragma::gui::types::WIBase *Lua::gui::create_label(lua::State *l, const std::string &str)
{
	auto *pText = pragma::gui::WGUI::GetInstance().Create<pragma::gui::types::WIText>();
	if(pText == nullptr)
		return nullptr;
	pText->SetText(str);
	pText->SizeToContents();
	initialize_element(*pText);
	return pText;
}

void Lua::gui::register_element(const std::string &className, const classObject &classData)
{
	auto &manager = pragma::get_cgame()->GetLuaGUIManager();
	manager.RegisterGUIElement(className, const_cast<classObject &>(classData));
}

pragma::gui::types::WIBase *Lua::gui::get_base_element(const prosper::Window &window) { return pragma::gui::WGUI::GetInstance().GetBaseElement(&window); }
pragma::gui::types::WIBase *Lua::gui::get_base_element() { return pragma::gui::WGUI::GetInstance().GetBaseElement(); }

pragma::gui::types::WIBase *Lua::gui::get_element_under_cursor(lua::State *l, pragma::gui::types::WIBase &elRoot, const func<bool, pragma::gui::types::WIBase> &condition)
{
	return pragma::gui::WGUI::GetInstance().GetCursorGUIElement(&elRoot, [l, condition](pragma::gui::types::WIBase *el) -> bool {
		auto &oFunc = condition;
		auto result = CallFunction(
		  l,
		  [&oFunc, el](lua::State *l) -> StatusCode {
			  oFunc.push(l);
			  auto o = pragma::gui::WGUILuaInterface::GetLuaObject(l, *el);
			  o.push(l);
			  return StatusCode::Ok;
		  },
		  1);
		if(result != StatusCode::Ok)
			return true;
		auto r = false;
		if(IsSet(l, -1)) {
			r = CheckBool(l, -1);
			Pop(l, 1);
		}
		return r;
	});
}
pragma::gui::types::WIBase *Lua::gui::get_element_under_cursor(lua::State *l, const prosper::Window *window, const func<bool, pragma::gui::types::WIBase> &condition)
{
	auto *el = pragma::gui::WGUI::GetInstance().GetBaseElement(window);
	if(!el)
		return nullptr;
	return get_element_under_cursor(l, *el, condition);
}
pragma::gui::types::WIBase *Lua::gui::get_element_under_cursor(lua::State *l, const func<bool, pragma::gui::types::WIBase> &condition) { return get_element_under_cursor(l, nullptr, condition); }
pragma::gui::types::WIBase *Lua::gui::get_element_under_cursor(lua::State *l, const prosper::Window *window)
{
	auto *el = pragma::gui::WGUI::GetInstance().GetBaseElement(window);
	if(!el)
		return nullptr;
	return pragma::gui::WGUI::GetInstance().GetCursorGUIElement(el, [l](pragma::gui::types::WIBase *el) -> bool { return true; }, window);
}
pragma::gui::types::WIBase *Lua::gui::get_element_under_cursor(lua::State *l, pragma::gui::types::WIBase &elRoot)
{
	return pragma::gui::WGUI::GetInstance().GetCursorGUIElement(&elRoot, [l](pragma::gui::types::WIBase *el) -> bool { return true; });
}

pragma::gui::types::WIBase *Lua::gui::get_element_at_position(lua::State *l, prosper::Window *window, pragma::gui::types::WIBase *baseElement, int32_t x, int32_t y, const func<bool, pragma::gui::types::WIBase> &condition)
{
	std::function<bool(pragma::gui::types::WIBase *)> fCondition = nullptr;

	fCondition = [condition, l](pragma::gui::types::WIBase *el) -> bool {
		auto result = CallFunction(
		  l,
		  [condition, el](lua::State *l) -> StatusCode {
			  condition.push(l);
			  auto o = pragma::gui::WGUILuaInterface::GetLuaObject(l, *el);
			  o.push(l);
			  return StatusCode::Ok;
		  },
		  1);
		if(result != StatusCode::Ok)
			return true;
		if(IsSet(l, -1) == false)
			return false;
		return CheckBool(l, -1);
	};

	return pragma::gui::WGUI::GetInstance().GetGUIElement(baseElement, x, y, fCondition, window);
}
pragma::gui::types::WIBase *Lua::gui::get_element_at_position(lua::State *l, prosper::Window *window, pragma::gui::types::WIBase *baseElement, int32_t x, int32_t y) { return pragma::gui::WGUI::GetInstance().GetGUIElement(baseElement, x, y, nullptr, window); }
pragma::gui::types::WIBase *Lua::gui::get_element_at_position(lua::State *l, prosper::Window *window, pragma::gui::types::WIBase *baseElement)
{
	int32_t x;
	int32_t y;
	pragma::gui::WGUI::GetInstance().GetMousePos(x, y);
	return pragma::gui::WGUI::GetInstance().GetGUIElement(baseElement, x, y, nullptr, window);
}
pragma::gui::types::WIBase *Lua::gui::get_element_at_position(lua::State *l, prosper::Window *window)
{
	int32_t x;
	int32_t y;
	pragma::gui::WGUI::GetInstance().GetMousePos(x, y);
	return pragma::gui::WGUI::GetInstance().GetGUIElement(nullptr, x, y, nullptr, window);
}
pragma::gui::types::WIBase *Lua::gui::get_element_at_position(lua::State *l)
{
	int32_t x;
	int32_t y;
	pragma::gui::WGUI::GetInstance().GetMousePos(x, y);
	return pragma::gui::WGUI::GetInstance().GetGUIElement(nullptr, x, y, nullptr);
}

pragma::gui::types::WIBase *Lua::gui::get_focused_element(lua::State *l) { return pragma::gui::WGUI::GetInstance().GetFocusedElement(); }

pragma::gui::types::WIBase *Lua::gui::get_focused_element(lua::State *l, prosper::Window &window) { return pragma::gui::WGUI::GetInstance().GetFocusedElement(&window); }

static bool register_skin(lua::State *l, const std::string &skin, const luabind::tableT<void> &vars, const luabind::tableT<void> &skinData, const std::string *baseName)
{
	if(l != pragma::get_client_state()->GetGUILuaState()) {
		Lua::Error(l, "Attempted to register GUI skin with client lua state! This is not allowed, skins can only be registered with GUI lua state!");
		return false;
	}

	auto s = std::make_unique<pragma::gui::WILuaSkin>();
	pragma::gui::WILuaSkin::Settings settings;
	settings.vars = vars;
	settings.skin = skinData;
	if(baseName)
		settings.base = dynamic_cast<pragma::gui::WILuaSkin *>(pragma::gui::WGUI::GetInstance().GetSkin(*baseName));
	s->Initialize(l, settings);
	pragma::gui::WGUI::GetInstance().RegisterSkin(skin, std::move(s));
	return true;
}
void Lua::gui::register_default_skin(const std::string &vars, const std::string &skinData)
{
	auto *skin = dynamic_cast<pragma::gui::WILuaSkin *>(pragma::gui::WGUI::GetInstance().GetSkin("default"));
	if(!skin)
		return;
	auto *l = pragma::get_client_state()->GetGUILuaState();
	std::string errMsg;
	auto resVars = pragma::scripting::lua_core::run_string(l, "return " + vars, "register_default_skin", 1, &errMsg);
	CheckTable(l, -1);
	if(resVars != StatusCode::Ok) {
		pragma::scripting::lua_core::raise_error(l, errMsg);
		return;
	}
	auto tVars = luabind::object {luabind::from_stack(l, -1)};
	Pop(l);
	auto resSkinData = pragma::scripting::lua_core::run_string(l, "return " + skinData, "register_default_skin", 1, &errMsg);
	if(resSkinData != StatusCode::Ok) {
		pragma::scripting::lua_core::raise_error(l, errMsg);
		return;
	}
	CheckTable(l, -1);
	auto tSkinData = luabind::object {luabind::from_stack(l, -1)};
	Pop(l);

	pragma::gui::WILuaSkin::Settings settings;
	settings.vars = tVars;
	settings.skin = tSkinData;
	skin->MergeInto(l, settings);
}
bool Lua::gui::register_skin(lua::State *l, const std::string &skin, const luabind::tableT<void> &vars, const luabind::tableT<void> &skinData) { return ::register_skin(l, skin, vars, skinData, nullptr); }
bool Lua::gui::register_skin(lua::State *l, const std::string &skin, const luabind::tableT<void> &vars, const luabind::tableT<void> &skinData, const std::string &baseName) { return ::register_skin(l, skin, vars, skinData, &baseName); }

void Lua::gui::set_skin(const std::string &skin) { pragma::gui::WGUI::GetInstance().SetSkin(skin); }

bool Lua::gui::skin_exists(const std::string &name)
{
	auto *skin = pragma::gui::WGUI::GetInstance().GetSkin(name);
	return (skin != nullptr) ? true : false;
}

pragma::platform::Cursor::Shape Lua::gui::get_cursor() { return pragma::gui::WGUI::GetInstance().GetCursor(); }
void Lua::gui::set_cursor(pragma::platform::Cursor::Shape shape) { pragma::gui::WGUI::GetInstance().SetCursor(shape); }
pragma::platform::CursorMode Lua::gui::get_cursor_input_mode() { return pragma::gui::WGUI::GetInstance().GetCursorInputMode(); }
void Lua::gui::set_cursor_input_mode(pragma::platform::CursorMode mode) { pragma::gui::WGUI::GetInstance().SetCursorInputMode(mode); }
Vector2i Lua::gui::get_window_size(lua::State *l)
{
	auto &context = pragma::gui::WGUI::GetInstance().GetContext();
	auto &window = context.GetWindow();
	return window->GetSize();
}

std::shared_ptr<prosper::IImage> Lua::gui::create_color_image(uint32_t w, uint32_t h, prosper::ImageUsageFlags usageFlags, prosper::ImageLayout initialLayout, bool msaa)
{
	auto &context = pragma::get_cengine()->GetRenderContext();
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
		imgCreateInfo.samples = pragma::gui::wGUI::MSAA_SAMPLE_COUNT;
	return context.CreateImage(imgCreateInfo);
}

std::shared_ptr<prosper::RenderTarget> Lua::gui::create_render_target(uint32_t w, uint32_t h, bool enableMsaa, bool enableSampling)
{
	auto &context = pragma::get_cengine()->GetRenderContext();
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
		imgCreateInfo.samples = pragma::gui::wGUI::MSAA_SAMPLE_COUNT;
	auto depthStencilImg = context.CreateImage(imgCreateInfo);

	auto tex = context.CreateTexture(prosper::util::TextureCreateInfo {}, *img, prosper::util::ImageViewCreateInfo {}, prosper::util::SamplerCreateInfo {});
	if(!tex)
		return nullptr;

	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	imgViewCreateInfo.aspectFlags = prosper::ImageAspectFlags::StencilBit;
	auto depthStencilTex = context.CreateTexture({}, *depthStencilImg, imgViewCreateInfo);

	auto &rp = enableMsaa ? pragma::gui::WGUI::GetInstance().GetMsaaRenderPass() : context.GetWindow().GetStagingRenderPass();
	return context.CreateRenderTarget({tex, depthStencilTex}, rp.shared_from_this());
}

bool Lua::gui::inject_mouse_input(pragma::platform::MouseButton button, pragma::platform::KeyState state, pragma::platform::Modifier mods, const ::Vector2i &pCursorPos)
{
	::Vector2i cursorPos = {};
	auto &gui = pragma::gui::WGUI::GetInstance();

	gui.GetMousePos(cursorPos.x, cursorPos.y);
	auto &window = pragma::get_cengine()->GetWindow();
	window->SetCursorPos({pCursorPos.x, pCursorPos.y});

	auto b = gui.HandleMouseInput(window, static_cast<pragma::platform::MouseButton>(button), static_cast<pragma::platform::KeyState>(state), static_cast<pragma::platform::Modifier>(mods));
	window->SetCursorPos(cursorPos);
	return b;
}
bool Lua::gui::inject_mouse_input(pragma::platform::MouseButton button, pragma::platform::KeyState state, pragma::platform::Modifier mods)
{
	::Vector2i cursorPos = {};
	auto &gui = pragma::gui::WGUI::GetInstance();
	auto &window = pragma::get_cengine()->GetWindow();
	return gui.HandleMouseInput(window, static_cast<pragma::platform::MouseButton>(button), static_cast<pragma::platform::KeyState>(state), static_cast<pragma::platform::Modifier>(mods));
}
bool Lua::gui::inject_keyboard_input(pragma::platform::Key key, pragma::platform::KeyState state, pragma::platform::Modifier mods)
{
	auto &window = pragma::get_cengine()->GetWindow();
	return pragma::gui::WGUI::GetInstance().HandleKeyboardInput(window, key, 0, state, mods);
}
bool Lua::gui::inject_char_input(const std::string &c)
{
	auto &window = pragma::get_cengine()->GetWindow();
	auto b = false;
	if(!c.empty())
		b = pragma::gui::WGUI::GetInstance().HandleCharInput(window, c.front());
	return b;
}
bool Lua::gui::inject_scroll_input(lua::State *l, const ::Vector2 &offset, const ::Vector2i &pCursorPos)
{
	::Vector2i cursorPos = {};
	auto &gui = pragma::gui::WGUI::GetInstance();

	gui.GetMousePos(cursorPos.x, cursorPos.y);
	auto &window = pragma::get_cengine()->GetWindow();
	window->SetCursorPos({pCursorPos.x, pCursorPos.y});

	auto b = gui.HandleScrollInput(window, offset);
	window->SetCursorPos(cursorPos);
	return b;
}
bool Lua::gui::inject_scroll_input(lua::State *l, const ::Vector2 &offset)
{
	::Vector2i cursorPos = {};
	auto &gui = pragma::gui::WGUI::GetInstance();
	auto &window = pragma::get_cengine()->GetWindow();
	return gui.HandleScrollInput(window, offset);
}

float Lua::gui::RealTime(lua::State *l) { return pragma::get_client_state()->RealTime(); }

float Lua::gui::DeltaTime(lua::State *l) { return pragma::get_client_state()->DeltaTime(); }

float Lua::gui::LastThink(lua::State *l) { return pragma::get_client_state()->LastThink(); }
