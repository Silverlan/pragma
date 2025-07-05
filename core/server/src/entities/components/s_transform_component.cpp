// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_server.h"
#include "pragma/entities/components/s_transform_component.hpp"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/components/base_physics_component.hpp>
#include <pragma/networking/nwm_util.h>
#include <pragma/networking/enums.hpp>

using namespace pragma;

extern DLLSERVER ServerState *server;

void STransformComponent::GetBaseTypeIndex(std::type_index &outTypeIndex) const { outTypeIndex = std::type_index(typeid(BaseTransformComponent)); }
void STransformComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	nwm::write_vector(packet, GetPosition());
	nwm::write_quat(packet, GetRotation());
	packet->Write<Vector3>(GetEyeOffset());
	packet->Write<Vector3>(GetScale());
}
void STransformComponent::SetScale(const Vector3 &scale)
{
	if(scale == GetScale())
		return;
	BaseTransformComponent::SetScale(scale);
	NetPacket p;
	p->Write<Vector3>(scale);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	ent.SendNetEvent(m_netEvSetScale, p, pragma::networking::Protocol::SlowReliable);
	auto pPhysComponent = ent.GetPhysicsComponent();
	if(pPhysComponent != nullptr)
		pPhysComponent->InitializePhysics(pPhysComponent->GetPhysicsType());
}
void STransformComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void STransformComponent::SetEyeOffset(const Vector3 &offset)
{
	BaseTransformComponent::SetEyeOffset(offset);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(!ent.IsShared())
		return;
	NetPacket p;
	nwm::write_entity(p, &ent);
	nwm::write_vector(p, offset);
	server->SendPacket("ent_eyeoffset", p, pragma::networking::Protocol::SlowReliable);
}
