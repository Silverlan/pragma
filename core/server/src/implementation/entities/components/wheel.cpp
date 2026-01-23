// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

//#include "ldef_wheel.h"

module pragma.server;
import :entities.components.wheel;

import :game;

using namespace pragma;

void SWheel::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent("wheel");
}

////////////

SWheelComponent::~SWheelComponent()
{
	//Detach();
}

void SWheelComponent::SendSnapshotData(NetPacket &packet, BasePlayerComponent &pl)
{
	//packet->Write<Float>(GetSteeringAngle());
	//packet->Write<Float>(GetWheelRotation());
#ifdef ENABLE_DEPRECATED_PHYSICS
	auto *info = GetWheelInfo();
	if(info != nullptr) {
		auto &t = info->m_worldTransform;
		auto &origin = t.getOrigin();
		auto rot = t.getRotation();
		packet->Write<Vector3>(Vector3(origin.x(), origin.y(), origin.z()));
		packet->Write<Quat>(Quat(CFloat(rot.w()), CFloat(rot.x()), CFloat(rot.y()), CFloat(rot.z())));
	}
	else {
		packet->Write<Vector3>(Vector3(0.f, 0.f, 0.f));
		packet->Write<Quat>(uquat::identity());
	}
#endif
}
void SWheelComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void SWheelComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	networking::write_entity(packet, m_vehicle.valid() ? &m_vehicle->GetEntity() : nullptr);
	packet->Write<UChar>(m_wheelId);
#if 0
	packet->Write<Bool>(m_wheelInfo.bFrontWheel);
	packet->Write<Vector3>(m_wheelInfo.connectionPoint);
	packet->Write<Vector3>(m_wheelInfo.wheelAxle);
	packet->Write<Vector3>(m_wheelInfo.wheelDirection);
	packet->Write<Float>(m_wheelInfo.suspensionLength);
	packet->Write<Float>(m_wheelInfo.suspensionCompression);
	packet->Write<Float>(m_wheelInfo.dampingRelaxation);
	packet->Write<Float>(m_wheelInfo.wheelRadius);
	packet->Write<Float>(m_wheelInfo.suspensionStiffness);
	packet->Write<Float>(m_wheelInfo.wheelDampingCompression);
	packet->Write<Float>(m_wheelInfo.frictionSlip);
	packet->Write<Float>(m_wheelInfo.steeringAngle);
	packet->Write<Float>(m_wheelInfo.wheelRotation);
	packet->Write<Float>(m_wheelInfo.rollInfluence);
#endif
}

