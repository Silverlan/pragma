// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "definitions.hpp"

module pragma.client;

import :audio.sound;
import :client_state;
import :console.register_commands;
import :engine;
import :networking.util;

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

static_assert(sizeof(al::EffectParams) == sizeof(SoundEffectParams));
bool CALSound::AddEffect(al::IEffect &effect, const SoundEffectParams &params) { return (*this)->AddEffect(effect, reinterpret_cast<const al::EffectParams &>(params)); }
bool CALSound::AddEffect(al::IEffect &effect, uint32_t &slotId, const SoundEffectParams &params) { return (*this)->AddEffect(effect, slotId, reinterpret_cast<const al::EffectParams &>(params)); }
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
	if(pragma::get_cengine()->IsWindowFocused() == false && cvAlwaysPlay->GetBool() == false)
		gain = 0.f;
	else {
		gain *= GetVolumeModifier();
		auto *client = pragma::get_client_state();
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
		auto t = pragma::get_client_state()->RealTime() - m_fade->start;
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
	m_fade = std::unique_ptr<SoundFade>(new SoundFade(true, pragma::get_client_state()->RealTime(), time, gain));
}

void CALSound::FadeOut(float time)
{
	if(!IsPlaying())
		return;
	float gain = GetGain();
	CancelFade();
	m_fade = std::unique_ptr<SoundFade>(new SoundFade(false, pragma::get_client_state()->RealTime(), time, gain));
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
void CALSound::SetDirectFilter(const SoundEffectParams &params)
{
	if(m_bTerminated == true)
		return;
	(*this)->SetDirectFilter(reinterpret_cast<const al::EffectParams &>(params));
}
const SoundEffectParams &CALSound::GetDirectFilter() const
{
	if(m_bTerminated == true) {
		static SoundEffectParams params {};
		return params;
	}
	return reinterpret_cast<const SoundEffectParams &>((*this)->GetDirectFilter());
}
bool CALSound::AddEffect(const std::string &effectName, const SoundEffectParams &params)
{
	auto effect = pragma::get_cengine()->GetAuxEffect(effectName);
	if(effect == nullptr)
		return false;
	return (*this)->AddEffect(*effect, reinterpret_cast<const al::EffectParams &>(params));
}
void CALSound::RemoveEffect(const std::string &effectName)
{
	auto effect = pragma::get_cengine()->GetAuxEffect(effectName);
	if(effect == nullptr)
		return;
	(*this)->RemoveEffect(*effect);
}
void CALSound::SetEffectParameters(const std::string &effectName, const SoundEffectParams &params)
{
	auto effect = pragma::get_cengine()->GetAuxEffect(effectName);
	if(effect == nullptr)
		return;
	(*this)->SetEffectParameters(*effect, reinterpret_cast<const al::EffectParams &>(params));
}

void CALSound::SetType(pragma::audio::ALSoundType type)
{
	if(m_bTerminated == true)
		return;
	ALSound::SetType(type);
	UpdateVolume();
}
namespace {
	auto _ = pragma::console::client::register_variable_listener<bool>("cl_audio_always_play", +[](NetworkState *, const ConVar &, bool, bool) { pragma::get_client_state()->UpdateSoundVolume(); });
}
