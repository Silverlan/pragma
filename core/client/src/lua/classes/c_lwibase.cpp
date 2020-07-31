/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/lua/classes/c_lwibase.h"
#include <wgui/wibase.h>
#include <wgui/wihandle.h>
#include "luasystem.h"
#include <wgui/types/wirect.h>
#include "pragma/gui/wisilkicon.h"
#include "pragma/gui/wisnaparea.hpp"
#include "pragma/lua/classes/c_ldef_wgui.h"
#include "pragma/lua/libraries/c_lua_vulkan.h"
#include "pragma/gui/wgui_luainterface.h"
#include <pragma/lua/classes/ldef_vector.h>
#include <pragma/lua/classes/lproperty.hpp>
#include <pragma/lua/lua_call.hpp>
#include <sharedutils/property/util_property_color.hpp>
#include <util_formatted_text.hpp>

DEFINE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WI,WIShape,WIShape);
DEFINE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WI,WIText,WIText);
DEFINE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WI,WITextEntry,WITextEntry);
DEFINE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WITextEntry,WICommandLineEntry,WICommandLineEntry);
DEFINE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WI,WIOutlinedRect,WIOutlinedRect);
DEFINE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WI,WILine,WILine);
DEFINE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WI,WIScrollBar,WIScrollBar);
DEFINE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WI,WIButton,WIButton);
DEFINE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WI,WIContainer,WIContainer);
DEFINE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WI,WIScrollContainer,WIScrollContainer);
DEFINE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WI,WIProgressBar,WIProgressBar);
DEFINE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WI,WITransformable,WITransformable);
DEFINE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WITransformable,WIFrame,WIFrame);
DEFINE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WI,WIDebugDepthTexture,WIDebugDepthTexture);
DEFINE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WI,WIDebugShadowMap,WIDebugShadowMap);
DEFINE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WI,WISnapArea,WISnapArea);
DEFINE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WI,WIConsole,WIConsole);

DEFINE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WIContainer,WITable,WITable);
DEFINE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WIContainer,WITableRow,WITableRow);
DEFINE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WIContainer,WITableCell,WITableCell);
DEFINE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WITextEntry,WINumericEntry,WINumericEntry);
DEFINE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WITextEntry,WIDropDownMenu,WIDropDownMenu);
DEFINE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WIProgressBar,WISlider,WISlider);
DEFINE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WITable,WITreeList,WITreeList);
DEFINE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WITable,WIGridPanel,WIGridPanel);
DEFINE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WITableRow,WITreeListElement,WITreeListElement);
DEFINE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WIShape,WIArrow,WIArrow);
DEFINE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WIShape,WICheckbox,WICheckbox);
DEFINE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WIShape,WIRoundedRect,WIRoundedRect);
DEFINE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WIShape,WITexturedShape,WITexturedShape);
DEFINE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WITexturedShape,WITexturedRect,WITexturedRect);
DEFINE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WITexturedShape,WIIcon,WIIcon);
DEFINE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WITexturedShape,WIRoundedTexturedRect,WIRoundedTexturedRect);
DEFINE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WIIcon,WISilkIcon,WISilkIcon);
DEFINE_DERIVED_CHILD_HANDLE(DLLCLIENT,WI,WIBase,WITexturedShape,WIDebugSSAO,WIDebugSSAO);

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

DLLCLIENT Con::c_cout & operator<<(Con::c_cout &os,const WIHandle &handle)
{
	if(!handle.IsValid())
		os<<"NULL";
	else
	{
		WIBase *p = handle.get();
		os<<"WIElement["<<p->GetClass()<<"]["<<p->GetName()<<"]["<<p->GetIndex()<<"]["<<&handle<<"]";
	}
	return os;
}
DLLCLIENT std::ostream& operator<<(std::ostream &os,const WIHandle &handle)
{
	if(!handle.IsValid())
		os<<"NULL";
	else
	{
		WIBase *p = handle.get();
		os<<"WIElement["<<p->GetClass()<<"]["<<p->GetName()<<"]["<<p->GetIndex()<<"]["<<&handle<<"]";
	}
	return os;
}

extern DLLCLIENT ClientState *client;

static bool operator==(WIHandle &a,WIHandle &b)
{
	if(!a.IsValid())
		return !b.IsValid();
	return a.get() == b.get();
}

void Lua::WIBase::register_class(luabind::class_<WIHandle> &classDef)
{
	classDef.def(luabind::tostring(luabind::self));
	classDef.def(luabind::self ==luabind::self);
	classDef.def("IsValid",&IsValid);
	classDef.def("GetIndex",static_cast<void(*)(lua_State*,WIHandle&)>([](lua_State *l,WIHandle &hPanel) {
		lua_checkgui(l,hPanel);
		Lua::PushInt(l,hPanel->GetIndex());
	}));
	classDef.def("Remove",&Remove);
	classDef.def("RemoveSafely",&RemoveSafely);
	classDef.def("SetZPos",&SetZPos);
	classDef.def("GetZPos",&GetZPos);
	classDef.def("HasFocus",&HasFocus);
	classDef.def("RequestFocus",&RequestFocus);
	classDef.def("KillFocus",&KillFocus);
	classDef.def("TrapFocus",static_cast<void(*)(lua_State*,WIHandle&,bool)>(&TrapFocus));
	classDef.def("TrapFocus",static_cast<void(*)(lua_State*,WIHandle&)>(&TrapFocus));
	classDef.def("IsFocusTrapped",&IsFocusTrapped);
	classDef.def("IsHidden",static_cast<void(*)(lua_State*,WIHandle&)>([](lua_State *l,WIHandle &hPanel) {
		lua_checkgui(l,hPanel);
		Lua::PushBool(l,!hPanel->IsVisible());
	}));
	classDef.def("SetVisible",&SetVisible);
	classDef.def("IsVisible",static_cast<void(*)(lua_State*,WIHandle&)>([](lua_State *l,WIHandle &hPanel) {
		lua_checkgui(l,hPanel);
		Lua::PushBool(l,hPanel->IsSelfVisible());
	}));
	classDef.def("IsParentVisible",static_cast<void(*)(lua_State*,WIHandle&)>([](lua_State *l,WIHandle &hPanel) {
		lua_checkgui(l,hPanel);
		Lua::PushBool(l,hPanel->IsParentVisible());
	}));
	classDef.def("SetAutoSizeToContents",static_cast<void(*)(lua_State*,WIHandle&)>([](lua_State *l,WIHandle &hPanel) {
		lua_checkgui(l,hPanel);
		hPanel->SetAutoSizeToContents(true);
	}));
	classDef.def("SetAutoSizeToContents",static_cast<void(*)(lua_State*,WIHandle&,bool)>([](lua_State *l,WIHandle &hPanel,bool autoSize) {
		lua_checkgui(l,hPanel);
		hPanel->SetAutoSizeToContents(autoSize);
	}));
	classDef.def("SetAutoSizeToContents",static_cast<void(*)(lua_State*,WIHandle&,bool,bool)>([](lua_State *l,WIHandle &hPanel,bool x,bool y) {
		lua_checkgui(l,hPanel);
		hPanel->SetAutoSizeToContents(x,y);
	}));
	classDef.def("GetMouseInputEnabled",&GetMouseInputEnabled);
	classDef.def("SetMouseInputEnabled",&SetMouseInputEnabled);
	classDef.def("GetKeyboardInputEnabled",&GetKeyboardInputEnabled);
	classDef.def("SetKeyboardInputEnabled",&SetKeyboardInputEnabled);
	classDef.def("SetScrollInputEnabled",&SetScrollInputEnabled);
	classDef.def("GetScrollInputEnabled",&GetScrollInputEnabled);
	classDef.def("SetCursorMovementCheckEnabled",&SetMouseMovementCheckEnabled);
	classDef.def("GetCursorMovementCheckEnabled",&GetMouseMovementCheckEnabled);
	classDef.def("GetPos",&GetPos);
	classDef.def("SetPos",static_cast<void(*)(lua_State*,WIHandle&,Vector2)>(&SetPos));
	classDef.def("SetPos",static_cast<void(*)(lua_State*,WIHandle&,float,float)>(&SetPos));
	classDef.def("GetAbsolutePos",&GetAbsolutePos);
	classDef.def("SetAbsolutePos",static_cast<void(*)(lua_State*,WIHandle&,Vector2)>(&SetAbsolutePos));
	classDef.def("SetAbsolutePos",static_cast<void(*)(lua_State*,WIHandle&,float,float)>(&SetAbsolutePos));
	classDef.def("GetColor",&GetColor);
	classDef.def("GetColorProperty",&GetColorProperty);
	classDef.def("GetFocusProperty",static_cast<void(*)(lua_State*,WIHandle&)>([](lua_State *l,WIHandle &hPanel) {
		lua_checkgui(l,hPanel);
		Lua::Property::push(l,*hPanel->GetFocusProperty());
	}));
	classDef.def("GetVisibilityProperty",static_cast<void(*)(lua_State*,WIHandle&)>([](lua_State *l,WIHandle &hPanel) {
		lua_checkgui(l,hPanel);
		Lua::Property::push(l,*hPanel->GetVisibilityProperty());
	}));
	classDef.def("GetPosProperty",static_cast<void(*)(lua_State*,WIHandle&)>([](lua_State *l,WIHandle &hPanel) {
		lua_checkgui(l,hPanel);
		Lua::Property::push(l,*hPanel->GetPosProperty());
	}));
	classDef.def("GetSizeProperty",static_cast<void(*)(lua_State*,WIHandle&)>([](lua_State *l,WIHandle &hPanel) {
		lua_checkgui(l,hPanel);
		Lua::Property::push(l,*hPanel->GetSizeProperty());
	}));
	classDef.def("GetMouseInBoundsProperty",static_cast<void(*)(lua_State*,WIHandle&)>([](lua_State *l,WIHandle &hPanel) {
		lua_checkgui(l,hPanel);
		Lua::Property::push(l,*hPanel->GetMouseInBoundsProperty());
	}));
	classDef.def("SetColor",&SetColor);
	classDef.def("SetColorRGB",static_cast<void(*)(lua_State*,WIHandle&,const Color&)>([](lua_State *l,WIHandle &hPanel,const Color &color) {
		lua_checkgui(l,hPanel);
		auto vCol = color.ToVector4();
		vCol.a = hPanel->GetAlpha();
		hPanel->SetColor(vCol);
	}));
	classDef.def("GetAlpha",&GetAlpha);
	classDef.def("SetAlpha",&SetAlpha);
	classDef.def("GetWidth",&GetWidth);
	classDef.def("GetHeight",&GetHeight);
	classDef.def("GetSize",&GetSize);
	classDef.def("SetSize",static_cast<void(*)(lua_State*,WIHandle&,Vector2)>(&SetSize));
	classDef.def("SetSize",static_cast<void(*)(lua_State*,WIHandle&,float,float)>(&SetSize));
	classDef.def("Wrap",static_cast<void(*)(lua_State*,WIHandle&,const std::string&)>(&Wrap));
	classDef.def("Wrap",static_cast<void(*)(lua_State*,WIHandle&,WIHandle&)>(&Wrap));
	classDef.def("GetParent",&GetParent);
	classDef.def("SetParent",&SetParent);
	classDef.def("SetParent",static_cast<void(*)(lua_State*,WIHandle&,WIHandle&,uint32_t)>([](lua_State *l,WIHandle &hPanel,WIHandle &hParent,uint32_t index) {
		lua_checkgui(l,hPanel);
		lua_checkgui(l,hParent);
		hPanel->SetParent(hParent.get(),index);
	}));
	classDef.def("ClearParent",&ClearParent);
	classDef.def("GetChildren",static_cast<void(*)(lua_State*,WIHandle&)>(&GetChildren));
	classDef.def("GetChildren",static_cast<void(*)(lua_State*,WIHandle&,std::string)>(&GetChildren));
	classDef.def("GetFirstChild",&GetFirstChild);
	classDef.def("GetChild",static_cast<void(*)(lua_State*,WIHandle&,unsigned int)>(&GetChild));
	classDef.def("GetChild",static_cast<void(*)(lua_State*,WIHandle&,std::string,unsigned int)>(&GetChild));
	classDef.def("IsPosInBounds",&PosInBounds);
	classDef.def("IsCursorInBounds",&MouseInBounds);
	classDef.def("GetCursorPos",&GetMousePos);
	classDef.def("Draw",static_cast<void(*)(lua_State*,WIHandle&,const ::WIBase::DrawInfo&,const Vector2i&,const Vector2i&,const Vector2i&)>(&Draw));
	classDef.def("Draw",static_cast<void(*)(lua_State*,WIHandle&,const ::WIBase::DrawInfo&,const Vector2i&,const Vector2i&)>(&Draw));
	classDef.def("Draw",static_cast<void(*)(lua_State*,WIHandle&,const ::WIBase::DrawInfo&)>(&Draw));
	classDef.def("GetX",&GetX);
	classDef.def("GetY",&GetY);
	classDef.def("SetX",&SetX);
	classDef.def("SetY",&SetY);
	classDef.def("SetWidth",&SetWidth);
	classDef.def("SetHeight",&SetHeight);
	classDef.def("SizeToContents",&SizeToContents);
	classDef.def("AddCallback",&AddCallback);
	classDef.def("CallCallbacks",static_cast<void(*)(lua_State*,WIHandle&,std::string)>(&CallCallbacks));
	classDef.def("CallCallbacks",static_cast<void(*)(lua_State*,WIHandle&,std::string,luabind::object)>(&CallCallbacks));
	classDef.def("CallCallbacks",static_cast<void(*)(lua_State*,WIHandle&,std::string,luabind::object,luabind::object)>(&CallCallbacks));
	classDef.def("CallCallbacks",static_cast<void(*)(lua_State*,WIHandle&,std::string,luabind::object,luabind::object,luabind::object)>(&CallCallbacks));
	classDef.def("CallCallbacks",static_cast<void(*)(lua_State*,WIHandle&,std::string,luabind::object,luabind::object,luabind::object,luabind::object)>(&CallCallbacks));
	classDef.def("CallCallbacks",static_cast<void(*)(lua_State*,WIHandle&,std::string,luabind::object,luabind::object,luabind::object,luabind::object,luabind::object)>(&CallCallbacks));
	classDef.def("CallCallbacks",static_cast<void(*)(lua_State*,WIHandle&,std::string,luabind::object,luabind::object,luabind::object,luabind::object,luabind::object,luabind::object)>(&CallCallbacks));
	classDef.def("CallCallbacks",static_cast<void(*)(lua_State*,WIHandle&,std::string,luabind::object,luabind::object,luabind::object,luabind::object,luabind::object,luabind::object,luabind::object)>(&CallCallbacks));
	classDef.def("CallCallbacks",static_cast<void(*)(lua_State*,WIHandle&,std::string,luabind::object,luabind::object,luabind::object,luabind::object,luabind::object,luabind::object,luabind::object,luabind::object)>(&CallCallbacks));
	classDef.def("FadeIn",static_cast<void(*)(lua_State*,WIHandle&,float)>(&FadeIn));
	classDef.def("FadeIn",static_cast<void(*)(lua_State*,WIHandle&,float,float)>(&FadeIn));
	classDef.def("FadeOut",static_cast<void(*)(lua_State*,WIHandle&,float)>(&FadeOut));
	classDef.def("FadeOut",static_cast<void(*)(lua_State*,WIHandle&,float,bool)>(&FadeOut));
	classDef.def("IsFading",&IsFading);
	classDef.def("IsFadingOut",&IsFadingOut);
	classDef.def("IsFadingIn",&IsFadingIn);
	classDef.def("GetClass",&GetClass);
	classDef.def("Think",&Think);
	classDef.def("InjectMouseMoveInput",&InjectMouseMoveInput);
	classDef.def("InjectMouseInput",static_cast<void(*)(lua_State*,WIHandle&,const Vector2&,int,int,int)>(&InjectMouseInput));
	classDef.def("InjectMouseInput",static_cast<void(*)(lua_State*,WIHandle&,const Vector2&,int,int)>(&InjectMouseInput));
	classDef.def("InjectKeyboardInput",static_cast<void(*)(lua_State*,WIHandle&,int,int,int)>(&InjectKeyboardInput));
	classDef.def("InjectKeyboardInput",static_cast<void(*)(lua_State*,WIHandle&,int,int)>(&InjectKeyboardInput));
	classDef.def("InjectCharInput",static_cast<void(*)(lua_State*,WIHandle&,std::string,uint32_t)>(&InjectCharInput));
	classDef.def("InjectCharInput",static_cast<void(*)(lua_State*,WIHandle&,std::string)>(&InjectCharInput));
	classDef.def("InjectScrollInput",&InjectScrollInput);
	classDef.def("IsDescendant",&IsDescendant);
	classDef.def("IsDescendantOf",&IsDescendantOf);
	classDef.def("IsAncestor",&IsAncestor);
	classDef.def("IsAncestorOf",&IsAncestorOf);
	classDef.def("GetName",&GetName);
	classDef.def("SetName",&SetName);
	classDef.def("FindChildByName",&FindChildByName);
	classDef.def("FindChildrenByName",&FindChildrenByName);
	classDef.def("SetAutoAlignToParent",static_cast<void(*)(lua_State*,WIHandle&,bool,bool)>(&SetAutoAlignToParent));
	classDef.def("SetAutoAlignToParent",static_cast<void(*)(lua_State*,WIHandle&,bool)>(&SetAutoAlignToParent));
	classDef.def("GetAutoAlignToParentX",&GetAutoAlignToParentX);
	classDef.def("GetAutoAlignToParentY",&GetAutoAlignToParentY);
	classDef.def("Resize",&Resize);
	classDef.def("ScheduleUpdate",&ScheduleUpdate);
	classDef.def("SetSkin",&SetSkin);
	classDef.def("ResetSkin",&ResetSkin);
	classDef.def("GetStyleClasses",&GetStyleClasses);
	classDef.def("AddStyleClass",&AddStyleClass);
	classDef.def("SetCursor",&SetCursor);
	classDef.def("GetCursor",&GetCursor);
	classDef.def("RemoveElementOnRemoval",&RemoveElementOnRemoval);
	classDef.def("GetTooltip",&GetTooltip);
	classDef.def("SetTooltip",&SetTooltip);
	classDef.def("HasTooltip",&HasTooltip);
	classDef.def("GetLeft",&GetLeft);
	classDef.def("GetTop",&GetTop);
	classDef.def("GetRight",&GetRight);
	classDef.def("GetBottom",&GetBottom);
	classDef.def("GetEndPos",&GetEndPos);
	classDef.def("SetClippingEnabled",&SetClippingEnabled);
	classDef.def("IsClippingEnabled",&IsClippingEnabled);
	classDef.def("SetAlwaysUpdate",&SetAlwaysUpdate);
	classDef.def("SetBounds",&SetBounds);
	classDef.def("SetBackgroundElement",static_cast<void(*)(lua_State*,WIHandle&,bool,bool)>(&SetBackgroundElement));
	classDef.def("SetBackgroundElement",static_cast<void(*)(lua_State*,WIHandle&,bool)>(&SetBackgroundElement));
	classDef.def("SetBackgroundElement",static_cast<void(*)(lua_State*,WIHandle&)>(&SetBackgroundElement));
	classDef.def("IsBackgroundElement",&IsBackgroundElement);
	classDef.def("FindDescendantByName",static_cast<void(*)(lua_State*,WIHandle&,const std::string&)>([](lua_State *l,WIHandle &hPanel,const std::string &name) {
		lua_checkgui(l,hPanel);
		auto *el = hPanel->FindDescendantByName(name);
		if(el == nullptr)
			return;
		auto oChild = WGUILuaInterface::GetLuaObject(l,*el);
		oChild.push(l);
	}));
	classDef.def("FindDescendantsByName",static_cast<void(*)(lua_State*,WIHandle&,const std::string&)>([](lua_State *l,WIHandle &hPanel,const std::string &name) {
		lua_checkgui(l,hPanel);
		std::vector<WIHandle> children {};
		hPanel->FindDescendantsByName(name,children);
		auto t = Lua::CreateTable(l);
		auto idx = 1;
		for(auto &hChild : children)
		{
			if(hChild.IsValid() == false)
				continue;
			auto oChild = WGUILuaInterface::GetLuaObject(l,*hChild.get());
			Lua::PushInt(l,idx++);
			oChild.push(l);
			Lua::SetTableValue(l,t);
		}
	}));
	classDef.def("Update",static_cast<void(*)(lua_State*,WIHandle&)>([](lua_State *l,WIHandle &hPanel) {
		lua_checkgui(l,hPanel);
		hPanel->Update();
	}));
	classDef.def("ApplySkin",static_cast<void(*)(lua_State*,WIHandle&)>([](lua_State *l,WIHandle &hPanel) {
		lua_checkgui(l,hPanel);
		hPanel->ApplySkin();
	}));
	classDef.def("SetLeft",static_cast<void(*)(lua_State*,WIHandle&,int32_t)>([](lua_State *l,WIHandle &hPanel,int32_t pos) {
		lua_checkgui(l,hPanel);
		hPanel->SetLeft(pos);
	}));
	classDef.def("SetRight",static_cast<void(*)(lua_State*,WIHandle&,int32_t)>([](lua_State *l,WIHandle &hPanel,int32_t pos) {
		lua_checkgui(l,hPanel);
		hPanel->SetRight(pos);
	}));
	classDef.def("SetTop",static_cast<void(*)(lua_State*,WIHandle&,int32_t)>([](lua_State *l,WIHandle &hPanel,int32_t pos) {
		lua_checkgui(l,hPanel);
		hPanel->SetTop(pos);
	}));
	classDef.def("SetBottom",static_cast<void(*)(lua_State*,WIHandle&,int32_t)>([](lua_State *l,WIHandle &hPanel,int32_t pos) {
		lua_checkgui(l,hPanel);
		hPanel->SetBottom(pos);
	}));
	classDef.def("EnableThinking",static_cast<void(*)(lua_State*,WIHandle&)>([](lua_State *l,WIHandle &hPanel) {
		lua_checkgui(l,hPanel);
		hPanel->EnableThinking();
	}));
	classDef.def("DisableThinking",static_cast<void(*)(lua_State*,WIHandle&)>([](lua_State *l,WIHandle &hPanel) {
		lua_checkgui(l,hPanel);
		hPanel->DisableThinking();
	}));
	classDef.def("SetThinkingEnabled",static_cast<void(*)(lua_State*,WIHandle&,bool)>([](lua_State *l,WIHandle &hPanel,bool enabled) {
		lua_checkgui(l,hPanel);
		hPanel->SetThinkingEnabled(enabled);
	}));

	classDef.def("AddAttachment",static_cast<void(*)(lua_State*,WIHandle&,const std::string&)>(&AddAttachment));
	classDef.def("AddAttachment",static_cast<void(*)(lua_State*,WIHandle&,const std::string&,const Vector2&)>(&AddAttachment));
	classDef.def("SetAttachmentPos",&SetAttachmentPos);
	classDef.def("GetAttachmentPos",&GetAttachmentPos);
	classDef.def("GetAbsoluteAttachmentPos",&GetAbsoluteAttachmentPos);
	classDef.def("GetAttachmentPosProperty",&GetAttachmentPosProperty);
	classDef.def("SetAnchor",static_cast<void(*)(lua_State*,WIHandle&,float,float,float,float,uint32_t,uint32_t)>(&SetAnchor));
	classDef.def("SetAnchor",static_cast<void(*)(lua_State*,WIHandle&,float,float,float,float)>(&SetAnchor));
	classDef.def("SetAnchorLeft",&SetAnchorLeft);
	classDef.def("SetAnchorRight",&SetAnchorRight);
	classDef.def("SetAnchorTop",&SetAnchorTop);
	classDef.def("SetAnchorBottom",&SetAnchorBottom);
	classDef.def("ClearAnchor",static_cast<void(*)(lua_State*,WIHandle&)>([](lua_State *l,WIHandle &hPanel) {
		lua_checkgui(l,hPanel);
		hPanel->ClearAnchor();
	}));
	classDef.def("GetAnchor",&GetAnchor);
	classDef.def("HasAnchor",&HasAnchor);
	classDef.def("GetCenter",static_cast<void(*)(lua_State*,WIHandle&)>([](lua_State *l,WIHandle &hPanel) {
		lua_checkgui(l,hPanel);
		Lua::Push<Vector2>(l,hPanel->GetCenter());
	}));
	classDef.def("GetCenterX",static_cast<void(*)(lua_State*,WIHandle&)>([](lua_State *l,WIHandle &hPanel) {
		lua_checkgui(l,hPanel);
		Lua::PushNumber(l,hPanel->GetCenterX());
	}));
	classDef.def("GetCenterY",static_cast<void(*)(lua_State*,WIHandle&)>([](lua_State *l,WIHandle &hPanel) {
		lua_checkgui(l,hPanel);
		Lua::PushNumber(l,hPanel->GetCenterY());
	}));
	classDef.def("GetHalfWidth",static_cast<void(*)(lua_State*,WIHandle&)>([](lua_State *l,WIHandle &hPanel) {
		lua_checkgui(l,hPanel);
		Lua::PushNumber(l,hPanel->GetHalfWidth());
	}));
	classDef.def("GetHalfHeight",static_cast<void(*)(lua_State*,WIHandle&)>([](lua_State *l,WIHandle &hPanel) {
		lua_checkgui(l,hPanel);
		Lua::PushNumber(l,hPanel->GetHalfHeight());
	}));
	classDef.def("GetHalfSize",static_cast<void(*)(lua_State*,WIHandle&)>([](lua_State *l,WIHandle &hPanel) {
		lua_checkgui(l,hPanel);
		Lua::Push<Vector2>(l,hPanel->GetHalfSize());
	}));
	classDef.def("SetCenterPos",static_cast<void(*)(lua_State*,WIHandle&,const Vector2i&)>([](lua_State *l,WIHandle &hPanel,const Vector2i &pos) {
		lua_checkgui(l,hPanel);
		hPanel->SetCenterPos(pos);
	}));
	classDef.def("CenterToParent",static_cast<void(*)(lua_State*,WIHandle&)>([](lua_State *l,WIHandle &hPanel) {
		lua_checkgui(l,hPanel);
		hPanel->CenterToParent();
	}));
	classDef.def("CenterToParent",static_cast<void(*)(lua_State*,WIHandle&,bool)>([](lua_State *l,WIHandle &hPanel,bool applyAnchor) {
		lua_checkgui(l,hPanel);
		hPanel->CenterToParent(applyAnchor);
	}));
	classDef.def("CenterToParentX",static_cast<void(*)(lua_State*,WIHandle&)>([](lua_State *l,WIHandle &hPanel) {
		lua_checkgui(l,hPanel);
		hPanel->CenterToParentX();
	}));
	classDef.def("CenterToParentY",static_cast<void(*)(lua_State*,WIHandle&)>([](lua_State *l,WIHandle &hPanel) {
		lua_checkgui(l,hPanel);
		hPanel->CenterToParentY();
	}));
	classDef.def("RemoveStyleClass",static_cast<void(*)(lua_State*,WIHandle&,const std::string&)>([](lua_State *l,WIHandle &hPanel,const std::string &className) {
		lua_checkgui(l,hPanel);
		hPanel->RemoveStyleClass(className);
	}));
	classDef.def("ClearStyleClasses",static_cast<void(*)(lua_State*,WIHandle&)>([](lua_State *l,WIHandle &hPanel) {
		lua_checkgui(l,hPanel);
		hPanel->ClearStyleClasses();
	}));
	classDef.def("FindChildIndex",static_cast<void(*)(lua_State*,WIHandle&,WIHandle&)>([](lua_State *l,WIHandle &hPanel,WIHandle &hChild) {
		lua_checkgui(l,hPanel);
		lua_checkgui(l,hChild);
		auto index = hPanel->FindChildIndex(*hChild.get());
		if(index.has_value() == false)
			return;
		Lua::PushInt(l,*index);
	}));

	auto defDrawInfo = luabind::class_<::WIBase::DrawInfo>("DrawInfo");
	defDrawInfo.def(luabind::constructor<>());
	defDrawInfo.def_readwrite("offset",&::WIBase::DrawInfo::offset);
	defDrawInfo.def_readwrite("size",&::WIBase::DrawInfo::size);
	defDrawInfo.def_readwrite("transform",&::WIBase::DrawInfo::transform);
	defDrawInfo.def_readwrite("useScissor",&::WIBase::DrawInfo::useScissor);
	defDrawInfo.def("SetColor",static_cast<void(*)(lua_State*,::WIBase::DrawInfo&,const Color&)>([](lua_State *l,::WIBase::DrawInfo &drawInfo,const Color &color) {
		drawInfo.color = color.ToVector4();
	}));
	defDrawInfo.def("SetPostTransform",static_cast<void(*)(lua_State*,::WIBase::DrawInfo&,const Mat4&)>([](lua_State *l,::WIBase::DrawInfo &drawInfo,const Mat4 &t) {
		drawInfo.postTransform = t;
	}));
	classDef.scope[defDrawInfo];
}

