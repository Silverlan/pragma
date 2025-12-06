// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

module pragma.client;

import :gui.lua_base;

import :client_state;

#undef DrawState

pragma::gui::WILuaBase::WILuaBase() {}

pragma::gui::WILuaBase::~WILuaBase() {}

void pragma::gui::WILuaBase::SetupLua(const luabind::object &o, std::string &className)
{
	m_class = className;
	SetLuaObject(o);
	SetUserData2(m_baseLuaObj);
}

void pragma::gui::WILuaBase::Initialize()
{
	auto hThis = GetHandle();
	WIBase::Initialize();
	if(hThis.IsValid() == false)
		return;
	CallLuaMember("OnInitialize");
}

void pragma::gui::WILuaBase::OnSkinApplied()
{
	if(umath::is_flag_set(m_stateFlags, StateFlags::SkinCallbacksEnabled))
		CallLuaMember("OnSkinApplied");
}

void pragma::gui::WILuaBase::Think(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd)
{
	auto hThis = GetHandle();
	WIBase::Think(drawCmd);
	if(hThis.IsValid() == false)
		return;
	CallLuaMember("OnThink");
}

void pragma::gui::WILuaBase::OnFirstThink()
{
	auto hThis = GetHandle();
	WIBase::OnFirstThink();
	if(hThis.IsValid() == false)
		return;
	CallLuaMember("OnFirstThink");
}

util::EventReply pragma::gui::WILuaBase::MouseCallback(pragma::platform::MouseButton button, pragma::platform::KeyState state, pragma::platform::Modifier mods)
{
	auto hThis = GetHandle();
	if(WIBase::MouseCallback(button, state, mods) == util::EventReply::Handled)
		return util::EventReply::Handled;
	if(!hThis.IsValid())
		return util::EventReply::Unhandled;
	uint32_t reply = umath::to_integral(util::EventReply::Unhandled);
	CallLuaMember<uint32_t, int, int, int>("MouseCallback", &reply, static_cast<int>(button), static_cast<int>(state), static_cast<int>(mods));
	return static_cast<util::EventReply>(reply);
}
util::EventReply pragma::gui::WILuaBase::KeyboardCallback(pragma::platform::Key key, int scanCode, pragma::platform::KeyState state, pragma::platform::Modifier mods)
{
	auto hThis = GetHandle();
	if(WIBase::KeyboardCallback(key, scanCode, state, mods) == util::EventReply::Handled)
		return util::EventReply::Handled;
	if(!hThis.IsValid())
		return util::EventReply::Unhandled;
	uint32_t reply = umath::to_integral(util::EventReply::Unhandled);
	CallLuaMember<uint32_t, int, int, int, int>("KeyboardCallback", &reply, static_cast<int>(key), scanCode, static_cast<int>(state), static_cast<int>(mods));
	return static_cast<util::EventReply>(reply);
}
util::EventReply pragma::gui::WILuaBase::CharCallback(unsigned int c, pragma::platform::Modifier mods)
{
	auto hThis = GetHandle();
	if(WIBase::CharCallback(c) == util::EventReply::Handled)
		return util::EventReply::Handled;
	if(!hThis.IsValid())
		return util::EventReply::Unhandled;
	uint32_t reply = umath::to_integral(util::EventReply::Unhandled);
	CallLuaMember<uint32_t, unsigned int, uint32_t>("CharCallback", &reply, c, umath::to_integral(mods));
	return static_cast<util::EventReply>(reply);
}
util::EventReply pragma::gui::WILuaBase::ScrollCallback(Vector2 offset, bool offsetAsPixels)
{
	auto hThis = GetHandle();
	if(WIBase::ScrollCallback(offset, offsetAsPixels) == util::EventReply::Handled)
		return util::EventReply::Handled;
	if(!hThis.IsValid())
		return util::EventReply::Unhandled;
	uint32_t reply = umath::to_integral(util::EventReply::Unhandled);
	CallLuaMember<uint32_t, double, double, bool>("ScrollCallback", &reply, offset.x, offset.y, offsetAsPixels);
	return static_cast<util::EventReply>(reply);
}

