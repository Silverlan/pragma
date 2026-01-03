// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/console/helper.hpp"

module pragma.client;

import :console.commands;

class GUIDebugCursorManager {
  public:
	GUIDebugCursorManager();
	~GUIDebugCursorManager();
	bool Initialize();
	void SetTargetGUIElementOverride(pragma::gui::types::WIBase *el);
  private:
	static std::string GetElementInfo(pragma::gui::types::WIBase &el);
	void Clear();
	void OnThink();
	bool ShouldPass(pragma::gui::types::WIBase &el) const;
	void SetTargetGUIElement(pragma::gui::types::WIBase *optEl, bool clear);
	void SetWindow(prosper::Window &window);

	void SelectNextParentInHierarchy();
	void SelectNextChildInHierarchy();

	CallbackHandle m_cbThink = {};
	CallbackHandle m_cbScroll = {};
	CallbackHandle m_cbMiddleMouse = {};
	CallbackHandle m_cbOnClose = {};
	std::weak_ptr<prosper::Window> m_curWindow {};
	pragma::gui::WIHandle m_hText = {};
	pragma::gui::WIHandle m_targetElementOverride;
	std::array<pragma::gui::WIHandle, 4> m_borderElements = {};
	std::array<pragma::gui::WIHandle, 4> m_borderElementsConstrained = {};
	std::vector<pragma::gui::WIHandle> m_cursorElementList = {}; // Last element is bottom-most element in hierarchy, all elements above are parents
};

GUIDebugCursorManager::GUIDebugCursorManager() {}

GUIDebugCursorManager::~GUIDebugCursorManager() { Clear(); }

void GUIDebugCursorManager::Clear()
{
	if(m_cbThink.IsValid())
		m_cbThink.Remove();
	if(m_cbScroll.IsValid())
		m_cbScroll.Remove();
	if(m_cbMiddleMouse.IsValid())
		m_cbMiddleMouse.Remove();
	if(m_cbOnClose.IsValid())
		m_cbOnClose.Remove();
	if(m_hText.IsValid())
		m_hText->Remove();
	for(auto &hEl : m_borderElements) {
		if(hEl.IsValid())
			hEl->Remove();
	}
	for(auto &hEl : m_borderElementsConstrained) {
		if(hEl.IsValid())
			hEl->Remove();
	}
}

bool GUIDebugCursorManager::Initialize()
{
	auto &gui = pragma::gui::WGUI::GetInstance();
	auto *pText = gui.Create<pragma::gui::types::WIText>();
	if(pText == nullptr)
		return false;
	m_hText = pText->GetHandle();
	pText->SetZPos(std::numeric_limits<int32_t>::max());
	pText->SetVisible(false);
	pText->EnableShadow(true);
	pText->SetShadowOffset(Vector2i(1, 1));
	pText->SetColor(colors::Orange);

	for(auto &hEl : m_borderElements) {
		auto *el = pragma::gui::WGUI::GetInstance().Create<pragma::gui::types::WIRect>();
		el->SetColor(colors::Red);
		el->SetZPos(std::numeric_limits<int>::max());
		el->SetVisible(false);
		hEl = el->GetHandle();
	}
	for(auto &hEl : m_borderElementsConstrained) {
		auto *el = pragma::gui::WGUI::GetInstance().Create<pragma::gui::types::WIRect>();
		el->SetColor(colors::Aqua);
		el->SetAlpha(0.75f);
		el->SetZPos(std::numeric_limits<int>::max());
		el->SetVisible(false);
		hEl = el->GetHandle();
	}

	m_cbScroll = pragma::get_cengine()->AddCallback("OnScrollInput", FunctionCallback<bool, std::reference_wrapper<prosper::Window>, Vector2>::CreateWithOptionalReturn([this](bool *reply, std::reference_wrapper<prosper::Window> window, Vector2 scrollAmount) -> CallbackReturnType {
		if(scrollAmount.y == 0.f)
			return CallbackReturnType::NoReturnValue;
		*reply = true;
		if(scrollAmount.y < 0.f)
			SelectNextParentInHierarchy();
		else
			SelectNextChildInHierarchy();
		return CallbackReturnType::HasReturnValue;
	}));
	m_cbMiddleMouse = pragma::get_cengine()->AddCallback("OnMouseInput",
	  FunctionCallback<bool, std::reference_wrapper<prosper::Window>, pragma::platform::MouseButton, pragma::platform::KeyState, pragma::platform::Modifier>::CreateWithOptionalReturn(
	    [this](bool *reply, std::reference_wrapper<prosper::Window> window, pragma::platform::MouseButton button, pragma::platform::KeyState state, pragma::platform::Modifier mods) -> CallbackReturnType {
		    if(button == pragma::platform::MouseButton::Middle) {
			    *reply = true;
			    if(state == pragma::platform::KeyState::Press && !m_cursorElementList.empty()) {
				    auto &hEl = m_cursorElementList.front();
				    if(hEl.IsValid()) {
					    auto *el = hEl.get();
					    auto *cl = pragma::get_cengine()->GetClientState();
					    auto *game = cl ? cl->GetGameState() : nullptr;
					    if(game) {
						    auto *l = game->GetLuaState();
						    luabind::object g = luabind::globals(l);
						    // Assign UI element to global variable x
						    g["x"] = pragma::gui::WGUILuaInterface::GetLuaObject(l, *el);
						    std::stringstream ss;
						    el->Print(ss);
						    Con::COUT << "Assigned element '" << ss.str() << "' to global variable 'x'!" << Con::endl;
					    }
				    }
			    }
			    return CallbackReturnType::HasReturnValue;
		    }
		    return CallbackReturnType::NoReturnValue;
	    }));
	auto *client = pragma::get_client_state();
	m_cbOnClose = client->AddCallback("OnClose", FunctionCallback<void>::Create([this]() { Clear(); }));
	m_cbThink = client->AddCallback("Think", FunctionCallback<void>::Create([this]() { OnThink(); }));
	return true;
}

