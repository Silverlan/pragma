// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "definitions.hpp"

#undef DrawState

export module pragma.client:scripting.lua.classes.gui;
export import :gui;
export import pragma.gui;

export namespace Lua {
	namespace gui {
		DLLCLIENT void clear_lua_callbacks(lua::State *l);
	};
	namespace WIBase {
		DLLCLIENT void register_class(luabind::class_<::WIBase> &classDef);
		DLLCLIENT bool IsValid(lua::State *l, ::WIBase *hPanel);
		DLLCLIENT void TrapFocus(lua::State *l, ::WIBase &hPanel, bool bTrap);
		DLLCLIENT void TrapFocus(lua::State *l, ::WIBase &hPanel);
		DLLCLIENT void SetPos(lua::State *l, ::WIBase &hPanel, ::Vector2 pos);
		DLLCLIENT void SetPos(lua::State *l, ::WIBase &hPanel, float x, float y);
		DLLCLIENT void SetAbsolutePos(lua::State *l, ::WIBase &hPanel, ::Vector2 pos);
		DLLCLIENT void SetAbsolutePos(lua::State *l, ::WIBase &hPanel, float x, float y);
		DLLCLIENT void SetColor(lua::State *l, ::WIBase &hPanel, ::Color col);
		DLLCLIENT void GetAlpha(lua::State *l, ::WIBase &hPanel);
		DLLCLIENT void SetAlpha(lua::State *l, ::WIBase &hPanel, float alpha);
		DLLCLIENT void SetSize(lua::State *l, ::WIBase &hPanel, ::Vector2 size);
		DLLCLIENT void SetSize(lua::State *l, ::WIBase &hPanel, float x, float y);
		DLLCLIENT void Wrap(lua::State *l, ::WIBase &hPanel, const std::string &wrapperClassName);
		DLLCLIENT void ClearParent(lua::State *l, ::WIBase &hPanel);
		DLLCLIENT void ResetParent(lua::State *l, ::WIBase &hPanel);
		DLLCLIENT void GetChildren(lua::State *l, ::WIBase &hPanel, std::string className);
		DLLCLIENT void GetChild(lua::State *l, ::WIBase &hPanel, unsigned int idx);
		DLLCLIENT void GetChild(lua::State *l, ::WIBase &hPanel, std::string className, unsigned int idx);
		DLLCLIENT void PosInBounds(lua::State *l, ::WIBase &hPanel, ::Vector2 pos);
		DLLCLIENT void GetMousePos(lua::State *l, ::WIBase &hPanel);
		DLLCLIENT void Draw(lua::State *l, ::WIBase &hPanel, const ::wgui::DrawInfo &drawInfo, wgui::DrawState &drawState);
		DLLCLIENT void Draw(lua::State *l, ::WIBase &hPanel, const ::wgui::DrawInfo &drawInfo, wgui::DrawState &drawState, const ::Vector2i &scissorOffset, const ::Vector2i &scissorSize);
		DLLCLIENT void Draw(lua::State *l, ::WIBase &hPanel, const ::wgui::DrawInfo &drawInfo, wgui::DrawState &drawState, const ::Vector2i &scissorOffset, const ::Vector2i &scissorSize, const ::Vector2i &offsetParent);
		DLLCLIENT void Draw(lua::State *l, ::WIBase &hPanel, const ::wgui::DrawInfo &drawInfo, wgui::DrawState &drawState, const ::Vector2i &scissorOffset, const ::Vector2i &scissorSize, const ::Vector2i &offsetParent, const ::Vector2 &scale);
		DLLCLIENT void GetX(lua::State *l, ::WIBase &hPanel);
		DLLCLIENT void GetY(lua::State *l, ::WIBase &hPanel);
		DLLCLIENT void SetX(lua::State *l, ::WIBase &hPanel, float x); // TODO This should be int, but float-to-int conversion is currently broken with luabind
		DLLCLIENT void SetY(lua::State *l, ::WIBase &hPanel, float y);
		DLLCLIENT void SetWidth(lua::State *l, ::WIBase &hPanel, float w);
		DLLCLIENT void SetHeight(lua::State *l, ::WIBase &hPanel, float h);
		DLLCLIENT void CallCallbacks(lua::State *l, ::WIBase &hPanel, std::string name);
		DLLCLIENT void CallCallbacks(lua::State *l, ::WIBase &hPanel, std::string name, luabind::object o1);
		DLLCLIENT void CallCallbacks(lua::State *l, ::WIBase &hPanel, std::string name, luabind::object o1, luabind::object o2);
		DLLCLIENT void CallCallbacks(lua::State *l, ::WIBase &hPanel, std::string name, luabind::object o1, luabind::object o2, luabind::object o3);
		DLLCLIENT void CallCallbacks(lua::State *l, ::WIBase &hPanel, std::string name, luabind::object o1, luabind::object o2, luabind::object o3, luabind::object o4);
		DLLCLIENT void CallCallbacks(lua::State *l, ::WIBase &hPanel, std::string name, luabind::object o1, luabind::object o2, luabind::object o3, luabind::object o4, luabind::object o5);
		DLLCLIENT void CallCallbacks(lua::State *l, ::WIBase &hPanel, std::string name, luabind::object o1, luabind::object o2, luabind::object o3, luabind::object o4, luabind::object o5, luabind::object o6);
		DLLCLIENT void CallCallbacks(lua::State *l, ::WIBase &hPanel, std::string name, luabind::object o1, luabind::object o2, luabind::object o3, luabind::object o4, luabind::object o5, luabind::object o6, luabind::object o7);
		DLLCLIENT void CallCallbacks(lua::State *l, ::WIBase &hPanel, std::string name, luabind::object o1, luabind::object o2, luabind::object o3, luabind::object o4, luabind::object o5, luabind::object o6, luabind::object o7, luabind::object o8);
		DLLCLIENT CallbackHandle AddCallback(lua::State *l, ::WIBase &hPanel, std::string name, Lua::func<void> o);
		DLLCLIENT void FadeIn(lua::State *l, ::WIBase &hPanel, float tFadeIn);
		DLLCLIENT void FadeIn(lua::State *l, ::WIBase &hPanel, float tFadeIn, float alphaTarget);
		DLLCLIENT void InjectMouseMoveInput(lua::State *l, ::WIBase &hPanel, const ::Vector2 &mousePos);
		DLLCLIENT ::util::EventReply InjectMouseInput(lua::State *l, ::WIBase &hPanel, const ::Vector2 &mousePos, int button, int action, int mods);
		DLLCLIENT ::util::EventReply InjectMouseInput(lua::State *l, ::WIBase &hPanel, const ::Vector2 &mousePos, int button, int action);
		DLLCLIENT ::util::EventReply InjectMouseClick(lua::State *l, ::WIBase &hPanel, const ::Vector2 &mousePos, int button, int mods);
		DLLCLIENT ::util::EventReply InjectMouseClick(lua::State *l, ::WIBase &hPanel, const ::Vector2 &mousePos, int button);
		DLLCLIENT ::util::EventReply InjectKeyboardInput(lua::State *l, ::WIBase &hPanel, int key, int action, int mods);
		DLLCLIENT ::util::EventReply InjectKeyboardInput(lua::State *l, ::WIBase &hPanel, int key, int action);
		DLLCLIENT ::util::EventReply InjectKeyPress(lua::State *l, ::WIBase &hPanel, int key, int mods);
		DLLCLIENT ::util::EventReply InjectKeyPress(lua::State *l, ::WIBase &hPanel, int key);
		DLLCLIENT ::util::EventReply InjectCharInput(lua::State *l, ::WIBase &hPanel, std::string c, uint32_t mods);
		DLLCLIENT ::util::EventReply InjectCharInput(lua::State *l, ::WIBase &hPanel, std::string c);
		DLLCLIENT ::util::EventReply InjectScrollInput(lua::State *l, ::WIBase &hPanel, const ::Vector2 &mousePos, const ::Vector2 &offset, bool offsetAsPixels);
		DLLCLIENT ::util::EventReply InjectScrollInput(lua::State *l, ::WIBase &hPanel, const ::Vector2 &mousePos, const ::Vector2 &offset);
		DLLCLIENT void FindChildByName(lua::State *l, ::WIBase &hPanel, std::string name);
		DLLCLIENT void FindChildrenByName(lua::State *l, ::WIBase &hPanel, std::string name);
		DLLCLIENT void SetBounds(lua::State *l, ::WIBase &hPanel, const ::Vector2 &start, const ::Vector2 &end);
	};