void Lua::WIButton::register_class(luabind::class_<WIButtonHandle,WIHandle> &classDef)
{
	classDef.def("SetText",&SetText);
	classDef.def("GetText",&GetText);
}

void Lua::WIProgressBar::register_class(luabind::class_<WIProgressBarHandle,WIHandle> &classDef)
{
	classDef.def("SetProgress",&SetProgress);
	classDef.def("GetProgress",&GetProgress);
	classDef.def("SetValue",&SetValue);
	classDef.def("GetValue",&GetValue);
	classDef.def("SetRange",static_cast<void(*)(lua_State*,WIProgressBarHandle&,float,float,float)>(&SetRange));
	classDef.def("SetRange",static_cast<void(*)(lua_State*,WIProgressBarHandle&,float,float)>(&SetRange));
	classDef.def("SetOptions",&SetOptions);
	classDef.def("AddOption",&AddOption);
	classDef.def("SetPostFix",&SetPostFix);
	classDef.def("GetRange",static_cast<void(*)(lua_State*,WIProgressBarHandle&)>([](lua_State *l,WIProgressBarHandle &hProgressBar) {
		lua_checkgui(l,hProgressBar);
		auto range = static_cast<::WIProgressBar*>(hProgressBar.get())->GetRange();
		Lua::PushNumber(l,range.at(0));
		Lua::PushNumber(l,range.at(1));
		Lua::PushNumber(l,range.at(2));
	}));
	//classDef.def("SetValueTranslator",&SetValueTranslator);
}

void Lua::WISlider::register_class(luabind::class_<WISliderHandle,luabind::bases<WIProgressBarHandle,WIHandle>> &classDef)
{
	classDef.def("IsBeingDragged",static_cast<void(*)(lua_State*,WISliderHandle&)>([](lua_State *l,WISliderHandle &hSlider) {
		lua_checkgui(l,hSlider);
		Lua::PushBool(l,static_cast<::WISlider*>(hSlider.get())->IsBeingDragged());
	}));
}

void Lua::WIShape::register_class(luabind::class_<WIShapeHandle,WIHandle> &classDef)
{
	classDef.def("AddVertex",&AddVertex);
	classDef.def("SetVertexPos",&SetVertexPos);
	classDef.def("ClearVertices",&ClearVertices);
	classDef.def("InvertVertexPositions",static_cast<void(*)(lua_State*,WIShapeHandle&,bool,bool)>(&InvertVertexPositions));
	classDef.def("InvertVertexPositions",static_cast<void(*)(lua_State*,WIShapeHandle&)>(&InvertVertexPositions));
}

void Lua::WITexturedShape::register_class(luabind::class_<WITexturedShapeHandle,luabind::bases<WIShapeHandle,WIHandle>> &classDef)
{
	classDef.def("SetMaterial",static_cast<void(*)(lua_State*,WITexturedShapeHandle&,std::string)>(&SetMaterial));
	classDef.def("SetMaterial",static_cast<void(*)(lua_State*,WITexturedShapeHandle&,Material*)>(&SetMaterial));
	classDef.def("GetMaterial",&GetMaterial);
	classDef.def("SetTexture",&SetTexture);
	classDef.def("GetTexture",&GetTexture);
	classDef.def("AddVertex",static_cast<void(*)(lua_State*,WITexturedShapeHandle&,Vector2,Vector2)>(&AddVertex));
	classDef.def("AddVertex",static_cast<void(*)(lua_State*,WITexturedShapeHandle&,Vector2)>(&AddVertex));
	classDef.def("SetVertexUVCoord",&SetVertexUVCoord);
	classDef.def("InvertVertexUVCoordinates",static_cast<void(*)(lua_State*,WITexturedShapeHandle&,bool,bool)>(&InvertVertexUVCoordinates));
	classDef.def("InvertVertexUVCoordinates",static_cast<void(*)(lua_State*,WITexturedShapeHandle&)>(&InvertVertexUVCoordinates));
	classDef.def("ClearTexture",static_cast<void(*)(lua_State*,WITexturedShapeHandle&)>([](lua_State *l,WITexturedShapeHandle &hPanel) {
		lua_checkgui(l,hPanel);
		static_cast<::WITexturedShape*>(hPanel.get())->ClearTexture();
	}));
	classDef.def("SizeToTexture",static_cast<void(*)(lua_State*,WITexturedShapeHandle&)>([](lua_State *l,WITexturedShapeHandle &hPanel) {
		lua_checkgui(l,hPanel);
		static_cast<::WITexturedShape*>(hPanel.get())->SizeToTexture();
	}));
	classDef.def("SetChannelSwizzle",static_cast<void(*)(lua_State*,WITexturedShapeHandle&,uint32_t,uint32_t)>([](lua_State *l,WITexturedShapeHandle &hPanel,uint32_t src,uint32_t dst) {
		lua_checkgui(l,hPanel);
		static_cast<::WITexturedShape*>(hPanel.get())->SetChannelSwizzle(static_cast<::wgui::ShaderTextured::Channel>(dst),static_cast<::wgui::ShaderTextured::Channel>(src));
	}));
	classDef.def("GetChannelSwizzle",static_cast<void(*)(lua_State*,WITexturedShapeHandle&,uint32_t)>([](lua_State *l,WITexturedShapeHandle &hPanel,uint32_t channel) {
		lua_checkgui(l,hPanel);
		Lua::PushInt(l,umath::to_integral(static_cast<::WITexturedShape*>(hPanel.get())->GetChannelSwizzle(static_cast<::wgui::ShaderTextured::Channel>(channel))));
	}));
	classDef.def("SetShader",static_cast<void(*)(lua_State*,WITexturedShapeHandle&,wgui::ShaderTextured&)>([](lua_State *l,WITexturedShapeHandle &hPanel,wgui::ShaderTextured &shader) {
		lua_checkgui(l,hPanel);
		static_cast<::WITexturedShape*>(hPanel.get())->SetShader(shader);
	}));
	classDef.add_static_constant("CHANNEL_RED",umath::to_integral(::wgui::ShaderTextured::Channel::Red));
	classDef.add_static_constant("CHANNEL_GREEN",umath::to_integral(::wgui::ShaderTextured::Channel::Green));
	classDef.add_static_constant("CHANNEL_BLUE",umath::to_integral(::wgui::ShaderTextured::Channel::Blue));
	classDef.add_static_constant("CHANNEL_ALPHA",umath::to_integral(::wgui::ShaderTextured::Channel::Alpha));
}

void Lua::WIIcon::register_class(luabind::class_<WIIconHandle,luabind::bases<WITexturedShapeHandle,WIShapeHandle,WIHandle>> &classDef)
{
	classDef.def("SetClipping",&SetClipping);
}

void Lua::WISilkIcon::register_class(luabind::class_<WISilkIconHandle,luabind::bases<WIIconHandle,WITexturedShapeHandle,WIShapeHandle,WIHandle>> &classDef)
{
	classDef.def("SetIcon",&SetIcon);
}

void Lua::WIArrow::register_class(luabind::class_<WIArrowHandle,luabind::bases<WIShapeHandle,WIHandle>> &classDef)
{
	classDef.def("SetDirection",&SetDirection);
	classDef.add_static_constant("DIRECTION_RIGHT",0);
	classDef.add_static_constant("DIRECTION_DOWN",1);
	classDef.add_static_constant("DIRECTION_LEFT",2);
	classDef.add_static_constant("DIRECTION_UP",3);
}

void Lua::WICheckbox::register_class(luabind::class_<WICheckboxHandle,luabind::bases<WIShapeHandle,WIHandle>> &classDef)
{
	classDef.def("SetChecked",&SetChecked);
	classDef.def("IsChecked",&IsChecked);
	classDef.def("Toggle",&Toggle);
}

void Lua::WIGridPanel::register_class(luabind::class_<WIGridPanelHandle,luabind::bases<WITableHandle,WIHandle>> &classDef)
{
	classDef.def("AddItem",&AddItem);
	classDef.def("GetColumnCount",&GetColumnCount);
}

void Lua::WITreeList::register_class(luabind::class_<WITreeListHandle,luabind::bases<WITableHandle,WIHandle>> &classDef)
{
	classDef.def("AddItem",static_cast<void(*)(lua_State*,WITreeListHandle&,const std::string&)>([](lua_State *l,WITreeListHandle &hPanel,const std::string &text) {
		lua_checkgui(l,hPanel);
		auto *pEl = static_cast<::WITreeList*>(hPanel.get())->AddItem(text);
		if(pEl != nullptr)
		{
			auto o = WGUILuaInterface::GetLuaObject(l,*pEl);
			o.push(l);
		}
	}));
	classDef.def("AddItem",static_cast<void(*)(lua_State*,WITreeListHandle&,const std::string&,luabind::object)>([](lua_State *l,WITreeListHandle &hPanel,const std::string &text,luabind::object oPopulate) {
		lua_checkgui(l,hPanel);
		Lua::CheckFunction(l,3);
		auto fPopulate = [l,oPopulate](::WITreeListElement &el) {
			Lua::CallFunction(l,[&oPopulate,&el](lua_State *l) {
				oPopulate.push(l);
				auto o = WGUILuaInterface::GetLuaObject(l,el);
				o.push(l);
				return Lua::StatusCode::Ok;
			});
		};
		auto *pEl = static_cast<::WITreeList*>(hPanel.get())->AddItem(text,fPopulate);
		if(pEl != nullptr)
		{
			auto o = WGUILuaInterface::GetLuaObject(l,*pEl);
			o.push(l);
		}
	}));
	classDef.def("ExpandAll",&ExpandAll);
	classDef.def("CollapseAll",&CollapseAll);
	classDef.def("GetRootItem",&GetRootItem);
}
void Lua::WITreeListElement::register_class(luabind::class_<WITreeListElementHandle,luabind::bases<WITableRowHandle,WIHandle>> &classDef)
{
	classDef.def("Expand",static_cast<void(*)(lua_State*,WITreeListElementHandle&,bool)>(&Expand));
	classDef.def("Expand",static_cast<void(*)(lua_State*,WITreeListElementHandle&)>(&Expand));
	classDef.def("Collapse",static_cast<void(*)(lua_State*,WITreeListElementHandle&,bool)>(&Collapse));
	classDef.def("Collapse",static_cast<void(*)(lua_State*,WITreeListElementHandle&)>(&Collapse));
	classDef.def("GetItems",&GetItems);
	classDef.def("AddItem",static_cast<void(*)(lua_State*,WITreeListElementHandle&,const std::string&)>([](lua_State *l,WITreeListElementHandle &hPanel,const std::string &text) {
		lua_checkgui(l,hPanel);
		auto *pEl = static_cast<::WITreeListElement*>(hPanel.get())->AddItem(text);
		if(pEl != nullptr)
		{
			auto o = WGUILuaInterface::GetLuaObject(l,*pEl);
			o.push(l);
		}
	}));
	classDef.def("AddItem",static_cast<void(*)(lua_State*,WITreeListElementHandle&,const std::string&,luabind::object)>([](lua_State *l,WITreeListElementHandle &hPanel,const std::string &text,luabind::object oPopulate) {
		lua_checkgui(l,hPanel);
		Lua::CheckFunction(l,3);
		auto fPopulate = [l,oPopulate](::WITreeListElement &el) {
			Lua::CallFunction(l,[&oPopulate,&el](lua_State *l) {
				oPopulate.push(l);
				auto o = WGUILuaInterface::GetLuaObject(l,el);
				o.push(l);
				return Lua::StatusCode::Ok;
			});
		};
		auto *pEl = static_cast<::WITreeListElement*>(hPanel.get())->AddItem(text,fPopulate);
		if(pEl != nullptr)
		{
			auto o = WGUILuaInterface::GetLuaObject(l,*pEl);
			o.push(l);
		}
	}));
}

void Lua::WIContainer::register_class(luabind::class_<WIContainerHandle,WIHandle> &classDef)
{
	classDef.def("SetPadding",static_cast<void(*)(lua_State*,WIContainerHandle&,int32_t,int32_t,int32_t,int32_t)>(&SetPadding));
	classDef.def("SetPadding",static_cast<void(*)(lua_State*,WIContainerHandle&,int32_t)>(&SetPadding));
	classDef.def("SetPaddingTop",&SetPaddingTop);
	classDef.def("SetPaddingRight",&SetPaddingRight);
	classDef.def("SetPaddingBottom",&SetPaddingBottom);
	classDef.def("SetPaddingLeft",&SetPaddingLeft);
	classDef.def("GetPadding",&GetPadding);
	classDef.def("GetPaddingTop",&GetPaddingTop);
	classDef.def("GetPaddingRight",&GetPaddingRight);
	classDef.def("GetPaddingBottom",&GetPaddingBottom);
	classDef.def("GetPaddingLeft",&GetPaddingLeft);
}

void Lua::WITable::register_class(luabind::class_<WITableHandle,luabind::bases<WIContainerHandle,WIHandle>> &classDef)
{
	classDef.def("GetRowHeight",static_cast<void(*)(lua_State*,WITableHandle&)>([](lua_State *l,WITableHandle &hPanel) {
		lua_checkgui(l,hPanel);
		Lua::PushInt(l,static_cast<::WITable*>(hPanel.get())->GetRowHeight());
	}));
	classDef.def("SetRowHeight",&SetRowHeight);
	classDef.def("SetSelectableMode",&SetSelectable);
	classDef.def("GetSelectableMode",&IsSelectable);
	classDef.def("SetColumnWidth",&SetColumnWidth);
	classDef.def("AddRow",&AddRow);
	classDef.def("AddHeaderRow",&AddHeaderRow);
	classDef.def("GetRowCount",&GetRowCount);
	classDef.def("SetSortable",&SetSortable);
	classDef.def("IsSortable",&IsSortable);
	classDef.def("SetScrollable",&SetScrollable);
	classDef.def("IsScrollable",&IsScrollable);
	classDef.def("Clear",&Clear);
	classDef.def("GetRow",&GetRow);
	classDef.def("GetSelectedRows",&GetSelectedRows);
	classDef.def("GetRows",&GetRows);
	classDef.def("RemoveRow",&RemoveRow);
	classDef.def("MoveRow",static_cast<void(*)(lua_State*,WITableHandle&,WITableRowHandle&,WITableRowHandle&,bool)>([](lua_State *l,WITableHandle &hPanel,WITableRowHandle &rowA,WITableRowHandle &rowB,bool after) {
		lua_checkgui(l,hPanel);
		lua_checkgui(l,rowA);
		lua_checkgui(l,rowB);
		static_cast<::WITable*>(hPanel.get())->MoveRow(static_cast<::WITableRow*>(rowA.get()),static_cast<::WITableRow*>(rowB.get()),after);
	}));
	classDef.def("MoveRow",static_cast<void(*)(lua_State*,WITableHandle&,WITableRowHandle&,WITableRowHandle&)>([](lua_State *l,WITableHandle &hPanel,WITableRowHandle &rowA,WITableRowHandle &rowB) {
		lua_checkgui(l,hPanel);
		lua_checkgui(l,rowA);
		lua_checkgui(l,rowB);
		static_cast<::WITable*>(hPanel.get())->MoveRow(static_cast<::WITableRow*>(rowA.get()),static_cast<::WITableRow*>(rowB.get()));
	}));
	classDef.def("SelectRow",static_cast<void(*)(lua_State*,WITableHandle&,WITableRowHandle&)>([](lua_State *l,WITableHandle &hPanel,WITableRowHandle &row) {
		lua_checkgui(l,hPanel);
		lua_checkgui(l,row);
		static_cast<::WITable*>(hPanel.get())->SelectRow(*static_cast<::WITableRow*>(row.get()));
	}));
	classDef.def("GetFirstSelectedRow",static_cast<void(*)(lua_State*,WITableHandle&)>([](lua_State *l,WITableHandle &hPanel) {
		lua_checkgui(l,hPanel);
		auto hRow = static_cast<::WITable*>(hPanel.get())->GetFirstSelectedRow();
		if(hRow.IsValid() == false)
			return;
		auto o = WGUILuaInterface::GetLuaObject(l,*hRow.get());
		o.push(l);
	}));
	classDef.add_static_constant("SELECTABLE_MODE_NONE",umath::to_integral(::WITable::SelectableMode::None));
	classDef.add_static_constant("SELECTABLE_MODE_SINGLE",umath::to_integral(::WITable::SelectableMode::Single));
	classDef.add_static_constant("SELECTABLE_MODE_MULTI",umath::to_integral(::WITable::SelectableMode::Multi));
}

