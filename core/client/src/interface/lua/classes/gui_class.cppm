// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:scripting.lua.classes.gui;
export import :gui;
export import pragma.gui;

#undef DrawState

export namespace Lua {
	namespace gui {
		DLLCLIENT void clear_lua_callbacks(lua::State *l);
	};
	namespace WIBase {
		DLLCLIENT void register_class(luabind::class_<pragma::gui::types::WIBase> &classDef);
		DLLCLIENT bool IsValid(lua::State *l, pragma::gui::types::WIBase *hPanel);
		DLLCLIENT void TrapFocus(lua::State *l, pragma::gui::types::WIBase &hPanel, bool bTrap);
		DLLCLIENT void TrapFocus(lua::State *l, pragma::gui::types::WIBase &hPanel);
		DLLCLIENT void SetPos(lua::State *l, pragma::gui::types::WIBase &hPanel, ::Vector2 pos);
		DLLCLIENT void SetPos(lua::State *l, pragma::gui::types::WIBase &hPanel, float x, float y);
		DLLCLIENT void SetAbsolutePos(lua::State *l, pragma::gui::types::WIBase &hPanel, ::Vector2 pos);
		DLLCLIENT void SetAbsolutePos(lua::State *l, pragma::gui::types::WIBase &hPanel, float x, float y);
		DLLCLIENT void SetColor(lua::State *l, pragma::gui::types::WIBase &hPanel, ::Color col);
		DLLCLIENT void GetAlpha(lua::State *l, pragma::gui::types::WIBase &hPanel);
		DLLCLIENT void SetAlpha(lua::State *l, pragma::gui::types::WIBase &hPanel, float alpha);
		DLLCLIENT void SetSize(lua::State *l, pragma::gui::types::WIBase &hPanel, ::Vector2 size);
		DLLCLIENT void SetSize(lua::State *l, pragma::gui::types::WIBase &hPanel, float x, float y);
		DLLCLIENT void Wrap(lua::State *l, pragma::gui::types::WIBase &hPanel, const std::string &wrapperClassName);
		DLLCLIENT void ClearParent(lua::State *l, pragma::gui::types::WIBase &hPanel);
		DLLCLIENT void ResetParent(lua::State *l, pragma::gui::types::WIBase &hPanel);
		DLLCLIENT void GetChildren(lua::State *l, pragma::gui::types::WIBase &hPanel, std::string className);
		DLLCLIENT void GetChild(lua::State *l, pragma::gui::types::WIBase &hPanel, unsigned int idx);
		DLLCLIENT void GetChild(lua::State *l, pragma::gui::types::WIBase &hPanel, std::string className, unsigned int idx);
		DLLCLIENT void PosInBounds(lua::State *l, pragma::gui::types::WIBase &hPanel, ::Vector2 pos);
		DLLCLIENT void GetMousePos(lua::State *l, pragma::gui::types::WIBase &hPanel);
		DLLCLIENT void Draw(lua::State *l, pragma::gui::types::WIBase &hPanel, const pragma::gui::DrawInfo &drawInfo, pragma::gui::DrawState &drawState);
		DLLCLIENT void Draw(lua::State *l, pragma::gui::types::WIBase &hPanel, const pragma::gui::DrawInfo &drawInfo, pragma::gui::DrawState &drawState, const ::Vector2i &scissorOffset, const ::Vector2i &scissorSize);
		DLLCLIENT void Draw(lua::State *l, pragma::gui::types::WIBase &hPanel, const pragma::gui::DrawInfo &drawInfo, pragma::gui::DrawState &drawState, const ::Vector2i &scissorOffset, const ::Vector2i &scissorSize, const ::Vector2i &offsetParent);
		DLLCLIENT void Draw(lua::State *l, pragma::gui::types::WIBase &hPanel, const pragma::gui::DrawInfo &drawInfo, pragma::gui::DrawState &drawState, const ::Vector2i &scissorOffset, const ::Vector2i &scissorSize, const ::Vector2i &offsetParent, const ::Vector2 &scale);
		DLLCLIENT void GetX(lua::State *l, pragma::gui::types::WIBase &hPanel);
		DLLCLIENT void GetY(lua::State *l, pragma::gui::types::WIBase &hPanel);
		DLLCLIENT void SetX(lua::State *l, pragma::gui::types::WIBase &hPanel, float x); // TODO This should be int, but float-to-int conversion is currently broken with luabind
		DLLCLIENT void SetY(lua::State *l, pragma::gui::types::WIBase &hPanel, float y);
		DLLCLIENT void SetWidth(lua::State *l, pragma::gui::types::WIBase &hPanel, float w);
		DLLCLIENT void SetHeight(lua::State *l, pragma::gui::types::WIBase &hPanel, float h);
		DLLCLIENT void CallCallbacks(lua::State *l, pragma::gui::types::WIBase &hPanel, std::string name);
		DLLCLIENT void CallCallbacks(lua::State *l, pragma::gui::types::WIBase &hPanel, std::string name, luabind::object o1);
		DLLCLIENT void CallCallbacks(lua::State *l, pragma::gui::types::WIBase &hPanel, std::string name, luabind::object o1, luabind::object o2);
		DLLCLIENT void CallCallbacks(lua::State *l, pragma::gui::types::WIBase &hPanel, std::string name, luabind::object o1, luabind::object o2, luabind::object o3);
		DLLCLIENT void CallCallbacks(lua::State *l, pragma::gui::types::WIBase &hPanel, std::string name, luabind::object o1, luabind::object o2, luabind::object o3, luabind::object o4);
		DLLCLIENT void CallCallbacks(lua::State *l, pragma::gui::types::WIBase &hPanel, std::string name, luabind::object o1, luabind::object o2, luabind::object o3, luabind::object o4, luabind::object o5);
		DLLCLIENT void CallCallbacks(lua::State *l, pragma::gui::types::WIBase &hPanel, std::string name, luabind::object o1, luabind::object o2, luabind::object o3, luabind::object o4, luabind::object o5, luabind::object o6);
		DLLCLIENT void CallCallbacks(lua::State *l, pragma::gui::types::WIBase &hPanel, std::string name, luabind::object o1, luabind::object o2, luabind::object o3, luabind::object o4, luabind::object o5, luabind::object o6, luabind::object o7);
		DLLCLIENT void CallCallbacks(lua::State *l, pragma::gui::types::WIBase &hPanel, std::string name, luabind::object o1, luabind::object o2, luabind::object o3, luabind::object o4, luabind::object o5, luabind::object o6, luabind::object o7, luabind::object o8);
		DLLCLIENT CallbackHandle AddCallback(lua::State *l, pragma::gui::types::WIBase &hPanel, std::string name, func<void> o);
		DLLCLIENT void FadeIn(lua::State *l, pragma::gui::types::WIBase &hPanel, float tFadeIn);
		DLLCLIENT void FadeIn(lua::State *l, pragma::gui::types::WIBase &hPanel, float tFadeIn, float alphaTarget);
		DLLCLIENT void InjectMouseMoveInput(lua::State *l, pragma::gui::types::WIBase &hPanel, const ::Vector2 &mousePos);
		DLLCLIENT pragma::util::EventReply InjectMouseInput(lua::State *l, pragma::gui::types::WIBase &hPanel, const ::Vector2 &mousePos, int button, int action, int mods);
		DLLCLIENT pragma::util::EventReply InjectMouseInput(lua::State *l, pragma::gui::types::WIBase &hPanel, const ::Vector2 &mousePos, int button, int action);
		DLLCLIENT pragma::util::EventReply InjectMouseClick(lua::State *l, pragma::gui::types::WIBase &hPanel, const ::Vector2 &mousePos, int button, int mods);
		DLLCLIENT pragma::util::EventReply InjectMouseClick(lua::State *l, pragma::gui::types::WIBase &hPanel, const ::Vector2 &mousePos, int button);
		DLLCLIENT pragma::util::EventReply InjectKeyboardInput(lua::State *l, pragma::gui::types::WIBase &hPanel, int key, int action, int mods);
		DLLCLIENT pragma::util::EventReply InjectKeyboardInput(lua::State *l, pragma::gui::types::WIBase &hPanel, int key, int action);
		DLLCLIENT pragma::util::EventReply InjectKeyPress(lua::State *l, pragma::gui::types::WIBase &hPanel, int key, int mods);
		DLLCLIENT pragma::util::EventReply InjectKeyPress(lua::State *l, pragma::gui::types::WIBase &hPanel, int key);
		DLLCLIENT pragma::util::EventReply InjectCharInput(lua::State *l, pragma::gui::types::WIBase &hPanel, std::string c, uint32_t mods);
		DLLCLIENT pragma::util::EventReply InjectCharInput(lua::State *l, pragma::gui::types::WIBase &hPanel, std::string c);
		DLLCLIENT pragma::util::EventReply InjectScrollInput(lua::State *l, pragma::gui::types::WIBase &hPanel, const ::Vector2 &mousePos, const ::Vector2 &offset, bool offsetAsPixels);
		DLLCLIENT pragma::util::EventReply InjectScrollInput(lua::State *l, pragma::gui::types::WIBase &hPanel, const ::Vector2 &mousePos, const ::Vector2 &offset);
		DLLCLIENT void FindChildByName(lua::State *l, pragma::gui::types::WIBase &hPanel, std::string name);
		DLLCLIENT void FindChildrenByName(lua::State *l, pragma::gui::types::WIBase &hPanel, std::string name);
		DLLCLIENT void SetBounds(lua::State *l, pragma::gui::types::WIBase &hPanel, const ::Vector2 &start, const ::Vector2 &end);
	};

