/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __WILUABASE_H__
#define __WILUABASE_H__
#include "pragma/clientdefinitions.h"
#include <wgui/wibase.h>
#include <pragma/lua/luaapi.h>
#include <prosper_prepared_command_buffer.hpp>
#include "pragma/gui/wgui_luainterface.h"
#include "pragma/lua/luaobjectbase.h"
#include <pragma/lua/lua_call.hpp>
#include <pragma/lua/handle_holder.hpp>

#define LUAGUI_CALL_MEMBER(name, numargs, numret, args, ret)                                                                                                                                                                                                                                     \
	{                                                                                                                                                                                                                                                                                            \
		luabind::object *obj = WGUILuaInterface::GetLuaObject(m_stateLua, this);                                                                                                                                                                                                                 \
		obj->push(m_stateLua);                                                                                                                                                                                                                                                                   \
		Lua::PushString(m_stateLua, name);                                                                                                                                                                                                                                                       \
		Lua::GetTableValue(m_stateLua, -2);                                                                                                                                                                                                                                                      \
		if(Lua::IsSet(m_stateLua, -1) && Lua::IsFunction(m_stateLua, -1)) {                                                                                                                                                                                                                      \
			int cargs = numargs + 1;                                                                                                                                                                                                                                                             \
			obj->push(m_stateLua);                                                                                                                                                                                                                                                               \
			args;                                                                                                                                                                                                                                                                                \
			int s = Lua::CallFunction(m_stateLua, cargs, numret, 0);                                                                                                                                                                                                                             \
			if(s == 0) {                                                                                                                                                                                                                                                                         \
				ret;                                                                                                                                                                                                                                                                             \
			}                                                                                                                                                                                                                                                                                    \
			Error(s);                                                                                                                                                                                                                                                                            \
		}                                                                                                                                                                                                                                                                                        \
		else                                                                                                                                                                                                                                                                                     \
			Lua::Pop(m_stateLua, 1);                                                                                                                                                                                                                                                             \
		Lua::Pop(m_stateLua, 1);                                                                                                                                                                                                                                                                 \
	}

struct lua_State;
struct WILuaWrapper;
class DLLCLIENT WILuaBase : public WIBase, public LuaObjectBase {
  public:
	WILuaBase();
	void SetupLua(const luabind::object &o, std::string &className);
	virtual ~WILuaBase() override;
	virtual void Initialize() override;
	virtual void Think(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd) override;
	virtual void OnFirstThink() override;
	virtual util::EventReply MouseCallback(GLFW::MouseButton button, GLFW::KeyState state, GLFW::Modifier mods) override;
	virtual util::EventReply KeyboardCallback(GLFW::Key key, int scanCode, GLFW::KeyState state, GLFW::Modifier mods) override;
	virtual util::EventReply CharCallback(unsigned int c, GLFW::Modifier mods = GLFW::Modifier::None) override;
	virtual util::EventReply ScrollCallback(Vector2 offset, bool offsetAsPixels = false) override;
	virtual void SetSize(int x, int y) override;
	virtual void OnVisibilityChanged(bool bVisible) override;
	virtual void SetColor(float r, float g, float b, float a = 1.f) override;
	virtual void SetAlpha(float alpha) override;
	virtual void Render(const DrawInfo &drawInfo, wgui::DrawState &drawState, const Mat4 &matDraw, const Vector2 &scale = {1.f, 1.f}, uint32_t testStencilLevel = 0u, wgui::StencilPipeline stencilPipeline = wgui::StencilPipeline::Test) override;
	virtual void OnCursorEntered() override;
	virtual void OnCursorExited() override;
	virtual void OnFileDragEntered() override;
	virtual void OnFileDragExited() override;
	virtual util::EventReply OnFilesDropped(const std::vector<std::string> &files) override;
	virtual void OnFocusGained() override;
	virtual void OnFocusKilled() override;
	virtual void OnRemove() override;

	void SetRenderCommandBuffer(const std::shared_ptr<prosper::util::PreparedCommandBuffer> &cmd);