	namespace WIButton {
		DLLCLIENT void register_class(luabind::class_<::WIButton, ::WIBase> &classDef);
	};

	namespace WIProgressBar {
		DLLCLIENT void register_class(luabind::class_<pragma::gui::WIProgressBar, ::WIBase> &classDef);
	};

	namespace WISlider {
		DLLCLIENT void register_class(luabind::class_<pragma::gui::WISlider, luabind::bases<pragma::gui::WIProgressBar, ::WIBase>> &classDef);
	};

	namespace WIShape {
		DLLCLIENT void register_class(luabind::class_<::WIShape, ::WIBase> &classDef);
	};

	namespace WITexturedShape {
		DLLCLIENT void register_class(luabind::class_<::WITexturedShape, luabind::bases<::WIShape, ::WIBase>> &classDef);
	};

	namespace WIText {
		DLLCLIENT void register_class(luabind::class_<::WIText, ::WIBase> &classDef);
		DLLCLIENT void SetShadowColor(lua::State *l, ::WIText &hPanel, ::Color &col);
		DLLCLIENT void SetShadowOffset(lua::State *l, ::WIText &hPanel, ::Vector2 pos);
		DLLCLIENT void SetShadowXOffset(lua::State *l, ::WIText &hPanel, float x);
		DLLCLIENT void SetShadowYOffset(lua::State *l, ::WIText &hPanel, float y);
		DLLCLIENT ::Color GetShadowColor(lua::State *l, ::WIText &hPanel);
		DLLCLIENT ::Vector2 GetShadowOffset(lua::State *l, ::WIText &hPanel);
		DLLCLIENT float GetShadowXOffset(lua::State *l, ::WIText &hPanel);
		DLLCLIENT float GetShadowYOffset(lua::State *l, ::WIText &hPanel);
	};

