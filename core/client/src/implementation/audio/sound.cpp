// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"
#include <pragma/networking/nwm_util.h>
#include <pragma/logging.hpp>
#include "pragma/console/c_cvar.h"
#include "pragma/networking/c_nwm_util.h"
#include "luasystem.h"
#include "alsound_types.hpp";
#include <pragma/audio/alsound_type.h>
#include <pragma/entities/components/base_transform_component.hpp>

module pragma.client.audio;

import :sound;

import pragma.client.client_state;
import pragma.client.engine;

extern CEngine *c_engine;
extern ClientState *client;
DLLCLIENT void NET_cl_snd_precache(NetPacket packet)
{
	std::string snd = packet->ReadString();
	auto mode = packet->Read<uint8_t>();
	client->PrecacheSound(snd, static_cast<ALChannel>(mode));
}

DLLCLIENT void NET_cl_snd_create(NetPacket packet)
{
	std::string snd = packet->ReadString();
	auto type = packet->Read<ALSoundType>();
	unsigned int idx = packet->Read<unsigned int>();
	auto createFlags = packet->Read<ALCreateFlags>();
	auto as = client->CreateSound(snd, ALSoundType::Generic, createFlags);
	if(as == nullptr)
		return;
	client->IndexSound(as, idx);

	auto fullUpdate = packet->Read<bool>();
	if(fullUpdate == false)
		return;
	auto state = packet->Read<ALState>();

	as->SetOffset(packet->Read<float>());
	as->SetPitch(packet->Read<float>());
	as->SetLooping(packet->Read<bool>());
	as->SetGain(packet->Read<float>());
	as->SetPosition(packet->Read<Vector3>());
	as->SetVelocity(packet->Read<Vector3>());
	as->SetDirection(packet->Read<Vector3>());
	as->SetRelative(packet->Read<bool>());
	as->SetReferenceDistance(packet->Read<float>());
	as->SetRolloffFactor(packet->Read<float>());
	as->SetRoomRolloffFactor(packet->Read<float>());
	as->SetMaxDistance(packet->Read<float>());
	as->SetMinGain(packet->Read<float>());
	as->SetMaxGain(packet->Read<float>());
	as->SetInnerConeAngle(packet->Read<float>());
	as->SetOuterConeAngle(packet->Read<float>());
	as->SetOuterConeGain(packet->Read<float>());
	as->SetOuterConeGainHF(packet->Read<float>());
	as->SetFlags(packet->Read<uint32_t>());

	auto hasRange = packet->Read<bool>();
	if(hasRange) {
		auto start = packet->Read<float>();
		auto end = packet->Read<float>();
		as->SetRange(start, end);
	}

	as->SetFadeInDuration(packet->Read<float>());
	as->SetFadeOutDuration(packet->Read<float>());
	as->SetPriority(packet->Read<uint32_t>());

	auto at = packet->Read<Vector3>();
	auto up = packet->Read<Vector3>();
	as->SetOrientation(at, up);

	as->SetDopplerFactor(packet->Read<float>());
	as->SetLeftStereoAngle(packet->Read<float>());
	as->SetRightStereoAngle(packet->Read<float>());

	as->SetAirAbsorptionFactor(packet->Read<float>());

	auto directHF = packet->Read<bool>();
	auto send = packet->Read<bool>();
	auto sendHF = packet->Read<bool>();
	as->SetGainAuto(directHF, send, sendHF);

	auto gain = packet->Read<float>();
	auto gainHF = packet->Read<float>();
	auto gainLF = packet->Read<float>();
	as->SetDirectFilter({gain, gainHF, gainLF});

	std::weak_ptr<ALSound> wpSnd = as;
	nwm::read_unique_entity(packet, [wpSnd](BaseEntity *ent) {
		if(ent == nullptr || wpSnd.expired())
			return;
		wpSnd.lock()->SetSource(ent);
	});

	switch(state) {
	case ALState::Paused:
		as->Pause();
		break;
	case ALState::Playing:
		as->Play();
		break;
	case ALState::Stopped:
		as->Stop();
		break;
	case ALState::Initial:
		break;
	}
}

