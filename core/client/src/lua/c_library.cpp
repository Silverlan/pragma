#include "stdafx_client.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/game/c_game.h"
#include "pragma/lua/libraries/c_ldebugoverlay.h"
#include "pragma/lua/libraries/c_lgui.h"
#include "pragma/lua/libraries/c_lphysx.h"
#include "pragma/lua/libraries/c_lsound.h"
#include "pragma/lua/classes/c_lshader.h"
#include "pragma/lua/libraries/c_lutil.h"
#include "pragma/lua/libraries/c_linput.h"
#include "pragma/audio/c_laleffect.h"
#include "pragma/audio/c_lalsound.hpp"
#include "pragma/gui/wiluabase.h"
#include "pragma/gui/wiconsole.hpp"
#include "pragma/lua/classes/c_lwibase.h"
#include "pragma/lua/classes/c_ldef_wgui.h"
#include "pragma/ai/c_lai.hpp"
#include <pragma/lua/lua_entity_component.hpp>
#include <alsoundsystem.hpp>
#include <luainterface.hpp>

static void register_gui(Lua::Interface &lua)
{
	auto *l = lua.GetState();
	lua.RegisterLibrary("gui",{
		{"create",Lua::gui::create},
		{"create_label",Lua::gui::create_label},
		{"create_button",Lua::gui::create_button},
		{"create_checkbox",Lua::gui::create_checkbox},
		{"register",Lua::gui::register_element},
		{"get_base_element",Lua::gui::get_base_element},
		{"get_focused_element",Lua::gui::get_focused_element},
		{"register_skin",Lua::gui::register_skin},
		{"set_skin",Lua::gui::set_skin},
		{"skin_exists",Lua::gui::skin_exists},
		{"get_cursor",Lua::gui::get_cursor},
		{"set_cursor",Lua::gui::set_cursor},
		{"get_cursor_input_mode",Lua::gui::get_cursor_input_mode},
		{"set_cursor_input_mode",Lua::gui::set_cursor_input_mode},
		{"get_window_size",Lua::gui::get_window_size},
		{"inject_mouse_input",Lua::gui::inject_mouse_input},
		{"inject_keyboard_input",Lua::gui::inject_keyboard_input},
		{"inject_char_input",Lua::gui::inject_char_input},
		{"inject_scroll_input",Lua::gui::inject_scroll_input},
		{"find_element_by_name",static_cast<int32_t(*)(lua_State*)>([](lua_State *l) -> int32_t {
			std::string name = Lua::CheckString(l,1);
			auto *p = WGUI::GetInstance().GetBaseElement();
			if(p == nullptr)
				return 0;
			auto *r = p->FindDescendantByName(name);
			if(r == nullptr)
				return 0;
			auto oChild = WGUILuaInterface::GetLuaObject(l,*r);
			oChild.push(l);
			return 1;
		})},
		{"find_elements_by_name",static_cast<int32_t(*)(lua_State*)>([](lua_State *l) -> int32_t {
			std::vector<WIHandle> results {};
			auto t = Lua::CreateTable(l);
			std::string name = Lua::CheckString(l,1);
			auto *p = WGUI::GetInstance().GetBaseElement();
			if(p != nullptr)
				p->FindDescendantsByName(name,results);
			auto idx = 1;
			for(auto &hEl : results)
			{
				if(hEl.IsValid() == false)
					continue;
				Lua::PushInt(l,idx++);
				auto oChild = WGUILuaInterface::GetLuaObject(l,*hEl.get());
				oChild.push(l);
				Lua::SetTableValue(l,t);
			}
			return 1;
		})},
		{"get_console",static_cast<int32_t(*)(lua_State*)>([](lua_State *l) -> int32_t {
			auto *pConsole = WIConsole::GetConsole();
			if(pConsole == nullptr)
				return 0;
			auto oChild = WGUILuaInterface::GetLuaObject(l,*pConsole);
			oChild.push(l);
			return 1;
		})},
		{"open_console",static_cast<int32_t(*)(lua_State*)>([](lua_State *l) -> int32_t {
			auto *pConsole = WIConsole::Open();
			if(pConsole == nullptr)
				return 0;
			auto oChild = WGUILuaInterface::GetLuaObject(l,*pConsole);
			oChild.push(l);
			return 1;
		})},
		{"close_console",static_cast<int32_t(*)(lua_State*)>([](lua_State *l) -> int32_t {
			WIConsole::Close();
			return 0;
		})},
		{"is_console_open",static_cast<int32_t(*)(lua_State*)>([](lua_State *l) -> int32_t {
			auto *pConsole = WIConsole::GetConsole();
			auto *pFrame = pConsole ? pConsole->GetFrame() : nullptr;
			Lua::PushBool(l,pFrame ? pFrame->IsVisible() : false);
			return 1;
		})}
	});
	auto guiMod = luabind::module(l,"gui");

	//
	auto wiElementClassDef = luabind::class_<WIHandle>("Element");
	Lua::WIBase::register_class(wiElementClassDef);
	guiMod[wiElementClassDef];

	// Custom Classes
	auto wiBaseWIElement = luabind::class_<WILuaHandle,luabind::bases<WIHandle>,luabind::default_holder,WILuaWrapper>("Base");
	wiBaseWIElement.def(luabind::constructor<>());
	wiBaseWIElement.def("OnInitialize",&WILuaWrapper::OnInitialize,&WILuaWrapper::default_OnInitialize);
	wiBaseWIElement.def("OnThink",&WILuaWrapper::OnThink,&WILuaWrapper::default_OnThink);
	wiBaseWIElement.def("MouseCallback",&WILuaWrapper::MouseCallback,&WILuaWrapper::default_MouseCallback);
	wiBaseWIElement.def("KeyboardCallback",&WILuaWrapper::KeyboardCallback,&WILuaWrapper::default_KeyboardCallback);
	wiBaseWIElement.def("CharCallback",&WILuaWrapper::CharCallback,&WILuaWrapper::default_CharCallback);
	wiBaseWIElement.def("ScrollCallback",&WILuaWrapper::ScrollCallback,&WILuaWrapper::default_ScrollCallback);
	wiBaseWIElement.def("OnVisibilityChanged",&WILuaWrapper::OnSetVisible,&WILuaWrapper::default_OnSetVisible);
	wiBaseWIElement.def("OnSizeChanged",&WILuaWrapper::OnSetSize,&WILuaWrapper::default_OnSetSize);
	wiBaseWIElement.def("OnColorChanged",&WILuaWrapper::OnSetColor,&WILuaWrapper::default_OnSetColor);
	wiBaseWIElement.def("OnAlphaChanged",&WILuaWrapper::OnSetAlpha,&WILuaWrapper::default_OnSetAlpha);
	wiBaseWIElement.def("OnDraw",&WILuaWrapper::Render,&WILuaWrapper::default_Render);
	wiBaseWIElement.def("OnCursorEntered",&WILuaWrapper::OnCursorEntered,&WILuaWrapper::default_OnCursorEntered);
	wiBaseWIElement.def("OnCursorExited",&WILuaWrapper::OnCursorExited,&WILuaWrapper::default_OnCursorExited);
	wiBaseWIElement.def("OnFocusGained",&WILuaWrapper::OnFocusGained,&WILuaWrapper::default_OnFocusGained);
	wiBaseWIElement.def("OnFocusKilled",&WILuaWrapper::OnFocusKilled,&WILuaWrapper::default_OnFocusKilled);
	wiBaseWIElement.def("OnRemove",&WILuaWrapper::OnRemove,&WILuaWrapper::default_OnRemove);
	guiMod[wiBaseWIElement];
	//

	// Class specific handles have to also be defined in CGame::InitializeGUIElement and WGUIHandleFactory!
	auto wiShapeClassDef = luabind::class_<WIShapeHandle,WIHandle>("Shape");
	Lua::WIShape::register_class(wiShapeClassDef);
	guiMod[wiShapeClassDef];

	auto wiButtonClassDef = luabind::class_<WIButtonHandle,WIHandle>("Button");
	Lua::WIButton::register_class(wiButtonClassDef);
	guiMod[wiButtonClassDef];

	auto wiTexturedShapeClassDef = luabind::class_<WITexturedShapeHandle,luabind::bases<WIShapeHandle,WIHandle>>("TexturedShape");
	Lua::WITexturedShape::register_class(wiTexturedShapeClassDef);
	guiMod[wiTexturedShapeClassDef];

	auto wiIconClassDef = luabind::class_<WIIconHandle,luabind::bases<WITexturedShapeHandle,WIShapeHandle,WIHandle>>("Icon");
	Lua::WIIcon::register_class(wiIconClassDef);
	guiMod[wiIconClassDef];

	auto wiSilkIconClassDef = luabind::class_<WISilkIconHandle,luabind::bases<WIIconHandle,WITexturedShapeHandle,WIShapeHandle,WIHandle>>("SilkIcon");
	Lua::WISilkIcon::register_class(wiSilkIconClassDef);
	guiMod[wiSilkIconClassDef];

	auto wiArrowClassDef = luabind::class_<WIArrowHandle,luabind::bases<WIShapeHandle,WIHandle>>("Arrow");
	Lua::WIArrow::register_class(wiArrowClassDef);
	guiMod[wiArrowClassDef];

	auto wiCheckboxClassDef = luabind::class_<WICheckboxHandle,luabind::bases<WIShapeHandle,WIHandle>>("Checkbox");
	Lua::WICheckbox::register_class(wiCheckboxClassDef);
	guiMod[wiCheckboxClassDef];

	auto wiTransformableClassDef = luabind::class_<WITransformableHandle,WIHandle>("Transformable");
	Lua::WITransformable::register_class(wiTransformableClassDef);
	guiMod[wiTransformableClassDef];

	auto wiSnapAreaClassDef = luabind::class_<WISnapAreaHandle,WIHandle>("SnapArea");
	wiSnapAreaClassDef.def("GetTriggerArea",static_cast<void(*)(lua_State*,WISnapAreaHandle&)>([](lua_State *l,WISnapAreaHandle &hEl) {
		lua_checkgui(l,hEl);
		auto *pTriggerArea = static_cast<::WISnapArea*>(hEl.get())->GetTriggerArea();
		if(pTriggerArea == nullptr)
			return;
		auto o = WGUILuaInterface::GetLuaObject(l,*pTriggerArea);
		o.push(l);
	}));
	guiMod[wiSnapAreaClassDef];

	auto wiDebugDepthTextureClassDef = luabind::class_<WIDebugDepthTextureHandle,WIHandle>("DebugDepthTexture");
	wiDebugDepthTextureClassDef.def("SetContrastFactor",static_cast<void(*)(lua_State*,WIDebugDepthTextureHandle&,float)>([](lua_State *l,WIDebugDepthTextureHandle &hEl,float contrastFactor) {
		lua_checkgui(l,hEl);
		static_cast<::WIDebugDepthTexture*>(hEl.get())->SetContrastFactor(contrastFactor);
	}));
	wiDebugDepthTextureClassDef.def("GetContrastFactor",static_cast<void(*)(lua_State*,WIDebugDepthTextureHandle&)>([](lua_State *l,WIDebugDepthTextureHandle &hEl) {
		lua_checkgui(l,hEl);
		Lua::PushNumber(l,static_cast<::WIDebugDepthTexture*>(hEl.get())->GetContrastFactor());
	}));
	wiDebugDepthTextureClassDef.def("SetTexture",static_cast<void(*)(lua_State*,WIDebugDepthTextureHandle&,std::shared_ptr<prosper::Texture>&)>([](lua_State *l,WIDebugDepthTextureHandle &hEl,std::shared_ptr<prosper::Texture> &tex) {
		lua_checkgui(l,hEl);
		static_cast<::WIDebugDepthTexture*>(hEl.get())->SetTexture(*tex);
	}));
	guiMod[wiDebugDepthTextureClassDef];

	auto wiDebugShadowMapClassDef = luabind::class_<WIDebugShadowMapHandle,WIHandle>("DebugShadowMap");
	wiDebugShadowMapClassDef.def("SetContrastFactor",static_cast<void(*)(lua_State*,WIDebugShadowMapHandle&,float)>([](lua_State *l,WIDebugShadowMapHandle &hEl,float contrastFactor) {
		lua_checkgui(l,hEl);
		static_cast<::WIDebugShadowMap*>(hEl.get())->SetContrastFactor(contrastFactor);
	}));
	wiDebugShadowMapClassDef.def("GetContrastFactor",static_cast<void(*)(lua_State*,WIDebugShadowMapHandle&)>([](lua_State *l,WIDebugShadowMapHandle &hEl) {
		lua_checkgui(l,hEl);
		Lua::PushNumber(l,static_cast<::WIDebugShadowMap*>(hEl.get())->GetContrastFactor());
	}));
	wiDebugShadowMapClassDef.def("SetShadowMapSize",static_cast<void(*)(lua_State*,WIDebugShadowMapHandle&,uint32_t,uint32_t)>([](lua_State *l,WIDebugShadowMapHandle &hEl,uint32_t w,uint32_t h) {
		lua_checkgui(l,hEl);
		static_cast<::WIDebugShadowMap*>(hEl.get())->SetShadowMapSize(w,h);
	}));
	wiDebugShadowMapClassDef.def("SetLightSource",static_cast<void(*)(lua_State*,WIDebugShadowMapHandle&,CLightHandle&)>([](lua_State *l,WIDebugShadowMapHandle &hEl,CLightHandle &hLight) {
		lua_checkgui(l,hEl);
		pragma::Lua::check_component(l,hLight);
		static_cast<::WIDebugShadowMap*>(hEl.get())->SetLightSource(*hLight);
	}));
	guiMod[wiDebugShadowMapClassDef];

	auto wiDebugSsaoClassDef = luabind::class_<WIDebugSSAOHandle,luabind::bases<WITexturedShapeHandle,WIShapeHandle,WIHandle>>("DebugSSAO");
	wiDebugSsaoClassDef.def("SetUseBlurredSSAOImage",static_cast<void(*)(lua_State*,WIDebugSSAOHandle&,bool)>([](lua_State *l,WIDebugSSAOHandle &hEl,bool useBlurredImage) {
		lua_checkgui(l,hEl);
		static_cast<::WIDebugSSAO*>(hEl.get())->SetUseBlurredSSAOImage(useBlurredImage);
	}));
	guiMod[wiDebugSsaoClassDef];

	auto wiProgressBarClassDef = luabind::class_<WIProgressBarHandle,WIHandle>("ProgressBar");
	Lua::WIProgressBar::register_class(wiProgressBarClassDef);
	guiMod[wiProgressBarClassDef];

	auto wiSliderClassDef = luabind::class_<WISliderHandle,luabind::bases<WIProgressBarHandle,WIHandle>>("Slider");
	Lua::WISlider::register_class(wiSliderClassDef);
	guiMod[wiSliderClassDef];

	auto wiScrollContainerClassDef = luabind::class_<WIScrollContainerHandle,WIHandle>("ScrollContainer");
	wiScrollContainerClassDef.def("SetAutoStickToBottom",static_cast<void(*)(lua_State*,WIScrollContainerHandle&,bool)>([](lua_State *l,WIScrollContainerHandle &hEl,bool stick) {
		lua_checkgui(l,hEl);
		static_cast<::WIScrollContainer*>(hEl.get())->SetAutoStickToBottom(stick);
	}));
	wiScrollContainerClassDef.def("ShouldAutoStickToBottom",static_cast<void(*)(lua_State*,WIScrollContainerHandle&)>([](lua_State *l,WIScrollContainerHandle &hEl) {
		lua_checkgui(l,hEl);
		Lua::PushBool(l,static_cast<::WIScrollContainer*>(hEl.get())->ShouldAutoStickToBottom());
	}));
	guiMod[wiScrollContainerClassDef];

	auto wiContainerClassDef = luabind::class_<WIContainerHandle,WIHandle>("Container");
	Lua::WIContainer::register_class(wiContainerClassDef);
	guiMod[wiContainerClassDef];

	auto wiTableClassDef = luabind::class_<WITableHandle,luabind::bases<WIContainerHandle,WIHandle>>("Table");
	Lua::WITable::register_class(wiTableClassDef);

	auto wiTableRowClassDef = luabind::class_<WITableRowHandle,luabind::bases<WIContainerHandle,WIHandle>>("Row");
	Lua::WITableRow::register_class(wiTableRowClassDef);

	auto wiTableCellClassDef = luabind::class_<WITableCellHandle,luabind::bases<WIContainerHandle,WIHandle>>("Cell");
	Lua::WITableCell::register_class(wiTableCellClassDef);
	wiTableClassDef.scope[wiTableCellClassDef];
	wiTableClassDef.scope[wiTableRowClassDef];
	guiMod[wiTableClassDef];

	auto wiGridPanelClassDef = luabind::class_<WIGridPanelHandle,luabind::bases<WITableHandle,WIHandle>>("GridPanel");
	Lua::WIGridPanel::register_class(wiGridPanelClassDef);
	guiMod[wiGridPanelClassDef];

	auto wiTreeListClassDef = luabind::class_<WITreeListHandle,luabind::bases<WITableHandle,WIHandle>>("TreeList");
	Lua::WITreeList::register_class(wiTreeListClassDef);

	auto wiTreeListElementClassDef = luabind::class_<WITreeListElementHandle,luabind::bases<WITableRowHandle,WIHandle>>("Element");
	Lua::WITreeListElement::register_class(wiTreeListElementClassDef);
	wiTreeListClassDef.scope[wiTreeListElementClassDef];
	guiMod[wiTreeListClassDef];

	auto wiFrameClassDef = luabind::class_<WIFrameHandle,luabind::bases<WITransformableHandle,WIHandle>>("Frame");
	Lua::WIFrame::register_class(wiFrameClassDef);
	guiMod[wiFrameClassDef];

	auto wiTextClassDef = luabind::class_<WITextHandle,WIHandle>("Text");
	Lua::WIText::register_class(wiTextClassDef);
	guiMod[wiTextClassDef];

	auto wiTextEntryClassDef = luabind::class_<WITextEntryHandle,WIHandle>("TextEntry");
	Lua::WITextEntry::register_class(wiTextEntryClassDef);
	wiTextEntryClassDef.def("GetCaretElement",static_cast<void(*)(lua_State*,WITextEntryHandle&)>([](lua_State *l,WITextEntryHandle &hEl) {
		lua_checkgui(l,hEl);
		auto *pCaretElement = static_cast<::WITextEntry*>(hEl.get())->GetCaretElement();
		if(pCaretElement == nullptr)
			return;
		auto oChild = WGUILuaInterface::GetLuaObject(l,*pCaretElement);
		oChild.push(l);
	}));
	guiMod[wiTextEntryClassDef];

	auto wiCommandLineEntryClassDef = luabind::class_<WICommandLineEntryHandle,luabind::bases<WITextEntryHandle,WIHandle>>("CommandLineEntry");
	wiCommandLineEntryClassDef.def("SetAutoCompleteEntryLimit",static_cast<void(*)(lua_State*,WICommandLineEntryHandle&,uint32_t)>([](lua_State *l,WICommandLineEntryHandle &hEl,uint32_t limit) {
		lua_checkgui(l,hEl);
		static_cast<::WICommandLineEntry*>(hEl.get())->SetAutoCompleteEntryLimit(limit);
	}));
	wiCommandLineEntryClassDef.def("GetAutoCompleteEntryLimit",static_cast<void(*)(lua_State*,WICommandLineEntryHandle&)>([](lua_State *l,WICommandLineEntryHandle &hEl) {
		lua_checkgui(l,hEl);
		Lua::PushInt(l,static_cast<::WICommandLineEntry*>(hEl.get())->GetAutoCompleteEntryLimit());
	}));
	guiMod[wiCommandLineEntryClassDef];

	auto wiOutlinedRectClassDef = luabind::class_<WIOutlinedRectHandle,WIHandle>("OutlinedRect");
	Lua::WIOutlinedRect::register_class(wiOutlinedRectClassDef);

	auto wiLineClassDef = luabind::class_<WILineHandle,WIHandle>("Line");
	Lua::WILine::register_class(wiLineClassDef);

	auto wiRoundedRectClassDef = luabind::class_<WIRoundedRectHandle,luabind::bases<WIShapeHandle,WIHandle>>("RoundedRect");
	Lua::WIRoundedRect::register_class(wiRoundedRectClassDef);
	guiMod[wiRoundedRectClassDef];

	auto wiRoundedTexturedRect = luabind::class_<WIRoundedTexturedRectHandle,luabind::bases<WITexturedShapeHandle,WIShapeHandle,WIHandle>>("RoundedTexturedRect");
	Lua::WIRoundedTexturedRect::register_class(wiRoundedTexturedRect);
	guiMod[wiRoundedTexturedRect];

	auto wiScrollBarClassDef = luabind::class_<WIScrollBarHandle,WIHandle>("ScrollBar");
	Lua::WIScrollBar::register_class(wiScrollBarClassDef);

	auto wiNumericEntryClassDef = luabind::class_<WINumericEntryHandle,luabind::bases<WITextEntryHandle,WIHandle>>("NumericTextEntry");
	Lua::WINumericEntry::register_class(wiNumericEntryClassDef);
	guiMod[wiNumericEntryClassDef];
	
	auto wiDropDownMenuClassDef = luabind::class_<WIDropDownMenuHandle,luabind::bases<WITextEntryHandle,WIHandle>>("DropDownMenu");
	Lua::WIDropDownMenu::register_class(wiDropDownMenuClassDef);
	guiMod[wiDropDownMenuClassDef];

	auto wiConsoleClassDef = luabind::class_<WIConsoleHandle,WIHandle>("Console");
	wiConsoleClassDef.def("GetCommandLineEntryElement",static_cast<void(*)(lua_State*,WIConsoleHandle&)>([](lua_State *l,WIConsoleHandle &hEl) {
		lua_checkgui(l,hEl);
		auto *pCommandLineEntry = static_cast<::WIConsole*>(hEl.get())->GetCommandLineEntryElement();
		if(pCommandLineEntry == nullptr)
			return;
		auto oChild = WGUILuaInterface::GetLuaObject(l,*pCommandLineEntry);
		oChild.push(l);
	}));
	wiConsoleClassDef.def("GetTextLogElement",static_cast<void(*)(lua_State*,WIConsoleHandle&)>([](lua_State *l,WIConsoleHandle &hEl) {
		lua_checkgui(l,hEl);
		auto *pLog = static_cast<::WIConsole*>(hEl.get())->GetTextLogElement();
		if(pLog == nullptr)
			return;
		auto oChild = WGUILuaInterface::GetLuaObject(l,*pLog);
		oChild.push(l);
	}));
	wiConsoleClassDef.def("GetFrame",static_cast<void(*)(lua_State*,WIConsoleHandle&)>([](lua_State *l,WIConsoleHandle &hEl) {
		lua_checkgui(l,hEl);
		auto *pFrame = static_cast<::WIConsole*>(hEl.get())->GetFrame();
		if(pFrame == nullptr)
			return;
		auto oChild = WGUILuaInterface::GetLuaObject(l,*pFrame);
		oChild.push(l);
	}));
	wiConsoleClassDef.def("GetText",static_cast<void(*)(lua_State*,WIConsoleHandle&)>([](lua_State *l,WIConsoleHandle &hEl) {
		lua_checkgui(l,hEl);
		Lua::PushString(l,static_cast<::WIConsole*>(hEl.get())->GetText());
	}));
	wiConsoleClassDef.def("SetText",static_cast<void(*)(lua_State*,WIConsoleHandle&,const std::string&)>([](lua_State *l,WIConsoleHandle &hEl,const std::string &text) {
		lua_checkgui(l,hEl);
		static_cast<::WIConsole*>(hEl.get())->SetText(text);
	}));
	wiConsoleClassDef.def("AppendText",static_cast<void(*)(lua_State*,WIConsoleHandle&,const std::string&)>([](lua_State *l,WIConsoleHandle &hEl,const std::string &text) {
		lua_checkgui(l,hEl);
		static_cast<::WIConsole*>(hEl.get())->AppendText(text);
	}));
	wiConsoleClassDef.def("Clear",static_cast<void(*)(lua_State*,WIConsoleHandle&)>([](lua_State *l,WIConsoleHandle &hEl) {
		lua_checkgui(l,hEl);
		static_cast<::WIConsole*>(hEl.get())->Clear();
	}));
	wiConsoleClassDef.def("SetMaxLogLineCount",static_cast<void(*)(lua_State*,WIConsoleHandle&,uint32_t)>([](lua_State *l,WIConsoleHandle &hEl,uint32_t maxLogLineCount) {
		lua_checkgui(l,hEl);
		static_cast<::WIConsole*>(hEl.get())->SetMaxLogLineCount(maxLogLineCount);
	}));
	wiConsoleClassDef.def("GetMaxLogLineCount",static_cast<void(*)(lua_State*,WIConsoleHandle&)>([](lua_State *l,WIConsoleHandle &hEl) {
		lua_checkgui(l,hEl);
		Lua::PushInt(l,static_cast<::WIConsole*>(hEl.get())->GetMaxLogLineCount());
	}));
	guiMod[wiConsoleClassDef];

	guiMod[wiOutlinedRectClassDef];
	guiMod[wiLineClassDef];
	guiMod[wiScrollBarClassDef];
}

