// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :particle_system.lua_particle_modifier_manager;
import :entities.components;

pragma::pts::CParticleModifierLua *pragma::pts::LuaParticleModifierManager::CreateModifier(std::string className) const
{
	string::to_lower(className);
	auto it = m_modifiers.find(className);
	if(it == m_modifiers.end())
		return nullptr;
	auto &modInfo = it->second;
	auto &o = modInfo.luaClassObject;

	auto *l = o.interpreter();
	luabind::object r;
#ifndef LUABIND_NO_EXCEPTIONS
	try {
#endif
		r = o();
#ifndef LUABIND_NO_EXCEPTIONS
	}
	catch(luabind::error &) {
		Lua::HandleLuaError(l);
		return nullptr;
	}
#endif
	if(!r) {
		Con::CWAR << Con::PREFIX_CLIENT << "Unable to create lua particle modifier '" << className << "'!" << Con::endl;
		return nullptr;
	}

	CParticleModifierLua *modifier = nullptr;
	switch(modInfo.type) {
	case Type::Initializer:
		{
			auto *oModifier = luabind::object_cast_nothrow<CParticleInitializerLua *>(r, static_cast<CParticleInitializerLua *>(nullptr));
			modifier = dynamic_cast<CParticleModifierLua *>(oModifier);
			break;
		}
	case Type::Operator:
		{
			auto *oModifier = luabind::object_cast_nothrow<CParticleOperatorLua *>(r, static_cast<CParticleOperatorLua *>(nullptr));
			modifier = dynamic_cast<CParticleModifierLua *>(oModifier);
			break;
		}
	case Type::Renderer:
		{
			auto *oModifier = luabind::object_cast_nothrow<CParticleRendererLua *>(r, static_cast<CParticleRendererLua *>(nullptr));
			modifier = dynamic_cast<CParticleModifierLua *>(oModifier);
			break;
		}
	case Type::Emitter:
		break;
	}
	if(modifier == nullptr) {
		// TODO: Can we check this when the particle modifier is being registered?
		Con::CWAR << Con::PREFIX_CLIENT << "Unable to create lua particle modifier '" << className << "': Lua class is not derived from valid particle modifier base!" << Con::endl;
		return nullptr;
	}
	r.push(l); /* 1 */
	auto idx = Lua::GetStackTop(l);

	modifier->Initialize(r);
	modifier->SetIdentifier(className);

	Lua::Pop(l); /* 0 */
	return modifier;
}

bool pragma::pts::LuaParticleModifierManager::RegisterModifier(Type type, std::string className, luabind::object &o)
{
	string::to_lower(className);
	auto itShader = m_modifiers.find(className);
	if(itShader != m_modifiers.end()) {
		Con::CWAR << "Attempted to register particle modifier '" << className << "', which has already been registered previously! Ignoring..." << Con::endl;
		return false;
		//Con::CWAR<<"Attempted to register particle modifier '"<<className<<"', which has already been registered previously! Overwriting previous definition..."<<Con::endl;
	}
	auto &pair = m_modifiers[className] = {};
	pair.luaClassObject = o;
	pair.type = type;
	return true;
}

luabind::object *pragma::pts::LuaParticleModifierManager::GetClassObject(std::string className)
{
	string::to_lower(className);
	auto it = m_modifiers.find(className);
	if(it == m_modifiers.end())
		return nullptr;
	return &it->second.luaClassObject;
}