DLLCLIENT void NET_cl_snd_ev(NetPacket packet)
{
	unsigned char ev = packet->Read<unsigned char>();
	unsigned int idx = packet->Read<unsigned int>();
	std::shared_ptr<ALSound> as = client->GetSoundByIndex(idx);
	if(as == NULL)
		return;
	switch(static_cast<ALSound::NetEvent>(ev)) {
	case ALSound::NetEvent::Play:
		as->Play();
		break;
	case ALSound::NetEvent::Stop:
		as->Stop();
		break;
	case ALSound::NetEvent::Pause:
		as->Pause();
		break;
	case ALSound::NetEvent::Rewind:
		as->Rewind();
		break;
	case ALSound::NetEvent::SetOffset:
		{
			float offset = packet->Read<float>();
			as->SetOffset(offset);
			break;
		}
	case ALSound::NetEvent::SetPitch:
		{
			float pitch = packet->Read<float>();
			as->SetPitch(pitch);
			break;
		}
	case ALSound::NetEvent::SetLooping:
		{
			bool loop = packet->Read<bool>();
			as->SetLooping(loop);
			break;
		}
	case ALSound::NetEvent::SetGain:
		{
			float gain = packet->Read<float>();
			as->SetGain(gain);
			break;
		}
	case ALSound::NetEvent::SetPos:
		{
			Vector3 pos = nwm::read_vector(packet);
			as->SetPosition(pos);
			break;
		}
	case ALSound::NetEvent::SetVelocity:
		{
			Vector3 vel = nwm::read_vector(packet);
			as->SetVelocity(vel);
			break;
		}
	case ALSound::NetEvent::SetDirection:
		{
			Vector3 dir = nwm::read_vector(packet);
			as->SetDirection(dir);
			break;
		}
	case ALSound::NetEvent::SetRelative:
		{
			bool relative = packet->Read<bool>();
			as->SetRelative(relative);
			break;
		}
	case ALSound::NetEvent::SetReferenceDistance:
		{
			float distRef = packet->Read<float>();
			as->SetReferenceDistance(distRef);
			break;
		}
	case ALSound::NetEvent::SetRolloffFactor:
		{
			float rolloff = packet->Read<float>();
			as->SetRolloffFactor(rolloff);
			break;
		}
	case ALSound::NetEvent::SetRoomRolloffFactor:
		{
			auto roomRolloff = packet->Read<float>();
			as->SetRoomRolloffFactor(roomRolloff);
			break;
		}
	case ALSound::NetEvent::SetMaxDistance:
		{
			float dist = packet->Read<float>();
			as->SetMaxDistance(dist);
			break;
		}
	case ALSound::NetEvent::SetMinGain:
		{
			float gain = packet->Read<float>();
			as->SetMinGain(gain);
			break;
		}
	case ALSound::NetEvent::SetMaxGain:
		{
			float gain = packet->Read<float>();
			as->SetMaxGain(gain);
			break;
		}
	case ALSound::NetEvent::SetConeInnerAngle:
		{
			float coneInnerAngle = packet->Read<float>();
			as->SetInnerConeAngle(coneInnerAngle);
			break;
		}
	case ALSound::NetEvent::SetConeOuterAngle:
		{
			float coneOuterAngle = packet->Read<float>();
			as->SetOuterConeAngle(coneOuterAngle);
			break;
		}
	case ALSound::NetEvent::SetConeOuterGain:
		{
			float coneOuterGain = packet->Read<float>();
			as->SetOuterConeGain(coneOuterGain);
			break;
		}
	case ALSound::NetEvent::SetConeOuterGainHF:
		{
			float coneOuterGainHF = packet->Read<float>();
			as->SetOuterConeGainHF(coneOuterGainHF);
			break;
		}
	case ALSound::NetEvent::SetFlags:
		{
			unsigned int flags = packet->Read<unsigned int>();
			as->SetFlags(flags);
			break;
		}
	case ALSound::NetEvent::SetType:
		{
			auto type = packet->Read<ALSoundType>();
			as->SetType(type);
			break;
		}
	case ALSound::NetEvent::SetSource:
		{
			auto *ent = nwm::read_entity(packet);
			as->SetSource(ent);
			break;
		}
	case ALSound::NetEvent::SetRange:
		{
			auto start = packet->Read<float>();
			auto end = packet->Read<float>();
			as->SetRange(start, end);
			break;
		}
	case ALSound::NetEvent::ClearRange:
		{
			as->ClearRange();
			break;
		}
	case ALSound::NetEvent::SetFadeInDuration:
		{
			auto t = packet->Read<float>();
			as->SetFadeInDuration(t);
			break;
		}
	case ALSound::NetEvent::SetFadeOutDuration:
		{
			auto t = packet->Read<float>();
			as->SetFadeOutDuration(t);
			break;
		}
	case ALSound::NetEvent::FadeIn:
		{
			auto t = packet->Read<float>();
			as->FadeIn(t);
			break;
		}
	case ALSound::NetEvent::FadeOut:
		{
			auto t = packet->Read<float>();
			as->FadeOut(t);
			break;
		}
	case ALSound::NetEvent::SetIndex:
		{
			auto idx = packet->Read<uint32_t>();
			CALSound::SetIndex(as.get(), idx);
			break;
		}
	case ALSound::NetEvent::SetPriority:
		{
			auto priority = packet->Read<uint32_t>();
			as->SetPriority(priority);
			break;
		}
	case ALSound::NetEvent::SetOrientation:
		{
			auto at = packet->Read<Vector3>();
			auto up = packet->Read<Vector3>();
			as->SetOrientation(at, up);
			break;
		}
	case ALSound::NetEvent::SetDopplerFactor:
		{
			auto factor = packet->Read<float>();
			as->SetDopplerFactor(factor);
			break;
		}
	case ALSound::NetEvent::SetLeftStereoAngle:
		{
			auto ang = packet->Read<float>();
			as->SetLeftStereoAngle(ang);
			break;
		}
	case ALSound::NetEvent::SetRightStereoAngle:
		{
			auto ang = packet->Read<float>();
			as->SetRightStereoAngle(ang);
			break;
		}
	case ALSound::NetEvent::SetAirAbsorptionFactor:
		{
			auto factor = packet->Read<float>();
			as->SetAirAbsorptionFactor(factor);
			break;
		}
	case ALSound::NetEvent::SetGainAuto:
		{
			auto directHF = packet->Read<float>();
			auto send = packet->Read<float>();
			auto sendHF = packet->Read<float>();
			as->SetGainAuto(directHF, send, sendHF);
			break;
		}
	case ALSound::NetEvent::SetDirectFilter:
		{
			auto gain = packet->Read<float>();
			auto gainHF = packet->Read<float>();
			auto gainLF = packet->Read<float>();
			as->SetDirectFilter({gain, gainHF, gainLF});
			break;
		}
	case ALSound::NetEvent::AddEffect:
		{
			auto effectName = packet->ReadString();
			auto gain = packet->Read<float>();
			auto gainHF = packet->Read<float>();
			auto gainLF = packet->Read<float>();
			as->AddEffect(effectName, {gain, gainHF, gainLF});
			break;
		}
	case ALSound::NetEvent::RemoveEffect:
		{
			auto effectName = packet->ReadString();
			as->RemoveEffect(effectName);
			break;
		}
	case ALSound::NetEvent::SetEffectParameters:
		{
			auto effectName = packet->ReadString();
			auto gain = packet->Read<float>();
			auto gainHF = packet->Read<float>();
			auto gainLF = packet->Read<float>();
			as->SetEffectParameters(effectName, {gain, gainHF, gainLF});
			break;
		}
	case ALSound::NetEvent::SetEntityMapIndex:
		{
			auto idx = packet->Read<uint32_t>();
			//as->SetIdentifier("world_sound" +std::to_string(idx)); // Has to correspond to identifier in c_game_audio.cpp
			break;
		}
	default:
		{
			spdlog::warn("Unhandled sound net event {}!", ev);
			break;
		}
	}
}

