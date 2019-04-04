#include "stdafx_server.h"
#include "pragma/entities/environment/audio/s_env_sound_dsp_distortion.h"
#include "pragma/entities/s_entityfactories.h"
#include <pragma/networking/nwm_util.h>
#include <sharedutils/util.h>
#include "pragma/lua/s_lentity_handles.hpp"
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(env_sound_dsp_distortion,EnvSoundDspDistortion);

bool SSoundDspDistortionComponent::OnSetKeyValue(const std::string &key,const std::string &val)
{
	if(SBaseSoundDspComponent::OnSetKeyValue(key,val))
		return true;
	if(ustring::compare(key,"edge",false))
		m_kvEdge = util::to_float(val);
	else if(ustring::compare(key,"gain",false))
		m_kvGain = util::to_float(val);
	else if(ustring::compare(key,"lowpass_cutoff",false))
		m_kvLowpassCutoff = util::to_float(val);
	else if(ustring::compare(key,"eqcenter",false))
		m_kvEqCenter = util::to_float(val);
	else if(ustring::compare(key,"eqbandwith",false))
		m_kvEqBandwidth = util::to_float(val);
	else
		return false;
	return true;
}

void SSoundDspDistortionComponent::SendData(NetPacket &packet,nwm::RecipientFilter &rp)
{
	packet->Write<float>(m_kvEdge);
	packet->Write<float>(m_kvGain);
	packet->Write<float>(m_kvLowpassCutoff);
	packet->Write<float>(m_kvEqCenter);
	packet->Write<float>(m_kvEqBandwidth);
}
luabind::object SSoundDspDistortionComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SSoundDspDistortionComponentHandleWrapper>(l);}

/////////////////

void EnvSoundDspDistortion::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SSoundDspDistortionComponent>();
}