	namespace WIButton {
		DLLCLIENT void register_class(luabind::class_<pragma::gui::types::WIButton, pragma::gui::types::WIBase> &classDef);
	};

	namespace WIProgressBar {
		DLLCLIENT void register_class(luabind::class_<pragma::gui::types::WIProgressBar, pragma::gui::types::WIBase> &classDef);
	};

	namespace WISlider {
		DLLCLIENT void register_class(luabind::class_<pragma::gui::types::WISlider, luabind::bases<pragma::gui::types::WIProgressBar, pragma::gui::types::WIBase>> &classDef);
	};

	namespace WIShape {
		DLLCLIENT void register_class(luabind::class_<pragma::gui::types::WIShape, pragma::gui::types::WIBase> &classDef);
	};

	namespace WITexturedShape {
		DLLCLIENT void register_class(luabind::class_<pragma::gui::types::WITexturedShape, luabind::bases<pragma::gui::types::WIShape, pragma::gui::types::WIBase>> &classDef);
	};

	namespace WIText {
		DLLCLIENT void register_class(luabind::class_<pragma::gui::types::WIText, pragma::gui::types::WIBase> &classDef);
		DLLCLIENT void SetShadowColor(lua::State *l, pragma::gui::types::WIText &hPanel, ::Color &col);
		DLLCLIENT void SetShadowOffset(lua::State *l, pragma::gui::types::WIText &hPanel, ::Vector2 pos);
		DLLCLIENT void SetShadowXOffset(lua::State *l, pragma::gui::types::WIText &hPanel, float x);
		DLLCLIENT void SetShadowYOffset(lua::State *l, pragma::gui::types::WIText &hPanel, float y);
		DLLCLIENT ::Color GetShadowColor(lua::State *l, pragma::gui::types::WIText &hPanel);
		DLLCLIENT ::Vector2 GetShadowOffset(lua::State *l, pragma::gui::types::WIText &hPanel);
		DLLCLIENT float GetShadowXOffset(lua::State *l, pragma::gui::types::WIText &hPanel);
		DLLCLIENT float GetShadowYOffset(lua::State *l, pragma::gui::types::WIText &hPanel);
	};

