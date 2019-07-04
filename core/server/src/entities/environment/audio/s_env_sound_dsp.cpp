#include "stdafx_server.h"
#include "pragma/entities/environment/audio/s_env_sound_dsp.h"
#include "pragma/entities/s_entityfactories.h"
#include <sharedutils/util_string.h>
#include <sharedutils/util.h>
#include <pragma/networking/nwm_util.h>
#include <pragma/networking/enums.hpp>
#include "pragma/lua/s_lentity_handles.hpp"
#include <pragma/entities/baseentity_events.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(env_sound_dsp,EnvSoundDsp);

void SBaseSoundDspComponent::Initialize()
{
	BaseEnvSoundDspComponent::Initialize();

	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData&>(evData.get());
		if(OnSetKeyValue(kvData.key,kvData.value) == false)
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
}
void SBaseSoundDspComponent::SendData(NetPacket &packet,networking::ClientRecipientFilter &rp)
{
	packet->WriteString(m_kvDsp);
	packet->Write<float>(m_kvInnerRadius);
	packet->Write<float>(m_kvOuterRadius);
	packet->Write<float>(GetGain());
}

void SBaseSoundDspComponent::SetGain(float gain)
{
	BaseEnvSoundDspComponent::SetGain(gain);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(ent.IsSpawned() == false)
		return;
	NetPacket p {};
	p->Write<float>(gain);
	ent.SendNetEvent(m_netEvSetGain,p,pragma::networking::Protocol::SlowReliable);
}

luabind::object SSoundDspComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SSoundDspComponentHandleWrapper>(l);}

/////////////////

void EnvSoundDsp::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SSoundDspComponent>();
}