void SWheelComponent::Initialize()
{
	BaseWheelComponent::Initialize();
	static_cast<SBaseEntity &>(GetEntity()).SetShared(true);
}
/*
void SWheelComponent::Attach(BaseEntity *ent,UChar wheelId)
{
	BaseWheelComponent::Attach(ent,wheelId);
	auto &entThis = static_cast<SBaseEntity&>(GetEntity());
	if(!entThis.IsShared())
		return;
	NetPacket p;
	pragma::networking::write_entity(p,ent);
	p->Write<UChar>(wheelId);
	entThis.SendNetEvent(m_netEvAttach,p,pragma::networking::Protocol::SlowReliable);
}
void SWheelComponent::Detach()
{
	BaseWheelComponent::Detach();
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(!ent.IsShared())
		return;
	ent.SendNetEvent(m_netEvDetach,pragma::networking::Protocol::SlowReliable);
}
void SWheelComponent::SetFrontWheel(Bool b)
{
	BaseWheelComponent::SetFrontWheel(b);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(!ent.IsShared())
		return;
	NetPacket p;
	p->Write<Bool>(b);
	ent.SendNetEvent(m_netEvFrontWheel,p,pragma::networking::Protocol::SlowReliable);
}
void SWheelComponent::SetChassisConnectionPoint(const Vector3 &point)
{
	BaseWheelComponent::SetChassisConnectionPoint(point);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(!ent.IsShared())
		return;
	NetPacket p;
	p->Write<Vector3>(point);
	ent.SendNetEvent(m_netEvChassisConnectionPoint,p,pragma::networking::Protocol::SlowReliable);
}
void SWheelComponent::SetWheelAxle(const Vector3 &axis)
{
	BaseWheelComponent::SetWheelAxle(axis);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(!ent.IsShared())
		return;
	NetPacket p;
	p->Write<Vector3>(axis);
	ent.SendNetEvent(m_netEvAxle,p,pragma::networking::Protocol::SlowReliable);
}
void SWheelComponent::SetWheelDirection(const Vector3 &dir)
{
	BaseWheelComponent::SetWheelDirection(dir);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(!ent.IsShared())
		return;
	NetPacket p;
	p->Write<Vector3>(dir);
	ent.SendNetEvent(m_netEvDirection,p,pragma::networking::Protocol::SlowReliable);
}
void SWheelComponent::SetMaxSuspensionLength(Float len)
{
	BaseWheelComponent::SetMaxSuspensionLength(len);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(!ent.IsShared())
		return;
	NetPacket p;
	p->Write<Float>(len);
	ent.SendNetEvent(m_netEvMaxSuspensionLength,p,pragma::networking::Protocol::SlowReliable);
}
void SWheelComponent::SetMaxSuspensionCompression(Float cmp)
{
	BaseWheelComponent::SetMaxSuspensionCompression(cmp);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(!ent.IsShared())
		return;
	NetPacket p;
	p->Write<Float>(cmp);
	ent.SendNetEvent(m_netEvMaxSuspensionCompression,p,pragma::networking::Protocol::SlowReliable);
}
void SWheelComponent::SetMaxDampingRelaxation(Float damping)
{
	BaseWheelComponent::SetMaxDampingRelaxation(damping);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(!ent.IsShared())
		return;
	NetPacket p;
	p->Write<Float>(damping);
	ent.SendNetEvent(m_netEvMaxDampingRelaxation,p,pragma::networking::Protocol::SlowReliable);
}
void SWheelComponent::SetWheelRadius(Float radius)
{
	BaseWheelComponent::SetWheelRadius(radius);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(!ent.IsShared())
		return;
	NetPacket p;
	p->Write<Float>(radius);
	ent.SendNetEvent(m_netEvRadius,p,pragma::networking::Protocol::SlowReliable);
}
void SWheelComponent::SetSuspensionStiffness(Float stiffness)
{
	BaseWheelComponent::SetSuspensionStiffness(stiffness);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(!ent.IsShared())
		return;
	NetPacket p;
	p->Write<Float>(stiffness);
	ent.SendNetEvent(m_netEvSuspensionStiffness,p,pragma::networking::Protocol::SlowReliable);
}
void SWheelComponent::SetWheelDampingCompression(Float cmp)
{
	BaseWheelComponent::SetWheelDampingCompression(cmp);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(!ent.IsShared())
		return;
	NetPacket p;
	p->Write<Float>(cmp);
	ent.SendNetEvent(m_netEvDampingCompression,p,pragma::networking::Protocol::SlowReliable);
}
void SWheelComponent::SetFrictionSlip(Float slip)
{
	BaseWheelComponent::SetFrictionSlip(slip);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(!ent.IsShared())
		return;
	NetPacket p;
	p->Write<Float>(slip);
	ent.SendNetEvent(m_netEvFrictionSlip,p,pragma::networking::Protocol::SlowReliable);
}
void SWheelComponent::SetSteeringAngle(Float ang)
{
	BaseWheelComponent::SetSteeringAngle(ang);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
}
void SWheelComponent::SetWheelRotation(Float rot)
{
	BaseWheelComponent::SetWheelRotation(rot);

}
void SWheelComponent::SetRollInfluence(Float influence)
{
	BaseWheelComponent::SetRollInfluence(influence);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(!ent.IsShared())
		return;
	NetPacket p;
	p->Write<Float>(influence);
	ent.SendNetEvent(m_netEvRollInfluence,p,pragma::networking::Protocol::SlowReliable);
}
*/
