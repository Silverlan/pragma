// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.triggers.remove;

using namespace pragma;

void STriggerRemoveComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void TriggerRemove::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<STriggerRemoveComponent>();
}
