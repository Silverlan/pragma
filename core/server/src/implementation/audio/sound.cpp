// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :audio.sound;

import :server_state;

pragma::audio::SALSoundBase::SALSoundBase(bool bShared) : m_bShared(bShared) {}
bool pragma::audio::SALSoundBase::IsShared() const { return m_bShared; }
void pragma::audio::SALSoundBase::SetShared(bool b) { m_bShared = b; }

pragma::audio::SALSoundBase *pragma::audio::SALSound::GetBase(ALSound *snd) { return dynamic_cast<SALSoundBase *>(snd); } //(snd->IsSoundScript() == false) ? static_cast<SALSoundBase*>(static_cast<SALSound*>(snd)) : static_cast<SALSoundBase*>(static_cast<SALSoundScript*>(snd));}

#pragma warning(disable : 4056)
pragma::audio::SALSound::SALSound(NetworkState *nw, unsigned int idx, float duration, const std::string &soundName, ALCreateFlags createFlags)
    : ALSound(nw), SALSoundBase(math::is_flag_set(createFlags, ALCreateFlags::DontTransmit) == false), m_soundName {soundName}, m_createFlags {createFlags}
{
	m_index = idx;
	m_duration = duration;
}
#pragma warning(default : 4056)

pragma::audio::SALSound::~SALSound()
{
	SendEvent(NetEvent::SetIndex, [](NetPacket &p) { p->Write<uint32_t>(static_cast<uint32_t>(0)); });
	Game *game = ServerState::Get()->GetGameState();
	if(game == nullptr)
		return;
	//for(int i=0;i<m_luaCallbacks.size();i++)
	//	lua_removereference(game->GetLuaState(),m_luaCallbacks[i]);
}

const std::string &pragma::audio::SALSound::GetSoundName() const { return m_soundName; }
pragma::audio::ALCreateFlags pragma::audio::SALSound::GetCreateFlags() const { return m_createFlags; }

void pragma::audio::SALSound::SendEvent(NetEvent evId, const std::function<void(NetPacket &)> &write, bool bUDP) const
{
	if(IsShared() == false)
		return;
	NetPacket p;
	p->Write<uint8_t>(math::to_integral(evId));
	p->Write<unsigned int>(this->GetIndex());
	if(write != nullptr)
		write(p);
	if(bUDP == true)
		ServerState::Get()->SendPacket(networking::net_messages::client::SND_EV, p, networking::Protocol::FastUnreliable);
	else
		ServerState::Get()->SendPacket(networking::net_messages::client::SND_EV, p, networking::Protocol::SlowReliable);
}

void pragma::audio::SALSound::SetState(ALState state)
{
	auto old = GetState();
	if(state != old) {
		CallCallbacks<void, ALState, ALState>("OnStateChanged", old, state);
		CallLuaCallbacks<void, int32_t, int32_t>("OnStateChanged", math::to_integral(old), math::to_integral(state));
	}
	ALSoundBase::SetState(state);
}

unsigned int pragma::audio::SALSound::GetIndex() const { return m_index; }

void pragma::audio::SALSound::FadeIn(float time)
{
	float gain = GetGain();
	SetGain(0);
	CancelFade();
	if(!IsPlaying())
		Play();
	m_fade = std::unique_ptr<SoundFade>(new SoundFade(true, ServerState::Get()->RealTime(), time, gain));

	SendEvent(NetEvent::FadeIn, [&time](NetPacket &p) { p->Write<float>(time); });
}

void pragma::audio::SALSound::FadeOut(float time)
{
	if(!IsPlaying())
		return;
	float gain = GetGain();
	CancelFade();
	m_fade = std::unique_ptr<SoundFade>(new SoundFade(false, ServerState::Get()->RealTime(), time, gain));

	SendEvent(NetEvent::FadeOut, [&time](NetPacket &p) { p->Write<float>(time); });
}