void Lua::WITableRow::register_class(luabind::class_<WITableRowHandle,luabind::bases<WIContainerHandle,WIHandle>> &classDef)
{
	classDef.def("Select",&Select);
	classDef.def("Deselect",&Deselect);
	classDef.def("IsSelected",&IsSelected);
	classDef.def("SetCellWidth",&SetCellWidth);
	classDef.def("SetValue",&SetValue);
	classDef.def("GetValue",&GetValue);
	classDef.def("InsertElement",&InsertElement);
	classDef.def("GetCellCount",&GetCellCount);
	classDef.def("GetCell",&GetCell);
	classDef.def("GetRowIndex",static_cast<void(*)(lua_State*,WITableRowHandle&)>([](lua_State *l,WITableRowHandle &hRow) {
		lua_checkgui(l,hRow);
		auto *pTable = static_cast<::WITableRow*>(hRow.get())->GetTable();
		uint32_t rowIndex = 0u;
		if(pTable)
			rowIndex = pTable->GetRowIndex(static_cast<::WITableRow*>(hRow.get()));
		Lua::PushInt(l,rowIndex);
	}));
}

void Lua::WITableCell::register_class(luabind::class_<WITableCellHandle,luabind::bases<WIContainerHandle,WIHandle>> &classDef)
{
	classDef.def("GetFirstElement",&GetFirstElement);
	classDef.def("GetColSpan",&GetColSpan);
	//classDef.def("GetRowSpan",&GetRowSpan);
	classDef.def("SetColSpan",&SetColSpan);
	//classDef.def("SetRowSpan",&SetRowSpan);
}

void Lua::WIFrame::register_class(luabind::class_<WIFrameHandle,luabind::bases<WITransformableHandle,WIHandle>> &classDef)
{
	classDef.def("SetTitle",&SetTitle);
	classDef.def("GetTitle",&GetTitle);
	classDef.def("SetCloseButtonEnabled",static_cast<void(*)(lua_State*,WIFrameHandle&,bool)>([](lua_State *l,WIFrameHandle &hPanel,bool enabled) {
		lua_checkgui(l,hPanel);
		static_cast<::WIFrame*>(hPanel.get())->SetCloseButtonEnabled(enabled);
	}));
}

void Lua::WIDropDownMenu::register_class(luabind::class_<WIDropDownMenuHandle,luabind::bases<WITextEntryHandle,WIHandle>> &classDef)
{
	classDef.def("SelectOption",static_cast<void(*)(lua_State*,WIDropDownMenuHandle&,unsigned int)>(&SelectOption));
	classDef.def("SelectOption",static_cast<void(*)(lua_State*,WIDropDownMenuHandle&,const std::string&)>(&SelectOption));
	classDef.def("ClearOptions",&ClearOptions);
	classDef.def("SelectOptionByText",&SelectOptionByText);
	classDef.def("GetOptionText",&GetOptionText);
	classDef.def("GetOptionValue",&GetOptionValue);
	classDef.def("SetOptionText",&SetOptionText);
	classDef.def("SetOptionValue",&SetOptionValue);
	classDef.def("GetValue",&GetValue);
	classDef.def("GetText",&GetText);
	classDef.def("SetText",SetText);
	classDef.def("GetOptionCount",GetOptionCount);
	classDef.def("AddOption",static_cast<void(*)(lua_State*,WIDropDownMenuHandle&,std::string,const std::string&)>(&AddOption));
	classDef.def("AddOption",static_cast<void(*)(lua_State*,WIDropDownMenuHandle&,std::string)>(&AddOption));
	classDef.def("OpenMenu",OpenMenu);
	classDef.def("CloseMenu",CloseMenu);
	classDef.def("ToggleMenu",ToggleMenu);
	classDef.def("IsMenuOpen",IsMenuOpen);
	classDef.def("GetSelectedOption",static_cast<void(*)(lua_State*,WIDropDownMenuHandle&)>([](lua_State *l,WIDropDownMenuHandle &hPanel) {
		lua_checkgui(l,hPanel);
		Lua::PushInt(l,static_cast<::WIDropDownMenu*>(hPanel.get())->GetSelectedOption());
	}));
	classDef.def("ClearSelectedOption",static_cast<void(*)(lua_State*,WIDropDownMenuHandle&)>([](lua_State *l,WIDropDownMenuHandle &hPanel) {
		lua_checkgui(l,hPanel);
		static_cast<::WIDropDownMenu*>(hPanel.get())->ClearSelectedOption();
	}));
}

void Lua::WIText::register_class(luabind::class_<WITextHandle,WIHandle> &classDef)
{
	classDef.def("SetText",&SetText);
	classDef.def("GetText",&GetText);
	classDef.def("SetFont",&SetFont);
	classDef.def("EnableShadow",&EnableShadow);
	classDef.def("IsShadowEnabled",&IsShadowEnabled);
	classDef.def("SetShadowColor",&SetShadowColor);
	classDef.def("SetShadowOffset",&SetShadowOffset);
	classDef.def("SetShadowXOffset",&SetShadowXOffset);
	classDef.def("SetShadowYOffset",&SetShadowYOffset);
	classDef.def("GetShadowColor",&GetShadowColor);
	classDef.def("GetShadowOffset",&GetShadowOffset);
	classDef.def("GetShadowXOffset",&GetShadowXOffset);
	classDef.def("GetShadowYOffset",&GetShadowYOffset);
	classDef.def("SetShadowAlpha",&SetShadowAlpha);
	classDef.def("GetShadowAlpha",&GetShadowAlpha);
	classDef.def("SetShadowBlurSize",&SetShadowBlurSize);
	classDef.def("GetShadowBlurSize",&GetShadowBlurSize);
	classDef.def("SetAutoBreakMode",&SetAutoBreakMode);
	classDef.def("GetLineCount",static_cast<void(*)(lua_State*,WITextHandle&)>([](lua_State *l,WITextHandle &hPanel) {
		lua_checkgui(l,hPanel);
		Lua::PushInt(l,static_cast<::WIText*>(hPanel.get())->GetLineCount());
	}));
	classDef.def("GetLine",static_cast<void(*)(lua_State*,WITextHandle&,int32_t)>([](lua_State *l,WITextHandle &hPanel,int32_t lineIndex) {
		lua_checkgui(l,hPanel);
		auto *pLine = static_cast<::WIText*>(hPanel.get())->GetLine(lineIndex);
		if(pLine == nullptr)
			return;
		Lua::PushString(l,pLine->GetUnformattedLine().GetText());
	}));
	classDef.def("GetTextLength",static_cast<void(*)(lua_State*,WITextHandle&)>([](lua_State *l,WITextHandle &hPanel) {
		lua_checkgui(l,hPanel);
		Lua::PushInt(l,static_cast<::WIText*>(hPanel.get())->GetText().length());
	}));
	classDef.def("SetTagArgument",static_cast<void(*)(lua_State*,WITextHandle&,const std::string&,uint32_t,luabind::object)>([](lua_State *l,WITextHandle &hPanel,const std::string &label,uint32_t argIdx,luabind::object o) {
		lua_checkgui(l,hPanel);
		if(Lua::IsString(l,4))
		{
			std::string arg = Lua::CheckString(l,4);
			static_cast<::WIText*>(hPanel.get())->SetTagArgument(label,argIdx,arg);
		}
		else if(Lua::IsType<Vector4>(l,4))
		{
			auto &arg = Lua::Check<Vector4>(l,4);
			static_cast<::WIText*>(hPanel.get())->SetTagArgument(label,argIdx,arg);
		}
		else if(Lua::IsType<Color>(l,4))
		{
			auto &arg = Lua::Check<Color>(l,4);
			static_cast<::WIText*>(hPanel.get())->SetTagArgument(label,argIdx,arg);
		}
		else if(Lua::IsFunction(l,4))
		{
			auto f = luabind::object(luabind::from_stack(l,4));
			auto arg = FunctionCallback<::util::EventReply>::CreateWithOptionalReturn([f,l](::util::EventReply *reply) -> CallbackReturnType {
				auto r = Lua::CallFunction(l,[&f](lua_State *l) {
					f.push(l);
					return Lua::StatusCode::Ok;
				},1);
				if(r == Lua::StatusCode::Ok)
				{
					if(Lua::IsSet(l,-1))
						*reply = static_cast<::util::EventReply>(Lua::CheckInt(l,-1));
					else
						*reply = ::util::EventReply::Unhandled;
					return (*reply != ::util::EventReply::Unhandled) ? CallbackReturnType::HasReturnValue : CallbackReturnType::NoReturnValue;
				}
				return CallbackReturnType::NoReturnValue;
			});
			static_cast<::WIText*>(hPanel.get())->SetTagArgument(label,argIdx,arg);
			Lua::Push<CallbackHandle>(l,arg);
		}
	}));
	classDef.def("SetTagsEnabled",static_cast<void(*)(lua_State*,WITextHandle&,bool)>([](lua_State *l,WITextHandle &hPanel,bool tagsEnabled) {
		lua_checkgui(l,hPanel);
		static_cast<::WIText*>(hPanel.get())->SetTagsEnabled(tagsEnabled);
	}));
	classDef.def("AreTagsEnabled",static_cast<void(*)(lua_State*,WITextHandle&)>([](lua_State *l,WITextHandle &hPanel) {
		lua_checkgui(l,hPanel);
		Lua::PushBool(l,static_cast<::WIText*>(hPanel.get())->AreTagsEnabled());
	}));
	classDef.def("PopFrontLine",static_cast<void(*)(lua_State*,WITextHandle&)>([](lua_State *l,WITextHandle &hPanel) {
		lua_checkgui(l,hPanel);
		static_cast<::WIText*>(hPanel.get())->PopFrontLine();
	}));
	classDef.def("PopBackLine",static_cast<void(*)(lua_State*,WITextHandle&)>([](lua_State *l,WITextHandle &hPanel) {
		lua_checkgui(l,hPanel);
		static_cast<::WIText*>(hPanel.get())->PopBackLine();
	}));
	classDef.def("RemoveText",static_cast<void(*)(lua_State*,WITextHandle&,uint32_t,uint32_t)>([](lua_State *l,WITextHandle &hPanel,uint32_t offset,uint32_t len) {
		lua_checkgui(l,hPanel);
		Lua::PushBool(l,static_cast<::WIText*>(hPanel.get())->RemoveText(offset,len));
	}));
	classDef.def("RemoveText",static_cast<void(*)(lua_State*,WITextHandle&,uint32_t,uint32_t,uint32_t)>([](lua_State *l,WITextHandle &hPanel,uint32_t lineIndex,uint32_t offset,uint32_t len) {
		lua_checkgui(l,hPanel);
		Lua::PushBool(l,static_cast<::WIText*>(hPanel.get())->RemoveText(lineIndex,offset,len));
	}));
	classDef.def("RemoveLine",static_cast<void(*)(lua_State*,WITextHandle&,uint32_t)>([](lua_State *l,WITextHandle &hPanel,uint32_t lineIndex) {
		lua_checkgui(l,hPanel);
		static_cast<::WIText*>(hPanel.get())->RemoveLine(lineIndex);
	}));
	classDef.def("InsertText",static_cast<void(*)(lua_State*,WITextHandle&,uint32_t,const std::string&,uint32_t,uint32_t)>([](lua_State *l,WITextHandle &hPanel,uint32_t lineIndex,const std::string &text,uint32_t lineIdx,uint32_t charOffset) {
		lua_checkgui(l,hPanel);
		Lua::PushBool(l,static_cast<::WIText*>(hPanel.get())->InsertText(text,lineIdx,charOffset));
	}));
	classDef.def("InsertText",static_cast<void(*)(lua_State*,WITextHandle&,uint32_t,const std::string&,uint32_t)>([](lua_State *l,WITextHandle &hPanel,uint32_t lineIndex,const std::string &text,uint32_t lineIdx) {
		lua_checkgui(l,hPanel);
		Lua::PushBool(l,static_cast<::WIText*>(hPanel.get())->InsertText(text,lineIdx));
	}));
	classDef.def("AppendText",static_cast<void(*)(lua_State*,WITextHandle&,const std::string&)>([](lua_State *l,WITextHandle &hPanel,const std::string &text) {
		lua_checkgui(l,hPanel);
		static_cast<::WIText*>(hPanel.get())->AppendText(text);
	}));
	classDef.def("AppendLine",static_cast<void(*)(lua_State*,WITextHandle&,const std::string&)>([](lua_State *l,WITextHandle &hPanel,const std::string &line) {
		lua_checkgui(l,hPanel);
		static_cast<::WIText*>(hPanel.get())->AppendLine(line);
	}));
	classDef.def("MoveText",static_cast<void(*)(lua_State*,WITextHandle&,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t)>(
		[](lua_State *l,WITextHandle &hPanel,uint32_t lineIdx,uint32_t startOffset,uint32_t len,uint32_t targetLineIdx,uint32_t targetCharOffset) {
		lua_checkgui(l,hPanel);
		Lua::PushBool(l,static_cast<::WIText*>(hPanel.get())->MoveText(lineIdx,startOffset,len,targetLineIdx,targetCharOffset));
	}));
	classDef.def("Clear",static_cast<void(*)(lua_State*,WITextHandle&)>([](lua_State *l,WITextHandle &hPanel) {
		lua_checkgui(l,hPanel);
		static_cast<::WIText*>(hPanel.get())->Clear();
	}));
	classDef.def("Substr",static_cast<void(*)(lua_State*,WITextHandle&,uint32_t,uint32_t)>([](lua_State *l,WITextHandle &hPanel,uint32_t startOffset,uint32_t len) {
		lua_checkgui(l,hPanel);
		Lua::PushString(l,static_cast<::WIText*>(hPanel.get())->Substr(startOffset,len));
	}));
	classDef.add_static_constant("AUTO_BREAK_NONE",umath::to_integral(::WIText::AutoBreak::NONE));
	classDef.add_static_constant("AUTO_BREAK_ANY",umath::to_integral(::WIText::AutoBreak::ANY));
	classDef.add_static_constant("AUTO_BREAK_WHITESPACE",umath::to_integral(::WIText::AutoBreak::WHITESPACE));
}

void Lua::WITextEntry::register_class(luabind::class_<WITextEntryHandle,WIHandle> &classDef)
{
	classDef.def("SetText",&SetText);
	classDef.def("GetText",&GetText);
	classDef.def("GetValue",&GetText);
	classDef.def("IsNumeric",&IsNumeric);
	classDef.def("IsEditable",&IsEditable);
	classDef.def("SetEditable",&SetEditable);
	classDef.def("SetMaxLength",&SetMaxLength);
	classDef.def("GetMaxLength",&GetMaxLength);
	classDef.def("IsMultiLine",&IsMultiLine);
	classDef.def("SetMultiLine",&SetMultiLine);

	classDef.def("IsSelectable",static_cast<void(*)(lua_State*,WITextEntryHandle&)>([](lua_State *l,WITextEntryHandle &hElement) {
		lua_checkgui(l,hElement);
		Lua::PushBool(l,static_cast<::WITextEntry*>(hElement.get())->IsSelectable());
	}));
	classDef.def("SetSelectable",static_cast<void(*)(lua_State*,WITextEntryHandle&,bool)>([](lua_State *l,WITextEntryHandle &hElement,bool bSelectable) {
		lua_checkgui(l,hElement);
		static_cast<::WITextEntry*>(hElement.get())->SetSelectable(bSelectable);
	}));
	classDef.def("GetTextElement",static_cast<void(*)(lua_State*,WITextEntryHandle&)>([](lua_State *l,WITextEntryHandle &hElement) {
		lua_checkgui(l,hElement);
		auto *pTextElement = static_cast<::WITextEntry*>(hElement.get())->GetTextElement();
		if(pTextElement == nullptr)
			return;
		auto o = WGUILuaInterface::GetLuaObject(l,*pTextElement);
		o.push(l);
	}));
}

////////////////////////////////////

void Lua::WINumericEntry::register_class(luabind::class_<WINumericEntryHandle,luabind::bases<WITextEntryHandle,WIHandle>> &classDef)
{
	classDef.def("SetMinValue",static_cast<void(*)(lua_State*,WINumericEntryHandle&,int32_t)>(&SetMinValue));
	classDef.def("SetMinValue",static_cast<void(*)(lua_State*,WINumericEntryHandle&)>(&SetMinValue));
	classDef.def("SetMaxValue",static_cast<void(*)(lua_State*,WINumericEntryHandle&,int32_t)>(&SetMaxValue));
	classDef.def("SetMaxValue",static_cast<void(*)(lua_State*,WINumericEntryHandle&)>(&SetMaxValue));
	classDef.def("SetRange",&SetRange);
	classDef.def("GetMinValue",&GetMinValue);
	classDef.def("GetMaxValue",&GetMaxValue);
}

////////////////////////////////////

void Lua::WIOutlinedRect::register_class(luabind::class_<WIOutlinedRectHandle,WIHandle> &classDef)
{
	classDef.def("SetOutlineWidth",&SetOutlineWidth);
	classDef.def("GetOutlineWidth",&GetOutlineWidth);
}

void Lua::WILine::register_class(luabind::class_<WILineHandle,WIHandle> &classDef)
{
	classDef.def("SetLineWidth",&SetLineWidth);
	classDef.def("GetLineWidth",&GetLineWidth);
	classDef.def("SetStartPos",&SetStartPos);
	classDef.def("GetStartPos",&GetStartPos);
	classDef.def("SetEndPos",&SetEndPos);
	classDef.def("GetEndPos",&GetEndPos);
	classDef.def("SetStartColor",&SetStartColor);
	classDef.def("SetEndColor",&SetEndColor);
	classDef.def("GetStartColor",&GetStartColor);
	classDef.def("GetEndColor",&GetEndColor);
	classDef.def("GetStartPosProperty",&GetStartPosProperty);
	classDef.def("GetEndPosProperty",&GetEndPosProperty);
}

void Lua::WIRoundedRect::register_class(luabind::class_<WIRoundedRectHandle,luabind::bases<WIShapeHandle,WIHandle>> &classDef)
{
	classDef.def("GetRoundness",&GetRoundness);
	classDef.def("SetRoundness",&SetRoundness);
	classDef.def("SetCornerSize",&SetCornerSize);
	classDef.def("GetCornerSize",&GetCornerSize);
	classDef.def("SetRoundTopRight",&SetRoundTopRight);
	classDef.def("SetRoundTopLeft",&SetRoundTopLeft);
	classDef.def("SetRoundBottomLeft",&SetRoundBottomLeft);
	classDef.def("SetRoundBottomRight",&SetRoundBottomRight);
	classDef.def("IsTopRightRound",&IsTopRightRound);
	classDef.def("IsTopLeftRound",&IsTopLeftRound);
	classDef.def("IsBottomLeftRound",&IsBottomLeftRound);
	classDef.def("IsBottomRightRound",&IsBottomRightRound);
}

void Lua::WIRoundedTexturedRect::register_class(luabind::class_<WIRoundedTexturedRectHandle,luabind::bases<WITexturedShapeHandle,WIShapeHandle,WIHandle>> &classDef)
{
	classDef.def("GetRoundness",&GetRoundness);
	classDef.def("SetRoundness",&SetRoundness);
	classDef.def("SetCornerSize",&SetCornerSize);
	classDef.def("GetCornerSize",&GetCornerSize);
	classDef.def("SetRoundTopRight",&SetRoundTopRight);
	classDef.def("SetRoundTopLeft",&SetRoundTopLeft);
	classDef.def("SetRoundBottomLeft",&SetRoundBottomLeft);
	classDef.def("SetRoundBottomRight",&SetRoundBottomRight);
	classDef.def("IsTopRightRound",&IsTopRightRound);
	classDef.def("IsTopLeftRound",&IsTopLeftRound);
	classDef.def("IsBottomLeftRound",&IsBottomLeftRound);
	classDef.def("IsBottomRightRound",&IsBottomRightRound);
}

void Lua::WIScrollBar::register_class(luabind::class_<WIScrollBarHandle,WIHandle> &classDef)
{
	classDef.def("GetScrollAmount",&GetScrollAmount);
	classDef.def("SetScrollAmount",&SetScrollAmount);
	classDef.def("GetScrollOffset",&GetScrollOffset);
	classDef.def("SetScrollOffset",&SetScrollOffset);
	classDef.def("AddScrollOffset",&AddScrollOffset);
	classDef.def("SetUp",&SetUp);
	classDef.def("SetHorizontal",&SetHorizontal);
	classDef.def("IsHorizontal",&IsHorizontal);
	classDef.def("IsVertical",&IsVertical);
}

void Lua::WIBase::IsValid(lua_State *l,WIHandle &hPanel)
{
	lua_pushboolean(l,(hPanel != nullptr && hPanel.IsValid()) ? true : false);
}

void Lua::WIBase::Remove(lua_State *l,WIHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	hPanel->Remove();
}

void Lua::WIBase::RemoveSafely(lua_State *l,WIHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	hPanel->RemoveSafely();
}

void Lua::WIBase::SetZPos(lua_State *l,WIHandle &hPanel,int zpos)
{
	lua_checkgui(l,hPanel);
	hPanel->SetZPos(zpos);
}

