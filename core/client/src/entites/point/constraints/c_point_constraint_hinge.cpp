#include "stdafx_client.h"
#include "pragma/entities/point/constraints/c_point_constraint_hinge.h"
#include "pragma/entities/c_entityfactories.h"
#include <pragma/networking/nwm_util.h>
#include "pragma/lua/c_lentity_handles.hpp"

using namespace pragma;

LINK_ENTITY_TO_CLASS(point_constraint_hinge,CPointConstraintHinge);

void CPointConstraintHingeComponent::ReceiveData(NetPacket &packet)
{
	m_kvSource = packet->ReadString();
	m_kvTarget = packet->ReadString();
	m_posTarget = nwm::read_vector(packet);

	m_kvLimitLow = packet->Read<float>();
	m_kvLimitHigh = packet->Read<float>();
	m_kvLimitSoftness = packet->Read<float>();
	m_kvLimitBiasFactor = packet->Read<float>();
	m_kvLimitRelaxationFactor = packet->Read<float>();
}
luabind::object CPointConstraintHingeComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CPointConstraintHingeComponentHandleWrapper>(l);}

void CPointConstraintHinge::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CPointConstraintHingeComponent>();
}