	namespace WITextEntry {
		DLLCLIENT void register_class(luabind::class_<::WITextEntry, ::WIBase> &classDef);
	};

	namespace WINumericEntry {
		DLLCLIENT void register_class(luabind::class_<::WINumericEntry, luabind::bases<::WITextEntry, ::WIBase>> &classDef);
	};

	namespace WIOutlinedRect {
		DLLCLIENT void register_class(luabind::class_<::WIOutlinedRect, ::WIBase> &classDef);
	};

	namespace WILine {
		DLLCLIENT void register_class(luabind::class_<::WILine, ::WIBase> &classDef);
		DLLCLIENT void SetStartPos(lua::State *l, ::WILine &hPanel, ::Vector2 &pos);
		DLLCLIENT void SetEndPos(lua::State *l, ::WILine &hPanel, ::Vector2 &pos);
		DLLCLIENT ::Vector2 GetStartPos(lua::State *l, ::WILine &hPanel);
		DLLCLIENT ::Vector2 GetEndPos(lua::State *l, ::WILine &hPanel);
	};

	namespace WIRoundedRect {
		DLLCLIENT void register_class(luabind::class_<::WIRoundedRect, luabind::bases<::WIShape, ::WIBase>> &classDef);
	};

	namespace WIRoundedTexturedRect {
		DLLCLIENT void register_class(luabind::class_<::WIRoundedTexturedRect, luabind::bases<::WITexturedShape, ::WIShape, ::WIBase>> &classDef);
	};