void Lua::WIBase::GetZPos(lua_State *l,WIHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	lua_pushinteger(l,hPanel->GetZPos());
}
void Lua::WIBase::HasFocus(lua_State *l,WIHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	lua_pushboolean(l,hPanel->HasFocus());
}
void Lua::WIBase::RequestFocus(lua_State *l,WIHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	hPanel->RequestFocus();
}
void Lua::WIBase::KillFocus(lua_State *l,WIHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	hPanel->KillFocus();
}
void Lua::WIBase::TrapFocus(lua_State *l,WIHandle &hPanel,bool bTrap)
{
	lua_checkgui(l,hPanel);
	hPanel->TrapFocus(bTrap);
}
void Lua::WIBase::TrapFocus(lua_State *l,WIHandle &hPanel) {TrapFocus(l,hPanel,true);}
void Lua::WIBase::IsFocusTrapped(lua_State *l,WIHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Lua::PushBool(l,hPanel->IsFocusTrapped());
}
void Lua::WIBase::IsVisible(lua_State *l,WIHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	lua_pushboolean(l,hPanel->IsVisible());
}
void Lua::WIBase::SetVisible(lua_State *l,WIHandle &hPanel,bool b)
{
	lua_checkgui(l,hPanel);
	hPanel->SetVisible(b);
}
void Lua::WIBase::GetMouseInputEnabled(lua_State *l,WIHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	lua_pushboolean(l,hPanel->GetMouseInputEnabled());
}
void Lua::WIBase::SetMouseInputEnabled(lua_State *l,WIHandle &hPanel,bool b)
{
	lua_checkgui(l,hPanel);
	hPanel->SetMouseInputEnabled(b);
}
void Lua::WIBase::GetKeyboardInputEnabled(lua_State *l,WIHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	lua_pushboolean(l,hPanel->GetKeyboardInputEnabled());
}
void Lua::WIBase::SetKeyboardInputEnabled(lua_State *l,WIHandle &hPanel,bool b)
{
	lua_checkgui(l,hPanel);
	hPanel->SetKeyboardInputEnabled(b);
}
void Lua::WIBase::GetScrollInputEnabled(lua_State *l,WIHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	lua_pushboolean(l,hPanel->GetScrollInputEnabled());
}
void Lua::WIBase::SetScrollInputEnabled(lua_State *l,WIHandle &hPanel,bool b)
{
	lua_checkgui(l,hPanel);
	hPanel->SetScrollInputEnabled(b);
}
void Lua::WIBase::GetMouseMovementCheckEnabled(lua_State *l,WIHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	lua_pushboolean(l,hPanel->GetMouseMovementCheckEnabled());
}
void Lua::WIBase::SetMouseMovementCheckEnabled(lua_State *l,WIHandle &hPanel,bool b)
{
	lua_checkgui(l,hPanel);
	hPanel->SetMouseMovementCheckEnabled(b);
}
void Lua::WIBase::GetPos(lua_State *l,WIHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Vector2i pos = hPanel->GetPos();
	luabind::object(l,Vector2(pos.x,pos.y)).push(l);
}
void Lua::WIBase::SetPos(lua_State *l,WIHandle &hPanel,Vector2 pos)
{
	lua_checkgui(l,hPanel);
	hPanel->SetPos(CInt32(pos.x),CInt32(pos.y));
}
void Lua::WIBase::SetPos(lua_State *l,WIHandle &hPanel,float x,float y)
{
	lua_checkgui(l,hPanel);
	hPanel->SetPos(CInt32(x),CInt32(y));
}
void Lua::WIBase::GetAbsolutePos(lua_State *l,WIHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Vector2i pos = hPanel->GetAbsolutePos();
	luabind::object(l,Vector2(pos.x,pos.y)).push(l);
}
void Lua::WIBase::SetAbsolutePos(lua_State *l,WIHandle &hPanel,Vector2 pos)
{
	lua_checkgui(l,hPanel);
	hPanel->SetAbsolutePos(Vector2i(pos.x,pos.y));
}
void Lua::WIBase::SetAbsolutePos(lua_State *l,WIHandle &hPanel,float x,float y)
{
	lua_checkgui(l,hPanel);
	hPanel->SetAbsolutePos(Vector2i(x,y));
}
void Lua::WIBase::GetColor(lua_State *l,WIHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	auto &col = hPanel->GetColor();
	Lua::Push<Color>(l,col);
}
void Lua::WIBase::GetColorProperty(lua_State *l,WIHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Lua::Property::push(l,*hPanel->GetColorProperty());
}
void Lua::WIBase::SetColor(lua_State *l,WIHandle &hPanel,Color col)
{
	lua_checkgui(l,hPanel);
	hPanel->SetColor(col.r /255.f,col.g /255.f,col.b /255.f,col.a /255.f);
}
void Lua::WIBase::GetAlpha(lua_State *l,WIHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Lua::PushNumber(l,hPanel->GetAlpha() *255);
}
void Lua::WIBase::SetAlpha(lua_State *l,WIHandle &hPanel,float alpha)
{
	lua_checkgui(l,hPanel);
	hPanel->SetAlpha(alpha /255.f);
}
void Lua::WIBase::GetWidth(lua_State *l,WIHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Lua::PushInt(l,hPanel->GetWidth());
}
void Lua::WIBase::GetHeight(lua_State *l,WIHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Lua::PushInt(l,hPanel->GetHeight());
}
void Lua::WIBase::GetSize(lua_State *l,WIHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Vector2i size = hPanel->GetSize();
	luabind::object(l,Vector2(size.x,size.y)).push(l);
}
void Lua::WIBase::SetSize(lua_State *l,WIHandle &hPanel,Vector2 size)
{
	lua_checkgui(l,hPanel);
	hPanel->SetSize(CInt32(size.x),CInt32(size.y));
}
void Lua::WIBase::SetSize(lua_State *l,WIHandle &hPanel,float x,float y)
{
	lua_checkgui(l,hPanel);
	hPanel->SetSize(CInt32(x),CInt32(y));
}
void Lua::WIBase::Wrap(lua_State *l,WIHandle &hPanel,const std::string &wrapperClassName)
{
	lua_checkgui(l,hPanel);
	auto *el = c_game->CreateGUIElement(wrapperClassName);
	if(el == nullptr)
		return;
	auto hWrapper = el->GetHandle();
	if(hPanel->Wrap(*hWrapper.get()) == false)
	{
		el->RemoveSafely();
		return;
	}
	auto o = WGUILuaInterface::GetLuaObject(l,*el);
	o.push(l);
}
void Lua::WIBase::Wrap(lua_State *l,WIHandle &hPanel,WIHandle &hWrapper)
{
	lua_checkgui(l,hPanel);
	lua_checkgui(l,hWrapper);
	Lua::PushBool(l,hPanel->Wrap(*hWrapper.get()));
}
void Lua::WIBase::GetParent(lua_State *l,WIHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	auto *parent = hPanel->GetParent();
	if(parent == NULL)
		return;
	auto o = WGUILuaInterface::GetLuaObject(l,*parent);
	o.push(l);
}
void Lua::WIBase::SetParent(lua_State *l,WIHandle &hPanel,WIHandle &hParent)
{
	lua_checkgui(l,hPanel);
	lua_checkgui(l,hParent);
	hPanel->SetParent(hParent.get());
}
void Lua::WIBase::ClearParent(lua_State *l,WIHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	hPanel->ClearParent();
}
void Lua::WIBase::GetChildren(lua_State *l,WIHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	std::vector<WIHandle> *children = hPanel->GetChildren();
	int table = Lua::CreateTable(l);
	unsigned int c = 1;
	for(unsigned int i=0;i<children->size();i++)
	{
		WIHandle &hChild = (*children)[i];
		if(hChild.IsValid())
		{
			auto *pChild = hChild.get();
			auto oChild = WGUILuaInterface::GetLuaObject(l,*pChild);
			Lua::PushInt(l,c);
			oChild.push(l);
			Lua::SetTableValue(l,table);
			c++;
		}
	}
}
void Lua::WIBase::GetChildren(lua_State *l,WIHandle &hPanel,std::string className)
{
	lua_checkgui(l,hPanel);
	std::vector<WIHandle> *children = hPanel->GetChildren();
	int table = Lua::CreateTable(l);
	unsigned int c = 1;
	for(unsigned int i=0;i<children->size();i++)
	{
		WIHandle &hChild = (*children)[i];
		if(hChild.IsValid())
		{
			auto *pChild = hChild.get();
			if(pChild->GetClass() == className)
			{
				auto oChild = WGUILuaInterface::GetLuaObject(l,*pChild);
				Lua::PushInt(l,c);
				oChild.push(l);
				Lua::SetTableValue(l,table);
				c++;
			}
		}
	}
}
void Lua::WIBase::GetFirstChild(lua_State *l,WIHandle &hPanel,std::string className)
{
	lua_checkgui(l,hPanel);
	auto *el = hPanel->GetFirstChild(className);
	if(el == NULL)
		return;
	auto oChild = WGUILuaInterface::GetLuaObject(l,*el);
	oChild.push(l);
}
void Lua::WIBase::GetChild(lua_State *l,WIHandle &hPanel,unsigned int idx)
{
	lua_checkgui(l,hPanel);
	auto *el = hPanel->GetChild(idx);
	if(el == NULL)
		return;
	auto oChild = WGUILuaInterface::GetLuaObject(l,*el);
	oChild.push(l);
}
void Lua::WIBase::GetChild(lua_State *l,WIHandle &hPanel,std::string className,unsigned int idx)
{
	lua_checkgui(l,hPanel);
	auto *el = hPanel->GetChild(className,idx);
	if(el == NULL)
		return;
	auto oChild = WGUILuaInterface::GetLuaObject(l,*el);
	oChild.push(l);
}
void Lua::WIBase::PosInBounds(lua_State *l,WIHandle &hPanel,Vector2 pos)
{
	lua_checkgui(l,hPanel);
	lua_pushboolean(l,hPanel->PosInBounds(CInt32(pos.x),CInt32(pos.y)));
}
void Lua::WIBase::MouseInBounds(lua_State *l,WIHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	lua_pushboolean(l,hPanel->MouseInBounds());
}
void Lua::WIBase::GetMousePos(lua_State *l,WIHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	int x,y;
	hPanel->GetMousePos(&x,&y);
	luabind::object(l,Vector2(x,y)).push(l);
}
void Lua::WIBase::Draw(lua_State *l,WIHandle &hPanel,const ::WIBase::DrawInfo &drawInfo)
{
	lua_checkgui(l,hPanel);
	hPanel->Draw(drawInfo);
}
void Lua::WIBase::Draw(lua_State *l,WIHandle &hPanel,const ::WIBase::DrawInfo &drawInfo,const Vector2i &scissorOffset,const Vector2i &scissorSize)
{
	lua_checkgui(l,hPanel);
	hPanel->Draw(drawInfo,Vector2i{},scissorOffset,scissorSize);
}
void Lua::WIBase::Draw(lua_State *l,WIHandle &hPanel,const ::WIBase::DrawInfo &drawInfo,const Vector2i &scissorOffset,const Vector2i &scissorSize,const Vector2i &offsetParent)
{
	lua_checkgui(l,hPanel);
	hPanel->Draw(drawInfo,offsetParent,scissorOffset,scissorSize);
}
void Lua::WIBase::GetX(lua_State *l,WIHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Vector2i pos = hPanel->GetPos();
	Lua::PushInt(l,pos.x);
}
void Lua::WIBase::GetY(lua_State *l,WIHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Vector2i pos = hPanel->GetPos();
	Lua::PushInt(l,pos.y);
}
void Lua::WIBase::SetX(lua_State *l,WIHandle &hPanel,float x)
{
	lua_checkgui(l,hPanel);
	Vector2i pos = hPanel->GetPos();
	hPanel->SetPos(Vector2i(x,pos.y));
}
void Lua::WIBase::SetY(lua_State *l,WIHandle &hPanel,float y)
{
	lua_checkgui(l,hPanel);
	Vector2i pos = hPanel->GetPos();
	hPanel->SetPos(Vector2i(pos.x,y));
}
void Lua::WIBase::SetWidth(lua_State *l,WIHandle &hPanel,float w)
{
	lua_checkgui(l,hPanel);
	Vector2i size = hPanel->GetSize();
	hPanel->SetSize(Vector2i(w,size.y));
}
void Lua::WIBase::SetHeight(lua_State *l,WIHandle &hPanel,float h)
{
	lua_checkgui(l,hPanel);
	Vector2i size = hPanel->GetSize();
	hPanel->SetSize(Vector2i(size.x,h));
}
void Lua::WIBase::SizeToContents(lua_State *l,WIHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	hPanel->SizeToContents();
}

struct LuaCallbacks
{
	struct CallbackInfo
	{
		CallbackInfo(lua_State *l,const CallbackHandle &_hCallback,const luabind::object &f)
			: luaState(l),hCallback(_hCallback),luaFunction(f)
		{}
		CallbackHandle hCallback;
		luabind::object luaFunction;
		lua_State *luaState;
	};
	~LuaCallbacks()
	{
		for(auto &pair : callbacks)
		{
			for(auto &cbInfo : pair.second)
			{
				if(cbInfo.hCallback.IsValid() == false)
					continue;
				cbInfo.hCallback.Remove();
			}
		}
	}
	std::unordered_map<std::string,std::vector<CallbackInfo>> callbacks;
};

void Lua::gui::clear_lua_callbacks(lua_State *l)
{
	auto &wgui = WGUI::GetInstance();
	std::function<void(::WIBase&)> fIterateElements = nullptr;
	fIterateElements = [&fIterateElements,l](::WIBase &el) {
		auto callbackPtr = std::static_pointer_cast<LuaCallbacks>(el.GetUserData4()); // User data 4 contains lua callbacks
		if(callbackPtr != nullptr)
		{
			for(auto &pair : callbackPtr->callbacks)
			{
				for(auto it=pair.second.begin();it!=pair.second.end();)
				{
					auto &cbInfo = *it;
					if(cbInfo.luaState != l)
					{
						++it;
						continue;
					}
					if(cbInfo.hCallback.IsValid())
						cbInfo.hCallback.Remove();
					it = pair.second.erase(it);
				}
			}
		}
		for(auto &hChild : *el.GetChildren())
		{
			if(hChild.IsValid() == false)
				continue;
			fIterateElements(*hChild.get());
		}
	};
	auto *elBase = wgui.GetBaseElement();
	if(elBase != nullptr)
		fIterateElements(*elBase);
}

