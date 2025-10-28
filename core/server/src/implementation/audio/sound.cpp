// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_server.h"

module pragma.server.audio.sound;

import pragma.server.server_state;

SALSoundBase::SALSoundBase(bool bShared) : m_bShared(bShared) {}
bool SALSoundBase::IsShared() const { return m_bShared; }
void SALSoundBase::SetShared(bool b) { m_bShared = b; }

SALSoundBase *SALSound::GetBase(ALSound *snd) { return dynamic_cast<SALSoundBase *>(snd); } //(snd->IsSoundScript() == false) ? static_cast<SALSoundBase*>(static_cast<SALSound*>(snd)) : static_cast<SALSoundBase*>(static_cast<SALSoundScript*>(snd));}

#pragma warning(disable : 4056)
SALSound::SALSound(NetworkState *nw, unsigned int idx, float duration, const std::string &soundName, pragma::audio::ALCreateFlags createFlags) : ALSound(nw), SALSoundBase(umath::is_flag_set(createFlags, pragma::audio::ALCreateFlags::DontTransmit) == false), m_soundName {soundName}, m_createFlags {createFlags}
{
	m_index = idx;
	m_duration = duration;
}
#pragma warning(default : 4056)

SALSound::~SALSound()
{
	SendEvent(NetEvent::SetIndex, [](NetPacket &p) { p->Write<uint32_t>(static_cast<uint32_t>(0)); });
	Game *game = ServerState::Get()->GetGameState();
	if(game == NULL)
		return;
	//for(int i=0;i<m_luaCallbacks.size();i++)
	//	lua_removereference(game->GetLuaState(),m_luaCallbacks[i]);
}

const std::string &SALSound::GetSoundName() const { return m_soundName; }
pragma::audio::ALCreateFlags SALSound::GetCreateFlags() const { return m_createFlags; }

void SALSound::SendEvent(NetEvent evId, const std::function<void(NetPacket &)> &write, bool bUDP) const
{
	if(IsShared() == false)
		return;
	NetPacket p;
	p->Write<uint8_t>(umath::to_integral(evId));
	p->Write<unsigned int>(this->GetIndex());
	if(write != nullptr)
		write(p);
	if(bUDP == true)
		ServerState::Get()->SendPacket("snd_ev", p, pragma::networking::Protocol::FastUnreliable);
	else
		ServerState::Get()->SendPacket("snd_ev", p, pragma::networking::Protocol::SlowReliable);
}

void SALSound::SetState(ALState state)
{
	auto old = GetState();
	if(state != old) {
		CallCallbacks<void, ALState, ALState>("OnStateChanged", old, state);
		CallLuaCallbacks<void, int32_t, int32_t>("OnStateChanged", umath::to_integral(old), umath::to_integral(state));
	}
	ALSoundBase::SetState(state);
}

unsigned int SALSound::GetIndex() const { return m_index; }

void SALSound::FadeIn(float time)
{
	float gain = GetGain();
	SetGain(0);
	CancelFade();
	if(!IsPlaying())
		Play();
	m_fade = std::unique_ptr<SoundFade>(new SoundFade(true, ServerState::Get()->RealTime(), time, gain));

	SendEvent(NetEvent::FadeIn, [&time](NetPacket &p) { p->Write<float>(time); });
}

void SALSound::FadeOut(float time)
{
	if(!IsPlaying())
		return;
	float gain = GetGain();
	CancelFade();
	m_fade = std::unique_ptr<SoundFade>(new SoundFade(false, ServerState::Get()->RealTime(), time, gain));

	SendEvent(NetEvent::FadeOut, [&time](NetPacket &p) { p->Write<float>(time); });
}

void SALSound::Update()
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

void SALSound::PostUpdate() { ALSound::PostUpdate(); }

void SALSound::Play()
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

void SALSound::Stop()
{
	SetState(ALState::Stopped);
	SendEvent(NetEvent::Stop);
}

void SALSound::Pause()
{
	SetState(ALState::Paused);
	SendEvent(NetEvent::Pause);
}

void SALSound::Rewind()
{
	m_offset = 0;
	SetState(ALState::Initial);
	SendEvent(NetEvent::Rewind);
}

void SALSound::SetOffset(float offset)
{
	offset = std::min(offset, 1.f);
	m_offset = offset;
	SendEvent(NetEvent::SetOffset, [&offset](NetPacket &p) { p->Write<float>(offset); });
}

