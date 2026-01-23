// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.camera;

import :entities.components.field_angle;

using namespace pragma;

void SCameraComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void SCameraComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseEnvCameraComponent::OnEntityComponentAdded(component);
	if(typeid(component) == typeid(SFieldAngleComponent))
		SetFieldAngleComponent(static_cast<SFieldAngleComponent &>(component));
}

void EnvCamera::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SCameraComponent>();
}
