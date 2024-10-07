/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/lua/classes/c_lwibase.h"
#include <wgui/wibase.h>
#include <wgui/wihandle.h>
#include <wgui/types/witooltip.h>
#include "luasystem.h"
#include <wgui/types/wirect.h>
#include <wgui/types/widropdownmenu.h>
#include <wgui/types/wiroot.h>
#include "pragma/lua/libraries/c_gui_callbacks.hpp"
#include "pragma/gui/wisilkicon.h"
#include "pragma/gui/wisnaparea.hpp"
#include "pragma/lua/classes/c_ldef_wgui.h"
#include "pragma/lua/libraries/c_lua_vulkan.h"
#include "pragma/lua/policies/gui_element_policy.hpp"
#include "pragma/lua/converters/gui_element_converter_t.hpp"
#include "pragma/lua/converters/shader_converter_t.hpp"
#include "pragma/lua/libraries/c_lgui.h"
#include "pragma/gui/wgui_luainterface.h"
#include <pragma/lua/classes/ldef_vector.h>
#include <pragma/lua/classes/lproperty.hpp>
#include <pragma/lua/policies/property_policy.hpp>
#include <pragma/lua/policies/optional_policy.hpp>
#include <pragma/lua/policies/vector_policy.hpp>
#include <pragma/lua/policies/string_view_policy.hpp>
#include <pragma/lua/policies/pair_policy.hpp>
#include <pragma/lua/policies/default_parameter_policy.hpp>
#include <pragma/lua/policies/shared_from_this_policy.hpp>
#include <pragma/lua/converters/pair_converter_t.hpp>
#include <pragma/lua/converters/string_view_converter_t.hpp>
#include <pragma/lua/converters/vector_converter_t.hpp>
#include <pragma/lua/converters/optional_converter_t.hpp>
#include <pragma/lua/converters/property_converter_t.hpp>
#include <pragma/lua/lua_call.hpp>
#include <prosper_command_buffer.hpp>
#include <prosper_render_pass.hpp>
#include <prosper_swap_command_buffer.hpp>
#include <shader/prosper_shader_copy_image.hpp>
#include <sharedutils/property/util_property_color.hpp>
#include <sharedutils/util_hash.hpp>
#include <luabind/out_value_policy.hpp>
#include <util_formatted_text.hpp>
#include <prosper_window.hpp>
#include <luabind/copy_policy.hpp>
#include <pragma/debug/intel_vtune.hpp>

import pragma.string.unicode;

extern DLLCLIENT CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

template<class TStream>
static TStream &print_ui_element(TStream &os, const ::WIBase &handle)
{
	const WIBase *p = &handle;
	auto pos = p->GetAbsolutePos();
	auto &size = p->GetSize();
	os << "WIElement[" << p->GetClass() << "]";
	os << "[" << p->GetName() << "]";
	os << "[" << p->GetIndex() << "]";
	os << "[" << &handle << "]";
	os << "[Pos:" << pos.x << ", " << pos.y << "]";
	os << "[Sz:" << size.x << ", " << size.y << "]";
	os << "[Vis:" << (p->IsVisible() ? "1" : "0") << "(" << (p->IsSelfVisible() ? "1" : "0") << ")]";

	auto *elText = dynamic_cast<const WIText *>(p);
	if(elText) {
		auto text = elText->GetText().cpp_str();
		if(text.length() > 10)
			text = text.substr(0, 10) + "...";
		os << "[" << text << "]";
	}
	else {
		auto *elTex = dynamic_cast<const WITexturedShape *>(p);
		if(elTex) {
			auto *mat = const_cast<WITexturedShape *>(elTex)->GetMaterial();
			if(mat)
				os << "[" << mat->GetName() << "]";
			else
				os << "[NULL]";
		}
	}
	return os;
}
DLLCLIENT Con::c_cout &operator<<(Con::c_cout &os, const ::WIBase &handle) { return print_ui_element<Con::c_cout>(os, handle); }
DLLCLIENT std::ostream &operator<<(std::ostream &os, const ::WIBase &handle) { return print_ui_element<std::ostream>(os, handle); }

extern DLLCLIENT ClientState *client;

static bool operator==(::WIBase &a, ::WIBase &b) { return &a == &b; }

static void record_render_ui(WIBase &el, prosper::IImage &img, const Lua::gui::DrawToTextureInfo &info, const std::shared_ptr<prosper::ICommandBuffer> &drawCmd)
{
	auto useMsaa = (img.GetSampleCount() > prosper::SampleCountFlags::e1Bit);
	if(info.clearColor) {
		auto clearCol = info.clearColor->ToVector4();
		drawCmd->RecordClearAttachment(img, std::array<float, 4> {clearCol[0], clearCol[1], clearCol[2], clearCol[3]});
	}

	WIBase::DrawInfo drawInfo {drawCmd};
	drawInfo.size = {el.GetWidth(), el.GetHeight()};
	umath::set_flag(drawInfo.flags, WIBase::DrawInfo::Flags::UseScissor, false);
	umath::set_flag(drawInfo.flags, WIBase::DrawInfo::Flags::UseStencil, info.useStencil);
	umath::set_flag(drawInfo.flags, WIBase::DrawInfo::Flags::Msaa, useMsaa);
	std::optional<Mat4> rotMat = el.GetRotationMatrix() ? *el.GetRotationMatrix() : std::optional<Mat4> {};
	if(rotMat.has_value()) {
		el.ResetRotation(); // We'll temporarily disable the rotation for this element
		umath::set_flag(drawInfo.flags, WIBase::DrawInfo::Flags::UseStencil, true);
	}
	wgui::DrawState drawState {};
	drawState.SetScissor(0, 0, drawInfo.size.x, drawInfo.size.y);
	el.Draw(drawInfo, drawState, {}, {}, drawInfo.size, el.GetScale());
	if(rotMat.has_value())
		el.SetRotation(*rotMat);
}
static bool render_ui(WIBase &el, prosper::RenderTarget &rt, const Lua::gui::DrawToTextureInfo &info)
{
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	debug::get_domain().BeginTask("draw_ui_to_texture");
	util::ScopeGuard sgVtune {[]() { debug::get_domain().EndTask(); }};
#endif
	auto &context = rt.GetContext();
	auto drawCmd = info.commandBuffer;
	if(!drawCmd)
		drawCmd = context.GetWindow().GetDrawCommandBuffer();
	else if(!drawCmd->IsPrimary())
		return false;
	auto &img = rt.GetTexture().GetImage();
	auto useMsaa = (img.GetSampleCount() > prosper::SampleCountFlags::e1Bit);
	if(useMsaa && !info.resolvedImage)
		return false;
	auto *texStencil = rt.GetTexture(1);
	if(!texStencil)
		return false;
	auto &imgStencil = texStencil->GetImage();

	std::vector<prosper::ClearValue> clearVals = {prosper::ClearValue {}, prosper::ClearValue {prosper::ClearDepthStencilValue {0.f, 0}}};

	drawCmd->GetPrimaryCommandBufferPtr()->RecordBeginRenderPass(rt, clearVals);
	record_render_ui(el, rt.GetTexture().GetImage(), info, drawCmd);
	drawCmd->GetPrimaryCommandBufferPtr()->RecordEndRenderPass();

	if(!useMsaa) {
		drawCmd->RecordImageBarrier(img, prosper::ImageLayout::ColorAttachmentOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal);
		return true;
	}

	drawCmd->RecordImageBarrier(img, prosper::ImageLayout::ColorAttachmentOptimal, prosper::ImageLayout::TransferSrcOptimal);
	drawCmd->RecordResolveImage(img, *info.resolvedImage);
	drawCmd->RecordImageBarrier(*info.resolvedImage, prosper::ImageLayout::TransferDstOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal);
	return true;
}

static void record_draw_ui(WIBase &el, Lua::Vulkan::CommandBufferRecorder &cmdBufGroup, prosper::IImage &img, const Lua::gui::DrawToTextureInfo &info)
{
	cmdBufGroup.Record([&el, &img, &info](prosper::ISecondaryCommandBuffer &drawCmd) mutable { record_render_ui(el, img, info, drawCmd.shared_from_this()); });
}

static std::shared_ptr<prosper::Texture> draw_to_texture(WIBase &el, const Lua::gui::DrawToTextureInfo &info)
{
	auto &context = c_engine->GetRenderContext();
	auto w = info.width.has_value() ? *info.width : el.GetWidth();
	auto h = info.height.has_value() ? *info.height : el.GetHeight();
	auto rt = Lua::gui::create_render_target(w, h, info.enableMsaa, !info.enableMsaa);
	if(!rt)
		return nullptr;
	auto tmpInfo = info;
	tmpInfo.clearColor = Color {0, 0, 0, 0};
	auto flushSetupCmd = false;
	if(!tmpInfo.commandBuffer) {
		tmpInfo.commandBuffer = context.GetSetupCommandBuffer();
		flushSetupCmd = true;
	}
	if(!info.enableMsaa) {
		auto res = render_ui(el, *rt, tmpInfo);
		if(flushSetupCmd)
			context.FlushSetupCommandBuffer();
		if(!res)
			return nullptr;
		return rt->GetTexture().shared_from_this();
	}
	auto imgDst = Lua::gui::create_color_image(w, h, prosper::ImageUsageFlags::SampledBit | prosper::ImageUsageFlags::TransferDstBit | prosper::ImageUsageFlags::TransferSrcBit, prosper::ImageLayout::TransferDstOptimal, false);
	tmpInfo.resolvedImage = imgDst;
	auto res = render_ui(el, *rt, tmpInfo);
	if(flushSetupCmd)
		context.FlushSetupCommandBuffer();
	if(!res)
		return nullptr;
	return context.CreateTexture({}, *imgDst, prosper::util::ImageViewCreateInfo {}, prosper::util::SamplerCreateInfo {});
}

static void clamp_to_parent_bounds(::WIBase &el, Vector2i &clampedPos, Vector2i &clamedSize)
{
	auto parent = el.GetParent();
	Vector2i pos = el.GetPos();
	Vector2i size = el.GetSize();
}

static void debug_print_hierarchy(const ::WIBase &el, const std::string &t = "")
{
	Con::cout << t << el << Con::endl;
	auto subT = t + "\t";
	for(auto &hChild : *const_cast<::WIBase &>(el).GetChildren()) {
		if(hChild.IsValid() == false)
			continue;
		debug_print_hierarchy(*hChild, subT);
	}
}

