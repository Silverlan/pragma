/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/c_wheel.hpp"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/entities/baseentity_luaobject.h"
#include "pragma/lua/c_lentity_handles.hpp"
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(vhc_wheel, CWheel);

void CWheel::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CWheelComponent>();
}

/////////

void CWheelComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void CWheelComponent::ReceiveData(NetPacket &packet)
{
	// TODO
}

void CWheelComponent::ReceiveSnapshotData(NetPacket &packet)
{
	// TODO
#ifdef ENABLE_DEPRECATED_PHYSICS
	auto steeringAngle = packet->Read<Float>();
	SetSteeringAngle(steeringAngle);
	auto wheelRot = packet->Read<Float>();
	SetWheelRotation(wheelRot);
	auto origin = packet->Read<Vector3>();
	auto rot = packet->Read<Quat>();
	auto *info = GetWheelInfo();
	if(info == nullptr)
		return;
	auto &t = info->m_worldTransform;
	t.setOrigin(btVector3(origin.x, origin.y, origin.z));
	t.setRotation(btQuaternion(rot.x, rot.y, rot.z, rot.w));
#endif
}

Bool CWheelComponent::ReceiveNetEvent(pragma::NetEventId eventId, NetPacket &packet)
{
#if 0
	if(eventId == m_netEvAttach)
	{
		auto *ent = nwm::read_entity(packet);
		if(ent != nullptr && ent->IsVehicle())
		{
			auto wheelId = packet->Read<UChar>();
			Attach(ent,wheelId);
		}
	}
	else if(eventId == m_netEvDetach)
		Detach();
	else if(eventId == m_netEvFrontWheel)
	{
		auto b = packet->Read<Bool>();
		SetFrontWheel(b);
	}
	else if(eventId == m_netEvAxle)
	{
		auto axle = packet->Read<Vector3>();
		SetWheelAxle(axle);
	}
	else if(eventId == m_netEvDirection)
	{
		auto dir = packet->Read<Vector3>();
		SetWheelDirection(dir);
	}
	else if(eventId == m_netEvMaxSuspensionLength)
	{
		auto len = packet->Read<Float>();
		SetMaxSuspensionLength(len);
	}
	else if(eventId == m_netEvMaxSuspensionCompression)
	{
		auto cmp = packet->Read<Float>();
		SetMaxSuspensionCompression(cmp);
	}
	else if(eventId == m_netEvRadius)
	{
		auto radius = packet->Read<Float>();
		SetWheelRadius(radius);
	}
	else if(eventId == m_netEvSuspensionStiffness)
	{
		auto stiffness = packet->Read<Float>();
		SetSuspensionStiffness(stiffness);
	}
	else if(eventId == m_netEvDampingCompression)
	{
		auto damping = packet->Read<Float>();
		SetWheelDampingCompression(damping);
	}
	else if(eventId == m_netEvFrictionSlip)
	{
		auto slip = packet->Read<Float>();
		SetFrictionSlip(slip);
	}
	else if(eventId == m_netEvSteeringAngle)
	{
		auto ang = packet->Read<Float>();
		SetSteeringAngle(ang);
	}
	else if(eventId == m_netEvRotation)
	{
		auto rot = packet->Read<Float>();
		SetWheelRotation(rot);
	}
	else if(eventId == m_netEvRollInfluence)
	{
		auto roll = packet->Read<Float>();
		SetRollInfluence(roll);
	}
	else if(eventId == m_netEvChassisConnectionPoint)
	{
		auto p = packet->Read<Vector3>();
		SetChassisConnectionPoint(p);
	}
	else if(eventId == m_netEvMaxDampingRelaxation)
	{
		auto damping = packet->Read<Float>();
		SetMaxDampingRelaxation(damping);
	}
	else
		return false;
#endif
	// TODO
	return false;
}
