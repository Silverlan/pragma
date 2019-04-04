#include "stdafx_client.h"
#include "pragma/entities/point/constraints/c_point_constraint_fixed.h"
#include "pragma/entities/c_entityfactories.h"
#include <pragma/networking/nwm_util.h>
#include "pragma/lua/c_lentity_handles.hpp"
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(point_constraint_fixed,CPointConstraintFixed);

void CPointConstraintFixedComponent::ReceiveData(NetPacket &packet)
{
	m_kvSource = packet->ReadString();
	m_kvTarget = packet->ReadString();
	m_posTarget = nwm::read_vector(packet);
}
luabind::object CPointConstraintFixedComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CPointConstraintFixedComponentHandleWrapper>(l);}

void CPointConstraintFixed::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CPointConstraintFixedComponent>();
}