decltype(CALSound::s_svIndexedSounds) CALSound::s_svIndexedSounds = {};
ALSound *CALSound::FindByServerIndex(uint32_t idx)
{
	auto it = s_svIndexedSounds.find(idx);
	if(it == s_svIndexedSounds.end())
		return nullptr;
	return it->second.lock().get();
}

CALSound::CALSound(NetworkState *nw, const al::PSoundChannel &channel) : ALSound(nw), al::SoundSource {channel}
{
	UpdateVolume();
	RegisterCallback<void, std::reference_wrapper<float>>("UpdateGain");
}

CALSound::~CALSound() {}

void CALSound::OnRelease()
{
	al::SoundSource::OnRelease();
	ALSound::OnRelease();
	auto it = s_svIndexedSounds.find(GetIndex());
	if(it != s_svIndexedSounds.end())
		s_svIndexedSounds.erase(it);
	for(auto it = s_svIndexedSounds.begin(); it != s_svIndexedSounds.end();) {
		if(it->second.expired() == true)
			it = s_svIndexedSounds.erase(it);
		else
			++it;
	}
}

void CALSound::Terminate()
{
	if(m_bTerminated == true)
		return;
	Stop();
	m_bTerminated = true;
}

static_assert(sizeof(al::EffectParams) == sizeof(ALSound::EffectParams));
bool CALSound::AddEffect(al::IEffect &effect, const EffectParams &params) { return (*this)->AddEffect(effect, reinterpret_cast<const al::EffectParams &>(params)); }
bool CALSound::AddEffect(al::IEffect &effect, uint32_t &slotId, const EffectParams &params) { return (*this)->AddEffect(effect, slotId, reinterpret_cast<const al::EffectParams &>(params)); }
bool CALSound::AddEffect(al::IEffect &effect, float gain) { return (*this)->AddEffect(effect, gain); }
bool CALSound::AddEffect(al::IEffect &effect, uint32_t &slotId, float gain) { return (*this)->AddEffect(effect, slotId, gain); }
void CALSound::RemoveEffect(al::IEffect &effect) { (*this)->RemoveEffect(effect); }
void CALSound::RemoveEffect(uint32_t slotId) { (*this)->RemoveEffect(slotId); }

