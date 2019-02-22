#include "stdafx_server.h"
#include "pragma/entities/environment/audio/s_env_sound_dsp_echo.h"
#include "pragma/entities/s_entityfactories.h"
#include <pragma/networking/nwm_util.h>
#include <sharedutils/util.h>
#include "pragma/lua/s_lentity_handles.hpp"

using namespace pragma;

LINK_ENTITY_TO_CLASS(env_sound_dsp_echo,EnvSoundDspEcho);

bool SSoundDspEchoComponent::OnSetKeyValue(const std::string &key,const std::string &val)
{
	if(SBaseSoundDspComponent::OnSetKeyValue(key,val))
		return true;
	if(ustring::compare(key,"delay",false))
		m_kvDelay = util::to_float(val);
	else if(ustring::compare(key,"lrdelay",false))
		m_kvLRDelay = util::to_float(val);
	else if(ustring::compare(key,"damping",false))
		m_kvDamping = util::to_float(val);
	else if(ustring::compare(key,"feedback",false))
		m_kvFeedback = util::to_float(val);
	else if(ustring::compare(key,"spread",false))
		m_kvSpread = util::to_float(val);
	else
		return false;
	return true;
}

void SSoundDspEchoComponent::SendData(NetPacket &packet,nwm::RecipientFilter &rp)
{
	packet->Write<float>(m_kvDelay);
	packet->Write<float>(m_kvLRDelay);
	packet->Write<float>(m_kvDamping);
	packet->Write<float>(m_kvFeedback);
	packet->Write<float>(m_kvSpread);
}
luabind::object SSoundDspEchoComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SSoundDspEchoComponentHandleWrapper>(l);}

/////////////////

void EnvSoundDspEcho::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SSoundDspEchoComponent>();
}