void pragma::gui::WILuaBase::SetSize(int x, int y)
{
	if(x == GetWidth() && y == GetHeight())
		return;
	WIBase::SetSize(x, y);
	// WIBase::SetSize may have called additional callbacks, which may have changed the size
	// of this element before we got to call the "OnSizeChanged" callback below. In this case
	// "OnSizeChanged" has already been called by one of the other callbacks, so we should skip it.
	auto newW = GetWidth();
	auto newH = GetHeight();
	if(newW == x && newH == y)
		CallLuaMember<void, int, int>("OnSizeChanged", x, y);
}
void pragma::gui::WILuaBase::OnVisibilityChanged(bool bVisible)
{
	// if(bVisible == *GetVisibilityProperty())
	// 	return;
	WIBase::OnVisibilityChanged(bVisible);
	if(*GetVisibilityProperty() == bVisible) // See explanation in pragma::gui::WILuaBase::SetSize
		CallLuaMember<void, bool>("OnVisibilityChanged", bVisible);
}
void pragma::gui::WILuaBase::DoUpdate()
{
	WIBase::DoUpdate();
	CallLuaMember<void>("OnUpdate");
}
void pragma::gui::WILuaBase::SetColor(float r, float g, float b, float a)
{
	auto newCol = Vector4 {r, g, b, a};
	auto vCol = GetColor().ToVector4();
	if(uvec::cmp(vCol, newCol))
		return;
	WIBase::SetColor(r, g, b, a);
	vCol = GetColor().ToVector4();
	if(uvec::cmp(vCol, newCol)) // See explanation in pragma::gui::WILuaBase::SetSize
		CallLuaMember<void, float, float, float, float>("OnColorChanged", r, g, b, a);
}
void pragma::gui::WILuaBase::SetAlpha(float alpha)
{
	if(umath::equals(alpha, GetAlpha()))
		return;
	WIBase::SetAlpha(alpha);
	if(umath::equals(alpha, GetAlpha())) // See explanation in pragma::gui::WILuaBase::SetSize
		CallLuaMember<void, float>("OnAlphaChanged", alpha);
}
bool pragma::gui::WILuaBase::DoPosInBounds(const Vector2i &pos) const
{
	bool res = false;
	auto r = const_cast<WILuaBase *>(this)->CallLuaMember<bool, Vector2i>("CheckPosInBounds", &res, pos);
	if(r == CallbackReturnType::HasReturnValue)
		return res;
	return WIBase::DoPosInBounds(pos);
}

void pragma::gui::WILuaBase::SetRenderCommandBuffer(const std::shared_ptr<prosper::util::PreparedCommandBuffer> &cmd)
{
	if(!cmd) {
		m_renderData = nullptr;
		return;
	}
	m_renderData = std::unique_ptr<RenderData> {new RenderData {}};
	m_renderData->renderCommandBuffer = cmd;
}