namespace Lua
{
	namespace WIBase
	{
		template<typename... TARGS>
			void CallCallbacks(lua_State *l,WIHandle &hPanel,std::string name,TARGS ...args)
		{
			lua_checkgui(l,hPanel);
			auto callbackPtr = std::static_pointer_cast<LuaCallbacks>(hPanel->GetUserData4());
			if(callbackPtr == nullptr)
				return;
			ustring::to_lower(name);
			auto itCallbacks = callbackPtr->callbacks.find(name);
			if(itCallbacks == callbackPtr->callbacks.end())
				return;
			uint32_t argOffset = 3;
			auto numArgs = Lua::GetStackTop(l) -argOffset +1;
			for(auto it=itCallbacks->second.begin();it!=itCallbacks->second.end();)
			{
				auto &cbInfo = *it;
				if(!cbInfo.hCallback.IsValid())
					it = itCallbacks->second.erase(it);
				else if(cbInfo.luaState == l)
				{
					auto &o = cbInfo.luaFunction;
					auto bReturn = false;
					Lua::Execute(l,[l,&o,&hPanel,numArgs,argOffset,&bReturn,&name](int(*traceback)(lua_State *l)) {
						auto n = Lua::GetStackTop(l);
						auto r = Lua::CallFunction(l,[&o,&hPanel,numArgs,argOffset](lua_State *l) {
							o.push(l);
							auto obj = WGUILuaInterface::GetLuaObject(l,*hPanel.get());
							obj.push(l);
							for(auto i=decltype(numArgs){0};i<numArgs;++i)
							{
								auto arg = argOffset +i;
								Lua::PushValue(l,arg);
							}
							return Lua::StatusCode::Ok;
						},LUA_MULTRET);
						if(r == Lua::StatusCode::Ok)
						{
							auto numResults = Lua::GetStackTop(l) -n;
							if(numResults > 0)
								bReturn = true;
						}
						return r;
					},Lua::GetErrorColorMode(l));
					if(bReturn == true)
						break;
					++it;
				}
			}
			if(itCallbacks->second.empty())
				callbackPtr->callbacks.erase(itCallbacks);
		}
	};
};
void Lua::WIBase::CallCallbacks(lua_State *l,WIHandle &hPanel,std::string name)
{
	CallCallbacks<>(l,hPanel,name);
}
void Lua::WIBase::CallCallbacks(lua_State *l,WIHandle &hPanel,std::string name,luabind::object o1)
{
	CallCallbacks<luabind::object>(l,hPanel,name,o1);
}
void Lua::WIBase::CallCallbacks(lua_State *l,WIHandle &hPanel,std::string name,luabind::object o1,luabind::object o2)
{
	CallCallbacks<luabind::object,luabind::object>(l,hPanel,name,o1,o2);
}
void Lua::WIBase::CallCallbacks(lua_State *l,WIHandle &hPanel,std::string name,luabind::object o1,luabind::object o2,luabind::object o3)
{
	CallCallbacks<luabind::object,luabind::object,luabind::object>(l,hPanel,name,o1,o2,o3);
}
void Lua::WIBase::CallCallbacks(lua_State *l,WIHandle &hPanel,std::string name,luabind::object o1,luabind::object o2,luabind::object o3,luabind::object o4)
{
	CallCallbacks<luabind::object,luabind::object,luabind::object,luabind::object>(l,hPanel,name,o1,o2,o3,o4);
}
void Lua::WIBase::CallCallbacks(lua_State *l,WIHandle &hPanel,std::string name,luabind::object o1,luabind::object o2,luabind::object o3,luabind::object o4,luabind::object o5)
{
	CallCallbacks<luabind::object,luabind::object,luabind::object,luabind::object,luabind::object>(l,hPanel,name,o1,o2,o3,o4,o5);
}
void Lua::WIBase::CallCallbacks(lua_State *l,WIHandle &hPanel,std::string name,luabind::object o1,luabind::object o2,luabind::object o3,luabind::object o4,luabind::object o5,luabind::object o6)
{
	CallCallbacks<luabind::object,luabind::object,luabind::object,luabind::object,luabind::object,luabind::object>(l,hPanel,name,o1,o2,o3,o4,o5,o6);
}
void Lua::WIBase::CallCallbacks(lua_State *l,WIHandle &hPanel,std::string name,luabind::object o1,luabind::object o2,luabind::object o3,luabind::object o4,luabind::object o5,luabind::object o6,luabind::object o7)
{
	CallCallbacks<luabind::object,luabind::object,luabind::object,luabind::object,luabind::object,luabind::object,luabind::object>(l,hPanel,name,o1,o2,o3,o4,o5,o6,o7);
}
void Lua::WIBase::CallCallbacks(lua_State *l,WIHandle &hPanel,std::string name,luabind::object o1,luabind::object o2,luabind::object o3,luabind::object o4,luabind::object o5,luabind::object o6,luabind::object o7,luabind::object o8)
{
	CallCallbacks<luabind::object,luabind::object,luabind::object,luabind::object,luabind::object,luabind::object,luabind::object,luabind::object>(l,hPanel,name,o1,o2,o3,o4,o5,o6,o7,o8);
}
void Lua::WIBase::AddCallback(lua_State *l,WIHandle &hPanel,std::string name,luabind::object o)
{
	lua_checkgui(l,hPanel);
	Lua::CheckFunction(l,3);

	CallbackHandle hCallback{};
	ustring::to_lower(name);

	auto callbackPtr = std::static_pointer_cast<LuaCallbacks>(hPanel->GetUserData4());
	if(callbackPtr == nullptr)
	{
		callbackPtr = std::make_shared<LuaCallbacks>();
		hPanel->SetUserData4(callbackPtr);
	}
	if(name == "ontextchanged")
	{
		hCallback = FunctionCallback<void,std::reference_wrapper<const std::string>,bool>::Create([l,hPanel,o](std::reference_wrapper<const std::string> text,bool changedByUser) {
			if(!hPanel.IsValid())
				return;
			Lua::CallFunction(l,[&o,hPanel,text,changedByUser](lua_State *l) {
				o.push(l);

				auto obj = WGUILuaInterface::GetLuaObject(l,*hPanel.get());
				obj.push(l);
				Lua::PushString(l,text.get());
				Lua::PushBool(l,changedByUser);
				return Lua::StatusCode::Ok;
			},0);
		});
	}
	if(name == "onscrolloffsetchanged")
	{
		hCallback = FunctionCallback<void,uint32_t>::Create([l,hPanel,o](uint32_t offset) {
			if(!hPanel.IsValid())
				return;
			Lua::CallFunction(l,[&o,hPanel,offset](lua_State *l) {
				o.push(l);

				auto obj = WGUILuaInterface::GetLuaObject(l,*hPanel.get());
				obj.push(l);
				Lua::PushInt(l,offset);
				return Lua::StatusCode::Ok;
			},0);
		});
	}
	else if(name == "oncharevent")
	{
		hCallback = FunctionCallback<::util::EventReply,int,GLFW::Modifier>::CreateWithOptionalReturn(
			[l,hPanel,o](::util::EventReply *reply,int c,GLFW::Modifier mods) -> CallbackReturnType {
			if(!hPanel.IsValid())
				return CallbackReturnType::NoReturnValue;
			if(Lua::CallFunction(l,[&o,hPanel,c,mods](lua_State *l) {
				o.push(l);

				auto obj = WGUILuaInterface::GetLuaObject(l,*hPanel.get());
				obj.push(l);
				Lua::PushString(l,std::string(1,static_cast<char>(c)));
				Lua::PushInt(l,umath::to_integral(mods));
				return Lua::StatusCode::Ok;
			},1) == Lua::StatusCode::Ok)
			{
				if(Lua::IsSet(l,-1) == false)
					return CallbackReturnType::NoReturnValue;
				auto result = static_cast<::util::EventReply>(Lua::CheckInt(l,-1));
				Lua::Pop(l,1);
				*reply = result;
				return (*reply != ::util::EventReply::Unhandled) ? CallbackReturnType::HasReturnValue : CallbackReturnType::NoReturnValue;
			}
			return CallbackReturnType::NoReturnValue;
		});
	}
	else if(name == "onkeyevent")
	{
		hCallback = FunctionCallback<::util::EventReply,GLFW::Key,int,GLFW::KeyState,GLFW::Modifier>::CreateWithOptionalReturn(
			[l,hPanel,o](::util::EventReply *reply,GLFW::Key key,int,GLFW::KeyState action,GLFW::Modifier mods) -> CallbackReturnType {
			if(!hPanel.IsValid())
				return CallbackReturnType::NoReturnValue;
			if(Lua::CallFunction(l,[&o,hPanel,key,action,mods](lua_State *l) {
				o.push(l);

				auto obj = WGUILuaInterface::GetLuaObject(l,*hPanel.get());
				obj.push(l);
				Lua::PushInt(l,umath::to_integral(key));
				Lua::PushInt(l,umath::to_integral(action));
				Lua::PushInt(l,umath::to_integral(mods));
				return Lua::StatusCode::Ok;
			},1) == Lua::StatusCode::Ok)
			{
				if(Lua::IsSet(l,-1) == false)
					return CallbackReturnType::NoReturnValue;
				auto result = static_cast<::util::EventReply>(Lua::CheckInt(l,-1));
				Lua::Pop(l,1);
				*reply = result;
				return (*reply != ::util::EventReply::Unhandled) ? CallbackReturnType::HasReturnValue : CallbackReturnType::NoReturnValue;
			}
			return CallbackReturnType::NoReturnValue;
		});
	}
	else if(name == "onmouseevent")
	{
		hCallback = FunctionCallback<::util::EventReply,GLFW::MouseButton,GLFW::KeyState,GLFW::Modifier>::CreateWithOptionalReturn(
			[l,hPanel,o](::util::EventReply *reply,GLFW::MouseButton button,GLFW::KeyState action,GLFW::Modifier mods) -> CallbackReturnType {
			if(!hPanel.IsValid())
				return CallbackReturnType::NoReturnValue;
			if(Lua::CallFunction(l,[&o,hPanel,button,action,mods](lua_State *l) {
				o.push(l);

				auto obj = WGUILuaInterface::GetLuaObject(l,*hPanel.get());
				obj.push(l);
				Lua::PushInt(l,umath::to_integral(button));
				Lua::PushInt(l,umath::to_integral(action));
				Lua::PushInt(l,umath::to_integral(mods));
				return Lua::StatusCode::Ok;
			},1) == Lua::StatusCode::Ok)
			{
				if(Lua::IsSet(l,-1) == false)
					return CallbackReturnType::NoReturnValue;
				auto result = static_cast<::util::EventReply>(Lua::CheckInt(l,-1));
				Lua::Pop(l,1);
				*reply = result;
				return (*reply != ::util::EventReply::Unhandled) ? CallbackReturnType::HasReturnValue : CallbackReturnType::NoReturnValue;
			}
			return CallbackReturnType::NoReturnValue;
		});
	}
	else if(name == "oncursormoved")
	{
		hCallback = FunctionCallback<void,int32_t,int32_t>::Create(
			[l,hPanel,o](int32_t x,int32_t y) {
			if(!hPanel.IsValid())
				return;
			Lua::CallFunction(l,[&o,hPanel,x,y](lua_State *l) {
				o.push(l);

				auto obj = WGUILuaInterface::GetLuaObject(l,*hPanel.get());
				obj.push(l);
				Lua::PushInt(l,x);
				Lua::PushInt(l,y);
				return Lua::StatusCode::Ok;
			});
		});
	}
	else if(name == "onchildadded")
	{
		hCallback = FunctionCallback<void,::WIBase*>::Create(
			[l,hPanel,o](::WIBase *el) {
			if(!hPanel.IsValid())
				return;
			Lua::CallFunction(l,[&o,hPanel,el](lua_State *l) {
				o.push(l);

				auto obj = WGUILuaInterface::GetLuaObject(l,*hPanel.get());
				obj.push(l);
				if(el)
				{
					auto objEl = WGUILuaInterface::GetLuaObject(l,*el);
					objEl.push(l);
				}
				return Lua::StatusCode::Ok;
			});
		});
	}
	else if(name == "onchildremoved")
	{
		hCallback = FunctionCallback<void,::WIBase*>::Create(
			[l,hPanel,o](::WIBase *el) {
			if(!hPanel.IsValid())
				return;
			Lua::CallFunction(l,[&o,hPanel,el](lua_State *l) {
				o.push(l);

				auto obj = WGUILuaInterface::GetLuaObject(l,*hPanel.get());
				obj.push(l);
				if(el)
				{
					auto objEl = WGUILuaInterface::GetLuaObject(l,*el);
					objEl.push(l);
				}
				return Lua::StatusCode::Ok;
			});
		});
	}
	else if(name == "onmousepressed")
	{
		hCallback = FunctionCallback<::util::EventReply>::CreateWithOptionalReturn(
			[l,hPanel,o](::util::EventReply *reply) -> CallbackReturnType {
			if(!hPanel.IsValid())
				return CallbackReturnType::NoReturnValue;
			if(Lua::CallFunction(l,[&o,hPanel](lua_State *l) {
				o.push(l);

				auto obj = WGUILuaInterface::GetLuaObject(l,*hPanel.get());
				obj.push(l);
				return Lua::StatusCode::Ok;
			},1) == Lua::StatusCode::Ok)
			{
				if(Lua::IsSet(l,-1) == false)
					return CallbackReturnType::NoReturnValue;
				auto result = static_cast<::util::EventReply>(Lua::CheckInt(l,-1));
				Lua::Pop(l,1);
				*reply = result;
				return (*reply != ::util::EventReply::Unhandled) ? CallbackReturnType::HasReturnValue : CallbackReturnType::NoReturnValue;
			}
			return CallbackReturnType::NoReturnValue;
		});
	}
	else if(name == "onmousereleased")
	{
		hCallback = FunctionCallback<::util::EventReply>::CreateWithOptionalReturn(
			[l,hPanel,o](::util::EventReply *reply) -> CallbackReturnType {
			if(!hPanel.IsValid())
				return CallbackReturnType::NoReturnValue;
			if(Lua::CallFunction(l,[&o,hPanel](lua_State *l) {
				o.push(l);

				auto obj = WGUILuaInterface::GetLuaObject(l,*hPanel.get());
				obj.push(l);
				return Lua::StatusCode::Ok;
			},1) == Lua::StatusCode::Ok)
			{
				if(Lua::IsSet(l,-1) == false)
					return CallbackReturnType::NoReturnValue;
				auto result = static_cast<::util::EventReply>(Lua::CheckInt(l,-1));
				Lua::Pop(l,1);
				*reply = result;
				return (*reply != ::util::EventReply::Unhandled) ? CallbackReturnType::HasReturnValue : CallbackReturnType::NoReturnValue;
			}
			return CallbackReturnType::NoReturnValue;
		});
	}
	else if(name == "ondoubleclick")
	{
		hCallback = FunctionCallback<::util::EventReply>::CreateWithOptionalReturn(
			[l,hPanel,o](::util::EventReply *reply) -> CallbackReturnType {
			if(!hPanel.IsValid())
				return CallbackReturnType::NoReturnValue;
			if(Lua::CallFunction(l,[&o,hPanel](lua_State *l) {
				o.push(l);

				auto obj = WGUILuaInterface::GetLuaObject(l,*hPanel.get());
				obj.push(l);
				return Lua::StatusCode::Ok;
			},1) == Lua::StatusCode::Ok)
			{
				if(Lua::IsSet(l,-1) == false)
					return CallbackReturnType::NoReturnValue;
				auto result = static_cast<::util::EventReply>(Lua::CheckInt(l,-1));
				Lua::Pop(l,1);
				*reply = result;
				return (*reply != ::util::EventReply::Unhandled) ? CallbackReturnType::HasReturnValue : CallbackReturnType::NoReturnValue;
			}
			return CallbackReturnType::NoReturnValue;
		});
	}
	else if(name == "onjoystickevent")
	{
		hCallback = FunctionCallback<::util::EventReply,std::reference_wrapper<const GLFW::Joystick>,uint32_t,GLFW::KeyState>::CreateWithOptionalReturn(
			[l,hPanel,o](::util::EventReply *reply,std::reference_wrapper<const GLFW::Joystick> joystick,uint32_t key,GLFW::KeyState state) -> CallbackReturnType {
			if(!hPanel.IsValid())
				return CallbackReturnType::NoReturnValue;
			if(Lua::CallFunction(l,[&o,hPanel,key,state](lua_State *l) {
				o.push(l);

				auto obj = WGUILuaInterface::GetLuaObject(l,*hPanel.get());
				obj.push(l);
				Lua::PushInt(l,key);
				Lua::PushInt(l,umath::to_integral(state));
				return Lua::StatusCode::Ok;
			},1) == Lua::StatusCode::Ok)
			{
				if(Lua::IsSet(l,-1) == false)
					return CallbackReturnType::NoReturnValue;
				auto result = static_cast<::util::EventReply>(Lua::CheckInt(l,-1));
				Lua::Pop(l,1);
				*reply = result;
				return (*reply != ::util::EventReply::Unhandled) ? CallbackReturnType::HasReturnValue : CallbackReturnType::NoReturnValue;
			}
			return CallbackReturnType::NoReturnValue;
		});
	}
	else if(name == "onscroll")
	{
		hCallback = FunctionCallback<::util::EventReply,Vector2>::CreateWithOptionalReturn([l,hPanel,o](::util::EventReply *reply,Vector2 offset) -> CallbackReturnType {
			if(!hPanel.IsValid())
				return CallbackReturnType::NoReturnValue;
			if(Lua::CallFunction(l,[&o,hPanel,&offset](lua_State *l) {
				o.push(l);

				auto obj = WGUILuaInterface::GetLuaObject(l,*hPanel.get());
				obj.push(l);
				Lua::PushNumber(l,offset.x);
				Lua::PushNumber(l,offset.y);
				return Lua::StatusCode::Ok;
			},1) == Lua::StatusCode::Ok)
			{
				if(Lua::IsSet(l,-1) == false)
					return CallbackReturnType::NoReturnValue;
				auto result = static_cast<::util::EventReply>(Lua::CheckInt(l,-1));
				Lua::Pop(l,1);
				*reply = result;
				return (*reply != ::util::EventReply::Unhandled) ? CallbackReturnType::HasReturnValue : CallbackReturnType::NoReturnValue;
			}
			return CallbackReturnType::NoReturnValue;
		});
	}
	else if(name == "onchange" && dynamic_cast<::WICheckbox*>(hPanel.get()) != nullptr)
	{
		hCallback = FunctionCallback<void,bool>::Create([l,hPanel,o](bool bChecked) {
			if(!hPanel.IsValid())
				return ::util::EventReply::Unhandled;
			if(Lua::CallFunction(l,[&o,hPanel,bChecked](lua_State *l) {
				o.push(l);

				auto obj = WGUILuaInterface::GetLuaObject(l,*hPanel.get());
				obj.push(l);
				Lua::PushBool(l,bChecked);
				return Lua::StatusCode::Ok;
			},1) == Lua::StatusCode::Ok)
			{
				auto result = Lua::IsSet(l,-1) ? static_cast<::util::EventReply>(Lua::CheckInt(l,-1)) : ::util::EventReply::Unhandled;
				Lua::Pop(l,1);
				return result;
			}
			return ::util::EventReply::Unhandled;
		});
	}
	else if(name == "onoptionselected")
	{
		hCallback = FunctionCallback<void,uint32_t>::Create([l,hPanel,o](uint32_t idx) {
			if(!hPanel.IsValid())
				return;
			Lua::CallFunction(l,[&o,hPanel,idx](lua_State *l) {
				o.push(l);

				auto *p = hPanel.get();
				auto obj = WGUILuaInterface::GetLuaObject(l,*p);
				obj.push(l);

				auto optIdx = (idx == std::numeric_limits<uint32_t>::max()) ? -1 : static_cast<int32_t>(idx);
				Lua::PushInt(l,optIdx);
				return Lua::StatusCode::Ok;
			},0);
		});
	}
	else if(name == "translatetransformposition")
	{
		hCallback = FunctionCallback<void,std::reference_wrapper<Vector2i>,bool>::Create([l,hPanel,o](std::reference_wrapper<Vector2i> pos,bool bDrag) {
			if(!hPanel.IsValid())
				return;
			auto r = Lua::CallFunction(l,[&o,hPanel,pos,bDrag](lua_State *l) {
				o.push(l);

				auto obj = WGUILuaInterface::GetLuaObject(l,*hPanel.get());
				obj.push(l);
				Lua::Push<Vector2i>(l,pos);
				Lua::PushBool(l,bDrag);
				return Lua::StatusCode::Ok;
			},1);
			if(r == Lua::StatusCode::Ok)
			{
				if(Lua::IsVector2i(l,-1))
					pos.get() = *Lua::CheckVector2i(l,-1);
				Lua::Pop(l,1);
			}
		});
	}
	else if(name == "onchange")
	{
		hCallback = FunctionCallback<void,float,float>::Create([l,hPanel,o](float progress,float value) {
			if(!hPanel.IsValid())
				return;
			Lua::CallFunction(l,[&o,hPanel,progress,value](lua_State *l) {
				o.push(l);

				auto *p = hPanel.get();
				auto obj = WGUILuaInterface::GetLuaObject(l,*p);
				obj.push(l);

				Lua::PushNumber(l,progress);
				Lua::PushNumber(l,value);
				return Lua::StatusCode::Ok;
			},0);
		});
	}
	else if(name == "translatevalue")
	{
		hCallback = FunctionCallback<bool,float,std::reference_wrapper<std::string>>::Create([l,hPanel,o](float rawValue,std::reference_wrapper<std::string> value) {
			if(!hPanel.IsValid())
				return false;
			auto retVal = false;
			auto r = Lua::CallFunction(l,[&o,hPanel,rawValue,value,&retVal](lua_State *l) {
				o.push(l);

				auto *p = hPanel.get();
				auto obj = WGUILuaInterface::GetLuaObject(l,*p);
				obj.push(l);

				Lua::PushNumber(l,rawValue);
				return Lua::StatusCode::Ok;
			},1);
			if(r == Lua::StatusCode::Ok && Lua::IsSet(l,-1))
			{
				value.get() = Lua::CheckString(l,-1);
				retVal = true;
			}
			return retVal;
		});
	}
	else
	{
		hCallback = FunctionCallback<>::Create([l,hPanel,o]() {
			if(!hPanel.IsValid())
				return;
			Lua::CallFunction(l,[&o,hPanel](lua_State *l) {
				o.push(l);

				auto obj = WGUILuaInterface::GetLuaObject(l,*hPanel.get());
				obj.push(l);
				return Lua::StatusCode::Ok;
			},0);
		});
	}
	auto hCallbackRet = hPanel->AddCallback(name,hCallback);
	auto itCallback = callbackPtr->callbacks.find(name);
	if(itCallback == callbackPtr->callbacks.end())
		itCallback = callbackPtr->callbacks.insert(decltype(callbackPtr->callbacks)::value_type(name,{})).first;
	itCallback->second.push_back({l,hCallback,o});
	Lua::Push<CallbackHandle>(l,hCallbackRet);
}
void Lua::WIBase::FadeIn(lua_State *l,WIHandle &hPanel,float tFadeIn,float alphaTarget)
{
	lua_checkgui(l,hPanel);
	hPanel->FadeIn(tFadeIn,alphaTarget /255.f);
}
void Lua::WIBase::FadeIn(lua_State *l,WIHandle &hPanel,float tFadeIn)
{
	Lua::WIBase::FadeIn(l,hPanel,tFadeIn,255.f);
}
void Lua::WIBase::FadeOut(lua_State *l,WIHandle &hPanel,float tFadeOut,bool remove)
{
	lua_checkgui(l,hPanel);
	hPanel->FadeOut(tFadeOut,remove);
}
void Lua::WIBase::FadeOut(lua_State *l,WIHandle &hPanel,float tFadeOut)
{
	Lua::WIBase::FadeOut(l,hPanel,tFadeOut,false);
}
void Lua::WIBase::IsFading(lua_State *l,WIHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Lua::PushBool(l,hPanel->IsFading());
}
void Lua::WIBase::IsFadingIn(lua_State *l,WIHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Lua::PushBool(l,hPanel->IsFadingIn());
}
void Lua::WIBase::IsFadingOut(lua_State *l,WIHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Lua::PushBool(l,hPanel->IsFadingOut());
}
void Lua::WIBase::GetClass(lua_State *l,WIHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Lua::PushString(l,hPanel->GetClass());
}
void Lua::WIBase::Think(lua_State *l,WIHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	hPanel->Think();
}
void Lua::WIBase::InjectMouseMoveInput(lua_State *l,WIHandle &hPanel,const Vector2 &mousePos)
{
	lua_checkgui(l,hPanel);
	auto &window = c_engine->GetWindow();
	auto absPos = hPanel->GetAbsolutePos();
	window.SetCursorPosOverride(Vector2{static_cast<float>(absPos.x +mousePos.x),static_cast<float>(absPos.y +mousePos.y)});
	ScopeGuard sg {[&window]() {
		window.ClearCursorPosOverride();
	}};
	hPanel->InjectMouseMoveInput(mousePos.x,mousePos.y);
}
void Lua::WIBase::InjectMouseInput(lua_State *l,WIHandle &hPanel,const Vector2 &mousePos,int button,int action,int mods)
{
 	lua_checkgui(l,hPanel);
	auto &window = c_engine->GetWindow();
	auto absPos = hPanel->GetAbsolutePos();
	window.SetCursorPosOverride(Vector2{static_cast<float>(absPos.x +mousePos.x),static_cast<float>(absPos.y +mousePos.y)});
	ScopeGuard sg {[&window]() {
		window.ClearCursorPosOverride();
	}};
	hPanel->InjectMouseInput(GLFW::MouseButton(button),GLFW::KeyState(action),GLFW::Modifier(mods));
}
void Lua::WIBase::InjectMouseInput(lua_State *l,WIHandle &hPanel,const Vector2 &mousePos,int button,int action) {InjectMouseInput(l,hPanel,mousePos,button,action,0);}
void Lua::WIBase::InjectKeyboardInput(lua_State *l,WIHandle &hPanel,int key,int action,int mods)
{
	lua_checkgui(l,hPanel);
	hPanel->InjectKeyboardInput(GLFW::Key(key),0,GLFW::KeyState(action),GLFW::Modifier(mods));
	 // Vulkan TODO
}
void Lua::WIBase::InjectKeyboardInput(lua_State *l,WIHandle &hPanel,int key,int action) {InjectKeyboardInput(l,hPanel,key,action,0);}
void Lua::WIBase::InjectCharInput(lua_State *l,WIHandle &hPanel,std::string c,uint32_t mods)
{
	lua_checkgui(l,hPanel);
	if(c.empty())
		return;
	const char *cStr = c.c_str();
	hPanel->InjectCharInput(cStr[0],static_cast<GLFW::Modifier>(mods));
}
void Lua::WIBase::InjectCharInput(lua_State *l,WIHandle &hPanel,std::string c)
{
	lua_checkgui(l,hPanel);
	if(c.empty())
		return;
	const char *cStr = c.c_str();
	hPanel->InjectCharInput(cStr[0]);
}
void Lua::WIBase::InjectScrollInput(lua_State *l,WIHandle &hPanel,const Vector2 &mousePos,const Vector2 &offset)
{
	lua_checkgui(l,hPanel);
	auto &window = c_engine->GetWindow();
	auto cursorPos = window.GetCursorPos();
	auto absPos = hPanel->GetAbsolutePos();
	window.SetCursorPosOverride(Vector2{static_cast<float>(absPos.x +mousePos.x),static_cast<float>(absPos.y +mousePos.y)});
	hPanel->InjectScrollInput(offset);
	window.ClearCursorPosOverride();
}
void Lua::WIBase::IsDescendant(lua_State *l,WIHandle &hPanel,WIHandle &hOther)
{
	lua_checkgui(l,hPanel);
	lua_checkgui(l,hOther);
	Lua::PushBool(l,hPanel->IsDescendant(hOther.get()));
}
void Lua::WIBase::IsDescendantOf(lua_State *l,WIHandle &hPanel,WIHandle &hOther)
{
	lua_checkgui(l,hPanel);
	lua_checkgui(l,hOther);
	Lua::PushBool(l,hPanel->IsDescendantOf(hOther.get()));
}
void Lua::WIBase::IsAncestor(lua_State *l,WIHandle &hPanel,WIHandle &hOther)
{
	lua_checkgui(l,hPanel);
	lua_checkgui(l,hOther);
	Lua::PushBool(l,hPanel->IsAncestor(hOther.get()));
}
void Lua::WIBase::IsAncestorOf(lua_State *l,WIHandle &hPanel,WIHandle &hOther)
{
	lua_checkgui(l,hPanel);
	lua_checkgui(l,hOther);
	Lua::PushBool(l,hPanel->IsAncestorOf(hOther.get()));
}
void Lua::WIBase::GetName(lua_State *l,WIHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Lua::PushString(l,hPanel->GetName());
}
void Lua::WIBase::SetName(lua_State *l,WIHandle &hPanel,std::string name)
{
	lua_checkgui(l,hPanel);
	hPanel->SetName(name);
}
void Lua::WIBase::FindChildByName(lua_State *l,WIHandle &hPanel,std::string name)
{
	lua_checkgui(l,hPanel);
	auto *el = hPanel->FindChildByName(name);
	if(el == nullptr)
		return;
	auto oChild = WGUILuaInterface::GetLuaObject(l,*el);
	oChild.push(l);
}
void Lua::WIBase::FindChildrenByName(lua_State *l,WIHandle &hPanel,std::string name)
{
	lua_checkgui(l,hPanel);
	std::vector<WIHandle> children;
	hPanel->FindChildrenByName(name,children);
	int table = Lua::CreateTable(l);
	unsigned int c = 1;
	for(unsigned int i=0;i<children.size();i++)
	{
		WIHandle &hChild = children[i];
		if(hChild.IsValid())
		{
			auto *pChild = hChild.get();
			auto oChild = WGUILuaInterface::GetLuaObject(l,*pChild);
			Lua::PushInt(l,c);
			oChild.push(l);
			Lua::SetTableValue(l,table);
			c++;
		}
	}
}
void Lua::WIBase::SetAutoAlignToParent(lua_State *l,WIHandle &hPanel,bool bX,bool bY)
{
	lua_checkgui(l,hPanel);
	hPanel->SetAutoAlignToParent(bX,bY);
}
void Lua::WIBase::SetAutoAlignToParent(lua_State *l,WIHandle &hPanel,bool b)
{
	lua_checkgui(l,hPanel);
	hPanel->SetAutoAlignToParent(b);
}
void Lua::WIBase::GetAutoAlignToParentX(lua_State *l,WIHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Lua::PushBool(l,hPanel->GetAutoAlignToParentX());
}
void Lua::WIBase::GetAutoAlignToParentY(lua_State *l,WIHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Lua::PushBool(l,hPanel->GetAutoAlignToParentY());
}
void Lua::WIBase::Resize(lua_State *l,WIHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	hPanel->Resize();
}
void Lua::WIBase::ScheduleUpdate(lua_State *l,WIHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	hPanel->ScheduleUpdate();
}
void Lua::WIBase::SetSkin(lua_State *l,WIHandle &hPanel,std::string skin)
{
	lua_checkgui(l,hPanel);
	hPanel->SetSkin(skin);
}
void Lua::WIBase::ResetSkin(lua_State *l,WIHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	hPanel->ResetSkin();
}
void Lua::WIBase::GetStyleClasses(lua_State *l,WIHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	std::vector<std::string> &classes = hPanel->GetStyleClasses();
	std::vector<std::string>::iterator it;
	int table = Lua::CreateTable(l);
	unsigned int idx = 1;
	for(it=classes.begin();it!=classes.end();it++)
	{
		Lua::PushInt(l,idx);
		Lua::PushString(l,*it);
		Lua::SetTableValue(l,table);
		idx++;
	}
}
void Lua::WIBase::AddStyleClass(lua_State *l,WIHandle &hPanel,std::string styleClass)
{
	lua_checkgui(l,hPanel);
	hPanel->AddStyleClass(styleClass);
}
void Lua::WIBase::SetCursor(lua_State *l,WIHandle &hPanel,uint32_t cursor)
{
	lua_checkgui(l,hPanel);
	hPanel->SetCursor(static_cast<GLFW::Cursor::Shape>(cursor));
}
void Lua::WIBase::GetCursor(lua_State *l,WIHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Lua::PushInt(l,umath::to_integral(hPanel->GetCursor()));
}
void Lua::WIBase::RemoveElementOnRemoval(lua_State *l,WIHandle &hPanel,WIHandle &hOther)
{
	lua_checkgui(l,hPanel);
	lua_checkgui(l,hOther);
	hPanel->RemoveOnRemoval(hOther.get());
}
void Lua::WIBase::GetTooltip(lua_State *l,WIHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Lua::PushString(l,hPanel->GetTooltip());
}
void Lua::WIBase::SetTooltip(lua_State *l,WIHandle &hPanel,const std::string &tooltip)
{
	lua_checkgui(l,hPanel);
	hPanel->SetTooltip(tooltip);
}
void Lua::WIBase::HasTooltip(lua_State *l,WIHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Lua::PushBool(l,hPanel->HasTooltip());
}
void Lua::WIBase::GetLeft(lua_State *l,WIHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Lua::PushInt(l,hPanel->GetLeft());
}
void Lua::WIBase::GetTop(lua_State *l,WIHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Lua::PushInt(l,hPanel->GetTop());
}
void Lua::WIBase::GetRight(lua_State *l,WIHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Lua::PushInt(l,hPanel->GetRight());
}
void Lua::WIBase::GetBottom(lua_State *l,WIHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Lua::PushInt(l,hPanel->GetBottom());
}
void Lua::WIBase::GetEndPos(lua_State *l,WIHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Lua::Push<Vector2i>(l,hPanel->GetEndPos());
}
void Lua::WIBase::SetClippingEnabled(lua_State *l,WIHandle &hPanel,bool b)
{
	lua_checkgui(l,hPanel);
	hPanel->SetShouldScissor(b);
}
void Lua::WIBase::IsClippingEnabled(lua_State *l,WIHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Lua::PushBool(l,hPanel->GetShouldScissor());
}
void Lua::WIBase::SetAlwaysUpdate(lua_State *l,WIHandle &hPanel,bool b)
{
	lua_checkgui(l,hPanel);
	hPanel->SetThinkIfInvisible(b);
}
void Lua::WIBase::SetBounds(lua_State *l,WIHandle &hPanel,const Vector2 &start,const Vector2 &end)
{
	lua_checkgui(l,hPanel);
	auto &pos = start;
	auto size = end -start;
	hPanel->SetPos(pos);
	hPanel->SetSize(size);
}
void Lua::WIBase::SetBackgroundElement(lua_State *l,WIHandle &hPanel,bool backgroundElement,bool autoAlignToParent)
{
	lua_checkgui(l,hPanel);
	hPanel->SetBackgroundElement(backgroundElement,autoAlignToParent);
}
void Lua::WIBase::SetBackgroundElement(lua_State *l,WIHandle &hPanel,bool backgroundElement) {SetBackgroundElement(l,hPanel,backgroundElement,true);}
void Lua::WIBase::SetBackgroundElement(lua_State *l,WIHandle &hPanel) {SetBackgroundElement(l,hPanel,true);}
void Lua::WIBase::IsBackgroundElement(lua_State *l,WIHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Lua::PushBool(l,hPanel->IsBackgroundElement());
}
void Lua::WIBase::AddAttachment(lua_State *l,WIHandle &hPanel,const std::string &name,const Vector2 &position)
{
	lua_checkgui(l,hPanel);
	auto *pAttachment = hPanel->AddAttachment(name,position);
	if(pAttachment == nullptr)
		return;
	Lua::Property::push(l,*pAttachment->GetAbsPosProperty());
}
void Lua::WIBase::AddAttachment(lua_State *l,WIHandle &hPanel,const std::string &name) {AddAttachment(l,hPanel,name,{});}
void Lua::WIBase::SetAttachmentPos(lua_State *l,WIHandle &hPanel,const std::string &name,const Vector2 &position)
{
	lua_checkgui(l,hPanel);
	hPanel->SetAttachmentPos(name,position);
}
void Lua::WIBase::GetAttachmentPos(lua_State *l,WIHandle &hPanel,const std::string &name)
{
	lua_checkgui(l,hPanel);
	auto *pos = hPanel->GetAttachmentPos(name);
	if(pos == nullptr)
		return;
	Lua::Push<Vector2i>(l,*pos);
}
void Lua::WIBase::GetAbsoluteAttachmentPos(lua_State *l,WIHandle &hPanel,const std::string &name)
{
	lua_checkgui(l,hPanel);
	auto *pos = hPanel->GetAbsoluteAttachmentPos(name);
	if(pos == nullptr)
		return;
	Lua::Push<Vector2>(l,*pos);
}
void Lua::WIBase::GetAttachmentPosProperty(lua_State *l,WIHandle &hPanel,const std::string &name)
{
	lua_checkgui(l,hPanel);
	auto *posProperty = hPanel->GetAttachmentPosProperty(name);
	if(posProperty == nullptr)
		return;
	Lua::Property::push(l,**posProperty);
}
void Lua::WIBase::SetAnchor(lua_State *l,WIHandle &hPanel,float left,float top,float right,float bottom)
{
	lua_checkgui(l,hPanel);
	hPanel->SetAnchor(left,top,right,bottom);
}
void Lua::WIBase::SetAnchor(lua_State *l,WIHandle &hPanel,float left,float top,float right,float bottom,uint32_t refWidth,uint32_t refHeight)
{
	lua_checkgui(l,hPanel);
	hPanel->SetAnchor(left,top,right,bottom,refWidth,refHeight);
}
void Lua::WIBase::SetAnchorLeft(lua_State *l,WIHandle &hPanel,float f)
{
	lua_checkgui(l,hPanel);
	hPanel->SetAnchorLeft(f);
}
void Lua::WIBase::SetAnchorRight(lua_State *l,WIHandle &hPanel,float f)
{
	lua_checkgui(l,hPanel);
	hPanel->SetAnchorRight(f);
}
void Lua::WIBase::SetAnchorTop(lua_State *l,WIHandle &hPanel,float f)
{
	lua_checkgui(l,hPanel);
	hPanel->SetAnchorTop(f);
}
void Lua::WIBase::SetAnchorBottom(lua_State *l,WIHandle &hPanel,float f)
{
	lua_checkgui(l,hPanel);
	hPanel->SetAnchorBottom(f);
}
void Lua::WIBase::GetAnchor(lua_State *l,WIHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	float left,right,top,bottom;
	auto bAnchor = hPanel->GetAnchor(left,top,right,bottom);
	if(bAnchor == false)
		return;
	Lua::PushNumber(l,left);
	Lua::PushNumber(l,top);
	Lua::PushNumber(l,right);
	Lua::PushNumber(l,bottom);
}
void Lua::WIBase::HasAnchor(lua_State *l,WIHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Lua::PushBool(l,hPanel->HasAnchor());
}