void Lua::WIBase::register_class(luabind::class_<::WIBase> &classDef)
{
	classDef.def(luabind::tostring(luabind::self));
	classDef.def(luabind::self == luabind::self);
	classDef.def("IsValid", &IsValid);
	classDef.def("GetIndex", &::WIBase::GetIndex);
	classDef.def("GetDepth", &::WIBase::GetDepth);
	classDef.def("SetSkinCallbacksEnabled", &::WIBase::SetSkinCallbacksEnabled);
	classDef.def("Remove", &::WIBase::Remove);
	classDef.def("RemoveSafely", &::WIBase::RemoveSafely);
	classDef.def("SetIgnoreParentAlpha", &::WIBase::SetIgnoreParentAlpha);
	classDef.def("ShouldIgnoreParentAlpha", &::WIBase::ShouldIgnoreParentAlpha);
	classDef.def("SetZPos", &::WIBase::SetZPos);
	classDef.def("GetZPos", &::WIBase::GetZPos);
	classDef.def("HasFocus", &::WIBase::HasFocus);
	classDef.def("RequestFocus", &::WIBase::RequestFocus);
	classDef.def(
	  "ClearFocus", +[](::WIBase &el) { return WGUI::GetInstance().ClearFocus(el); });
	classDef.def("KillFocus", &::WIBase::KillFocus);
	classDef.def("KillFocus", &::WIBase::KillFocus, luabind::default_parameter_policy<2, false> {});
	classDef.def("TrapFocus", static_cast<void (*)(lua_State *, ::WIBase &, bool)>(&TrapFocus));
	classDef.def("TrapFocus", static_cast<void (*)(lua_State *, ::WIBase &)>(&TrapFocus));
	classDef.def("IsFocusTrapped", &::WIBase::IsFocusTrapped);
	classDef.def("IsHidden", static_cast<bool (*)(lua_State *, ::WIBase &)>([](lua_State *l, ::WIBase &hPanel) { return !hPanel.IsVisible(); }));
	classDef.def("SetVisible", &::WIBase::SetVisible);
	classDef.def("IsVisible", &::WIBase::IsSelfVisible);
	classDef.def("IsParentVisible", &::WIBase::IsParentVisible);
	classDef.def("SetAutoSizeToContents", static_cast<void (*)(lua_State *, ::WIBase &)>([](lua_State *l, ::WIBase &hPanel) { hPanel.SetAutoSizeToContents(true); }));
	classDef.def("SetAutoSizeToContents", static_cast<void (*)(lua_State *, ::WIBase &, bool)>([](lua_State *l, ::WIBase &hPanel, bool autoSize) { hPanel.SetAutoSizeToContents(autoSize); }));
	classDef.def("SetAutoSizeToContents", static_cast<void (*)(lua_State *, ::WIBase &, bool, bool)>([](lua_State *l, ::WIBase &hPanel, bool x, bool y) { hPanel.SetAutoSizeToContents(x, y); }));
	classDef.def(
	  "SetAutoSizeToContents", +[](lua_State *l, ::WIBase &hPanel, bool x, bool y, bool updateImmediately) { hPanel.SetAutoSizeToContents(x, y, updateImmediately); });
	classDef.def(
	  "UpdateAutoSizeToContents", +[](lua_State *l, ::WIBase &hPanel) { hPanel.UpdateAutoSizeToContents(hPanel.ShouldAutoSizeToContentsX(), hPanel.ShouldAutoSizeToContentsY()); });
	classDef.def(
	  "FindAncestorByClass", +[](lua_State *l, ::WIBase &hPanel, std::string className) -> ::WIBase * {
		  ustring::to_lower(className);
		  auto *parent = hPanel.GetParent();
		  while(parent) {
			  if(parent->GetClass() == className)
				  return parent;
			  parent = parent->GetParent();
		  }
		  return nullptr;
	  });
	classDef.def("ShouldAutoSizeToContentsX", &::WIBase::ShouldAutoSizeToContentsX);
	classDef.def("ShouldAutoSizeToContentsY", &::WIBase::ShouldAutoSizeToContentsY);
	classDef.def("SetStencilEnabled", &::WIBase::SetStencilEnabled);
	classDef.def("IsStencilEnabled", &::WIBase::IsStencilEnabled);
	classDef.def("ResetRotation", &::WIBase::ResetRotation);
	classDef.def("GetRotationMatrix", &::WIBase::GetRotationMatrix, luabind::copy_policy<0> {});
	classDef.def("SetRotation", static_cast<void (::WIBase::*)(umath::Degree, const Vector2 &)>(&::WIBase::SetRotation));
	classDef.def("SetRotation", static_cast<void (::WIBase::*)(const Mat4 &)>(&::WIBase::SetRotation));
	classDef.def("SetLocalRenderTransform", &::WIBase::SetLocalRenderTransform);
	classDef.def("ClearLocalRenderTransform", &::WIBase::ClearLocalRenderTransform);
	classDef.def("GetLocalRenderTransform", static_cast<const umath::ScaledTransform *(::WIBase::*)() const>(&::WIBase::GetLocalRenderTransform), luabind::copy_policy<0> {});
	classDef.def("GetMouseInputEnabled", &::WIBase::GetMouseInputEnabled);
	classDef.def("SetMouseInputEnabled", &::WIBase::SetMouseInputEnabled);
	classDef.def("GetKeyboardInputEnabled", &::WIBase::GetKeyboardInputEnabled);
	classDef.def("SetKeyboardInputEnabled", &::WIBase::SetKeyboardInputEnabled);
	classDef.def("SetScrollInputEnabled", &::WIBase::SetScrollInputEnabled);
	classDef.def("GetScrollInputEnabled", &::WIBase::GetScrollInputEnabled);
	classDef.def("SetCursorMovementCheckEnabled", &::WIBase::SetMouseMovementCheckEnabled);
	classDef.def("GetCursorMovementCheckEnabled", &::WIBase::GetMouseMovementCheckEnabled);
	classDef.def("GetPos", static_cast<const Vector2i &(::WIBase::*)() const>(&::WIBase::GetPos), luabind::copy_policy<0> {});
	classDef.def("SetPos", static_cast<void (*)(lua_State *, ::WIBase &, Vector2)>(&SetPos));
	classDef.def("SetPos", static_cast<void (*)(lua_State *, ::WIBase &, float, float)>(&SetPos));
	classDef.def("GetAbsolutePos", static_cast<Vector2 (::WIBase::*)(bool) const>(&::WIBase::GetAbsolutePos));
	classDef.def("GetAbsolutePos", static_cast<Vector2 (::WIBase::*)(bool) const>(&::WIBase::GetAbsolutePos), luabind::default_parameter_policy<2, true> {});
	classDef.def("GetAbsolutePos", static_cast<Vector2 (::WIBase::*)(const Vector2 &, bool) const>(&::WIBase::GetAbsolutePos));
	classDef.def("GetAbsolutePos", static_cast<Vector2 (::WIBase::*)(const Vector2 &, bool) const>(&::WIBase::GetAbsolutePos), luabind::default_parameter_policy<3, true> {});
	classDef.def("SetAbsolutePos", static_cast<void (*)(lua_State *, ::WIBase &, Vector2)>(&SetAbsolutePos));
	classDef.def("SetAbsolutePos", static_cast<void (*)(lua_State *, ::WIBase &, float, float)>(&SetAbsolutePos));
	classDef.def("GetAbsolutePose", static_cast<Mat4 (::WIBase::*)() const>(&::WIBase::GetAbsolutePose));
	classDef.def("GetRelativePos", &::WIBase::GetRelativePos);
	classDef.def("GetColor", &::WIBase::GetColor, luabind::copy_policy<0> {});
	classDef.def("GetColorProperty", &::WIBase::GetColorProperty);
	classDef.def("GetFocusProperty", &::WIBase::GetFocusProperty);
	classDef.def("GetVisibilityProperty", &::WIBase::GetVisibilityProperty);
	classDef.def("GetPosProperty", &::WIBase::GetPosProperty);
	classDef.def("GetSizeProperty", &::WIBase::GetSizeProperty);
	classDef.def("GetMouseInBoundsProperty", &::WIBase::GetMouseInBoundsProperty);
	classDef.def("SetColor", &SetColor);
	classDef.def("SetColorRGB", static_cast<void (*)(lua_State *, ::WIBase &, const Color &)>([](lua_State *l, ::WIBase &hPanel, const Color &color) {
		auto vCol = color.ToVector4();
		vCol.a = hPanel.GetAlpha();
		hPanel.SetColor(vCol);
	}));
	classDef.def("GetAlpha", &GetAlpha);
	classDef.def("SetAlpha", &SetAlpha);
	classDef.def("GetWidth", &::WIBase::GetWidth);
	classDef.def("GetHeight", &::WIBase::GetHeight);
	classDef.def("GetSize", static_cast<const Vector2i &(::WIBase::*)() const>(&::WIBase::GetSize), luabind::copy_policy<0> {});
	classDef.def("SetSize", static_cast<void (*)(lua_State *, ::WIBase &, Vector2)>(&SetSize));
	classDef.def("SetSize", static_cast<void (*)(lua_State *, ::WIBase &, float, float)>(&SetSize));
	classDef.def("Wrap", static_cast<void (*)(lua_State *, ::WIBase &, const std::string &)>(&Wrap));
	classDef.def("Wrap", static_cast<bool (::WIBase::*)(::WIBase &)>(&::WIBase::Wrap));
	classDef.def("AnchorWithMargin", static_cast<void (::WIBase::*)(uint32_t, uint32_t, uint32_t, uint32_t)>(&::WIBase::AnchorWithMargin));
	classDef.def("AnchorWithMargin", static_cast<void (::WIBase::*)(uint32_t)>(&::WIBase::AnchorWithMargin));
	classDef.def("GetParent", &::WIBase::GetParent);
	classDef.def(
	  "SetParent", +[](lua_State *l, ::WIBase &hPanel, ::WIBase &hParent) { hPanel.SetParent(&hParent); });
	classDef.def(
	  "SetParent", +[](lua_State *l, ::WIBase &hPanel, ::WIBase &hParent, uint32_t index) { hPanel.SetParent(&hParent, index); });
	classDef.def(
	  "SetParentAndUpdateWindow", +[](lua_State *l, ::WIBase &hPanel, ::WIBase &hParent) { hPanel.SetParentAndUpdateWindow(&hParent); });
	classDef.def(
	  "SetParentAndUpdateWindow", +[](lua_State *l, ::WIBase &hPanel, ::WIBase &hParent, uint32_t index) { hPanel.SetParentAndUpdateWindow(&hParent, index); });
	classDef.def("ClearParent", &ClearParent);
	classDef.def("ResetParent", &ResetParent);
	classDef.def(
	  "GetChildren", +[](lua_State *l, ::WIBase &hPanel) -> luabind::tableT<::WIBase> {
		  auto &children = *hPanel.GetChildren();
		  auto t = luabind::newtable(l);
		  for(uint32_t idx = 1; auto &hChild : children) {
			  if(hChild.expired())
				  continue;
			  t[idx++] = hChild;
		  }
		  return t;
	  });
	classDef.def("GetChildren", static_cast<void (*)(lua_State *, ::WIBase &, std::string)>(&GetChildren));
	classDef.def("GetFirstChild", &::WIBase::GetFirstChild);
	classDef.def("GetChild", static_cast<void (*)(lua_State *, ::WIBase &, unsigned int)>(&GetChild));
	classDef.def("GetChild", static_cast<void (*)(lua_State *, ::WIBase &, std::string, unsigned int)>(&GetChild));
	classDef.def("IsPosInBounds", &PosInBounds);
	classDef.def("IsCursorInBounds", &::WIBase::MouseInBounds);
	classDef.def("GetCursorPos", &GetMousePos);
	classDef.def("Draw", static_cast<void (*)(lua_State *, ::WIBase &, const ::WIBase::DrawInfo &, wgui::DrawState &, const Vector2i &, const Vector2i &, const Vector2i &)>(&Draw));
	classDef.def("Draw", static_cast<void (*)(lua_State *, ::WIBase &, const ::WIBase::DrawInfo &, wgui::DrawState &, const Vector2i &, const Vector2i &)>(&Draw));
	classDef.def("Draw", static_cast<void (*)(lua_State *, ::WIBase &, const ::WIBase::DrawInfo &, wgui::DrawState &)>(&Draw));
	classDef.def("DrawToTexture", &render_ui);
	classDef.def(
	  "DrawToTexture", +[](::WIBase &el, prosper::RenderTarget &rt) { return render_ui(el, rt, {}); });
	classDef.def("DrawToTexture", &draw_to_texture);
	classDef.def(
	  "DrawToTexture", +[](::WIBase &el) { return draw_to_texture(el, {}); });
	classDef.def("RecordDraw", &record_draw_ui);
	classDef.def("GetX", &GetX);
	classDef.def("GetY", &GetY);
	classDef.def("SetX", &SetX);
	classDef.def("SetY", &SetY);
	classDef.def("SetWidth", &SetWidth);
	classDef.def("SetHeight", &SetHeight);
	classDef.def("SizeToContents", &::WIBase::SizeToContents);
	classDef.def("SizeToContents", &::WIBase::SizeToContents, luabind::default_parameter_policy<3, true> {});
	classDef.def("SizeToContents", &::WIBase::SizeToContents, luabind::meta::join<luabind::default_parameter_policy<2, true>, luabind::default_parameter_policy<3, true>>::type {});
	classDef.def("AddCallback", &AddCallback);
	classDef.def("CallCallbacks", static_cast<void (*)(lua_State *, ::WIBase &, std::string)>(&CallCallbacks));
	classDef.def("CallCallbacks", static_cast<void (*)(lua_State *, ::WIBase &, std::string, luabind::object)>(&CallCallbacks));
	classDef.def("CallCallbacks", static_cast<void (*)(lua_State *, ::WIBase &, std::string, luabind::object, luabind::object)>(&CallCallbacks));
	classDef.def("CallCallbacks", static_cast<void (*)(lua_State *, ::WIBase &, std::string, luabind::object, luabind::object, luabind::object)>(&CallCallbacks));
	classDef.def("CallCallbacks", static_cast<void (*)(lua_State *, ::WIBase &, std::string, luabind::object, luabind::object, luabind::object, luabind::object)>(&CallCallbacks));
	classDef.def("CallCallbacks", static_cast<void (*)(lua_State *, ::WIBase &, std::string, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object)>(&CallCallbacks));
	classDef.def("CallCallbacks", static_cast<void (*)(lua_State *, ::WIBase &, std::string, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object)>(&CallCallbacks));
	classDef.def("CallCallbacks", static_cast<void (*)(lua_State *, ::WIBase &, std::string, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object)>(&CallCallbacks));
	classDef.def("CallCallbacks", static_cast<void (*)(lua_State *, ::WIBase &, std::string, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object)>(&CallCallbacks));
	classDef.def("AddEventListener", &AddCallback);
	classDef.def("CallEventListeners", static_cast<void (*)(lua_State *, ::WIBase &, std::string)>(&CallCallbacks));
	classDef.def("CallEventListeners", static_cast<void (*)(lua_State *, ::WIBase &, std::string, luabind::object)>(&CallCallbacks));
	classDef.def("CallEventListeners", static_cast<void (*)(lua_State *, ::WIBase &, std::string, luabind::object, luabind::object)>(&CallCallbacks));
	classDef.def("CallEventListeners", static_cast<void (*)(lua_State *, ::WIBase &, std::string, luabind::object, luabind::object, luabind::object)>(&CallCallbacks));
	classDef.def("CallEventListeners", static_cast<void (*)(lua_State *, ::WIBase &, std::string, luabind::object, luabind::object, luabind::object, luabind::object)>(&CallCallbacks));
	classDef.def("CallEventListeners", static_cast<void (*)(lua_State *, ::WIBase &, std::string, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object)>(&CallCallbacks));
	classDef.def("CallEventListeners", static_cast<void (*)(lua_State *, ::WIBase &, std::string, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object)>(&CallCallbacks));
	classDef.def("CallEventListeners", static_cast<void (*)(lua_State *, ::WIBase &, std::string, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object)>(&CallCallbacks));
	classDef.def("CallEventListeners", static_cast<void (*)(lua_State *, ::WIBase &, std::string, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object)>(&CallCallbacks));
	classDef.def("FadeIn", static_cast<void (*)(lua_State *, ::WIBase &, float)>(&FadeIn));
	classDef.def("FadeIn", static_cast<void (*)(lua_State *, ::WIBase &, float, float)>(&FadeIn));
	classDef.def("FadeOut", static_cast<void (::WIBase::*)(float, bool)>(&::WIBase::FadeOut));
	classDef.def("FadeOut", static_cast<void (*)(::WIBase &, float)>([](::WIBase &el, float tFade) { el.FadeOut(tFade); }));
	classDef.def("IsFading", &::WIBase::IsFading);
	classDef.def("IsFadingOut", &::WIBase::IsFadingOut);
	classDef.def("IsFadingIn", &::WIBase::IsFadingIn);
	classDef.def("GetClass", &::WIBase::GetClass);
	classDef.def(
	  "Think", +[](::WIBase &el, std::shared_ptr<prosper::ICommandBuffer> &cmd) {
		  if(!cmd->IsPrimary())
			  return;
		  auto primaryCmdBuffer = std::dynamic_pointer_cast<prosper::IPrimaryCommandBuffer>(cmd);
		  el.Think(primaryCmdBuffer);
	  });
	classDef.def("InjectMouseMoveInput", &InjectMouseMoveInput);
	classDef.def("InjectMouseInput", static_cast<::util::EventReply (*)(lua_State *, ::WIBase &, const Vector2 &, int, int, int)>(&InjectMouseInput));
	classDef.def("InjectMouseInput", static_cast<::util::EventReply (*)(lua_State *, ::WIBase &, const Vector2 &, int, int)>(&InjectMouseInput));
	classDef.def("InjectMouseClick", static_cast<::util::EventReply (*)(lua_State *, ::WIBase &, const Vector2 &, int, int)>(&InjectMouseClick));
	classDef.def("InjectMouseClick", static_cast<::util::EventReply (*)(lua_State *, ::WIBase &, const Vector2 &, int)>(&InjectMouseClick));
	classDef.def("InjectKeyboardInput", static_cast<::util::EventReply (*)(lua_State *, ::WIBase &, int, int, int)>(&InjectKeyboardInput));
	classDef.def("InjectKeyboardInput", static_cast<::util::EventReply (*)(lua_State *, ::WIBase &, int, int)>(&InjectKeyboardInput));
	classDef.def("InjectKeyPress", static_cast<::util::EventReply (*)(lua_State *, ::WIBase &, int, int)>(&InjectKeyPress));
	classDef.def("InjectKeyPress", static_cast<::util::EventReply (*)(lua_State *, ::WIBase &, int)>(&InjectKeyPress));
	classDef.def("InjectCharInput", static_cast<::util::EventReply (*)(lua_State *, ::WIBase &, std::string, uint32_t)>(&InjectCharInput));
	classDef.def("InjectCharInput", static_cast<::util::EventReply (*)(lua_State *, ::WIBase &, std::string)>(&InjectCharInput));
	classDef.def("InjectScrollInput", static_cast<::util::EventReply (*)(lua_State *, ::WIBase &, const Vector2 &, const Vector2 &, bool)>(&InjectScrollInput));
	classDef.def("InjectScrollInput", static_cast<::util::EventReply (*)(lua_State *, ::WIBase &, const Vector2 &, const Vector2 &)>(&InjectScrollInput));
	classDef.def("IsDescendant", &::WIBase::IsDescendant);
	classDef.def("IsDescendantOf", &::WIBase::IsDescendantOf);
	classDef.def("IsAncestor", &::WIBase::IsAncestor);
	classDef.def("IsAncestorOf", &::WIBase::IsAncestorOf);
	classDef.def("GetName", &::WIBase::GetName);
	classDef.def("SetName", &::WIBase::SetName);
	classDef.def("FindChildByName", &FindChildByName);
	classDef.def("FindChildrenByName", &FindChildrenByName);
	classDef.def("SetAutoAlignToParent", static_cast<void (::WIBase::*)(bool, bool)>(&::WIBase::SetAutoAlignToParent));
	classDef.def("SetAutoAlignToParent", static_cast<void (::WIBase::*)(bool)>(&::WIBase::SetAutoAlignToParent));
	classDef.def("GetAutoAlignToParentX", &::WIBase::GetAutoAlignToParentX);
	classDef.def("GetAutoAlignToParentY", &::WIBase::GetAutoAlignToParentY);
	classDef.def("Resize", &::WIBase::Resize);
	classDef.def("ScheduleUpdate", &::WIBase::ScheduleUpdate);
	classDef.def("SetSkin", &::WIBase::SetSkin);
	classDef.def("ResetSkin", &::WIBase::ResetSkin);
	classDef.def("GetStyleClasses", &::WIBase::GetStyleClasses);
	classDef.def("AddStyleClass", &::WIBase::AddStyleClass);
	classDef.def("SetCursor", &::WIBase::SetCursor);
	classDef.def("GetCursor", &::WIBase::GetCursor);
	classDef.def("RemoveElementOnRemoval", &::WIBase::RemoveOnRemoval);
	classDef.def("GetTooltip", &::WIBase::GetTooltip);
	classDef.def("SetTooltip", &::WIBase::SetTooltip);
	classDef.def("HasTooltip", &::WIBase::HasTooltip);
	classDef.def("GetLeft", &::WIBase::GetLeft);
	classDef.def("GetTop", &::WIBase::GetTop);
	classDef.def("GetRight", &::WIBase::GetRight);
	classDef.def("GetBottom", &::WIBase::GetBottom);
	classDef.def("GetEndPos", &::WIBase::GetEndPos);
	classDef.def("SetClippingEnabled", &::WIBase::SetShouldScissor);
	classDef.def("IsClippingEnabled", &::WIBase::GetShouldScissor);
	classDef.def("SetAlwaysUpdate", &::WIBase::SetThinkIfInvisible);
	classDef.def("SetBounds", &SetBounds);
	classDef.def("SetBackgroundElement", static_cast<void (::WIBase::*)(bool, bool)>(&::WIBase::SetBackgroundElement));
	classDef.def("SetBackgroundElement", static_cast<void (*)(::WIBase &, bool)>([](::WIBase &el, bool backgroundElement) { el.SetBackgroundElement(backgroundElement); }));
	classDef.def("SetBackgroundElement", static_cast<void (*)(::WIBase &)>([](::WIBase &el) { el.SetBackgroundElement(true); }));
	classDef.def("IsBackgroundElement", &::WIBase::IsBackgroundElement);
	classDef.def("FindDescendantByName", static_cast<::WIBase *(*)(lua_State *, ::WIBase &, const std::string &)>([](lua_State *l, ::WIBase &hPanel, const std::string &name) { return hPanel.FindDescendantByName(name); }));
	classDef.def("FindDescendantsByName", static_cast<luabind::tableT<::WIBase> (*)(lua_State *, ::WIBase &, const std::string &)>([](lua_State *l, ::WIBase &hPanel, const std::string &name) -> luabind::tableT<::WIBase> {
		std::vector<::WIHandle> children {};
		hPanel.FindDescendantsByName(name, children);
		auto t = luabind::newtable(l);
		auto idx = 1;
		for(auto &hChild : children) {
			if(hChild.IsValid() == false)
				continue;
			t[idx++] = WGUILuaInterface::GetLuaObject(l, *hChild.get());
		}
		return t;
	}));
	classDef.def("Update", &::WIBase::Update);
	classDef.def("ApplySkin", &::WIBase::ApplySkin);
	classDef.def("ApplySkin", &::WIBase::ApplySkin, luabind::default_parameter_policy<2, static_cast<WISkin *>(nullptr)> {});
	classDef.def("RefreshSkin", &::WIBase::RefreshSkin);
	classDef.def("SetLeft", &::WIBase::SetLeft);
	classDef.def("SetRight", &::WIBase::SetRight);
	classDef.def("SetTop", &::WIBase::SetTop);
	classDef.def("SetBottom", &::WIBase::SetBottom);
	classDef.def("EnableThinking", &::WIBase::EnableThinking);
	classDef.def("DisableThinking", &::WIBase::DisableThinking);
	classDef.def("SetThinkingEnabled", &::WIBase::SetThinkingEnabled);
	classDef.def(
	  "InvokeThink", +[](::WIBase &el, std::shared_ptr<prosper::ICommandBuffer> &cmd) {
		  if(!cmd->IsPrimary())
			  return;
		  auto primaryCmdBuffer = std::dynamic_pointer_cast<prosper::IPrimaryCommandBuffer>(cmd);
		  el.Think(primaryCmdBuffer);
	  });

	classDef.def("AddAttachment", static_cast<WIAttachment *(::WIBase::*)(const std::string &, const Vector2 &)>(&::WIBase::AddAttachment));
	classDef.def("AddAttachment", static_cast<WIAttachment *(*)(::WIBase &, const std::string &)>([](::WIBase &el, const std::string &name) { return el.AddAttachment(name); }));
	classDef.def("SetAttachmentPos", &::WIBase::SetAttachmentPos);
	classDef.def("GetAttachmentPos", &::WIBase::GetAttachmentPos, luabind::copy_policy<0> {});
	classDef.def("GetAbsoluteAttachmentPos", &::WIBase::GetAbsoluteAttachmentPos, luabind::copy_policy<0> {});
	classDef.def("GetAttachmentPosProperty", &::WIBase::GetAttachmentPosProperty);
	classDef.def("SetAnchor", static_cast<void (::WIBase::*)(float, float, float, float, uint32_t, uint32_t)>(&::WIBase::SetAnchor));
	classDef.def("SetAnchor", static_cast<void (*)(::WIBase &, float, float, float, float)>([](::WIBase &el, float left, float top, float right, float bottom) { el.SetAnchor(left, top, right, bottom); }));
	classDef.def("SetAnchorLeft", &::WIBase::SetAnchorLeft);
	classDef.def("SetAnchorRight", &::WIBase::SetAnchorRight);
	classDef.def("SetAnchorTop", &::WIBase::SetAnchorTop);
	classDef.def("SetAnchorBottom", &::WIBase::SetAnchorBottom);
	classDef.def("ClearAnchor", &::WIBase::ClearAnchor);
	classDef.def(
	  "GetAnchor", +[](::WIBase &el) -> std::optional<std::tuple<float, float, float, float>> {
		  float left, top, right, bottom;
		  if(!el.GetAnchor(left, top, right, bottom))
			  return {};
		  return std::tuple<float, float, float, float> {left, top, right, bottom};
	  });
	classDef.def("HasAnchor", &::WIBase::HasAnchor);
	classDef.def("SetRemoveOnParentRemoval", &::WIBase::SetRemoveOnParentRemoval);
	classDef.def("GetCenter", &::WIBase::GetCenter);
	classDef.def("GetCenterX", &::WIBase::GetCenterX);
	classDef.def("GetCenterY", &::WIBase::GetCenterY);
	classDef.def("GetHalfWidth", &::WIBase::GetHalfWidth);
	classDef.def("GetHalfHeight", &::WIBase::GetHalfHeight);
	classDef.def("GetHalfSize", &::WIBase::GetHalfSize);
	classDef.def("SetCenterPos", &::WIBase::SetCenterPos);
	classDef.def("SetLocalAlpha", &::WIBase::SetLocalAlpha);
	classDef.def("GetLocalAlpha", &::WIBase::GetLocalAlpha);
	classDef.def("CenterToParent", static_cast<void (::WIBase::*)(bool)>(&::WIBase::CenterToParent));
	classDef.def("CenterToParent", static_cast<void (*)(lua_State *, ::WIBase &)>([](lua_State *l, ::WIBase &hPanel) { hPanel.CenterToParent(false); }));
	classDef.def("CenterToParentX", &::WIBase::CenterToParentX);
	classDef.def("CenterToParentY", &::WIBase::CenterToParentY);
	classDef.def("RemoveStyleClass", &::WIBase::RemoveStyleClass);
	classDef.def("ClearStyleClasses", &::WIBase::ClearStyleClasses);
	classDef.def("FindChildIndex", &::WIBase::FindChildIndex);
	classDef.def("SetScale", static_cast<void (::WIBase::*)(const Vector2 &)>(&::WIBase::SetScale));
	classDef.def("SetScale", static_cast<void (::WIBase::*)(float, float)>(&::WIBase::SetScale));
	classDef.def("GetScale", &::WIBase::GetScale, luabind::copy_policy<0> {});
	classDef.def("GetScaleProperty", &::WIBase::GetScaleProperty);
	classDef.def("IsUpdateScheduled", &::WIBase::IsUpdateScheduled);
	classDef.def("IsRemovalScheduled", &::WIBase::IsRemovalScheduled);
	classDef.def("GetRootElement", static_cast<::WIBase *(::WIBase::*)()>(&::WIBase::GetRootElement));
	classDef.def("GetRootWindow", static_cast<prosper::Window *(::WIBase::*)()>(&::WIBase::GetRootWindow));
	classDef.def(
	  "ClampToBounds", +[](const ::WIBase &el, Vector2i &pos) { el.ClampToBounds(pos); });
	classDef.def(
	  "ClampToBounds", +[](const ::WIBase &el, Vector2i &pos, Vector2i &size) { el.ClampToBounds(pos, size); });
	classDef.def(
	  "GetVisibleBounds", +[](const ::WIBase &el) -> std::pair<Vector2i, Vector2i> {
		  Vector2i pos, size;
		  el.GetVisibleBounds(pos, size);
		  return {pos, size};
	  });
	classDef.def(
	  "GetAbsoluteVisibleBounds", +[](const ::WIBase &el) -> std::tuple<Vector2i, Vector2i, Vector2i> {
		  Vector2i pos, size;
		  Vector2i absPosParent;
		  el.GetAbsoluteVisibleBounds(pos, size, &absPosParent);
		  return {pos, size, absPosParent};
	  });
	classDef.def(
	  "ClampToVisibleBounds", +[](const ::WIBase &el, Vector2i &pos) { el.ClampToVisibleBounds(pos); });
	classDef.def(
	  "ClampToVisibleBounds", +[](const ::WIBase &el, Vector2i &pos, Vector2i &size) { el.ClampToVisibleBounds(pos, size); });
	classDef.def(
	  "DebugPrintHierarchy", +[](const ::WIBase &el) { debug_print_hierarchy(el); });
	classDef.def("IsFileHovering", &::WIBase::IsFileHovering);
	classDef.def("SetFileHovering", &::WIBase::SetFileHovering);
	classDef.def("GetFileDropInputEnabled", &::WIBase::GetFileDropInputEnabled);
	classDef.def("SetFileDropInputEnabled", &::WIBase::SetFileDropInputEnabled);

	auto defDrawInfo = luabind::class_<::WIBase::DrawInfo>("DrawInfo");
	defDrawInfo.add_static_constant("FLAG_NONE", umath::to_integral(::WIBase::DrawInfo::Flags::None));
	defDrawInfo.add_static_constant("FLAG_USE_SCISSOR_BIT", umath::to_integral(::WIBase::DrawInfo::Flags::UseScissor));
	defDrawInfo.add_static_constant("FLAG_USE_STENCIL_BIT", umath::to_integral(::WIBase::DrawInfo::Flags::UseStencil));
	defDrawInfo.add_static_constant("FLAG_MSAA_BIT", umath::to_integral(::WIBase::DrawInfo::Flags::Msaa));
	defDrawInfo.add_static_constant("FLAG_DONT_SKIP_IF_OUT_OF_BOUNDS_BIT", umath::to_integral(::WIBase::DrawInfo::Flags::DontSkipIfOutOfBounds));
	defDrawInfo.def(luabind::constructor<const std::shared_ptr<prosper::ICommandBuffer> &>());
	defDrawInfo.def_readwrite("offset", &::WIBase::DrawInfo::offset);
	defDrawInfo.def_readwrite("size", &::WIBase::DrawInfo::size);
	defDrawInfo.def_readwrite("transform", &::WIBase::DrawInfo::transform);
	defDrawInfo.def_readwrite("flags", &::WIBase::DrawInfo::flags);
	defDrawInfo.property("commandBuffer", static_cast<luabind::object (*)(lua_State *, ::WIBase::DrawInfo &)>([](lua_State *l, ::WIBase::DrawInfo &drawInfo) -> luabind::object {
		return drawInfo.commandBuffer ? luabind::object {l, drawInfo.commandBuffer} : luabind::object {};
	}),
	  static_cast<void (*)(lua_State *, ::WIBase::DrawInfo &, luabind::object)>([](lua_State *l, ::WIBase::DrawInfo &drawInfo, luabind::object o) {
		  if(Lua::IsSet(l, 2) == false) {
			  drawInfo.commandBuffer = nullptr;
			  return;
		  }
		  drawInfo.commandBuffer = Lua::Check<Lua::Vulkan::CommandBuffer>(l, 2).shared_from_this();
	  }));
	defDrawInfo.def("SetColor", static_cast<void (*)(lua_State *, ::WIBase::DrawInfo &, const Color &)>([](lua_State *l, ::WIBase::DrawInfo &drawInfo, const Color &color) { drawInfo.color = color.ToVector4(); }));
	defDrawInfo.def("SetPostTransform", static_cast<void (*)(lua_State *, ::WIBase::DrawInfo &, const Mat4 &)>([](lua_State *l, ::WIBase::DrawInfo &drawInfo, const Mat4 &t) { drawInfo.postTransform = t; }));
	classDef.scope[defDrawInfo];
}