bool GUIDebugCursorManager::ShouldPass(pragma::gui::types::WIBase &el) const
{
	if(el.IsVisible() == false || &el == m_hText.get() || el.IsDescendantOf(const_cast<pragma::gui::types::WIBase *>(m_hText.get())))
		return false;
	for(auto &hEl : m_borderElements) {
		if(&el == hEl.get())
			return false;
	}
	for(auto &hEl : m_borderElementsConstrained) {
		if(&el == hEl.get())
			return false;
	}
	return true;
}

void GUIDebugCursorManager::SetWindow(prosper::Window &window)
{
	auto pwindow = window.shared_from_this();
	if(m_curWindow.lock() == pwindow)
		return;
	m_curWindow = pwindow;
	auto *elBase = pragma::gui::WGUI::GetInstance().GetBaseElement(&window);
	if(!elBase)
		return;
	if(m_hText.IsValid())
		m_hText->SetParent(elBase);
	for(auto &hEl : m_borderElements) {
		if(!hEl.IsValid())
			continue;
		hEl->SetParent(elBase);
	}
	for(auto &hEl : m_borderElementsConstrained) {
		if(!hEl.IsValid())
			continue;
		hEl->SetParent(elBase);
	}
	m_cursorElementList.clear();
}

std::string GUIDebugCursorManager::GetElementInfo(pragma::gui::types::WIBase &el)
{
	std::stringstream ss;
	auto pos = el.GetAbsolutePos();
	auto size = el.GetSize();

	ss << "Element: " << el << "; Class: " << el.GetClass() << "; Name: " << el.GetName() << "; Index: " << el.GetIndex() << "; Size: (" << size.x << "," << size.y << ")"
	   << "; Pos: (" << pos.x << "," << pos.y << ")";
	auto debugInfo = el.GetDebugInfo();
	if(debugInfo.empty() == false)
		ss << "; Debug Info: " << debugInfo;
	return ss.str();
}

void GUIDebugCursorManager::SelectNextParentInHierarchy()
{
	if(m_cursorElementList.empty() || m_cursorElementList.front().IsValid() == false)
		return;
	auto &hEl = m_cursorElementList.front();
	auto *parent = hEl->GetParent();
	if(parent)
		SetTargetGUIElement(parent, false);
}
void GUIDebugCursorManager::SelectNextChildInHierarchy()
{
	auto isFirstElement = true;
	for(auto it = m_cursorElementList.begin(); it != m_cursorElementList.end();) {
		auto first = isFirstElement;
		isFirstElement = false;
		auto &hEl = *it;
		if(hEl.IsValid() == false || first) {
			it = m_cursorElementList.erase(it);
			continue;
		}
		auto *el = it->get();
		m_cursorElementList.erase(it);
		SetTargetGUIElement(el, false);
		break;
	}
}

void GUIDebugCursorManager::SetTargetGUIElementOverride(pragma::gui::types::WIBase *el)
{
	m_targetElementOverride = el ? el->GetHandle() : pragma::gui::WIHandle {};
	SetTargetGUIElement(el, true);
}