float SALSound::GetOffset() const { return m_offset; }
void SALSound::SetPitch(float pitch)
{
	ALSoundBase::SetPitch(pitch);
	SendEvent(NetEvent::SetPitch, [&pitch](NetPacket &p) { p->Write<float>(pitch); });
}

float SALSound::GetPitch() const { return ALSoundBase::GetPitch(); }
void SALSound::SetRange(float start, float end)
{
	ALSound::SetRange(start, end);
	SendEvent(NetEvent::SetRange, [&start, &end](NetPacket &p) {
		p->Write<float>(start);
		p->Write<float>(end);
	});
}
void SALSound::ClearRange()
{
	ALSound::ClearRange();
	SendEvent(NetEvent::ClearRange);
}
void SALSound::SetLooping(bool loop)
{
	ALSoundBase::SetLooping(loop);
	SendEvent(NetEvent::SetLooping, [&loop](NetPacket &p) { p->Write<bool>(loop); });
}

ALState SALSound::GetState() const { return ALSoundBase::GetState(); }

bool SALSound::IsLooping() const { return ALSoundBase::IsLooping(); }
bool SALSound::IsPlaying() const { return ALSoundBase::IsPlaying(); }
bool SALSound::IsPaused() const { return ALSoundBase::IsPaused(); }
bool SALSound::IsStopped() const { return ALSoundBase::IsStopped(); }
void SALSound::SetGain(float gain)
{
	ALSoundBase::SetGain(gain);
	SendEvent(NetEvent::SetGain, [&gain](NetPacket &p) { p->Write<float>(gain); });
}

float SALSound::GetGain() const { return ALSoundBase::GetGain(); }
void SALSound::SetPosition(const Vector3 &pos, bool bDontTransmit)
{
	ALSoundBase::SetPosition(pos);
	if(bDontTransmit == true)
		return;
	SendEvent(NetEvent::SetPos, [&pos](NetPacket &p) { nwm::write_vector(p, pos); });
}
void SALSound::SetPosition(const Vector3 &pos) { SetPosition(pos, false); }

Vector3 SALSound::GetPosition() const
{
	if(m_hSourceEntity.valid()) {
		auto pTrComponent = m_hSourceEntity.get()->GetTransformComponent();
		if(pTrComponent != nullptr)
			return pTrComponent->GetPosition();
	}
	return ALSoundBase::GetPosition();
}
void SALSound::SetVelocity(const Vector3 &vel, bool bDontTransmit)
{
	ALSoundBase::SetVelocity(vel);
	if(bDontTransmit == true)
		return;
	SendEvent(NetEvent::SetVelocity, [&vel](NetPacket &p) { nwm::write_vector(p, vel); });
}
void SALSound::SetVelocity(const Vector3 &vel) { SetVelocity(vel, false); }

Vector3 SALSound::GetVelocity() const { return ALSoundBase::GetVelocity(); }
void SALSound::SetDirection(const Vector3 &dir, bool bDontTransmit)
{
	ALSoundBase::SetDirection(dir);
	if(bDontTransmit == true)
		return;
	SendEvent(NetEvent::SetDirection, [&dir](NetPacket &p) { nwm::write_vector(p, dir); });
}
void SALSound::SetDirection(const Vector3 &dir) { SetDirection(dir, false); }

Vector3 SALSound::GetDirection() const { return ALSoundBase::GetDirection(); }
void SALSound::SetRelative(bool b)
{
	ALSoundBase::SetRelative(b);
	SendEvent(NetEvent::SetRelative, [&b](NetPacket &p) { p->Write<bool>(b); });
}

bool SALSound::IsRelative() const { return ALSoundBase::IsRelative(); }
float SALSound::GetDuration() const { return ALSoundBase::GetDuration(); }
float SALSound::GetReferenceDistance() const { return ALSoundBase::GetReferenceDistance(); }
void SALSound::SetReferenceDistance(float dist)
{
	ALSoundBase::SetReferenceDistance(dist);
	SendEvent(NetEvent::SetReferenceDistance, [&dist](NetPacket &p) { p->Write<float>(dist); });
}

