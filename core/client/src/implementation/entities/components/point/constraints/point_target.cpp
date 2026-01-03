// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.point_target;
import :entities.components.transform;

using namespace pragma;

void CPointTargetComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void CPointTarget::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CTransformComponent>();
	AddComponent<CPointTargetComponent>();
}
