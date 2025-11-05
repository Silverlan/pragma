// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "pragma/lua/core.hpp"

#undef DrawState

export module pragma.client:scripting.lua.classes.gui;
export import :gui;
export import pragma.gui;

export namespace Lua {
	namespace gui {
		DLLCLIENT void clear_lua_callbacks(lua_State *l);
	};
	namespace WIBase {
		DLLCLIENT void register_class(luabind::class_<::WIBase> &classDef);
		DLLCLIENT bool IsValid(lua_State *l, ::WIBase *hPanel);
		DLLCLIENT void TrapFocus(lua_State *l, ::WIBase &hPanel, bool bTrap);
		DLLCLIENT void TrapFocus(lua_State *l, ::WIBase &hPanel);
		DLLCLIENT void SetPos(lua_State *l, ::WIBase &hPanel, ::Vector2 pos);
		DLLCLIENT void SetPos(lua_State *l, ::WIBase &hPanel, float x, float y);
		DLLCLIENT void SetAbsolutePos(lua_State *l, ::WIBase &hPanel, ::Vector2 pos);
		DLLCLIENT void SetAbsolutePos(lua_State *l, ::WIBase &hPanel, float x, float y);
		DLLCLIENT void SetColor(lua_State *l, ::WIBase &hPanel, ::Color col);
		DLLCLIENT void GetAlpha(lua_State *l, ::WIBase &hPanel);
		DLLCLIENT void SetAlpha(lua_State *l, ::WIBase &hPanel, float alpha);
		DLLCLIENT void SetSize(lua_State *l, ::WIBase &hPanel, ::Vector2 size);
		DLLCLIENT void SetSize(lua_State *l, ::WIBase &hPanel, float x, float y);
		DLLCLIENT void Wrap(lua_State *l, ::WIBase &hPanel, const std::string &wrapperClassName);
		DLLCLIENT void ClearParent(lua_State *l, ::WIBase &hPanel);
		DLLCLIENT void ResetParent(lua_State *l, ::WIBase &hPanel);
		DLLCLIENT void GetChildren(lua_State *l, ::WIBase &hPanel, std::string className);
		DLLCLIENT void GetChild(lua_State *l, ::WIBase &hPanel, unsigned int idx);
		DLLCLIENT void GetChild(lua_State *l, ::WIBase &hPanel, std::string className, unsigned int idx);
		DLLCLIENT void PosInBounds(lua_State *l, ::WIBase &hPanel, ::Vector2 pos);
		DLLCLIENT void GetMousePos(lua_State *l, ::WIBase &hPanel);
		DLLCLIENT void Draw(lua_State *l, ::WIBase &hPanel, const ::wgui::DrawInfo &drawInfo, wgui::DrawState &drawState);
		DLLCLIENT void Draw(lua_State *l, ::WIBase &hPanel, const ::wgui::DrawInfo &drawInfo, wgui::DrawState &drawState, const ::Vector2i &scissorOffset, const ::Vector2i &scissorSize);
		DLLCLIENT void Draw(lua_State *l, ::WIBase &hPanel, const ::wgui::DrawInfo &drawInfo, wgui::DrawState &drawState, const ::Vector2i &scissorOffset, const ::Vector2i &scissorSize, const ::Vector2i &offsetParent);
		DLLCLIENT void Draw(lua_State *l, ::WIBase &hPanel, const ::wgui::DrawInfo &drawInfo, wgui::DrawState &drawState, const ::Vector2i &scissorOffset, const ::Vector2i &scissorSize, const ::Vector2i &offsetParent, const ::Vector2 &scale);
		DLLCLIENT void GetX(lua_State *l, ::WIBase &hPanel);
		DLLCLIENT void GetY(lua_State *l, ::WIBase &hPanel);
		DLLCLIENT void SetX(lua_State *l, ::WIBase &hPanel, float x); // TODO This should be int, but float-to-int conversion is currently broken with luabind
		DLLCLIENT void SetY(lua_State *l, ::WIBase &hPanel, float y);
		DLLCLIENT void SetWidth(lua_State *l, ::WIBase &hPanel, float w);
		DLLCLIENT void SetHeight(lua_State *l, ::WIBase &hPanel, float h);
		DLLCLIENT void CallCallbacks(lua_State *l, ::WIBase &hPanel, std::string name);
		DLLCLIENT void CallCallbacks(lua_State *l, ::WIBase &hPanel, std::string name, luabind::object o1);
		DLLCLIENT void CallCallbacks(lua_State *l, ::WIBase &hPanel, std::string name, luabind::object o1, luabind::object o2);
		DLLCLIENT void CallCallbacks(lua_State *l, ::WIBase &hPanel, std::string name, luabind::object o1, luabind::object o2, luabind::object o3);
		DLLCLIENT void CallCallbacks(lua_State *l, ::WIBase &hPanel, std::string name, luabind::object o1, luabind::object o2, luabind::object o3, luabind::object o4);
		DLLCLIENT void CallCallbacks(lua_State *l, ::WIBase &hPanel, std::string name, luabind::object o1, luabind::object o2, luabind::object o3, luabind::object o4, luabind::object o5);
		DLLCLIENT void CallCallbacks(lua_State *l, ::WIBase &hPanel, std::string name, luabind::object o1, luabind::object o2, luabind::object o3, luabind::object o4, luabind::object o5, luabind::object o6);
		DLLCLIENT void CallCallbacks(lua_State *l, ::WIBase &hPanel, std::string name, luabind::object o1, luabind::object o2, luabind::object o3, luabind::object o4, luabind::object o5, luabind::object o6, luabind::object o7);
		DLLCLIENT void CallCallbacks(lua_State *l, ::WIBase &hPanel, std::string name, luabind::object o1, luabind::object o2, luabind::object o3, luabind::object o4, luabind::object o5, luabind::object o6, luabind::object o7, luabind::object o8);
		DLLCLIENT CallbackHandle AddCallback(lua_State *l, ::WIBase &hPanel, std::string name, Lua::func<void> o);
		DLLCLIENT void FadeIn(lua_State *l, ::WIBase &hPanel, float tFadeIn);
		DLLCLIENT void FadeIn(lua_State *l, ::WIBase &hPanel, float tFadeIn, float alphaTarget);
		DLLCLIENT void InjectMouseMoveInput(lua_State *l, ::WIBase &hPanel, const ::Vector2 &mousePos);
		DLLCLIENT ::util::EventReply InjectMouseInput(lua_State *l, ::WIBase &hPanel, const ::Vector2 &mousePos, int button, int action, int mods);
		DLLCLIENT ::util::EventReply InjectMouseInput(lua_State *l, ::WIBase &hPanel, const ::Vector2 &mousePos, int button, int action);
		DLLCLIENT ::util::EventReply InjectMouseClick(lua_State *l, ::WIBase &hPanel, const ::Vector2 &mousePos, int button, int mods);
		DLLCLIENT ::util::EventReply InjectMouseClick(lua_State *l, ::WIBase &hPanel, const ::Vector2 &mousePos, int button);
		DLLCLIENT ::util::EventReply InjectKeyboardInput(lua_State *l, ::WIBase &hPanel, int key, int action, int mods);
		DLLCLIENT ::util::EventReply InjectKeyboardInput(lua_State *l, ::WIBase &hPanel, int key, int action);
		DLLCLIENT ::util::EventReply InjectKeyPress(lua_State *l, ::WIBase &hPanel, int key, int mods);
		DLLCLIENT ::util::EventReply InjectKeyPress(lua_State *l, ::WIBase &hPanel, int key);
		DLLCLIENT ::util::EventReply InjectCharInput(lua_State *l, ::WIBase &hPanel, std::string c, uint32_t mods);
		DLLCLIENT ::util::EventReply InjectCharInput(lua_State *l, ::WIBase &hPanel, std::string c);
		DLLCLIENT ::util::EventReply InjectScrollInput(lua_State *l, ::WIBase &hPanel, const ::Vector2 &mousePos, const ::Vector2 &offset, bool offsetAsPixels);
		DLLCLIENT ::util::EventReply InjectScrollInput(lua_State *l, ::WIBase &hPanel, const ::Vector2 &mousePos, const ::Vector2 &offset);
		DLLCLIENT void FindChildByName(lua_State *l, ::WIBase &hPanel, std::string name);
		DLLCLIENT void FindChildrenByName(lua_State *l, ::WIBase &hPanel, std::string name);
		DLLCLIENT void SetBounds(lua_State *l, ::WIBase &hPanel, const ::Vector2 &start, const ::Vector2 &end);
	};