	// Lua
	void Lua_OnInitialize();
	static void default_OnInitialize(lua_State *l, WILuaBase &hElement);

	void Lua_OnThink();
	static void default_OnThink(lua_State *l, WILuaBase &hElement);

	void Lua_OnFirstThink();
	static void default_OnFirstThink(lua_State *l, WILuaBase &hElement);

	void Lua_MouseCallback(int button, int action, int mods);
	static void default_MouseCallback(lua_State *l, WILuaBase &hElement, int button, int action, int mods);

	void Lua_KeyboardCallback(int key, int scancode, int action, int mods);
	static void default_KeyboardCallback(lua_State *l, WILuaBase &hElement, int key, int scancode, int action, int mods);

	void Lua_CharCallback(unsigned int c, uint32_t mods);
	static void default_CharCallback(lua_State *l, WILuaBase &hElement, unsigned int c, uint32_t mods);

	void Lua_ScrollCallback(double xoffset, double yoffset);
	static void default_ScrollCallback(lua_State *l, WILuaBase &hElement, double xoffset, double yoffset);

	void Lua_OnUpdate();
	static void default_OnUpdate(lua_State *l, WILuaBase &hElement);

	void Lua_OnSetSize(int x, int y);
	static void default_OnSetSize(lua_State *l, WILuaBase &hElement, int x, int y);

	void Lua_OnSetVisible(bool b);
	static void default_OnSetVisible(lua_State *l, WILuaBase &hElement, bool b);

	void Lua_OnSetColor(float r, float g, float b, float a = 1.f);
	static void default_OnSetColor(lua_State *l, WILuaBase &hElement, float r, float g, float b, float a = 1.f);

	void Lua_OnSetAlpha(float alpha);
	static void default_OnSetAlpha(lua_State *l, WILuaBase &hElement, float alpha);

	bool Lua_CheckPosInBounds(const Vector2i &pos);
	static bool default_CheckPosInBounds(lua_State *l, WILuaBase &hElement, const Vector2i &pos);

	void Lua_Render(const ::WIBase::DrawInfo &drawInfo, const Mat4 &matDraw, const Vector2 &scale);
	static void default_Render(lua_State *l, WILuaBase &hElement, const ::WIBase::DrawInfo &drawInfo, const Mat4 &matDraw, const Vector2 &scale);

	void Lua_OnCursorEntered();
	static void default_OnCursorEntered(lua_State *l, WILuaBase &hElement);

	void Lua_OnCursorExited();
	static void default_OnCursorExited(lua_State *l, WILuaBase &hElement);

	void Lua_OnFileDragEntered();
	static void default_OnFileDragEntered(lua_State *l, WILuaBase &hElement);

	void Lua_OnFileDragExited();
	static void default_OnFileDragExited(lua_State *l, WILuaBase &hElement);

	void Lua_OnFilesDropped(const std::vector<std::string> &files);
	static void default_OnFilesDropped(lua_State *l, WILuaBase &hElement, const std::vector<std::string> &files);

	void Lua_OnFocusGained();
	static void default_OnFocusGained(lua_State *l, WILuaBase &hElement);

	void Lua_OnFocusKilled();
	static void default_OnFocusKilled(lua_State *l, WILuaBase &hElement);

	void Lua_OnRemove();
	static void default_OnRemove(lua_State *l, WILuaBase &hElement);
  protected:
	virtual void DoUpdate() override;
	virtual bool DoPosInBounds(const Vector2i &pos) const override;
	virtual void OnSkinApplied() override;

	struct RenderData {
		std::shared_ptr<prosper::util::PreparedCommandBuffer> renderCommandBuffer = nullptr;
		prosper::util::PreparedCommandArgumentMap drawArgs;
		prosper::util::PreparedCommandBufferUserData userData;
		std::mutex drawArgMutex;
	};
	std::unique_ptr<RenderData> m_renderData = nullptr;
};

namespace pragma::lua {
	using WILuaBaseHolder = HandleHolder<WILuaBase>;
};

#endif
