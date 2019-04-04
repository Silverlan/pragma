#include "stdafx_client.h"
#include "pragma/entities/point/constraints/c_point_constraint_conetwist.h"
#include "pragma/entities/c_entityfactories.h"
#include <pragma/networking/nwm_util.h>
#include "pragma/lua/c_lentity_handles.hpp"
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(point_constraint_conetwist,CPointConstraintConeTwist);

void CPointConstraintConeTwistComponent::ReceiveData(NetPacket &packet)
{
	m_kvSource = packet->ReadString();
	m_kvTarget = packet->ReadString();
	m_posTarget = nwm::read_vector(packet);

	m_kvSwingSpan1 = packet->Read<float>();
	m_kvSwingSpan2 = packet->Read<float>();
	m_kvTwistSpan = packet->Read<float>();
	m_kvSoftness = packet->Read<float>();
	m_kvBiasFactor = packet->Read<float>();
	m_kvRelaxationFactor = packet->Read<float>();
}
luabind::object CPointConstraintConeTwistComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CPointConstraintConeTwistComponentHandleWrapper>(l);}

void CPointConstraintConeTwist::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CPointConstraintConeTwistComponent>();
}