void CALSound::SetPitchModifier(float mod)
{
	m_modPitch = mod;
	UpdatePitch();
}
float CALSound::GetPitchModifier() const { return m_modPitch; }
void CALSound::SetVolumeModifier(float mod)
{
	m_modVol = mod;
	UpdateVolume();
}
float CALSound::GetVolumeModifier() const { return m_modVol; }

static auto cvAlwaysPlay = GetClientConVar("cl_audio_always_play");
void CALSound::UpdateVolume()
{
	// TODO: CALSound::GetEffectiveGain
	if(m_bTerminated == true)
		return;
	auto gain = m_gain;
	if(c_engine->IsWindowFocused() == false && cvAlwaysPlay->GetBool() == false)
		gain = 0.f;
	else {
		gain *= GetVolumeModifier();
		auto &volumes = client->GetSoundVolumes();
		auto minGain = 1.f;
		for(auto it = volumes.begin(); it != volumes.end(); ++it) {
			if((umath::to_integral(m_type) & umath::to_integral(it->first)) != 0) {
				if(it->second < minGain)
					minGain = it->second;
			}
		}
		gain *= minGain;
		gain *= client->GetMasterSoundVolume();
	}
	CallCallbacks<void, std::reference_wrapper<float>>("UpdateGain", std::ref<float>(gain));
	(*this)->SetGain(gain);
}

float CALSound::GetMaxAudibleDistance() const { return (*this)->GetMaxAudibleDistance(); }

void CALSound::UpdatePitch()
{
	if(m_bTerminated == true)
		return;
	float pitch = GetPitch();
	pitch *= GetPitchModifier();
	(*this)->SetPitch(pitch);
}

unsigned int CALSound::GetIndex() const { return m_index; }

void CALSound::SetIndex(ALSound *snd, uint32_t idx)
{
	auto it = s_svIndexedSounds.find(idx);
	if(it != s_svIndexedSounds.end())
		s_svIndexedSounds.erase(it);

	snd->SetIndex(idx);
	if(idx == 0)
		return;
	s_svIndexedSounds.insert(decltype(s_svIndexedSounds)::value_type(idx, snd->shared_from_this()));
}

void CALSound::Update()
{
	if(m_bTerminated == true)
		return;
	al::SoundSource::Update();
	auto old = GetState();
	UpdateState();
	if(IsStopped() == true) {
		CancelFade();
		CheckStateChange(old);
	}
	else if(m_fade != nullptr) {
		auto t = client->RealTime() - m_fade->start;
		if(t >= m_fade->duration)
			CancelFade();
		else {
			auto gain = (t / m_fade->duration) * m_fade->gain;
			if(m_fade->fadein)
				SetGain(gain);
			else
				SetGain(m_fade->gain - gain);
		}
	}
}