void Lua::WIButton::register_class(luabind::class_<::WIButton, ::WIBase> &classDef)
{
	classDef.def(
	  "SetText", +[](::WIButton &button, const std::string &text) { button.SetText(text); });
	classDef.def(
	  "GetText", +[](::WIButton &button) { return button.GetText().cpp_str(); });
}

void Lua::WIProgressBar::register_class(luabind::class_<::WIProgressBar, ::WIBase> &classDef)
{
	classDef.def("SetProgress", &::WIProgressBar::SetProgress);
	classDef.def("GetProgress", &::WIProgressBar::GetProgress);
	classDef.def("SetLabelVisible", &::WIProgressBar::SetLabelVisible);
	classDef.def("SetValue", &::WIProgressBar::SetValue);
	classDef.def("GetValue", static_cast<float (::WIProgressBar::*)() const>(&::WIProgressBar::GetValue));
	classDef.def("SetRange", static_cast<void (::WIProgressBar::*)(float, float, float)>(&::WIProgressBar::SetRange));
	classDef.def("SetRange", static_cast<void (*)(::WIProgressBar &, float, float)>([](::WIProgressBar &el, float min, float max) { el.SetRange(min, max); }));
	classDef.def("SetOptions", &::WIProgressBar::SetOptions);
	classDef.def("AddOption", &::WIProgressBar::AddOption);
	classDef.def("SetPostFix", &::WIProgressBar::SetPostFix);
	classDef.def(
	  "GetRange", +[](::WIProgressBar &el) -> std::tuple<float, float, float> {
		  auto range = el.GetRange();
		  return {range[0], range[1], range[2]};
	  });
	//classDef.def("SetValueTranslator",&SetValueTranslator);
}

void Lua::WISlider::register_class(luabind::class_<::WISlider, luabind::bases<::WIProgressBar, ::WIBase>> &classDef) { classDef.def("IsBeingDragged", &::WISlider::IsBeingDragged); }

void Lua::WIShape::register_class(luabind::class_<::WIShape, ::WIBase> &classDef)
{
	classDef.def("AddVertex", &::WIShape::AddVertex);
	classDef.def("SetVertexPos", &::WIShape::SetVertexPos);
	classDef.def("ClearVertices", &::WIShape::ClearVertices);
	classDef.def("GetBuffer", &::WIShape::GetBuffer, luabind::shared_from_this_policy<0> {});
	classDef.def("SetBuffer", &::WIShape::SetBuffer);
	classDef.def("ClearBuffer", &::WIShape::ClearBuffer);
	classDef.def("GetVertexCount", &::WIShape::GetVertexCount);
	classDef.def("InvertVertexPositions", static_cast<void (::WIShape::*)(bool, bool)>(&::WIShape::InvertVertexPositions));
	classDef.def("InvertVertexPositions", static_cast<void (*)(::WIShape &, bool)>([](::WIShape &el, bool x) { el.InvertVertexPositions(x); }));
	classDef.def("InvertVertexPositions", static_cast<void (*)(::WIShape &)>([](::WIShape &el) { el.InvertVertexPositions(); }));
	classDef.def("SetShape", &::WIShape::SetShape);
	classDef.add_static_constant("SHAPE_RECTANGLE", umath::to_integral(::WIShape::BasicShape::Rectangle));
	classDef.add_static_constant("SHAPE_CIRCLE", umath::to_integral(::WIShape::BasicShape::Circle));
}

void Lua::WITexturedShape::register_class(luabind::class_<::WITexturedShape, luabind::bases<::WIShape, ::WIBase>> &classDef)
{
	classDef.def("SetMaterial", static_cast<void (::WITexturedShape::*)(Material *)>(&::WITexturedShape::SetMaterial));
	classDef.def("SetMaterial", static_cast<void (::WITexturedShape::*)(const std::string &)>(&::WITexturedShape::SetMaterial));
	classDef.def("GetMaterial", &::WITexturedShape::GetMaterial);
	classDef.def("SetTexture", static_cast<void (*)(::WITexturedShape &, prosper::Texture &)>([](::WITexturedShape &shape, prosper::Texture &tex) { shape.SetTexture(tex); }));
	classDef.def("GetTexture", &::WITexturedShape::GetTexture);
	classDef.def("AddVertex", static_cast<uint32_t (::WITexturedShape::*)(Vector2)>(&::WITexturedShape::AddVertex));
	classDef.def("AddVertex", static_cast<uint32_t (::WITexturedShape::*)(Vector2, Vector2)>(&::WITexturedShape::AddVertex));
	classDef.def("SetVertexUVCoord", &::WITexturedShape::SetVertexUVCoord);
	classDef.def("InvertVertexUVCoordinates", static_cast<void (::WITexturedShape::*)(bool, bool)>(&::WITexturedShape::InvertVertexUVCoordinates));
	classDef.def("InvertVertexUVCoordinates", static_cast<void (*)(::WITexturedShape &, bool)>([](::WITexturedShape &el, bool x) { el.InvertVertexUVCoordinates(x); }));
	classDef.def("InvertVertexUVCoordinates", static_cast<void (*)(::WITexturedShape &)>([](::WITexturedShape &el) { el.InvertVertexUVCoordinates(); }));
	classDef.def("ClearTexture", &::WITexturedShape::ClearTexture);
	classDef.def("SizeToTexture", &::WITexturedShape::SizeToTexture);
	classDef.def("GetTextureSize", &::WITexturedShape::GetTextureSize);
	classDef.def("SetChannelSwizzle", &::WITexturedShape::SetChannelSwizzle);
	classDef.def("SetChannelSwizzle", &::WITexturedShape::GetChannelSwizzle);
	classDef.def("SetShader", static_cast<void (::WITexturedShape::*)(wgui::ShaderTextured &)>(&::WITexturedShape::SetShader));
	classDef.def("GetAlphaMode", &::WITexturedShape::GetAlphaMode);
	classDef.def("SetAlphaMode", &::WITexturedShape::SetAlphaMode);
	classDef.def("SetAlphaCutoff", &::WITexturedShape::SetAlphaCutoff);
	classDef.def("GetAlphaCutoff", &::WITexturedShape::GetAlphaCutoff);
	classDef.add_static_constant("CHANNEL_RED", umath::to_integral(::wgui::ShaderTextured::Channel::Red));
	classDef.add_static_constant("CHANNEL_GREEN", umath::to_integral(::wgui::ShaderTextured::Channel::Green));
	classDef.add_static_constant("CHANNEL_BLUE", umath::to_integral(::wgui::ShaderTextured::Channel::Blue));
	classDef.add_static_constant("CHANNEL_ALPHA", umath::to_integral(::wgui::ShaderTextured::Channel::Alpha));
}

