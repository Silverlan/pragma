// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :scripting.lua.classes.gui;
import :scripting.lua.libraries.gui;
import :scripting.lua.libraries.gui_callbacks;
import :scripting.lua.libraries.vulkan;

import :game;
import :gui;
import :engine;
import pragma.gui;
import pragma.string.unicode;
// import pragma.scripting.lua;

#undef DrawState

template<class TStream>
static TStream &print_ui_element(TStream &os, const pragma::gui::types::WIBase &handle)
{
	using ::operator<<; // Workaround for msvc compiler bug
	const pragma::gui::types::WIBase *p = &handle;
	auto pos = p->GetAbsolutePos();
	auto &size = p->GetSize();
	os << "WIElement[" << p->GetClass() << "]";
	os << "[" << p->GetName() << "]";
	os << "[" << p->GetIndex() << "]";
	os << "[" << &handle << "]";
	os << "[Pos:" << pos.x << ", " << pos.y << "]";
	os << "[Sz:" << size.x << ", " << size.y << "]";
	os << "[Vis:" << (p->IsVisible() ? "1" : "0") << "(" << (p->IsSelfVisible() ? "1" : "0") << ")]";

	auto *elText = dynamic_cast<const pragma::gui::types::WIText *>(p);
	if(elText) {
		auto text = elText->GetText().cpp_str();
		if(text.length() > 10)
			text = text.substr(0, 10) + "...";
		os << "[" << text << "]";
	}
	else {
		auto *elTex = dynamic_cast<const pragma::gui::types::WITexturedShape *>(p);
		if(elTex) {
			auto *mat = const_cast<pragma::gui::types::WITexturedShape *>(elTex)->GetMaterial();
			if(mat)
				os << "[" << mat->GetName() << "]";
			else
				os << "[NULL]";
		}
	}
	return os;
}
namespace pragma::gui::types {
	DLLCLIENT Con::c_cout &operator<<(Con::c_cout &os, const WIBase &handle) { return print_ui_element<Con::c_cout>(os, handle); }
	DLLCLIENT std::ostream &operator<<(std::ostream &os, const WIBase &handle) { return print_ui_element<std::ostream>(os, handle); }
	static bool operator==(WIBase &a, WIBase &b) { return &a == &b; }
}

