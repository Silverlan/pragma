#include "stdafx_server.h"
#include "pragma/entities/environment/audio/s_env_soundscape.h"
#include "pragma/entities/s_entityfactories.h"
#include <sharedutils/util_string.h>
#include <sharedutils/util.h>
#include <pragma/networking/nwm_util.h>
#include "pragma/lua/s_lentity_handles.hpp"
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(env_soundscape,EnvSoundScape);

void SSoundScapeComponent::SendData(NetPacket &packet,networking::ClientRecipientFilter &rp)
{
	packet->WriteString(m_kvSoundScape);
	packet->Write<float>(m_kvRadius);
	packet->Write<unsigned int>(CUInt32(m_positions.size()));
	std::unordered_map<unsigned int,std::string>::iterator it;
	for(it=m_positions.begin();it!=m_positions.end();it++)
	{
		packet->Write<unsigned int>(it->first);
		packet->WriteString(it->second);
	}
}

luabind::object SSoundScapeComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SSoundScapeComponentHandleWrapper>(l);}

///////

void EnvSoundScape::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SSoundScapeComponent>();
}
