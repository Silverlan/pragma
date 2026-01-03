// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.triggers.push;

using namespace pragma;

void STriggerPushComponent::Initialize()
{
	BaseTriggerPushComponent::Initialize();

	SetTickPolicy(TickPolicy::Always); // TODO
}

///////

void STriggerPushComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void TriggerPush::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<STriggerPushComponent>();
}