void pragma::audio::SALSound::Update()
{
	if(GetState() == ALState::Playing) {
		double t = ServerState::Get()->RealTime();
		double tDelta = t - m_tLastUpdate;
		tDelta *= GetPitch() / 1.f;
		float dur = GetDuration();
		m_offset += CFloat(tDelta / double(dur));
		if(m_offset > 1.0) {
			if(IsLooping())
				m_offset -= std::floor(m_offset);
			else {
				m_offset = 1.0;
				SetState(ALState::Stopped);
			}
		}
		m_tLastUpdate = t;
	}
}

void pragma::audio::SALSound::PostUpdate() { ALSound::PostUpdate(); }

void pragma::audio::SALSound::Play()
{
	if(GetState() != ALState::Paused) {
		m_offset = 0;
		InitRange();
	}
	SetState(ALState::Playing);
	SendEvent(NetEvent::Play);
	m_tLastUpdate = ServerState::Get()->RealTime();
	if(m_tFadeIn > 0.f)
		FadeIn(m_tFadeIn);
}

void pragma::audio::SALSound::Stop()
{
	SetState(ALState::Stopped);
	SendEvent(NetEvent::Stop);
}

void pragma::audio::SALSound::Pause()
{
	SetState(ALState::Paused);
	SendEvent(NetEvent::Pause);
}

void pragma::audio::SALSound::Rewind()
{
	m_offset = 0;
	SetState(ALState::Initial);
	SendEvent(NetEvent::Rewind);
}

void pragma::audio::SALSound::SetOffset(float offset)
{
	offset = std::min(offset, 1.f);
	m_offset = offset;
	SendEvent(NetEvent::SetOffset, [&offset](NetPacket &p) { p->Write<float>(offset); });
}

float pragma::audio::SALSound::GetOffset() const { return m_offset; }
void pragma::audio::SALSound::SetPitch(float pitch)
{
	ALSoundBase::SetPitch(pitch);
	SendEvent(NetEvent::SetPitch, [&pitch](NetPacket &p) { p->Write<float>(pitch); });
}

float pragma::audio::SALSound::GetPitch() const { return ALSoundBase::GetPitch(); }
void pragma::audio::SALSound::SetRange(float start, float end)
{
	ALSound::SetRange(start, end);
	SendEvent(NetEvent::SetRange, [&start, &end](NetPacket &p) {
		p->Write<float>(start);
		p->Write<float>(end);
	});
}
void pragma::audio::SALSound::ClearRange()
{
	ALSound::ClearRange();
	SendEvent(NetEvent::ClearRange);
}
void pragma::audio::SALSound::SetLooping(bool loop)
{
	ALSoundBase::SetLooping(loop);
	SendEvent(NetEvent::SetLooping, [&loop](NetPacket &p) { p->Write<bool>(loop); });
}

pragma::audio::ALState pragma::audio::SALSound::GetState() const { return ALSoundBase::GetState(); }

bool pragma::audio::SALSound::IsLooping() const { return ALSoundBase::IsLooping(); }
bool pragma::audio::SALSound::IsPlaying() const { return ALSoundBase::IsPlaying(); }
bool pragma::audio::SALSound::IsPaused() const { return ALSoundBase::IsPaused(); }
bool pragma::audio::SALSound::IsStopped() const { return ALSoundBase::IsStopped(); }
void pragma::audio::SALSound::SetGain(float gain)
{
	ALSoundBase::SetGain(gain);
	SendEvent(NetEvent::SetGain, [&gain](NetPacket &p) { p->Write<float>(gain); });
}

float pragma::audio::SALSound::GetGain() const { return ALSoundBase::GetGain(); }
void pragma::audio::SALSound::SetPosition(const Vector3 &pos, bool bDontTransmit)
{
	ALSoundBase::SetPosition(pos);
	if(bDontTransmit == true)
		return;
	SendEvent(NetEvent::SetPos, [&pos](NetPacket &p) { networking::write_vector(p, pos); });
}
void pragma::audio::SALSound::SetPosition(const Vector3 &pos) { SetPosition(pos, false); }

