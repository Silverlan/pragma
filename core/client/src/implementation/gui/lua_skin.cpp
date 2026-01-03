// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :gui.lua_interface;
import :gui.lua_skin;

import :client_state;

#pragma optimize("", off)
pragma::gui::WILuaSkin::WILuaSkin() : WISkin {}, m_lua(nullptr), m_rootClass(nullptr) {}

void pragma::gui::WILuaSkin::Release(types::WIBase *el)
{
	WISkin::Release(el);
	std::vector<types::WIBase *> els;
	types::WIBase *parent = el;
	while(parent != nullptr) {
		els.push_back(parent);
		parent = parent->GetParent();
	}
	std::vector<WISkinClass *> classes = {&m_rootClass};
	for(auto i = els.size() - 1; i != size_t(-1); i--) {
		types::WIBase *el = els[i];
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
			  [this, fRelease, el](lua::State *) {
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

void pragma::gui::WILuaSkin::Initialize(types::WIBase *el)
{
	WISkin::Initialize(el);
	std::vector<types::WIBase *> els;
	types::WIBase *parent = el;
	while(parent != nullptr) {
		els.push_back(parent);
		parent = parent->GetParent();
	}
	std::vector<WISkinClass *> classes = {&m_rootClass};
	for(auto i = els.size() - 1; i != size_t(-1); i--) {
		types::WIBase *el = els[i];
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
			  [this, fInit, el](lua::State *) {
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
void pragma::gui::WILuaSkin::FindSkinClasses(types::WIBase *el, std::unordered_map<std::string, std::unique_ptr<WISkinClass>> &classes, std::vector<WISkinClass *> &outClasses)
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
pragma::gui::WISkinClass *pragma::gui::WILuaSkin::FindSkinClass(const std::string &className, std::unordered_map<std::string, std::unique_ptr<WISkinClass>> &classes)
{
	auto it = classes.find(className);
	return (it != classes.end()) ? it->second.get() : nullptr;
}

void pragma::gui::WILuaSkin::InitializeClasses() { InitializeClasses(m_rootClass); }

void pragma::gui::WILuaSkin::InitializeClasses(WISkinClass &cl)
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

void pragma::gui::WILuaSkin::InitializeBase(WILuaSkin *base)
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
				if(static_cast<Lua::Type>(luabind::type(val)) == Lua::Type::Table && static_cast<Lua::Type>(luabind::type(dstVal)) == Lua::Type::Table) {
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

void pragma::gui::WILuaSkin::InitializeBaseClass(WISkinClass &base, WISkinClass &cl)
{
	for(auto &pair : base.classes) {
		auto itThis = cl.classes.find(pair.first);
		if(itThis == cl.classes.end())
			itThis = cl.classes.insert(std::make_pair(pair.first, std::unique_ptr<WISkinClass>(pair.second->Copy()))).first;
		InitializeBaseClass(*pair.second, *itThis->second);
	}
}

void pragma::gui::WILuaSkin::Initialize(lua::State *l, Settings &settings)
{
	m_lua = l;
	m_rootClass.lua = l;
	m_vars = settings.vars;
	MergeInto(l, settings);
}

void pragma::gui::WILuaSkin::MergeInto(lua::State *l, Settings &settings)
{
	settings.skin->push(l); /* 1 */
	InitializeClasses();
	Lua::Pop(l, 1); /* 0 */
	settings.skin = {};
	InitializeBase(settings.base);
}

//////////////////////////////////

pragma::gui::WISkinClass::WISkinClass(lua::State *l) : lua(l) {}
pragma::gui::WISkinClass *pragma::gui::WISkinClass::Copy()
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
