#include "stdafx_server.h"
#include "pragma/entities/environment/audio/s_env_sound_dsp_flanger.h"
#include "pragma/entities/s_entityfactories.h"
#include <pragma/networking/nwm_util.h>
#include <sharedutils/util.h>
#include "pragma/lua/s_lentity_handles.hpp"

using namespace pragma;

LINK_ENTITY_TO_CLASS(env_sound_dsp_flanger,EnvSoundDspFlanger);

bool SSoundDspFlangerComponent::OnSetKeyValue(const std::string &key,const std::string &val)
{
	if(SBaseSoundDspComponent::OnSetKeyValue(key,val))
		return true;
	if(ustring::compare(key,"waveform",false))
		m_kvWaveform = util::to_int(val);
	else if(ustring::compare(key,"phase",false))
		m_kvPhase = util::to_int(val);
	else if(ustring::compare(key,"rate",false))
		m_kvRate = util::to_float(val);
	else if(ustring::compare(key,"depth",false))
		m_kvDepth = util::to_float(val);
	else if(ustring::compare(key,"feedback",false))
		m_kvFeedback = util::to_float(val);
	else if(ustring::compare(key,"delay",false))
		m_kvDelay = util::to_float(val);
	else
		return false;
	return true;
}

void SSoundDspFlangerComponent::SendData(NetPacket &packet,nwm::RecipientFilter &rp)
{
	packet->Write<int>(m_kvWaveform);
	packet->Write<int>(m_kvPhase);
	packet->Write<float>(m_kvRate);
	packet->Write<float>(m_kvDepth);
	packet->Write<float>(m_kvFeedback);
	packet->Write<float>(m_kvDelay);
}
luabind::object SSoundDspFlangerComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SSoundDspFlangerComponentHandleWrapper>(l);}

/////////////////

void EnvSoundDspFlanger::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SSoundDspFlangerComponent>();
}