Vector3 pragma::audio::SALSound::GetPosition() const
{
	if(m_hSourceEntity.valid()) {
		auto pTrComponent = m_hSourceEntity.get()->GetTransformComponent();
		if(pTrComponent != nullptr)
			return pTrComponent->GetPosition();
	}
	return ALSoundBase::GetPosition();
}
void pragma::audio::SALSound::SetVelocity(const Vector3 &vel, bool bDontTransmit)
{
	ALSoundBase::SetVelocity(vel);
	if(bDontTransmit == true)
		return;
	SendEvent(NetEvent::SetVelocity, [&vel](NetPacket &p) { networking::write_vector(p, vel); });
}
void pragma::audio::SALSound::SetVelocity(const Vector3 &vel) { SetVelocity(vel, false); }

Vector3 pragma::audio::SALSound::GetVelocity() const { return ALSoundBase::GetVelocity(); }
void pragma::audio::SALSound::SetDirection(const Vector3 &dir, bool bDontTransmit)
{
	ALSoundBase::SetDirection(dir);
	if(bDontTransmit == true)
		return;
	SendEvent(NetEvent::SetDirection, [&dir](NetPacket &p) { networking::write_vector(p, dir); });
}
void pragma::audio::SALSound::SetDirection(const Vector3 &dir) { SetDirection(dir, false); }

Vector3 pragma::audio::SALSound::GetDirection() const { return ALSoundBase::GetDirection(); }
void pragma::audio::SALSound::SetRelative(bool b)
{
	ALSoundBase::SetRelative(b);
	SendEvent(NetEvent::SetRelative, [&b](NetPacket &p) { p->Write<bool>(b); });
}

bool pragma::audio::SALSound::IsRelative() const { return ALSoundBase::IsRelative(); }
float pragma::audio::SALSound::GetDuration() const { return ALSoundBase::GetDuration(); }
float pragma::audio::SALSound::GetReferenceDistance() const { return ALSoundBase::GetReferenceDistance(); }
void pragma::audio::SALSound::SetReferenceDistance(float dist)
{
	ALSoundBase::SetReferenceDistance(dist);
	SendEvent(NetEvent::SetReferenceDistance, [&dist](NetPacket &p) { p->Write<float>(dist); });
}

float pragma::audio::SALSound::GetRolloffFactor() const { return ALSoundBase::GetRolloffFactor(); }
void pragma::audio::SALSound::SetRolloffFactor(float rolloff)
{
	ALSoundBase::SetRolloffFactor(rolloff);
	SendEvent(NetEvent::SetRolloffFactor, [&rolloff](NetPacket &p) { p->Write<float>(rolloff); });
}
float pragma::audio::SALSound::GetRoomRolloffFactor() const { return ALSoundBase::GetRoomRolloffFactor(); }
void pragma::audio::SALSound::SetRoomRolloffFactor(float roomFactor)
{
	ALSoundBase::SetRoomRolloffFactor(roomFactor);
	SendEvent(NetEvent::SetRoomRolloffFactor, [&roomFactor](NetPacket &p) { p->Write<float>(roomFactor); });
}

float pragma::audio::SALSound::GetMaxDistance() const { return ALSoundBase::GetMaxDistance(); }
void pragma::audio::SALSound::SetMaxDistance(float dist)
{
	ALSoundBase::SetMaxDistance(dist);
	SendEvent(NetEvent::SetMaxDistance, [&dist](NetPacket &p) { p->Write<float>(dist); });
}