float SALSound::GetRolloffFactor() const { return ALSoundBase::GetRolloffFactor(); }
void SALSound::SetRolloffFactor(float rolloff)
{
	ALSoundBase::SetRolloffFactor(rolloff);
	SendEvent(NetEvent::SetRolloffFactor, [&rolloff](NetPacket &p) { p->Write<float>(rolloff); });
}
float SALSound::GetRoomRolloffFactor() const { return ALSoundBase::GetRoomRolloffFactor(); }
void SALSound::SetRoomRolloffFactor(float roomFactor)
{
	ALSoundBase::SetRoomRolloffFactor(roomFactor);
	SendEvent(NetEvent::SetRoomRolloffFactor, [&roomFactor](NetPacket &p) { p->Write<float>(roomFactor); });
}

float SALSound::GetMaxDistance() const { return ALSoundBase::GetMaxDistance(); }
void SALSound::SetMaxDistance(float dist)
{
	ALSoundBase::SetMaxDistance(dist);
	SendEvent(NetEvent::SetMaxDistance, [&dist](NetPacket &p) { p->Write<float>(dist); });
}

float SALSound::GetMinGain() const { return ALSoundBase::GetMinGain(); }
void SALSound::SetMinGain(float gain)
{
	ALSoundBase::SetMinGain(gain);
	SendEvent(NetEvent::SetMinGain, [&gain](NetPacket &p) { p->Write<float>(gain); });
}
float SALSound::GetMaxGain() const { return ALSoundBase::GetMaxGain(); }
void SALSound::SetMaxGain(float gain)
{
	ALSoundBase::SetMaxGain(gain);
	SendEvent(NetEvent::SetMaxGain, [&gain](NetPacket &p) { p->Write<float>(gain); });
}
float SALSound::GetInnerConeAngle() const { return ALSoundBase::GetInnerConeAngle(); }
void SALSound::SetInnerConeAngle(float ang)
{
	ALSoundBase::SetInnerConeAngle(ang);
	SendEvent(NetEvent::SetConeInnerAngle, [&ang](NetPacket &p) { p->Write<float>(ang); });
}
float SALSound::GetOuterConeAngle() const { return ALSoundBase::GetOuterConeAngle(); }
void SALSound::SetOuterConeAngle(float ang)
{
	ALSoundBase::SetOuterConeAngle(ang);
	SendEvent(NetEvent::SetConeOuterAngle, [&ang](NetPacket &p) { p->Write<float>(ang); });
}
float SALSound::GetOuterConeGain() const { return ALSoundBase::GetOuterConeGain(); }
void SALSound::SetOuterConeGain(float gain)
{
	ALSoundBase::SetOuterConeGain(gain);
	SendEvent(NetEvent::SetConeOuterGain, [&gain](NetPacket &p) { p->Write<float>(gain); });
}
float SALSound::GetOuterConeGainHF() const { return ALSoundBase::GetOuterConeGainHF(); }
void SALSound::SetOuterConeGainHF(float gain)
{
	ALSoundBase::SetOuterConeGainHF(gain);
	SendEvent(NetEvent::SetConeOuterGainHF, [&gain](NetPacket &p) { p->Write<float>(gain); });
}
void SALSound::SetType(pragma::audio::ALSoundType type)
{
	ALSound::SetType(type);
	SendEvent(NetEvent::SetType, [&type](NetPacket &p) { p->Write<pragma::audio::ALSoundType>(type); });
}

void SALSound::SetFlags(unsigned int flags)
{
	ALSound::SetFlags(flags);
	SendEvent(NetEvent::SetFlags, [&flags](NetPacket &p) { p->Write<unsigned int>(flags); });
}

void SALSound::SetSource(pragma::ecs::BaseEntity *ent)
{
	ALSound::SetSource(ent);
	SendEvent(NetEvent::SetSource, [&ent](NetPacket &p) { nwm::write_entity(p, ent); });
}

void SALSound::SetFadeInDuration(float t)
{
	ALSound::SetFadeInDuration(t);
	SendEvent(NetEvent::SetFadeInDuration, [&t](NetPacket &p) { p->Write<float>(t); });
}
void SALSound::SetFadeOutDuration(float t)
{
	ALSound::SetFadeOutDuration(t);
	SendEvent(NetEvent::SetFadeOutDuration, [&t](NetPacket &p) { p->Write<float>(t); });
}