void GUIDebugCursorManager::SetTargetGUIElement(pragma::gui::types::WIBase *optEl, bool clear)
{
	auto dbgGUIVisible = (optEl != nullptr);
	if(m_hText.IsValid())
		m_hText->SetVisible(dbgGUIVisible);
	for(auto &hEl : m_borderElements) {
		if(hEl.IsValid())
			hEl->SetVisible(dbgGUIVisible);
	}
	for(auto &hEl : m_borderElementsConstrained) {
		if(hEl.IsValid())
			hEl->SetVisible(dbgGUIVisible);
	}

	if(optEl == nullptr) {
		m_cursorElementList.clear();
		return;
	}
	auto &el = *optEl;
	if(m_cursorElementList.empty() == false && m_cursorElementList.back().get() == &el)
		return;
	if(clear)
		m_cursorElementList.clear();
	m_cursorElementList.insert(m_cursorElementList.begin(), el.GetHandle());

	auto *pText = static_cast<pragma::gui::types::WIText *>(m_hText.get());
	if(pText) {
		pText->SetText(GetElementInfo(el));
		Con::COUT << pText->GetText().cpp_str() << Con::endl;
	}

	auto *l = pragma::get_cgame()->GetLuaState();
	if(l) {
		// Assign element to global 'debug_ui_element' Lua variable
		auto o = pragma::gui::WGUILuaInterface::GetLuaObject(l, el);
		if(o)
			luabind::globals(l)["debug_ui_element"] = o;
		else
#ifdef WINDOWS_CLANG_COMPILER_FIX
			luabind::globals(l)["debug_ui_element"] = luabind::object {};
#else
			luabind::globals(l)["debug_ui_element"] = Lua::nil;
#endif
	}

	// Initialize border to highlight the element
	auto pos = el.GetAbsolutePos();
	auto size = el.GetSize();
	auto constrainedEndPos = pos + Vector2 {size};
	auto *parent = el.GetParent();
	while(parent) {
		auto endPos = parent->GetAbsolutePos() + Vector2 {parent->GetSize()};
		constrainedEndPos.x = pragma::math::min(constrainedEndPos.x, endPos.x);
		constrainedEndPos.y = pragma::math::min(constrainedEndPos.y, endPos.y);

		parent = parent->GetParent();
	}
	auto constrainedSize = constrainedEndPos - pos;
	constrainedSize.x = pragma::math::max(constrainedSize.x, 0.f);
	constrainedSize.y = pragma::math::max(constrainedSize.y, 0.f);

	auto fInitBorder = [](std::array<pragma::gui::WIHandle, 4> &elements, const Vector2i &pos, const Vector2i &size) {
		auto *top = elements.at(0).get();
		auto *right = elements.at(1).get();
		auto *bottom = elements.at(2).get();
		auto *left = elements.at(3).get();
		if(top) {
			top->SetPos(pos.x - 1, pos.y - 1);
			top->SetSize(size.x + 2, 1);
		}

		if(right) {
			right->SetPos(pos.x + size.x, pos.y - 1);
			right->SetSize(1, size.y + 2);
		}

		if(bottom) {
			bottom->SetPos(pos.x - 1, pos.y + size.y);
			bottom->SetSize(size.x + 2, 1);
		}

		if(left) {
			left->SetPos(pos.x - 1, pos.y - 1);
			left->SetSize(1, size.y + 2);
		}
	};
	fInitBorder(m_borderElements, pos, size);
	fInitBorder(m_borderElementsConstrained, pos, constrainedSize);

	std::string t = "\t";
	auto *pParent = el.GetParent();
	while(pParent != nullptr) {
		Con::COUT << t << GetElementInfo(*pParent) << Con::endl;
		pParent = pParent->GetParent();
		t += "\t";
	}
	if(pText)
		pText->SizeToContents();
}

void GUIDebugCursorManager::OnThink()
{
	if(m_hText.IsValid() == false)
		return;
	auto &gui = pragma::gui::WGUI::GetInstance();
	auto *window = gui.FindWindowUnderCursor();
	if(window)
		SetWindow(*window);
	auto *pText = static_cast<pragma::gui::types::WIText *>(m_hText.get());
	auto *pEl = gui.GetCursorGUIElement(nullptr, [this](pragma::gui::types::WIBase *pEl) -> bool { return ShouldPass(*pEl); }, window);
	if(!m_targetElementOverride.IsValid())
		SetTargetGUIElement(pEl, true);

	if(m_hText.IsValid()) {
		int32_t x, y;
		gui.GetMousePos(x, y, window);
		m_hText->SetPos(x, y);
	}
}