void Lua::WIIcon::register_class(luabind::class_<::WIIcon, luabind::bases<::WITexturedShape, ::WIShape, ::WIBase>> &classDef) { classDef.def("SetClipping", &::WIIcon::SetClipping); }

void Lua::WISilkIcon::register_class(luabind::class_<::WISilkIcon, luabind::bases<::WIIcon, ::WITexturedShape, ::WIShape, ::WIBase>> &classDef) { classDef.def("SetIcon", &::WISilkIcon::SetIcon); }

void Lua::WIArrow::register_class(luabind::class_<::WIArrow, luabind::bases<::WIShape, ::WIBase>> &classDef)
{
	classDef.def("SetDirection", &::WIArrow::SetDirection);
	classDef.add_static_constant("DIRECTION_RIGHT", 0);
	classDef.add_static_constant("DIRECTION_DOWN", 1);
	classDef.add_static_constant("DIRECTION_LEFT", 2);
	classDef.add_static_constant("DIRECTION_UP", 3);
}

void Lua::WICheckbox::register_class(luabind::class_<::WICheckbox, luabind::bases<::WIShape, ::WIBase>> &classDef)
{
	classDef.def("SetChecked", &::WICheckbox::SetChecked);
	classDef.def("IsChecked", &::WICheckbox::IsChecked);
	classDef.def("Toggle", &::WICheckbox::Toggle);
}

void Lua::WIGridPanel::register_class(luabind::class_<::WIGridPanel, luabind::bases<::WITable, ::WIBase>> &classDef)
{
	classDef.def("AddItem", &::WIGridPanel::AddItem);
	classDef.def("GetColumnCount", &::WIGridPanel::GetColumnCount);
}

void Lua::WITreeList::register_class(luabind::class_<::WITreeList, luabind::bases<::WITable, ::WIBase>> &classDef)
{
	classDef.def("AddItem", static_cast<::WITreeListElement *(*)(lua_State *, ::WITreeList &, const std::string &)>([](lua_State *l, ::WITreeList &hPanel, const std::string &text) -> ::WITreeListElement * { return hPanel.AddItem(text); }));
	classDef.def("AddItem", static_cast<::WIBase *(*)(lua_State *, ::WITreeList &, const std::string &, Lua::func<void(::WIBase)>)>([](lua_State *l, ::WITreeList &hPanel, const std::string &text, Lua::func<void(::WIBase)> populate) -> ::WIBase * {
		auto fPopulate = [l, populate](::WITreeListElement &el) {
			Lua::CallFunction(l, [&populate, &el](lua_State *l) {
				populate.push(l);
				auto o = WGUILuaInterface::GetLuaObject(l, el);
				o.push(l);
				return Lua::StatusCode::Ok;
			});
		};
		return hPanel.AddItem(text, fPopulate);
	}));
	classDef.def("ExpandAll", &::WITreeList::ExpandAll);
	classDef.def("CollapseAll", &::WITreeList::CollapseAll);
	classDef.def("GetRootItem", &::WITreeList::GetRootItem);
}
void Lua::WITreeListElement::register_class(luabind::class_<::WITreeListElement, luabind::bases<::WITableRow, ::WIBase>> &classDef)
{
	classDef.def("Expand", static_cast<void (::WITreeListElement::*)(bool)>(&::WITreeListElement::Expand));
	classDef.def("Expand", static_cast<void (*)(::WITreeListElement &)>([](::WITreeListElement &el) { el.Expand(); }));
	classDef.def("Collapse", static_cast<void (::WITreeListElement::*)(bool)>(&::WITreeListElement::Collapse));
	classDef.def("Collapse", static_cast<void (*)(::WITreeListElement &)>([](::WITreeListElement &el) { el.Collapse(); }));
	classDef.def("GetItems", &::WITreeListElement::GetItems);
	classDef.def("AddItem", static_cast<::WITreeListElement *(*)(lua_State *, ::WITreeListElement &, const std::string &)>([](lua_State *l, ::WITreeListElement &hPanel, const std::string &text) -> ::WITreeListElement * { return hPanel.AddItem(text); }));
	classDef.def("AddItem", static_cast<::WIBase *(*)(lua_State *, ::WITreeListElement &, const std::string &, Lua::func<void(::WIBase)>)>([](lua_State *l, ::WITreeListElement &hPanel, const std::string &text, Lua::func<void(::WIBase)> populate) -> ::WIBase * {
		auto fPopulate = [l, populate](::WITreeListElement &el) {
			Lua::CallFunction(l, [&populate, &el](lua_State *l) {
				populate.push(l);
				auto o = WGUILuaInterface::GetLuaObject(l, el);
				o.push(l);
				return Lua::StatusCode::Ok;
			});
		};
		return hPanel.AddItem(text, fPopulate);
	}));
}

void Lua::WIContainer::register_class(luabind::class_<::WIContainer, ::WIBase> &classDef)
{
	classDef.def("SetPadding", static_cast<void (::WIContainer::*)(int32_t)>(&::WIContainer::SetPadding));
	classDef.def("SetPadding", static_cast<void (::WIContainer::*)(int32_t, int32_t, int32_t, int32_t)>(&::WIContainer::SetPadding));
	classDef.def("SetPaddingTop", &::WIContainer::SetPaddingTop);
	classDef.def("SetPaddingRight", &::WIContainer::SetPaddingRight);
	classDef.def("SetPaddingBottom", &::WIContainer::SetPaddingBottom);
	classDef.def("SetPaddingLeft", &::WIContainer::SetPaddingLeft);
	classDef.def("GetPadding", static_cast<const std::array<int32_t, 4> &(::WIContainer::*)() const>(&::WIContainer::GetPadding));
	classDef.def("GetPaddingTop", &::WIContainer::GetPaddingTop);
	classDef.def("GetPaddingRight", &::WIContainer::GetPaddingRight);
	classDef.def("GetPaddingBottom", &::WIContainer::GetPaddingBottom);
	classDef.def("GetPaddingLeft", &::WIContainer::GetPaddingLeft);
}

void Lua::WITable::register_class(luabind::class_<::WITable, luabind::bases<::WIContainer, ::WIBase>> &classDef)
{
	classDef.def("GetRowHeight", &::WITable::GetRowHeight);
	classDef.def("SetRowHeight", &::WITable::SetRowHeight);
	classDef.def("SetSelectableMode", &::WITable::SetSelectable);
	classDef.def("GetSelectableMode", &::WITable::GetSelectableMode);
	classDef.def("SetColumnWidth", &::WITable::SetColumnWidth);
	classDef.def("AddRow", static_cast<::WITableRow *(*)(::WITable &)>([](::WITable &el) { return el.AddRow(); }));
	classDef.def("AddHeaderRow", &::WITable::AddHeaderRow);
	classDef.def("GetRowCount", &::WITable::GetRowCount);
	classDef.def("SetSortable", &::WITable::SetSortable);
	classDef.def("IsSortable", &::WITable::IsSortable);
	classDef.def("Sort", static_cast<void (::WITable::*)()>(&::WITable::Sort));
	classDef.def(
	  "SetSortFunction", +[](lua_State *l, ::WITable &table, const Lua::func<bool, const ::WITableRow &, const ::WITableRow &> &lfunc) {
		  table.SetSortFunction([l, lfunc](const ::WITableRow &rowA, const ::WITableRow &rowB, uint32_t columnIndex, bool ascending) -> bool {
			  auto r = Lua::CallFunction(
			    l,
			    [&lfunc, &rowA, &rowB, columnIndex, ascending](lua_State *l) {
				    lfunc.push(l);
				    Lua::Push<::WIBase *>(l, const_cast<::WITableRow *>(&rowA));
				    Lua::Push<::WIBase *>(l, const_cast<::WITableRow *>(&rowB));
				    Lua::Push<uint32_t>(l, columnIndex);
				    Lua::Push<bool>(l, ascending);
				    return Lua::StatusCode::Ok;
			    },
			    1);
			  if(r == Lua::StatusCode::Ok) {
				  auto res = Lua::CheckBool(l, -1);
				  Lua::Pop(l, 1);
				  return res;
			  }
			  return false;
		  });
	  });

	classDef.def("SetScrollable", &::WITable::SetScrollable);
	classDef.def("IsScrollable", &::WITable::IsScrollable);
	classDef.def("Clear", &::WITable::Clear);
	classDef.def("Clear", &::WITable::Clear, luabind::default_parameter_policy<2, false> {});
	classDef.def("GetRow", &::WITable::GetRow);
	classDef.def("GetSelectedRows", &GetSelectedRows);
	classDef.def("GetRows", &GetRows);
	classDef.def("RemoveRow", &::WITable::RemoveRow);
	classDef.def("MoveRow", &::WITable::MoveRow);
	classDef.def("MoveRow", static_cast<void (*)(::WITable &, ::WITableRow &, ::WITableRow &)>([](::WITable &el, ::WITableRow &a, ::WITableRow &pos) { el.MoveRow(&a, &pos); }));
	classDef.def("SelectRow", &::WITable::SelectRow);
	classDef.def("GetFirstSelectedRow", &::WITable::GetFirstSelectedRow);
	classDef.add_static_constant("SELECTABLE_MODE_NONE", umath::to_integral(::WITable::SelectableMode::None));
	classDef.add_static_constant("SELECTABLE_MODE_SINGLE", umath::to_integral(::WITable::SelectableMode::Single));
	classDef.add_static_constant("SELECTABLE_MODE_MULTI", umath::to_integral(::WITable::SelectableMode::Multi));
}

void Lua::WITableRow::register_class(luabind::class_<::WITableRow, luabind::bases<::WIContainer, ::WIBase>> &classDef)
{
	classDef.def("Select", &::WITableRow::Select);
	classDef.def("Deselect", &::WITableRow::Deselect);
	classDef.def("IsSelected", &::WITableRow::IsSelected);
	classDef.def("SetCellWidth", &::WITableRow::SetCellWidth);
	classDef.def("SetValue", &::WITableRow::SetValue);
	classDef.def("GetValue", static_cast<std::string (::WIBase::*)(uint32_t) const>(&::WITableRow::GetValue));
	classDef.def("InsertElement", static_cast<::WITableCell *(::WIBase::*)(uint32_t, ::WIBase *)>(&::WITableRow::InsertElement));
	classDef.def("GetCellCount", &::WITableRow::GetCellCount);
	classDef.def("GetCell", &::WITableRow::GetCell);
	classDef.def("GetRowIndex", static_cast<uint32_t (*)(lua_State *, ::WITableRow &)>([](lua_State *l, ::WITableRow &hRow) -> uint32_t {
		auto *pTable = hRow.GetTable();
		uint32_t rowIndex = 0u;
		if(pTable)
			rowIndex = pTable->GetRowIndex(&hRow);
		return rowIndex;
	}));
}

void Lua::WITableCell::register_class(luabind::class_<::WITableCell, luabind::bases<::WIContainer, ::WIBase>> &classDef)
{
	classDef.def("GetFirstElement", &::WITableCell::GetFirstElement);
	classDef.def("GetColSpan", &::WITableCell::GetColSpan);
	//classDef.def("GetRowSpan",&GetRowSpan);
	classDef.def("SetColSpan", &::WITableCell::SetColSpan);
	//classDef.def("SetRowSpan",&SetRowSpan);
}

void Lua::WIFrame::register_class(luabind::class_<::WIFrame, luabind::bases<::WITransformable, ::WIBase>> &classDef)
{
	classDef.def("SetTitle", &::WIFrame::SetTitle);
	classDef.def("GetTitle", &::WIFrame::GetTitle);
	classDef.def("SetCloseButtonEnabled", &::WIFrame::SetCloseButtonEnabled);
}

void Lua::WIDropDownMenu::register_class(luabind::class_<::WIDropDownMenu, luabind::bases<::WITextEntry, ::WIBase>> &classDef)
{
	classDef.def("SelectOption", static_cast<void (::WIDropDownMenu::*)(uint32_t)>(&::WIDropDownMenu::SelectOption));
	classDef.def("SelectOption", static_cast<void (::WIDropDownMenu::*)(const std::string &)>(&::WIDropDownMenu::SelectOption));
	classDef.def("ClearOptions", &::WIDropDownMenu::ClearOptions);
	classDef.def(
	  "SelectOptionByText", +[](::WIDropDownMenu &menu, const std::string &text) { return menu.SelectOptionByText(text); });
	classDef.def(
	  "GetOptionText", +[](::WIDropDownMenu &menu, uint32_t idx) { return menu.GetOptionText(idx).cpp_str(); });
	classDef.def(
	  "FindOptionIndex", +[](::WIDropDownMenu &menu, const std::string &value) -> std::optional<uint32_t> {
		  auto n = menu.GetOptionCount();
		  for(auto i = decltype(n) {0u}; i < n; ++i) {
			  auto val = menu.GetOptionValue(i);
			  if(val == value)
				  return i;
		  }
		  return {};
	  });
	classDef.def("GetOptionValue", &::WIDropDownMenu::GetOptionValue);
	classDef.def("SetOptionText", &::WIDropDownMenu::SetOptionText);
	classDef.def("SetOptionValue", &::WIDropDownMenu::SetOptionValue);
	classDef.def("GetValue", &::WIDropDownMenu::GetValue);
	classDef.def(
	  "GetText", +[](const ::WIDropDownMenu &menu) { return menu.GetText().cpp_str(); });
	classDef.def(
	  "SetText", +[](::WIDropDownMenu &menu, const std::string &text) { return menu.SetText(text); });
	classDef.def("GetOptionCount", &::WIDropDownMenu::GetOptionCount);
	classDef.def("AddOption", static_cast<::WIDropDownMenuOption *(::WIDropDownMenu::*)(const std::string &, const std::string &)>(&::WIDropDownMenu::AddOption));
	classDef.def("AddOption", static_cast<::WIDropDownMenuOption *(::WIDropDownMenu::*)(const std::string &)>(&::WIDropDownMenu::AddOption));
	classDef.def("OpenMenu", &::WIDropDownMenu::OpenMenu);
	classDef.def("CloseMenu", &::WIDropDownMenu::CloseMenu);
	classDef.def("ToggleMenu", &::WIDropDownMenu::ToggleMenu);
	classDef.def("IsMenuOpen", &::WIDropDownMenu::IsMenuOpen);
	classDef.def("HasOption", &::WIDropDownMenu::HasOption);
	classDef.def("GetSelectedOption", &::WIDropDownMenu::GetSelectedOption);
	classDef.def("ClearSelectedOption", &::WIDropDownMenu::ClearSelectedOption);
	classDef.def("GetOptionElement", &::WIDropDownMenu::GetOptionElement);
	classDef.def("FindOptionSelectedByCursor", &::WIDropDownMenu::FindOptionSelectedByCursor);
	classDef.def("SetListItemCount", &::WIDropDownMenu::SetListItemCount);
	classDef.def("ScrollToOption", static_cast<void (::WIDropDownMenu::*)(uint32_t, bool)>(&::WIDropDownMenu::ScrollToOption));
	classDef.def("ScrollToOption", static_cast<void (*)(::WIDropDownMenu &, uint32_t)>([](::WIDropDownMenu &el, uint32_t offset) { el.ScrollToOption(offset); }));
}

