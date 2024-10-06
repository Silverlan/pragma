/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/console/c_cvar_global_functions.h"
#include "pragma/gui/wgui_luainterface.h"
#include <util_image.hpp>
#include <wgui/types/witext.h>
#include <wgui/types/wirect.h>
#include <wgui/types/wiroot.h>
#include <prosper_window.hpp>
#include <fsys/ifile.hpp>

extern DLLCLIENT CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

import pragma.string.unicode;

class GUIDebugCursorManager {
  public:
	GUIDebugCursorManager();
	~GUIDebugCursorManager();
	bool Initialize();
  private:
	static std::string GetElementInfo(WIBase &el);
	void Clear();
	void OnThink();
	bool ShouldPass(WIBase &el) const;
	void SetTargetGUIElement(WIBase *optEl, bool clear);
	void SetWindow(prosper::Window &window);

	void SelectNextParentInHierarchy();
	void SelectNextChildInHierarchy();

	CallbackHandle m_cbThink = {};
	CallbackHandle m_cbScroll = {};
	CallbackHandle m_cbMiddleMouse = {};
	CallbackHandle m_cbOnClose = {};
	std::weak_ptr<prosper::Window> m_curWindow {};
	WIHandle m_hText = {};
	std::array<WIHandle, 4> m_borderElements = {};
	std::array<WIHandle, 4> m_borderElementsConstrained = {};
	std::vector<WIHandle> m_cursorElementList = {}; // Last element is bottom-most element in hierarchy, all elements above are parents
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
	auto &gui = WGUI::GetInstance();
	auto *pText = gui.Create<WIText>();
	if(pText == nullptr)
		return false;
	m_hText = pText->GetHandle();
	pText->SetZPos(std::numeric_limits<int32_t>::max());
	pText->SetVisible(false);
	pText->EnableShadow(true);
	pText->SetShadowOffset(Vector2i(1, 1));
	pText->SetColor(Color::Orange);

	for(auto &hEl : m_borderElements) {
		auto *el = WGUI::GetInstance().Create<WIRect>();
		el->SetColor(Color::Red);
		el->SetZPos(std::numeric_limits<int>::max());
		el->SetVisible(false);
		hEl = el->GetHandle();
	}
	for(auto &hEl : m_borderElementsConstrained) {
		auto *el = WGUI::GetInstance().Create<WIRect>();
		el->SetColor(Color::Aqua);
		el->SetAlpha(0.75f);
		el->SetZPos(std::numeric_limits<int>::max());
		el->SetVisible(false);
		hEl = el->GetHandle();
	}

	m_cbScroll = c_engine->AddCallback("OnScrollInput", FunctionCallback<bool, std::reference_wrapper<prosper::Window>, Vector2>::CreateWithOptionalReturn([this](bool *reply, std::reference_wrapper<prosper::Window> window, Vector2 scrollAmount) -> CallbackReturnType {
		if(scrollAmount.y == 0.f)
			return CallbackReturnType::NoReturnValue;
		*reply = true;
		if(scrollAmount.y < 0.f)
			SelectNextParentInHierarchy();
		else
			SelectNextChildInHierarchy();
		return CallbackReturnType::HasReturnValue;
	}));
	m_cbMiddleMouse = c_engine->AddCallback("OnMouseInput",
	  FunctionCallback<bool, std::reference_wrapper<prosper::Window>, GLFW::MouseButton, GLFW::KeyState, GLFW::Modifier>::CreateWithOptionalReturn(
	    [this](bool *reply, std::reference_wrapper<prosper::Window> window, GLFW::MouseButton button, GLFW::KeyState state, GLFW::Modifier mods) -> CallbackReturnType {
		    if(button == GLFW::MouseButton::Middle) {
			    *reply = true;
			    if(state == GLFW::KeyState::Press && !m_cursorElementList.empty()) {
				    auto &hEl = m_cursorElementList.front();
				    if(hEl.IsValid()) {
					    auto *el = hEl.get();
					    auto *cl = c_engine->GetClientState();
					    auto *game = cl ? cl->GetGameState() : nullptr;
					    if(game) {
						    auto *l = game->GetLuaState();
						    luabind::object g = luabind::globals(l);
						    // Assign UI element to global variable x
						    g["x"] = WGUILuaInterface::GetLuaObject(l, *el);
						    std::stringstream ss;
						    el->Print(ss);
						    Con::cout << "Assigned element '" << ss.str() << "' to global variable 'x'!" << Con::endl;
					    }
				    }
			    }
			    return CallbackReturnType::HasReturnValue;
		    }
		    return CallbackReturnType::NoReturnValue;
	    }));
	m_cbOnClose = client->AddCallback("OnClose", FunctionCallback<void>::Create([this]() { Clear(); }));

	m_cbThink = client->AddCallback("Think", FunctionCallback<void>::Create([this]() { OnThink(); }));
	return true;
}

