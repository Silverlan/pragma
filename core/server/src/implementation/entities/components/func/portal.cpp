// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.func.portal;

using namespace pragma;

void SFuncPortalComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void FuncPortal::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SFuncPortalComponent>();
}