void Lua::WIText::register_class(luabind::class_<::WIText, ::WIBase> &classDef)
{
	classDef.def("SetText", static_cast<void (*)(lua_State *, ::WIText &, const std::string &)>([](lua_State *l, ::WIText &hPanel, const std::string &text) { hPanel.SetText(text); }));
	classDef.def(
	  "GetText", +[](const ::WIText &text) { return text.GetText().cpp_str(); });
	classDef.def("GetTextHeight", &::WIText::GetTextHeight);
	classDef.def("CalcTextSize", &::WIText::CalcTextSize);
	classDef.def("GetTextWidth", &::WIText::GetTextWidth);
	classDef.def("SetFont", static_cast<void (*)(lua_State *, ::WIText &, const std::string &)>([](lua_State *l, ::WIText &hPanel, const std::string &font) { hPanel.SetFont(font); }));
	classDef.def(
	  "GetFont", +[](lua_State *l, ::WIText &hPanel) -> std::optional<std::string> {
		  auto *font = hPanel.GetFont();
		  if(!font)
			  return {};
		  return font->GetName();
	  });
	classDef.def("UpdateSubLines", &::WIText::UpdateSubLines);
	classDef.def("EnableShadow", &::WIText::EnableShadow);
	classDef.def("IsShadowEnabled", &::WIText::IsShadowEnabled);
	classDef.def("SetShadowColor", &SetShadowColor);
	classDef.def("SetShadowOffset", &SetShadowOffset);
	classDef.def("SetShadowXOffset", &SetShadowXOffset);
	classDef.def("SetShadowYOffset", &SetShadowYOffset);
	classDef.def("GetShadowColor", &GetShadowColor);
	classDef.def("GetShadowOffset", &GetShadowOffset);
	classDef.def("GetShadowXOffset", &GetShadowXOffset);
	classDef.def("GetShadowYOffset", &GetShadowYOffset);
	classDef.def("SetShadowAlpha", &::WIText::SetShadowAlpha);
	classDef.def("GetShadowAlpha", &::WIText::GetShadowAlpha);
	classDef.def("SetShadowBlurSize", &::WIText::SetShadowBlurSize);
	classDef.def("GetShadowBlurSize", &::WIText::GetShadowBlurSize);
	classDef.def("SetAutoBreakMode", &::WIText::SetAutoBreakMode);
	classDef.def("GetLineCount", &::WIText::GetLineCount);
	classDef.def("GetLine", static_cast<std::optional<std::string> (*)(lua_State *, ::WIText &, int32_t)>([](lua_State *l, ::WIText &hPanel, int32_t lineIndex) -> std::optional<std::string> {
		auto *pLine = hPanel.GetLine(lineIndex);
		if(pLine == nullptr)
			return {};
		return pLine->GetUnformattedLine().GetText().cpp_str();
	}));
	classDef.def("GetTextLength", static_cast<uint32_t (*)(lua_State *, ::WIText &)>([](lua_State *l, ::WIText &hPanel) -> uint32_t { return hPanel.GetText().length(); }));
	classDef.def("SetTagArgument", static_cast<void (*)(lua_State *, ::WIText &, const std::string &, uint32_t, luabind::object)>([](lua_State *l, ::WIText &hPanel, const std::string &label, uint32_t argIdx, luabind::object o) {
		if(Lua::IsString(l, 4)) {
			std::string arg = Lua::CheckString(l, 4);
			hPanel.SetTagArgument(label, argIdx, arg);
		}
		else if(Lua::IsType<Vector4>(l, 4)) {
			auto &arg = Lua::Check<Vector4>(l, 4);
			hPanel.SetTagArgument(label, argIdx, arg);
		}
		else if(Lua::IsType<Color>(l, 4)) {
			auto &arg = Lua::Check<Color>(l, 4);
			hPanel.SetTagArgument(label, argIdx, arg);
		}
		else if(Lua::IsFunction(l, 4)) {
			auto f = luabind::object(luabind::from_stack(l, 4));
			auto arg = FunctionCallback<::util::EventReply>::CreateWithOptionalReturn([f, l](::util::EventReply *reply) -> CallbackReturnType {
				auto r = Lua::CallFunction(
				  l,
				  [&f](lua_State *l) {
					  f.push(l);
					  return Lua::StatusCode::Ok;
				  },
				  1);
				if(r == Lua::StatusCode::Ok) {
					if(Lua::IsSet(l, -1))
						*reply = static_cast<::util::EventReply>(Lua::CheckInt(l, -1));
					else
						*reply = ::util::EventReply::Unhandled;
					return (*reply != ::util::EventReply::Unhandled) ? CallbackReturnType::HasReturnValue : CallbackReturnType::NoReturnValue;
				}
				return CallbackReturnType::NoReturnValue;
			});
			hPanel.SetTagArgument(label, argIdx, arg);
			Lua::Push<CallbackHandle>(l, arg);
		}
	}));
	classDef.def("SetTagsEnabled", &::WIText::SetTagsEnabled);
	classDef.def("AreTagsEnabled", &::WIText::AreTagsEnabled);
	classDef.def("PopFrontLine", &::WIText::PopFrontLine);
	classDef.def("PopBackLine", &::WIText::PopBackLine);
	classDef.def("RemoveText", static_cast<bool (::WIBase::*)(util::text::LineIndex, util::text::CharOffset, util::text::TextLength)>(&::WIText::RemoveText));
	classDef.def("RemoveText", static_cast<bool (::WIBase::*)(util::text::TextOffset, util::text::TextLength)>(&::WIText::RemoveText));
	classDef.def("RemoveLine", &::WIText::RemoveLine);
	classDef.def("InsertText", static_cast<bool (*)(::WIText &, const std::string &, util::text::LineIndex)>([](::WIText &hPanel, const std::string &text, util::text::LineIndex lineIdx) { return hPanel.InsertText(text, lineIdx); }));
	classDef.def("InsertText",
	  static_cast<bool (*)(::WIText &, const std::string &, util::text::LineIndex, util::text::CharOffset)>([](::WIText &hPanel, const std::string &text, util::text::LineIndex lineIdx, util::text::CharOffset charOffset) { return hPanel.InsertText(text, lineIdx, charOffset); }));
	classDef.def(
	  "SetMaxLineCount", +[](lua_State *l, ::WIText &hPanel, uint32_t c) { hPanel.GetFormattedTextObject().SetMaxLineCount(c); });
	classDef.def(
	  "GetMaxLineCount", +[](lua_State *l, ::WIText &hPanel) { return hPanel.GetFormattedTextObject().GetMaxLineCount(); });
	classDef.def(
	  "AppendText", +[](::WIText &el, const std::string &text) { return el.AppendText(text); });
	classDef.def("AppendLine", static_cast<void (*)(lua_State *, ::WIText &, const std::string &)>([](lua_State *l, ::WIText &hPanel, const std::string &line) { hPanel.AppendLine(line); }));
	classDef.def("MoveText", static_cast<void (*)(lua_State *, ::WIText &, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t)>([](lua_State *l, ::WIText &hPanel, uint32_t lineIdx, uint32_t startOffset, uint32_t len, uint32_t targetLineIdx, uint32_t targetCharOffset) {
		Lua::PushBool(l, hPanel.MoveText(lineIdx, startOffset, len, targetLineIdx, targetCharOffset));
	}));
	classDef.def("Clear", &::WIText::Clear);
	classDef.def(
	  "Substr", +[](::WIText &el, util::text::TextOffset startOffset, util::text::TextLength len) { return el.Substr(startOffset, len); });
	classDef.add_static_constant("AUTO_BREAK_NONE", umath::to_integral(::WIText::AutoBreak::NONE));
	classDef.add_static_constant("AUTO_BREAK_ANY", umath::to_integral(::WIText::AutoBreak::ANY));
	classDef.add_static_constant("AUTO_BREAK_WHITESPACE", umath::to_integral(::WIText::AutoBreak::WHITESPACE));
}

void Lua::WITextEntry::register_class(luabind::class_<::WITextEntry, ::WIBase> &classDef)
{
	classDef.def(
	  "SetText", +[](::WITextEntry &el, const std::string &text) { return el.SetText(text); });
	classDef.def(
	  "GetText", +[](const ::WITextEntry &el) { return el.GetText().cpp_str(); });
	classDef.def(
	  "GetValue", +[](const ::WITextEntry &el) { return el.GetText().cpp_str(); });
	classDef.def("IsNumeric", &::WITextEntry::IsNumeric);
	classDef.def("IsEditable", &::WITextEntry::IsEditable);
	classDef.def("SetEditable", &::WITextEntry::SetEditable);
	classDef.def("SetMaxLength", &::WITextEntry::SetMaxLength);
	classDef.def("GetMaxLength", &::WITextEntry::GetMaxLength);
	classDef.def("IsMultiLine", &::WITextEntry::IsMultiLine);
	classDef.def("SetMultiLine", &::WITextEntry::SetMultiLine);

	classDef.def("IsSelectable", &::WITextEntry::IsSelectable);
	classDef.def("SetSelectable", &::WITextEntry::SetSelectable);
	classDef.def("GetTextElement", &::WITextEntry::GetTextElement);
}

////////////////////////////////////

void Lua::WINumericEntry::register_class(luabind::class_<::WINumericEntry, luabind::bases<::WITextEntry, ::WIBase>> &classDef)
{
	classDef.def("SetMinValue", static_cast<void (::WINumericEntry::*)(int32_t)>(&::WINumericEntry::SetMinValue));
	classDef.def("SetMinValue", static_cast<void (::WINumericEntry::*)()>(&::WINumericEntry::SetMinValue));
	classDef.def("SetMaxValue", static_cast<void (::WINumericEntry::*)(int32_t)>(&::WINumericEntry::SetMaxValue));
	classDef.def("SetMaxValue", static_cast<void (::WINumericEntry::*)()>(&::WINumericEntry::SetMaxValue));
	classDef.def("SetRange", &::WINumericEntry::SetRange);
	classDef.def("GetMinValue", &::WINumericEntry::GetMinValue);
	classDef.def("GetMaxValue", &::WINumericEntry::GetMaxValue);
}

////////////////////////////////////

void Lua::WIOutlinedRect::register_class(luabind::class_<::WIOutlinedRect, ::WIBase> &classDef)
{
	classDef.def("SetOutlineWidth", &::WIOutlinedRect::SetOutlineWidth);
	classDef.def("GetOutlineWidth", &::WIOutlinedRect::GetOutlineWidth);
}

void Lua::WILine::register_class(luabind::class_<::WILine, ::WIBase> &classDef)
{
	classDef.def("SetLineWidth", &::WILine::SetLineWidth);
	classDef.def("GetLineWidth", &::WILine::GetLineWidth);
	classDef.def("SetStartPos", &SetStartPos);
	classDef.def("GetStartPos", &GetStartPos);
	classDef.def("SetEndPos", &SetEndPos);
	classDef.def("GetEndPos", &GetEndPos);
	classDef.def("SetStartColor", &::WILine::SetStartColor);
	classDef.def("SetEndColor", &::WILine::SetEndColor);
	classDef.def("GetStartColor", &::WILine::GetStartColor, luabind::copy_policy<0> {});
	classDef.def("GetEndColor", &::WILine::GetEndColor, luabind::copy_policy<0> {});
	classDef.def("GetStartPosProperty", &::WILine::GetStartPosProperty);
	classDef.def("GetEndPosProperty", &::WILine::GetEndPosProperty);
}

void Lua::WIRoundedRect::register_class(luabind::class_<::WIRoundedRect, luabind::bases<::WIShape, ::WIBase>> &classDef)
{
	classDef.def("GetRoundness", &::WIRoundedRect::GetRoundness);
	classDef.def("SetRoundness", &::WIRoundedRect::SetRoundness);
	classDef.def("SetCornerSize", &::WIRoundedRect::SetCornerSize);
	classDef.def("GetCornerSize", &::WIRoundedRect::GetCornerSize);
	classDef.def("SetRoundTopRight", &::WIRoundedRect::SetRoundTopRight);
	classDef.def("SetRoundTopLeft", &::WIRoundedRect::SetRoundTopLeft);
	classDef.def("SetRoundBottomLeft", &::WIRoundedRect::SetRoundBottomLeft);
	classDef.def("SetRoundBottomRight", &::WIRoundedRect::SetRoundBottomRight);
	classDef.def("IsTopRightRound", &::WIRoundedRect::IsTopRightRound);
	classDef.def("IsTopLeftRound", &::WIRoundedRect::IsTopLeftRound);
	classDef.def("IsBottomLeftRound", &::WIRoundedRect::IsBottomLeftRound);
	classDef.def("IsBottomRightRound", &::WIRoundedRect::IsBottomRightRound);
}

void Lua::WIRoundedTexturedRect::register_class(luabind::class_<::WIRoundedTexturedRect, luabind::bases<::WITexturedShape, ::WIShape, ::WIBase>> &classDef)
{
	classDef.def("GetRoundness", &::WIRoundedTexturedRect::GetRoundness);
	classDef.def("SetRoundness", &::WIRoundedTexturedRect::SetRoundness);
	classDef.def("SetCornerSize", &::WIRoundedTexturedRect::SetCornerSize);
	classDef.def("GetCornerSize", &::WIRoundedTexturedRect::GetCornerSize);
	classDef.def("SetRoundTopRight", &::WIRoundedTexturedRect::SetRoundTopRight);
	classDef.def("SetRoundTopLeft", &::WIRoundedTexturedRect::SetRoundTopLeft);
	classDef.def("SetRoundBottomLeft", &::WIRoundedTexturedRect::SetRoundBottomLeft);
	classDef.def("SetRoundBottomRight", &::WIRoundedTexturedRect::SetRoundBottomRight);
	classDef.def("IsTopRightRound", &::WIRoundedTexturedRect::IsTopRightRound);
	classDef.def("IsTopLeftRound", &::WIRoundedTexturedRect::IsTopLeftRound);
	classDef.def("IsBottomLeftRound", &::WIRoundedTexturedRect::IsBottomLeftRound);
	classDef.def("IsBottomRightRound", &::WIRoundedTexturedRect::IsBottomRightRound);
}

void Lua::WIScrollBar::register_class(luabind::class_<::WIScrollBar, ::WIBase> &classDef)
{
	classDef.def("GetScrollAmount", &::WIScrollBar::GetScrollAmount);
	classDef.def("SetScrollAmount", &::WIScrollBar::SetScrollAmount);
	classDef.def("GetScrollOffset", &::WIScrollBar::GetScrollOffset);
	classDef.def("SetScrollOffset", &::WIScrollBar::SetScrollOffset);
	classDef.def("AddScrollOffset", &::WIScrollBar::AddScrollOffset);
	classDef.def("SetUp", &::WIScrollBar::SetUp);
	classDef.def("SetHorizontal", &::WIScrollBar::SetHorizontal);
	classDef.def("IsHorizontal", &::WIScrollBar::IsHorizontal);
	classDef.def("IsVertical", &::WIScrollBar::IsVertical);
}

bool Lua::WIBase::IsValid(lua_State *l, ::WIBase *hPanel) { return hPanel != nullptr; }

