#include "stdafx_server.h"
#include "pragma/entities/components/s_sound_emitter_component.hpp"
#include "pragma/audio/s_alsound.h"
#include "pragma/lua/s_lentity_handles.hpp"
#include <pragma/entities/components/base_transform_component.hpp>
#include <pragma/entities/components/velocity_component.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/networking/nwm_util.h>

using namespace pragma;

extern DLLSERVER ServerState *server;

luabind::object SSoundEmitterComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SSoundEmitterComponentHandleWrapper>(l);}
void SSoundEmitterComponent::SendData(NetPacket &packet,nwm::RecipientFilter &rp)
{
	auto &sounds = m_sounds;
	auto offset = packet->GetOffset();
	auto numSounds = umath::min(sounds.size(),static_cast<size_t>(std::numeric_limits<uint8_t>::max()));
	packet->Write<uint8_t>(numSounds);
	auto numWritten = decltype(numSounds){0};
	for(auto &snd : sounds)
	{
		if(dynamic_cast<SALSoundBase*>(snd.get())->IsShared() == false)
			continue;
		packet->Write<uint32_t>(snd->GetIndex());
		if(++numWritten == numSounds)
			break;
	}
	packet->Write<uint8_t>(numWritten,&offset);
}
void SSoundEmitterComponent::UpdateSoundTransform(ALSound &snd) const
{
	auto *baseSnd = SALSound::GetBase(&snd);
	auto &ent = GetEntity();
	auto pTrComponent = ent.GetTransformComponent();
	if(pTrComponent.valid())
	{
		baseSnd->SetPosition(pTrComponent->GetPosition(),true);
		baseSnd->SetDirection(pTrComponent->GetForward(),true);
	}
	auto pVelComponent = ent.GetComponent<pragma::VelocityComponent>();
	if(pVelComponent.valid())
		baseSnd->SetVelocity(pVelComponent->GetVelocity(),true);
}
std::shared_ptr<ALSound> SSoundEmitterComponent::CreateSound(std::string sndname,ALSoundType type,bool bTransmit)
{
	auto flags = ALCreateFlags::Mono;
	if(bTransmit == false)
		flags |= ALCreateFlags::DontTransmit;
	auto ptrSnd = server->CreateSound(sndname,type,flags);
	auto *snd = static_cast<ALSound*>(ptrSnd.get());
	if(snd == nullptr)
		return ptrSnd;
	InitializeSound(ptrSnd);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(ent.IsShared() && bTransmit == true)
	{
		NetPacket p;
		nwm::write_entity(p,&ent);
		p->Write<unsigned int>(snd->GetIndex());
		server->BroadcastUDP("ent_sound",p);
	}
	return ptrSnd;
}
std::shared_ptr<ALSound> SSoundEmitterComponent::EmitSharedSound(const std::string &snd,ALSoundType type,float gain,float pitch) {return EmitSound(snd,type,gain,pitch,false);}
std::shared_ptr<ALSound> SSoundEmitterComponent::EmitSound(std::string sndname,ALSoundType type,float gain,float pitch,bool bTransmit)
{
	std::shared_ptr<ALSound> snd = CreateSound(sndname,type,bTransmit);
	ALSound *al = snd.get();
	if(al == NULL)
		return snd;
	al->SetGain(gain);
	al->SetPitch(pitch);
	auto &ent = GetEntity();
	auto pTrComponent = ent.GetTransformComponent();
	if(pTrComponent.valid())
		al->SetPosition(pTrComponent->GetPosition());
	auto pVelComponent = ent.GetComponent<pragma::VelocityComponent>();
	if(pVelComponent.valid())
		al->SetVelocity(pVelComponent->GetVelocity());
	// TODO: Orientation
	al->Play();
	return snd;
}
std::shared_ptr<ALSound> SSoundEmitterComponent::EmitSound(std::string sndname,ALSoundType type,float gain,float pitch) {return EmitSound(sndname,type,gain,pitch,true);}

std::shared_ptr<ALSound> SSoundEmitterComponent::CreateSound(std::string sndname,ALSoundType type) {return CreateSound(sndname,type,true);}