uint32_t SALSound::GetPriority() { return ALSoundBase::GetPriority(); }
void SALSound::SetPriority(uint32_t priority)
{
	ALSoundBase::SetPriority(priority);
	SendEvent(NetEvent::SetPriority, [priority](NetPacket &p) { p->Write<uint32_t>(priority); });
}
void SALSound::SetOrientation(const Vector3 &at, const Vector3 &up)
{
	ALSoundBase::SetOrientation(at, up);
	SendEvent(NetEvent::SetOrientation, [&at, &up](NetPacket &p) {
		p->Write<Vector3>(at);
		p->Write<Vector3>(up);
	});
}
std::pair<Vector3, Vector3> SALSound::GetOrientation() const { return ALSoundBase::GetOrientation(); }
void SALSound::SetDopplerFactor(float factor)
{
	ALSoundBase::SetDopplerFactor(factor);
	SendEvent(NetEvent::SetDopplerFactor, [factor](NetPacket &p) { p->Write<float>(factor); });
}
float SALSound::GetDopplerFactor() const { return ALSoundBase::GetDopplerFactor(); }
void SALSound::SetLeftStereoAngle(float ang)
{
	ALSoundBase::SetLeftStereoAngle(ang);
	SendEvent(NetEvent::SetLeftStereoAngle, [ang](NetPacket &p) { p->Write<float>(ang); });
}
float SALSound::GetLeftStereoAngle() const { return ALSoundBase::GetLeftStereoAngle(); }
void SALSound::SetRightStereoAngle(float ang)
{
	ALSoundBase::SetRightStereoAngle(ang);
	SendEvent(NetEvent::SetRightStereoAngle, [ang](NetPacket &p) { p->Write<float>(ang); });
}
float SALSound::GetRightStereoAngle() const { return ALSoundBase::GetRightStereoAngle(); }
void SALSound::SetAirAbsorptionFactor(float factor)
{
	ALSoundBase::SetAirAbsorptionFactor(factor);
	SendEvent(NetEvent::SetAirAbsorptionFactor, [factor](NetPacket &p) { p->Write<float>(factor); });
}
float SALSound::GetAirAbsorptionFactor() const { return ALSoundBase::GetAirAbsorptionFactor(); }
void SALSound::SetGainAuto(bool directHF, bool send, bool sendHF)
{
	ALSoundBase::SetGainAuto(directHF, send, sendHF);
	SendEvent(NetEvent::SetAirAbsorptionFactor, [directHF, send, sendHF](NetPacket &p) {
		p->Write<float>(directHF);
		p->Write<float>(send);
		p->Write<float>(sendHF);
	});
}
std::tuple<bool, bool, bool> SALSound::GetGainAuto() const { return ALSoundBase::GetGainAuto(); }
void SALSound::SetDirectFilter(const SoundEffectParams &params)
{
	ALSoundBase::SetDirectFilter(params);
	SendEvent(NetEvent::SetAirAbsorptionFactor, [&params](NetPacket &p) {
		p->Write<float>(params.gain);
		p->Write<float>(params.gainHF);
		p->Write<float>(params.gainLF);
	});
}
const SoundEffectParams &SALSound::GetDirectFilter() const { return ALSoundBase::GetDirectFilter(); }
bool SALSound::AddEffect(const std::string &effectName, const SoundEffectParams &params)
{
	SendEvent(NetEvent::AddEffect, [&effectName, &params](NetPacket &p) {
		p->WriteString(effectName);
		p->Write<float>(params.gain);
		p->Write<float>(params.gainHF);
		p->Write<float>(params.gainLF);
	});
	return true;
}
void SALSound::RemoveEffect(const std::string &effectName)
{
	SendEvent(NetEvent::RemoveEffect, [&effectName](NetPacket &p) { p->WriteString(effectName); });
}
void SALSound::SetEffectParameters(const std::string &effectName, const SoundEffectParams &params)
{
	SendEvent(NetEvent::SetEffectParameters, [&effectName, &params](NetPacket &p) {
		p->WriteString(effectName);
		p->Write<float>(params.gain);
		p->Write<float>(params.gainHF);
		p->Write<float>(params.gainLF);
	});
}

void SALSound::SetEntityMapIndex(uint32_t idx)
{
	SendEvent(NetEvent::SetEntityMapIndex, [&idx](NetPacket &p) { p->Write<uint32_t>(idx); });
}