void pragma::gui::WILuaBase::Render(const wgui::DrawInfo &drawInfo, wgui::DrawState &drawState, const Mat4 &matDraw, const Vector2 &scale, uint32_t testStencilLevel, wgui::StencilPipeline stencilPipeline)
{
	WIBase::Render(drawInfo, drawState, matDraw, scale, testStencilLevel, stencilPipeline);
	if(!m_renderData)
		return;
	auto &drawArgs = m_renderData->drawArgs;
	if(!m_renderData->renderCommandBuffer->enableDrawArgs)
		m_renderData->renderCommandBuffer->RecordCommands(*drawInfo.commandBuffer, drawArgs, m_renderData->userData);
	else {
		// Some arguments have to be updated at render time,
		// however since the render-code is multi-threaded and we only have
		// one container for the draw arguments, we have to secure it
		// with a mutex. Since the same UI element is usually not rendered multiple times in parallel,
		// it's unlikely this will cause a significant negative performance impact.
		std::scoped_lock lock {m_renderData->drawArgMutex};
		drawArgs.SetArgumentValue(ustring::string_switch::hash("x"), drawInfo.offset.x);
		drawArgs.SetArgumentValue(ustring::string_switch::hash("y"), drawInfo.offset.y);
		drawArgs.SetArgumentValue(ustring::string_switch::hash("w"), drawInfo.size.x);
		drawArgs.SetArgumentValue(ustring::string_switch::hash("h"), drawInfo.size.y);
		drawArgs.SetArgumentValue(ustring::string_switch::hash("stencilPipeline"), umath::to_integral(stencilPipeline));
		drawArgs.SetArgumentValue(ustring::string_switch::hash("testStencilLevel"), testStencilLevel);
		drawArgs.SetArgumentValue(ustring::string_switch::hash("msaa"), umath::is_flag_set(drawInfo.flags, wgui::DrawInfo::Flags::Msaa));
		drawArgs.SetArgumentValue(ustring::string_switch::hash("matDraw"), matDraw);
		drawArgs.SetArgumentValue(ustring::string_switch::hash("scale"), scale);
		drawArgs.SetArgumentValue(ustring::string_switch::hash("viewportSize"), wgui::ElementData::ToViewportSize(drawInfo.size));
		m_renderData->userData.Set(ustring::string_switch::hash("guiDrawState"), drawState);
		m_renderData->renderCommandBuffer->RecordCommands(*drawInfo.commandBuffer, drawArgs, m_renderData->userData);
	}
}
void pragma::gui::WILuaBase::OnCursorEntered()
{
	WIBase::OnCursorEntered();
	CallLuaMember("OnCursorEntered");
}
void pragma::gui::WILuaBase::OnCursorExited()
{
	WIBase::OnCursorExited();
	CallLuaMember("OnCursorExited");
}
void pragma::gui::WILuaBase::OnFileDragEntered()
{
	WIBase::OnFileDragEntered();
	CallLuaMember("OnFileDragEntered");
}
void pragma::gui::WILuaBase::OnFileDragExited()
{
	WIBase::OnFileDragExited();
	CallLuaMember("OnFileDragExited");
}
util::EventReply pragma::gui::WILuaBase::OnFilesDropped(const std::vector<std::string> &files)
{
	WIBase::OnFilesDropped(files);
	uint32_t reply = umath::to_integral(util::EventReply::Unhandled);
	CallLuaMember<uint32_t, std::vector<std::string>>("OnFilesDropped", &reply, files);
	return static_cast<util::EventReply>(reply);
}
void pragma::gui::WILuaBase::OnFocusGained()
{
	WIBase::OnFocusGained();
	CallLuaMember("OnFocusGained");
}
void pragma::gui::WILuaBase::OnFocusKilled()
{
	WIBase::OnFocusKilled();
	CallLuaMember("OnFocusKilled");
}
void pragma::gui::WILuaBase::OnRemove()
{
	WIBase::OnRemove();
	CallLuaMember("OnRemove");
}

///////////////////////////////////////////

void pragma::gui::WILuaBase::Lua_OnInitialize() {}
void pragma::gui::WILuaBase::default_OnInitialize(lua::State *, WILuaBase &) {}

void pragma::gui::WILuaBase::Lua_OnThink() {}
void pragma::gui::WILuaBase::default_OnThink(lua::State *, WILuaBase &) {}

void pragma::gui::WILuaBase::Lua_OnFirstThink() {}
void pragma::gui::WILuaBase::default_OnFirstThink(lua::State *, WILuaBase &) {}

void pragma::gui::WILuaBase::Lua_MouseCallback(int, int, int) {}
void pragma::gui::WILuaBase::default_MouseCallback(lua::State *, WILuaBase &, int, int, int) {}

