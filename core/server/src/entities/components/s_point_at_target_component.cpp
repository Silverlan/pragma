#include "stdafx_server.h"
#include "pragma/networking/s_nwm_util.h"
#include "pragma/entities/components/s_point_at_target_component.hpp"
#include "pragma/lua/s_lentity_handles.hpp"

using namespace pragma;

#pragma optimize("",off)
void SPointAtTargetComponent::Initialize()
{
	BasePointAtTargetComponent::Initialize();
}
luabind::object SPointAtTargetComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SPointAtTargetComponentHandleWrapper>(l);}

void SPointAtTargetComponent::SendData(NetPacket &packet,nwm::RecipientFilter &rp)
{
	nwm::write_unique_entity(packet,GetPointAtTarget());
}

void SPointAtTargetComponent::SetPointAtTarget(BaseEntity *ent)
{
	BasePointAtTargetComponent::SetPointAtTarget(ent);
	NetPacket p {};
	nwm::write_entity(p,ent);
	static_cast<SBaseEntity&>(GetEntity()).SendNetEventTCP(m_netEvSetPointAtTarget,p);
}
#pragma optimize("",on)
