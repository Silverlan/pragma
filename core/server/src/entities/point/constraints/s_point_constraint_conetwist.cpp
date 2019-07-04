#include "stdafx_server.h"
#include "pragma/entities/point/constraints/s_point_constraint_conetwist.h"
#include "pragma/entities/s_entityfactories.h"
#include <pragma/physics/physobj.h>
#include <sharedutils/util_string.h>
#include <pragma/networking/nwm_util.h>
#include "pragma/lua/s_lentity_handles.hpp"
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(point_constraint_conetwist,PointConstraintConeTwist);

void SPointConstraintConeTwistComponent::SendData(NetPacket &packet,networking::ClientRecipientFilter &rp)
{
	packet->WriteString(m_kvSource);
	packet->WriteString(m_kvTarget);
	nwm::write_vector(packet,m_posTarget);

	packet->Write<float>(m_kvSwingSpan1);
	packet->Write<float>(m_kvSwingSpan2);
	packet->Write<float>(m_kvTwistSpan);
	packet->Write<float>(m_kvSoftness);
	packet->Write<float>(m_kvBiasFactor);
	packet->Write<float>(m_kvRelaxationFactor);
}

luabind::object SPointConstraintConeTwistComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SPointConstraintConeTwistComponentHandleWrapper>(l);}

void PointConstraintConeTwist::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SPointConstraintConeTwistComponent>();
}