	namespace WIButton {
		DLLCLIENT void register_class(luabind::class_<::WIButton, ::WIBase> &classDef);
	};

	namespace WIProgressBar {
		DLLCLIENT void register_class(luabind::class_<::WIProgressBar, ::WIBase> &classDef);
	};

	namespace WISlider {
		DLLCLIENT void register_class(luabind::class_<::WISlider, luabind::bases<::WIProgressBar, ::WIBase>> &classDef);
	};

	namespace WIShape {
		DLLCLIENT void register_class(luabind::class_<::WIShape, ::WIBase> &classDef);
	};

	namespace WITexturedShape {
		DLLCLIENT void register_class(luabind::class_<::WITexturedShape, luabind::bases<::WIShape, ::WIBase>> &classDef);
	};

	namespace WIText {
		DLLCLIENT void register_class(luabind::class_<::WIText, ::WIBase> &classDef);
		DLLCLIENT void SetShadowColor(lua_State *l, ::WIText &hPanel, ::Color &col);
		DLLCLIENT void SetShadowOffset(lua_State *l, ::WIText &hPanel, ::Vector2 pos);
		DLLCLIENT void SetShadowXOffset(lua_State *l, ::WIText &hPanel, float x);
		DLLCLIENT void SetShadowYOffset(lua_State *l, ::WIText &hPanel, float y);
		DLLCLIENT ::Color GetShadowColor(lua_State *l, ::WIText &hPanel);
		DLLCLIENT ::Vector2 GetShadowOffset(lua_State *l, ::WIText &hPanel);
		DLLCLIENT float GetShadowXOffset(lua_State *l, ::WIText &hPanel);
		DLLCLIENT float GetShadowYOffset(lua_State *l, ::WIText &hPanel);
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
		DLLCLIENT void SetStartPos(lua_State *l, ::WILine &hPanel, ::Vector2 &pos);
		DLLCLIENT void SetEndPos(lua_State *l, ::WILine &hPanel, ::Vector2 &pos);
		DLLCLIENT ::Vector2 GetStartPos(lua_State *l, ::WILine &hPanel);
		DLLCLIENT ::Vector2 GetEndPos(lua_State *l, ::WILine &hPanel);
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
		DLLCLIENT void register_class(luabind::class_<::WIIcon, luabind::bases<::WITexturedShape, ::WIShape, ::WIBase>> &classDef);
	};