	namespace WITextEntry {
		DLLCLIENT void register_class(luabind::class_<pragma::gui::types::WITextEntry, pragma::gui::types::WIBase> &classDef);
	};

	namespace WINumericEntry {
		DLLCLIENT void register_class(luabind::class_<pragma::gui::types::WINumericEntry, luabind::bases<pragma::gui::types::WITextEntry, pragma::gui::types::WIBase>> &classDef);
	};

	namespace WIOutlinedRect {
		DLLCLIENT void register_class(luabind::class_<pragma::gui::types::WIOutlinedRect, pragma::gui::types::WIBase> &classDef);
	};

	namespace WILine {
		DLLCLIENT void register_class(luabind::class_<pragma::gui::types::WILine, pragma::gui::types::WIBase> &classDef);
		DLLCLIENT void SetStartPos(lua::State *l, pragma::gui::types::WILine &hPanel, ::Vector2 &pos);
		DLLCLIENT void SetEndPos(lua::State *l, pragma::gui::types::WILine &hPanel, ::Vector2 &pos);
		DLLCLIENT ::Vector2 GetStartPos(lua::State *l, pragma::gui::types::WILine &hPanel);
		DLLCLIENT ::Vector2 GetEndPos(lua::State *l, pragma::gui::types::WILine &hPanel);
	};

	namespace WIRoundedRect {
		DLLCLIENT void register_class(luabind::class_<pragma::gui::types::WIRoundedRect, luabind::bases<pragma::gui::types::WIShape, pragma::gui::types::WIBase>> &classDef);
	};