////////////////////////////////////

void Lua::WIProgressBar::SetProgress(lua_State *l,WIProgressBarHandle &hBar,float progress)
{
	lua_checkgui(l,hBar);
	static_cast<::WIProgressBar*>(hBar.get())->SetProgress(progress);
}
void Lua::WIProgressBar::GetProgress(lua_State *l,WIProgressBarHandle &hBar)
{
	lua_checkgui(l,hBar);
	Lua::PushNumber(l,static_cast<::WIProgressBar*>(hBar.get())->GetProgress());
}
void Lua::WIProgressBar::SetValue(lua_State *l,WIProgressBarHandle &hBar,float v)
{
	lua_checkgui(l,hBar);
	static_cast<::WIProgressBar*>(hBar.get())->SetValue(v);
}
void Lua::WIProgressBar::GetValue(lua_State *l,WIProgressBarHandle &hBar)
{
	lua_checkgui(l,hBar);
	Lua::PushNumber(l,static_cast<::WIProgressBar*>(hBar.get())->GetValue());
}
void Lua::WIProgressBar::SetRange(lua_State *l,WIProgressBarHandle &hBar,float min,float max,float stepSize)
{
	lua_checkgui(l,hBar);
	static_cast<::WIProgressBar*>(hBar.get())->SetRange(min,max,stepSize);
}
void Lua::WIProgressBar::SetRange(lua_State *l,WIProgressBarHandle &hBar,float min,float max)
{
	lua_checkgui(l,hBar);
	static_cast<::WIProgressBar*>(hBar.get())->SetRange(min,max);
}
void Lua::WIProgressBar::SetOptions(lua_State *l,WIProgressBarHandle &hBar,luabind::object o)
{
	lua_checkgui(l,hBar);
	int32_t t = 2;
	Lua::CheckTable(l,t);
	o.push(l); /* 1 */

	std::vector<std::string> options;
	auto numOptions = Lua::GetObjectLength(l,t);
	options.reserve(numOptions);

	Lua::PushNil(l); /* 2 */
	while(Lua::GetNextPair(l,t) != 0) /* 3 */
	{
		auto *option = Lua::CheckString(l,-1);
		options.push_back(option);
		Lua::Pop(l,1); /* 2 */
	} /* 1 */

	Lua::Pop(l,1); /* 0 */

	static_cast<::WIProgressBar*>(hBar.get())->SetOptions(options);
}
void Lua::WIProgressBar::AddOption(lua_State *l,WIProgressBarHandle &hBar,const std::string &option)
{
	lua_checkgui(l,hBar);
	static_cast<::WIProgressBar*>(hBar.get())->AddOption(option);
}
void Lua::WIProgressBar::SetPostFix(lua_State *l,WIProgressBarHandle &hBar,const std::string &postfix)
{
	lua_checkgui(l,hBar);
	static_cast<::WIProgressBar*>(hBar.get())->SetPostFix(postfix);
}

////////////////////////////////////

void Lua::WIButton::SetText(lua_State *l,WIButtonHandle &hPanel,std::string text)
{
	lua_checkgui(l,hPanel);
	hPanel.get<::WIButton>()->SetText(text);
}
void Lua::WIButton::GetText(lua_State *l,WIButtonHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Lua::PushString(l,hPanel.get<::WIButton>()->GetText());
}

////////////////////////////////////

void Lua::WIShape::AddVertex(lua_State *l,WIShapeHandle &hPanel,Vector2 v)
{
	lua_checkgui(l,hPanel);
	unsigned int vertID = hPanel.get<::WIShape>()->AddVertex(v);
	Lua::PushInt(l,vertID);
}

void Lua::WIShape::SetVertexPos(lua_State *l,WIShapeHandle &hPanel,unsigned int vertID,Vector2 v)
{
	lua_checkgui(l,hPanel);
	hPanel.get<::WIShape>()->SetVertexPos(vertID,v);
}

void Lua::WIShape::ClearVertices(lua_State *l,WIShapeHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	hPanel.get<::WIShape>()->ClearVertices();
}

void Lua::WIShape::InvertVertexPositions(lua_State *l,WIShapeHandle &hPanel,bool x,bool y)
{
	lua_checkgui(l,hPanel);
	hPanel.get<::WIShape>()->InvertVertexPositions(x,y);
}
void Lua::WIShape::InvertVertexPositions(lua_State *l,WIShapeHandle &hPanel) {InvertVertexPositions(l,hPanel,true,true);}

////////////////////////////////////

void Lua::WITexturedShape::SetMaterial(lua_State *l,WITexturedShapeHandle &hPanel,std::string mat)
{
	lua_checkgui(l,hPanel);
	hPanel.get<::WITexturedShape>()->SetMaterial(mat);
}

void Lua::WITexturedShape::SetMaterial(lua_State *l,WITexturedShapeHandle &hPanel,Material *mat)
{
	lua_checkgui(l,hPanel);
	hPanel.get<::WITexturedShape>()->SetMaterial(mat);
}

void Lua::WITexturedShape::GetMaterial(lua_State *l,WITexturedShapeHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Material *mat = hPanel.get<::WITexturedShape>()->GetMaterial();
	if(mat == NULL)
		return;
	luabind::object(l,mat).push(l);
}

void Lua::WITexturedShape::SetTexture(lua_State *l,WITexturedShapeHandle &hPanel,std::shared_ptr<prosper::Texture> &tex)
{
	lua_checkgui(l,hPanel);
	hPanel.get<::WITexturedShape>()->SetTexture(*tex);
}

void Lua::WITexturedShape::GetTexture(lua_State *l,WITexturedShapeHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	auto &tex = hPanel.get<::WITexturedShape>()->GetTexture();
	if(tex == nullptr)
		return;
	Lua::Push(l,tex);
}

void Lua::WITexturedShape::AddVertex(lua_State *l,WITexturedShapeHandle &hPanel,Vector2 v)
{
	lua_checkgui(l,hPanel);
	unsigned int vertID = hPanel.get<::WITexturedShape>()->AddVertex(v);
	Lua::PushInt(l,vertID);
}

void Lua::WITexturedShape::AddVertex(lua_State *l,WITexturedShapeHandle &hPanel,Vector2 v,Vector2 uv)
{
	lua_checkgui(l,hPanel);
	unsigned int vertID = hPanel.get<::WITexturedShape>()->AddVertex(v,uv);
	Lua::PushInt(l,vertID);
}

void Lua::WITexturedShape::SetVertexUVCoord(lua_State *l,WITexturedShapeHandle &hPanel,unsigned int vertID,Vector2 uv)
{
	lua_checkgui(l,hPanel);
	hPanel.get<::WITexturedShape>()->SetVertexUVCoord(vertID,uv);
}

void Lua::WITexturedShape::InvertVertexUVCoordinates(lua_State *l,WITexturedShapeHandle &hPanel,bool x,bool y)
{
	lua_checkgui(l,hPanel);
	hPanel.get<::WITexturedShape>()->InvertVertexUVCoordinates(x,y);
}
void Lua::WITexturedShape::InvertVertexUVCoordinates(lua_State *l,WITexturedShapeHandle &hPanel) {InvertVertexUVCoordinates(l,hPanel,true,true);}

////////////////////////////////////

void Lua::WIText::SetText(lua_State *l,WITextHandle &hPanel,std::string text)
{
	lua_checkgui(l,hPanel);
	hPanel.get<::WIText>()->SetText(text);
}

void Lua::WIText::GetText(lua_State *l,WITextHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	std::string text = hPanel.get<::WIText>()->GetText();
	Lua::PushString(l,text);
}

void Lua::WIText::SetFont(lua_State *l,WITextHandle &hPanel,std::string font)
{
	lua_checkgui(l,hPanel);
	hPanel.get<::WIText>()->SetFont(font);
}

void Lua::WIText::EnableShadow(lua_State *l,WITextHandle &hPanel,bool b)
{
	lua_checkgui(l,hPanel);
	hPanel.get<::WIText>()->EnableShadow(b);
}

void Lua::WIText::IsShadowEnabled(lua_State *l,WITextHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Lua::PushBool(l,hPanel.get<::WIText>()->IsShadowEnabled());
}

void Lua::WIText::SetShadowColor(lua_State *l,WITextHandle &hPanel,Color &col)
{
	lua_checkgui(l,hPanel);
	hPanel.get<::WIText>()->SetShadowColor(Vector4(col.r /255.f,col.g /255.f,col.b /255.f,col.a /255.f));
}

void Lua::WIText::SetShadowOffset(lua_State *l,WITextHandle &hPanel,Vector2 pos)
{
	lua_checkgui(l,hPanel);
	hPanel.get<::WIText>()->SetShadowOffset(CInt32(pos.x),CInt32(pos.y));
}

void Lua::WIText::SetShadowXOffset(lua_State *l,WITextHandle &hPanel,float x)
{
	lua_checkgui(l,hPanel);
	hPanel.get<::WIText>()->SetShadowOffset(Vector2i(x,hPanel.get<::WIText>()->GetShadowOffset()->y));
}

void Lua::WIText::SetShadowYOffset(lua_State *l,WITextHandle &hPanel,float y)
{
	lua_checkgui(l,hPanel);
	hPanel.get<::WIText>()->SetShadowOffset(Vector2i(hPanel.get<::WIText>()->GetShadowOffset()->x,y));
}

void Lua::WIText::GetShadowColor(lua_State *l,WITextHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Vector4 *col = hPanel.get<::WIText>()->GetShadowColor();
	Lua::Push<Color>(l,Color(CInt16(col->r *255),CInt16(col->g *255),CInt16(col->b *255),CInt16(col->a *255)));
}

void Lua::WIText::GetShadowOffset(lua_State *l,WITextHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Vector2i *offset = hPanel.get<::WIText>()->GetShadowOffset();
	Lua::Push<Vector2>(l,Vector2(offset->x,offset->y));
}

void Lua::WIText::GetShadowXOffset(lua_State *l,WITextHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Lua::PushNumber(l,float(hPanel.get<::WIText>()->GetShadowOffset()->x));
}

void Lua::WIText::GetShadowYOffset(lua_State *l,WITextHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Lua::PushNumber(l,float(hPanel.get<::WIText>()->GetShadowOffset()->y));
}

void Lua::WIText::SetShadowAlpha(lua_State *l,WITextHandle &hPanel,float alpha)
{
	lua_checkgui(l,hPanel);
	hPanel.get<::WIText>()->SetShadowAlpha(alpha);
}
void Lua::WIText::GetShadowAlpha(lua_State *l,WITextHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Lua::PushNumber(l,hPanel.get<::WIText>()->GetShadowAlpha());
}
void Lua::WIText::SetShadowBlurSize(lua_State *l,WITextHandle &hPanel,float blurSize)
{
	lua_checkgui(l,hPanel);
	hPanel.get<::WIText>()->SetShadowBlurSize(blurSize);
}
void Lua::WIText::GetShadowBlurSize(lua_State *l,WITextHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Lua::PushNumber(l,hPanel.get<::WIText>()->GetShadowBlurSize());
}
void Lua::WIText::SetAutoBreakMode(lua_State *l,WITextHandle &hPanel,uint32_t autoBreakMode)
{
	lua_checkgui(l,hPanel);
	static_cast<::WIText*>(hPanel.get())->SetAutoBreakMode(static_cast<::WIText::AutoBreak>(autoBreakMode));
}

////////////////////////////////////

void Lua::WITextEntry::SetText(lua_State *l,WITextEntryHandle &hPanel,std::string text)
{
	lua_checkgui(l,hPanel);
	hPanel.get<::WITextEntry>()->SetText(text);
}

void Lua::WITextEntry::GetText(lua_State *l,WITextEntryHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Lua::PushString(l,std::string{hPanel.get<::WITextEntry>()->GetText()});
}

void Lua::WITextEntry::IsNumeric(lua_State *l,WITextEntryHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Lua::PushBool(l,hPanel.get<::WITextEntry>()->IsNumeric());
}

void Lua::WITextEntry::IsEditable(lua_State *l,WITextEntryHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Lua::PushBool(l,hPanel.get<::WITextEntry>()->IsEditable());
}

void Lua::WITextEntry::SetEditable(lua_State *l,WITextEntryHandle &hPanel,bool b)
{
	lua_checkgui(l,hPanel);
	hPanel.get<::WITextEntry>()->SetEditable(b);
}

void Lua::WITextEntry::SetMaxLength(lua_State *l,WITextEntryHandle &hPanel,int len)
{
	lua_checkgui(l,hPanel);
	hPanel.get<::WITextEntry>()->SetMaxLength(len);
}

void Lua::WITextEntry::GetMaxLength(lua_State *l,WITextEntryHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Lua::PushInt(l,hPanel.get<::WITextEntry>()->GetMaxLength());
}

void Lua::WITextEntry::IsMultiLine(lua_State *l,WITextEntryHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Lua::PushBool(l,hPanel.get<::WITextEntry>()->IsMultiLine());
}

void Lua::WITextEntry::SetMultiLine(lua_State *l,WITextEntryHandle &hPanel,bool b)
{
	lua_checkgui(l,hPanel);
	hPanel.get<::WITextEntry>()->SetMultiLine(b);
}

////////////////////////////////////

void Lua::WINumericEntry::SetMinValue(lua_State *l,WINumericEntryHandle &hPanel,int32_t min)
{
	lua_checkgui(l,hPanel);
	hPanel.get<::WINumericEntry>()->SetMinValue(min);
}
void Lua::WINumericEntry::SetMinValue(lua_State *l,WINumericEntryHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	hPanel.get<::WINumericEntry>()->SetMinValue();
}
void Lua::WINumericEntry::SetMaxValue(lua_State *l,WINumericEntryHandle &hPanel,int32_t max)
{
	lua_checkgui(l,hPanel);
	hPanel.get<::WINumericEntry>()->SetMaxValue(max);
}
void Lua::WINumericEntry::SetMaxValue(lua_State *l,WINumericEntryHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	hPanel.get<::WINumericEntry>()->SetMaxValue();
}
void Lua::WINumericEntry::SetRange(lua_State *l,WINumericEntryHandle &hPanel,int32_t min,int32_t max)
{
	lua_checkgui(l,hPanel);
	hPanel.get<::WINumericEntry>()->SetRange(min,max);
}
void Lua::WINumericEntry::GetMinValue(lua_State *l,WINumericEntryHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	auto *val = hPanel.get<::WINumericEntry>()->GetMinValue();
	if(val == nullptr)
		return;
	Lua::PushInt(l,*val);
}
void Lua::WINumericEntry::GetMaxValue(lua_State *l,WINumericEntryHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	auto *val = hPanel.get<::WINumericEntry>()->GetMaxValue();
	if(val == nullptr)
		return;
	Lua::PushInt(l,*val);
}

