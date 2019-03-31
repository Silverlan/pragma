#include "stdafx_server.h"
//#include "ldef_wheel.h"
#include "pragma/entities/components/s_wheel_component.hpp"
#include "pragma/entities/s_entityfactories.h"
#include "pragma/entities/baseentity_luaobject.h"
#include "pragma/lua/s_lentity_handles.hpp"
#include <pragma/entities/components/base_vehicle_component.hpp>
#include <pragma/networking/nwm_util.h>

using namespace pragma;

LINK_ENTITY_TO_CLASS(vhc_wheel,SWheel);

extern DLLSERVER SGame *s_game;

void SWheel::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent("wheel");
}

////////////

SWheelComponent::~SWheelComponent() {Detach();}

void SWheelComponent::SendSnapshotData(NetPacket &packet,pragma::BasePlayerComponent &pl)
{
	packet->Write<Float>(GetSteeringAngle());
	packet->Write<Float>(GetWheelRotation());
	auto *info = GetWheelInfo();
	if(info != nullptr)
	{
		auto &t = info->m_worldTransform;
		auto &origin = t.getOrigin();
		auto rot = t.getRotation();
		packet->Write<Vector3>(Vector3(origin.x(),origin.y(),origin.z()));
		packet->Write<Quat>(Quat(CFloat(rot.w()),CFloat(rot.x()),CFloat(rot.y()),CFloat(rot.z())));
	}
	else
	{
		packet->Write<Vector3>(Vector3(0.f,0.f,0.f));
		packet->Write<Quat>(uquat::identity());
	}
}
luabind::object SWheelComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SWheelComponentHandleWrapper>(l);}
void SWheelComponent::SendData(NetPacket &packet,nwm::RecipientFilter &rp)
{
	nwm::write_entity(packet,m_vehicle.valid() ? &m_vehicle->GetEntity() : nullptr);
	packet->Write<UChar>(m_wheelId);
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
}

void SWheelComponent::Initialize()
{
	BaseWheelComponent::Initialize();
	static_cast<SBaseEntity&>(GetEntity()).SetShared(true);
}

void SWheelComponent::Attach(BaseEntity *ent,UChar wheelId)
{
	BaseWheelComponent::Attach(ent,wheelId);
	auto &entThis = static_cast<SBaseEntity&>(GetEntity());
	if(!entThis.IsShared())
		return;
	NetPacket p;
	nwm::write_entity(p,ent);
	p->Write<UChar>(wheelId);
	entThis.SendNetEventTCP(m_netEvAttach,p);
}
void SWheelComponent::Detach()
{
	BaseWheelComponent::Detach();
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(!ent.IsShared())
		return;
	ent.SendNetEventTCP(m_netEvDetach);
}
void SWheelComponent::SetFrontWheel(Bool b)
{
	BaseWheelComponent::SetFrontWheel(b);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(!ent.IsShared())
		return;
	NetPacket p;
	p->Write<Bool>(b);
	ent.SendNetEventTCP(m_netEvFrontWheel,p);
}
void SWheelComponent::SetChassisConnectionPoint(const Vector3 &point)
{
	BaseWheelComponent::SetChassisConnectionPoint(point);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(!ent.IsShared())
		return;
	NetPacket p;
	p->Write<Vector3>(point);
	ent.SendNetEventTCP(m_netEvChassisConnectionPoint,p);
}
void SWheelComponent::SetWheelAxle(const Vector3 &axis)
{
	BaseWheelComponent::SetWheelAxle(axis);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(!ent.IsShared())
		return;
	NetPacket p;
	p->Write<Vector3>(axis);
	ent.SendNetEventTCP(m_netEvAxle,p);
}
void SWheelComponent::SetWheelDirection(const Vector3 &dir)
{
	BaseWheelComponent::SetWheelDirection(dir);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(!ent.IsShared())
		return;
	NetPacket p;
	p->Write<Vector3>(dir);
	ent.SendNetEventTCP(m_netEvDirection,p);
}
void SWheelComponent::SetMaxSuspensionLength(Float len)
{
	BaseWheelComponent::SetMaxSuspensionLength(len);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(!ent.IsShared())
		return;
	NetPacket p;
	p->Write<Float>(len);
	ent.SendNetEventTCP(m_netEvMaxSuspensionLength,p);
}
void SWheelComponent::SetMaxSuspensionCompression(Float cmp)
{
	BaseWheelComponent::SetMaxSuspensionCompression(cmp);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(!ent.IsShared())
		return;
	NetPacket p;
	p->Write<Float>(cmp);
	ent.SendNetEventTCP(m_netEvMaxSuspensionCompression,p);
}
void SWheelComponent::SetMaxDampingRelaxation(Float damping)
{
	BaseWheelComponent::SetMaxDampingRelaxation(damping);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(!ent.IsShared())
		return;
	NetPacket p;
	p->Write<Float>(damping);
	ent.SendNetEventTCP(m_netEvMaxDampingRelaxation,p);
}
void SWheelComponent::SetWheelRadius(Float radius)
{
	BaseWheelComponent::SetWheelRadius(radius);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(!ent.IsShared())
		return;
	NetPacket p;
	p->Write<Float>(radius);
	ent.SendNetEventTCP(m_netEvRadius,p);
}
void SWheelComponent::SetSuspensionStiffness(Float stiffness)
{
	BaseWheelComponent::SetSuspensionStiffness(stiffness);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(!ent.IsShared())
		return;
	NetPacket p;
	p->Write<Float>(stiffness);
	ent.SendNetEventTCP(m_netEvSuspensionStiffness,p);
}
void SWheelComponent::SetWheelDampingCompression(Float cmp)
{
	BaseWheelComponent::SetWheelDampingCompression(cmp);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(!ent.IsShared())
		return;
	NetPacket p;
	p->Write<Float>(cmp);
	ent.SendNetEventTCP(m_netEvDampingCompression,p);
}
void SWheelComponent::SetFrictionSlip(Float slip)
{
	BaseWheelComponent::SetFrictionSlip(slip);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(!ent.IsShared())
		return;
	NetPacket p;
	p->Write<Float>(slip);
	ent.SendNetEventTCP(m_netEvFrictionSlip,p);
}
void SWheelComponent::SetSteeringAngle(Float ang)
{
	BaseWheelComponent::SetSteeringAngle(ang);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	/*if(!ent.IsShared())
		return;
	NetPacket p;
	p->Write<Float>(ang);
	ent.SendNetEventTCP(m_netEvSteeringAngle,p);*/
}
void SWheelComponent::SetWheelRotation(Float rot)
{
	BaseWheelComponent::SetWheelRotation(rot);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	/*if(!ent.IsShared())
		return;
	NetPacket p;
	p->Write<Float>(rot);
	ent.SendNetEventTCP(m_netEvRotation,p);*/
}
void SWheelComponent::SetRollInfluence(Float influence)
{
	BaseWheelComponent::SetRollInfluence(influence);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(!ent.IsShared())
		return;
	NetPacket p;
	p->Write<Float>(influence);
	ent.SendNetEventTCP(m_netEvRollInfluence,p);
}