void pragma::gui::WILuaBase::Lua_KeyboardCallback(int, int, int, int) {}
void pragma::gui::WILuaBase::default_KeyboardCallback(lua::State *, WILuaBase &, int, int, int, int) {}

void pragma::gui::WILuaBase::Lua_CharCallback(unsigned int, uint32_t) {}
void pragma::gui::WILuaBase::default_CharCallback(lua::State *, WILuaBase &, unsigned int, uint32_t) {}

void pragma::gui::WILuaBase::Lua_ScrollCallback(double, double) {}
void pragma::gui::WILuaBase::default_ScrollCallback(lua::State *, WILuaBase &, double, double) {}

void pragma::gui::WILuaBase::Lua_OnUpdate() {}
void pragma::gui::WILuaBase::default_OnUpdate(lua::State *l, WILuaBase &hElement) {}

void pragma::gui::WILuaBase::Lua_OnSetSize(int, int) {}
void pragma::gui::WILuaBase::default_OnSetSize(lua::State *, WILuaBase &, int, int) {}

void pragma::gui::WILuaBase::Lua_OnSetVisible(bool) {}
void pragma::gui::WILuaBase::default_OnSetVisible(lua::State *, WILuaBase &, bool) {}

void pragma::gui::WILuaBase::Lua_OnSetColor(float, float, float, float) {}
void pragma::gui::WILuaBase::default_OnSetColor(lua::State *, WILuaBase &, float, float, float, float) {}

void pragma::gui::WILuaBase::Lua_OnSetAlpha(float) {}
void pragma::gui::WILuaBase::default_OnSetAlpha(lua::State *, WILuaBase &, float) {}

void pragma::gui::WILuaBase::Lua_Render(const ::wgui::DrawInfo &drawInfo, const Mat4 &matDraw, const Vector2 &scale) {}
void pragma::gui::WILuaBase::default_Render(lua::State *, WILuaBase &, const ::wgui::DrawInfo &drawInfo, const Mat4 &matDraw, const Vector2 &scale) {}

void pragma::gui::WILuaBase::Lua_OnCursorEntered() {}
void pragma::gui::WILuaBase::default_OnCursorEntered(lua::State *, WILuaBase &) {}

void pragma::gui::WILuaBase::Lua_OnCursorExited() {}
void pragma::gui::WILuaBase::default_OnCursorExited(lua::State *, WILuaBase &) {}

void pragma::gui::WILuaBase::Lua_OnFileDragEntered() {}
void pragma::gui::WILuaBase::default_OnFileDragEntered(lua::State *l, WILuaBase &hElement) {}

void pragma::gui::WILuaBase::Lua_OnFileDragExited() {}
void pragma::gui::WILuaBase::default_OnFileDragExited(lua::State *l, WILuaBase &hElement) {}

void pragma::gui::WILuaBase::Lua_OnFilesDropped(const std::vector<std::string> &files) {}
void pragma::gui::WILuaBase::default_OnFilesDropped(lua::State *l, WILuaBase &hElement, const std::vector<std::string> &files) {}

void pragma::gui::WILuaBase::Lua_OnFocusGained() {}
void pragma::gui::WILuaBase::default_OnFocusGained(lua::State *, WILuaBase &) {}

void pragma::gui::WILuaBase::Lua_OnFocusKilled() {}
void pragma::gui::WILuaBase::default_OnFocusKilled(lua::State *, WILuaBase &) {}

void pragma::gui::WILuaBase::Lua_OnRemove() {}
void pragma::gui::WILuaBase::default_OnRemove(lua::State *, WILuaBase &) {}

bool pragma::gui::WILuaBase::Lua_CheckPosInBounds(const Vector2i &pos) { return WIBase::DoPosInBounds(pos); }
bool pragma::gui::WILuaBase::default_CheckPosInBounds(lua::State *l, WILuaBase &hElement, const Vector2i &pos) { return hElement.WIBase::DoPosInBounds(pos); }