	namespace WIRoundedTexturedRect {
		DLLCLIENT void register_class(luabind::class_<pragma::gui::types::WIRoundedTexturedRect, luabind::bases<pragma::gui::types::WITexturedShape, pragma::gui::types::WIShape, pragma::gui::types::WIBase>> &classDef);
	};

	namespace WIScrollBar {
		DLLCLIENT void register_class(luabind::class_<pragma::gui::types::WIScrollBar, pragma::gui::types::WIBase> &classDef);
	};

	namespace WIIcon {
		DLLCLIENT void register_class(luabind::class_<pragma::gui::types::WIIcon, luabind::bases<pragma::gui::types::WITexturedShape, pragma::gui::types::WIShape, pragma::gui::types::WIBase>> &classDef);
	};

	namespace WISilkIcon {
		DLLCLIENT void register_class(luabind::class_<pragma::gui::types::WISilkIcon, luabind::bases<pragma::gui::types::WIIcon, pragma::gui::types::WITexturedShape, pragma::gui::types::WIShape, pragma::gui::types::WIBase>> &classDef);
	};

	namespace WIDropDownMenu {
		DLLCLIENT void register_class(luabind::class_<pragma::gui::types::WIDropDownMenu, luabind::bases<pragma::gui::types::WITextEntry, pragma::gui::types::WIBase>> &classDef);
	};

	namespace WICheckbox {
		DLLCLIENT void register_class(luabind::class_<pragma::gui::types::WICheckbox, luabind::bases<pragma::gui::types::WIShape, pragma::gui::types::WIBase>> &classDef);
	};

	namespace WIArrow {
		DLLCLIENT void register_class(luabind::class_<pragma::gui::types::WIArrow, luabind::bases<pragma::gui::types::WIShape, pragma::gui::types::WIBase>> &classDef);
	};

	namespace WITransformable {
		DLLCLIENT void register_class(luabind::class_<pragma::gui::types::WITransformable, pragma::gui::types::WIBase> &classDef);
	};

	namespace WIGridPanel {
		DLLCLIENT void register_class(luabind::class_<pragma::gui::types::WIGridPanel, luabind::bases<pragma::gui::types::WITable, pragma::gui::types::WIBase>> &classDef);
	};

	namespace WITreeList {
		DLLCLIENT void register_class(luabind::class_<pragma::gui::types::WITreeList, luabind::bases<pragma::gui::types::WITable, pragma::gui::types::WIBase>> &classDef);
	};

	namespace WITreeListElement {
		DLLCLIENT void register_class(luabind::class_<pragma::gui::types::WITreeListElement, luabind::bases<pragma::gui::types::WITableRow, pragma::gui::types::WIBase>> &classDef);
		DLLCLIENT luabind::tableT<pragma::gui::types::WITreeListElement> GetItems(lua::State *l, pragma::gui::types::WITreeListElement &hElement);
	};

	namespace WIContainer {
		DLLCLIENT void register_class(luabind::class_<pragma::gui::types::WIContainer, pragma::gui::types::WIBase> &classDef);
	};

	namespace WITable {
		DLLCLIENT void register_class(luabind::class_<pragma::gui::types::WITable, luabind::bases<pragma::gui::types::WIContainer, pragma::gui::types::WIBase>> &classDef);
		DLLCLIENT luabind::tableT<pragma::gui::types::WITableRow> GetSelectedRows(lua::State *l, pragma::gui::types::WITable &hTable);
		DLLCLIENT luabind::tableT<pragma::gui::types::WITableRow> GetRows(lua::State *l, pragma::gui::types::WITable &hTable);
	};

	namespace WITableRow {
		DLLCLIENT void register_class(luabind::class_<pragma::gui::types::WITableRow, luabind::bases<pragma::gui::types::WIContainer, pragma::gui::types::WIBase>> &classDef);
	};

	namespace WITableCell {
		DLLCLIENT void register_class(luabind::class_<pragma::gui::types::WITableCell, luabind::bases<pragma::gui::types::WIContainer, pragma::gui::types::WIBase>> &classDef);
	};

	namespace WIFrame {
		DLLCLIENT void register_class(luabind::class_<pragma::gui::types::WIFrame, luabind::bases<pragma::gui::types::WITransformable, pragma::gui::types::WIBase>> &classDef);
	};
};

export {
	DLLCLIENT Con::c_cout &operator<<(Con::c_cout &os, const pragma::gui::WIHandle &handle);
	DLLCLIENT std::ostream &operator<<(std::ostream &os, const pragma::gui::WIHandle &handle);
};