static void debug_gui_cursor(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	static std::unique_ptr<GUIDebugCursorManager> s_dbgManager = nullptr;
	if(s_dbgManager != nullptr && argv.empty()) {
		s_dbgManager = nullptr;
		return;
	}
	if(!s_dbgManager) {
		s_dbgManager = std::make_unique<GUIDebugCursorManager>();
		if(s_dbgManager->Initialize() == false)
			s_dbgManager = nullptr;
	}
	if(!argv.empty()) {
		auto &elName = argv.front();
		auto *el = pragma::gui::WGUI::GetInstance().FindByFilter([&elName](pragma::gui::types::WIBase &el) -> bool { return pragma::string::compare(el.GetName(), elName, false); });
		if(!el) {
			Con::CWAR << "Unable to find element by name '" << elName << "'!" << Con::endl;
			return;
		}
		s_dbgManager->SetTargetGUIElementOverride(el);
	}
}
namespace {
	auto UVN = pragma::console::client::register_command("debug_gui_cursor", &debug_gui_cursor, pragma::console::ConVarFlags::None, "Prints information about the GUI element currently hovered over by the cursor.");
}

static void debug_dump_font_glyph_map(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	auto &wgui = pragma::gui::WGUI::GetInstance();
	if(argv.empty()) {
		Con::CWAR << "No font specified!" << Con::endl;
		return;
	}
	auto &fontName = argv.front();
	auto font = pragma::gui::FontManager::GetFont(fontName);
	if(font == nullptr) {
		Con::CWAR << "No font by name '" << fontName << "' found!" << Con::endl;
		return;
	}
	auto glyphMap = font->GetGlyphMap();
	if(glyphMap == nullptr) {
		Con::CWAR << "Font '" << fontName << "' has invalid glyph map!" << Con::endl;
		return;
	}
	auto &glyphImg = glyphMap->GetImage();
	std::string fileName = "tmp/font_" + fontName + "_glyph_map";

	pragma::image::TextureInfo texInfo {};
	texInfo.containerFormat = pragma::image::TextureInfo::ContainerFormat::DDS;
	if(!prosper::util::save_texture(fileName, glyphImg, texInfo)) {
		Con::CWAR << "Failed to save glyph map as '" << fileName << "'!" << Con::endl;
		return;
	}
}
namespace {
	auto UVN = pragma::console::client::register_command("debug_dump_font_glyph_map", &debug_dump_font_glyph_map, pragma::console::ConVarFlags::None, "Dumps the glyph map for the specified font to an image file.");
}

static void debug_font_glyph_map(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	auto &wgui = pragma::gui::WGUI::GetInstance();
	auto *el = static_cast<pragma::gui::types::WITexturedRect *>(wgui.GetBaseElement()->FindDescendantByName("dbg_glyph_map"));
	if(el) {
		el->RemoveSafely();
		return;
	}
	if(argv.empty()) {
		Con::CWAR << "No font specified!" << Con::endl;
		return;
	}
	auto &fontName = argv.front();
	auto font = pragma::gui::FontManager::GetFont(fontName);
	if(font == nullptr) {
		Con::CWAR << "No font by name '" << fontName << "' found!" << Con::endl;
		return;
	}
	auto glyphMap = font->GetGlyphMap();
	if(glyphMap == nullptr) {
		Con::CWAR << "Font '" << fontName << "' has invalid glyph map!" << Con::endl;
		return;
	}
	auto &glyphImg = glyphMap->GetImage();
	auto aspectRatio = glyphImg.GetWidth() / static_cast<float>(glyphImg.GetHeight());
	auto w = pragma::get_cengine()->GetRenderResolution().x;
	auto h = pragma::math::round(w / aspectRatio);
	auto scale = 5.f;
	w *= scale;
	h *= scale;
	el = wgui.Create<pragma::gui::types::WITexturedRect>();
	el->SetZPos(std::numeric_limits<int32_t>::max());
	el->SetName("dbg_glyph_map");
	el->SetTexture(*glyphMap);
	el->SetSize(w, h);
}
namespace {
	auto UVN = pragma::console::client::register_command("debug_font_glyph_map", &debug_font_glyph_map, pragma::console::ConVarFlags::None, "Displays the glyph map for the specified font.");
}
