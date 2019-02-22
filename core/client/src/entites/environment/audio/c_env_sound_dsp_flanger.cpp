#include "stdafx_client.h"
#include "pragma/entities/environment/audio/c_env_sound_dsp_flanger.h"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/c_engine.h"
#include "pragma/entities/components/c_player_component.hpp"
#include <pragma/networking/nwm_util.h>
#include "pragma/lua/c_lentity_handles.hpp"
#include <alsoundsystem.hpp>

using namespace pragma;

extern DLLCENGINE CEngine *c_engine;

LINK_ENTITY_TO_CLASS(env_sound_dsp_flanger,CEnvSoundDspFlanger);

void CSoundDspFlangerComponent::ReceiveData(NetPacket &packet)
{
	m_kvWaveform = packet->Read<int>();
	m_kvPhase = packet->Read<int>();
	m_kvRate = packet->Read<float>();
	m_kvDepth = packet->Read<float>();
	m_kvFeedback = packet->Read<float>();
	m_kvDelay = packet->Read<float>();
}

void CSoundDspFlangerComponent::OnEntitySpawn()
{
	//BaseEnvSoundDspFlanger::OnEntitySpawn(); // Not calling BaseEnvSoundDspFlanger::OnEntitySpawn() to skip the dsp effect lookup
	auto *soundSys = c_engine->GetSoundSystem();
	if(soundSys == nullptr)
		return;
	al::EfxFlangerProperties props {};
	props.iWaveform = m_kvWaveform;
	props.iPhase = m_kvPhase;
	props.flRate = m_kvRate;
	props.flDepth = m_kvDepth;
	props.flFeedback = m_kvFeedback;
	props.flDelay = m_kvDelay;
	m_dsp = soundSys->CreateEffect(props);
}
luabind::object CSoundDspFlangerComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CSoundDspFlangerComponentHandleWrapper>(l);}

//////////////////

void CEnvSoundDspFlanger::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CSoundDspFlangerComponent>();
}
