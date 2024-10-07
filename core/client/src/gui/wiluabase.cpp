/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/gui/wiluabase.h"
#include "luasystem.h"
#include <wgui/wihandle.h>
#include "pragma/lua/classes/c_ldef_wgui.h"
#include <prosper_prepared_command_buffer.hpp>

extern DLLCLIENT CGame *c_game;

extern ClientState *client;
WILuaBase::WILuaBase() {}

WILuaBase::~WILuaBase() {}

void WILuaBase::SetupLua(const luabind::object &o, std::string &className)
{
	m_class = className;
	SetLuaObject(o);
	SetUserData2(m_baseLuaObj);
}

void WILuaBase::Initialize()
{
	auto hThis = GetHandle();
	WIBase::Initialize();
	if(hThis.IsValid() == false)
		return;
	CallLuaMember("OnInitialize");
}

void WILuaBase::OnSkinApplied()
{
	if(umath::is_flag_set(m_stateFlags, StateFlags::SkinCallbacksEnabled))
		CallLuaMember("OnSkinApplied");
}

void WILuaBase::Think(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd)
{
	auto hThis = GetHandle();
	WIBase::Think(drawCmd);
	if(hThis.IsValid() == false)
		return;
	CallLuaMember("OnThink");
}

void WILuaBase::OnFirstThink()
{
	auto hThis = GetHandle();
	WIBase::OnFirstThink();
	if(hThis.IsValid() == false)
		return;
	CallLuaMember("OnFirstThink");
}

util::EventReply WILuaBase::MouseCallback(GLFW::MouseButton button, GLFW::KeyState state, GLFW::Modifier mods)
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
util::EventReply WILuaBase::KeyboardCallback(GLFW::Key key, int scanCode, GLFW::KeyState state, GLFW::Modifier mods)
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
util::EventReply WILuaBase::CharCallback(unsigned int c, GLFW::Modifier mods)
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
util::EventReply WILuaBase::ScrollCallback(Vector2 offset, bool offsetAsPixels)
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

void WILuaBase::SetSize(int x, int y)
{
	if(x == GetWidth() && y == GetHeight())
		return;
	WIBase::SetSize(x, y);
	CallLuaMember<void, int, int>("OnSizeChanged", x, y);
}
void WILuaBase::OnVisibilityChanged(bool bVisible)
{
	// if(bVisible == *GetVisibilityProperty())
	// 	return;
	WIBase::OnVisibilityChanged(bVisible);
	CallLuaMember<void, bool>("OnVisibilityChanged", bVisible);
}
void WILuaBase::DoUpdate()
{
	WIBase::DoUpdate();
	CallLuaMember<void>("OnUpdate");
}
void WILuaBase::SetColor(float r, float g, float b, float a)
{
	// TODO: Check against current values?
	WIBase::SetColor(r, g, b, a);
	CallLuaMember<void, float, float, float, float>("OnColorChanged", r, g, b, a);
}
void WILuaBase::SetAlpha(float alpha)
{
	if(alpha == GetAlpha())
		return;
	WIBase::SetAlpha(alpha);
	CallLuaMember<void, float>("OnAlphaChanged", alpha);
}
bool WILuaBase::DoPosInBounds(const Vector2i &pos) const
{
	bool res = false;
	auto r = const_cast<WILuaBase *>(this)->CallLuaMember<bool, Vector2i>("CheckPosInBounds", &res, pos);
	if(r == CallbackReturnType::HasReturnValue)
		return res;
	return WIBase::DoPosInBounds(pos);
}

void WILuaBase::SetRenderCommandBuffer(const std::shared_ptr<prosper::util::PreparedCommandBuffer> &cmd)
{
	if(!cmd) {
		m_renderData = nullptr;
		return;
	}
	m_renderData = std::unique_ptr<RenderData> {new RenderData {}};
	m_renderData->renderCommandBuffer = cmd;
}

void WILuaBase::Render(const DrawInfo &drawInfo, wgui::DrawState &drawState, const Mat4 &matDraw, const Vector2 &scale, uint32_t testStencilLevel, wgui::StencilPipeline stencilPipeline)
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
		using namespace ustring::string_switch;
		std::scoped_lock lock {m_renderData->drawArgMutex};
		drawArgs.SetArgumentValue(hash("x"), drawInfo.offset.x);
		drawArgs.SetArgumentValue(hash("y"), drawInfo.offset.y);
		drawArgs.SetArgumentValue(hash("w"), drawInfo.size.x);
		drawArgs.SetArgumentValue(hash("h"), drawInfo.size.y);
		drawArgs.SetArgumentValue(hash("stencilPipeline"), umath::to_integral(stencilPipeline));
		drawArgs.SetArgumentValue(hash("testStencilLevel"), testStencilLevel);
		drawArgs.SetArgumentValue(hash("msaa"), umath::is_flag_set(drawInfo.flags, DrawInfo::Flags::Msaa));
		drawArgs.SetArgumentValue(hash("matDraw"), matDraw);
		drawArgs.SetArgumentValue(hash("scale"), scale);
		drawArgs.SetArgumentValue(hash("viewportSize"), wgui::ElementData::ToViewportSize(drawInfo.size));
		m_renderData->userData.Set(hash("guiDrawState"), drawState);
		m_renderData->renderCommandBuffer->RecordCommands(*drawInfo.commandBuffer, drawArgs, m_renderData->userData);
	}
}
void WILuaBase::OnCursorEntered()
{
	WIBase::OnCursorEntered();
	CallLuaMember("OnCursorEntered");
}
void WILuaBase::OnCursorExited()
{
	WIBase::OnCursorExited();
	CallLuaMember("OnCursorExited");
}
void WILuaBase::OnFileDragEntered()
{
	WIBase::OnFileDragEntered();
	CallLuaMember("OnFileDragEntered");
}
void WILuaBase::OnFileDragExited()
{
	WIBase::OnFileDragExited();
	CallLuaMember("OnFileDragExited");
}
util::EventReply WILuaBase::OnFilesDropped(const std::vector<std::string> &files)
{
	WIBase::OnFilesDropped(files);
	uint32_t reply = umath::to_integral(util::EventReply::Unhandled);
	CallLuaMember<uint32_t, std::vector<std::string>>("OnFilesDropped", &reply, files);
	return static_cast<util::EventReply>(reply);
}
void WILuaBase::OnFocusGained()
{
	WIBase::OnFocusGained();
	CallLuaMember("OnFocusGained");
}
void WILuaBase::OnFocusKilled()
{
	WIBase::OnFocusKilled();
	CallLuaMember("OnFocusKilled");
}
void WILuaBase::OnRemove()
{
	WIBase::OnRemove();
	CallLuaMember("OnRemove");
}