bool GUIDebugCursorManager::ShouldPass(WIBase &el) const
{
	if(el.IsVisible() == false || &el == m_hText.get() || el.IsDescendantOf(const_cast<WIBase *>(m_hText.get())))
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
	auto *elBase = WGUI::GetInstance().GetBaseElement(&window);
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

std::string GUIDebugCursorManager::GetElementInfo(WIBase &el)
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

void GUIDebugCursorManager::SetTargetGUIElement(WIBase *optEl, bool clear)
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

	auto *pText = static_cast<WIText *>(m_hText.get());
	if(pText) {
		pText->SetText(GetElementInfo(el));
		Con::cout << pText->GetText().cpp_str() << Con::endl;
	}

	auto *l = c_game->GetLuaState();
	if(l) {
		// Assign element to global 'debug_ui_element' Lua variable
		auto o = WGUILuaInterface::GetLuaObject(l, el);
		if(o)
			luabind::globals(l)["debug_ui_element"] = o;
		else
			luabind::globals(l)["debug_ui_element"] = Lua::nil;
	}

	// Initialize border to highlight the element
	auto pos = el.GetAbsolutePos();
	auto size = el.GetSize();
	auto constrainedEndPos = pos + Vector2 {size};
	auto *parent = el.GetParent();
	while(parent) {
		auto endPos = parent->GetAbsolutePos() + Vector2 {parent->GetSize()};
		constrainedEndPos.x = umath::min(constrainedEndPos.x, endPos.x);
		constrainedEndPos.y = umath::min(constrainedEndPos.y, endPos.y);

		parent = parent->GetParent();
	}
	auto constrainedSize = constrainedEndPos - pos;
	constrainedSize.x = umath::max(constrainedSize.x, 0.f);
	constrainedSize.y = umath::max(constrainedSize.y, 0.f);

	auto fInitBorder = [](std::array<WIHandle, 4> &elements, const Vector2i &pos, const Vector2i &size) {
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
		Con::cout << t << GetElementInfo(*pParent) << Con::endl;
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
	auto &gui = WGUI::GetInstance();
	auto *window = gui.FindWindowUnderCursor();
	if(window)
		SetWindow(*window);
	auto *pText = static_cast<WIText *>(m_hText.get());
	auto *pEl = gui.GetCursorGUIElement(
	  nullptr, [this](WIBase *pEl) -> bool { return ShouldPass(*pEl); }, window);
	SetTargetGUIElement(pEl, true);

	if(m_hText.IsValid()) {
		int32_t x, y;
		gui.GetMousePos(x, y, window);
		m_hText->SetPos(x, y);
	}
}

void Console::commands::debug_gui_cursor(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	static std::unique_ptr<GUIDebugCursorManager> s_dbgManager = nullptr;
	if(s_dbgManager != nullptr) {
		s_dbgManager = nullptr;
		return;
	}
	s_dbgManager = std::make_unique<GUIDebugCursorManager>();
	if(s_dbgManager->Initialize() == false)
		s_dbgManager = nullptr;
}

void Console::commands::debug_dump_font_glyph_map(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	auto &wgui = WGUI::GetInstance();
	if(argv.empty()) {
		Con::cwar << "No font specified!" << Con::endl;
		return;
	}
	auto &fontName = argv.front();
	auto font = FontManager::GetFont(fontName);
	if(font == nullptr) {
		Con::cwar << "No font by name '" << fontName << "' found!" << Con::endl;
		return;
	}
	auto glyphMap = font->GetGlyphMap();
	if(glyphMap == nullptr) {
		Con::cwar << "Font '" << fontName << "' has invalid glyph map!" << Con::endl;
		return;
	}
	auto &glyphImg = glyphMap->GetImage();
	std::string fileName = "tmp/font_" + fontName + "_glyph_map";

	uimg::TextureInfo texInfo {};
	texInfo.containerFormat = uimg::TextureInfo::ContainerFormat::DDS;
	if(!prosper::util::save_texture(fileName, glyphImg, texInfo)) {
		Con::cwar << "Failed to save glyph map as '" << fileName << "'!" << Con::endl;
		return;
	}
}

void Console::commands::debug_font_glyph_map(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	auto &wgui = WGUI::GetInstance();
	auto *el = static_cast<WITexturedRect *>(wgui.GetBaseElement()->FindDescendantByName("dbg_glyph_map"));
	if(el) {
		el->RemoveSafely();
		return;
	}
	if(argv.empty()) {
		Con::cwar << "No font specified!" << Con::endl;
		return;
	}
	auto &fontName = argv.front();
	auto font = FontManager::GetFont(fontName);
	if(font == nullptr) {
		Con::cwar << "No font by name '" << fontName << "' found!" << Con::endl;
		return;
	}
	auto glyphMap = font->GetGlyphMap();
	if(glyphMap == nullptr) {
		Con::cwar << "Font '" << fontName << "' has invalid glyph map!" << Con::endl;
		return;
	}
	auto &glyphImg = glyphMap->GetImage();
	auto aspectRatio = glyphImg.GetWidth() / static_cast<float>(glyphImg.GetHeight());
	auto w = c_engine->GetRenderResolution().x;
	auto h = umath::round(w / aspectRatio);
	auto scale = 5.f;
	w *= scale;
	h *= scale;
	el = wgui.Create<WITexturedRect>();
	el->SetZPos(std::numeric_limits<int32_t>::max());
	el->SetName("dbg_glyph_map");
	el->SetTexture(*glyphMap);
	el->SetSize(w, h);
}
