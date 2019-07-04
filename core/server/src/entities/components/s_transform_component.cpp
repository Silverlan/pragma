#include "stdafx_server.h"
#include "pragma/entities/components/s_transform_component.hpp"
#include "pragma/lua/s_lentity_handles.hpp"
#include <pragma/entities/components/base_physics_component.hpp>
#include <pragma/networking/nwm_util.h>
#include <pragma/networking/enums.hpp>

using namespace pragma;

extern DLLSERVER ServerState *server;

void STransformComponent::SendData(NetPacket &packet,networking::ClientRecipientFilter &rp)
{
	nwm::write_vector(packet,GetPosition());
	nwm::write_quat(packet,GetOrientation());
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
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	ent.SendNetEvent(m_netEvSetScale,p,pragma::networking::Protocol::SlowReliable);
	auto pPhysComponent = ent.GetPhysicsComponent();
	if(pPhysComponent.valid())
		pPhysComponent->InitializePhysics(pPhysComponent->GetPhysicsType());
}
luabind::object STransformComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<STransformComponentHandleWrapper>(l);}
void STransformComponent::SetEyeOffset(const Vector3 &offset)
{
	BaseTransformComponent::SetEyeOffset(offset);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(!ent.IsShared())
		return;
	NetPacket p;
	nwm::write_entity(p,&ent);
	nwm::write_vector(p,offset);
	server->SendPacket("ent_eyeoffset",p,pragma::networking::Protocol::SlowReliable);
}