	namespace WISilkIcon {
		DLLCLIENT void register_class(luabind::class_<::WISilkIcon, luabind::bases<::WIIcon, ::WITexturedShape, ::WIShape, ::WIBase>> &classDef);
	};

	namespace WIDropDownMenu {
		DLLCLIENT void register_class(luabind::class_<::WIDropDownMenu, luabind::bases<::WITextEntry, ::WIBase>> &classDef);
	};

	namespace WICheckbox {
		DLLCLIENT void register_class(luabind::class_<::WICheckbox, luabind::bases<::WIShape, ::WIBase>> &classDef);
	};

	namespace WIArrow {
		DLLCLIENT void register_class(luabind::class_<::WIArrow, luabind::bases<::WIShape, ::WIBase>> &classDef);
	};

	namespace WITransformable {
		DLLCLIENT void register_class(luabind::class_<::WITransformable, ::WIBase> &classDef);
	};

	namespace WIGridPanel {
		DLLCLIENT void register_class(luabind::class_<::WIGridPanel, luabind::bases<::WITable, ::WIBase>> &classDef);
	};

	namespace WITreeList {
		DLLCLIENT void register_class(luabind::class_<::WITreeList, luabind::bases<::WITable, ::WIBase>> &classDef);
	};

	namespace WITreeListElement {
		DLLCLIENT void register_class(luabind::class_<::WITreeListElement, luabind::bases<::WITableRow, ::WIBase>> &classDef);
		DLLCLIENT luabind::tableT<::WITreeListElement> GetItems(lua_State *l, ::WITreeListElement &hElement);
	};

	namespace WIContainer {
		DLLCLIENT void register_class(luabind::class_<::WIContainer, ::WIBase> &classDef);
	};

	namespace WITable {
		DLLCLIENT void register_class(luabind::class_<::WITable, luabind::bases<::WIContainer, ::WIBase>> &classDef);
		DLLCLIENT luabind::tableT<::WITableRow> GetSelectedRows(lua_State *l, ::WITable &hTable);
		DLLCLIENT luabind::tableT<::WITableRow> GetRows(lua_State *l, ::WITable &hTable);
	};

	namespace WITableRow {
		DLLCLIENT void register_class(luabind::class_<::WITableRow, luabind::bases<::WIContainer, ::WIBase>> &classDef);
	};

	namespace WITableCell {
		DLLCLIENT void register_class(luabind::class_<::WITableCell, luabind::bases<::WIContainer, ::WIBase>> &classDef);
	};

	namespace WIFrame {
		DLLCLIENT void register_class(luabind::class_<::WIFrame, luabind::bases<::WITransformable, ::WIBase>> &classDef);
	};
};

export {
	DLLCLIENT Con::c_cout &operator<<(Con::c_cout &os, const WIHandle &handle);
	DLLCLIENT std::ostream &operator<<(std::ostream &os, const WIHandle &handle);
};
