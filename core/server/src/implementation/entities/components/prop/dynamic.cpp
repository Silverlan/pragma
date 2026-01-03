// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.prop.dynamic;

using namespace pragma;

void SPropDynamicComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void PropDynamic::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SPropDynamicComponent>();
}