////////////////////////////////////

void Lua::WIOutlinedRect::SetOutlineWidth(lua_State *l,WIOutlinedRectHandle &hPanel,int w)
{
	lua_checkgui(l,hPanel);
	hPanel.get<::WIOutlinedRect>()->SetOutlineWidth(w);
}

void Lua::WIOutlinedRect::GetOutlineWidth(lua_State *l,WIOutlinedRectHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Lua::PushInt(l,hPanel.get<::WIOutlinedRect>()->GetOutlineWidth());
}

////////////////////////////////////

void Lua::WILine::SetLineWidth(lua_State *l,WILineHandle &hPanel,int w)
{
	lua_checkgui(l,hPanel);
	hPanel.get<::WILine>()->SetLineWidth(w);
}
void Lua::WILine::GetLineWidth(lua_State *l,WILineHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Lua::PushInt(l,hPanel.get<::WILine>()->GetLineWidth());
}
void Lua::WILine::SetStartPos(lua_State *l,WILineHandle &hPanel,Vector2 &pos)
{
	lua_checkgui(l,hPanel);
	hPanel.get<::WILine>()->SetStartPos(Vector2i(pos.x,pos.y));
}
void Lua::WILine::SetEndPos(lua_State *l,WILineHandle &hPanel,Vector2 &pos)
{
	lua_checkgui(l,hPanel);
	hPanel.get<::WILine>()->SetEndPos(Vector2i(pos.x,pos.y));
}
void Lua::WILine::GetStartPos(lua_State *l,WILineHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Vector2i &pos = hPanel.get<::WILine>()->GetStartPos();
	Lua::Push<Vector2>(l,Vector2(pos.x,pos.y));
}
void Lua::WILine::GetEndPos(lua_State *l,WILineHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Vector2i &pos = hPanel.get<::WILine>()->GetEndPos();
	Lua::Push<Vector2>(l,Vector2(pos.x,pos.y));
}
void Lua::WILine::SetStartColor(lua_State *l,WILineHandle &hPanel,const Color &col)
{
	lua_checkgui(l,hPanel);
	hPanel.get<::WILine>()->SetStartColor(col);
}
void Lua::WILine::SetEndColor(lua_State *l,WILineHandle &hPanel,const Color &col)
{
	lua_checkgui(l,hPanel);
	hPanel.get<::WILine>()->SetEndColor(col);
}
void Lua::WILine::GetStartColor(lua_State *l,WILineHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Lua::Push<Color>(l,hPanel.get<::WILine>()->GetStartColor());
}
void Lua::WILine::GetEndColor(lua_State *l,WILineHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Lua::Push<Color>(l,hPanel.get<::WILine>()->GetEndColor());
}
void Lua::WILine::GetStartPosProperty(lua_State *l,WILineHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Lua::Property::push(l,*static_cast<::WILine*>(hPanel.get())->GetStartPosProperty());
}
void Lua::WILine::GetEndPosProperty(lua_State *l,WILineHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Lua::Property::push(l,*static_cast<::WILine*>(hPanel.get())->GetEndPosProperty());
}

////////////////////////////////////

void Lua::WIRoundedRect::GetRoundness(lua_State *l,WIRoundedRectHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Lua::PushInt(l,hPanel.get<::WIRoundedRect>()->GetRoundness());
}
void Lua::WIRoundedRect::SetRoundness(lua_State *l,WIRoundedRectHandle &hPanel,char roundness)
{
	lua_checkgui(l,hPanel);
	hPanel.get<::WIRoundedRect>()->SetRoundness(roundness);
}
void Lua::WIRoundedRect::SetCornerSize(lua_State *l,WIRoundedRectHandle &hPanel,float cornerSize)
{
	lua_checkgui(l,hPanel);
	hPanel.get<::WIRoundedRect>()->SetCornerSize(cornerSize);
}
void Lua::WIRoundedRect::GetCornerSize(lua_State *l,WIRoundedRectHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Lua::PushInt(l,hPanel.get<::WIRoundedRect>()->GetCornerSize());
}
void Lua::WIRoundedRect::SetRoundTopRight(lua_State *l,WIRoundedRectHandle &hPanel,bool b)
{
	lua_checkgui(l,hPanel);
	hPanel.get<::WIRoundedRect>()->SetRoundTopRight(b);
}
void Lua::WIRoundedRect::SetRoundTopLeft(lua_State *l,WIRoundedRectHandle &hPanel,bool b)
{
	lua_checkgui(l,hPanel);
	hPanel.get<::WIRoundedRect>()->SetRoundTopLeft(b);
}
void Lua::WIRoundedRect::SetRoundBottomLeft(lua_State *l,WIRoundedRectHandle &hPanel,bool b)
{
	lua_checkgui(l,hPanel);
	hPanel.get<::WIRoundedRect>()->SetRoundBottomLeft(b);
}
void Lua::WIRoundedRect::SetRoundBottomRight(lua_State *l,WIRoundedRectHandle &hPanel,bool b)
{
	lua_checkgui(l,hPanel);
	hPanel.get<::WIRoundedRect>()->SetRoundBottomRight(b);
}
void Lua::WIRoundedRect::IsTopRightRound(lua_State *l,WIRoundedRectHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Lua::PushBool(l,hPanel.get<::WIRoundedRect>()->IsTopRightRound());
}
void Lua::WIRoundedRect::IsTopLeftRound(lua_State *l,WIRoundedRectHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Lua::PushBool(l,hPanel.get<::WIRoundedRect>()->IsTopLeftRound());
}
void Lua::WIRoundedRect::IsBottomLeftRound(lua_State *l,WIRoundedRectHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Lua::PushBool(l,hPanel.get<::WIRoundedRect>()->IsBottomLeftRound());
}
void Lua::WIRoundedRect::IsBottomRightRound(lua_State *l,WIRoundedRectHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Lua::PushBool(l,hPanel.get<::WIRoundedRect>()->IsBottomRightRound());
}

////////////////////////////////////

void Lua::WIRoundedTexturedRect::GetRoundness(lua_State *l,WIRoundedTexturedRectHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Lua::PushInt(l,hPanel.get<::WIRoundedTexturedRect>()->GetRoundness());
}
void Lua::WIRoundedTexturedRect::SetRoundness(lua_State *l,WIRoundedTexturedRectHandle &hPanel,char roundness)
{
	lua_checkgui(l,hPanel);
	hPanel.get<::WIRoundedTexturedRect>()->SetRoundness(roundness);
}
void Lua::WIRoundedTexturedRect::SetCornerSize(lua_State *l,WIRoundedTexturedRectHandle &hPanel,float cornerSize)
{
	lua_checkgui(l,hPanel);
	hPanel.get<::WIRoundedTexturedRect>()->SetCornerSize(cornerSize);
}
void Lua::WIRoundedTexturedRect::GetCornerSize(lua_State *l,WIRoundedTexturedRectHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Lua::PushInt(l,hPanel.get<::WIRoundedTexturedRect>()->GetCornerSize());
}
void Lua::WIRoundedTexturedRect::SetRoundTopRight(lua_State *l,WIRoundedTexturedRectHandle &hPanel,bool b)
{
	lua_checkgui(l,hPanel);
	hPanel.get<::WIRoundedTexturedRect>()->SetRoundTopRight(b);
}
void Lua::WIRoundedTexturedRect::SetRoundTopLeft(lua_State *l,WIRoundedTexturedRectHandle &hPanel,bool b)
{
	lua_checkgui(l,hPanel);
	hPanel.get<::WIRoundedTexturedRect>()->SetRoundTopLeft(b);
}
void Lua::WIRoundedTexturedRect::SetRoundBottomLeft(lua_State *l,WIRoundedTexturedRectHandle &hPanel,bool b)
{
	lua_checkgui(l,hPanel);
	hPanel.get<::WIRoundedTexturedRect>()->SetRoundBottomLeft(b);
}
void Lua::WIRoundedTexturedRect::SetRoundBottomRight(lua_State *l,WIRoundedTexturedRectHandle &hPanel,bool b)
{
	lua_checkgui(l,hPanel);
	hPanel.get<::WIRoundedTexturedRect>()->SetRoundBottomRight(b);
}
void Lua::WIRoundedTexturedRect::IsTopRightRound(lua_State *l,WIRoundedTexturedRectHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Lua::PushBool(l,hPanel.get<::WIRoundedTexturedRect>()->IsTopRightRound());
}
void Lua::WIRoundedTexturedRect::IsTopLeftRound(lua_State *l,WIRoundedTexturedRectHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Lua::PushBool(l,hPanel.get<::WIRoundedTexturedRect>()->IsTopLeftRound());
}
void Lua::WIRoundedTexturedRect::IsBottomLeftRound(lua_State *l,WIRoundedTexturedRectHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Lua::PushBool(l,hPanel.get<::WIRoundedTexturedRect>()->IsBottomLeftRound());
}
void Lua::WIRoundedTexturedRect::IsBottomRightRound(lua_State *l,WIRoundedTexturedRectHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Lua::PushBool(l,hPanel.get<::WIRoundedTexturedRect>()->IsBottomRightRound());
}

////////////////////////////////////

void Lua::WIScrollBar::GetScrollAmount(lua_State *l,WIScrollBarHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Lua::PushInt(l,hPanel.get<::WIScrollBar>()->GetScrollAmount());
}
void Lua::WIScrollBar::SetScrollAmount(lua_State *l,WIScrollBarHandle &hPanel,int am)
{
	lua_checkgui(l,hPanel);
	hPanel.get<::WIScrollBar>()->SetScrollAmount(am);
}
void Lua::WIScrollBar::GetScrollOffset(lua_State *l,WIScrollBarHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Lua::PushInt(l,hPanel.get<::WIScrollBar>()->GetScrollOffset());
}
void Lua::WIScrollBar::SetScrollOffset(lua_State *l,WIScrollBarHandle &hPanel,int offset)
{
	lua_checkgui(l,hPanel);
	hPanel.get<::WIScrollBar>()->SetScrollOffset(offset);
}
void Lua::WIScrollBar::AddScrollOffset(lua_State *l,WIScrollBarHandle &hPanel,int offset)
{
	lua_checkgui(l,hPanel);
	hPanel.get<::WIScrollBar>()->AddScrollOffset(offset);
}
void Lua::WIScrollBar::SetUp(lua_State *l,WIScrollBarHandle &hPanel,int numElementsListed,int numElementsTotal)
{
	lua_checkgui(l,hPanel);
	hPanel.get<::WIScrollBar>()->SetUp(numElementsListed,numElementsTotal);
}
void Lua::WIScrollBar::SetHorizontal(lua_State *l,WIScrollBarHandle &hPanel,bool b)
{
	lua_checkgui(l,hPanel);
	hPanel.get<::WIScrollBar>()->SetHorizontal(b);
}
void Lua::WIScrollBar::IsHorizontal(lua_State *l,WIScrollBarHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Lua::PushBool(l,hPanel.get<::WIScrollBar>()->IsHorizontal());
}
void Lua::WIScrollBar::IsVertical(lua_State *l,WIScrollBarHandle &hPanel)
{
	lua_checkgui(l,hPanel);
	Lua::PushBool(l,hPanel.get<::WIScrollBar>()->IsVertical());
}

////////////////////////////////////

void Lua::WIIcon::SetClipping(lua_State *l,WIIconHandle &hIcon,uint32_t xStart,uint32_t yStart,uint32_t width,uint32_t height)
{
	lua_checkgui(l,hIcon);
	hIcon.get<::WIIcon>()->SetClipping(xStart,yStart,width,height);
}

////////////////////////////////////

void Lua::WISilkIcon::SetIcon(lua_State *l,WISilkIconHandle &hIcon,std::string icon)
{
	lua_checkgui(l,hIcon);
	hIcon.get<::WISilkIcon>()->SetIcon(icon);
}

////////////////////////////////////

void Lua::WIDropDownMenu::SelectOptionByText(lua_State *l,WIDropDownMenuHandle &hDm,const std::string &text)
{
	lua_checkgui(l,hDm);
	static_cast<::WIDropDownMenu*>(hDm.get())->SelectOptionByText(text);
}
void Lua::WIDropDownMenu::ClearOptions(lua_State *l,WIDropDownMenuHandle &hDm)
{
	lua_checkgui(l,hDm);
	static_cast<::WIDropDownMenu*>(hDm.get())->ClearOptions();
}
void Lua::WIDropDownMenu::GetOptionText(lua_State *l,WIDropDownMenuHandle &hDm,uint32_t idx)
{
	lua_checkgui(l,hDm);
	Lua::PushString(l,std::string{static_cast<::WIDropDownMenu*>(hDm.get())->GetOptionText(idx)});
}
void Lua::WIDropDownMenu::GetOptionValue(lua_State *l,WIDropDownMenuHandle &hDm,uint32_t idx)
{
	lua_checkgui(l,hDm);
	Lua::PushString(l,static_cast<::WIDropDownMenu*>(hDm.get())->GetOptionValue(idx));
}
void Lua::WIDropDownMenu::SetOptionText(lua_State *l,WIDropDownMenuHandle &hDm,uint32_t idx,const std::string &text)
{
	lua_checkgui(l,hDm);
	static_cast<::WIDropDownMenu*>(hDm.get())->SetOptionText(idx,text);
}
void Lua::WIDropDownMenu::SetOptionValue(lua_State *l,WIDropDownMenuHandle &hDm,uint32_t idx,const std::string &val)
{
	lua_checkgui(l,hDm);
	static_cast<::WIDropDownMenu*>(hDm.get())->SetOptionValue(idx,val);
}
void Lua::WIDropDownMenu::SelectOption(lua_State *l,WIDropDownMenuHandle &hDm,unsigned int idx)
{
	lua_checkgui(l,hDm);
	auto *pMenu = hDm.get<::WIDropDownMenu>();
	pMenu->SelectOption(idx);
}
void Lua::WIDropDownMenu::SelectOption(lua_State *l,WIDropDownMenuHandle &hDm,const std::string &option)
{
	lua_checkgui(l,hDm);
	auto *pMenu = hDm.get<::WIDropDownMenu>();
	pMenu->SelectOption(option);
}
void Lua::WIDropDownMenu::GetText(lua_State *l,WIDropDownMenuHandle &hDm)
{
	lua_checkgui(l,hDm);
	auto *pMenu = hDm.get<::WIDropDownMenu>();
	Lua::PushString(l,std::string{pMenu->GetText()});
}
void Lua::WIDropDownMenu::GetValue(lua_State *l,WIDropDownMenuHandle &hDm)
{
	lua_checkgui(l,hDm);
	Lua::PushString(l,static_cast<::WIDropDownMenu*>(hDm.get())->GetValue());
}
void Lua::WIDropDownMenu::SetText(lua_State *l,WIDropDownMenuHandle &hDm,std::string text)
{
	lua_checkgui(l,hDm);
	auto *pMenu = hDm.get<::WIDropDownMenu>();
	pMenu->SetText(text);
}
void Lua::WIDropDownMenu::GetOptionCount(lua_State *l,WIDropDownMenuHandle &hDm)
{
	lua_checkgui(l,hDm);
	auto *pMenu = hDm.get<::WIDropDownMenu>();
	Lua::PushInt(l,pMenu->GetOptionCount());
}
void Lua::WIDropDownMenu::AddOption(lua_State *l,WIDropDownMenuHandle &hDm,std::string option)
{
	lua_checkgui(l,hDm);
	auto *pMenu = hDm.get<::WIDropDownMenu>();
	auto *el = pMenu->AddOption(option);
	if(el == nullptr)
		return;
	auto o = WGUILuaInterface::GetLuaObject(l,*el);
	o.push(l);
}
void Lua::WIDropDownMenu::AddOption(lua_State *l,WIDropDownMenuHandle &hDm,std::string option,const std::string &optionValue)
{
	lua_checkgui(l,hDm);
	auto *pMenu = hDm.get<::WIDropDownMenu>();
	auto *el = pMenu->AddOption(option,optionValue);
	if(el == nullptr)
		return;
	auto o = WGUILuaInterface::GetLuaObject(l,*el);
	o.push(l);
}
void Lua::WIDropDownMenu::OpenMenu(lua_State *l,WIDropDownMenuHandle &hDm)
{
	lua_checkgui(l,hDm);
	auto *pMenu = hDm.get<::WIDropDownMenu>();
	pMenu->OpenMenu();
}
void Lua::WIDropDownMenu::CloseMenu(lua_State *l,WIDropDownMenuHandle &hDm)
{
	lua_checkgui(l,hDm);
	auto *pMenu = hDm.get<::WIDropDownMenu>();
	pMenu->CloseMenu();
}
void Lua::WIDropDownMenu::ToggleMenu(lua_State *l,WIDropDownMenuHandle &hDm)
{
	lua_checkgui(l,hDm);
	auto *pMenu = hDm.get<::WIDropDownMenu>();
	pMenu->ToggleMenu();
}
void Lua::WIDropDownMenu::IsMenuOpen(lua_State *l,WIDropDownMenuHandle &hDm)
{
	lua_checkgui(l,hDm);
	auto *pMenu = hDm.get<::WIDropDownMenu>();
	Lua::PushBool(l,pMenu->IsMenuOpen());
}

////////////////////////////////////

void Lua::WIArrow::SetDirection(lua_State *l,WIArrowHandle &hArrow,uint32_t dir)
{
	lua_checkgui(l,hArrow);
	static_cast<::WIArrow*>(hArrow.get())->SetDirection(static_cast<::WIArrow::Direction>(dir));
}

////////////////////////////////////

void Lua::WICheckbox::SetChecked(lua_State *l,WICheckboxHandle &hCheckBox,bool bChecked)
{
	lua_checkgui(l,hCheckBox);
	auto *pCheckBox = hCheckBox.get<::WICheckbox>();
	pCheckBox->SetChecked(bChecked);
}
void Lua::WICheckbox::IsChecked(lua_State *l,WICheckboxHandle &hCheckBox)
{
	lua_checkgui(l,hCheckBox);
	auto *pCheckBox = hCheckBox.get<::WICheckbox>();
	Lua::PushBool(l,pCheckBox->IsChecked());
}
void Lua::WICheckbox::Toggle(lua_State *l,WICheckboxHandle &hCheckBox)
{
	lua_checkgui(l,hCheckBox);
	auto *pCheckBox = hCheckBox.get<::WICheckbox>();
	pCheckBox->Toggle();
}

////////////////////////////////////

void Lua::WITransformable::register_class(luabind::class_<WITransformableHandle,WIHandle> &classDef)
{
	classDef.def("SetDraggable",&SetDraggable);
	classDef.def("SetResizable",&SetResizable);
	classDef.def("IsDraggable",&IsDraggable);
	classDef.def("IsResizable",&IsResizable);
	classDef.def("SetMinWidth",&SetMinWidth);
	classDef.def("SetMinHeight",&SetMinHeight);
	classDef.def("SetMinSize",static_cast<void(*)(lua_State*,WITransformableHandle&,int32_t,int32_t)>(&SetMinSize));
	classDef.def("SetMinSize",static_cast<void(*)(lua_State*,WITransformableHandle&,const Vector2i&)>(&SetMinSize));
	classDef.def("GetMinWidth",&GetMinWidth);
	classDef.def("GetMinHeight",&GetMinHeight);
	classDef.def("GetMinSize",&GetMinSize);
	classDef.def("SetMaxWidth",&SetMaxWidth);
	classDef.def("SetMaxHeight",&SetMaxHeight);
	classDef.def("SetMaxSize",static_cast<void(*)(lua_State*,WITransformableHandle&,int32_t,int32_t)>(&SetMaxSize));
	classDef.def("SetMaxSize",static_cast<void(*)(lua_State*,WITransformableHandle&,const Vector2i&)>(&SetMaxSize));
	classDef.def("GetMaxWidth",&GetMaxWidth);
	classDef.def("GetMaxHeight",&GetMaxHeight);
	classDef.def("GetMaxSize",&GetMaxSize);
	classDef.def("Close",&Close);
	classDef.def("GetDragArea",&GetDragArea);
	classDef.def("IsBeingDragged",static_cast<void(*)(lua_State*,WITransformableHandle&)>([](lua_State *l,WITransformableHandle &hTransformable) {
		lua_checkgui(l,hTransformable);
		Lua::PushBool(l,static_cast<::WITransformable*>(hTransformable.get())->IsBeingDragged());
	}));
	classDef.def("IsBeingResized",static_cast<void(*)(lua_State*,WITransformableHandle&)>([](lua_State *l,WITransformableHandle &hTransformable) {
		lua_checkgui(l,hTransformable);
		Lua::PushBool(l,static_cast<::WITransformable*>(hTransformable.get())->IsBeingResized());
	}));
	classDef.def("SetDragBounds",static_cast<void(*)(lua_State*,WITransformableHandle&,const Vector2i&,const Vector2i&)>([](lua_State *l,WITransformableHandle &hTransformable,const Vector2i &min,const Vector2i &max) {
		lua_checkgui(l,hTransformable);
		static_cast<::WITransformable*>(hTransformable.get())->SetDragBounds(min,max);
	}));
	classDef.def("GetDragBounds",static_cast<void(*)(lua_State*,WITransformableHandle&)>([](lua_State *l,WITransformableHandle &hTransformable) {
		lua_checkgui(l,hTransformable);
		auto bounds = static_cast<::WITransformable*>(hTransformable.get())->GetDragBounds();
		Lua::Push<Vector2i>(l,bounds.first);
		Lua::Push<Vector2i>(l,bounds.second);
	}));
	classDef.def("SetResizeRatioLocked",static_cast<void(*)(lua_State*,WITransformableHandle&,bool)>([](lua_State *l,WITransformableHandle &hTransformable,bool bResizeRatioLocked) {
		lua_checkgui(l,hTransformable);
		static_cast<::WITransformable*>(hTransformable.get())->SetResizeRatioLocked(bResizeRatioLocked);
	}));
	classDef.def("IsResizeRatioLocked",static_cast<void(*)(lua_State*,WITransformableHandle&)>([](lua_State *l,WITransformableHandle &hTransformable) {
		lua_checkgui(l,hTransformable);
		Lua::PushBool(l,static_cast<::WITransformable*>(hTransformable.get())->IsResizeRatioLocked());
	}));
	classDef.def("AddSnapTarget",static_cast<void(*)(lua_State*,WITransformableHandle&,WISnapAreaHandle&)>([](lua_State *l,WITransformableHandle &hTransformable,WISnapAreaHandle &hSnapTarget) {
		lua_checkgui(l,hTransformable);
		lua_checkgui(l,hSnapTarget);
		auto *pSnapTarget = static_cast<WISnapArea*>(hSnapTarget.get());
		if(pSnapTarget == nullptr)
			return;
		static_cast<::WITransformable*>(hTransformable.get())->AddSnapTarget(*pSnapTarget);
	}));
}