float pragma::audio::SALSound::GetMinGain() const { return ALSoundBase::GetMinGain(); }
void pragma::audio::SALSound::SetMinGain(float gain)
{
	ALSoundBase::SetMinGain(gain);
	SendEvent(NetEvent::SetMinGain, [&gain](NetPacket &p) { p->Write<float>(gain); });
}
float pragma::audio::SALSound::GetMaxGain() const { return ALSoundBase::GetMaxGain(); }
void pragma::audio::SALSound::SetMaxGain(float gain)
{
	ALSoundBase::SetMaxGain(gain);
	SendEvent(NetEvent::SetMaxGain, [&gain](NetPacket &p) { p->Write<float>(gain); });
}
float pragma::audio::SALSound::GetInnerConeAngle() const { return ALSoundBase::GetInnerConeAngle(); }
void pragma::audio::SALSound::SetInnerConeAngle(float ang)
{
	ALSoundBase::SetInnerConeAngle(ang);
	SendEvent(NetEvent::SetConeInnerAngle, [&ang](NetPacket &p) { p->Write<float>(ang); });
}
float pragma::audio::SALSound::GetOuterConeAngle() const { return ALSoundBase::GetOuterConeAngle(); }
void pragma::audio::SALSound::SetOuterConeAngle(float ang)
{
	ALSoundBase::SetOuterConeAngle(ang);
	SendEvent(NetEvent::SetConeOuterAngle, [&ang](NetPacket &p) { p->Write<float>(ang); });
}
float pragma::audio::SALSound::GetOuterConeGain() const { return ALSoundBase::GetOuterConeGain(); }
void pragma::audio::SALSound::SetOuterConeGain(float gain)
{
	ALSoundBase::SetOuterConeGain(gain);
	SendEvent(NetEvent::SetConeOuterGain, [&gain](NetPacket &p) { p->Write<float>(gain); });
}
float pragma::audio::SALSound::GetOuterConeGainHF() const { return ALSoundBase::GetOuterConeGainHF(); }
void pragma::audio::SALSound::SetOuterConeGainHF(float gain)
{
	ALSoundBase::SetOuterConeGainHF(gain);
	SendEvent(NetEvent::SetConeOuterGainHF, [&gain](NetPacket &p) { p->Write<float>(gain); });
}
void pragma::audio::SALSound::SetType(ALSoundType type)
{
	ALSound::SetType(type);
	SendEvent(NetEvent::SetType, [&type](NetPacket &p) { p->Write<ALSoundType>(type); });
}

void pragma::audio::SALSound::SetFlags(unsigned int flags)
{
	ALSound::SetFlags(flags);
	SendEvent(NetEvent::SetFlags, [&flags](NetPacket &p) { p->Write<unsigned int>(flags); });
}

void pragma::audio::SALSound::SetSource(ecs::BaseEntity *ent)
{
	ALSound::SetSource(ent);
	SendEvent(NetEvent::SetSource, [&ent](NetPacket &p) { networking::write_entity(p, ent); });
}

void pragma::audio::SALSound::SetFadeInDuration(float t)
{
	ALSound::SetFadeInDuration(t);
	SendEvent(NetEvent::SetFadeInDuration, [&t](NetPacket &p) { p->Write<float>(t); });
}
void pragma::audio::SALSound::SetFadeOutDuration(float t)
{
	ALSound::SetFadeOutDuration(t);
	SendEvent(NetEvent::SetFadeOutDuration, [&t](NetPacket &p) { p->Write<float>(t); });
}

