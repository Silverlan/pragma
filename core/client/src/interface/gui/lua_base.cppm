// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:gui.lua_base;

import :gui.lua_interface;
export import pragma.gui;
export import pragma.shared;

#undef DrawState

#define LUAGUI_CALL_MEMBER(name, numargs, numret, args, ret)                                                                                                                                                                                                                                     \
	{                                                                                                                                                                                                                                                                                            \
		luabind::object *obj = pragma::gui::WGUILuaInterface::GetLuaObject(m_stateLua, this);                                                                                                                                                                                                    \
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

export namespace pragma::gui::types {
	class DLLCLIENT WILuaBase : public WIBase, public LuaObjectBase {
	  public:
		WILuaBase();
		void SetupLua(const luabind::object &o, std::string &className);
		virtual ~WILuaBase() override;
		virtual void Initialize() override;
		virtual void Think(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd) override;
		virtual void OnFirstThink() override;
		virtual util::EventReply MouseCallback(platform::MouseButton button, platform::KeyState state, platform::Modifier mods) override;
		virtual util::EventReply KeyboardCallback(platform::Key key, int scanCode, platform::KeyState state, platform::Modifier mods) override;
		virtual util::EventReply CharCallback(unsigned int c, platform::Modifier mods = platform::Modifier::None) override;
		virtual util::EventReply ScrollCallback(Vector2 offset, bool offsetAsPixels = false) override;
		virtual void SetSize(int x, int y) override;
		virtual void OnVisibilityChanged(bool bVisible) override;
		virtual void SetColor(float r, float g, float b, float a = 1.f) override;
		virtual void SetAlpha(float alpha) override;
		virtual void Render(const DrawInfo &drawInfo, DrawState &drawState, const Mat4 &matDraw, const Vector2 &scale = {1.f, 1.f}, uint32_t testStencilLevel = 0u, StencilPipeline stencilPipeline = StencilPipeline::Test) override;
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
		static void default_OnInitialize(lua::State *l, WILuaBase &hElement);

		void Lua_OnThink();
		static void default_OnThink(lua::State *l, WILuaBase &hElement);

		void Lua_OnFirstThink();
		static void default_OnFirstThink(lua::State *l, WILuaBase &hElement);

		void Lua_MouseCallback(int button, int action, int mods);
		static void default_MouseCallback(lua::State *l, WILuaBase &hElement, int button, int action, int mods);

		void Lua_KeyboardCallback(int key, int scancode, int action, int mods);
		static void default_KeyboardCallback(lua::State *l, WILuaBase &hElement, int key, int scancode, int action, int mods);

		void Lua_CharCallback(unsigned int c, uint32_t mods);
		static void default_CharCallback(lua::State *l, WILuaBase &hElement, unsigned int c, uint32_t mods);

		void Lua_ScrollCallback(double xoffset, double yoffset);
		static void default_ScrollCallback(lua::State *l, WILuaBase &hElement, double xoffset, double yoffset);

		void Lua_OnUpdate();
		static void default_OnUpdate(lua::State *l, WILuaBase &hElement);

		void Lua_OnSetSize(int x, int y);
		static void default_OnSetSize(lua::State *l, WILuaBase &hElement, int x, int y);

		void Lua_OnSetVisible(bool b);
		static void default_OnSetVisible(lua::State *l, WILuaBase &hElement, bool b);

		void Lua_OnSetColor(float r, float g, float b, float a = 1.f);
		static void default_OnSetColor(lua::State *l, WILuaBase &hElement, float r, float g, float b, float a = 1.f);

		void Lua_OnSetAlpha(float alpha);
		static void default_OnSetAlpha(lua::State *l, WILuaBase &hElement, float alpha);

		bool Lua_CheckPosInBounds(const Vector2i &pos);
		static bool default_CheckPosInBounds(lua::State *l, WILuaBase &hElement, const Vector2i &pos);

		void Lua_Render(const DrawInfo &drawInfo, const Mat4 &matDraw, const Vector2 &scale);
		static void default_Render(lua::State *l, WILuaBase &hElement, const DrawInfo &drawInfo, const Mat4 &matDraw, const Vector2 &scale);

		void Lua_OnCursorEntered();
		static void default_OnCursorEntered(lua::State *l, WILuaBase &hElement);

		void Lua_OnCursorExited();
		static void default_OnCursorExited(lua::State *l, WILuaBase &hElement);

		void Lua_OnFileDragEntered();
		static void default_OnFileDragEntered(lua::State *l, WILuaBase &hElement);

		void Lua_OnFileDragExited();
		static void default_OnFileDragExited(lua::State *l, WILuaBase &hElement);

		void Lua_OnFilesDropped(const std::vector<std::string> &files);
		static void default_OnFilesDropped(lua::State *l, WILuaBase &hElement, const std::vector<std::string> &files);

		void Lua_OnFocusGained();
		static void default_OnFocusGained(lua::State *l, WILuaBase &hElement);

		void Lua_OnFocusKilled();
		static void default_OnFocusKilled(lua::State *l, WILuaBase &hElement);

		void Lua_OnRemove();
		static void default_OnRemove(lua::State *l, WILuaBase &hElement);
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
};

export namespace pragma::LuaCore {
	using WILuaBaseHolder = HandleHolder<gui::types::WILuaBase>;
};