static void record_render_ui(pragma::gui::types::WIBase &el, prosper::IImage &img, const Lua::gui::DrawToTextureInfo &info, const std::shared_ptr<prosper::ICommandBuffer> &drawCmd)
{
	auto useMsaa = (img.GetSampleCount() > prosper::SampleCountFlags::e1Bit);
	if(info.clearColor) {
		auto clearCol = info.clearColor->ToVector4();
		drawCmd->RecordClearAttachment(img, std::array<float, 4> {clearCol[0], clearCol[1], clearCol[2], clearCol[3]});
	}

	pragma::gui::DrawInfo drawInfo {drawCmd};
	drawInfo.size = {el.GetWidth(), el.GetHeight()};
	pragma::math::set_flag(drawInfo.flags, pragma::gui::DrawInfo::Flags::UseScissor, false);
	pragma::math::set_flag(drawInfo.flags, pragma::gui::DrawInfo::Flags::UseStencil, info.useStencil);
	pragma::math::set_flag(drawInfo.flags, pragma::gui::DrawInfo::Flags::Msaa, useMsaa);
	std::optional<Mat4> rotMat = el.GetRotationMatrix() ? *el.GetRotationMatrix() : std::optional<Mat4> {};
	if(rotMat.has_value()) {
		el.ResetRotation(); // We'll temporarily disable the rotation for this element
		pragma::math::set_flag(drawInfo.flags, pragma::gui::DrawInfo::Flags::UseStencil, true);
	}
	pragma::gui::DrawState drawState {};
	drawState.SetScissor(0, 0, drawInfo.size.x, drawInfo.size.y);
	el.Draw(drawInfo, drawState, {}, {}, drawInfo.size, el.GetScale());
	if(rotMat.has_value())
		el.SetRotation(*rotMat);
}
static bool render_ui(pragma::gui::types::WIBase &el, prosper::RenderTarget &rt, const Lua::gui::DrawToTextureInfo &info)
{
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	debug::get_domain().BeginTask("draw_ui_to_texture");
	pragma::util::ScopeGuard sgVtune {[]() { debug::get_domain().EndTask(); }};
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

static void record_draw_ui(pragma::gui::types::WIBase &el, Lua::Vulkan::CommandBufferRecorder &cmdBufGroup, prosper::IImage &img, const Lua::gui::DrawToTextureInfo &info)
{
	cmdBufGroup.Record([&el, &img, &info](prosper::ISecondaryCommandBuffer &drawCmd) mutable { record_render_ui(el, img, info, drawCmd.shared_from_this()); });
}

static std::shared_ptr<prosper::Texture> draw_to_texture(pragma::gui::types::WIBase &el, const Lua::gui::DrawToTextureInfo &info)
{
	auto &context = pragma::get_cengine()->GetRenderContext();
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

static void clamp_to_parent_bounds(pragma::gui::types::WIBase &el, Vector2i &clampedPos, Vector2i &clamedSize)
{
	auto parent = el.GetParent();
	Vector2i pos = el.GetPos();
	Vector2i size = el.GetSize();
}

static void debug_print_hierarchy(const pragma::gui::types::WIBase &el, const std::string &t = "")
{
	Con::COUT << t << el << Con::endl;
	auto subT = t + "\t";
	for(auto &hChild : *const_cast<pragma::gui::types::WIBase &>(el).GetChildren()) {
		if(hChild.IsValid() == false)
			continue;
		debug_print_hierarchy(*hChild, subT);
	}
}

static pragma::gui::types::WIBase *find_descendant_by_path(pragma::gui::types::WIBase &el, const std::string &strPath)
{
	auto path = pragma::util::DirPath(strPath);
	auto *p = &el;
	for(auto &name : path) {
		auto *child = p->FindDescendantByName(std::string {name});
		if(!child)
			return nullptr;
		p = child;
	}
	return p;
}

void Lua::WIBase::register_class(luabind::class_<pragma::gui::types::WIBase> &classDef)
{
	classDef.def(luabind::tostring(luabind::self));
	classDef.def(luabind::self == luabind::self);
	classDef.def("IsValid", &IsValid);
	classDef.def("GetIndex", &pragma::gui::types::WIBase::GetIndex);
	classDef.def("GetDepth", &pragma::gui::types::WIBase::GetDepth);
	classDef.def("SetSkinCallbacksEnabled", &pragma::gui::types::WIBase::SetSkinCallbacksEnabled);
	classDef.def("Remove", &pragma::gui::types::WIBase::Remove);
	classDef.def("RemoveSafely", &pragma::gui::types::WIBase::RemoveSafely);
	classDef.def("SetIgnoreParentAlpha", &pragma::gui::types::WIBase::SetIgnoreParentAlpha);
	classDef.def("ShouldIgnoreParentAlpha", &pragma::gui::types::WIBase::ShouldIgnoreParentAlpha);
	classDef.def("SetZPos", &pragma::gui::types::WIBase::SetZPos);
	classDef.def("GetZPos", &pragma::gui::types::WIBase::GetZPos);
	classDef.def("HasFocus", &pragma::gui::types::WIBase::HasFocus);
	classDef.def("RequestFocus", &pragma::gui::types::WIBase::RequestFocus);
	classDef.def("ClearFocus", +[](pragma::gui::types::WIBase &el) { return pragma::gui::WGUI::GetInstance().ClearFocus(el); });
	classDef.def("KillFocus", &pragma::gui::types::WIBase::KillFocus);
	classDef.def("KillFocus", &pragma::gui::types::WIBase::KillFocus, luabind::default_parameter_policy<2, false> {});
	classDef.def("TrapFocus", static_cast<void (*)(lua::State *, pragma::gui::types::WIBase &, bool)>(&TrapFocus));
	classDef.def("TrapFocus", static_cast<void (*)(lua::State *, pragma::gui::types::WIBase &)>(&TrapFocus));
	classDef.def("IsFocusTrapped", &pragma::gui::types::WIBase::IsFocusTrapped);
	classDef.def("IsHidden", static_cast<bool (*)(lua::State *, pragma::gui::types::WIBase &)>([](lua::State *l, pragma::gui::types::WIBase &hPanel) { return !hPanel.IsVisible(); }));
	classDef.def("SetVisible", &pragma::gui::types::WIBase::SetVisible);
	classDef.def("IsVisible", &pragma::gui::types::WIBase::IsSelfVisible);
	classDef.def("IsParentVisible", &pragma::gui::types::WIBase::IsParentVisible);
	classDef.def("SetAutoSizeToContents", static_cast<void (*)(lua::State *, pragma::gui::types::WIBase &)>([](lua::State *l, pragma::gui::types::WIBase &hPanel) { hPanel.SetAutoSizeToContents(true); }));
	classDef.def("SetAutoSizeToContents", static_cast<void (*)(lua::State *, pragma::gui::types::WIBase &, bool)>([](lua::State *l, pragma::gui::types::WIBase &hPanel, bool autoSize) { hPanel.SetAutoSizeToContents(autoSize); }));
	classDef.def("SetAutoSizeToContents", static_cast<void (*)(lua::State *, pragma::gui::types::WIBase &, bool, bool)>([](lua::State *l, pragma::gui::types::WIBase &hPanel, bool x, bool y) { hPanel.SetAutoSizeToContents(x, y); }));
	classDef.def("SetAutoSizeToContents", +[](lua::State *l, pragma::gui::types::WIBase &hPanel, bool x, bool y, bool updateImmediately) { hPanel.SetAutoSizeToContents(x, y, updateImmediately); });
	classDef.def("UpdateAutoSizeToContents", +[](lua::State *l, pragma::gui::types::WIBase &hPanel) { hPanel.UpdateAutoSizeToContents(hPanel.ShouldAutoSizeToContentsX(), hPanel.ShouldAutoSizeToContentsY()); });
	classDef.def(
	  "FindAncestorByClass", +[](lua::State *l, pragma::gui::types::WIBase &hPanel, std::string className) -> pragma::gui::types::WIBase * {
		  pragma::string::to_lower(className);
		  auto *parent = hPanel.GetParent();
		  while(parent) {
			  if(parent->GetClass() == className)
				  return parent;
			  parent = parent->GetParent();
		  }
		  return nullptr;
	  });
	classDef.def("ShouldAutoSizeToContentsX", &pragma::gui::types::WIBase::ShouldAutoSizeToContentsX);
	classDef.def("ShouldAutoSizeToContentsY", &pragma::gui::types::WIBase::ShouldAutoSizeToContentsY);
	classDef.def("SetStencilEnabled", &pragma::gui::types::WIBase::SetStencilEnabled);
	classDef.def("IsStencilEnabled", &pragma::gui::types::WIBase::IsStencilEnabled);
	classDef.def("ResetRotation", &pragma::gui::types::WIBase::ResetRotation);
	classDef.def("GetRotationMatrix", &pragma::gui::types::WIBase::GetRotationMatrix, luabind::copy_policy<0> {});
	classDef.def("SetRotation", static_cast<void (pragma::gui::types::WIBase::*)(pragma::math::Degree, const ::Vector2 &)>(&pragma::gui::types::WIBase::SetRotation));
	classDef.def("SetRotation", static_cast<void (pragma::gui::types::WIBase::*)(const ::Mat4 &)>(&pragma::gui::types::WIBase::SetRotation));
	classDef.def("SetLocalRenderTransform", &pragma::gui::types::WIBase::SetLocalRenderTransform);
	classDef.def("ClearLocalRenderTransform", &pragma::gui::types::WIBase::ClearLocalRenderTransform);
	classDef.def("GetLocalRenderTransform", static_cast<const pragma::math::ScaledTransform *(pragma::gui::types::WIBase::*)() const>(&pragma::gui::types::WIBase::GetLocalRenderTransform), luabind::copy_policy<0> {});
	classDef.def("GetMouseInputEnabled", &pragma::gui::types::WIBase::GetMouseInputEnabled);
	classDef.def("SetMouseInputEnabled", &pragma::gui::types::WIBase::SetMouseInputEnabled);
	classDef.def("GetKeyboardInputEnabled", &pragma::gui::types::WIBase::GetKeyboardInputEnabled);
	classDef.def("SetKeyboardInputEnabled", &pragma::gui::types::WIBase::SetKeyboardInputEnabled);
	classDef.def("SetScrollInputEnabled", &pragma::gui::types::WIBase::SetScrollInputEnabled);
	classDef.def("GetScrollInputEnabled", &pragma::gui::types::WIBase::GetScrollInputEnabled);
	classDef.def("SetCursorMovementCheckEnabled", &pragma::gui::types::WIBase::SetMouseMovementCheckEnabled);
	classDef.def("GetCursorMovementCheckEnabled", &pragma::gui::types::WIBase::GetMouseMovementCheckEnabled);
	classDef.def("GetPos", static_cast<const ::Vector2i &(pragma::gui::types::WIBase::*)() const>(&pragma::gui::types::WIBase::GetPos), luabind::copy_policy<0> {});
	classDef.def("SetPos", static_cast<void (*)(lua::State *, pragma::gui::types::WIBase &, ::Vector2)>(&SetPos));
	classDef.def("SetPos", static_cast<void (*)(lua::State *, pragma::gui::types::WIBase &, float, float)>(&SetPos));
	classDef.def("GetAbsolutePos", static_cast<::Vector2 (pragma::gui::types::WIBase::*)(bool) const>(&pragma::gui::types::WIBase::GetAbsolutePos));
	classDef.def("GetAbsolutePos", static_cast<::Vector2 (pragma::gui::types::WIBase::*)(bool) const>(&pragma::gui::types::WIBase::GetAbsolutePos), luabind::default_parameter_policy<2, true> {});
	classDef.def("GetAbsolutePos", static_cast<::Vector2 (pragma::gui::types::WIBase::*)(const ::Vector2 &, bool) const>(&pragma::gui::types::WIBase::GetAbsolutePos));
	classDef.def("GetAbsolutePos", static_cast<::Vector2 (pragma::gui::types::WIBase::*)(const ::Vector2 &, bool) const>(&pragma::gui::types::WIBase::GetAbsolutePos), luabind::default_parameter_policy<3, true> {});
	classDef.def("SetAbsolutePos", static_cast<void (*)(lua::State *, pragma::gui::types::WIBase &, ::Vector2)>(&SetAbsolutePos));
	classDef.def("SetAbsolutePos", static_cast<void (*)(lua::State *, pragma::gui::types::WIBase &, float, float)>(&SetAbsolutePos));
	classDef.def("GetAbsolutePose", static_cast<::Mat4 (pragma::gui::types::WIBase::*)() const>(&pragma::gui::types::WIBase::GetAbsolutePose));
	classDef.def("GetRelativePos", &pragma::gui::types::WIBase::GetRelativePos);
	classDef.def("GetColor", &pragma::gui::types::WIBase::GetColor, luabind::copy_policy<0> {});
	classDef.def("GetColorProperty", &pragma::gui::types::WIBase::GetColorProperty);
	classDef.def("GetFocusProperty", &pragma::gui::types::WIBase::GetFocusProperty);
	classDef.def("GetVisibilityProperty", &pragma::gui::types::WIBase::GetVisibilityProperty);
	classDef.def("GetPosProperty", &pragma::gui::types::WIBase::GetPosProperty);
	classDef.def("GetSizeProperty", &pragma::gui::types::WIBase::GetSizeProperty);
	classDef.def("GetMouseInBoundsProperty", &pragma::gui::types::WIBase::GetMouseInBoundsProperty);
	classDef.def("SetColor", &SetColor);
	classDef.def("SetColorRGB", static_cast<void (*)(lua::State *, pragma::gui::types::WIBase &, const ::Color &)>([](lua::State *l, pragma::gui::types::WIBase &hPanel, const ::Color &color) {
		auto vCol = color.ToVector4();
		vCol.a = hPanel.GetAlpha();
		hPanel.SetColor(vCol);
	}));
	classDef.def("GetAlpha", &GetAlpha);
	classDef.def("SetAlpha", &SetAlpha);
	classDef.def("GetWidth", &pragma::gui::types::WIBase::GetWidth);
	classDef.def("GetHeight", &pragma::gui::types::WIBase::GetHeight);
	classDef.def("GetSize", static_cast<const ::Vector2i &(pragma::gui::types::WIBase::*)() const>(&pragma::gui::types::WIBase::GetSize), luabind::copy_policy<0> {});
	classDef.def("SetSize", static_cast<void (*)(lua::State *, pragma::gui::types::WIBase &, ::Vector2)>(&SetSize));
	classDef.def("SetSize", static_cast<void (*)(lua::State *, pragma::gui::types::WIBase &, float, float)>(&SetSize));
	classDef.def("Wrap", static_cast<void (*)(lua::State *, pragma::gui::types::WIBase &, const std::string &)>(&Wrap));
	classDef.def("Wrap", static_cast<bool (pragma::gui::types::WIBase::*)(pragma::gui::types::WIBase &)>(&pragma::gui::types::WIBase::Wrap));
	classDef.def("AnchorWithMargin", static_cast<void (pragma::gui::types::WIBase::*)(uint32_t, uint32_t, uint32_t, uint32_t)>(&pragma::gui::types::WIBase::AnchorWithMargin));
	classDef.def("AnchorWithMargin", static_cast<void (pragma::gui::types::WIBase::*)(uint32_t)>(&pragma::gui::types::WIBase::AnchorWithMargin));
	classDef.def("GetParent", &pragma::gui::types::WIBase::GetParent);
	classDef.def("SetParent", +[](lua::State *l, pragma::gui::types::WIBase &hPanel, pragma::gui::types::WIBase &hParent) { hPanel.SetParent(&hParent); });
	classDef.def("SetParent", +[](lua::State *l, pragma::gui::types::WIBase &hPanel, pragma::gui::types::WIBase &hParent, uint32_t index) { hPanel.SetParent(&hParent, index); });
	classDef.def("SetParentAndUpdateWindow", +[](lua::State *l, pragma::gui::types::WIBase &hPanel, pragma::gui::types::WIBase &hParent) { hPanel.SetParentAndUpdateWindow(&hParent); });
	classDef.def("SetParentAndUpdateWindow", +[](lua::State *l, pragma::gui::types::WIBase &hPanel, pragma::gui::types::WIBase &hParent, uint32_t index) { hPanel.SetParentAndUpdateWindow(&hParent, index); });
	classDef.def("ClearParent", &ClearParent);
	classDef.def("ResetParent", &ResetParent);
	classDef.def(
	  "GetChildren", +[](lua::State *l, pragma::gui::types::WIBase &hPanel) -> luabind::tableT<pragma::gui::types::WIBase> {
		  auto &children = *hPanel.GetChildren();
		  auto t = luabind::newtable(l);
		  for(uint32_t idx = 1; auto &hChild : children) {
			  if(hChild.expired())
				  continue;
			  t[idx++] = hChild;
		  }
		  return t;
	  });
	classDef.def("GetChildren", static_cast<void (*)(lua::State *, pragma::gui::types::WIBase &, std::string)>(&GetChildren));
	classDef.def("GetFirstChild", &pragma::gui::types::WIBase::GetFirstChild);
	classDef.def("GetChild", static_cast<void (*)(lua::State *, pragma::gui::types::WIBase &, unsigned int)>(&GetChild));
	classDef.def("GetChild", static_cast<void (*)(lua::State *, pragma::gui::types::WIBase &, std::string, unsigned int)>(&GetChild));
	classDef.def("IsPosInBounds", &PosInBounds);
	classDef.def("IsCursorInBounds", &pragma::gui::types::WIBase::MouseInBounds);
	classDef.def("GetCursorPos", &GetMousePos);
	classDef.def("Draw", static_cast<void (*)(lua::State *, pragma::gui::types::WIBase &, const pragma::gui::DrawInfo &, pragma::gui::DrawState &, const ::Vector2i &, const ::Vector2i &, const ::Vector2i &)>(&Draw));
	classDef.def("Draw", static_cast<void (*)(lua::State *, pragma::gui::types::WIBase &, const pragma::gui::DrawInfo &, pragma::gui::DrawState &, const ::Vector2i &, const ::Vector2i &)>(&Draw));
	classDef.def("Draw", static_cast<void (*)(lua::State *, pragma::gui::types::WIBase &, const pragma::gui::DrawInfo &, pragma::gui::DrawState &)>(&Draw));
	classDef.def("DrawToTexture", &render_ui);
	classDef.def("DrawToTexture", +[](pragma::gui::types::WIBase &el, prosper::RenderTarget &rt) { return render_ui(el, rt, {}); });
	classDef.def("DrawToTexture", &draw_to_texture);
	classDef.def("DrawToTexture", +[](pragma::gui::types::WIBase &el) { return draw_to_texture(el, {}); });
	classDef.def("RecordDraw", &record_draw_ui);
	classDef.def("GetX", &GetX);
	classDef.def("GetY", &GetY);
	classDef.def("SetX", &SetX);
	classDef.def("SetY", &SetY);
	classDef.def("SetWidth", &SetWidth);
	classDef.def("SetHeight", &SetHeight);
	classDef.def("SizeToContents", &pragma::gui::types::WIBase::SizeToContents);
	classDef.def("SizeToContents", &pragma::gui::types::WIBase::SizeToContents, luabind::default_parameter_policy<3, true> {});
	classDef.def("SizeToContents", &pragma::gui::types::WIBase::SizeToContents, luabind::meta::join<luabind::default_parameter_policy<2, true>, luabind::default_parameter_policy<3, true>>::type {});
	classDef.def("AddCallback", &AddCallback);
	classDef.def("CallCallbacks", static_cast<void (*)(lua::State *, pragma::gui::types::WIBase &, std::string)>(&CallCallbacks));
	classDef.def("CallCallbacks", static_cast<void (*)(lua::State *, pragma::gui::types::WIBase &, std::string, luabind::object)>(&CallCallbacks));
	classDef.def("CallCallbacks", static_cast<void (*)(lua::State *, pragma::gui::types::WIBase &, std::string, luabind::object, luabind::object)>(&CallCallbacks));
	classDef.def("CallCallbacks", static_cast<void (*)(lua::State *, pragma::gui::types::WIBase &, std::string, luabind::object, luabind::object, luabind::object)>(&CallCallbacks));
	classDef.def("CallCallbacks", static_cast<void (*)(lua::State *, pragma::gui::types::WIBase &, std::string, luabind::object, luabind::object, luabind::object, luabind::object)>(&CallCallbacks));
	classDef.def("CallCallbacks", static_cast<void (*)(lua::State *, pragma::gui::types::WIBase &, std::string, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object)>(&CallCallbacks));
	classDef.def("CallCallbacks", static_cast<void (*)(lua::State *, pragma::gui::types::WIBase &, std::string, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object)>(&CallCallbacks));
	classDef.def("CallCallbacks", static_cast<void (*)(lua::State *, pragma::gui::types::WIBase &, std::string, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object)>(&CallCallbacks));
	classDef.def("CallCallbacks", static_cast<void (*)(lua::State *, pragma::gui::types::WIBase &, std::string, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object)>(&CallCallbacks));
	classDef.def("AddEventListener", &AddCallback);
	classDef.def("CallEventListeners", static_cast<void (*)(lua::State *, pragma::gui::types::WIBase &, std::string)>(&CallCallbacks));
	classDef.def("CallEventListeners", static_cast<void (*)(lua::State *, pragma::gui::types::WIBase &, std::string, luabind::object)>(&CallCallbacks));
	classDef.def("CallEventListeners", static_cast<void (*)(lua::State *, pragma::gui::types::WIBase &, std::string, luabind::object, luabind::object)>(&CallCallbacks));
	classDef.def("CallEventListeners", static_cast<void (*)(lua::State *, pragma::gui::types::WIBase &, std::string, luabind::object, luabind::object, luabind::object)>(&CallCallbacks));
	classDef.def("CallEventListeners", static_cast<void (*)(lua::State *, pragma::gui::types::WIBase &, std::string, luabind::object, luabind::object, luabind::object, luabind::object)>(&CallCallbacks));
	classDef.def("CallEventListeners", static_cast<void (*)(lua::State *, pragma::gui::types::WIBase &, std::string, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object)>(&CallCallbacks));
	classDef.def("CallEventListeners", static_cast<void (*)(lua::State *, pragma::gui::types::WIBase &, std::string, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object)>(&CallCallbacks));
	classDef.def("CallEventListeners", static_cast<void (*)(lua::State *, pragma::gui::types::WIBase &, std::string, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object)>(&CallCallbacks));
	classDef.def("CallEventListeners", static_cast<void (*)(lua::State *, pragma::gui::types::WIBase &, std::string, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object)>(&CallCallbacks));
	classDef.def("FadeIn", static_cast<void (*)(lua::State *, pragma::gui::types::WIBase &, float)>(&FadeIn));
	classDef.def("FadeIn", static_cast<void (*)(lua::State *, pragma::gui::types::WIBase &, float, float)>(&FadeIn));
	classDef.def("FadeOut", static_cast<void (pragma::gui::types::WIBase::*)(float, bool)>(&pragma::gui::types::WIBase::FadeOut));
	classDef.def("FadeOut", static_cast<void (*)(pragma::gui::types::WIBase &, float)>([](pragma::gui::types::WIBase &el, float tFade) { el.FadeOut(tFade); }));
	classDef.def("IsFading", &pragma::gui::types::WIBase::IsFading);
	classDef.def("IsFadingOut", &pragma::gui::types::WIBase::IsFadingOut);
	classDef.def("IsFadingIn", &pragma::gui::types::WIBase::IsFadingIn);
	classDef.def("GetClass", &pragma::gui::types::WIBase::GetClass);
	classDef.def(
	  "Think", +[](pragma::gui::types::WIBase &el, std::shared_ptr<prosper::ICommandBuffer> &cmd) {
		  if(!cmd->IsPrimary())
			  return;
		  auto primaryCmdBuffer = std::dynamic_pointer_cast<prosper::IPrimaryCommandBuffer>(cmd);
		  el.Think(primaryCmdBuffer);
	  });
	classDef.def("InjectMouseMoveInput", &InjectMouseMoveInput);
	classDef.def("InjectMouseInput", static_cast<pragma::util::EventReply (*)(lua::State *, pragma::gui::types::WIBase &, const ::Vector2 &, int, int, int)>(&InjectMouseInput));
	classDef.def("InjectMouseInput", static_cast<pragma::util::EventReply (*)(lua::State *, pragma::gui::types::WIBase &, const ::Vector2 &, int, int)>(&InjectMouseInput));
	classDef.def("InjectMouseClick", static_cast<pragma::util::EventReply (*)(lua::State *, pragma::gui::types::WIBase &, const ::Vector2 &, int, int)>(&InjectMouseClick));
	classDef.def("InjectMouseClick", static_cast<pragma::util::EventReply (*)(lua::State *, pragma::gui::types::WIBase &, const ::Vector2 &, int)>(&InjectMouseClick));
	classDef.def("InjectKeyboardInput", static_cast<pragma::util::EventReply (*)(lua::State *, pragma::gui::types::WIBase &, int, int, int)>(&InjectKeyboardInput));
	classDef.def("InjectKeyboardInput", static_cast<pragma::util::EventReply (*)(lua::State *, pragma::gui::types::WIBase &, int, int)>(&InjectKeyboardInput));
	classDef.def("InjectKeyPress", static_cast<pragma::util::EventReply (*)(lua::State *, pragma::gui::types::WIBase &, int, int)>(&InjectKeyPress));
	classDef.def("InjectKeyPress", static_cast<pragma::util::EventReply (*)(lua::State *, pragma::gui::types::WIBase &, int)>(&InjectKeyPress));
	classDef.def("InjectCharInput", static_cast<pragma::util::EventReply (*)(lua::State *, pragma::gui::types::WIBase &, std::string, uint32_t)>(&InjectCharInput));
	classDef.def("InjectCharInput", static_cast<pragma::util::EventReply (*)(lua::State *, pragma::gui::types::WIBase &, std::string)>(&InjectCharInput));
	classDef.def("InjectScrollInput", static_cast<pragma::util::EventReply (*)(lua::State *, pragma::gui::types::WIBase &, const ::Vector2 &, const ::Vector2 &, bool)>(&InjectScrollInput));
	classDef.def("InjectScrollInput", static_cast<pragma::util::EventReply (*)(lua::State *, pragma::gui::types::WIBase &, const ::Vector2 &, const ::Vector2 &)>(&InjectScrollInput));
	classDef.def("IsDescendant", &pragma::gui::types::WIBase::IsDescendant);
	classDef.def("IsDescendantOf", &pragma::gui::types::WIBase::IsDescendantOf);
	classDef.def("IsAncestor", &pragma::gui::types::WIBase::IsAncestor);
	classDef.def("IsAncestorOf", &pragma::gui::types::WIBase::IsAncestorOf);
	classDef.def("GetName", &pragma::gui::types::WIBase::GetName);
	classDef.def("SetName", &pragma::gui::types::WIBase::SetName);
	classDef.def("FindChildByName", &FindChildByName);
	classDef.def("FindChildrenByName", &FindChildrenByName);
	classDef.def("SetAutoAlignToParent", static_cast<void (pragma::gui::types::WIBase::*)(bool, bool)>(&pragma::gui::types::WIBase::SetAutoAlignToParent));
	classDef.def("SetAutoAlignToParent", static_cast<void (pragma::gui::types::WIBase::*)(bool)>(&pragma::gui::types::WIBase::SetAutoAlignToParent));
	classDef.def("GetAutoAlignToParentX", &pragma::gui::types::WIBase::GetAutoAlignToParentX);
	classDef.def("GetAutoAlignToParentY", &pragma::gui::types::WIBase::GetAutoAlignToParentY);
	classDef.def("Resize", &pragma::gui::types::WIBase::Resize);
	classDef.def("ScheduleUpdate", &pragma::gui::types::WIBase::ScheduleUpdate);
	classDef.def("SetSkin", &pragma::gui::types::WIBase::SetSkin);
	classDef.def("GetSkinName", &pragma::gui::types::WIBase::GetSkinName);
	classDef.def("ResetSkin", &pragma::gui::types::WIBase::ResetSkin);
	classDef.def("GetStyleClasses", +[](pragma::gui::types::WIBase &el) -> std::vector<std::string> { return el.GetStyleClasses(); });
	classDef.def("AddStyleClass", &pragma::gui::types::WIBase::AddStyleClass);
	classDef.def("SetCursor", &pragma::gui::types::WIBase::SetCursor);
	classDef.def("GetCursor", &pragma::gui::types::WIBase::GetCursor);
	classDef.def("RemoveElementOnRemoval", &pragma::gui::types::WIBase::RemoveOnRemoval);
	classDef.def("GetTooltip", &pragma::gui::types::WIBase::GetTooltip);
	classDef.def("SetTooltip", &pragma::gui::types::WIBase::SetTooltip);
	classDef.def("HasTooltip", &pragma::gui::types::WIBase::HasTooltip);
	classDef.def("GetLeft", &pragma::gui::types::WIBase::GetLeft);
	classDef.def("GetTop", &pragma::gui::types::WIBase::GetTop);
	classDef.def("GetRight", &pragma::gui::types::WIBase::GetRight);
	classDef.def("GetBottom", &pragma::gui::types::WIBase::GetBottom);
	classDef.def("GetEndPos", &pragma::gui::types::WIBase::GetEndPos);
	classDef.def("SetClippingEnabled", &pragma::gui::types::WIBase::SetShouldScissor);
	classDef.def("IsClippingEnabled", &pragma::gui::types::WIBase::GetShouldScissor);
	classDef.def("SetAlwaysUpdate", &pragma::gui::types::WIBase::SetThinkIfInvisible);
	classDef.def("SetBounds", &SetBounds);
	classDef.def("SetBackgroundElement", static_cast<void (pragma::gui::types::WIBase::*)(bool, bool)>(&pragma::gui::types::WIBase::SetBackgroundElement));
	classDef.def("SetBackgroundElement", static_cast<void (*)(pragma::gui::types::WIBase &, bool)>([](pragma::gui::types::WIBase &el, bool backgroundElement) { el.SetBackgroundElement(backgroundElement); }));
	classDef.def("SetBackgroundElement", static_cast<void (*)(pragma::gui::types::WIBase &)>([](pragma::gui::types::WIBase &el) { el.SetBackgroundElement(true); }));
	classDef.def("IsBackgroundElement", &pragma::gui::types::WIBase::IsBackgroundElement);
	classDef.def("SetBaseElement", &pragma::gui::types::WIBase::SetBaseElement);
	classDef.def("IsBaseElement", &pragma::gui::types::WIBase::IsBaseElement);
	classDef.def("FindDescendantByName", static_cast<pragma::gui::types::WIBase *(*)(lua::State *, pragma::gui::types::WIBase &, const std::string &)>([](lua::State *l, pragma::gui::types::WIBase &hPanel, const std::string &name) { return hPanel.FindDescendantByName(name); }));
	classDef.def("FindDescendantByPath", &find_descendant_by_path);
	classDef.def("FindDescendantsByName",
	  static_cast<luabind::tableT<pragma::gui::types::WIBase> (*)(lua::State *, pragma::gui::types::WIBase &, const std::string &)>([](lua::State *l, pragma::gui::types::WIBase &hPanel, const std::string &name) -> luabind::tableT<pragma::gui::types::WIBase> {
		  std::vector<pragma::gui::WIHandle> children {};
		  hPanel.FindDescendantsByName(name, children);
		  auto t = luabind::newtable(l);
		  auto idx = 1;
		  for(auto &hChild : children) {
			  if(hChild.IsValid() == false)
				  continue;
			  t[idx++] = pragma::gui::WGUILuaInterface::GetLuaObject(l, *hChild.get());
		  }
		  return t;
	  }));
	classDef.def("Update", &pragma::gui::types::WIBase::Update);
	classDef.def("ApplySkin", &pragma::gui::types::WIBase::ApplySkin);
	classDef.def("ApplySkin", &pragma::gui::types::WIBase::ApplySkin, luabind::default_parameter_policy<2, static_cast<pragma::gui::WISkin *>(nullptr)> {});
	classDef.def("RefreshSkin", &pragma::gui::types::WIBase::RefreshSkin);
	classDef.def("SetLeft", &pragma::gui::types::WIBase::SetLeft);
	classDef.def("SetRight", &pragma::gui::types::WIBase::SetRight);
	classDef.def("SetTop", &pragma::gui::types::WIBase::SetTop);
	classDef.def("SetBottom", &pragma::gui::types::WIBase::SetBottom);
	classDef.def("EnableThinking", &pragma::gui::types::WIBase::EnableThinking);
	classDef.def("DisableThinking", &pragma::gui::types::WIBase::DisableThinking);
	classDef.def("SetThinkingEnabled", &pragma::gui::types::WIBase::SetThinkingEnabled);
	classDef.def(
	  "InvokeThink", +[](pragma::gui::types::WIBase &el, std::shared_ptr<prosper::ICommandBuffer> &cmd) {
		  if(!cmd->IsPrimary())
			  return;
		  auto primaryCmdBuffer = std::dynamic_pointer_cast<prosper::IPrimaryCommandBuffer>(cmd);
		  el.Think(primaryCmdBuffer);
	  });

	classDef.def("AddAttachment", static_cast<pragma::gui::WIAttachment *(pragma::gui::types::WIBase::*)(const std::string &, const ::Vector2 &)>(&pragma::gui::types::WIBase::AddAttachment));
	classDef.def("AddAttachment", static_cast<pragma::gui::WIAttachment *(*)(pragma::gui::types::WIBase &, const std::string &)>([](pragma::gui::types::WIBase &el, const std::string &name) { return el.AddAttachment(name); }));
	classDef.def("SetAttachmentPos", &pragma::gui::types::WIBase::SetAttachmentPos);
	classDef.def("GetAttachmentPos", &pragma::gui::types::WIBase::GetAttachmentPos, luabind::copy_policy<0> {});
	classDef.def("GetAbsoluteAttachmentPos", &pragma::gui::types::WIBase::GetAbsoluteAttachmentPos, luabind::copy_policy<0> {});
	classDef.def("GetAttachmentPosProperty", &pragma::gui::types::WIBase::GetAttachmentPosProperty);
	classDef.def("SetAnchor", static_cast<void (pragma::gui::types::WIBase::*)(float, float, float, float, uint32_t, uint32_t)>(&pragma::gui::types::WIBase::SetAnchor));
	classDef.def("SetAnchor", static_cast<void (*)(pragma::gui::types::WIBase &, float, float, float, float)>([](pragma::gui::types::WIBase &el, float left, float top, float right, float bottom) { el.SetAnchor(left, top, right, bottom); }));
	classDef.def("SetAnchorLeft", &pragma::gui::types::WIBase::SetAnchorLeft);
	classDef.def("SetAnchorRight", &pragma::gui::types::WIBase::SetAnchorRight);
	classDef.def("SetAnchorTop", &pragma::gui::types::WIBase::SetAnchorTop);
	classDef.def("SetAnchorBottom", &pragma::gui::types::WIBase::SetAnchorBottom);
	classDef.def("ClearAnchor", &pragma::gui::types::WIBase::ClearAnchor);
	classDef.def(
	  "GetAnchor", +[](pragma::gui::types::WIBase &el) -> std::optional<std::tuple<float, float, float, float>> {
		  float left, top, right, bottom;
		  if(!el.GetAnchor(left, top, right, bottom))
			  return {};
		  return std::tuple<float, float, float, float> {left, top, right, bottom};
	  });
	classDef.def("HasAnchor", &pragma::gui::types::WIBase::HasAnchor);
	classDef.def("SetRemoveOnParentRemoval", &pragma::gui::types::WIBase::SetRemoveOnParentRemoval);
	classDef.def("GetCenter", &pragma::gui::types::WIBase::GetCenter);
	classDef.def("GetCenterX", &pragma::gui::types::WIBase::GetCenterX);
	classDef.def("GetCenterY", &pragma::gui::types::WIBase::GetCenterY);
	classDef.def("GetHalfWidth", &pragma::gui::types::WIBase::GetHalfWidth);
	classDef.def("GetHalfHeight", &pragma::gui::types::WIBase::GetHalfHeight);
	classDef.def("GetHalfSize", &pragma::gui::types::WIBase::GetHalfSize);
	classDef.def("SetCenterPos", &pragma::gui::types::WIBase::SetCenterPos);
	classDef.def("SetLocalAlpha", &pragma::gui::types::WIBase::SetLocalAlpha);
	classDef.def("GetLocalAlpha", &pragma::gui::types::WIBase::GetLocalAlpha);
	classDef.def("CenterToParent", static_cast<void (pragma::gui::types::WIBase::*)(bool)>(&pragma::gui::types::WIBase::CenterToParent));
	classDef.def("CenterToParent", static_cast<void (*)(lua::State *, pragma::gui::types::WIBase &)>([](lua::State *l, pragma::gui::types::WIBase &hPanel) { hPanel.CenterToParent(false); }));
	classDef.def("CenterToParentX", &pragma::gui::types::WIBase::CenterToParentX);
	classDef.def("CenterToParentY", &pragma::gui::types::WIBase::CenterToParentY);
	classDef.def("RemoveStyleClass", &pragma::gui::types::WIBase::RemoveStyleClass);
	classDef.def("ClearStyleClasses", &pragma::gui::types::WIBase::ClearStyleClasses);
	classDef.def("FindChildIndex", &pragma::gui::types::WIBase::FindChildIndex);
	classDef.def("SetScale", static_cast<void (pragma::gui::types::WIBase::*)(const ::Vector2 &)>(&pragma::gui::types::WIBase::SetScale));
	classDef.def("SetScale", static_cast<void (pragma::gui::types::WIBase::*)(float, float)>(&pragma::gui::types::WIBase::SetScale));
	classDef.def("GetScale", &pragma::gui::types::WIBase::GetScale, luabind::copy_policy<0> {});
	classDef.def("GetScaleProperty", &pragma::gui::types::WIBase::GetScaleProperty);
	classDef.def("IsUpdateScheduled", &pragma::gui::types::WIBase::IsUpdateScheduled);
	classDef.def("IsRemovalScheduled", &pragma::gui::types::WIBase::IsRemovalScheduled);
	classDef.def("GetRootElement", static_cast<pragma::gui::types::WIBase *(pragma::gui::types::WIBase::*)()>(&pragma::gui::types::WIBase::GetRootElement));
	classDef.def("GetRootWindow", static_cast<prosper::Window *(pragma::gui::types::WIBase::*)()>(&pragma::gui::types::WIBase::GetRootWindow));
	classDef.def("ClampToBounds", +[](const pragma::gui::types::WIBase &el, ::Vector2i &pos) { el.ClampToBounds(pos); });
	classDef.def("ClampToBounds", +[](const pragma::gui::types::WIBase &el, ::Vector2i &pos, ::Vector2i &size) { el.ClampToBounds(pos, size); });
	classDef.def(
	  "GetVisibleBounds", +[](const pragma::gui::types::WIBase &el) -> std::pair<::Vector2i, ::Vector2i> {
		  ::Vector2i pos, size;
		  el.GetVisibleBounds(pos, size);
		  return {pos, size};
	  });
	classDef.def(
	  "GetAbsoluteVisibleBounds", +[](const pragma::gui::types::WIBase &el) -> std::tuple<::Vector2i, ::Vector2i, ::Vector2i> {
		  ::Vector2i pos, size;
		  ::Vector2i absPosParent;
		  el.GetAbsoluteVisibleBounds(pos, size, &absPosParent);
		  return {pos, size, absPosParent};
	  });
	classDef.def("ClampToVisibleBounds", +[](const pragma::gui::types::WIBase &el, ::Vector2i &pos) { el.ClampToVisibleBounds(pos); });
	classDef.def("ClampToVisibleBounds", +[](const pragma::gui::types::WIBase &el, ::Vector2i &pos, ::Vector2i &size) { el.ClampToVisibleBounds(pos, size); });
	classDef.def("DebugPrintHierarchy", +[](const pragma::gui::types::WIBase &el) { debug_print_hierarchy(el); });
	classDef.def("IsFileHovering", &pragma::gui::types::WIBase::IsFileHovering);
	classDef.def("SetFileHovering", &pragma::gui::types::WIBase::SetFileHovering);
	classDef.def("GetFileDropInputEnabled", &pragma::gui::types::WIBase::GetFileDropInputEnabled);
	classDef.def("SetFileDropInputEnabled", &pragma::gui::types::WIBase::SetFileDropInputEnabled);

	auto defDrawInfo = luabind::class_<pragma::gui::DrawInfo>("DrawInfo");
	defDrawInfo.add_static_constant("FLAG_NONE", pragma::math::to_integral(pragma::gui::DrawInfo::Flags::None));
	defDrawInfo.add_static_constant("FLAG_USE_SCISSOR_BIT", pragma::math::to_integral(pragma::gui::DrawInfo::Flags::UseScissor));
	defDrawInfo.add_static_constant("FLAG_USE_STENCIL_BIT", pragma::math::to_integral(pragma::gui::DrawInfo::Flags::UseStencil));
	defDrawInfo.add_static_constant("FLAG_MSAA_BIT", pragma::math::to_integral(pragma::gui::DrawInfo::Flags::Msaa));
	defDrawInfo.add_static_constant("FLAG_DONT_SKIP_IF_OUT_OF_BOUNDS_BIT", pragma::math::to_integral(pragma::gui::DrawInfo::Flags::DontSkipIfOutOfBounds));
	defDrawInfo.def(luabind::constructor<const std::shared_ptr<prosper::ICommandBuffer> &>());
	defDrawInfo.def_readwrite("offset", &pragma::gui::DrawInfo::offset);
	defDrawInfo.def_readwrite("size", &pragma::gui::DrawInfo::size);
	defDrawInfo.def_readwrite("transform", &pragma::gui::DrawInfo::transform);
	defDrawInfo.def_readwrite("flags", &pragma::gui::DrawInfo::flags);
	defDrawInfo.property("commandBuffer", static_cast<luabind::object (*)(lua::State *, pragma::gui::DrawInfo &)>([](lua::State *l, pragma::gui::DrawInfo &drawInfo) -> luabind::object { return drawInfo.commandBuffer ? luabind::object {l, drawInfo.commandBuffer} : luabind::object {}; }),
	  static_cast<void (*)(lua::State *, pragma::gui::DrawInfo &, luabind::object)>([](lua::State *l, pragma::gui::DrawInfo &drawInfo, luabind::object o) {
		  if(IsSet(l, 2) == false) {
			  drawInfo.commandBuffer = nullptr;
			  return;
		  }
		  drawInfo.commandBuffer = Lua::Check<Vulkan::CommandBuffer>(l, 2).shared_from_this();
	  }));
	defDrawInfo.def("SetColor", static_cast<void (*)(lua::State *, pragma::gui::DrawInfo &, const ::Color &)>([](lua::State *l, pragma::gui::DrawInfo &drawInfo, const ::Color &color) { drawInfo.color = color.ToVector4(); }));
	defDrawInfo.def("SetPostTransform", static_cast<void (*)(lua::State *, pragma::gui::DrawInfo &, const ::Mat4 &)>([](lua::State *l, pragma::gui::DrawInfo &drawInfo, const ::Mat4 &t) { drawInfo.postTransform = t; }));
	classDef.scope[defDrawInfo];
}

void Lua::WIButton::register_class(luabind::class_<pragma::gui::types::WIButton, pragma::gui::types::WIBase> &classDef)
{
	classDef.def("SetText", +[](pragma::gui::types::WIButton &button, const std::string &text) { button.SetText(text); });
	classDef.def("GetText", +[](pragma::gui::types::WIButton &button) { return button.GetText().cpp_str(); });
}

void Lua::WIProgressBar::register_class(luabind::class_<pragma::gui::types::WIProgressBar, pragma::gui::types::WIBase> &classDef)
{
	classDef.def("SetProgress", &pragma::gui::types::WIProgressBar::SetProgress);
	classDef.def("GetProgress", &pragma::gui::types::WIProgressBar::GetProgress);
	classDef.def("SetLabelVisible", &pragma::gui::types::WIProgressBar::SetLabelVisible);
	classDef.def("SetValue", &pragma::gui::types::WIProgressBar::SetValue);
	classDef.def("GetValue", static_cast<float (pragma::gui::types::WIProgressBar::*)() const>(&pragma::gui::types::WIProgressBar::GetValue));
	classDef.def("SetRange", static_cast<void (pragma::gui::types::WIProgressBar::*)(float, float, float)>(&pragma::gui::types::WIProgressBar::SetRange));
	classDef.def("SetRange", static_cast<void (*)(pragma::gui::types::WIProgressBar &, float, float)>([](pragma::gui::types::WIProgressBar &el, float min, float max) { el.SetRange(min, max); }));
	classDef.def("SetOptions", &pragma::gui::types::WIProgressBar::SetOptions);
	classDef.def("AddOption", &pragma::gui::types::WIProgressBar::AddOption);
	classDef.def("SetPostFix", &pragma::gui::types::WIProgressBar::SetPostFix);
	classDef.def(
	  "GetRange", +[](pragma::gui::types::WIProgressBar &el) -> std::tuple<float, float, float> {
		  auto range = el.GetRange();
		  return {range[0], range[1], range[2]};
	  });
	//classDef.def("SetValueTranslator",&SetValueTranslator);
}

void Lua::WISlider::register_class(luabind::class_<pragma::gui::types::WISlider, luabind::bases<pragma::gui::types::WIProgressBar, pragma::gui::types::WIBase>> &classDef) { classDef.def("IsBeingDragged", &pragma::gui::types::WISlider::IsBeingDragged); }

void Lua::WIShape::register_class(luabind::class_<pragma::gui::types::WIShape, pragma::gui::types::WIBase> &classDef)
{
	classDef.def("AddVertex", &pragma::gui::types::WIShape::AddVertex);
	classDef.def("SetVertexPos", &pragma::gui::types::WIShape::SetVertexPos);
	classDef.def("ClearVertices", &pragma::gui::types::WIShape::ClearVertices);
	classDef.def("GetBuffer", &pragma::gui::types::WIShape::GetBuffer, luabind::shared_from_this_policy<0> {});
	classDef.def("SetBuffer", &pragma::gui::types::WIShape::SetBuffer);
	classDef.def("ClearBuffer", &pragma::gui::types::WIShape::ClearBuffer);
	classDef.def("GetVertexCount", &pragma::gui::types::WIShape::GetVertexCount);
	classDef.def("InvertVertexPositions", static_cast<void (pragma::gui::types::WIShape::*)(bool, bool)>(&pragma::gui::types::WIShape::InvertVertexPositions));
	classDef.def("InvertVertexPositions", static_cast<void (*)(pragma::gui::types::WIShape &, bool)>([](pragma::gui::types::WIShape &el, bool x) { el.InvertVertexPositions(x); }));
	classDef.def("InvertVertexPositions", static_cast<void (*)(pragma::gui::types::WIShape &)>([](pragma::gui::types::WIShape &el) { el.InvertVertexPositions(); }));
	classDef.def("SetShape", &pragma::gui::types::WIShape::SetShape);
	classDef.add_static_constant("SHAPE_RECTANGLE", pragma::math::to_integral(pragma::gui::types::WIShape::BasicShape::Rectangle));
	classDef.add_static_constant("SHAPE_CIRCLE", pragma::math::to_integral(pragma::gui::types::WIShape::BasicShape::Circle));
}

void Lua::WITexturedShape::register_class(luabind::class_<pragma::gui::types::WITexturedShape, luabind::bases<pragma::gui::types::WIShape, pragma::gui::types::WIBase>> &classDef)
{
	classDef.def("SetMaterial", static_cast<void (pragma::gui::types::WITexturedShape::*)(pragma::material::Material *)>(&pragma::gui::types::WITexturedShape::SetMaterial));
	classDef.def("SetMaterial", static_cast<void (pragma::gui::types::WITexturedShape::*)(const std::string &)>(&pragma::gui::types::WITexturedShape::SetMaterial));
	classDef.def("GetMaterial", &pragma::gui::types::WITexturedShape::GetMaterial);
	classDef.def("SetTexture", static_cast<void (*)(pragma::gui::types::WITexturedShape &, prosper::Texture &)>([](pragma::gui::types::WITexturedShape &shape, prosper::Texture &tex) { shape.SetTexture(tex); }));
	classDef.def("GetTexture", &pragma::gui::types::WITexturedShape::GetTexture);
	classDef.def("AddVertex", static_cast<uint32_t (pragma::gui::types::WITexturedShape::*)(::Vector2)>(&pragma::gui::types::WITexturedShape::AddVertex));
	classDef.def("AddVertex", static_cast<uint32_t (pragma::gui::types::WITexturedShape::*)(::Vector2, ::Vector2)>(&pragma::gui::types::WITexturedShape::AddVertex));
	classDef.def("SetVertexUVCoord", &pragma::gui::types::WITexturedShape::SetVertexUVCoord);
	classDef.def("InvertVertexUVCoordinates", static_cast<void (pragma::gui::types::WITexturedShape::*)(bool, bool)>(&pragma::gui::types::WITexturedShape::InvertVertexUVCoordinates));
	classDef.def("InvertVertexUVCoordinates", static_cast<void (*)(pragma::gui::types::WITexturedShape &, bool)>([](pragma::gui::types::WITexturedShape &el, bool x) { el.InvertVertexUVCoordinates(x); }));
	classDef.def("InvertVertexUVCoordinates", static_cast<void (*)(pragma::gui::types::WITexturedShape &)>([](pragma::gui::types::WITexturedShape &el) { el.InvertVertexUVCoordinates(); }));
	classDef.def("ClearTexture", &pragma::gui::types::WITexturedShape::ClearTexture);
	classDef.def("SizeToTexture", &pragma::gui::types::WITexturedShape::SizeToTexture);
	classDef.def("GetTextureSize", &pragma::gui::types::WITexturedShape::GetTextureSize);
	classDef.def("SetChannelSwizzle", &pragma::gui::types::WITexturedShape::SetChannelSwizzle);
	classDef.def("SetChannelSwizzle", &pragma::gui::types::WITexturedShape::GetChannelSwizzle);
	classDef.def("SetShader", static_cast<void (pragma::gui::types::WITexturedShape::*)(pragma::gui::shaders::ShaderTextured &)>(&pragma::gui::types::WITexturedShape::SetShader));
	classDef.def("GetAlphaMode", &pragma::gui::types::WITexturedShape::GetAlphaMode);
	classDef.def("SetAlphaMode", &pragma::gui::types::WITexturedShape::SetAlphaMode);
	classDef.def("SetAlphaCutoff", &pragma::gui::types::WITexturedShape::SetAlphaCutoff);
	classDef.def("GetAlphaCutoff", &pragma::gui::types::WITexturedShape::GetAlphaCutoff);
	classDef.add_static_constant("CHANNEL_RED", pragma::math::to_integral(pragma::gui::shaders::ShaderTextured::Channel::Red));
	classDef.add_static_constant("CHANNEL_GREEN", pragma::math::to_integral(pragma::gui::shaders::ShaderTextured::Channel::Green));
	classDef.add_static_constant("CHANNEL_BLUE", pragma::math::to_integral(pragma::gui::shaders::ShaderTextured::Channel::Blue));
	classDef.add_static_constant("CHANNEL_ALPHA", pragma::math::to_integral(pragma::gui::shaders::ShaderTextured::Channel::Alpha));
}

void Lua::WIIcon::register_class(luabind::class_<pragma::gui::types::WIIcon, luabind::bases<pragma::gui::types::WITexturedShape, pragma::gui::types::WIShape, pragma::gui::types::WIBase>> &classDef) { classDef.def("SetClipping", &pragma::gui::types::WIIcon::SetClipping); }

void Lua::WISilkIcon::register_class(luabind::class_<pragma::gui::types::WISilkIcon, luabind::bases<pragma::gui::types::WIIcon, pragma::gui::types::WITexturedShape, pragma::gui::types::WIShape, pragma::gui::types::WIBase>> &classDef)
{
	classDef.def("SetIcon", &pragma::gui::types::WISilkIcon::SetIcon);
}

void Lua::WIArrow::register_class(luabind::class_<pragma::gui::types::WIArrow, luabind::bases<pragma::gui::types::WIShape, pragma::gui::types::WIBase>> &classDef)
{
	classDef.def("SetDirection", &pragma::gui::types::WIArrow::SetDirection);
	classDef.add_static_constant("DIRECTION_RIGHT", 0);
	classDef.add_static_constant("DIRECTION_DOWN", 1);
	classDef.add_static_constant("DIRECTION_LEFT", 2);
	classDef.add_static_constant("DIRECTION_UP", 3);
}

void Lua::WICheckbox::register_class(luabind::class_<pragma::gui::types::WICheckbox, luabind::bases<pragma::gui::types::WIShape, pragma::gui::types::WIBase>> &classDef)
{
	classDef.def("SetChecked", &pragma::gui::types::WICheckbox::SetChecked);
	classDef.def("IsChecked", &pragma::gui::types::WICheckbox::IsChecked);
	classDef.def("Toggle", &pragma::gui::types::WICheckbox::Toggle);
}

void Lua::WIGridPanel::register_class(luabind::class_<pragma::gui::types::WIGridPanel, luabind::bases<pragma::gui::types::WITable, pragma::gui::types::WIBase>> &classDef)
{
	classDef.def("AddItem", &pragma::gui::types::WIGridPanel::AddItem);
	classDef.def("GetColumnCount", &pragma::gui::types::WIGridPanel::GetColumnCount);
}

void Lua::WITreeList::register_class(luabind::class_<pragma::gui::types::WITreeList, luabind::bases<pragma::gui::types::WITable, pragma::gui::types::WIBase>> &classDef)
{
	classDef.def("AddItem", static_cast<pragma::gui::types::WITreeListElement *(*)(lua::State *, pragma::gui::types::WITreeList &, const std::string &)>([](lua::State *l, pragma::gui::types::WITreeList &hPanel, const std::string &text) -> pragma::gui::types::WITreeListElement * {
		return hPanel.AddItem(text);
	}));
	classDef.def("AddItem",
	  static_cast<pragma::gui::types::WIBase *(*)(lua::State *, pragma::gui::types::WITreeList &, const std::string &, func<void(pragma::gui::types::WIBase)>)>(
	    [](lua::State *l, pragma::gui::types::WITreeList &hPanel, const std::string &text, func<void(pragma::gui::types::WIBase)> populate) -> pragma::gui::types::WIBase * {
		    auto fPopulate = [l, populate](pragma::gui::types::WITreeListElement &el) {
			    CallFunction(l, [&populate, &el](lua::State *l) {
				    populate.push(l);
				    auto o = pragma::gui::WGUILuaInterface::GetLuaObject(l, el);
				    o.push(l);
				    return StatusCode::Ok;
			    });
		    };
		    return hPanel.AddItem(text, fPopulate);
	    }));
	classDef.def("ExpandAll", &pragma::gui::types::WITreeList::ExpandAll);
	classDef.def("CollapseAll", &pragma::gui::types::WITreeList::CollapseAll);
	classDef.def("GetRootItem", &pragma::gui::types::WITreeList::GetRootItem);
}
void Lua::WITreeListElement::register_class(luabind::class_<pragma::gui::types::WITreeListElement, luabind::bases<pragma::gui::types::WITableRow, pragma::gui::types::WIBase>> &classDef)
{
	classDef.def("Expand", static_cast<void (pragma::gui::types::WITreeListElement::*)(bool)>(&pragma::gui::types::WITreeListElement::Expand));
	classDef.def("Expand", static_cast<void (*)(pragma::gui::types::WITreeListElement &)>([](pragma::gui::types::WITreeListElement &el) { el.Expand(); }));
	classDef.def("Collapse", static_cast<void (pragma::gui::types::WITreeListElement::*)(bool)>(&pragma::gui::types::WITreeListElement::Collapse));
	classDef.def("Collapse", static_cast<void (*)(pragma::gui::types::WITreeListElement &)>([](pragma::gui::types::WITreeListElement &el) { el.Collapse(); }));
	classDef.def("GetItems", &pragma::gui::types::WITreeListElement::GetItems);
	classDef.def("AddItem",
	  static_cast<pragma::gui::types::WITreeListElement *(*)(lua::State *, pragma::gui::types::WITreeListElement &, const std::string &)>(
	    [](lua::State *l, pragma::gui::types::WITreeListElement &hPanel, const std::string &text) -> pragma::gui::types::WITreeListElement * { return hPanel.AddItem(text); }));
	classDef.def("AddItem",
	  static_cast<pragma::gui::types::WIBase *(*)(lua::State *, pragma::gui::types::WITreeListElement &, const std::string &, func<void(pragma::gui::types::WIBase)>)>(
	    [](lua::State *l, pragma::gui::types::WITreeListElement &hPanel, const std::string &text, func<void(pragma::gui::types::WIBase)> populate) -> pragma::gui::types::WIBase * {
		    auto fPopulate = [l, populate](pragma::gui::types::WITreeListElement &el) {
			    CallFunction(l, [&populate, &el](lua::State *l) {
				    populate.push(l);
				    auto o = pragma::gui::WGUILuaInterface::GetLuaObject(l, el);
				    o.push(l);
				    return StatusCode::Ok;
			    });
		    };
		    return hPanel.AddItem(text, fPopulate);
	    }));
}

void Lua::WIContainer::register_class(luabind::class_<pragma::gui::types::WIContainer, pragma::gui::types::WIBase> &classDef)
{
	classDef.def("SetPadding", static_cast<void (pragma::gui::types::WIContainer::*)(int32_t)>(&pragma::gui::types::WIContainer::SetPadding));
	classDef.def("SetPadding", static_cast<void (pragma::gui::types::WIContainer::*)(int32_t, int32_t, int32_t, int32_t)>(&pragma::gui::types::WIContainer::SetPadding));
	classDef.def("SetPaddingTop", &pragma::gui::types::WIContainer::SetPaddingTop);
	classDef.def("SetPaddingRight", &pragma::gui::types::WIContainer::SetPaddingRight);
	classDef.def("SetPaddingBottom", &pragma::gui::types::WIContainer::SetPaddingBottom);
	classDef.def("SetPaddingLeft", &pragma::gui::types::WIContainer::SetPaddingLeft);
	classDef.def("GetPadding", static_cast<const std::array<int32_t, 4> &(pragma::gui::types::WIContainer::*)() const>(&pragma::gui::types::WIContainer::GetPadding));
	classDef.def("GetPaddingTop", &pragma::gui::types::WIContainer::GetPaddingTop);
	classDef.def("GetPaddingRight", &pragma::gui::types::WIContainer::GetPaddingRight);
	classDef.def("GetPaddingBottom", &pragma::gui::types::WIContainer::GetPaddingBottom);
	classDef.def("GetPaddingLeft", &pragma::gui::types::WIContainer::GetPaddingLeft);
}

void Lua::WITable::register_class(luabind::class_<pragma::gui::types::WITable, luabind::bases<pragma::gui::types::WIContainer, pragma::gui::types::WIBase>> &classDef)
{
	classDef.def("GetRowHeight", &pragma::gui::types::WITable::GetRowHeight);
	classDef.def("SetRowHeight", &pragma::gui::types::WITable::SetRowHeight);
	classDef.def("SetSelectableMode", &pragma::gui::types::WITable::SetSelectable);
	classDef.def("GetSelectableMode", &pragma::gui::types::WITable::GetSelectableMode);
	classDef.def("SetColumnWidth", &pragma::gui::types::WITable::SetColumnWidth);
	classDef.def("AddRow", static_cast<pragma::gui::types::WITableRow *(*)(pragma::gui::types::WITable &)>([](pragma::gui::types::WITable &el) { return el.AddRow(); }));
	classDef.def("AddHeaderRow", &pragma::gui::types::WITable::AddHeaderRow);
	classDef.def("GetRowCount", &pragma::gui::types::WITable::GetRowCount);
	classDef.def("SetSortable", &pragma::gui::types::WITable::SetSortable);
	classDef.def("IsSortable", &pragma::gui::types::WITable::IsSortable);
	classDef.def("Sort", static_cast<void (pragma::gui::types::WITable::*)()>(&pragma::gui::types::WITable::Sort));
	classDef.def(
	  "SetSortFunction", +[](lua::State *l, pragma::gui::types::WITable &table, const func<bool, const pragma::gui::types::WITableRow &, const pragma::gui::types::WITableRow &> &lfunc) {
		  table.SetSortFunction([l, lfunc](const pragma::gui::types::WITableRow &rowA, const pragma::gui::types::WITableRow &rowB, uint32_t columnIndex, bool ascending) -> bool {
			  auto r = CallFunction(
			    l,
			    [&lfunc, &rowA, &rowB, columnIndex, ascending](lua::State *l) {
				    lfunc.push(l);
				    Lua::Push<pragma::gui::types::WIBase *>(l, const_cast<pragma::gui::types::WITableRow *>(&rowA));
				    Lua::Push<pragma::gui::types::WIBase *>(l, const_cast<pragma::gui::types::WITableRow *>(&rowB));
				    Lua::Push<uint32_t>(l, columnIndex);
				    Lua::Push<bool>(l, ascending);
				    return StatusCode::Ok;
			    },
			    1);
			  if(r == StatusCode::Ok) {
				  auto res = CheckBool(l, -1);
				  Pop(l, 1);
				  return res;
			  }
			  return false;
		  });
	  });

	classDef.def("SetScrollable", &pragma::gui::types::WITable::SetScrollable);
	classDef.def("IsScrollable", &pragma::gui::types::WITable::IsScrollable);
	classDef.def("Clear", &pragma::gui::types::WITable::Clear);
	classDef.def("Clear", &pragma::gui::types::WITable::Clear, luabind::default_parameter_policy<2, false> {});
	classDef.def("GetRow", &pragma::gui::types::WITable::GetRow);
	classDef.def("GetSelectedRows", &GetSelectedRows);
	classDef.def("GetRows", &GetRows);
	classDef.def("RemoveRow", &pragma::gui::types::WITable::RemoveRow);
	classDef.def("MoveRow", &pragma::gui::types::WITable::MoveRow);
	classDef.def("MoveRow", static_cast<void (*)(pragma::gui::types::WITable &, pragma::gui::types::WITableRow &, pragma::gui::types::WITableRow &)>([](pragma::gui::types::WITable &el, pragma::gui::types::WITableRow &a, pragma::gui::types::WITableRow &pos) { el.MoveRow(&a, &pos); }));
	classDef.def("SelectRow", &pragma::gui::types::WITable::SelectRow);
	classDef.def("GetFirstSelectedRow", &pragma::gui::types::WITable::GetFirstSelectedRow);
	classDef.add_static_constant("SELECTABLE_MODE_NONE", pragma::math::to_integral(pragma::gui::types::WITable::SelectableMode::None));
	classDef.add_static_constant("SELECTABLE_MODE_SINGLE", pragma::math::to_integral(pragma::gui::types::WITable::SelectableMode::Single));
	classDef.add_static_constant("SELECTABLE_MODE_MULTI", pragma::math::to_integral(pragma::gui::types::WITable::SelectableMode::Multi));
}

void Lua::WITableRow::register_class(luabind::class_<pragma::gui::types::WITableRow, luabind::bases<pragma::gui::types::WIContainer, pragma::gui::types::WIBase>> &classDef)
{
	classDef.def("Select", &pragma::gui::types::WITableRow::Select);
	classDef.def("Deselect", &pragma::gui::types::WITableRow::Deselect);
	classDef.def("IsSelected", &pragma::gui::types::WITableRow::IsSelected);
	classDef.def("SetCellWidth", &pragma::gui::types::WITableRow::SetCellWidth);
	classDef.def("SetValue", &pragma::gui::types::WITableRow::SetValue);
	classDef.def("GetValue", static_cast<std::string (pragma::gui::types::WIBase::*)(uint32_t) const>(&pragma::gui::types::WITableRow::GetValue));
	classDef.def("InsertElement", static_cast<pragma::gui::types::WITableCell *(pragma::gui::types::WIBase::*)(uint32_t, pragma::gui::types::WIBase *)>(&pragma::gui::types::WITableRow::InsertElement));
	classDef.def("GetCellCount", &pragma::gui::types::WITableRow::GetCellCount);
	classDef.def("GetCell", &pragma::gui::types::WITableRow::GetCell);
	classDef.def("GetRowIndex", static_cast<uint32_t (*)(lua::State *, pragma::gui::types::WITableRow &)>([](lua::State *l, pragma::gui::types::WITableRow &hRow) -> uint32_t {
		auto *pTable = hRow.GetTable();
		uint32_t rowIndex = 0u;
		if(pTable)
			rowIndex = pTable->GetRowIndex(&hRow);
		return rowIndex;
	}));
}

void Lua::WITableCell::register_class(luabind::class_<pragma::gui::types::WITableCell, luabind::bases<pragma::gui::types::WIContainer, pragma::gui::types::WIBase>> &classDef)
{
	classDef.def("GetFirstElement", &pragma::gui::types::WITableCell::GetFirstElement);
	classDef.def("GetColSpan", &pragma::gui::types::WITableCell::GetColSpan);
	//classDef.def("GetRowSpan",&GetRowSpan);
	classDef.def("SetColSpan", &pragma::gui::types::WITableCell::SetColSpan);
	//classDef.def("SetRowSpan",&SetRowSpan);
}

void Lua::WIFrame::register_class(luabind::class_<pragma::gui::types::WIFrame, luabind::bases<pragma::gui::types::WITransformable, pragma::gui::types::WIBase>> &classDef)
{
	classDef.def("SetTitle", &pragma::gui::types::WIFrame::SetTitle);
	classDef.def("GetTitle", &pragma::gui::types::WIFrame::GetTitle);
	classDef.def("SetCloseButtonEnabled", &pragma::gui::types::WIFrame::SetCloseButtonEnabled);
	classDef.def("SetDetachButtonEnabled", &pragma::gui::types::WIFrame::SetDetachButtonEnabled);
}

void Lua::WIDropDownMenu::register_class(luabind::class_<pragma::gui::types::WIDropDownMenu, luabind::bases<pragma::gui::types::WITextEntry, pragma::gui::types::WIBase>> &classDef)
{
	classDef.def("SelectOption", static_cast<void (pragma::gui::types::WIDropDownMenu::*)(uint32_t)>(&pragma::gui::types::WIDropDownMenu::SelectOption));
	classDef.def("SelectOption", static_cast<void (pragma::gui::types::WIDropDownMenu::*)(const std::string &)>(&pragma::gui::types::WIDropDownMenu::SelectOption));
	classDef.def("ClearOptions", &pragma::gui::types::WIDropDownMenu::ClearOptions);
	classDef.def("SelectOptionByText", +[](pragma::gui::types::WIDropDownMenu &menu, const std::string &text) { return menu.SelectOptionByText(text); });
	classDef.def("GetOptionText", +[](pragma::gui::types::WIDropDownMenu &menu, uint32_t idx) { return menu.GetOptionText(idx).cpp_str(); });
	classDef.def(
	  "FindOptionIndex", +[](pragma::gui::types::WIDropDownMenu &menu, const std::string &value) -> std::optional<uint32_t> {
		  auto n = menu.GetOptionCount();
		  for(auto i = decltype(n) {0u}; i < n; ++i) {
			  auto val = menu.GetOptionValue(i);
			  if(val == value)
				  return i;
		  }
		  return {};
	  });
	classDef.def("GetOptionValue", &pragma::gui::types::WIDropDownMenu::GetOptionValue);
	classDef.def("SetOptionText", &pragma::gui::types::WIDropDownMenu::SetOptionText);
	classDef.def("SetOptionValue", &pragma::gui::types::WIDropDownMenu::SetOptionValue);
	classDef.def("GetValue", &pragma::gui::types::WIDropDownMenu::GetValue);
	classDef.def("GetText", +[](const pragma::gui::types::WIDropDownMenu &menu) { return menu.GetText().cpp_str(); });
	classDef.def("SetText", +[](pragma::gui::types::WIDropDownMenu &menu, const std::string &text) { return menu.SetText(text); });
	classDef.def("GetOptionCount", &pragma::gui::types::WIDropDownMenu::GetOptionCount);
	classDef.def("AddOption", static_cast<pragma::gui::types::WIDropDownMenuOption *(pragma::gui::types::WIDropDownMenu::*)(const std::string &, const std::string &)>(&pragma::gui::types::WIDropDownMenu::AddOption));
	classDef.def("AddOption", static_cast<pragma::gui::types::WIDropDownMenuOption *(pragma::gui::types::WIDropDownMenu::*)(const std::string &)>(&pragma::gui::types::WIDropDownMenu::AddOption));
	classDef.def("OpenMenu", &pragma::gui::types::WIDropDownMenu::OpenMenu);
	classDef.def("CloseMenu", &pragma::gui::types::WIDropDownMenu::CloseMenu);
	classDef.def("ToggleMenu", &pragma::gui::types::WIDropDownMenu::ToggleMenu);
	classDef.def("IsMenuOpen", &pragma::gui::types::WIDropDownMenu::IsMenuOpen);
	classDef.def("HasOption", &pragma::gui::types::WIDropDownMenu::HasOption);
	classDef.def("GetSelectedOption", &pragma::gui::types::WIDropDownMenu::GetSelectedOption);
	classDef.def("ClearSelectedOption", &pragma::gui::types::WIDropDownMenu::ClearSelectedOption);
	classDef.def("GetOptionElement", &pragma::gui::types::WIDropDownMenu::GetOptionElement);
	classDef.def("FindOptionSelectedByCursor", &pragma::gui::types::WIDropDownMenu::FindOptionSelectedByCursor);
	classDef.def("SetListItemCount", &pragma::gui::types::WIDropDownMenu::SetListItemCount);
	classDef.def("ScrollToOption", static_cast<void (pragma::gui::types::WIDropDownMenu::*)(uint32_t, bool)>(&pragma::gui::types::WIDropDownMenu::ScrollToOption));
	classDef.def("ScrollToOption", static_cast<void (*)(pragma::gui::types::WIDropDownMenu &, uint32_t)>([](pragma::gui::types::WIDropDownMenu &el, uint32_t offset) { el.ScrollToOption(offset); }));
}

void Lua::WIText::register_class(luabind::class_<pragma::gui::types::WIText, pragma::gui::types::WIBase> &classDef)
{
	classDef.def("SetText", static_cast<void (*)(lua::State *, pragma::gui::types::WIText &, const std::string &)>([](lua::State *l, pragma::gui::types::WIText &hPanel, const std::string &text) { hPanel.SetText(text); }));
	classDef.def("GetText", +[](const pragma::gui::types::WIText &text) { return text.GetText().cpp_str(); });
	classDef.def("GetTextHeight", &pragma::gui::types::WIText::GetTextHeight);
	classDef.def("CalcTextSize", &pragma::gui::types::WIText::CalcTextSize);
	classDef.def("GetTextWidth", &pragma::gui::types::WIText::GetTextWidth);
	classDef.def("SetFont", static_cast<void (*)(lua::State *, pragma::gui::types::WIText &, const std::string &)>([](lua::State *l, pragma::gui::types::WIText &hPanel, const std::string &font) { hPanel.SetFont(font); }));
	classDef.def(
	  "GetFont", +[](lua::State *l, pragma::gui::types::WIText &hPanel) -> std::optional<std::string> {
		  auto *font = hPanel.GetFont();
		  if(!font)
			  return {};
		  return font->GetName();
	  });
	classDef.def("UpdateSubLines", &pragma::gui::types::WIText::UpdateSubLines);
	classDef.def("EnableShadow", &pragma::gui::types::WIText::EnableShadow);
	classDef.def("IsShadowEnabled", &pragma::gui::types::WIText::IsShadowEnabled);
	classDef.def("SetShadowColor", &SetShadowColor);
	classDef.def("SetShadowOffset", &SetShadowOffset);
	classDef.def("SetShadowXOffset", &SetShadowXOffset);
	classDef.def("SetShadowYOffset", &SetShadowYOffset);
	classDef.def("GetShadowColor", &GetShadowColor);
	classDef.def("GetShadowOffset", &GetShadowOffset);
	classDef.def("GetShadowXOffset", &GetShadowXOffset);
	classDef.def("GetShadowYOffset", &GetShadowYOffset);
	classDef.def("SetShadowAlpha", &pragma::gui::types::WIText::SetShadowAlpha);
	classDef.def("GetShadowAlpha", &pragma::gui::types::WIText::GetShadowAlpha);
	classDef.def("SetShadowBlurSize", &pragma::gui::types::WIText::SetShadowBlurSize);
	classDef.def("GetShadowBlurSize", &pragma::gui::types::WIText::GetShadowBlurSize);
	classDef.def("SetAutoBreakMode", &pragma::gui::types::WIText::SetAutoBreakMode);
	classDef.def("GetLineCount", &pragma::gui::types::WIText::GetLineCount);
	classDef.def("GetLine", static_cast<std::optional<std::string> (*)(lua::State *, pragma::gui::types::WIText &, int32_t)>([](lua::State *l, pragma::gui::types::WIText &hPanel, int32_t lineIndex) -> std::optional<std::string> {
		auto *pLine = hPanel.GetLine(lineIndex);
		if(pLine == nullptr)
			return {};
		return pLine->GetUnformattedLine().GetText().cpp_str();
	}));
	classDef.def("GetTextLength", static_cast<uint32_t (*)(lua::State *, pragma::gui::types::WIText &)>([](lua::State *l, pragma::gui::types::WIText &hPanel) -> uint32_t { return hPanel.GetText().length(); }));
	classDef.def("SetTagArgument", static_cast<void (*)(lua::State *, pragma::gui::types::WIText &, const std::string &, uint32_t, luabind::object)>([](lua::State *l, pragma::gui::types::WIText &hPanel, const std::string &label, uint32_t argIdx, luabind::object o) {
		if(IsString(l, 4)) {
			std::string arg = CheckString(l, 4);
			hPanel.SetTagArgument(label, argIdx, arg);
		}
		else if(Lua::IsType<::Vector4>(l, 4)) {
			auto &arg = Lua::Check<::Vector4>(l, 4);
			hPanel.SetTagArgument(label, argIdx, arg);
		}
		else if(Lua::IsType<::Color>(l, 4)) {
			auto &arg = Lua::Check<::Color>(l, 4);
			hPanel.SetTagArgument(label, argIdx, arg);
		}
		else if(IsFunction(l, 4)) {
			auto f = luabind::object(luabind::from_stack(l, 4));
			auto arg = FunctionCallback<pragma::util::EventReply>::CreateWithOptionalReturn([f, l](pragma::util::EventReply *reply) -> CallbackReturnType {
				auto r = CallFunction(
				  l,
				  [&f](lua::State *l) {
					  f.push(l);
					  return StatusCode::Ok;
				  },
				  1);
				if(r == StatusCode::Ok) {
					if(IsSet(l, -1))
						*reply = static_cast<pragma::util::EventReply>(CheckInt(l, -1));
					else
						*reply = pragma::util::EventReply::Unhandled;
					return (*reply != pragma::util::EventReply::Unhandled) ? CallbackReturnType::HasReturnValue : CallbackReturnType::NoReturnValue;
				}
				return CallbackReturnType::NoReturnValue;
			});
			hPanel.SetTagArgument(label, argIdx, arg);
			Lua::Push<CallbackHandle>(l, arg);
		}
	}));
	classDef.def("SetTagsEnabled", &pragma::gui::types::WIText::SetTagsEnabled);
	classDef.def("AreTagsEnabled", &pragma::gui::types::WIText::AreTagsEnabled);
	classDef.def("PopFrontLine", &pragma::gui::types::WIText::PopFrontLine);
	classDef.def("PopBackLine", &pragma::gui::types::WIText::PopBackLine);
	classDef.def("RemoveText", static_cast<bool (pragma::gui::types::WIBase::*)(pragma::string::LineIndex, pragma::string::CharOffset, pragma::string::TextLength)>(&pragma::gui::types::WIText::RemoveText));
	classDef.def("RemoveText", static_cast<bool (pragma::gui::types::WIBase::*)(pragma::string::TextOffset, pragma::string::TextLength)>(&pragma::gui::types::WIText::RemoveText));
	classDef.def("RemoveLine", &pragma::gui::types::WIText::RemoveLine);
	classDef.def("InsertText", static_cast<bool (*)(pragma::gui::types::WIText &, const std::string &, pragma::string::LineIndex)>([](pragma::gui::types::WIText &hPanel, const std::string &text, pragma::string::LineIndex lineIdx) { return hPanel.InsertText(text, lineIdx); }));
	classDef.def("InsertText",
	  static_cast<bool (*)(pragma::gui::types::WIText &, const std::string &, pragma::string::LineIndex, pragma::string::CharOffset)>(
	    [](pragma::gui::types::WIText &hPanel, const std::string &text, pragma::string::LineIndex lineIdx, pragma::string::CharOffset charOffset) { return hPanel.InsertText(text, lineIdx, charOffset); }));
	classDef.def("SetMaxLineCount", +[](lua::State *l, pragma::gui::types::WIText &hPanel, uint32_t c) { hPanel.GetFormattedTextObject().SetMaxLineCount(c); });
	classDef.def("GetMaxLineCount", +[](lua::State *l, pragma::gui::types::WIText &hPanel) { return hPanel.GetFormattedTextObject().GetMaxLineCount(); });
	classDef.def("AppendText", +[](pragma::gui::types::WIText &el, const std::string &text) { return el.AppendText(text); });
	classDef.def("AppendLine", static_cast<void (*)(lua::State *, pragma::gui::types::WIText &, const std::string &)>([](lua::State *l, pragma::gui::types::WIText &hPanel, const std::string &line) { hPanel.AppendLine(line); }));
	classDef.def("MoveText",
	  static_cast<void (*)(lua::State *, pragma::gui::types::WIText &, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t)>(
	    [](lua::State *l, pragma::gui::types::WIText &hPanel, uint32_t lineIdx, uint32_t startOffset, uint32_t len, uint32_t targetLineIdx, uint32_t targetCharOffset) { PushBool(l, hPanel.MoveText(lineIdx, startOffset, len, targetLineIdx, targetCharOffset)); }));
	classDef.def("Clear", &pragma::gui::types::WIText::Clear);
	classDef.def("Substr", +[](pragma::gui::types::WIText &el, pragma::string::TextOffset startOffset, pragma::string::TextLength len) { return el.Substr(startOffset, len); });
	classDef.add_static_constant("AUTO_BREAK_NONE", pragma::math::to_integral(pragma::gui::types::WIText::AutoBreak::NONE));
	classDef.add_static_constant("AUTO_BREAK_ANY", pragma::math::to_integral(pragma::gui::types::WIText::AutoBreak::ANY));
	classDef.add_static_constant("AUTO_BREAK_WHITESPACE", pragma::math::to_integral(pragma::gui::types::WIText::AutoBreak::WHITESPACE));
}

void Lua::WITextEntry::register_class(luabind::class_<pragma::gui::types::WITextEntry, pragma::gui::types::WIBase> &classDef)
{
	classDef.def("SetText", +[](pragma::gui::types::WITextEntry &el, const std::string &text) { return el.SetText(text); });
	classDef.def("GetText", +[](const pragma::gui::types::WITextEntry &el) { return el.GetText().cpp_str(); });
	classDef.def("GetValue", +[](const pragma::gui::types::WITextEntry &el) { return el.GetText().cpp_str(); });
	classDef.def("IsNumeric", &pragma::gui::types::WITextEntry::IsNumeric);
	classDef.def("IsEditable", &pragma::gui::types::WITextEntry::IsEditable);
	classDef.def("SetEditable", &pragma::gui::types::WITextEntry::SetEditable);
	classDef.def("SetMaxLength", &pragma::gui::types::WITextEntry::SetMaxLength);
	classDef.def("GetMaxLength", &pragma::gui::types::WITextEntry::GetMaxLength);
	classDef.def("IsMultiLine", &pragma::gui::types::WITextEntry::IsMultiLine);
	classDef.def("SetMultiLine", &pragma::gui::types::WITextEntry::SetMultiLine);

	classDef.def("IsSelectable", &pragma::gui::types::WITextEntry::IsSelectable);
	classDef.def("SetSelectable", &pragma::gui::types::WITextEntry::SetSelectable);
	classDef.def("GetTextElement", &pragma::gui::types::WITextEntry::GetTextElement);

	classDef.def("GetCaretPos", &pragma::gui::types::WITextEntry::GetCaretPos);
	classDef.def("SetCaretPos", &pragma::gui::types::WITextEntry::SetCaretPos);
}

////////////////////////////////////

void Lua::WINumericEntry::register_class(luabind::class_<pragma::gui::types::WINumericEntry, luabind::bases<pragma::gui::types::WITextEntry, pragma::gui::types::WIBase>> &classDef)
{
	classDef.def("SetMinValue", static_cast<void (pragma::gui::types::WINumericEntry::*)(int32_t)>(&pragma::gui::types::WINumericEntry::SetMinValue));
	classDef.def("SetMinValue", static_cast<void (pragma::gui::types::WINumericEntry::*)()>(&pragma::gui::types::WINumericEntry::SetMinValue));
	classDef.def("SetMaxValue", static_cast<void (pragma::gui::types::WINumericEntry::*)(int32_t)>(&pragma::gui::types::WINumericEntry::SetMaxValue));
	classDef.def("SetMaxValue", static_cast<void (pragma::gui::types::WINumericEntry::*)()>(&pragma::gui::types::WINumericEntry::SetMaxValue));
	classDef.def("SetRange", &pragma::gui::types::WINumericEntry::SetRange);
	classDef.def("GetMinValue", &pragma::gui::types::WINumericEntry::GetMinValue);
	classDef.def("GetMaxValue", &pragma::gui::types::WINumericEntry::GetMaxValue);
}

////////////////////////////////////

void Lua::WIOutlinedRect::register_class(luabind::class_<pragma::gui::types::WIOutlinedRect, pragma::gui::types::WIBase> &classDef)
{
	classDef.def("SetOutlineWidth", &pragma::gui::types::WIOutlinedRect::SetOutlineWidth);
	classDef.def("GetOutlineWidth", &pragma::gui::types::WIOutlinedRect::GetOutlineWidth);
}

void Lua::WILine::register_class(luabind::class_<pragma::gui::types::WILine, pragma::gui::types::WIBase> &classDef)
{
	classDef.def("SetLineWidth", &pragma::gui::types::WILine::SetLineWidth);
	classDef.def("GetLineWidth", &pragma::gui::types::WILine::GetLineWidth);
	classDef.def("SetStartPos", &SetStartPos);
	classDef.def("GetStartPos", &GetStartPos);
	classDef.def("SetEndPos", &SetEndPos);
	classDef.def("GetEndPos", &GetEndPos);
	classDef.def("SetStartColor", &pragma::gui::types::WILine::SetStartColor);
	classDef.def("SetEndColor", &pragma::gui::types::WILine::SetEndColor);
	classDef.def("GetStartColor", &pragma::gui::types::WILine::GetStartColor, luabind::copy_policy<0> {});
	classDef.def("GetEndColor", &pragma::gui::types::WILine::GetEndColor, luabind::copy_policy<0> {});
	classDef.def("GetStartPosProperty", &pragma::gui::types::WILine::GetStartPosProperty);
	classDef.def("GetEndPosProperty", &pragma::gui::types::WILine::GetEndPosProperty);
}

void Lua::WIRoundedRect::register_class(luabind::class_<pragma::gui::types::WIRoundedRect, luabind::bases<pragma::gui::types::WIShape, pragma::gui::types::WIBase>> &classDef)
{
	classDef.def("GetRoundness", &pragma::gui::types::WIRoundedRect::GetRoundness);
	classDef.def("SetRoundness", &pragma::gui::types::WIRoundedRect::SetRoundness);
	classDef.def("SetCornerSize", &pragma::gui::types::WIRoundedRect::SetCornerSize);
	classDef.def("GetCornerSize", &pragma::gui::types::WIRoundedRect::GetCornerSize);
	classDef.def("SetRoundTopRight", &pragma::gui::types::WIRoundedRect::SetRoundTopRight);
	classDef.def("SetRoundTopLeft", &pragma::gui::types::WIRoundedRect::SetRoundTopLeft);
	classDef.def("SetRoundBottomLeft", &pragma::gui::types::WIRoundedRect::SetRoundBottomLeft);
	classDef.def("SetRoundBottomRight", &pragma::gui::types::WIRoundedRect::SetRoundBottomRight);
	classDef.def("IsTopRightRound", &pragma::gui::types::WIRoundedRect::IsTopRightRound);
	classDef.def("IsTopLeftRound", &pragma::gui::types::WIRoundedRect::IsTopLeftRound);
	classDef.def("IsBottomLeftRound", &pragma::gui::types::WIRoundedRect::IsBottomLeftRound);
	classDef.def("IsBottomRightRound", &pragma::gui::types::WIRoundedRect::IsBottomRightRound);
}

void Lua::WIRoundedTexturedRect::register_class(luabind::class_<pragma::gui::types::WIRoundedTexturedRect, luabind::bases<pragma::gui::types::WITexturedShape, pragma::gui::types::WIShape, pragma::gui::types::WIBase>> &classDef)
{
	classDef.def("GetRoundness", &pragma::gui::types::WIRoundedTexturedRect::GetRoundness);
	classDef.def("SetRoundness", &pragma::gui::types::WIRoundedTexturedRect::SetRoundness);
	classDef.def("SetCornerSize", &pragma::gui::types::WIRoundedTexturedRect::SetCornerSize);
	classDef.def("GetCornerSize", &pragma::gui::types::WIRoundedTexturedRect::GetCornerSize);
	classDef.def("SetRoundTopRight", &pragma::gui::types::WIRoundedTexturedRect::SetRoundTopRight);
	classDef.def("SetRoundTopLeft", &pragma::gui::types::WIRoundedTexturedRect::SetRoundTopLeft);
	classDef.def("SetRoundBottomLeft", &pragma::gui::types::WIRoundedTexturedRect::SetRoundBottomLeft);
	classDef.def("SetRoundBottomRight", &pragma::gui::types::WIRoundedTexturedRect::SetRoundBottomRight);
	classDef.def("IsTopRightRound", &pragma::gui::types::WIRoundedTexturedRect::IsTopRightRound);
	classDef.def("IsTopLeftRound", &pragma::gui::types::WIRoundedTexturedRect::IsTopLeftRound);
	classDef.def("IsBottomLeftRound", &pragma::gui::types::WIRoundedTexturedRect::IsBottomLeftRound);
	classDef.def("IsBottomRightRound", &pragma::gui::types::WIRoundedTexturedRect::IsBottomRightRound);
}

void Lua::WIScrollBar::register_class(luabind::class_<pragma::gui::types::WIScrollBar, pragma::gui::types::WIBase> &classDef)
{
	classDef.def("GetScrollAmount", &pragma::gui::types::WIScrollBar::GetScrollAmount);
	classDef.def("SetScrollAmount", &pragma::gui::types::WIScrollBar::SetScrollAmount);
	classDef.def("GetScrollOffset", &pragma::gui::types::WIScrollBar::GetScrollOffset);
	classDef.def("SetScrollOffset", &pragma::gui::types::WIScrollBar::SetScrollOffset);
	classDef.def("AddScrollOffset", &pragma::gui::types::WIScrollBar::AddScrollOffset);
	classDef.def("SetUp", &pragma::gui::types::WIScrollBar::SetUp);
	classDef.def("SetHorizontal", &pragma::gui::types::WIScrollBar::SetHorizontal);
	classDef.def("IsHorizontal", &pragma::gui::types::WIScrollBar::IsHorizontal);
	classDef.def("IsVertical", &pragma::gui::types::WIScrollBar::IsVertical);
}

bool Lua::WIBase::IsValid(lua::State *l, pragma::gui::types::WIBase *hPanel) { return hPanel != nullptr; }

void Lua::WIBase::TrapFocus(lua::State *l, pragma::gui::types::WIBase &hPanel, bool bTrap) { hPanel.TrapFocus(bTrap); }
void Lua::WIBase::TrapFocus(lua::State *l, pragma::gui::types::WIBase &hPanel) { TrapFocus(l, hPanel, true); }
void Lua::WIBase::SetPos(lua::State *l, pragma::gui::types::WIBase &hPanel, ::Vector2 pos) { hPanel.SetPos(CInt32(pos.x), CInt32(pos.y)); }
void Lua::WIBase::SetPos(lua::State *l, pragma::gui::types::WIBase &hPanel, float x, float y) { hPanel.SetPos(CInt32(x), CInt32(y)); }
void Lua::WIBase::SetAbsolutePos(lua::State *l, pragma::gui::types::WIBase &hPanel, ::Vector2 pos) { hPanel.SetAbsolutePos(::Vector2i(pos.x, pos.y)); }
void Lua::WIBase::SetAbsolutePos(lua::State *l, pragma::gui::types::WIBase &hPanel, float x, float y) { hPanel.SetAbsolutePos(::Vector2i(x, y)); }
void Lua::WIBase::SetColor(lua::State *l, pragma::gui::types::WIBase &hPanel, ::Color col) { hPanel.SetColor(col.r / 255.f, col.g / 255.f, col.b / 255.f, col.a / 255.f); }
void Lua::WIBase::GetAlpha(lua::State *l, pragma::gui::types::WIBase &hPanel) { PushNumber(l, hPanel.GetAlpha() * 255); }
void Lua::WIBase::SetAlpha(lua::State *l, pragma::gui::types::WIBase &hPanel, float alpha) { hPanel.SetAlpha(alpha / 255.f); }
void Lua::WIBase::SetSize(lua::State *l, pragma::gui::types::WIBase &hPanel, ::Vector2 size) { hPanel.SetSize(CInt32(size.x), CInt32(size.y)); }
void Lua::WIBase::SetSize(lua::State *l, pragma::gui::types::WIBase &hPanel, float x, float y) { hPanel.SetSize(CInt32(x), CInt32(y)); }
void Lua::WIBase::Wrap(lua::State *l, pragma::gui::types::WIBase &hPanel, const std::string &wrapperClassName)
{
	auto *el = pragma::get_cgame()->CreateGUIElement(wrapperClassName);
	if(el == nullptr)
		return;
	auto hWrapper = el->GetHandle();
	if(hPanel.Wrap(*hWrapper.get()) == false) {
		el->RemoveSafely();
		return;
	}
	auto o = pragma::gui::WGUILuaInterface::GetLuaObject(l, *el);
	o.push(l);
}
void Lua::WIBase::ClearParent(lua::State *l, pragma::gui::types::WIBase &hPanel) { hPanel.SetParent(nullptr); }
void Lua::WIBase::ResetParent(lua::State *l, pragma::gui::types::WIBase &hPanel) { hPanel.SetParent(pragma::gui::WGUI::GetInstance().GetBaseElement()); }
void Lua::WIBase::GetChildren(lua::State *l, pragma::gui::types::WIBase &hPanel, std::string className)
{
	std::vector<pragma::gui::WIHandle> *children = hPanel.GetChildren();
	int table = CreateTable(l);
	unsigned int c = 1;
	for(unsigned int i = 0; i < children->size(); i++) {
		pragma::gui::WIHandle &hChild = (*children)[i];
		if(hChild.IsValid()) {
			auto *pChild = hChild.get();
			if(pChild->GetClass() == className) {
				auto oChild = pragma::gui::WGUILuaInterface::GetLuaObject(l, *pChild);
				PushInt(l, c);
				oChild.push(l);
				SetTableValue(l, table);
				c++;
			}
		}
	}
}
void Lua::WIBase::GetChild(lua::State *l, pragma::gui::types::WIBase &hPanel, unsigned int idx)
{
	auto *el = hPanel.GetChild(idx);
	if(el == nullptr)
		return;
	auto oChild = pragma::gui::WGUILuaInterface::GetLuaObject(l, *el);
	oChild.push(l);
}
void Lua::WIBase::GetChild(lua::State *l, pragma::gui::types::WIBase &hPanel, std::string className, unsigned int idx)
{
	auto *el = hPanel.GetChild(className, idx);
	if(el == nullptr)
		return;
	auto oChild = pragma::gui::WGUILuaInterface::GetLuaObject(l, *el);
	oChild.push(l);
}
void Lua::WIBase::PosInBounds(lua::State *l, pragma::gui::types::WIBase &hPanel, ::Vector2 pos) { lua::push_boolean(l, hPanel.PosInBounds(CInt32(pos.x), CInt32(pos.y))); }
void Lua::WIBase::GetMousePos(lua::State *l, pragma::gui::types::WIBase &hPanel)
{
	int x, y;
	hPanel.GetMousePos(&x, &y);
	luabind::object(l, ::Vector2(x, y)).push(l);
}
void Lua::WIBase::Draw(lua::State *l, pragma::gui::types::WIBase &hPanel, const pragma::gui::DrawInfo &drawInfo, pragma::gui::DrawState &drawState) { hPanel.Draw(drawInfo, drawState); }
void Lua::WIBase::Draw(lua::State *l, pragma::gui::types::WIBase &hPanel, const pragma::gui::DrawInfo &drawInfo, pragma::gui::DrawState &drawState, const ::Vector2i &scissorOffset, const ::Vector2i &scissorSize)
{
	hPanel.Draw(drawInfo, drawState, ::Vector2i {}, scissorOffset, scissorSize, hPanel.GetScale());
}
void Lua::WIBase::Draw(lua::State *l, pragma::gui::types::WIBase &hPanel, const pragma::gui::DrawInfo &drawInfo, pragma::gui::DrawState &drawState, const ::Vector2i &scissorOffset, const ::Vector2i &scissorSize, const ::Vector2i &offsetParent)
{
	hPanel.Draw(drawInfo, drawState, offsetParent, scissorOffset, scissorSize, hPanel.GetScale());
}
void Lua::WIBase::Draw(lua::State *l, pragma::gui::types::WIBase &hPanel, const pragma::gui::DrawInfo &drawInfo, pragma::gui::DrawState &drawState, const ::Vector2i &scissorOffset, const ::Vector2i &scissorSize, const ::Vector2i &offsetParent, const ::Vector2 &scale)
{
	hPanel.Draw(drawInfo, drawState, offsetParent, scissorOffset, scissorSize, scale);
}
void Lua::WIBase::GetX(lua::State *l, pragma::gui::types::WIBase &hPanel)
{
	::Vector2i pos = hPanel.GetPos();
	PushInt(l, pos.x);
}
void Lua::WIBase::GetY(lua::State *l, pragma::gui::types::WIBase &hPanel)
{
	::Vector2i pos = hPanel.GetPos();
	PushInt(l, pos.y);
}
void Lua::WIBase::SetX(lua::State *l, pragma::gui::types::WIBase &hPanel, float x)
{
	::Vector2i pos = hPanel.GetPos();
	hPanel.SetPos(::Vector2i(x, pos.y));
}
void Lua::WIBase::SetY(lua::State *l, pragma::gui::types::WIBase &hPanel, float y)
{
	::Vector2i pos = hPanel.GetPos();
	hPanel.SetPos(::Vector2i(pos.x, y));
}
void Lua::WIBase::SetWidth(lua::State *l, pragma::gui::types::WIBase &hPanel, float w)
{
	::Vector2i size = hPanel.GetSize();
	hPanel.SetSize(::Vector2i(w, size.y));
}
void Lua::WIBase::SetHeight(lua::State *l, pragma::gui::types::WIBase &hPanel, float h)
{
	::Vector2i size = hPanel.GetSize();
	hPanel.SetSize(::Vector2i(size.x, h));
}

struct LuaCallbacks {
	struct CallbackInfo {
		CallbackInfo(lua::State *l, const CallbackHandle &_hCallback, const luabind::object &f) : luaState(l), hCallback(_hCallback), luaFunction(f) {}
		CallbackHandle hCallback;
		luabind::object luaFunction;
		lua::State *luaState;
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

static std::unordered_map<pragma::util::Hash, Lua::gui::LUA_CALLBACK> g_uiCallbacks;
static pragma::util::Hash get_gui_callback_hash(const std::string &className, const std::string &callbackName)
{
	pragma::util::Hash hash = 0;
	hash = pragma::util::hash_combine<size_t>(hash, std::hash<std::string> {}(className));
	hash = pragma::util::hash_combine<size_t>(hash, std::hash<std::string> {}(callbackName));
	return hash;
}
void Lua::gui::register_lua_callback(std::string className, std::string callbackName, LUA_CALLBACK fCb)
{
	pragma::string::to_lower(className);
	pragma::string::to_lower(callbackName);
	g_uiCallbacks[get_gui_callback_hash(className, callbackName)] = fCb;
}

void Lua::gui::clear_lua_callbacks(lua::State *l)
{
	auto &wgui = pragma::gui::WGUI::GetInstance();
	std::function<void(pragma::gui::types::WIBase &)> fIterateElements = nullptr;
	fIterateElements = [&fIterateElements, l](pragma::gui::types::WIBase &el) {
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
		void CallCallbacks(lua::State *l, pragma::gui::types::WIBase &hPanel, std::string name, TARGS... args)
		{
			auto callbackPtr = std::static_pointer_cast<LuaCallbacks>(hPanel.GetUserData4());
			if(callbackPtr == nullptr)
				return;
			pragma::string::to_lower(name);
			auto itCallbacks = callbackPtr->callbacks.find(name);
			if(itCallbacks == callbackPtr->callbacks.end())
				return;
			uint32_t argOffset = 3;
			auto numArgs = GetStackTop(l) - argOffset + 1;
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
					auto n = GetStackTop(l);
					auto r = pragma::scripting::lua_core::protected_call(
					  l,
					  [&](lua::State *l) -> StatusCode {
						  o.push(l);
						  auto obj = pragma::gui::WGUILuaInterface::GetLuaObject(l, hPanel);
						  obj.push(l);
						  for(auto i = decltype(numArgs) {0}; i < numArgs; ++i) {
							  auto arg = argOffset + i;
							  PushValue(l, arg);
						  }
						  return StatusCode::Ok;
					  },
					  lua::MultiReturn);
					if(r == StatusCode::Ok) {
						auto numResults = GetStackTop(l) - n;
						if(numResults > 0)
							bReturn = true;
					}
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
void Lua::WIBase::CallCallbacks(lua::State *l, pragma::gui::types::WIBase &hPanel, std::string name) { CallCallbacks<>(l, hPanel, name); }
void Lua::WIBase::CallCallbacks(lua::State *l, pragma::gui::types::WIBase &hPanel, std::string name, luabind::object o1) { CallCallbacks<luabind::object>(l, hPanel, name, o1); }
void Lua::WIBase::CallCallbacks(lua::State *l, pragma::gui::types::WIBase &hPanel, std::string name, luabind::object o1, luabind::object o2) { CallCallbacks<luabind::object, luabind::object>(l, hPanel, name, o1, o2); }
void Lua::WIBase::CallCallbacks(lua::State *l, pragma::gui::types::WIBase &hPanel, std::string name, luabind::object o1, luabind::object o2, luabind::object o3) { CallCallbacks<luabind::object, luabind::object, luabind::object>(l, hPanel, name, o1, o2, o3); }
void Lua::WIBase::CallCallbacks(lua::State *l, pragma::gui::types::WIBase &hPanel, std::string name, luabind::object o1, luabind::object o2, luabind::object o3, luabind::object o4)
{
	CallCallbacks<luabind::object, luabind::object, luabind::object, luabind::object>(l, hPanel, name, o1, o2, o3, o4);
}
void Lua::WIBase::CallCallbacks(lua::State *l, pragma::gui::types::WIBase &hPanel, std::string name, luabind::object o1, luabind::object o2, luabind::object o3, luabind::object o4, luabind::object o5)
{
	CallCallbacks<luabind::object, luabind::object, luabind::object, luabind::object, luabind::object>(l, hPanel, name, o1, o2, o3, o4, o5);
}
void Lua::WIBase::CallCallbacks(lua::State *l, pragma::gui::types::WIBase &hPanel, std::string name, luabind::object o1, luabind::object o2, luabind::object o3, luabind::object o4, luabind::object o5, luabind::object o6)
{
	CallCallbacks<luabind::object, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object>(l, hPanel, name, o1, o2, o3, o4, o5, o6);
}
void Lua::WIBase::CallCallbacks(lua::State *l, pragma::gui::types::WIBase &hPanel, std::string name, luabind::object o1, luabind::object o2, luabind::object o3, luabind::object o4, luabind::object o5, luabind::object o6, luabind::object o7)
{
	CallCallbacks<luabind::object, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object>(l, hPanel, name, o1, o2, o3, o4, o5, o6, o7);
}
void Lua::WIBase::CallCallbacks(lua::State *l, pragma::gui::types::WIBase &hPanel, std::string name, luabind::object o1, luabind::object o2, luabind::object o3, luabind::object o4, luabind::object o5, luabind::object o6, luabind::object o7, luabind::object o8)
{
	CallCallbacks<luabind::object, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object>(l, hPanel, name, o1, o2, o3, o4, o5, o6, o7, o8);
}
CallbackHandle Lua::WIBase::AddCallback(lua::State *l, pragma::gui::types::WIBase &panel, std::string name, func<void> o)
{
	CallbackHandle hCallback {};
	pragma::string::to_lower(name);

	auto callbackPtr = std::static_pointer_cast<LuaCallbacks>(panel.GetUserData4());
	if(callbackPtr == nullptr) {
		callbackPtr = pragma::util::make_shared<LuaCallbacks>();
		panel.SetUserData4(callbackPtr);
	}
	auto hPanel = panel.GetHandle();
	auto it = g_uiCallbacks.find(get_gui_callback_hash(panel.GetClass(), name));
	if(it != g_uiCallbacks.end()) {
		hCallback = it->second(panel, l, [l, o, &panel](const std::function<void()> &pushArgs) {
			CallFunction(
			  l,
			  [&pushArgs, &o, &panel](lua::State *l) mutable {
				  o.push(l);

				  auto obj = pragma::gui::WGUILuaInterface::GetLuaObject(l, panel);
				  obj.push(l);

				  pushArgs();
				  return StatusCode::Ok;
			  },
			  0);
		});
	}
	else if(name == "ontextchanged") {
		if(pragma::string::compare(panel.GetClass(), std::string {"witext"}, false)) {
			hCallback = FunctionCallback<void, std::reference_wrapper<const pragma::string::Utf8String>>::Create([l, hPanel, o](std::reference_wrapper<const pragma::string::Utf8String> text) mutable {
				if(!hPanel.IsValid())
					return;
				CallFunction(
				  l,
				  [&o, hPanel, text](lua::State *l) mutable {
					  o.push(l);

					  auto obj = pragma::gui::WGUILuaInterface::GetLuaObject(l, *hPanel.get());
					  obj.push(l);
					  PushString(l, text.get().cpp_str());
					  return StatusCode::Ok;
				  },
				  0);
			});
		}
		else {
			hCallback = FunctionCallback<void, std::reference_wrapper<const pragma::string::Utf8String>, bool>::Create([l, hPanel, o](std::reference_wrapper<const pragma::string::Utf8String> text, bool changedByUser) mutable {
				if(!hPanel.IsValid())
					return;
				CallFunction(
				  l,
				  [&o, hPanel, text, changedByUser](lua::State *l) mutable {
					  o.push(l);

					  auto obj = pragma::gui::WGUILuaInterface::GetLuaObject(l, *hPanel.get());
					  obj.push(l);
					  PushString(l, text.get().cpp_str());
					  PushBool(l, changedByUser);
					  return StatusCode::Ok;
				  },
				  0);
			});
		}
	}
	else if(name == "handlelinktagaction") {
		hCallback = FunctionCallback<pragma::util::EventReply, std::string>::CreateWithOptionalReturn([l, hPanel, o](pragma::util::EventReply *reply, std::string arg) mutable -> CallbackReturnType {
			if(!hPanel.IsValid())
				return CallbackReturnType::NoReturnValue;
			if(CallFunction(
			     l,
			     [&o, hPanel, &arg](lua::State *l) mutable {
				     o.push(l);

				     auto obj = pragma::gui::WGUILuaInterface::GetLuaObject(l, *hPanel.get());
				     obj.push(l);

				     PushString(l, arg);

				     return StatusCode::Ok;
			     },
			     1)
			  == StatusCode::Ok) {
				if(IsSet(l, -1) == false) {
					Pop(l, 1);
					return CallbackReturnType::NoReturnValue;
				}
				auto result = static_cast<pragma::util::EventReply>(CheckInt(l, -1));
				Pop(l, 1);
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
			CallFunction(
			  l,
			  [&o, hPanel, offset](lua::State *l) mutable {
				  o.push(l);

				  auto obj = pragma::gui::WGUILuaInterface::GetLuaObject(l, *hPanel.get());
				  obj.push(l);
				  PushInt(l, offset);
				  return StatusCode::Ok;
			  },
			  0);
		});
	}
	else if(name == "oncharevent") {
		hCallback = FunctionCallback<pragma::util::EventReply, int, pragma::platform::Modifier>::CreateWithOptionalReturn([l, hPanel, o](pragma::util::EventReply *reply, int c, pragma::platform::Modifier mods) mutable -> CallbackReturnType {
			if(!hPanel.IsValid())
				return CallbackReturnType::NoReturnValue;
			if(CallFunction(
			     l,
			     [&o, hPanel, c, mods](lua::State *l) mutable {
				     o.push(l);

				     auto obj = pragma::gui::WGUILuaInterface::GetLuaObject(l, *hPanel.get());
				     obj.push(l);
				     PushString(l, std::string(1, static_cast<char>(c)));
				     PushInt(l, pragma::math::to_integral(mods));
				     return StatusCode::Ok;
			     },
			     1)
			  == StatusCode::Ok) {
				if(IsSet(l, -1) == false) {
					Pop(l, 1);
					return CallbackReturnType::NoReturnValue;
				}
				auto result = static_cast<pragma::util::EventReply>(CheckInt(l, -1));
				Pop(l, 1);
				*reply = result;
				return CallbackReturnType::NoReturnValue; // We'll always return 'NoReturnValue' to allow other callbacks for this element to be executed as well
			}
			return CallbackReturnType::NoReturnValue;
		});
	}
	else if(name == "onkeyevent") {
		hCallback = FunctionCallback<pragma::util::EventReply, pragma::platform::Key, int, pragma::platform::KeyState, pragma::platform::Modifier>::CreateWithOptionalReturn(
		  [l, hPanel, o](pragma::util::EventReply *reply, pragma::platform::Key key, int, pragma::platform::KeyState action, pragma::platform::Modifier mods) mutable -> CallbackReturnType {
			  if(!hPanel.IsValid())
				  return CallbackReturnType::NoReturnValue;
			  if(CallFunction(
			       l,
			       [&o, hPanel, key, action, mods](lua::State *l) mutable {
				       o.push(l);

				       auto obj = pragma::gui::WGUILuaInterface::GetLuaObject(l, *hPanel.get());
				       obj.push(l);
				       PushInt(l, pragma::math::to_integral(key));
				       PushInt(l, pragma::math::to_integral(action));
				       PushInt(l, pragma::math::to_integral(mods));
				       return StatusCode::Ok;
			       },
			       1)
			    == StatusCode::Ok) {
				  if(IsSet(l, -1) == false) {
					  Pop(l, 1);
					  return CallbackReturnType::NoReturnValue;
				  }
				  auto result = static_cast<pragma::util::EventReply>(CheckInt(l, -1));
				  Pop(l, 1);
				  *reply = result;
				  return CallbackReturnType::NoReturnValue; // We'll always return 'NoReturnValue' to allow other callbacks for this element to be executed as well
			  }
			  return CallbackReturnType::NoReturnValue;
		  });
	}
	else if(name == "onmouseevent") {
		hCallback = FunctionCallback<pragma::util::EventReply, pragma::platform::MouseButton, pragma::platform::KeyState, pragma::platform::Modifier>::CreateWithOptionalReturn(
		  [l, hPanel, o](pragma::util::EventReply *reply, pragma::platform::MouseButton button, pragma::platform::KeyState action, pragma::platform::Modifier mods) mutable -> CallbackReturnType {
			  if(!hPanel.IsValid())
				  return CallbackReturnType::NoReturnValue;
			  if(CallFunction(
			       l,
			       [&o, hPanel, button, action, mods](lua::State *l) mutable {
				       o.push(l);

				       auto obj = pragma::gui::WGUILuaInterface::GetLuaObject(l, *hPanel.get());
				       obj.push(l);
				       PushInt(l, pragma::math::to_integral(button));
				       PushInt(l, pragma::math::to_integral(action));
				       PushInt(l, pragma::math::to_integral(mods));
				       return StatusCode::Ok;
			       },
			       1)
			    == StatusCode::Ok) {
				  if(IsSet(l, -1) == false) {
					  Pop(l, 1);
					  return CallbackReturnType::NoReturnValue;
				  }
				  auto result = static_cast<pragma::util::EventReply>(CheckInt(l, -1));
				  Pop(l, 1);
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
			CallFunction(l, [&o, hPanel, x, y](lua::State *l) mutable {
				o.push(l);

				auto obj = pragma::gui::WGUILuaInterface::GetLuaObject(l, *hPanel.get());
				obj.push(l);
				PushInt(l, x);
				PushInt(l, y);
				return StatusCode::Ok;
			});
		});
	}
	else if(name == "onchildadded") {
		hCallback = FunctionCallback<void, pragma::gui::types::WIBase *>::Create([l, hPanel, o](pragma::gui::types::WIBase *el) mutable {
			if(!hPanel.IsValid())
				return;
			CallFunction(l, [&o, hPanel, el](lua::State *l) mutable {
				o.push(l);

				auto obj = pragma::gui::WGUILuaInterface::GetLuaObject(l, *hPanel.get());
				obj.push(l);
				if(el) {
					auto objEl = pragma::gui::WGUILuaInterface::GetLuaObject(l, *el);
					objEl.push(l);
				}
				return StatusCode::Ok;
			});
		});
	}
	else if(name == "onshowtooltip") {
		hCallback = FunctionCallback<void, pragma::gui::types::WITooltip *>::Create([l, hPanel, o](pragma::gui::types::WITooltip *el) mutable {
			if(!hPanel.IsValid())
				return;
			CallFunction(l, [&o, hPanel, el](lua::State *l) mutable {
				o.push(l);

				auto obj = pragma::gui::WGUILuaInterface::GetLuaObject(l, *hPanel.get());
				obj.push(l);
				if(el) {
					auto objEl = pragma::gui::WGUILuaInterface::GetLuaObject(l, *el);
					objEl.push(l);
				}
				return StatusCode::Ok;
			});
		});
	}
	else if(name == "onchildremoved") {
		hCallback = FunctionCallback<void, pragma::gui::types::WIBase *>::Create([l, hPanel, o](pragma::gui::types::WIBase *el) mutable {
			if(!hPanel.IsValid())
				return;
			CallFunction(l, [&o, hPanel, el](lua::State *l) mutable {
				o.push(l);

				auto obj = pragma::gui::WGUILuaInterface::GetLuaObject(l, *hPanel.get());
				obj.push(l);
				if(el) {
					auto objEl = pragma::gui::WGUILuaInterface::GetLuaObject(l, *el);
					objEl.push(l);
				}
				return StatusCode::Ok;
			});
		});
	}
	else if(name == "onmousepressed") {
		hCallback = FunctionCallback<pragma::util::EventReply>::CreateWithOptionalReturn([l, hPanel, o](pragma::util::EventReply *reply) mutable -> CallbackReturnType {
			if(!hPanel.IsValid())
				return CallbackReturnType::NoReturnValue;
			if(CallFunction(
			     l,
			     [&o, hPanel](lua::State *l) mutable {
				     o.push(l);

				     auto obj = pragma::gui::WGUILuaInterface::GetLuaObject(l, *hPanel.get());
				     obj.push(l);
				     return StatusCode::Ok;
			     },
			     1)
			  == StatusCode::Ok) {
				if(IsSet(l, -1) == false) {
					Pop(l, 1);
					return CallbackReturnType::NoReturnValue;
				}
				auto result = static_cast<pragma::util::EventReply>(CheckInt(l, -1));
				Pop(l, 1);
				*reply = result;
				return CallbackReturnType::NoReturnValue; // We'll always return 'NoReturnValue' to allow other callbacks for this element to be executed as well
			}
			return CallbackReturnType::NoReturnValue;
		});
	}
	else if(name == "onmousereleased") {
		hCallback = FunctionCallback<pragma::util::EventReply>::CreateWithOptionalReturn([l, hPanel, o](pragma::util::EventReply *reply) mutable -> CallbackReturnType {
			if(!hPanel.IsValid())
				return CallbackReturnType::NoReturnValue;
			if(CallFunction(
			     l,
			     [&o, hPanel](lua::State *l) mutable {
				     o.push(l);

				     auto obj = pragma::gui::WGUILuaInterface::GetLuaObject(l, *hPanel.get());
				     obj.push(l);
				     return StatusCode::Ok;
			     },
			     1)
			  == StatusCode::Ok) {
				if(IsSet(l, -1) == false) {
					Pop(l, 1);
					return CallbackReturnType::NoReturnValue;
				}
				auto result = static_cast<pragma::util::EventReply>(CheckInt(l, -1));
				Pop(l, 1);
				*reply = result;
				return CallbackReturnType::NoReturnValue; // We'll always return 'NoReturnValue' to allow other callbacks for this element to be executed as well
			}
			return CallbackReturnType::NoReturnValue;
		});
	}
	else if(name == "ondoubleclick") {
		hCallback = FunctionCallback<pragma::util::EventReply>::CreateWithOptionalReturn([l, hPanel, o](pragma::util::EventReply *reply) mutable -> CallbackReturnType {
			if(!hPanel.IsValid())
				return CallbackReturnType::NoReturnValue;
			if(CallFunction(
			     l,
			     [&o, hPanel](lua::State *l) mutable {
				     o.push(l);

				     auto obj = pragma::gui::WGUILuaInterface::GetLuaObject(l, *hPanel.get());
				     obj.push(l);
				     return StatusCode::Ok;
			     },
			     1)
			  == StatusCode::Ok) {
				if(IsSet(l, -1) == false) {
					Pop(l, 1);
					return CallbackReturnType::NoReturnValue;
				}
				auto result = static_cast<pragma::util::EventReply>(CheckInt(l, -1));
				Pop(l, 1);
				*reply = result;
				return CallbackReturnType::NoReturnValue; // We'll always return 'NoReturnValue' to allow other callbacks for this element to be executed as well
			}
			return CallbackReturnType::NoReturnValue;
		});
	}
	else if(name == "onjoystickevent") {
		hCallback = FunctionCallback<pragma::util::EventReply, std::reference_wrapper<const pragma::platform::Joystick>, uint32_t, pragma::platform::KeyState>::CreateWithOptionalReturn(
		  [l, hPanel, o](pragma::util::EventReply *reply, std::reference_wrapper<const pragma::platform::Joystick> joystick, uint32_t key, pragma::platform::KeyState state) mutable -> CallbackReturnType {
			  if(!hPanel.IsValid())
				  return CallbackReturnType::NoReturnValue;
			  if(CallFunction(
			       l,
			       [&o, hPanel, key, state](lua::State *l) mutable {
				       o.push(l);

				       auto obj = pragma::gui::WGUILuaInterface::GetLuaObject(l, *hPanel.get());
				       obj.push(l);
				       PushInt(l, key);
				       PushInt(l, pragma::math::to_integral(state));
				       return StatusCode::Ok;
			       },
			       1)
			    == StatusCode::Ok) {
				  if(IsSet(l, -1) == false) {
					  Pop(l, 1);
					  return CallbackReturnType::NoReturnValue;
				  }
				  auto result = static_cast<pragma::util::EventReply>(CheckInt(l, -1));
				  Pop(l, 1);
				  *reply = result;
				  return CallbackReturnType::NoReturnValue; // We'll always return 'NoReturnValue' to allow other callbacks for this element to be executed as well
			  }
			  return CallbackReturnType::NoReturnValue;
		  });
	}
	else if(name == "onscroll") {
		hCallback = FunctionCallback<pragma::util::EventReply, ::Vector2, bool>::CreateWithOptionalReturn([l, hPanel, o](pragma::util::EventReply *reply, ::Vector2 offset, bool offsetAsPixels) mutable -> CallbackReturnType {
			if(!hPanel.IsValid())
				return CallbackReturnType::NoReturnValue;
			if(CallFunction(
			     l,
			     [&o, hPanel, &offset, &offsetAsPixels](lua::State *l) mutable {
				     o.push(l);

				     auto obj = pragma::gui::WGUILuaInterface::GetLuaObject(l, *hPanel.get());
				     obj.push(l);
				     PushNumber(l, offset.x);
				     PushNumber(l, offset.y);
				     PushBool(l, offsetAsPixels);
				     return StatusCode::Ok;
			     },
			     1)
			  == StatusCode::Ok) {
				if(IsSet(l, -1) == false) {
					Pop(l, 1);
					return CallbackReturnType::NoReturnValue;
				}
				auto result = static_cast<pragma::util::EventReply>(CheckInt(l, -1));
				Pop(l, 1);
				*reply = result;
				return CallbackReturnType::NoReturnValue; // We'll always return 'NoReturnValue' to allow other callbacks for this element to be executed as well
			}
			return CallbackReturnType::NoReturnValue;
		});
	}
	else if(name == "onchange" && dynamic_cast<pragma::gui::types::WICheckbox *>(hPanel.get()) != nullptr) {
		hCallback = FunctionCallback<void, bool>::Create([l, hPanel, o](bool bChecked) mutable {
			if(!hPanel.IsValid())
				return pragma::util::EventReply::Unhandled;
			if(CallFunction(
			     l,
			     [&o, hPanel, bChecked](lua::State *l) mutable {
				     o.push(l);

				     auto obj = pragma::gui::WGUILuaInterface::GetLuaObject(l, *hPanel.get());
				     obj.push(l);
				     PushBool(l, bChecked);
				     return StatusCode::Ok;
			     },
			     1)
			  == StatusCode::Ok) {
				auto result = IsSet(l, -1) ? static_cast<pragma::util::EventReply>(CheckInt(l, -1)) : pragma::util::EventReply::Unhandled;
				Pop(l, 1);
				return result;
			}
			return pragma::util::EventReply::Unhandled;
		});
	}
	else if(name == "onoptionselected") {
		hCallback = FunctionCallback<void, uint32_t>::Create([l, hPanel, o](uint32_t idx) mutable {
			if(!hPanel.IsValid())
				return;
			CallFunction(
			  l,
			  [&o, hPanel, idx](lua::State *l) mutable {
				  o.push(l);

				  auto *p = hPanel.get();
				  auto obj = pragma::gui::WGUILuaInterface::GetLuaObject(l, *p);
				  obj.push(l);

				  auto optIdx = (idx == std::numeric_limits<uint32_t>::max()) ? -1 : static_cast<int32_t>(idx);
				  PushInt(l, optIdx);
				  return StatusCode::Ok;
			  },
			  0);
		});
	}
	else if(name == "translatetransformposition") {
		hCallback = FunctionCallback<void, std::reference_wrapper<::Vector2i>, bool>::Create([l, hPanel, o](std::reference_wrapper<::Vector2i> pos, bool bDrag) mutable {
			if(!hPanel.IsValid())
				return;
			auto r = CallFunction(
			  l,
			  [&o, hPanel, pos, bDrag](lua::State *l) mutable {
				  o.push(l);

				  auto obj = pragma::gui::WGUILuaInterface::GetLuaObject(l, *hPanel.get());
				  obj.push(l);
				  Lua::Push<::Vector2i>(l, pos);
				  PushBool(l, bDrag);
				  return StatusCode::Ok;
			  },
			  1);
			if(r == StatusCode::Ok) {
				if(Lua::IsType<::Vector2i>(l, -1))
					pos.get() = Lua::Check<::Vector2i>(l, -1);
				Pop(l, 1);
			}
		});
	}
	else if(name == "onchange") {
		hCallback = FunctionCallback<void, float, float>::Create([l, hPanel, o](float progress, float value) mutable {
			if(!hPanel.IsValid())
				return;
			CallFunction(
			  l,
			  [&o, hPanel, progress, value](lua::State *l) mutable {
				  o.push(l);

				  auto *p = hPanel.get();
				  auto obj = pragma::gui::WGUILuaInterface::GetLuaObject(l, *p);
				  obj.push(l);

				  PushNumber(l, progress);
				  PushNumber(l, value);
				  return StatusCode::Ok;
			  },
			  0);
		});
	}
	else if(name == "onselectionchanged" && typeid(*hPanel.get()) == typeid(pragma::gui::types::WIDropDownMenuOption)) {
		hCallback = FunctionCallback<void, bool>::Create([l, hPanel, o](bool selected) mutable {
			if(!hPanel.IsValid())
				return;
			CallFunction(
			  l,
			  [&o, hPanel, selected](lua::State *l) mutable {
				  o.push(l);

				  auto *p = hPanel.get();
				  auto obj = pragma::gui::WGUILuaInterface::GetLuaObject(l, *p);
				  obj.push(l);

				  PushBool(l, selected);
				  return StatusCode::Ok;
			  },
			  0);
		});
	}
	else if(name == "translatevalue") {
		hCallback = FunctionCallback<bool, float, std::reference_wrapper<std::string>>::Create([l, hPanel, o](float rawValue, std::reference_wrapper<std::string> value) mutable {
			if(!hPanel.IsValid())
				return false;
			auto retVal = false;
			auto r = CallFunction(
			  l,
			  [&o, hPanel, rawValue, value, &retVal](lua::State *l) mutable {
				  o.push(l);

				  auto *p = hPanel.get();
				  auto obj = pragma::gui::WGUILuaInterface::GetLuaObject(l, *p);
				  obj.push(l);

				  PushNumber(l, rawValue);
				  return StatusCode::Ok;
			  },
			  1);
			if(r == StatusCode::Ok && IsSet(l, -1)) {
				value.get() = CheckString(l, -1);
				retVal = true;
			}
			return retVal;
		});
	}
	else {
		hCallback = FunctionCallback<>::Create([l, hPanel, o]() mutable {
			if(!hPanel.IsValid())
				return;
			CallFunction(
			  l,
			  [&o, hPanel](lua::State *l) mutable {
				  o.push(l);

				  auto obj = pragma::gui::WGUILuaInterface::GetLuaObject(l, *hPanel.get());
				  obj.push(l);
				  return StatusCode::Ok;
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
void Lua::WIBase::FadeIn(lua::State *l, pragma::gui::types::WIBase &hPanel, float tFadeIn, float alphaTarget) { hPanel.FadeIn(tFadeIn, alphaTarget / 255.f); }
void Lua::WIBase::FadeIn(lua::State *l, pragma::gui::types::WIBase &hPanel, float tFadeIn) { FadeIn(l, hPanel, tFadeIn, 255.f); }
static std::optional<Vector2> get_cursor_pos_override(pragma::gui::types::WIRoot *elRoot)
{
	if(!elRoot)
		return {};
	return elRoot->GetCursorPosOverride();
}
static void restore_cursor_pos_override(pragma::gui::types::WIRoot *elRoot, const std::optional<Vector2> &pos)
{
	if(!elRoot)
		return;
	if(pos)
		elRoot->SetCursorPosOverride(*pos);
	else
		elRoot->ClearCursorPosOverride();
}
void Lua::WIBase::InjectMouseMoveInput(lua::State *l, pragma::gui::types::WIBase &hPanel, const ::Vector2 &mousePos)
{
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	::debug::get_domain().BeginTask("inect_mouse_move_input");
	pragma::util::ScopeGuard sgVtune {[]() { ::debug::get_domain().EndTask(); }};
#endif
	auto *elRoot = hPanel.GetBaseRootElement();
	auto absPos = hPanel.GetAbsolutePos();
	auto origOverride = get_cursor_pos_override(elRoot);
	if(elRoot)
		elRoot->SetCursorPosOverride(::Vector2 {static_cast<float>(absPos.x + mousePos.x), static_cast<float>(absPos.y + mousePos.y)});
	pragma::util::ScopeGuard sg {[elRoot, &origOverride]() { restore_cursor_pos_override(elRoot, origOverride); }};
	hPanel.InjectMouseMoveInput(mousePos.x, mousePos.y);
}
pragma::util::EventReply Lua::WIBase::InjectMouseInput(lua::State *l, pragma::gui::types::WIBase &hPanel, const ::Vector2 &mousePos, int button, int action, int mods)
{
	auto *elRoot = hPanel.GetBaseRootElement();
	auto absPos = hPanel.GetAbsolutePos();
	auto origOverride = get_cursor_pos_override(elRoot);
	if(elRoot)
		elRoot->SetCursorPosOverride(::Vector2 {static_cast<float>(absPos.x + mousePos.x), static_cast<float>(absPos.y + mousePos.y)});
	pragma::util::ScopeGuard sg {[elRoot, &origOverride]() { restore_cursor_pos_override(elRoot, origOverride); }};
	return hPanel.InjectMouseInput(pragma::platform::MouseButton(button), pragma::platform::KeyState(action), pragma::platform::Modifier(mods));
}
pragma::util::EventReply Lua::WIBase::InjectMouseInput(lua::State *l, pragma::gui::types::WIBase &hPanel, const ::Vector2 &mousePos, int button, int action) { return InjectMouseInput(l, hPanel, mousePos, button, action, 0); }
pragma::util::EventReply Lua::WIBase::InjectMouseClick(lua::State *l, pragma::gui::types::WIBase &hPanel, const ::Vector2 &mousePos, int button, int mods)
{
	auto handled0 = InjectMouseInput(l, hPanel, mousePos, button, KEY_PRESS, mods);
	auto handled1 = InjectMouseInput(l, hPanel, mousePos, button, KEY_RELEASE, mods);
	if(handled1 == pragma::util::EventReply::Handled)
		handled0 = handled1;
	return handled0;
}
pragma::util::EventReply Lua::WIBase::InjectMouseClick(lua::State *l, pragma::gui::types::WIBase &hPanel, const ::Vector2 &mousePos, int button) { return InjectMouseClick(l, hPanel, mousePos, button, 0); }
pragma::util::EventReply Lua::WIBase::InjectKeyboardInput(lua::State *l, pragma::gui::types::WIBase &hPanel, int key, int action, int mods)
{
	return hPanel.InjectKeyboardInput(pragma::platform::Key(key), 0, pragma::platform::KeyState(action), pragma::platform::Modifier(mods));
	// Vulkan TODO
}
pragma::util::EventReply Lua::WIBase::InjectKeyboardInput(lua::State *l, pragma::gui::types::WIBase &hPanel, int key, int action) { return InjectKeyboardInput(l, hPanel, key, action, 0); }
pragma::util::EventReply Lua::WIBase::InjectKeyPress(lua::State *l, pragma::gui::types::WIBase &hPanel, int key, int mods)
{
	auto handled0 = InjectKeyboardInput(l, hPanel, key, KEY_PRESS, mods);
	auto handled1 = InjectKeyboardInput(l, hPanel, key, KEY_RELEASE, mods);
	if(handled1 == pragma::util::EventReply::Handled)
		handled0 = handled1;
	return handled0;
}
pragma::util::EventReply Lua::WIBase::InjectKeyPress(lua::State *l, pragma::gui::types::WIBase &hPanel, int key) { return InjectKeyPress(l, hPanel, key, 0); }
pragma::util::EventReply Lua::WIBase::InjectCharInput(lua::State *l, pragma::gui::types::WIBase &hPanel, std::string c, uint32_t mods)
{
	if(c.empty())
		return pragma::util::EventReply::Unhandled;
	const char *cStr = c.c_str();
	return hPanel.InjectCharInput(cStr[0], static_cast<pragma::platform::Modifier>(mods));
}
pragma::util::EventReply Lua::WIBase::InjectCharInput(lua::State *l, pragma::gui::types::WIBase &hPanel, std::string c)
{
	if(c.empty())
		return pragma::util::EventReply::Unhandled;
	const char *cStr = c.c_str();
	return hPanel.InjectCharInput(cStr[0]);
}
pragma::util::EventReply Lua::WIBase::InjectScrollInput(lua::State *l, pragma::gui::types::WIBase &hPanel, const ::Vector2 &mousePos, const ::Vector2 &offset, bool offsetAsPixels)
{
	auto *elRoot = hPanel.GetBaseRootElement();
	auto cursorPos = elRoot ? elRoot->GetCursorPos() : ::Vector2 {};
	auto absPos = hPanel.GetAbsolutePos();
	auto origOverride = get_cursor_pos_override(elRoot);
	if(elRoot)
		elRoot->SetCursorPosOverride(::Vector2 {static_cast<float>(absPos.x + mousePos.x), static_cast<float>(absPos.y + mousePos.y)});
	auto result = hPanel.InjectScrollInput(offset, offsetAsPixels);
	if(elRoot)
		restore_cursor_pos_override(elRoot, origOverride);
	return result;
}
pragma::util::EventReply Lua::WIBase::InjectScrollInput(lua::State *l, pragma::gui::types::WIBase &hPanel, const ::Vector2 &mousePos, const ::Vector2 &offset) { return InjectScrollInput(l, hPanel, mousePos, offset, false); }
void Lua::WIBase::FindChildByName(lua::State *l, pragma::gui::types::WIBase &hPanel, std::string name)
{
	auto *el = hPanel.FindChildByName(name);
	if(el == nullptr)
		return;
	auto oChild = pragma::gui::WGUILuaInterface::GetLuaObject(l, *el);
	oChild.push(l);
}
void Lua::WIBase::FindChildrenByName(lua::State *l, pragma::gui::types::WIBase &hPanel, std::string name)
{
	std::vector<pragma::gui::WIHandle> children;
	hPanel.FindChildrenByName(name, children);
	int table = CreateTable(l);
	unsigned int c = 1;
	for(unsigned int i = 0; i < children.size(); i++) {
		pragma::gui::WIHandle &hChild = children[i];
		if(hChild.IsValid()) {
			auto *pChild = hChild.get();
			auto oChild = pragma::gui::WGUILuaInterface::GetLuaObject(l, *pChild);
			PushInt(l, c);
			oChild.push(l);
			SetTableValue(l, table);
			c++;
		}
	}
}
void Lua::WIBase::SetBounds(lua::State *l, pragma::gui::types::WIBase &hPanel, const ::Vector2 &start, const ::Vector2 &end)
{
	auto &pos = start;
	auto size = end - start;
	hPanel.SetPos(pos);
	hPanel.SetSize(size);
}

////////////////////////////////////

void Lua::WIText::SetShadowColor(lua::State *l, pragma::gui::types::WIText &hPanel, ::Color &col) { hPanel.SetShadowColor(::Vector4(col.r / 255.f, col.g / 255.f, col.b / 255.f, col.a / 255.f)); }

void Lua::WIText::SetShadowOffset(lua::State *l, pragma::gui::types::WIText &hPanel, ::Vector2 pos) { hPanel.SetShadowOffset(CInt32(pos.x), CInt32(pos.y)); }

void Lua::WIText::SetShadowXOffset(lua::State *l, pragma::gui::types::WIText &hPanel, float x) { hPanel.SetShadowOffset(::Vector2i(x, hPanel.GetShadowOffset()->y)); }

void Lua::WIText::SetShadowYOffset(lua::State *l, pragma::gui::types::WIText &hPanel, float y) { hPanel.SetShadowOffset(::Vector2i(hPanel.GetShadowOffset()->x, y)); }

Color Lua::WIText::GetShadowColor(lua::State *l, pragma::gui::types::WIText &hPanel)
{

	::Vector4 *col = hPanel.GetShadowColor();
	return ::Color(CInt16(col->r * 255), CInt16(col->g * 255), CInt16(col->b * 255), CInt16(col->a * 255));
}

Vector2 Lua::WIText::GetShadowOffset(lua::State *l, pragma::gui::types::WIText &hPanel)
{
	::Vector2i *offset = hPanel.GetShadowOffset();
	return ::Vector2(offset->x, offset->y);
}

float Lua::WIText::GetShadowXOffset(lua::State *l, pragma::gui::types::WIText &hPanel) { return float(hPanel.GetShadowOffset()->x); }

float Lua::WIText::GetShadowYOffset(lua::State *l, pragma::gui::types::WIText &hPanel) { return float(hPanel.GetShadowOffset()->y); }

////////////////////////////////////

void Lua::WILine::SetStartPos(lua::State *l, pragma::gui::types::WILine &hPanel, ::Vector2 &pos) { hPanel.SetStartPos(::Vector2i(pos.x, pos.y)); }
void Lua::WILine::SetEndPos(lua::State *l, pragma::gui::types::WILine &hPanel, ::Vector2 &pos) { hPanel.SetEndPos(::Vector2i(pos.x, pos.y)); }
Vector2 Lua::WILine::GetStartPos(lua::State *l, pragma::gui::types::WILine &hPanel) { return hPanel.GetStartPos(); }
Vector2 Lua::WILine::GetEndPos(lua::State *l, pragma::gui::types::WILine &hPanel)
{
	::Vector2i &pos = hPanel.GetEndPos();
	return {pos.x, pos.y};
}

////////////////////////////////////

void Lua::WITransformable::register_class(luabind::class_<pragma::gui::types::WITransformable, pragma::gui::types::WIBase> &classDef)
{
	classDef.def("SetDraggable", &pragma::gui::types::WITransformable::SetDraggable);
	classDef.def("SetResizable", &pragma::gui::types::WITransformable::SetResizable);
	classDef.def("IsDraggable", &pragma::gui::types::WITransformable::IsDraggable);
	classDef.def("IsResizable", &pragma::gui::types::WITransformable::IsResizable);
	classDef.def("SetMinWidth", &pragma::gui::types::WITransformable::SetMinWidth);
	classDef.def("SetMinHeight", &pragma::gui::types::WITransformable::SetMinHeight);
	classDef.def("SetMinSize", static_cast<void (pragma::gui::types::WITransformable::*)(int, int)>(&pragma::gui::types::WITransformable::SetMinSize));
	classDef.def("SetMinSize", static_cast<void (pragma::gui::types::WITransformable::*)(::Vector2i)>(&pragma::gui::types::WITransformable::SetMinSize));
	classDef.def("GetMinWidth", &pragma::gui::types::WITransformable::GetMinWidth);
	classDef.def("GetMinHeight", &pragma::gui::types::WITransformable::GetMinHeight);
	classDef.def("GetMinSize", &pragma::gui::types::WITransformable::GetMinSize, luabind::copy_policy<0> {});
	classDef.def("SetMaxWidth", &pragma::gui::types::WITransformable::SetMaxWidth);
	classDef.def("SetMaxHeight", &pragma::gui::types::WITransformable::SetMaxHeight);
	classDef.def("SetMaxSize", static_cast<void (pragma::gui::types::WITransformable::*)(int, int)>(&pragma::gui::types::WITransformable::SetMaxSize));
	classDef.def("SetMaxSize", static_cast<void (pragma::gui::types::WITransformable::*)(::Vector2i)>(&pragma::gui::types::WITransformable::SetMaxSize));
	classDef.def("GetMaxWidth", &pragma::gui::types::WITransformable::GetMaxWidth);
	classDef.def("GetMaxHeight", &pragma::gui::types::WITransformable::GetMaxHeight);
	classDef.def("GetMaxSize", &pragma::gui::types::WITransformable::GetMaxSize, luabind::copy_policy<0> {});
	classDef.def("Close", &pragma::gui::types::WITransformable::Close);
	classDef.def("GetDragArea", &pragma::gui::types::WITransformable::GetDragArea);
	classDef.def("IsBeingDragged", &pragma::gui::types::WITransformable::IsBeingDragged);
	classDef.def("IsBeingResized", &pragma::gui::types::WITransformable::IsBeingResized);
	classDef.def("SetDragBounds", &pragma::gui::types::WITransformable::SetDragBounds);
	classDef.def("GetDragBounds", &pragma::gui::types::WITransformable::GetDragBounds);
	classDef.def("SetResizeRatioLocked", &pragma::gui::types::WITransformable::SetResizeRatioLocked);
	classDef.def("IsResizeRatioLocked", &pragma::gui::types::WITransformable::IsResizeRatioLocked);
	classDef.def("AddSnapTarget", &pragma::gui::types::WITransformable::AddSnapTarget);
	classDef.def("StartDrag", &pragma::gui::types::WITransformable::StartDrag);
	classDef.def("EndDrag", &pragma::gui::types::WITransformable::EndDrag);
}

////////////////////////////////////

luabind::tableT<pragma::gui::types::WITreeListElement> Lua::WITreeListElement::GetItems(lua::State *l, pragma::gui::types::WITreeListElement &hElement)
{
	auto t = luabind::newtable(l);
	auto &items = hElement.GetItems();
	int32_t idx = 1;
	for(auto i = decltype(items.size()) {0}; i < items.size(); ++i) {
		auto &hItem = items[i];
		if(hItem.IsValid() == false)
			continue;
		t[idx++] = pragma::gui::WGUILuaInterface::GetLuaObject(l, const_cast<pragma::gui::types::WIBase &>(*hItem.get()));
	}
	return t;
}

////////////////////////////////////

luabind::tableT<pragma::gui::types::WITableRow> Lua::WITable::GetSelectedRows(lua::State *l, pragma::gui::types::WITable &hTable)
{
	auto t = luabind::newtable(l);
	auto &rows = hTable.GetSelectedRows();
	uint32_t rowId = 1;
	for(auto &hRow : rows) {
		if(hRow.IsValid() == false)
			continue;
		t[rowId++] = pragma::gui::WGUILuaInterface::GetLuaObject(l, const_cast<pragma::gui::types::WIBase &>(*hRow.get()));
	}
	return t;
}
luabind::tableT<pragma::gui::types::WITableRow> Lua::WITable::GetRows(lua::State *l, pragma::gui::types::WITable &hTable)
{
	auto t = luabind::newtable(l);
	auto numRows = hTable.GetRowCount();
	uint32_t rowId = 1;
	for(auto i = decltype(numRows) {0}; i < numRows; ++i) {
		auto *pRow = hTable.GetRow(i);
		if(!pRow)
			continue;
		t[rowId++] = pragma::gui::WGUILuaInterface::GetLuaObject(l, *pRow);
	}
	return t;
}