uint32_t pragma::audio::SALSound::GetPriority() { return ALSoundBase::GetPriority(); }
void pragma::audio::SALSound::SetPriority(uint32_t priority)
{
	ALSoundBase::SetPriority(priority);
	SendEvent(NetEvent::SetPriority, [priority](NetPacket &p) { p->Write<uint32_t>(priority); });
}
void pragma::audio::SALSound::SetOrientation(const Vector3 &at, const Vector3 &up)
{
	ALSoundBase::SetOrientation(at, up);
	SendEvent(NetEvent::SetOrientation, [&at, &up](NetPacket &p) {
		p->Write<Vector3>(at);
		p->Write<Vector3>(up);
	});
}
std::pair<Vector3, Vector3> pragma::audio::SALSound::GetOrientation() const { return ALSoundBase::GetOrientation(); }
void pragma::audio::SALSound::SetDopplerFactor(float factor)
{
	ALSoundBase::SetDopplerFactor(factor);
	SendEvent(NetEvent::SetDopplerFactor, [factor](NetPacket &p) { p->Write<float>(factor); });
}
float pragma::audio::SALSound::GetDopplerFactor() const { return ALSoundBase::GetDopplerFactor(); }
void pragma::audio::SALSound::SetLeftStereoAngle(float ang)
{
	ALSoundBase::SetLeftStereoAngle(ang);
	SendEvent(NetEvent::SetLeftStereoAngle, [ang](NetPacket &p) { p->Write<float>(ang); });
}
float pragma::audio::SALSound::GetLeftStereoAngle() const { return ALSoundBase::GetLeftStereoAngle(); }
void pragma::audio::SALSound::SetRightStereoAngle(float ang)
{
	ALSoundBase::SetRightStereoAngle(ang);
	SendEvent(NetEvent::SetRightStereoAngle, [ang](NetPacket &p) { p->Write<float>(ang); });
}
float pragma::audio::SALSound::GetRightStereoAngle() const { return ALSoundBase::GetRightStereoAngle(); }
void pragma::audio::SALSound::SetAirAbsorptionFactor(float factor)
{
	ALSoundBase::SetAirAbsorptionFactor(factor);
	SendEvent(NetEvent::SetAirAbsorptionFactor, [factor](NetPacket &p) { p->Write<float>(factor); });
}
float pragma::audio::SALSound::GetAirAbsorptionFactor() const { return ALSoundBase::GetAirAbsorptionFactor(); }
void pragma::audio::SALSound::SetGainAuto(bool directHF, bool send, bool sendHF)
{
	ALSoundBase::SetGainAuto(directHF, send, sendHF);
	SendEvent(NetEvent::SetAirAbsorptionFactor, [directHF, send, sendHF](NetPacket &p) {
		p->Write<float>(directHF);
		p->Write<float>(send);
		p->Write<float>(sendHF);
	});
}
std::tuple<bool, bool, bool> pragma::audio::SALSound::GetGainAuto() const { return ALSoundBase::GetGainAuto(); }
void pragma::audio::SALSound::SetDirectFilter(const SoundEffectParams &params)
{
	ALSoundBase::SetDirectFilter(params);
	SendEvent(NetEvent::SetAirAbsorptionFactor, [&params](NetPacket &p) {
		p->Write<float>(params.gain);
		p->Write<float>(params.gainHF);
		p->Write<float>(params.gainLF);
	});
}
const pragma::audio::SoundEffectParams &pragma::audio::SALSound::GetDirectFilter() const { return ALSoundBase::GetDirectFilter(); }
bool pragma::audio::SALSound::AddEffect(const std::string &effectName, const SoundEffectParams &params)
{
	SendEvent(NetEvent::AddEffect, [&effectName, &params](NetPacket &p) {
		p->WriteString(effectName);
		p->Write<float>(params.gain);
		p->Write<float>(params.gainHF);
		p->Write<float>(params.gainLF);
	});
	return true;
}
void pragma::audio::SALSound::RemoveEffect(const std::string &effectName)
{
	SendEvent(NetEvent::RemoveEffect, [&effectName](NetPacket &p) { p->WriteString(effectName); });
}
void pragma::audio::SALSound::SetEffectParameters(const std::string &effectName, const SoundEffectParams &params)
{
	SendEvent(NetEvent::SetEffectParameters, [&effectName, &params](NetPacket &p) {
		p->WriteString(effectName);
		p->Write<float>(params.gain);
		p->Write<float>(params.gainHF);
		p->Write<float>(params.gainLF);
	});
}

void pragma::audio::SALSound::SetEntityMapIndex(uint32_t idx)
{
	SendEvent(NetEvent::SetEntityMapIndex, [&idx](NetPacket &p) { p->Write<uint32_t>(idx); });
}