void CALSound::PostUpdate()
{
	if(m_bTerminated == true)
		return;
	ALSound::PostUpdate();
}

ALState CALSound::GetState() const
{
	if(IsPlaying())
		return ALState::Playing;
	if(IsStopped())
		return ALState::Stopped;
	if(IsPaused())
		return ALState::Paused;
	// TODO
	return ALState::Initial; //static_cast<ALState>(m_state);
}

void CALSound::FadeIn(float time)
{
	float gain = GetGain();
	SetGain(0);
	CancelFade();
	if(!IsPlaying())
		Play();
	m_fade = std::unique_ptr<SoundFade>(new SoundFade(true, client->RealTime(), time, gain));
}

void CALSound::FadeOut(float time)
{
	if(!IsPlaying())
		return;
	float gain = GetGain();
	CancelFade();
	m_fade = std::unique_ptr<SoundFade>(new SoundFade(false, client->RealTime(), time, gain));
}

void CALSound::UpdateState()
{
	if(m_bTerminated == true)
		return;
}

void CALSound::Play()
{
	if(m_bTerminated == true)
		return;
	CancelFade();
	auto old = GetState();

	auto bPaused = (GetState() == ALState::Paused) ? true : false;
	if(bPaused == false)
		InitRange();
	if(bPaused == false) {
		try {
			(*this)->Play();
		}
		catch(const std::runtime_error &err) {
			spdlog::warn("Unable to play sound {}: {}", GetIndex(), err.what());
			return;
		}
	}
	else
		(*this)->Resume();
	UpdateState();
	CheckStateChange(old);
	if(m_tFadeIn > 0.f)
		FadeIn(m_tFadeIn);
}

void CALSound::Stop()
{
	if(m_bTerminated == true)
		return;
	CancelFade();
	auto old = GetState();
	(*this)->Stop();
	UpdateState();
	CheckStateChange(old);
}

void CALSound::Pause()
{
	if(m_bTerminated == true)
		return;
	CancelFade();
	auto old = GetState();
	(*this)->Pause();
	UpdateState();
	CheckStateChange(old);
}

void CALSound::Rewind()
{
	if(m_bTerminated == true)
		return;
	CancelFade();
	auto old = GetState();
	SetOffset(0.f);
	UpdateState();
	CheckStateChange(old);
}

void CALSound::SetOffset(float offset)
{
	if(m_bTerminated == true)
		return;
	(*this)->SetOffset(offset);
}

float CALSound::GetOffset() const
{
	if(m_bTerminated == true)
		return 0.f;
	return (*this)->GetOffset();
}

void CALSound::SetPitch(float pitch)
{
	m_pitch = pitch;
	UpdatePitch();
}

float CALSound::GetPitch() const { return m_pitch; }

void CALSound::SetLooping(bool loop)
{
	if(m_bTerminated == true)
		return;
	(*this)->SetLooping(loop);
}