void ClientState::RegisterSharedLuaLibraries(Lua::Interface &lua,bool bGUI)
{
	register_gui(lua);

	Lua::RegisterLibrary(lua.GetState(),"input",{
		{"get_mouse_button_state",Lua::input::get_mouse_button_state},
		{"get_key_state",Lua::input::get_key_state},
		{"add_callback",Lua::input::add_callback},
		{"get_cursor_pos",Lua::input::get_cursor_pos},
		{"set_cursor_pos",Lua::input::set_cursor_pos},
		{"get_controller_count",Lua::input::get_controller_count},
		{"get_controller_name",Lua::input::get_controller_name},
		{"get_controller_axes",Lua::input::get_joystick_axes},
		{"get_controller_buttons",Lua::input::get_joystick_buttons},

		{"key_to_string",Lua::input::key_to_string},
		{"key_to_text",Lua::input::key_to_text},
		{"string_to_key",Lua::input::string_to_key},
		{"get_mapped_keys",Lua::input::get_mapped_keys}
	});

	Lua::RegisterLibrary(lua.GetState(),"sound",{
		{"create",Lua::sound::create},
		LUA_LIB_SOUND_SHARED,
		{"create_dsp_effect",Lua::sound::register_aux_effect},
		{"get_dsp_effect",Lua::sound::get_aux_effect},
		{"set_distance_model",&Lua::sound::set_distance_model},
		{"get_distance_model",&Lua::sound::get_distance_model},
		{"is_supported",&Lua::sound::is_supported},
		{"get_doppler_factor",&Lua::sound::get_doppler_factor},
		{"set_doppler_factor",&Lua::sound::set_doppler_factor},
		{"get_speed_of_sound",&Lua::sound::get_speed_of_sound},
		{"set_speed_of_sound",&Lua::sound::set_speed_of_sound},
		{"get_device_name",&Lua::sound::get_device_name},
		{"add_global_effect",&Lua::sound::add_global_effect},
		{"remove_global_effect",&Lua::sound::remove_global_effect},
		{"set_global_effect_parameters",&Lua::sound::set_global_effect_parameters}
	});
	Lua::RegisterLibraryEnums(lua.GetState(),"sound",{
		{"GLOBAL_EFFECT_FLAG_NONE",umath::to_integral(al::SoundSystem::GlobalEffectFlag::None)},
		{"GLOBAL_EFFECT_FLAG_BIT_RELATIVE",umath::to_integral(al::SoundSystem::GlobalEffectFlag::RelativeSounds)},
		{"GLOBAL_EFFECT_FLAG_BIT_WORLD",umath::to_integral(al::SoundSystem::GlobalEffectFlag::WorldSounds)},
		{"GLOBAL_EFFECT_FLAG_ALL",umath::to_integral(al::SoundSystem::GlobalEffectFlag::All)},
		
		{"DISTANCE_MODEL_NONE",umath::to_integral(al::DistanceModel::None)},
		{"DISTANCE_MODEL_INVERSE_CLAMPED",umath::to_integral(al::DistanceModel::InverseClamped)},
		{"DISTANCE_MODEL_LINEAR_CLAMPED",umath::to_integral(al::DistanceModel::LinearClamped)},
		{"DISTANCE_MODEL_EXPONENT_CLAMPED",umath::to_integral(al::DistanceModel::ExponentClamped)},
		{"DISTANCE_MODEL_INVERSE",umath::to_integral(al::DistanceModel::Inverse)},
		{"DISTANCE_MODEL_LINEAR",umath::to_integral(al::DistanceModel::Linear)},
		{"DISTANCE_MODEL_EXPONENT",umath::to_integral(al::DistanceModel::Exponent)}
	});
	Lua::sound::register_enums(lua.GetState());

	auto classDefAlEffect = luabind::class_<al::PEffect>("Effect");
	classDefAlEffect.def("SetProperties",static_cast<void(*)(lua_State*,al::PEffect&,const al::EfxEaxReverbProperties&)>(&Lua::ALEffect::SetProperties));
	classDefAlEffect.def("SetProperties",static_cast<void(*)(lua_State*,al::PEffect&,const al::EfxChorusProperties&)>(&Lua::ALEffect::SetProperties));
	classDefAlEffect.def("SetProperties",static_cast<void(*)(lua_State*,al::PEffect&,const al::EfxDistortionProperties&)>(&Lua::ALEffect::SetProperties));
	classDefAlEffect.def("SetProperties",static_cast<void(*)(lua_State*,al::PEffect&,const al::EfxEchoProperties&)>(&Lua::ALEffect::SetProperties));
	classDefAlEffect.def("SetProperties",static_cast<void(*)(lua_State*,al::PEffect&,const al::EfxFlangerProperties&)>(&Lua::ALEffect::SetProperties));
	classDefAlEffect.def("SetProperties",static_cast<void(*)(lua_State*,al::PEffect&,const al::EfxFrequencyShifterProperties&)>(&Lua::ALEffect::SetProperties));
	classDefAlEffect.def("SetProperties",static_cast<void(*)(lua_State*,al::PEffect&,const al::EfxVocalMorpherProperties&)>(&Lua::ALEffect::SetProperties));
	classDefAlEffect.def("SetProperties",static_cast<void(*)(lua_State*,al::PEffect&,const al::EfxPitchShifterProperties&)>(&Lua::ALEffect::SetProperties));
	classDefAlEffect.def("SetProperties",static_cast<void(*)(lua_State*,al::PEffect&,const al::EfxRingModulatorProperties&)>(&Lua::ALEffect::SetProperties));
	classDefAlEffect.def("SetProperties",static_cast<void(*)(lua_State*,al::PEffect&,const al::EfxAutoWahProperties&)>(&Lua::ALEffect::SetProperties));
	classDefAlEffect.def("SetProperties",static_cast<void(*)(lua_State*,al::PEffect&,const al::EfxCompressor&)>(&Lua::ALEffect::SetProperties));
	classDefAlEffect.def("SetProperties",static_cast<void(*)(lua_State*,al::PEffect&,const al::EfxEqualizer&)>(&Lua::ALEffect::SetProperties));

	auto classDefEaxReverb = luabind::class_<al::EfxEaxReverbProperties>("ReverbProperties");
	classDefEaxReverb.def(luabind::constructor<>());
	classDefEaxReverb.def_readwrite("density",&al::EfxEaxReverbProperties::flDensity);
	classDefEaxReverb.def_readwrite("diffusion",&al::EfxEaxReverbProperties::flDiffusion);
	classDefEaxReverb.def_readwrite("gain",&al::EfxEaxReverbProperties::flGain);
	classDefEaxReverb.def_readwrite("gainHF",&al::EfxEaxReverbProperties::flGainHF);
	classDefEaxReverb.def_readwrite("gainLF",&al::EfxEaxReverbProperties::flGainLF);
	classDefEaxReverb.def_readwrite("decayTime",&al::EfxEaxReverbProperties::flDecayTime);
	classDefEaxReverb.def_readwrite("decayHFRatio",&al::EfxEaxReverbProperties::flDecayHFRatio);
	classDefEaxReverb.def_readwrite("decayLFRatio",&al::EfxEaxReverbProperties::flDecayLFRatio);
	classDefEaxReverb.def_readwrite("reflectionsGain",&al::EfxEaxReverbProperties::flReflectionsGain);
	classDefEaxReverb.def_readwrite("reflectionsDelay",&al::EfxEaxReverbProperties::flReflectionsDelay);
	classDefEaxReverb.def_readwrite("reflectionsPan",reinterpret_cast<Vector3 al::EfxEaxReverbProperties::*>(&al::EfxEaxReverbProperties::flReflectionsPan));
	classDefEaxReverb.def_readwrite("lateReverbGain",&al::EfxEaxReverbProperties::flLateReverbGain);
	classDefEaxReverb.def_readwrite("lateReverbDelay",&al::EfxEaxReverbProperties::flLateReverbDelay);
	classDefEaxReverb.def_readwrite("lateReverbPan",reinterpret_cast<Vector3 al::EfxEaxReverbProperties::*>(&al::EfxEaxReverbProperties::flLateReverbPan));
	classDefEaxReverb.def_readwrite("echoTime",&al::EfxEaxReverbProperties::flEchoTime);
	classDefEaxReverb.def_readwrite("echoDepth",&al::EfxEaxReverbProperties::flEchoDepth);
	classDefEaxReverb.def_readwrite("modulationTime",&al::EfxEaxReverbProperties::flModulationTime);
	classDefEaxReverb.def_readwrite("modulationDepth",&al::EfxEaxReverbProperties::flModulationDepth);
	classDefEaxReverb.def_readwrite("airAbsorptionGainHF",&al::EfxEaxReverbProperties::flAirAbsorptionGainHF);
	classDefEaxReverb.def_readwrite("hfReference",&al::EfxEaxReverbProperties::flHFReference);
	classDefEaxReverb.def_readwrite("lfReference",&al::EfxEaxReverbProperties::flLFReference);
	classDefEaxReverb.def_readwrite("roomRolloffFactor",&al::EfxEaxReverbProperties::flRoomRolloffFactor);
	classDefEaxReverb.def_readwrite("decayHFLimit",&al::EfxEaxReverbProperties::iDecayHFLimit);
	classDefAlEffect.scope[classDefEaxReverb];

	auto classDefChorus = luabind::class_<al::EfxChorusProperties>("ChorusProperties");
	classDefChorus.def(luabind::constructor<>());
	classDefChorus.def_readwrite("waveform",&al::EfxChorusProperties::iWaveform);
	classDefChorus.def_readwrite("phase",&al::EfxChorusProperties::iPhase);
	classDefChorus.def_readwrite("rate",&al::EfxChorusProperties::flRate);
	classDefChorus.def_readwrite("depth",&al::EfxChorusProperties::flDepth);
	classDefChorus.def_readwrite("feedback",&al::EfxChorusProperties::flFeedback);
	classDefChorus.def_readwrite("delay",&al::EfxChorusProperties::flDelay);
	classDefAlEffect.scope[classDefChorus];

	auto classDefDistortionProperties = luabind::class_<al::EfxDistortionProperties>("DistortionProperties");
	classDefDistortionProperties.def(luabind::constructor<>());
	classDefDistortionProperties.def_readwrite("edge",&al::EfxDistortionProperties::flEdge);
	classDefDistortionProperties.def_readwrite("gain",&al::EfxDistortionProperties::flGain);
	classDefDistortionProperties.def_readwrite("lowpassCutoff",&al::EfxDistortionProperties::flLowpassCutoff);
	classDefDistortionProperties.def_readwrite("eqCenter",&al::EfxDistortionProperties::flEQCenter);
	classDefDistortionProperties.def_readwrite("eqBandwidth",&al::EfxDistortionProperties::flEQBandwidth);
	classDefAlEffect.scope[classDefDistortionProperties];

	auto classDefEchoProperties = luabind::class_<al::EfxEchoProperties>("EchoProperties");
	classDefEchoProperties.def(luabind::constructor<>());
	classDefEchoProperties.def_readwrite("delay",&al::EfxEchoProperties::flDelay);
	classDefEchoProperties.def_readwrite("lrDelay",&al::EfxEchoProperties::flLRDelay);
	classDefEchoProperties.def_readwrite("damping",&al::EfxEchoProperties::flDamping);
	classDefEchoProperties.def_readwrite("feedback",&al::EfxEchoProperties::flFeedback);
	classDefEchoProperties.def_readwrite("spread",&al::EfxEchoProperties::flSpread);
	classDefAlEffect.scope[classDefEchoProperties];

	auto classDefFlangerProperties = luabind::class_<al::EfxFlangerProperties>("FlangerProperties");
	classDefFlangerProperties.def(luabind::constructor<>());
	classDefFlangerProperties.def_readwrite("waveform",&al::EfxFlangerProperties::iWaveform);
	classDefFlangerProperties.def_readwrite("phase",&al::EfxFlangerProperties::iPhase);
	classDefFlangerProperties.def_readwrite("rate",&al::EfxFlangerProperties::flRate);
	classDefFlangerProperties.def_readwrite("depth",&al::EfxFlangerProperties::flDepth);
	classDefFlangerProperties.def_readwrite("feedback",&al::EfxFlangerProperties::flFeedback);
	classDefFlangerProperties.def_readwrite("delay",&al::EfxFlangerProperties::flDelay);
	classDefAlEffect.scope[classDefFlangerProperties];

	auto classDefFrequencyShifterProperties = luabind::class_<al::EfxFrequencyShifterProperties>("FrequencyShifterProperties");
	classDefFrequencyShifterProperties.def(luabind::constructor<>());
	classDefFrequencyShifterProperties.def_readwrite("frequency",&al::EfxFrequencyShifterProperties::flFrequency);
	classDefFrequencyShifterProperties.def_readwrite("leftDirection",&al::EfxFrequencyShifterProperties::iLeftDirection);
	classDefFrequencyShifterProperties.def_readwrite("rightDirection",&al::EfxFrequencyShifterProperties::iRightDirection);
	classDefAlEffect.scope[classDefFrequencyShifterProperties];

	auto classDefVocalMorpherProperties = luabind::class_<al::EfxVocalMorpherProperties>("VocalMorpherProperties");
	classDefVocalMorpherProperties.def(luabind::constructor<>());
	classDefVocalMorpherProperties.def_readwrite("phonemeA",&al::EfxVocalMorpherProperties::iPhonemeA);
	classDefVocalMorpherProperties.def_readwrite("phonemeB",&al::EfxVocalMorpherProperties::iPhonemeB);
	classDefVocalMorpherProperties.def_readwrite("phonemeACoarseTuning",&al::EfxVocalMorpherProperties::iPhonemeACoarseTuning);
	classDefVocalMorpherProperties.def_readwrite("phonemeBCoarseTuning",&al::EfxVocalMorpherProperties::iPhonemeBCoarseTuning);
	classDefVocalMorpherProperties.def_readwrite("waveform",&al::EfxVocalMorpherProperties::iWaveform);
	classDefVocalMorpherProperties.def_readwrite("rate",&al::EfxVocalMorpherProperties::flRate);
	classDefAlEffect.scope[classDefVocalMorpherProperties];

	auto classDefPitchShifterProperties = luabind::class_<al::EfxPitchShifterProperties>("PitchShifterProperties");
	classDefPitchShifterProperties.def(luabind::constructor<>());
	classDefPitchShifterProperties.def_readwrite("coarseTune",&al::EfxPitchShifterProperties::iCoarseTune);
	classDefPitchShifterProperties.def_readwrite("fineTune",&al::EfxPitchShifterProperties::iFineTune);
	classDefAlEffect.scope[classDefPitchShifterProperties];

	auto classDefRingModulatorProperties = luabind::class_<al::EfxRingModulatorProperties>("RingModulatorProperties");
	classDefRingModulatorProperties.def(luabind::constructor<>());
	classDefRingModulatorProperties.def_readwrite("frequency",&al::EfxRingModulatorProperties::flFrequency);
	classDefRingModulatorProperties.def_readwrite("highpassCutoff",&al::EfxRingModulatorProperties::flHighpassCutoff);
	classDefRingModulatorProperties.def_readwrite("waveform",&al::EfxRingModulatorProperties::iWaveform);
	classDefAlEffect.scope[classDefRingModulatorProperties];

	auto classDefAutoWahProperties = luabind::class_<al::EfxAutoWahProperties>("AutoWahProperties");
	classDefAutoWahProperties.def(luabind::constructor<>());
	classDefAutoWahProperties.def_readwrite("attackTime",&al::EfxAutoWahProperties::flAttackTime);
	classDefAutoWahProperties.def_readwrite("releaseTime",&al::EfxAutoWahProperties::flReleaseTime);
	classDefAutoWahProperties.def_readwrite("resonance",&al::EfxAutoWahProperties::flResonance);
	classDefAutoWahProperties.def_readwrite("peakGain",&al::EfxAutoWahProperties::flPeakGain);
	classDefAlEffect.scope[classDefAutoWahProperties];

	auto classDefCompressor = luabind::class_<al::EfxCompressor>("CompressorProperties");
	classDefCompressor.def(luabind::constructor<>());
	classDefCompressor.def_readwrite("onOff",&al::EfxCompressor::iOnOff);
	classDefAlEffect.scope[classDefCompressor];

	auto classDefEqualizer = luabind::class_<al::EfxEqualizer>("EqualizerProperties");
	classDefEqualizer.def(luabind::constructor<>());
	classDefEqualizer.def_readwrite("lowGain",&al::EfxEqualizer::flLowGain);
	classDefEqualizer.def_readwrite("lowCutoff",&al::EfxEqualizer::flLowCutoff);
	classDefEqualizer.def_readwrite("mid1Gain",&al::EfxEqualizer::flMid1Gain);
	classDefEqualizer.def_readwrite("mid1Center",&al::EfxEqualizer::flMid1Center);
	classDefEqualizer.def_readwrite("mid1Width",&al::EfxEqualizer::flMid1Width);
	classDefEqualizer.def_readwrite("mid2Gain",&al::EfxEqualizer::flMid2Gain);
	classDefEqualizer.def_readwrite("mid2Center",&al::EfxEqualizer::flMid2Center);
	classDefEqualizer.def_readwrite("mid2Width",&al::EfxEqualizer::flMid2Width);
	classDefEqualizer.def_readwrite("highGain",&al::EfxEqualizer::flHighGain);
	classDefEqualizer.def_readwrite("highCutoff",&al::EfxEqualizer::flHighCutoff);
	classDefAlEffect.scope[classDefEqualizer];

	auto alSoundClassDef = luabind::class_<ALSound>("Source");
	Lua::ALSound::Client::register_class(alSoundClassDef);

	auto alBufferClassDef = luabind::class_<al::SoundBuffer>("Source");
	Lua::ALSound::Client::register_buffer(alBufferClassDef);

	auto soundMod = luabind::module(lua.GetState(),"sound");
	soundMod[classDefAlEffect];
	soundMod[alSoundClassDef];
	soundMod[alBufferClassDef];

	RegisterVulkanLuaInterface(lua);
}
void CGame::RegisterLuaLibraries()
{
	GetLuaInterface().RegisterLibrary("util",{
		REGISTER_SHARED_UTIL
		{"calc_world_direction_from_2d_coordinates",Lua::util::Client::calc_world_direction_from_2d_coordinates},
		{"create_particle_tracer",Lua::util::Client::create_particle_tracer},
		{"fire_bullets",Lua::util::fire_bullets},
		{"create_muzzle_flash",Lua::util::Client::create_muzzle_flash},
		{"create_giblet",Lua::util::Client::create_giblet}
	});

	Lua::ai::client::register_library(GetLuaInterface());

	Game::RegisterLuaLibraries();
	ClientState::RegisterSharedLuaLibraries(GetLuaInterface());

	std::vector<luaL_Reg> debugFuncs = {
		{"draw_points",Lua::DebugRenderer::Client::DrawPoints},
		{"draw_lines",Lua::DebugRenderer::Client::DrawLines},
		{"draw_point",Lua::DebugRenderer::Client::DrawPoint},
		{"draw_line",Lua::DebugRenderer::Client::DrawLine},
		{"draw_box",Lua::DebugRenderer::Client::DrawBox},
		{"draw_mesh",Lua::DebugRenderer::Client::DrawMeshes},
		{"draw_sphere",Lua::DebugRenderer::Client::DrawSphere},
		{"draw_cone",Lua::DebugRenderer::Client::DrawCone},
		{"draw_truncated_cone",Lua::DebugRenderer::Client::DrawTruncatedCone},
		{"draw_cylinder",Lua::DebugRenderer::Client::DrawCylinder},
		{"draw_axis",Lua::DebugRenderer::Client::DrawAxis},
		{"draw_text",Lua::DebugRenderer::Client::DrawText},
		{"draw_path",Lua::DebugRenderer::Client::DrawPath},
		{"draw_spline",Lua::DebugRenderer::Client::DrawSpline},
		{"draw_plane",Lua::DebugRenderer::Client::DrawPlane},
		{"draw_frustum",Lua::DebugRenderer::Client::DrawFrustum}
	};
	for(auto &f : debugFuncs)
	{
		lua_pushtablecfunction(GetLuaState(),"debug",(f.name),(f.func));
	}
}
