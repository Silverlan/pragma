// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.point.target;

import :entities.components;

using namespace pragma;

void SPointTargetComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void PointTarget::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<STransformComponent>();
	AddComponent<SPointTargetComponent>();
}