void Lua::WIBase::TrapFocus(lua_State *l, ::WIBase &hPanel, bool bTrap) { hPanel.TrapFocus(bTrap); }
void Lua::WIBase::TrapFocus(lua_State *l, ::WIBase &hPanel) { TrapFocus(l, hPanel, true); }
void Lua::WIBase::SetPos(lua_State *l, ::WIBase &hPanel, Vector2 pos) { hPanel.SetPos(CInt32(pos.x), CInt32(pos.y)); }
void Lua::WIBase::SetPos(lua_State *l, ::WIBase &hPanel, float x, float y) { hPanel.SetPos(CInt32(x), CInt32(y)); }
void Lua::WIBase::SetAbsolutePos(lua_State *l, ::WIBase &hPanel, Vector2 pos) { hPanel.SetAbsolutePos(Vector2i(pos.x, pos.y)); }
void Lua::WIBase::SetAbsolutePos(lua_State *l, ::WIBase &hPanel, float x, float y) { hPanel.SetAbsolutePos(Vector2i(x, y)); }
void Lua::WIBase::SetColor(lua_State *l, ::WIBase &hPanel, Color col) { hPanel.SetColor(col.r / 255.f, col.g / 255.f, col.b / 255.f, col.a / 255.f); }
void Lua::WIBase::GetAlpha(lua_State *l, ::WIBase &hPanel) { Lua::PushNumber(l, hPanel.GetAlpha() * 255); }
void Lua::WIBase::SetAlpha(lua_State *l, ::WIBase &hPanel, float alpha) { hPanel.SetAlpha(alpha / 255.f); }
void Lua::WIBase::SetSize(lua_State *l, ::WIBase &hPanel, Vector2 size) { hPanel.SetSize(CInt32(size.x), CInt32(size.y)); }
void Lua::WIBase::SetSize(lua_State *l, ::WIBase &hPanel, float x, float y) { hPanel.SetSize(CInt32(x), CInt32(y)); }
void Lua::WIBase::Wrap(lua_State *l, ::WIBase &hPanel, const std::string &wrapperClassName)
{
	auto *el = c_game->CreateGUIElement(wrapperClassName);
	if(el == nullptr)
		return;
	auto hWrapper = el->GetHandle();
	if(hPanel.Wrap(*hWrapper.get()) == false) {
		el->RemoveSafely();
		return;
	}
	auto o = WGUILuaInterface::GetLuaObject(l, *el);
	o.push(l);
}
void Lua::WIBase::ClearParent(lua_State *l, ::WIBase &hPanel) { hPanel.SetParent(nullptr); }
void Lua::WIBase::ResetParent(lua_State *l, ::WIBase &hPanel) { hPanel.SetParent(WGUI::GetInstance().GetBaseElement()); }
void Lua::WIBase::GetChildren(lua_State *l, ::WIBase &hPanel, std::string className)
{

	std::vector<WIHandle> *children = hPanel.GetChildren();
	int table = Lua::CreateTable(l);
	unsigned int c = 1;
	for(unsigned int i = 0; i < children->size(); i++) {
		WIHandle &hChild = (*children)[i];
		if(hChild.IsValid()) {
			auto *pChild = hChild.get();
			if(pChild->GetClass() == className) {
				auto oChild = WGUILuaInterface::GetLuaObject(l, *pChild);
				Lua::PushInt(l, c);
				oChild.push(l);
				Lua::SetTableValue(l, table);
				c++;
			}
		}
	}
}
void Lua::WIBase::GetChild(lua_State *l, ::WIBase &hPanel, unsigned int idx)
{

	auto *el = hPanel.GetChild(idx);
	if(el == NULL)
		return;
	auto oChild = WGUILuaInterface::GetLuaObject(l, *el);
	oChild.push(l);
}
void Lua::WIBase::GetChild(lua_State *l, ::WIBase &hPanel, std::string className, unsigned int idx)
{

	auto *el = hPanel.GetChild(className, idx);
	if(el == NULL)
		return;
	auto oChild = WGUILuaInterface::GetLuaObject(l, *el);
	oChild.push(l);
}
void Lua::WIBase::PosInBounds(lua_State *l, ::WIBase &hPanel, Vector2 pos) { lua_pushboolean(l, hPanel.PosInBounds(CInt32(pos.x), CInt32(pos.y))); }
void Lua::WIBase::GetMousePos(lua_State *l, ::WIBase &hPanel)
{

	int x, y;
	hPanel.GetMousePos(&x, &y);
	luabind::object(l, Vector2(x, y)).push(l);
}
void Lua::WIBase::Draw(lua_State *l, ::WIBase &hPanel, const ::WIBase::DrawInfo &drawInfo, wgui::DrawState &drawState) { hPanel.Draw(drawInfo, drawState); }
void Lua::WIBase::Draw(lua_State *l, ::WIBase &hPanel, const ::WIBase::DrawInfo &drawInfo, wgui::DrawState &drawState, const Vector2i &scissorOffset, const Vector2i &scissorSize) { hPanel.Draw(drawInfo, drawState, Vector2i {}, scissorOffset, scissorSize, hPanel.GetScale()); }
void Lua::WIBase::Draw(lua_State *l, ::WIBase &hPanel, const ::WIBase::DrawInfo &drawInfo, wgui::DrawState &drawState, const Vector2i &scissorOffset, const Vector2i &scissorSize, const Vector2i &offsetParent)
{

	hPanel.Draw(drawInfo, drawState, offsetParent, scissorOffset, scissorSize, hPanel.GetScale());
}
void Lua::WIBase::Draw(lua_State *l, ::WIBase &hPanel, const ::WIBase::DrawInfo &drawInfo, wgui::DrawState &drawState, const Vector2i &scissorOffset, const Vector2i &scissorSize, const Vector2i &offsetParent, const Vector2 &scale)
{

	hPanel.Draw(drawInfo, drawState, offsetParent, scissorOffset, scissorSize, scale);
}
void Lua::WIBase::GetX(lua_State *l, ::WIBase &hPanel)
{

	Vector2i pos = hPanel.GetPos();
	Lua::PushInt(l, pos.x);
}
void Lua::WIBase::GetY(lua_State *l, ::WIBase &hPanel)
{

	Vector2i pos = hPanel.GetPos();
	Lua::PushInt(l, pos.y);
}
void Lua::WIBase::SetX(lua_State *l, ::WIBase &hPanel, float x)
{

	Vector2i pos = hPanel.GetPos();
	hPanel.SetPos(Vector2i(x, pos.y));
}
void Lua::WIBase::SetY(lua_State *l, ::WIBase &hPanel, float y)
{

	Vector2i pos = hPanel.GetPos();
	hPanel.SetPos(Vector2i(pos.x, y));
}
void Lua::WIBase::SetWidth(lua_State *l, ::WIBase &hPanel, float w)
{

	Vector2i size = hPanel.GetSize();
	hPanel.SetSize(Vector2i(w, size.y));
}
void Lua::WIBase::SetHeight(lua_State *l, ::WIBase &hPanel, float h)
{

	Vector2i size = hPanel.GetSize();
	hPanel.SetSize(Vector2i(size.x, h));
}

struct LuaCallbacks {
	struct CallbackInfo {
		CallbackInfo(lua_State *l, const CallbackHandle &_hCallback, const luabind::object &f) : luaState(l), hCallback(_hCallback), luaFunction(f) {}
		CallbackHandle hCallback;
		luabind::object luaFunction;
		lua_State *luaState;
	};
	~LuaCallbacks()
	{
		for(auto &pair : callbacks) {
			for(auto &cbInfo : pair.second) {
				if(cbInfo.hCallback.IsValid() == false)
					continue;
				cbInfo.hCallback.Remove();
			}
		}
	}
	std::unordered_map<std::string, std::vector<CallbackInfo>> callbacks;
};

static std::unordered_map<util::Hash, Lua::gui::LUA_CALLBACK> g_uiCallbacks;
static util::Hash get_gui_callback_hash(const std::string &className, const std::string &callbackName)
{
	util::Hash hash = 0;
	hash = util::hash_combine<size_t>(hash, std::hash<std::string> {}(className));
	hash = util::hash_combine<size_t>(hash, std::hash<std::string> {}(callbackName));
	return hash;
}
void Lua::gui::register_lua_callback(std::string className, std::string callbackName, LUA_CALLBACK fCb)
{
	ustring::to_lower(className);
	ustring::to_lower(callbackName);
	g_uiCallbacks[get_gui_callback_hash(className, callbackName)] = fCb;
}

void Lua::gui::clear_lua_callbacks(lua_State *l)
{
	auto &wgui = WGUI::GetInstance();
	std::function<void(::WIBase &)> fIterateElements = nullptr;
	fIterateElements = [&fIterateElements, l](::WIBase &el) {
		auto callbackPtr = std::static_pointer_cast<LuaCallbacks>(el.GetUserData4()); // User data 4 contains lua callbacks
		if(callbackPtr != nullptr) {
			for(auto &pair : callbackPtr->callbacks) {
				for(auto it = pair.second.begin(); it != pair.second.end();) {
					auto &cbInfo = *it;
					if(cbInfo.luaState != l) {
						++it;
						continue;
					}
					if(cbInfo.hCallback.IsValid())
						cbInfo.hCallback.Remove();
					it = pair.second.erase(it);
				}
			}
		}
		for(auto &hChild : *el.GetChildren()) {
			if(hChild.IsValid() == false)
				continue;
			fIterateElements(*hChild.get());
		}
	};
	auto *elBase = wgui.GetBaseElement();
	if(elBase != nullptr)
		fIterateElements(*elBase);
}

