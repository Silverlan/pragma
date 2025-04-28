/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __WILUASKIN_H__
#define __WILUASKIN_H__

#include "pragma/clientdefinitions.h"
#include "wgui/wiskin.h"
#include "luasystem.h"
#include <optional>

struct DLLCLIENT WISkinClass {
	WISkinClass(lua_State *l);
	WISkinClass(const WISkinClass &) = delete;
	WISkinClass &operator=(const WISkinClass &) = delete;
	std::unordered_map<std::string, std::unique_ptr<WISkinClass>> classes;
	std::optional<luabind::object> initializeFunction = {};
	std::optional<luabind::object> releaseFunction = {};
	lua_State *lua;
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
	lua_State *m_lua;
	WISkinClass m_rootClass;
	std::optional<luabind::object> m_vars = {};
	void InitializeClasses(WISkinClass &cl);
	void InitializeClasses();
	void FindSkinClasses(WIBase *el, std::unordered_map<std::string, std::unique_ptr<WISkinClass>> &classes, std::vector<WISkinClass *> &outClasses);
	WISkinClass *FindSkinClass(const std::string &className, std::unordered_map<std::string, std::unique_ptr<WISkinClass>> &classes);

	void InitializeBase(WILuaSkin *base);
	void InitializeBaseClass(WISkinClass &base, WISkinClass &cl);
  public:
	WILuaSkin();
	virtual void Release(WIBase *el) override;
	virtual void Initialize(WIBase *el) override;
	void Initialize(lua_State *l, Settings &settings);
	void MergeInto(lua_State *l, Settings &settings);
};

#endif