bool CALSound::IsIdle() const
{
	if(GetIndex() > 0)
		return false; // This is a server-side sound, keep it around until server-side representation is removed
	return al::SoundSource::IsIdle();
}
bool CALSound::IsLooping() const
{
	if(m_bTerminated == true)
		return false;
	return (*this)->IsLooping();
}
bool CALSound::IsPlaying() const
{
	if(m_bTerminated == true)
		return false;
	return (*this)->IsPlaying();
}
bool CALSound::IsPaused() const
{
	if(m_bTerminated == true)
		return false;
	return (*this)->IsPaused();
}
bool CALSound::IsStopped() const
{
	if(m_bTerminated == true)
		return false;
	return (*this)->IsStopped();
}
void CALSound::SetGain(float gain)
{
	m_gain = gain;
	UpdateVolume();
}
float CALSound::GetGain() const
{
	if(m_bTerminated == true)
		return 0.f;
	return glm::clamp((*this)->GetGain(), GetMinGain(), GetMaxGain());
}
void CALSound::SetPosition(const Vector3 &pos)
{
	if(m_bTerminated == true)
		return;
	(*this)->SetPosition(pos);
}
Vector3 CALSound::GetPosition() const
{
	if(m_bTerminated == true)
		return Vector3(0.f, 0.f, 0.f);
	if(m_hSourceEntity.valid()) {
		auto pTrComponent = m_hSourceEntity.get()->GetTransformComponent();
		if(pTrComponent != nullptr)
			return pTrComponent->GetPosition();
	}
	return (*this)->GetPosition();
}
void CALSound::SetVelocity(const Vector3 &vel)
{
	if(m_bTerminated == true)
		return;
	(*this)->SetVelocity(vel);
}
Vector3 CALSound::GetVelocity() const
{
	if(m_bTerminated == true)
		return Vector3(0.f, 0.f, 0.f);
	return (*this)->GetVelocity();
}
void CALSound::SetDirection(const Vector3 &dir)
{
	if(m_bTerminated == true)
		return;
	(*this)->SetDirection(dir);
}
Vector3 CALSound::GetDirection() const
{
	if(m_bTerminated == true)
		return Vector3(0.f, 0.f, 0.f);
	return (*this)->GetDirection();
}
void CALSound::SetRelative(bool b)
{
	if(m_bTerminated == true)
		return;
	(*this)->SetRelative(b);
}
bool CALSound::IsRelative() const
{
	if(m_bTerminated == true)
		return false;
	return (*this)->IsRelative();
}
float CALSound::GetDuration() const
{
	if(m_bTerminated == true)
		return 0.f;
	return (*this)->GetDuration();
}
float CALSound::GetReferenceDistance() const
{
	if(m_bTerminated == true)
		return 0.f;
	return (*this)->GetReferenceDistance();
}
void CALSound::SetReferenceDistance(float dist)
{
	if(m_bTerminated == true)
		return;
	(*this)->SetReferenceDistance(dist);
}
void CALSound::SetRoomRolloffFactor(float roomFactor)
{
	if(m_bTerminated == true)
		return;
	(*this)->SetRoomRolloffFactor(roomFactor);
}
float CALSound::GetRolloffFactor() const
{
	if(m_bTerminated == true)
		return 0.f;
	return (*this)->GetRolloffFactor();
}
float CALSound::GetRoomRolloffFactor() const
{
	if(m_bTerminated == true)
		return 0.f;
	return (*this)->GetRoomRolloffFactor();
}
void CALSound::SetRolloffFactor(float rolloff)
{
	if(m_bTerminated == true)
		return;
	(*this)->SetRolloffFactors(rolloff);
}
float CALSound::GetMaxDistance() const
{
	if(m_bTerminated == true)
		return 0.f;
	return (*this)->GetMaxDistance();
}
void CALSound::SetMaxDistance(float dist)
{
	if(m_bTerminated == true)
		return;
	(*this)->SetMaxDistance(dist);
}
float CALSound::GetMinGain() const
{
	if(m_bTerminated == true)
		return 0.f;
	return (*this)->GetMinGain();
}
void CALSound::SetMinGain(float gain)
{
	if(m_bTerminated == true)
		return;
	(*this)->SetMinGain(gain);
}
float CALSound::GetMaxGain() const
{
	if(m_bTerminated == true)
		return 0.f;
	return (*this)->GetMaxGain();
}
void CALSound::SetMaxGain(float gain)
{
	if(m_bTerminated == true)
		return;
	(*this)->SetMaxGain(gain);
}
float CALSound::GetInnerConeAngle() const
{
	if(m_bTerminated == true)
		return 0.f;
	return (*this)->GetInnerConeAngle();
}
void CALSound::SetInnerConeAngle(float ang)
{
	if(m_bTerminated == true)
		return;
	(*this)->SetInnerConeAngle(ang);
}
float CALSound::GetOuterConeAngle() const
{
	if(m_bTerminated == true)
		return 0.f;
	return (*this)->GetOuterConeAngle();
}
void CALSound::SetOuterConeAngle(float ang)
{
	if(m_bTerminated == true)
		return;
	(*this)->SetOuterConeAngle(ang);
}
float CALSound::GetOuterConeGain() const
{
	if(m_bTerminated == true)
		return 0.f;
	return (*this)->GetOuterConeGain();
}
float CALSound::GetOuterConeGainHF() const
{
	if(m_bTerminated == true)
		return 0.f;
	return (*this)->GetOuterConeGainHF();
}
void CALSound::SetOuterConeGain(float gain)
{
	if(m_bTerminated == true)
		return;
	(*this)->SetOuterConeGain(gain);
}
void CALSound::SetOuterConeGainHF(float gain)
{
	if(m_bTerminated == true)
		return;
	(*this)->SetOuterConeGainHF(gain);
}

