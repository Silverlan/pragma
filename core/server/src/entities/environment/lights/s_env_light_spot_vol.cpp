#include "stdafx_server.h"
#include "pragma/networking/s_nwm_util.h"
#include "pragma/entities/environment/lights/s_env_light_spot_vol.h"
#include "pragma/entities/s_entityfactories.h"
#include "pragma/lua/s_lentity_handles.hpp"
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(env_light_spot_vol,EnvLightSpotVol);

void SLightSpotVolComponent::Initialize()
{
	BaseEnvLightSpotVolComponent::Initialize();
	static_cast<SBaseEntity&>(GetEntity()).SetSynchronized(false);
}
void SLightSpotVolComponent::SendData(NetPacket &packet,nwm::RecipientFilter &rp)
{
	packet->Write<float>(m_coneAngle);
	packet->Write<float>(m_coneStartOffset);
	nwm::write_unique_entity(packet,m_hSpotlightTarget.get());
}

void SLightSpotVolComponent::SetSpotlightTarget(BaseEntity &ent)
{
	BaseEnvLightSpotVolComponent::SetSpotlightTarget(ent);
	NetPacket p {};
	nwm::write_entity(p,&ent);
	static_cast<SBaseEntity&>(GetEntity()).SendNetEventTCP(m_netEvSetSpotlightTarget,p);
}

luabind::object SLightSpotVolComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SLightSpotVolComponentHandleWrapper>(l);}

void EnvLightSpotVol::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SLightSpotVolComponent>();
}
