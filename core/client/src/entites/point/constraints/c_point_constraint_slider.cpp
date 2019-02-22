#include "stdafx_client.h"
#include "pragma/entities/point/constraints/c_point_constraint_slider.h"
#include "pragma/entities/c_entityfactories.h"
#include <pragma/networking/nwm_util.h>
#include "pragma/lua/c_lentity_handles.hpp"

using namespace pragma;

LINK_ENTITY_TO_CLASS(point_constraint_slider,CPointConstraintSlider);

void CPointConstraintSliderComponent::ReceiveData(NetPacket &packet)
{
	m_kvSource = packet->ReadString();
	m_kvTarget = packet->ReadString();
	m_posTarget = nwm::read_vector(packet);

	m_kvLimitLinLow = packet->Read<float>();
	m_kvLimitLinHigh = packet->Read<float>();
}
luabind::object CPointConstraintSliderComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CPointConstraintSliderComponentHandleWrapper>(l);}

/////////////

void CPointConstraintSlider::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CPointConstraintSliderComponent>();
}