///////////////////////////////////////////

void WILuaBase::Lua_OnInitialize() {}
void WILuaBase::default_OnInitialize(lua_State *, WILuaBase &) {}

void WILuaBase::Lua_OnThink() {}
void WILuaBase::default_OnThink(lua_State *, WILuaBase &) {}

void WILuaBase::Lua_OnFirstThink() {}
void WILuaBase::default_OnFirstThink(lua_State *, WILuaBase &) {}

void WILuaBase::Lua_MouseCallback(int, int, int) {}
void WILuaBase::default_MouseCallback(lua_State *, WILuaBase &, int, int, int) {}

void WILuaBase::Lua_KeyboardCallback(int, int, int, int) {}
void WILuaBase::default_KeyboardCallback(lua_State *, WILuaBase &, int, int, int, int) {}

void WILuaBase::Lua_CharCallback(unsigned int, uint32_t) {}
void WILuaBase::default_CharCallback(lua_State *, WILuaBase &, unsigned int, uint32_t) {}

void WILuaBase::Lua_ScrollCallback(double, double) {}
void WILuaBase::default_ScrollCallback(lua_State *, WILuaBase &, double, double) {}

void WILuaBase::Lua_OnUpdate() {}
void WILuaBase::default_OnUpdate(lua_State *l, WILuaBase &hElement) {}

void WILuaBase::Lua_OnSetSize(int, int) {}
void WILuaBase::default_OnSetSize(lua_State *, WILuaBase &, int, int) {}

void WILuaBase::Lua_OnSetVisible(bool) {}
void WILuaBase::default_OnSetVisible(lua_State *, WILuaBase &, bool) {}

void WILuaBase::Lua_OnSetColor(float, float, float, float) {}
void WILuaBase::default_OnSetColor(lua_State *, WILuaBase &, float, float, float, float) {}

void WILuaBase::Lua_OnSetAlpha(float) {}
void WILuaBase::default_OnSetAlpha(lua_State *, WILuaBase &, float) {}

void WILuaBase::Lua_Render(const ::WIBase::DrawInfo &drawInfo, const Mat4 &matDraw, const Vector2 &scale) {}
void WILuaBase::default_Render(lua_State *, WILuaBase &, const ::WIBase::DrawInfo &drawInfo, const Mat4 &matDraw, const Vector2 &scale) {}

void WILuaBase::Lua_OnCursorEntered() {}
void WILuaBase::default_OnCursorEntered(lua_State *, WILuaBase &) {}

void WILuaBase::Lua_OnCursorExited() {}
void WILuaBase::default_OnCursorExited(lua_State *, WILuaBase &) {}

void WILuaBase::Lua_OnFileDragEntered() {}
void WILuaBase::default_OnFileDragEntered(lua_State *l, WILuaBase &hElement) {}

void WILuaBase::Lua_OnFileDragExited() {}
void WILuaBase::default_OnFileDragExited(lua_State *l, WILuaBase &hElement) {}

void WILuaBase::Lua_OnFilesDropped(const std::vector<std::string> &files) {}
void WILuaBase::default_OnFilesDropped(lua_State *l, WILuaBase &hElement, const std::vector<std::string> &files) {}

void WILuaBase::Lua_OnFocusGained() {}
void WILuaBase::default_OnFocusGained(lua_State *, WILuaBase &) {}

void WILuaBase::Lua_OnFocusKilled() {}
void WILuaBase::default_OnFocusKilled(lua_State *, WILuaBase &) {}

void WILuaBase::Lua_OnRemove() {}
void WILuaBase::default_OnRemove(lua_State *, WILuaBase &) {}

bool WILuaBase::Lua_CheckPosInBounds(const Vector2i &pos) { return WIBase::DoPosInBounds(pos); }
bool WILuaBase::default_CheckPosInBounds(lua_State *l, WILuaBase &hElement, const Vector2i &pos) { return hElement.WIBase::DoPosInBounds(pos); }
