/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/gui/wiluaskin.h"
#include <wgui/wibase.h>
#include "pragma/gui/wgui_luainterface.h"
#include <pragma/lua/lua_call.hpp>
#include <queue>

extern DLLCLIENT ClientState *client;
#pragma optimize("", off)
WILuaSkin::WILuaSkin() : WISkin {}, m_lua(nullptr), m_rootClass(nullptr) {}

void WILuaSkin::Release(WIBase *el)
{
	WISkin::Release(el);
	std::vector<WIBase *> els;
	WIBase *parent = el;
	while(parent != nullptr) {
		els.push_back(parent);
		parent = parent->GetParent();
	}
	std::vector<WISkinClass *> classes = {&m_rootClass};
	for(auto i = els.size() - 1; i != size_t(-1); i--) {
		WIBase *el = els[i];
		auto numClasses = classes.size();
		for(size_t k = 0; k < numClasses; k++)
			FindSkinClasses(el, classes[k]->classes, classes);
	}
	std::vector<WISkinClass *> elClasses;
	for(unsigned int i = 0; i < classes.size(); i++)
		FindSkinClasses(el, classes[i]->classes, elClasses);
	for(unsigned int i = 0; i < elClasses.size(); i++) {
		if(elClasses[i]->releaseFunction.has_value() && m_vars.has_value()) {
			auto fRelease = elClasses[i]->releaseFunction;
			Lua::CallFunction(
			  m_lua,
			  [this, fRelease, el](lua_State *) {
				  fRelease->push(m_lua);

				  m_vars->push(m_lua);
				  //WISkinClass *cl = elClasses[i];
				  auto o = WGUILuaInterface::GetLuaObject(m_lua, *el);
				  o.push(m_lua);
				  return Lua::StatusCode::Ok;
			  },
			  0);
		}
	}
}

void WILuaSkin::Initialize(WIBase *el)
{
	WISkin::Initialize(el);
	std::vector<WIBase *> els;
	WIBase *parent = el;
	while(parent != nullptr) {
		els.push_back(parent);
		parent = parent->GetParent();
	}
	std::vector<WISkinClass *> classes = {&m_rootClass};
	for(auto i = els.size() - 1; i != size_t(-1); i--) {
		WIBase *el = els[i];
		auto numClasses = classes.size();
		for(size_t k = 0; k < numClasses; k++)
			FindSkinClasses(el, classes[k]->classes, classes);
	}
	std::vector<WISkinClass *> elClasses;
	for(unsigned int i = 0; i < classes.size(); i++)
		FindSkinClasses(el, classes[i]->classes, elClasses);
	for(unsigned int i = 0; i < elClasses.size(); i++) {
		if(elClasses[i]->initializeFunction.has_value() && m_vars.has_value()) {
			auto fInit = elClasses[i]->initializeFunction;
			Lua::CallFunction(
			  m_lua,
			  [this, fInit, el](lua_State *) {
				  fInit->push(m_lua);

				  m_vars->push(m_lua);
				  //WISkinClass *cl = elClasses[i];
				  auto o = WGUILuaInterface::GetLuaObject(m_lua, *el);
				  o.push(m_lua);
				  return Lua::StatusCode::Ok;
			  },
			  0);
		}
	}
}
void WILuaSkin::FindSkinClasses(WIBase *el, std::unordered_map<std::string, std::unique_ptr<WISkinClass>> &classes, std::vector<WISkinClass *> &outClasses)
{
	//WISkinClass *cl = FindSkinClass(el->GetClass(),classes);
	//if(cl != nullptr)
	//	outClasses.push_back(cl);
	std::vector<std::string> &styleClasses = el->GetStyleClasses();
	for(unsigned int i = 0; i < styleClasses.size(); i++) {
		auto *cl = FindSkinClass(styleClasses[i], classes);
		if(cl != nullptr)
			outClasses.push_back(cl);
	}
}
WISkinClass *WILuaSkin::FindSkinClass(const std::string &className, std::unordered_map<std::string, std::unique_ptr<WISkinClass>> &classes)
{
	auto it = classes.find(className);
	return (it != classes.end()) ? it->second.get() : nullptr;
}

void WILuaSkin::InitializeClasses() { InitializeClasses(m_rootClass); }