namespace Lua {
	namespace WIBase {
		template<typename... TARGS>
		void CallCallbacks(lua_State *l, ::WIBase &hPanel, std::string name, TARGS... args)
		{

			auto callbackPtr = std::static_pointer_cast<LuaCallbacks>(hPanel.GetUserData4());
			if(callbackPtr == nullptr)
				return;
			ustring::to_lower(name);
			auto itCallbacks = callbackPtr->callbacks.find(name);
			if(itCallbacks == callbackPtr->callbacks.end())
				return;
			uint32_t argOffset = 3;
			auto numArgs = Lua::GetStackTop(l) - argOffset + 1;
			auto &callbacks = itCallbacks->second;
			std::vector<LuaCallbacks::CallbackInfo> tmp;
			tmp = std::move(callbacks); // Move callbacks to tmp, in case new callbacks are added while we're executing the current callbacks (which would mess up our iterator)
			callbacks.clear();
			for(auto it = tmp.begin(); it != tmp.end();) {
				auto &cbInfo = *it;
				if(!cbInfo.hCallback.IsValid())
					it = tmp.erase(it);
				else if(cbInfo.luaState == l) {
					auto &o = cbInfo.luaFunction;
					auto bReturn = false;
					Lua::Execute(l, [l, &o, &hPanel, numArgs, argOffset, &bReturn, &name](int (*traceback)(lua_State *l)) {
						auto n = Lua::GetStackTop(l);
						auto r = Lua::CallFunction(
						  l,
						  [&o, &hPanel, numArgs, argOffset](lua_State *l) {
							  o.push(l);
							  auto obj = WGUILuaInterface::GetLuaObject(l, hPanel);
							  obj.push(l);
							  for(auto i = decltype(numArgs) {0}; i < numArgs; ++i) {
								  auto arg = argOffset + i;
								  Lua::PushValue(l, arg);
							  }
							  return Lua::StatusCode::Ok;
						  },
						  LUA_MULTRET);
						if(r == Lua::StatusCode::Ok) {
							auto numResults = Lua::GetStackTop(l) - n;
							if(numResults > 0)
								bReturn = true;
						}
						return r;
					});
					if(bReturn == true)
						break;
					++it;
				}
			}
			// LuaCallbacks map may have changed, we need to retrieve the callback list again
			auto &callbacksNew = callbackPtr->callbacks[name];
			for(auto &cb : callbacksNew)
				tmp.push_back(std::move(cb));
			callbacksNew = std::move(tmp);
		}
	};
};
void Lua::WIBase::CallCallbacks(lua_State *l, ::WIBase &hPanel, std::string name) { CallCallbacks<>(l, hPanel, name); }
void Lua::WIBase::CallCallbacks(lua_State *l, ::WIBase &hPanel, std::string name, luabind::object o1) { CallCallbacks<luabind::object>(l, hPanel, name, o1); }
void Lua::WIBase::CallCallbacks(lua_State *l, ::WIBase &hPanel, std::string name, luabind::object o1, luabind::object o2) { CallCallbacks<luabind::object, luabind::object>(l, hPanel, name, o1, o2); }
void Lua::WIBase::CallCallbacks(lua_State *l, ::WIBase &hPanel, std::string name, luabind::object o1, luabind::object o2, luabind::object o3) { CallCallbacks<luabind::object, luabind::object, luabind::object>(l, hPanel, name, o1, o2, o3); }
void Lua::WIBase::CallCallbacks(lua_State *l, ::WIBase &hPanel, std::string name, luabind::object o1, luabind::object o2, luabind::object o3, luabind::object o4) { CallCallbacks<luabind::object, luabind::object, luabind::object, luabind::object>(l, hPanel, name, o1, o2, o3, o4); }
void Lua::WIBase::CallCallbacks(lua_State *l, ::WIBase &hPanel, std::string name, luabind::object o1, luabind::object o2, luabind::object o3, luabind::object o4, luabind::object o5)
{
	CallCallbacks<luabind::object, luabind::object, luabind::object, luabind::object, luabind::object>(l, hPanel, name, o1, o2, o3, o4, o5);
}
void Lua::WIBase::CallCallbacks(lua_State *l, ::WIBase &hPanel, std::string name, luabind::object o1, luabind::object o2, luabind::object o3, luabind::object o4, luabind::object o5, luabind::object o6)
{
	CallCallbacks<luabind::object, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object>(l, hPanel, name, o1, o2, o3, o4, o5, o6);
}
void Lua::WIBase::CallCallbacks(lua_State *l, ::WIBase &hPanel, std::string name, luabind::object o1, luabind::object o2, luabind::object o3, luabind::object o4, luabind::object o5, luabind::object o6, luabind::object o7)
{
	CallCallbacks<luabind::object, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object>(l, hPanel, name, o1, o2, o3, o4, o5, o6, o7);
}
void Lua::WIBase::CallCallbacks(lua_State *l, ::WIBase &hPanel, std::string name, luabind::object o1, luabind::object o2, luabind::object o3, luabind::object o4, luabind::object o5, luabind::object o6, luabind::object o7, luabind::object o8)
{
	CallCallbacks<luabind::object, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object>(l, hPanel, name, o1, o2, o3, o4, o5, o6, o7, o8);
}
CallbackHandle Lua::WIBase::AddCallback(lua_State *l, ::WIBase &panel, std::string name, Lua::func<void> o)
{
	CallbackHandle hCallback {};
	ustring::to_lower(name);

	auto callbackPtr = std::static_pointer_cast<LuaCallbacks>(panel.GetUserData4());
	if(callbackPtr == nullptr) {
		callbackPtr = std::make_shared<LuaCallbacks>();
		panel.SetUserData4(callbackPtr);
	}
	auto hPanel = panel.GetHandle();
	auto it = g_uiCallbacks.find(get_gui_callback_hash(panel.GetClass(), name));
	if(it != g_uiCallbacks.end()) {
		hCallback = it->second(panel, l, [l, o, &panel](const std::function<void()> &pushArgs) {
			Lua::CallFunction(
			  l,
			  [&pushArgs, &o, &panel](lua_State *l) mutable {
				  o.push(l);

				  auto obj = WGUILuaInterface::GetLuaObject(l, panel);
				  obj.push(l);

				  pushArgs();
				  return Lua::StatusCode::Ok;
			  },
			  0);
		});
	}
	else if(name == "ontextchanged") {
		if(ustring::compare(panel.GetClass(), std::string {"witext"}, false)) {
			hCallback = FunctionCallback<void, std::reference_wrapper<const pragma::string::Utf8String>>::Create([l, hPanel, o](std::reference_wrapper<const pragma::string::Utf8String> text) mutable {
				if(!hPanel.IsValid())
					return;
				Lua::CallFunction(
				  l,
				  [&o, hPanel, text](lua_State *l) mutable {
					  o.push(l);

					  auto obj = WGUILuaInterface::GetLuaObject(l, *hPanel.get());
					  obj.push(l);
					  Lua::PushString(l, text.get().cpp_str());
					  return Lua::StatusCode::Ok;
				  },
				  0);
			});
		}
		else {
			hCallback = FunctionCallback<void, std::reference_wrapper<const pragma::string::Utf8String>, bool>::Create([l, hPanel, o](std::reference_wrapper<const pragma::string::Utf8String> text, bool changedByUser) mutable {
				if(!hPanel.IsValid())
					return;
				Lua::CallFunction(
				  l,
				  [&o, hPanel, text, changedByUser](lua_State *l) mutable {
					  o.push(l);

					  auto obj = WGUILuaInterface::GetLuaObject(l, *hPanel.get());
					  obj.push(l);
					  Lua::PushString(l, text.get().cpp_str());
					  Lua::PushBool(l, changedByUser);
					  return Lua::StatusCode::Ok;
				  },
				  0);
			});
		}
	}
	else if(name == "handlelinktagaction") {
		hCallback = FunctionCallback<::util::EventReply, std::string>::CreateWithOptionalReturn([l, hPanel, o](::util::EventReply *reply, std::string arg) mutable -> CallbackReturnType {
			if(!hPanel.IsValid())
				return CallbackReturnType::NoReturnValue;
			if(Lua::CallFunction(
			     l,
			     [&o, hPanel, &arg](lua_State *l) mutable {
				     o.push(l);

				     auto obj = WGUILuaInterface::GetLuaObject(l, *hPanel.get());
				     obj.push(l);

				     Lua::PushString(l, arg);

				     return Lua::StatusCode::Ok;
			     },
			     1)
			  == Lua::StatusCode::Ok) {
				if(Lua::IsSet(l, -1) == false) {
					Lua::Pop(l, 1);
					return CallbackReturnType::NoReturnValue;
				}
				auto result = static_cast<::util::EventReply>(Lua::CheckInt(l, -1));
				Lua::Pop(l, 1);
				*reply = result;
				return CallbackReturnType::NoReturnValue; // We'll always return 'NoReturnValue' to allow other callbacks for this element to be executed as well
			}
			return CallbackReturnType::NoReturnValue;
		});
	}
	else if(name == "onscrolloffsetchanged") {
		hCallback = FunctionCallback<void, uint32_t>::Create([l, hPanel, o](uint32_t offset) mutable {
			if(!hPanel.IsValid())
				return;
			Lua::CallFunction(
			  l,
			  [&o, hPanel, offset](lua_State *l) mutable {
				  o.push(l);

				  auto obj = WGUILuaInterface::GetLuaObject(l, *hPanel.get());
				  obj.push(l);
				  Lua::PushInt(l, offset);
				  return Lua::StatusCode::Ok;
			  },
			  0);
		});
	}
	else if(name == "oncharevent") {
		hCallback = FunctionCallback<::util::EventReply, int, GLFW::Modifier>::CreateWithOptionalReturn([l, hPanel, o](::util::EventReply *reply, int c, GLFW::Modifier mods) mutable -> CallbackReturnType {
			if(!hPanel.IsValid())
				return CallbackReturnType::NoReturnValue;
			if(Lua::CallFunction(
			     l,
			     [&o, hPanel, c, mods](lua_State *l) mutable {
				     o.push(l);

				     auto obj = WGUILuaInterface::GetLuaObject(l, *hPanel.get());
				     obj.push(l);
				     Lua::PushString(l, std::string(1, static_cast<char>(c)));
				     Lua::PushInt(l, umath::to_integral(mods));
				     return Lua::StatusCode::Ok;
			     },
			     1)
			  == Lua::StatusCode::Ok) {
				if(Lua::IsSet(l, -1) == false) {
					Lua::Pop(l, 1);
					return CallbackReturnType::NoReturnValue;
				}
				auto result = static_cast<::util::EventReply>(Lua::CheckInt(l, -1));
				Lua::Pop(l, 1);
				*reply = result;
				return CallbackReturnType::NoReturnValue; // We'll always return 'NoReturnValue' to allow other callbacks for this element to be executed as well
			}
			return CallbackReturnType::NoReturnValue;
		});
	}
	else if(name == "onkeyevent") {
		hCallback = FunctionCallback<::util::EventReply, GLFW::Key, int, GLFW::KeyState, GLFW::Modifier>::CreateWithOptionalReturn([l, hPanel, o](::util::EventReply *reply, GLFW::Key key, int, GLFW::KeyState action, GLFW::Modifier mods) mutable -> CallbackReturnType {
			if(!hPanel.IsValid())
				return CallbackReturnType::NoReturnValue;
			if(Lua::CallFunction(
			     l,
			     [&o, hPanel, key, action, mods](lua_State *l) mutable {
				     o.push(l);

				     auto obj = WGUILuaInterface::GetLuaObject(l, *hPanel.get());
				     obj.push(l);
				     Lua::PushInt(l, umath::to_integral(key));
				     Lua::PushInt(l, umath::to_integral(action));
				     Lua::PushInt(l, umath::to_integral(mods));
				     return Lua::StatusCode::Ok;
			     },
			     1)
			  == Lua::StatusCode::Ok) {
				if(Lua::IsSet(l, -1) == false) {
					Lua::Pop(l, 1);
					return CallbackReturnType::NoReturnValue;
				}
				auto result = static_cast<::util::EventReply>(Lua::CheckInt(l, -1));
				Lua::Pop(l, 1);
				*reply = result;
				return CallbackReturnType::NoReturnValue; // We'll always return 'NoReturnValue' to allow other callbacks for this element to be executed as well
			}
			return CallbackReturnType::NoReturnValue;
		});
	}
	else if(name == "onmouseevent") {
		hCallback = FunctionCallback<::util::EventReply, GLFW::MouseButton, GLFW::KeyState, GLFW::Modifier>::CreateWithOptionalReturn([l, hPanel, o](::util::EventReply *reply, GLFW::MouseButton button, GLFW::KeyState action, GLFW::Modifier mods) mutable -> CallbackReturnType {
			if(!hPanel.IsValid())
				return CallbackReturnType::NoReturnValue;
			if(Lua::CallFunction(
			     l,
			     [&o, hPanel, button, action, mods](lua_State *l) mutable {
				     o.push(l);

				     auto obj = WGUILuaInterface::GetLuaObject(l, *hPanel.get());
				     obj.push(l);
				     Lua::PushInt(l, umath::to_integral(button));
				     Lua::PushInt(l, umath::to_integral(action));
				     Lua::PushInt(l, umath::to_integral(mods));
				     return Lua::StatusCode::Ok;
			     },
			     1)
			  == Lua::StatusCode::Ok) {
				if(Lua::IsSet(l, -1) == false) {
					Lua::Pop(l, 1);
					return CallbackReturnType::NoReturnValue;
				}
				auto result = static_cast<::util::EventReply>(Lua::CheckInt(l, -1));
				Lua::Pop(l, 1);
				*reply = result;
				return CallbackReturnType::NoReturnValue; // We'll always return 'NoReturnValue' to allow other callbacks for this element to be executed as well
			}
			return CallbackReturnType::NoReturnValue;
		});
	}
	else if(name == "oncursormoved") {
		hCallback = FunctionCallback<void, int32_t, int32_t>::Create([l, hPanel, o](int32_t x, int32_t y) mutable {
			if(!hPanel.IsValid())
				return;
			Lua::CallFunction(l, [&o, hPanel, x, y](lua_State *l) mutable {
				o.push(l);

				auto obj = WGUILuaInterface::GetLuaObject(l, *hPanel.get());
				obj.push(l);
				Lua::PushInt(l, x);
				Lua::PushInt(l, y);
				return Lua::StatusCode::Ok;
			});
		});
	}
	else if(name == "onchildadded") {
		hCallback = FunctionCallback<void, ::WIBase *>::Create([l, hPanel, o](::WIBase *el) mutable {
			if(!hPanel.IsValid())
				return;
			Lua::CallFunction(l, [&o, hPanel, el](lua_State *l) mutable {
				o.push(l);

				auto obj = WGUILuaInterface::GetLuaObject(l, *hPanel.get());
				obj.push(l);
				if(el) {
					auto objEl = WGUILuaInterface::GetLuaObject(l, *el);
					objEl.push(l);
				}
				return Lua::StatusCode::Ok;
			});
		});
	}
	else if(name == "onshowtooltip") {
		hCallback = FunctionCallback<void, ::WITooltip *>::Create([l, hPanel, o](::WITooltip *el) mutable {
			if(!hPanel.IsValid())
				return;
			Lua::CallFunction(l, [&o, hPanel, el](lua_State *l) mutable {
				o.push(l);

				auto obj = WGUILuaInterface::GetLuaObject(l, *hPanel.get());
				obj.push(l);
				if(el) {
					auto objEl = WGUILuaInterface::GetLuaObject(l, *el);
					objEl.push(l);
				}
				return Lua::StatusCode::Ok;
			});
		});
	}
	else if(name == "onchildremoved") {
		hCallback = FunctionCallback<void, ::WIBase *>::Create([l, hPanel, o](::WIBase *el) mutable {
			if(!hPanel.IsValid())
				return;
			Lua::CallFunction(l, [&o, hPanel, el](lua_State *l) mutable {
				o.push(l);

				auto obj = WGUILuaInterface::GetLuaObject(l, *hPanel.get());
				obj.push(l);
				if(el) {
					auto objEl = WGUILuaInterface::GetLuaObject(l, *el);
					objEl.push(l);
				}
				return Lua::StatusCode::Ok;
			});
		});
	}
	else if(name == "onmousepressed") {
		hCallback = FunctionCallback<::util::EventReply>::CreateWithOptionalReturn([l, hPanel, o](::util::EventReply *reply) mutable -> CallbackReturnType {
			if(!hPanel.IsValid())
				return CallbackReturnType::NoReturnValue;
			if(Lua::CallFunction(
			     l,
			     [&o, hPanel](lua_State *l) mutable {
				     o.push(l);

				     auto obj = WGUILuaInterface::GetLuaObject(l, *hPanel.get());
				     obj.push(l);
				     return Lua::StatusCode::Ok;
			     },
			     1)
			  == Lua::StatusCode::Ok) {
				if(Lua::IsSet(l, -1) == false) {
					Lua::Pop(l, 1);
					return CallbackReturnType::NoReturnValue;
				}
				auto result = static_cast<::util::EventReply>(Lua::CheckInt(l, -1));
				Lua::Pop(l, 1);
				*reply = result;
				return CallbackReturnType::NoReturnValue; // We'll always return 'NoReturnValue' to allow other callbacks for this element to be executed as well
			}
			return CallbackReturnType::NoReturnValue;
		});
	}
	else if(name == "onmousereleased") {
		hCallback = FunctionCallback<::util::EventReply>::CreateWithOptionalReturn([l, hPanel, o](::util::EventReply *reply) mutable -> CallbackReturnType {
			if(!hPanel.IsValid())
				return CallbackReturnType::NoReturnValue;
			if(Lua::CallFunction(
			     l,
			     [&o, hPanel](lua_State *l) mutable {
				     o.push(l);

				     auto obj = WGUILuaInterface::GetLuaObject(l, *hPanel.get());
				     obj.push(l);
				     return Lua::StatusCode::Ok;
			     },
			     1)
			  == Lua::StatusCode::Ok) {
				if(Lua::IsSet(l, -1) == false) {
					Lua::Pop(l, 1);
					return CallbackReturnType::NoReturnValue;
				}
				auto result = static_cast<::util::EventReply>(Lua::CheckInt(l, -1));
				Lua::Pop(l, 1);
				*reply = result;
				return CallbackReturnType::NoReturnValue; // We'll always return 'NoReturnValue' to allow other callbacks for this element to be executed as well
			}
			return CallbackReturnType::NoReturnValue;
		});
	}
	else if(name == "ondoubleclick") {
		hCallback = FunctionCallback<::util::EventReply>::CreateWithOptionalReturn([l, hPanel, o](::util::EventReply *reply) mutable -> CallbackReturnType {
			if(!hPanel.IsValid())
				return CallbackReturnType::NoReturnValue;
			if(Lua::CallFunction(
			     l,
			     [&o, hPanel](lua_State *l) mutable {
				     o.push(l);

				     auto obj = WGUILuaInterface::GetLuaObject(l, *hPanel.get());
				     obj.push(l);
				     return Lua::StatusCode::Ok;
			     },
			     1)
			  == Lua::StatusCode::Ok) {
				if(Lua::IsSet(l, -1) == false) {
					Lua::Pop(l, 1);
					return CallbackReturnType::NoReturnValue;
				}
				auto result = static_cast<::util::EventReply>(Lua::CheckInt(l, -1));
				Lua::Pop(l, 1);
				*reply = result;
				return CallbackReturnType::NoReturnValue; // We'll always return 'NoReturnValue' to allow other callbacks for this element to be executed as well
			}
			return CallbackReturnType::NoReturnValue;
		});
	}
	else if(name == "onjoystickevent") {
		hCallback = FunctionCallback<::util::EventReply, std::reference_wrapper<const GLFW::Joystick>, uint32_t, GLFW::KeyState>::CreateWithOptionalReturn(
		  [l, hPanel, o](::util::EventReply *reply, std::reference_wrapper<const GLFW::Joystick> joystick, uint32_t key, GLFW::KeyState state) mutable -> CallbackReturnType {
			  if(!hPanel.IsValid())
				  return CallbackReturnType::NoReturnValue;
			  if(Lua::CallFunction(
			       l,
			       [&o, hPanel, key, state](lua_State *l) mutable {
				       o.push(l);

				       auto obj = WGUILuaInterface::GetLuaObject(l, *hPanel.get());
				       obj.push(l);
				       Lua::PushInt(l, key);
				       Lua::PushInt(l, umath::to_integral(state));
				       return Lua::StatusCode::Ok;
			       },
			       1)
			    == Lua::StatusCode::Ok) {
				  if(Lua::IsSet(l, -1) == false) {
					  Lua::Pop(l, 1);
					  return CallbackReturnType::NoReturnValue;
				  }
				  auto result = static_cast<::util::EventReply>(Lua::CheckInt(l, -1));
				  Lua::Pop(l, 1);
				  *reply = result;
				  return CallbackReturnType::NoReturnValue; // We'll always return 'NoReturnValue' to allow other callbacks for this element to be executed as well
			  }
			  return CallbackReturnType::NoReturnValue;
		  });
	}
	else if(name == "onscroll") {
		hCallback = FunctionCallback<::util::EventReply, Vector2, bool>::CreateWithOptionalReturn([l, hPanel, o](::util::EventReply *reply, Vector2 offset, bool offsetAsPixels) mutable -> CallbackReturnType {
			if(!hPanel.IsValid())
				return CallbackReturnType::NoReturnValue;
			if(Lua::CallFunction(
			     l,
			     [&o, hPanel, &offset, &offsetAsPixels](lua_State *l) mutable {
				     o.push(l);

				     auto obj = WGUILuaInterface::GetLuaObject(l, *hPanel.get());
				     obj.push(l);
				     Lua::PushNumber(l, offset.x);
				     Lua::PushNumber(l, offset.y);
				     Lua::PushBool(l, offsetAsPixels);
				     return Lua::StatusCode::Ok;
			     },
			     1)
			  == Lua::StatusCode::Ok) {
				if(Lua::IsSet(l, -1) == false) {
					Lua::Pop(l, 1);
					return CallbackReturnType::NoReturnValue;
				}
				auto result = static_cast<::util::EventReply>(Lua::CheckInt(l, -1));
				Lua::Pop(l, 1);
				*reply = result;
				return CallbackReturnType::NoReturnValue; // We'll always return 'NoReturnValue' to allow other callbacks for this element to be executed as well
			}
			return CallbackReturnType::NoReturnValue;
		});
	}
	else if(name == "onchange" && dynamic_cast<::WICheckbox *>(hPanel.get()) != nullptr) {
		hCallback = FunctionCallback<void, bool>::Create([l, hPanel, o](bool bChecked) mutable {
			if(!hPanel.IsValid())
				return ::util::EventReply::Unhandled;
			if(Lua::CallFunction(
			     l,
			     [&o, hPanel, bChecked](lua_State *l) mutable {
				     o.push(l);

				     auto obj = WGUILuaInterface::GetLuaObject(l, *hPanel.get());
				     obj.push(l);
				     Lua::PushBool(l, bChecked);
				     return Lua::StatusCode::Ok;
			     },
			     1)
			  == Lua::StatusCode::Ok) {
				auto result = Lua::IsSet(l, -1) ? static_cast<::util::EventReply>(Lua::CheckInt(l, -1)) : ::util::EventReply::Unhandled;
				Lua::Pop(l, 1);
				return result;
			}
			return ::util::EventReply::Unhandled;
		});
	}
	else if(name == "onoptionselected") {
		hCallback = FunctionCallback<void, uint32_t>::Create([l, hPanel, o](uint32_t idx) mutable {
			if(!hPanel.IsValid())
				return;
			Lua::CallFunction(
			  l,
			  [&o, hPanel, idx](lua_State *l) mutable {
				  o.push(l);

				  auto *p = hPanel.get();
				  auto obj = WGUILuaInterface::GetLuaObject(l, *p);
				  obj.push(l);

				  auto optIdx = (idx == std::numeric_limits<uint32_t>::max()) ? -1 : static_cast<int32_t>(idx);
				  Lua::PushInt(l, optIdx);
				  return Lua::StatusCode::Ok;
			  },
			  0);
		});
	}
	else if(name == "translatetransformposition") {
		hCallback = FunctionCallback<void, std::reference_wrapper<Vector2i>, bool>::Create([l, hPanel, o](std::reference_wrapper<Vector2i> pos, bool bDrag) mutable {
			if(!hPanel.IsValid())
				return;
			auto r = Lua::CallFunction(
			  l,
			  [&o, hPanel, pos, bDrag](lua_State *l) mutable {
				  o.push(l);

				  auto obj = WGUILuaInterface::GetLuaObject(l, *hPanel.get());
				  obj.push(l);
				  Lua::Push<Vector2i>(l, pos);
				  Lua::PushBool(l, bDrag);
				  return Lua::StatusCode::Ok;
			  },
			  1);
			if(r == Lua::StatusCode::Ok) {
				if(Lua::IsVector2i(l, -1))
					pos.get() = *Lua::CheckVector2i(l, -1);
				Lua::Pop(l, 1);
			}
		});
	}
	else if(name == "onchange") {
		hCallback = FunctionCallback<void, float, float>::Create([l, hPanel, o](float progress, float value) mutable {
			if(!hPanel.IsValid())
				return;
			Lua::CallFunction(
			  l,
			  [&o, hPanel, progress, value](lua_State *l) mutable {
				  o.push(l);

				  auto *p = hPanel.get();
				  auto obj = WGUILuaInterface::GetLuaObject(l, *p);
				  obj.push(l);

				  Lua::PushNumber(l, progress);
				  Lua::PushNumber(l, value);
				  return Lua::StatusCode::Ok;
			  },
			  0);
		});
	}
	else if(name == "onselectionchanged" && typeid(*hPanel.get()) == typeid(::WIDropDownMenuOption)) {
		hCallback = FunctionCallback<void, bool>::Create([l, hPanel, o](bool selected) mutable {
			if(!hPanel.IsValid())
				return;
			Lua::CallFunction(
			  l,
			  [&o, hPanel, selected](lua_State *l) mutable {
				  o.push(l);

				  auto *p = hPanel.get();
				  auto obj = WGUILuaInterface::GetLuaObject(l, *p);
				  obj.push(l);

				  Lua::PushBool(l, selected);
				  return Lua::StatusCode::Ok;
			  },
			  0);
		});
	}
	else if(name == "translatevalue") {
		hCallback = FunctionCallback<bool, float, std::reference_wrapper<std::string>>::Create([l, hPanel, o](float rawValue, std::reference_wrapper<std::string> value) mutable {
			if(!hPanel.IsValid())
				return false;
			auto retVal = false;
			auto r = Lua::CallFunction(
			  l,
			  [&o, hPanel, rawValue, value, &retVal](lua_State *l) mutable {
				  o.push(l);

				  auto *p = hPanel.get();
				  auto obj = WGUILuaInterface::GetLuaObject(l, *p);
				  obj.push(l);

				  Lua::PushNumber(l, rawValue);
				  return Lua::StatusCode::Ok;
			  },
			  1);
			if(r == Lua::StatusCode::Ok && Lua::IsSet(l, -1)) {
				value.get() = Lua::CheckString(l, -1);
				retVal = true;
			}
			return retVal;
		});
	}
	else {
		hCallback = FunctionCallback<>::Create([l, hPanel, o]() mutable {
			if(!hPanel.IsValid())
				return;
			Lua::CallFunction(
			  l,
			  [&o, hPanel](lua_State *l) mutable {
				  o.push(l);

				  auto obj = WGUILuaInterface::GetLuaObject(l, *hPanel.get());
				  obj.push(l);
				  return Lua::StatusCode::Ok;
			  },
			  0);
		});
	}
	auto hCallbackRet = panel.AddCallback(name, hCallback);
	auto itCallback = callbackPtr->callbacks.find(name);
	if(itCallback == callbackPtr->callbacks.end())
		itCallback = callbackPtr->callbacks.insert(decltype(callbackPtr->callbacks)::value_type(name, {})).first;
	itCallback->second.push_back({l, hCallback, o});
	return hCallbackRet;
}
void Lua::WIBase::FadeIn(lua_State *l, ::WIBase &hPanel, float tFadeIn, float alphaTarget) { hPanel.FadeIn(tFadeIn, alphaTarget / 255.f); }
void Lua::WIBase::FadeIn(lua_State *l, ::WIBase &hPanel, float tFadeIn) { Lua::WIBase::FadeIn(l, hPanel, tFadeIn, 255.f); }
static std::optional<Vector2> get_cursor_pos_override(WIRoot *elRoot)
{
	if(!elRoot)
		return {};
	return elRoot->GetCursorPosOverride();
}
static void restore_cursor_pos_override(WIRoot *elRoot, const std::optional<Vector2> &pos)
{
	if(!elRoot)
		return;
	if(pos)
		elRoot->SetCursorPosOverride(*pos);
	else
		elRoot->ClearCursorPosOverride();
}
void Lua::WIBase::InjectMouseMoveInput(lua_State *l, ::WIBase &hPanel, const Vector2 &mousePos)
{
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	debug::get_domain().BeginTask("inect_mouse_move_input");
	util::ScopeGuard sgVtune {[]() { debug::get_domain().EndTask(); }};
#endif
	auto *elRoot = hPanel.GetBaseRootElement();
	auto absPos = hPanel.GetAbsolutePos();
	auto origOverride = get_cursor_pos_override(elRoot);
	if(elRoot)
		elRoot->SetCursorPosOverride(Vector2 {static_cast<float>(absPos.x + mousePos.x), static_cast<float>(absPos.y + mousePos.y)});
	::util::ScopeGuard sg {[elRoot, &origOverride]() { restore_cursor_pos_override(elRoot, origOverride); }};
	hPanel.InjectMouseMoveInput(mousePos.x, mousePos.y);
}
::util::EventReply Lua::WIBase::InjectMouseInput(lua_State *l, ::WIBase &hPanel, const Vector2 &mousePos, int button, int action, int mods)
{
	auto *elRoot = hPanel.GetBaseRootElement();
	auto absPos = hPanel.GetAbsolutePos();
	auto origOverride = get_cursor_pos_override(elRoot);
	if(elRoot)
		elRoot->SetCursorPosOverride(Vector2 {static_cast<float>(absPos.x + mousePos.x), static_cast<float>(absPos.y + mousePos.y)});
	::util::ScopeGuard sg {[elRoot, &origOverride]() { restore_cursor_pos_override(elRoot, origOverride); }};
	return hPanel.InjectMouseInput(GLFW::MouseButton(button), GLFW::KeyState(action), GLFW::Modifier(mods));
}
::util::EventReply Lua::WIBase::InjectMouseInput(lua_State *l, ::WIBase &hPanel, const Vector2 &mousePos, int button, int action) { return InjectMouseInput(l, hPanel, mousePos, button, action, 0); }
::util::EventReply Lua::WIBase::InjectMouseClick(lua_State *l, ::WIBase &hPanel, const Vector2 &mousePos, int button, int mods)
{
	auto handled0 = InjectMouseInput(l, hPanel, mousePos, button, GLFW_PRESS, mods);
	auto handled1 = InjectMouseInput(l, hPanel, mousePos, button, GLFW_RELEASE, mods);
	if(handled1 == ::util::EventReply::Handled)
		handled0 = handled1;
	return handled0;
}
::util::EventReply Lua::WIBase::InjectMouseClick(lua_State *l, ::WIBase &hPanel, const Vector2 &mousePos, int button) { return InjectMouseClick(l, hPanel, mousePos, button, 0); }
::util::EventReply Lua::WIBase::InjectKeyboardInput(lua_State *l, ::WIBase &hPanel, int key, int action, int mods)
{
	return hPanel.InjectKeyboardInput(GLFW::Key(key), 0, GLFW::KeyState(action), GLFW::Modifier(mods));
	// Vulkan TODO
}
::util::EventReply Lua::WIBase::InjectKeyboardInput(lua_State *l, ::WIBase &hPanel, int key, int action) { return InjectKeyboardInput(l, hPanel, key, action, 0); }
::util::EventReply Lua::WIBase::InjectKeyPress(lua_State *l, ::WIBase &hPanel, int key, int mods)
{
	auto handled0 = InjectKeyboardInput(l, hPanel, key, GLFW_PRESS, mods);
	auto handled1 = InjectKeyboardInput(l, hPanel, key, GLFW_RELEASE, mods);
	if(handled1 == ::util::EventReply::Handled)
		handled0 = handled1;
	return handled0;
}
::util::EventReply Lua::WIBase::InjectKeyPress(lua_State *l, ::WIBase &hPanel, int key) { return InjectKeyPress(l, hPanel, key, 0); }
::util::EventReply Lua::WIBase::InjectCharInput(lua_State *l, ::WIBase &hPanel, std::string c, uint32_t mods)
{
	if(c.empty())
		return ::util::EventReply::Unhandled;
	const char *cStr = c.c_str();
	return hPanel.InjectCharInput(cStr[0], static_cast<GLFW::Modifier>(mods));
}
::util::EventReply Lua::WIBase::InjectCharInput(lua_State *l, ::WIBase &hPanel, std::string c)
{
	if(c.empty())
		return ::util::EventReply::Unhandled;
	const char *cStr = c.c_str();
	return hPanel.InjectCharInput(cStr[0]);
}
::util::EventReply Lua::WIBase::InjectScrollInput(lua_State *l, ::WIBase &hPanel, const Vector2 &mousePos, const Vector2 &offset, bool offsetAsPixels)
{
	auto *elRoot = hPanel.GetBaseRootElement();
	auto cursorPos = elRoot ? elRoot->GetCursorPos() : Vector2 {};
	auto absPos = hPanel.GetAbsolutePos();
	auto origOverride = get_cursor_pos_override(elRoot);
	if(elRoot)
		elRoot->SetCursorPosOverride(Vector2 {static_cast<float>(absPos.x + mousePos.x), static_cast<float>(absPos.y + mousePos.y)});
	auto result = hPanel.InjectScrollInput(offset, offsetAsPixels);
	if(elRoot)
		restore_cursor_pos_override(elRoot, origOverride);
	return result;
}
::util::EventReply Lua::WIBase::InjectScrollInput(lua_State *l, ::WIBase &hPanel, const Vector2 &mousePos, const Vector2 &offset) { return InjectScrollInput(l, hPanel, mousePos, offset, false); }
void Lua::WIBase::FindChildByName(lua_State *l, ::WIBase &hPanel, std::string name)
{
	auto *el = hPanel.FindChildByName(name);
	if(el == nullptr)
		return;
	auto oChild = WGUILuaInterface::GetLuaObject(l, *el);
	oChild.push(l);
}
void Lua::WIBase::FindChildrenByName(lua_State *l, ::WIBase &hPanel, std::string name)
{
	std::vector<WIHandle> children;
	hPanel.FindChildrenByName(name, children);
	int table = Lua::CreateTable(l);
	unsigned int c = 1;
	for(unsigned int i = 0; i < children.size(); i++) {
		WIHandle &hChild = children[i];
		if(hChild.IsValid()) {
			auto *pChild = hChild.get();
			auto oChild = WGUILuaInterface::GetLuaObject(l, *pChild);
			Lua::PushInt(l, c);
			oChild.push(l);
			Lua::SetTableValue(l, table);
			c++;
		}
	}
}
void Lua::WIBase::SetBounds(lua_State *l, ::WIBase &hPanel, const Vector2 &start, const Vector2 &end)
{
	auto &pos = start;
	auto size = end - start;
	hPanel.SetPos(pos);
	hPanel.SetSize(size);
}