	namespace WIScrollBar {
		DLLCLIENT void register_class(luabind::class_<::WIScrollBar, ::WIBase> &classDef);
	};

	namespace WIIcon {
		DLLCLIENT void register_class(luabind::class_<pragma::gui::WIIcon, luabind::bases<::WITexturedShape, ::WIShape, ::WIBase>> &classDef);
	};

	namespace WISilkIcon {
		DLLCLIENT void register_class(luabind::class_<pragma::gui::WISilkIcon, luabind::bases<pragma::gui::WIIcon, ::WITexturedShape, ::WIShape, ::WIBase>> &classDef);
	};

	namespace WIDropDownMenu {
		DLLCLIENT void register_class(luabind::class_<::WIDropDownMenu, luabind::bases<::WITextEntry, ::WIBase>> &classDef);
	};

	namespace WICheckbox {
		DLLCLIENT void register_class(luabind::class_<pragma::gui::WICheckbox, luabind::bases<::WIShape, ::WIBase>> &classDef);
	};

	namespace WIArrow {
		DLLCLIENT void register_class(luabind::class_<::WIArrow, luabind::bases<::WIShape, ::WIBase>> &classDef);
	};

	namespace WITransformable {
		DLLCLIENT void register_class(luabind::class_<pragma::gui::WITransformable, ::WIBase> &classDef);
	};

	namespace WIGridPanel {
		DLLCLIENT void register_class(luabind::class_<pragma::gui::WIGridPanel, luabind::bases<pragma::gui::WITable, ::WIBase>> &classDef);
	};

	namespace WITreeList {
		DLLCLIENT void register_class(luabind::class_<pragma::gui::WITreeList, luabind::bases<pragma::gui::WITable, ::WIBase>> &classDef);
	};

	namespace WITreeListElement {
		DLLCLIENT void register_class(luabind::class_<pragma::gui::WITreeListElement, luabind::bases<pragma::gui::WITableRow, ::WIBase>> &classDef);
		DLLCLIENT luabind::tableT<pragma::gui::WITreeListElement> GetItems(lua::State *l, pragma::gui::WITreeListElement &hElement);
	};

	namespace WIContainer {
		DLLCLIENT void register_class(luabind::class_<pragma::gui::WIContainer, ::WIBase> &classDef);
	};

	namespace WITable {
		DLLCLIENT void register_class(luabind::class_<pragma::gui::WITable, luabind::bases<pragma::gui::WIContainer, ::WIBase>> &classDef);
		DLLCLIENT luabind::tableT<pragma::gui::WITableRow> GetSelectedRows(lua::State *l, pragma::gui::WITable &hTable);
		DLLCLIENT luabind::tableT<pragma::gui::WITableRow> GetRows(lua::State *l, pragma::gui::WITable &hTable);
	};

	namespace WITableRow {
		DLLCLIENT void register_class(luabind::class_<pragma::gui::WITableRow, luabind::bases<pragma::gui::WIContainer, ::WIBase>> &classDef);
	};

	namespace WITableCell {
		DLLCLIENT void register_class(luabind::class_<pragma::gui::WITableCell, luabind::bases<pragma::gui::WIContainer, ::WIBase>> &classDef);
	};

	namespace WIFrame {
		DLLCLIENT void register_class(luabind::class_<pragma::gui::WIFrame, luabind::bases<pragma::gui::WITransformable, ::WIBase>> &classDef);
	};
};

export {
	DLLCLIENT Con::c_cout &operator<<(Con::c_cout &os, const WIHandle &handle);
	DLLCLIENT std::ostream &operator<<(std::ostream &os, const WIHandle &handle);
};
