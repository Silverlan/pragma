/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/func/c_func_kinematic.hpp"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/rendering/c_rendermode.h"
#include "pragma/entities/components/c_render_component.hpp"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/util/util_handled.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(func_kinematic, CFuncKinematic);

void CKinematicComponent::Initialize()
{
	BaseFuncKinematicComponent::Initialize();
	auto &ent = static_cast<CBaseEntity &>(GetEntity());
	auto pRenderComponent = ent.GetRenderComponent();
	if(pRenderComponent)
		pRenderComponent->SetSceneRenderPass(pragma::rendering::SceneRenderPass::World);
}

Bool CKinematicComponent::ReceiveNetEvent(pragma::NetEventId eventId, NetPacket &packet)
{
	if(eventId == m_netEvStartForward)
		StartForward();
	else if(eventId == m_netEvStartBackward)
		StartBackward();
	else
		return false;
	return true;
}

void CKinematicComponent::OnEntitySpawn()
{
	BaseFuncKinematicComponent::OnEntitySpawn();
	if(m_bInitiallyMoving == true)
		StartForward();
}

void CKinematicComponent::ReceiveData(NetPacket &packet)
{
	m_kvFirstNode = packet->ReadString();
	m_kvMoveSpeed = packet->Read<float>();
	m_bInitiallyMoving = packet->Read<bool>();
}
void CKinematicComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

////////////

void CFuncKinematic::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CKinematicComponent>();
}