////////////////////////////////////

void Lua::WIText::SetShadowColor(lua_State *l, ::WIText &hPanel, Color &col) { hPanel.SetShadowColor(Vector4(col.r / 255.f, col.g / 255.f, col.b / 255.f, col.a / 255.f)); }

void Lua::WIText::SetShadowOffset(lua_State *l, ::WIText &hPanel, Vector2 pos) { hPanel.SetShadowOffset(CInt32(pos.x), CInt32(pos.y)); }

void Lua::WIText::SetShadowXOffset(lua_State *l, ::WIText &hPanel, float x) { hPanel.SetShadowOffset(Vector2i(x, hPanel.GetShadowOffset()->y)); }

void Lua::WIText::SetShadowYOffset(lua_State *l, ::WIText &hPanel, float y) { hPanel.SetShadowOffset(Vector2i(hPanel.GetShadowOffset()->x, y)); }

Color Lua::WIText::GetShadowColor(lua_State *l, ::WIText &hPanel)
{

	Vector4 *col = hPanel.GetShadowColor();
	return Color(CInt16(col->r * 255), CInt16(col->g * 255), CInt16(col->b * 255), CInt16(col->a * 255));
}

Vector2 Lua::WIText::GetShadowOffset(lua_State *l, ::WIText &hPanel)
{
	Vector2i *offset = hPanel.GetShadowOffset();
	return Vector2(offset->x, offset->y);
}

float Lua::WIText::GetShadowXOffset(lua_State *l, ::WIText &hPanel) { return float(hPanel.GetShadowOffset()->x); }

float Lua::WIText::GetShadowYOffset(lua_State *l, ::WIText &hPanel) { return float(hPanel.GetShadowOffset()->y); }

////////////////////////////////////

void Lua::WILine::SetStartPos(lua_State *l, ::WILine &hPanel, Vector2 &pos) { hPanel.SetStartPos(Vector2i(pos.x, pos.y)); }
void Lua::WILine::SetEndPos(lua_State *l, ::WILine &hPanel, Vector2 &pos) { hPanel.SetEndPos(Vector2i(pos.x, pos.y)); }
Vector2 Lua::WILine::GetStartPos(lua_State *l, ::WILine &hPanel) { return hPanel.GetStartPos(); }
Vector2 Lua::WILine::GetEndPos(lua_State *l, ::WILine &hPanel)
{
	Vector2i &pos = hPanel.GetEndPos();
	return {pos.x, pos.y};
}

////////////////////////////////////

void Lua::WITransformable::register_class(luabind::class_<::WITransformable, ::WIBase> &classDef)
{
	classDef.def("SetDraggable", &::WITransformable::SetDraggable);
	classDef.def("SetResizable", &::WITransformable::SetResizable);
	classDef.def("IsDraggable", &::WITransformable::IsDraggable);
	classDef.def("IsResizable", &::WITransformable::IsResizable);
	classDef.def("SetMinWidth", &::WITransformable::SetMinWidth);
	classDef.def("SetMinHeight", &::WITransformable::SetMinHeight);
	classDef.def("SetMinSize", static_cast<void (::WITransformable::*)(int, int)>(&::WITransformable::SetMinSize));
	classDef.def("SetMinSize", static_cast<void (::WITransformable::*)(Vector2i)>(&::WITransformable::SetMinSize));
	classDef.def("GetMinWidth", &::WITransformable::GetMinWidth);
	classDef.def("GetMinHeight", &::WITransformable::GetMinHeight);
	classDef.def("GetMinSize", &::WITransformable::GetMinSize, luabind::copy_policy<0> {});
	classDef.def("SetMaxWidth", &::WITransformable::SetMaxWidth);
	classDef.def("SetMaxHeight", &::WITransformable::SetMaxHeight);
	classDef.def("SetMaxSize", static_cast<void (::WITransformable::*)(int, int)>(&::WITransformable::SetMaxSize));
	classDef.def("SetMaxSize", static_cast<void (::WITransformable::*)(Vector2i)>(&::WITransformable::SetMaxSize));
	classDef.def("GetMaxWidth", &::WITransformable::GetMaxWidth);
	classDef.def("GetMaxHeight", &::WITransformable::GetMaxHeight);
	classDef.def("GetMaxSize", &::WITransformable::GetMaxSize, luabind::copy_policy<0> {});
	classDef.def("Close", &::WITransformable::Close);
	classDef.def("GetDragArea", &::WITransformable::GetDragArea);
	classDef.def("IsBeingDragged", &::WITransformable::IsBeingDragged);
	classDef.def("IsBeingResized", &::WITransformable::IsBeingResized);
	classDef.def("SetDragBounds", &::WITransformable::SetDragBounds);
	classDef.def("GetDragBounds", &::WITransformable::GetDragBounds);
	classDef.def("SetResizeRatioLocked", &::WITransformable::SetResizeRatioLocked);
	classDef.def("IsResizeRatioLocked", &::WITransformable::IsResizeRatioLocked);
	classDef.def("AddSnapTarget", &::WITransformable::AddSnapTarget);
}

////////////////////////////////////

luabind::tableT<::WITreeListElement> Lua::WITreeListElement::GetItems(lua_State *l, ::WITreeListElement &hElement)
{
	auto t = luabind::newtable(l);
	auto &items = hElement.GetItems();
	int32_t idx = 1;
	for(auto i = decltype(items.size()) {0}; i < items.size(); ++i) {
		auto &hItem = items[i];
		if(hItem.IsValid() == false)
			continue;
		t[idx++] = WGUILuaInterface::GetLuaObject(l, const_cast<::WIBase &>(*hItem.get()));
	}
	return t;
}

////////////////////////////////////

luabind::tableT<::WITableRow> Lua::WITable::GetSelectedRows(lua_State *l, ::WITable &hTable)
{
	auto t = luabind::newtable(l);
	auto &rows = hTable.GetSelectedRows();
	uint32_t rowId = 1;
	for(auto &hRow : rows) {
		if(hRow.IsValid() == false)
			continue;
		t[rowId++] = WGUILuaInterface::GetLuaObject(l, const_cast<::WIBase &>(*hRow.get()));
	}
	return t;
}
luabind::tableT<::WITableRow> Lua::WITable::GetRows(lua_State *l, ::WITable &hTable)
{
	auto t = luabind::newtable(l);
	auto numRows = hTable.GetRowCount();
	uint32_t rowId = 1;
	for(auto i = decltype(numRows) {0}; i < numRows; ++i) {
		auto *pRow = hTable.GetRow(i);
		if(!pRow)
			continue;
		t[rowId++] = WGUILuaInterface::GetLuaObject(l, *pRow);
	}
	return t;
}