void Lua::WITransformable::SetDraggable(lua_State *l,WITransformableHandle &hTransformable,bool b)
{
	lua_checkgui(l,hTransformable);
	hTransformable.get<::WITransformable>()->SetDraggable(b);
}
void Lua::WITransformable::SetResizable(lua_State *l,WITransformableHandle &hTransformable,bool b)
{
	lua_checkgui(l,hTransformable);
	hTransformable.get<::WITransformable>()->SetResizable(b);
}
void Lua::WITransformable::IsDraggable(lua_State *l,WITransformableHandle &hTransformable)
{
	lua_checkgui(l,hTransformable);
	Lua::PushBool(l,hTransformable.get<::WITransformable>()->IsDraggable());
}
void Lua::WITransformable::IsResizable(lua_State *l,WITransformableHandle &hTransformable)
{
	lua_checkgui(l,hTransformable);
	Lua::PushBool(l,hTransformable.get<::WITransformable>()->IsResizable());
}
void Lua::WITransformable::SetMinWidth(lua_State *l,WITransformableHandle &hTransformable,int32_t width)
{
	lua_checkgui(l,hTransformable);
	hTransformable.get<::WITransformable>()->SetMinWidth(width);
}
void Lua::WITransformable::SetMinHeight(lua_State *l,WITransformableHandle &hTransformable,int32_t height)
{
	lua_checkgui(l,hTransformable);
	hTransformable.get<::WITransformable>()->SetMinHeight(height);
}
void Lua::WITransformable::SetMinSize(lua_State *l,WITransformableHandle &hTransformable,int32_t width,int32_t height)
{
	lua_checkgui(l,hTransformable);
	hTransformable.get<::WITransformable>()->SetMinSize(width,height);
}
void Lua::WITransformable::SetMinSize(lua_State *l,WITransformableHandle &hTransformable,const Vector2i &size)
{
	lua_checkgui(l,hTransformable);
	hTransformable.get<::WITransformable>()->SetMinSize(size);
}
void Lua::WITransformable::GetMinWidth(lua_State *l,WITransformableHandle &hTransformable)
{
	lua_checkgui(l,hTransformable);
	Lua::PushInt(l,hTransformable.get<::WITransformable>()->GetMinWidth());
}
void Lua::WITransformable::GetMinHeight(lua_State *l,WITransformableHandle &hTransformable)
{
	lua_checkgui(l,hTransformable);
	Lua::PushInt(l,hTransformable.get<::WITransformable>()->GetMinHeight());
}
void Lua::WITransformable::GetMinSize(lua_State *l,WITransformableHandle &hTransformable)
{
	lua_checkgui(l,hTransformable);
	Lua::Push<Vector2i>(l,hTransformable.get<::WITransformable>()->GetMinSize());
}
void Lua::WITransformable::SetMaxWidth(lua_State *l,WITransformableHandle &hTransformable,int32_t width)
{
	lua_checkgui(l,hTransformable);
	hTransformable.get<::WITransformable>()->SetMaxWidth(width);
}
void Lua::WITransformable::SetMaxHeight(lua_State *l,WITransformableHandle &hTransformable,int32_t height)
{
	lua_checkgui(l,hTransformable);
	hTransformable.get<::WITransformable>()->SetMaxHeight(height);
}
void Lua::WITransformable::SetMaxSize(lua_State *l,WITransformableHandle &hTransformable,int32_t width,int32_t height)
{
	lua_checkgui(l,hTransformable);
	hTransformable.get<::WITransformable>()->SetMaxSize(width,height);
}
void Lua::WITransformable::SetMaxSize(lua_State *l,WITransformableHandle &hTransformable,const Vector2i &size)
{
	lua_checkgui(l,hTransformable);
	hTransformable.get<::WITransformable>()->SetMaxSize(size);
}
void Lua::WITransformable::GetMaxWidth(lua_State *l,WITransformableHandle &hTransformable)
{
	lua_checkgui(l,hTransformable);
	Lua::PushInt(l,hTransformable.get<::WITransformable>()->GetMaxWidth());
}
void Lua::WITransformable::GetMaxHeight(lua_State *l,WITransformableHandle &hTransformable)
{
	lua_checkgui(l,hTransformable);
	Lua::PushInt(l,hTransformable.get<::WITransformable>()->GetMaxHeight());
}
void Lua::WITransformable::GetMaxSize(lua_State *l,WITransformableHandle &hTransformable)
{
	lua_checkgui(l,hTransformable);
	Lua::Push<Vector2i>(l,hTransformable.get<::WITransformable>()->GetMaxSize());
}
void Lua::WITransformable::Close(lua_State *l,WITransformableHandle &hTransformable)
{
	lua_checkgui(l,hTransformable);
	hTransformable.get<::WITransformable>()->Close();
}
void Lua::WITransformable::GetDragArea(lua_State *l,WITransformableHandle &hTransformable)
{
	lua_checkgui(l,hTransformable);
	auto *r = hTransformable.get<::WITransformable>()->GetDragArea();
	if(r == nullptr)
		return;
	auto o = WGUILuaInterface::GetLuaObject(l,*r);
	o.push(l);
}

////////////////////////////////////

void Lua::WIGridPanel::AddItem(lua_State *l,WIGridPanelHandle &hGridPanel,WIHandle &hEl,uint32_t row,uint32_t col)
{
	lua_checkgui(l,hGridPanel);
	lua_checkgui(l,hEl);
	auto *pEl = static_cast<::WIGridPanel*>(hGridPanel.get())->AddItem(hEl.get(),row,col);
	if(pEl != nullptr)
	{
		auto o = WGUILuaInterface::GetLuaObject(l,*pEl);
		o.push(l);
	}
}

void Lua::WIGridPanel::GetColumnCount(lua_State *l,WIGridPanelHandle &hGridPanel)
{
	lua_checkgui(l,hGridPanel);
	Lua::PushInt(l,static_cast<::WIGridPanel*>(hGridPanel.get())->GetColumnCount());
}

////////////////////////////////////

void Lua::WITreeList::ExpandAll(lua_State *l,WITreeListHandle &hTreeList)
{
	lua_checkgui(l,hTreeList);
	static_cast<::WITreeList*>(hTreeList.get())->ExpandAll();
}
void Lua::WITreeList::CollapseAll(lua_State *l,WITreeListHandle &hTreeList)
{
	lua_checkgui(l,hTreeList);
	static_cast<::WITreeList*>(hTreeList.get())->CollapseAll();
}
void Lua::WITreeList::GetRootItem(lua_State *l,WITreeListHandle &hTreeList)
{
	lua_checkgui(l,hTreeList);
	auto *pRoot = static_cast<::WITreeList*>(hTreeList.get())->GetRootItem();
	if(pRoot == nullptr)
		return;
	auto o = WGUILuaInterface::GetLuaObject(l,*pRoot);
	o.push(l);
}

////////////////////////////////////

void Lua::WITreeListElement::AddItem(lua_State *l,WITreeListElementHandle &hElement,const std::string &text)
{
	lua_checkgui(l,hElement);
	auto *pEl = static_cast<::WITreeListElement*>(hElement.get())->AddItem(text);
	if(pEl != nullptr)
	{
		auto o = WGUILuaInterface::GetLuaObject(l,*pEl);
		o.push(l);
	}
}
void Lua::WITreeListElement::Expand(lua_State *l,WITreeListElementHandle &hElement,bool bAll)
{
	lua_checkgui(l,hElement);
	static_cast<::WITreeListElement*>(hElement.get())->Expand(bAll);
}
void Lua::WITreeListElement::Expand(lua_State *l,WITreeListElementHandle &hElement)
{
	lua_checkgui(l,hElement);
	static_cast<::WITreeListElement*>(hElement.get())->Expand();
}
void Lua::WITreeListElement::Collapse(lua_State *l,WITreeListElementHandle &hElement,bool bAll)
{
	lua_checkgui(l,hElement);
	static_cast<::WITreeListElement*>(hElement.get())->Collapse(bAll);
}
void Lua::WITreeListElement::Collapse(lua_State *l,WITreeListElementHandle &hElement)
{
	lua_checkgui(l,hElement);
	static_cast<::WITreeListElement*>(hElement.get())->Expand();
}
void Lua::WITreeListElement::GetItems(lua_State *l,WITreeListElementHandle &hElement)
{
	lua_checkgui(l,hElement);
	auto t = Lua::CreateTable(l);
	auto &items = static_cast<::WITreeListElement*>(hElement.get())->GetItems();
	int32_t idx = 1;
	for(auto i=decltype(items.size()){0};i<items.size();++i)
	{
		auto &hItem = items[i];
		if(hItem.IsValid() == true)
		{
			Lua::PushInt(l,idx++);
			auto o = WGUILuaInterface::GetLuaObject(l,*hItem.get());
			o.push(l);

			Lua::SetTableValue(l,t);
		}
	}
}

////////////////////////////////////

void Lua::WIContainer::SetPadding(lua_State *l,WIContainerHandle &hContainer,int32_t padding)
{
	lua_checkgui(l,hContainer);
	static_cast<::WIContainer*>(hContainer.get())->SetPadding(padding);
}
void Lua::WIContainer::SetPadding(lua_State *l,WIContainerHandle &hContainer,int32_t top,int32_t right,int32_t bottom,int32_t left)
{
	lua_checkgui(l,hContainer);
	static_cast<::WIContainer*>(hContainer.get())->SetPadding(top,right,bottom,left);
}
void Lua::WIContainer::SetPaddingTop(lua_State *l,WIContainerHandle &hContainer,int32_t top)
{
	lua_checkgui(l,hContainer);
	static_cast<::WIContainer*>(hContainer.get())->SetPaddingTop(top);
}
void Lua::WIContainer::SetPaddingRight(lua_State *l,WIContainerHandle &hContainer,int32_t right)
{
	lua_checkgui(l,hContainer);
	static_cast<::WIContainer*>(hContainer.get())->SetPaddingRight(right);
}
void Lua::WIContainer::SetPaddingBottom(lua_State *l,WIContainerHandle &hContainer,int32_t bottom)
{
	lua_checkgui(l,hContainer);
	static_cast<::WIContainer*>(hContainer.get())->SetPaddingBottom(bottom);
}
void Lua::WIContainer::SetPaddingLeft(lua_State *l,WIContainerHandle &hContainer,int32_t left)
{
	lua_checkgui(l,hContainer);
	static_cast<::WIContainer*>(hContainer.get())->SetPaddingLeft(left);
}
void Lua::WIContainer::GetPadding(lua_State *l,WIContainerHandle &hContainer)
{
	lua_checkgui(l,hContainer);
	auto &padding = static_cast<::WIContainer*>(hContainer.get())->GetPadding();
	for(auto p : padding)
		Lua::PushInt(l,p);
}
void Lua::WIContainer::GetPaddingTop(lua_State *l,WIContainerHandle &hContainer)
{
	lua_checkgui(l,hContainer);
	Lua::PushInt(l,static_cast<::WIContainer*>(hContainer.get())->GetPaddingTop());
}
void Lua::WIContainer::GetPaddingRight(lua_State *l,WIContainerHandle &hContainer)
{
	lua_checkgui(l,hContainer);
	Lua::PushInt(l,static_cast<::WIContainer*>(hContainer.get())->GetPaddingRight());
}
void Lua::WIContainer::GetPaddingBottom(lua_State *l,WIContainerHandle &hContainer)
{
	lua_checkgui(l,hContainer);
	Lua::PushInt(l,static_cast<::WIContainer*>(hContainer.get())->GetPaddingBottom());
}
void Lua::WIContainer::GetPaddingLeft(lua_State *l,WIContainerHandle &hContainer)
{
	lua_checkgui(l,hContainer);
	Lua::PushInt(l,static_cast<::WIContainer*>(hContainer.get())->GetPaddingLeft());
}

////////////////////////////////////

void Lua::WITable::SetRowHeight(lua_State *l,WITableHandle &hTable,int height)
{
	lua_checkgui(l,hTable);
	auto *t = hTable.get<::WITable>();
	t->SetRowHeight(height);
}
void Lua::WITable::SetSelectable(lua_State *l,WITableHandle &hTable,uint32_t mode)
{
	lua_checkgui(l,hTable);
	auto *t = hTable.get<::WITable>();
	t->SetSelectable(static_cast<::WITable::SelectableMode>(mode));
}
void Lua::WITable::IsSelectable(lua_State *l,WITableHandle &hTable)
{
	lua_checkgui(l,hTable);
	auto *t = hTable.get<::WITable>();
	Lua::PushInt(l,umath::to_integral(t->GetSelectableMode()));
}

void Lua::WITable::SetColumnWidth(lua_State *l,WITableHandle &hTable,uint32_t colId,uint32_t width)
{
	lua_checkgui(l,hTable);
	auto *t = static_cast<::WITable*>(hTable.get());
	t->SetColumnWidth(colId,width);
}
void Lua::WITable::AddRow(lua_State *l,WITableHandle &hTable)
{
	lua_checkgui(l,hTable);
	auto *t = static_cast<::WITable*>(hTable.get());
	auto *row = t->AddRow();
	if(row != nullptr)
	{
		auto oRow = WGUILuaInterface::GetLuaObject(l,*row);
		oRow.push(l);
	}
}
void Lua::WITable::AddHeaderRow(lua_State *l,WITableHandle &hTable)
{
	lua_checkgui(l,hTable);
	auto *t = static_cast<::WITable*>(hTable.get());
	auto *row = t->AddHeaderRow();
	if(row != nullptr)
	{
		auto oRow = WGUILuaInterface::GetLuaObject(l,*row);
		oRow.push(l);
	}
}
void Lua::WITable::GetRowCount(lua_State *l,WITableHandle &hTable)
{
	lua_checkgui(l,hTable);
	auto *t = static_cast<::WITable*>(hTable.get());
	Lua::PushInt(l,t->GetRowCount());
}
void Lua::WITable::SetSortable(lua_State *l,WITableHandle &hTable,bool b)
{
	lua_checkgui(l,hTable);
	auto *t = static_cast<::WITable*>(hTable.get());
	t->SetSortable(b);
}
void Lua::WITable::IsSortable(lua_State *l,WITableHandle &hTable)
{
	lua_checkgui(l,hTable);
	auto *t = static_cast<::WITable*>(hTable.get());
	Lua::PushBool(l,t->IsSortable());
}
void Lua::WITable::SetScrollable(lua_State *l,WITableHandle &hTable,bool b)
{
	lua_checkgui(l,hTable);
	auto *t = static_cast<::WITable*>(hTable.get());
	t->SetScrollable(b);
}
void Lua::WITable::IsScrollable(lua_State *l,WITableHandle &hTable)
{
	lua_checkgui(l,hTable);
	auto *t = static_cast<::WITable*>(hTable.get());
	Lua::PushBool(l,t->IsScrollable());
}
void Lua::WITable::Clear(lua_State *l,WITableHandle &hTable)
{
	lua_checkgui(l,hTable);
	auto *t = static_cast<::WITable*>(hTable.get());
	t->Clear();
}
void Lua::WITable::GetRow(lua_State *l,WITableHandle &hTable,uint32_t rowId)
{
	lua_checkgui(l,hTable);
	auto *t = static_cast<::WITable*>(hTable.get());
	auto *pRow = t->GetRow(rowId);
	if(pRow == nullptr)
		return;
	auto oRow = WGUILuaInterface::GetLuaObject(l,*pRow);
	oRow.push(l);
}
void Lua::WITable::GetSelectedRows(lua_State *l,WITableHandle &hTable)
{
	lua_checkgui(l,hTable);
	auto *tb = static_cast<::WITable*>(hTable.get());
	auto &rows = tb->GetSelectedRows();
	auto t = Lua::CreateTable(l);
	int32_t idx = 1;
	for(auto &hRow : rows)
	{
		if(hRow.IsValid() == false)
			continue;
		Lua::PushInt(l,idx++);
		auto oRow = WGUILuaInterface::GetLuaObject(l,*hRow.get());
		oRow.push(l);
		Lua::SetTableValue(l,t);
	}
}
void Lua::WITable::GetRows(lua_State *l,WITableHandle &hTable)
{
	lua_checkgui(l,hTable);

	auto *pTable = static_cast<::WITable*>(hTable.get());
	auto t = Lua::CreateTable(l);
	auto numRows = pTable->GetRowCount();
	uint32_t rowId = 1;
	for(auto i=decltype(numRows){0};i<numRows;++i)
	{
		auto *pRow = pTable->GetRow(i);
		if(pRow != nullptr)
		{
			Lua::PushInt(l,rowId++);
			auto o = WGUILuaInterface::GetLuaObject(l,*pRow);
			o.push(l);

			Lua::SetTableValue(l,t);
		};
	}
}

void Lua::WITable::RemoveRow(lua_State *l,WITableHandle &hTable,uint32_t rowIdx)
{
	lua_checkgui(l,hTable);
	static_cast<::WITable*>(hTable.get())->RemoveRow(rowIdx);
}

void Lua::WITableRow::Select(lua_State *l,WITableRowHandle &hRow)
{
	lua_checkgui(l,hRow);
	auto *t = hRow.get<::WITableRow>();
	t->Select();
}

void Lua::WITableRow::Deselect(lua_State *l,WITableRowHandle &hRow)
{
	lua_checkgui(l,hRow);
	auto *t = hRow.get<::WITableRow>();
	t->Deselect();
}

void Lua::WITableRow::IsSelected(lua_State *l,WITableRowHandle &hRow)
{
	lua_checkgui(l,hRow);
	auto *t = hRow.get<::WITableRow>();
	Lua::PushBool(l,t->IsSelected());
}

void Lua::WITableRow::SetCellWidth(lua_State *l,WITableRowHandle &hRow,uint32_t column,uint32_t width)
{
	lua_checkgui(l,hRow);
	auto *t = static_cast<::WITableRow*>(hRow.get());
	t->SetCellWidth(column,width);
}
void Lua::WITableRow::SetValue(lua_State *l,WITableRowHandle &hRow,uint32_t column,const std::string &value)
{
	lua_checkgui(l,hRow);
	auto *t = static_cast<::WITableRow*>(hRow.get());
	auto hText = t->SetValue(column,value);
	if(!hText.IsValid())
		return;
	auto oText = WGUILuaInterface::GetLuaObject(l,*hText.get());
	oText.push(l);
}
void Lua::WITableRow::GetValue(lua_State *l,WITableRowHandle &hRow,uint32_t column)
{
	lua_checkgui(l,hRow);
	auto *t = static_cast<::WITableRow*>(hRow.get());
	Lua::PushString(l,t->GetValue(column));
}
void Lua::WITableRow::InsertElement(lua_State *l,WITableRowHandle &hRow,uint32_t column,WIHandle hElement)
{
	lua_checkgui(l,hRow);
	lua_checkgui(l,hElement);
	auto *t = static_cast<::WITableRow*>(hRow.get());
	t->InsertElement(column,hElement);
}
void Lua::WITableRow::GetCellCount(lua_State *l,WITableRowHandle &hRow)
{
	lua_checkgui(l,hRow);
	auto *t = static_cast<::WITableRow*>(hRow.get());
	Lua::PushInt(l,t->GetCellCount());
}
void Lua::WITableRow::GetCell(lua_State *l,WITableRowHandle &hRow,uint32_t cellId)
{
	lua_checkgui(l,hRow);
	auto *t = static_cast<::WITableRow*>(hRow.get());
	auto *pCell = t->GetCell(cellId);
	if(pCell == nullptr)
		return;
	auto oCell = WGUILuaInterface::GetLuaObject(l,*pCell);
	oCell.push(l);
}

void Lua::WITableCell::GetFirstElement(lua_State *l,WITableCellHandle &hCell)
{
	lua_checkgui(l,hCell);
	auto *c = static_cast<::WITableCell*>(hCell.get());
	auto *el = c->GetFirstElement();
	if(el == nullptr)
		return;
	auto oEl = WGUILuaInterface::GetLuaObject(l,*el);
	oEl.push(l);
}

void Lua::WITableCell::GetColSpan(lua_State *l,WITableCellHandle &hCell)
{
	lua_checkgui(l,hCell);
	Lua::PushInt(l,static_cast<::WITableCell*>(hCell.get())->GetColSpan());
}
void Lua::WITableCell::GetRowSpan(lua_State *l,WITableCellHandle &hCell)
{
	lua_checkgui(l,hCell);
	Lua::PushInt(l,static_cast<::WITableCell*>(hCell.get())->GetRowSpan());
}
void Lua::WITableCell::SetColSpan(lua_State *l,WITableCellHandle &hCell,int32_t colSpan)
{
	lua_checkgui(l,hCell);
	static_cast<::WITableCell*>(hCell.get())->SetColSpan(colSpan);
}
void Lua::WITableCell::SetRowSpan(lua_State *l,WITableCellHandle &hCell,int32_t rowSpan)
{
	lua_checkgui(l,hCell);
	static_cast<::WITableCell*>(hCell.get())->SetRowSpan(rowSpan);
}

////////////////////////////////////

void Lua::WIFrame::SetTitle(lua_State *l,WIFrameHandle &hFrame,std::string title)
{
	lua_checkgui(l,hFrame);
	hFrame.get<::WIFrame>()->SetTitle(title);
}
void Lua::WIFrame::GetTitle(lua_State *l,WIFrameHandle &hFrame)
{
	lua_checkgui(l,hFrame);
	Lua::PushString(l,hFrame.get<::WIFrame>()->GetTitle());
}

