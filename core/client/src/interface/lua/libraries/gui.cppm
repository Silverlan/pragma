// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:scripting.lua.libraries.gui;
export import :game;

export namespace Lua {
	namespace gui {
		struct DrawToTextureInfo {
			bool enableMsaa = false;
			bool useStencil = false;
			std::optional<uint32_t> width {};
			std::optional<uint32_t> height {};
			std::optional<::Color> clearColor {};
			std::shared_ptr<prosper::IImage> resolvedImage = nullptr;
			std::shared_ptr<prosper::ICommandBuffer> commandBuffer = nullptr;
		};

		DLLCLIENT pragma::gui::types::WIBase *create(pragma::CGame *game, const std::string &name, pragma::gui::types::WIBase &parent, int32_t x, int32_t y, uint32_t w, uint32_t h, float left, float top, float right, float bottom);
		DLLCLIENT pragma::gui::types::WIBase *create(pragma::CGame *game, const std::string &name, pragma::gui::types::WIBase &parent, int32_t x, int32_t y, uint32_t w, uint32_t h);
		DLLCLIENT pragma::gui::types::WIBase *create(pragma::CGame *game, const std::string &name, pragma::gui::types::WIBase &parent, int32_t x, int32_t y);
		DLLCLIENT pragma::gui::types::WIBase *create(pragma::CGame *game, const std::string &name, pragma::gui::types::WIBase *parent);
		DLLCLIENT pragma::gui::types::WIBase *create(pragma::CGame *game, const std::string &name);

		DLLCLIENT pragma::gui::types::WIBase *create_button(lua::State *l, const std::string &text);
		DLLCLIENT pragma::gui::types::WIBase *create_button(lua::State *l, const std::string &text, pragma::gui::types::WIBase &parent);
		DLLCLIENT pragma::gui::types::WIBase *create_button(lua::State *l, const std::string &text, pragma::gui::types::WIBase &parent, int32_t x, int32_t y);

		DLLCLIENT opt<mult<type<pragma::gui::types::WIBase>, type<pragma::gui::types::WIBase>, type<pragma::gui::types::WIBase>>> create_checkbox(lua::State *l, const std::string &label, pragma::gui::types::WIBase &parent);
		DLLCLIENT opt<mult<type<pragma::gui::types::WIBase>, type<pragma::gui::types::WIBase>, type<pragma::gui::types::WIBase>>> create_checkbox(lua::State *l, const std::string &label);

		DLLCLIENT pragma::gui::types::WIBase *create_label(lua::State *l, const std::string &str, pragma::gui::types::WIBase &parent, int32_t x, int32_t y);
		DLLCLIENT pragma::gui::types::WIBase *create_label(lua::State *l, const std::string &str, pragma::gui::types::WIBase &parent);
		DLLCLIENT pragma::gui::types::WIBase *create_label(lua::State *l, const std::string &str);

		DLLCLIENT void register_element(const std::string &className, const classObject &classData);
		DLLCLIENT pragma::gui::types::WIBase *get_base_element(const prosper::Window &window);
		DLLCLIENT pragma::gui::types::WIBase *get_base_element();

		DLLCLIENT pragma::gui::types::WIBase *get_element_at_position(lua::State *l);
		DLLCLIENT pragma::gui::types::WIBase *get_element_at_position(lua::State *l, prosper::Window *window);
		DLLCLIENT pragma::gui::types::WIBase *get_element_at_position(lua::State *l, prosper::Window *window, pragma::gui::types::WIBase *baseElement);
		DLLCLIENT pragma::gui::types::WIBase *get_element_at_position(lua::State *l, prosper::Window *window, pragma::gui::types::WIBase *baseElement, int32_t x, int32_t y);
		DLLCLIENT pragma::gui::types::WIBase *get_element_at_position(lua::State *l, prosper::Window *window, pragma::gui::types::WIBase *baseElement, int32_t x, int32_t y, const func<bool, pragma::gui::types::WIBase> &condition);

		DLLCLIENT pragma::gui::types::WIBase *get_element_under_cursor(lua::State *l, pragma::gui::types::WIBase &elRoot, const func<bool, pragma::gui::types::WIBase> &condition);
		DLLCLIENT pragma::gui::types::WIBase *get_element_under_cursor(lua::State *l, const prosper::Window *window, const func<bool, pragma::gui::types::WIBase> &condition);
		DLLCLIENT pragma::gui::types::WIBase *get_element_under_cursor(lua::State *l, const func<bool, pragma::gui::types::WIBase> &condition);
		DLLCLIENT pragma::gui::types::WIBase *get_element_under_cursor(lua::State *l, const prosper::Window *window = nullptr);
		DLLCLIENT pragma::gui::types::WIBase *get_element_under_cursor(lua::State *l, pragma::gui::types::WIBase &elRoot);
		DLLCLIENT pragma::gui::types::WIBase *get_focused_element(lua::State *l);
		DLLCLIENT pragma::gui::types::WIBase *get_focused_element(lua::State *l, prosper::Window &window);
		DLLCLIENT bool register_skin(lua::State *l, const std::string &skin, const luabind::tableT<void> &vars, const luabind::tableT<void> &skinData);
		DLLCLIENT bool register_skin(lua::State *l, const std::string &skin, const luabind::tableT<void> &vars, const luabind::tableT<void> &skinData, const std::string &baseName);
		DLLCLIENT void register_default_skin(const std::string &vars, const std::string &skinData);
		DLLCLIENT void set_skin(const std::string &skin);
		DLLCLIENT bool skin_exists(const std::string &name);
		DLLCLIENT pragma::platform::Cursor::Shape get_cursor();
		DLLCLIENT void set_cursor(pragma::platform::Cursor::Shape shape);
		DLLCLIENT pragma::platform::CursorMode get_cursor_input_mode();
		DLLCLIENT void set_cursor_input_mode(pragma::platform::CursorMode mode);
		DLLCLIENT ::Vector2i get_window_size(lua::State *l);
		DLLCLIENT bool inject_mouse_input(pragma::platform::MouseButton button, pragma::platform::KeyState state, pragma::platform::Modifier mods, const ::Vector2i &pCursorPos);
		DLLCLIENT bool inject_mouse_input(pragma::platform::MouseButton button, pragma::platform::KeyState state, pragma::platform::Modifier mods);
		DLLCLIENT bool inject_keyboard_input(pragma::platform::Key key, pragma::platform::KeyState state, pragma::platform::Modifier mods);
		DLLCLIENT bool inject_char_input(const std::string &c);
		DLLCLIENT bool inject_scroll_input(lua::State *l, const ::Vector2 &offset, const ::Vector2i &pCursorPos);
		DLLCLIENT bool inject_scroll_input(lua::State *l, const ::Vector2 &offset);
		DLLCLIENT std::shared_ptr<prosper::IImage> create_color_image(uint32_t w, uint32_t h, prosper::ImageUsageFlags usageFlags, prosper::ImageLayout initialLayout, bool msaa);
		DLLCLIENT std::shared_ptr<prosper::RenderTarget> create_render_target(uint32_t w, uint32_t h, bool enableMsaa, bool enableSampling);

		DLLCLIENT float RealTime(lua::State *l);
		DLLCLIENT float DeltaTime(lua::State *l);
		DLLCLIENT float LastThink(lua::State *l);
	};
};