uint32_t CALSound::GetPriority()
{
	if(m_bTerminated == true)
		return 0;
	return (*this)->GetPriority();
}
void CALSound::SetPriority(uint32_t priority)
{
	if(m_bTerminated == true)
		return;
	(*this)->SetPriority(priority);
}
void CALSound::SetOrientation(const Vector3 &at, const Vector3 &up)
{
	if(m_bTerminated == true)
		return;
	(*this)->SetOrientation(at, up);
}
std::pair<Vector3, Vector3> CALSound::GetOrientation() const
{
	if(m_bTerminated == true)
		return {};
	return (*this)->GetOrientation();
}
void CALSound::SetDopplerFactor(float factor)
{
	if(m_bTerminated == true)
		return;
	(*this)->SetDopplerFactor(factor);
}
float CALSound::GetDopplerFactor() const
{
	if(m_bTerminated == true)
		return 0.f;
	return (*this)->GetDopplerFactor();
}
void CALSound::SetLeftStereoAngle(float ang)
{
	if(m_bTerminated == true)
		return;
	(*this)->SetLeftStereoAngle(ang);
}
float CALSound::GetLeftStereoAngle() const
{
	if(m_bTerminated == true)
		return 0.f;
	return (*this)->GetLeftStereoAngle();
}
void CALSound::SetRightStereoAngle(float ang)
{
	if(m_bTerminated == true)
		return;
	(*this)->SetRightStereoAngle(ang);
}
float CALSound::GetRightStereoAngle() const
{
	if(m_bTerminated == true)
		return 0.f;
	return (*this)->GetRightStereoAngle();
}
void CALSound::SetAirAbsorptionFactor(float factor)
{
	if(m_bTerminated == true)
		return;
	(*this)->SetAirAbsorptionFactor(factor);
}
float CALSound::GetAirAbsorptionFactor() const
{
	if(m_bTerminated == true)
		return 0.f;
	return (*this)->GetAirAbsorptionFactor();
}
void CALSound::SetGainAuto(bool directHF, bool send, bool sendHF)
{
	if(m_bTerminated == true)
		return;
	(*this)->SetGainAuto(directHF, send, sendHF);
}
std::tuple<bool, bool, bool> CALSound::GetGainAuto() const
{
	if(m_bTerminated == true)
		return {false, false, false};
	return (*this)->GetGainAuto();
}
void CALSound::SetDirectFilter(const EffectParams &params)
{
	if(m_bTerminated == true)
		return;
	(*this)->SetDirectFilter(reinterpret_cast<const al::EffectParams &>(params));
}
const ALSound::EffectParams &CALSound::GetDirectFilter() const
{
	if(m_bTerminated == true) {
		static ALSound::EffectParams params {};
		return params;
	}
	return reinterpret_cast<const ALSound::EffectParams &>((*this)->GetDirectFilter());
}
bool CALSound::AddEffect(const std::string &effectName, const EffectParams &params)
{
	auto effect = c_engine->GetAuxEffect(effectName);
	if(effect == nullptr)
		return false;
	return (*this)->AddEffect(*effect, reinterpret_cast<const al::EffectParams &>(params));
}
void CALSound::RemoveEffect(const std::string &effectName)
{
	auto effect = c_engine->GetAuxEffect(effectName);
	if(effect == nullptr)
		return;
	(*this)->RemoveEffect(*effect);
}
void CALSound::SetEffectParameters(const std::string &effectName, const EffectParams &params)
{
	auto effect = c_engine->GetAuxEffect(effectName);
	if(effect == nullptr)
		return;
	(*this)->SetEffectParameters(*effect, reinterpret_cast<const al::EffectParams &>(params));
}

void CALSound::SetType(ALSoundType type)
{
	if(m_bTerminated == true)
		return;
	ALSound::SetType(type);
	UpdateVolume();
}

REGISTER_CONVAR_CALLBACK_CL(cl_audio_always_play, [](NetworkState *, const ConVar &, bool, bool) { client->UpdateSoundVolume(); })