void WILuaSkin::InitializeClasses(WISkinClass &cl)
{
	int table = Lua::GetStackTop(m_lua);
	Lua::PushNil(m_lua);                       /* 1 */
	while(Lua::GetNextPair(m_lua, table) != 0) /* 2 */
	{
		if(Lua::IsTable(m_lua, -1) && Lua::IsString(m_lua, -2)) {
			std::string key = Lua::CheckString(m_lua, -2);
			WISkinClass *subClass = new WISkinClass(m_lua);
			cl.classes.insert(std::unordered_map<std::string, WISkinClass *>::value_type(key, subClass));
			Lua::PushString(m_lua, "Initialize"); /* 3 */
			Lua::GetTableValue(m_lua, -2);        /* 3 */
			if(Lua::IsFunction(m_lua, -1)) {
				subClass->initializeFunction = luabind::object {luabind::from_stack(m_lua, -1)}; /* 2 */
				Lua::Pop(m_lua);
			}
			else
				Lua::Pop(m_lua, 1); /* 2 */

			Lua::PushString(m_lua, "Release"); /* 3 */
			Lua::GetTableValue(m_lua, -2);     /* 3 */
			if(Lua::IsFunction(m_lua, -1)) {
				subClass->releaseFunction = luabind::object {luabind::from_stack(m_lua, -1)}; /* 2 */
				Lua::Pop(m_lua);
			}
			else
				Lua::Pop(m_lua, 1); /* 2 */

			Lua::PushString(m_lua, "children"); /* 3 */
			Lua::GetTableValue(m_lua, -2);      /* 3 */
			if(Lua::IsTable(m_lua, -1))
				InitializeClasses(*subClass); /* 3 */
			Lua::Pop(m_lua, 1);               /* 2 */
		}
		Lua::Pop(m_lua, 1); /* 1 */
	} /* 0 */
}

void WILuaSkin::InitializeBase(WILuaSkin *base)
{
	if(base == nullptr || m_vars.has_value() == false || base->m_vars.has_value() == false)
		return;
	std::function<void(luabind::object &, luabind::object &)> mergeTable = nullptr;
	mergeTable = [&mergeTable](luabind::object &tSrc, luabind::object &tDst) {
		for(luabind::iterator it {tSrc}, end; it != end; ++it) {
			auto key = it.key();
			auto dstVal = tDst[key];
			if(dstVal) {
				auto val = *it;
				if(luabind::type(val) == LUA_TTABLE && luabind::type(dstVal) == LUA_TTABLE) {
					luabind::object oVal {val};
					luabind::object oDstVal {dstVal};
					mergeTable(oVal, oDstVal);
				}
				continue;
			}
			auto val = *it;
			tDst[key] = val;
		}
	};
	mergeTable(*base->m_vars, *m_vars);

	InitializeBaseClass(base->m_rootClass, m_rootClass);
}

void WILuaSkin::InitializeBaseClass(WISkinClass &base, WISkinClass &cl)
{
	for(auto &pair : base.classes) {
		auto itThis = cl.classes.find(pair.first);
		if(itThis == cl.classes.end())
			itThis = cl.classes.insert(std::make_pair(pair.first, std::unique_ptr<WISkinClass>(pair.second->Copy()))).first;
		InitializeBaseClass(*pair.second, *itThis->second);
	}
}

void WILuaSkin::Initialize(lua_State *l, Settings &settings)
{
	m_lua = l;
	m_rootClass.lua = l;
	m_vars = settings.vars;
	MergeInto(l, settings);
}

void WILuaSkin::MergeInto(lua_State *l, Settings &settings)
{
	settings.skin->push(l); /* 1 */
	InitializeClasses();
	Lua::Pop(l, 1); /* 0 */
	settings.skin = {};
	InitializeBase(settings.base);
}

//////////////////////////////////

WISkinClass::WISkinClass(lua_State *l) : lua(l) {}
WISkinClass *WISkinClass::Copy()
{
	WISkinClass *other = new WISkinClass(lua);

	other->initializeFunction = initializeFunction;
	other->releaseFunction = releaseFunction;

	for(auto &pair : classes) {
		auto cpy = std::unique_ptr<WISkinClass>(pair.second->Copy());
		classes.insert(std::make_pair(pair.first, std::move(cpy)));
	}
	return other;
}
