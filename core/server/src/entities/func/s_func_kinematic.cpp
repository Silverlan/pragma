#include "stdafx_server.h"
#include "pragma/entities/func/s_func_kinematic.hpp"
#include "pragma/entities/s_entityfactories.h"
#include "pragma/entities/point/s_point_path_node.h"
#include <sharedutils/util_string.h>
#include <pragma/audio/alsound.h>
#include "pragma/lua/s_lentity_handles.hpp"
#include <pragma/audio/alsound_type.h>
#include <networkmanager/nwm_packet.h>

using namespace pragma;

LINK_ENTITY_TO_CLASS(func_kinematic,FuncKinematic);

void SKinematicComponent::SendData(NetPacket &packet,nwm::RecipientFilter &rp)
{
	packet->WriteString(m_kvFirstNode);
	packet->Write<float>(m_kvMoveSpeed);
	packet->Write<bool>(m_bMoving);
}

void SKinematicComponent::StartForward()
{
	BaseFuncKinematicComponent::StartForward();
	static_cast<SBaseEntity&>(GetEntity()).SendNetEventTCP(m_netEvStartForward);
}

void SKinematicComponent::StartBackward()
{
	BaseFuncKinematicComponent::StartBackward();
	static_cast<SBaseEntity&>(GetEntity()).SendNetEventTCP(m_netEvStartBackward);
}

luabind::object SKinematicComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SKinematicComponentHandleWrapper>(l);}

void FuncKinematic::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SKinematicComponent>();
}
