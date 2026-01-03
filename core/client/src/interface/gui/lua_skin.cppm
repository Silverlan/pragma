// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:gui.lua_skin;

export import pragma.lua;
export import pragma.gui;

export namespace pragma::gui {
	struct DLLCLIENT WISkinClass {
		WISkinClass(lua::State *l);
		WISkinClass(const WISkinClass &) = delete;
		WISkinClass &operator=(const WISkinClass &) = delete;
		std::unordered_map<std::string, std::unique_ptr<WISkinClass>> classes;
		std::optional<luabind::object> initializeFunction = {};
		std::optional<luabind::object> releaseFunction = {};
		lua::State *lua;
		WISkinClass *Copy();
	};

	class DLLCLIENT WILuaSkin : public WISkin {
	  public:
		struct Settings {
			Settings() : base(nullptr) {}
			std::optional<luabind::object> vars = {};
			std::optional<luabind::object> skin = {};
			WILuaSkin *base;
		};
	  protected:
		lua::State *m_lua;
		WISkinClass m_rootClass;
		std::optional<luabind::object> m_vars = {};
		void InitializeClasses(WISkinClass &cl);
		void InitializeClasses();
		void FindSkinClasses(Element *el, std::unordered_map<std::string, std::unique_ptr<WISkinClass>> &classes, std::vector<WISkinClass *> &outClasses);
		WISkinClass *FindSkinClass(const std::string &className, std::unordered_map<std::string, std::unique_ptr<WISkinClass>> &classes);

		void InitializeBase(WILuaSkin *base);
		void InitializeBaseClass(WISkinClass &base, WISkinClass &cl);
	  public:
		WILuaSkin();
		virtual void Release(Element *el) override;
		virtual void Initialize(Element *el) override;
		void Initialize(lua::State *l, Settings &settings);
		void MergeInto(lua::State *l, Settings &settings);
	};
};
