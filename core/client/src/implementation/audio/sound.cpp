// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :audio.sound;
import :client_state;
import :console.register_commands;
import :engine;
import :networking.util;

decltype(pragma::audio::CALSound::s_svIndexedSounds) pragma::audio::CALSound::s_svIndexedSounds = {};
pragma::audio::ALSound *pragma::audio::CALSound::FindByServerIndex(uint32_t idx)
{
	auto it = s_svIndexedSounds.find(idx);
	if(it == s_svIndexedSounds.end())
		return nullptr;
	return it->second.lock().get();
}

std::shared_ptr<pragma::audio::CALSound> pragma::audio::CALSound::Create(NetworkState *nw, const PSoundChannel &channel)
{
	auto als = std::shared_ptr<CALSound> {new CALSound {nw, channel}, [](CALSound *snd) {
		                                      snd->OnRelease();
		                                      delete snd;
	                                      }};
	return als;
}

pragma::audio::CALSound::CALSound(NetworkState *nw, const PSoundChannel &channel) : ALSound(nw), SoundSource {channel}
{
	UpdateVolume();
	RegisterCallback<void, std::reference_wrapper<float>>("UpdateGain");
}

pragma::audio::CALSound::~CALSound() {}

void pragma::audio::CALSound::OnRelease()
{
	SoundSource::OnRelease();
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

void pragma::audio::CALSound::Terminate()
{
	if(m_bTerminated == true)
		return;
	Stop();
	m_bTerminated = true;
}

static_assert(sizeof(pragma::audio::EffectParams) == sizeof(pragma::audio::SoundEffectParams));
bool pragma::audio::CALSound::AddEffect(IEffect &effect, const SoundEffectParams &params) { return (*this)->AddEffect(effect, reinterpret_cast<const EffectParams &>(params)); }
bool pragma::audio::CALSound::AddEffect(IEffect &effect, uint32_t &slotId, const SoundEffectParams &params) { return (*this)->AddEffect(effect, slotId, reinterpret_cast<const EffectParams &>(params)); }
bool pragma::audio::CALSound::AddEffect(IEffect &effect, float gain) { return (*this)->AddEffect(effect, gain); }
bool pragma::audio::CALSound::AddEffect(IEffect &effect, uint32_t &slotId, float gain) { return (*this)->AddEffect(effect, slotId, gain); }
void pragma::audio::CALSound::RemoveEffect(IEffect &effect) { (*this)->RemoveEffect(effect); }
void pragma::audio::CALSound::RemoveEffect(uint32_t slotId) { (*this)->RemoveEffect(slotId); }

void pragma::audio::CALSound::SetPitchModifier(float mod)
{
	m_modPitch = mod;
	UpdatePitch();
}
float pragma::audio::CALSound::GetPitchModifier() const { return m_modPitch; }
void pragma::audio::CALSound::SetVolumeModifier(float mod)
{
	m_modVol = mod;
	UpdateVolume();
}
float pragma::audio::CALSound::GetVolumeModifier() const { return m_modVol; }

static auto cvAlwaysPlay = pragma::console::get_client_con_var("cl_audio_always_play");
void pragma::audio::CALSound::UpdateVolume()
{
	// TODO: pragma::audio::CALSound::GetEffectiveGain
	if(m_bTerminated == true)
		return;
	auto gain = m_gain;
	if(get_cengine()->IsWindowFocused() == false && cvAlwaysPlay->GetBool() == false)
		gain = 0.f;
	else {
		gain *= GetVolumeModifier();
		auto *client = get_client_state();
		auto &volumes = client->GetSoundVolumes();
		auto minGain = 1.f;
		for(auto it = volumes.begin(); it != volumes.end(); ++it) {
			if((math::to_integral(m_type) & math::to_integral(it->first)) != 0) {
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

float pragma::audio::CALSound::GetMaxAudibleDistance() const { return (*this)->GetMaxAudibleDistance(); }

void pragma::audio::CALSound::UpdatePitch()
{
	if(m_bTerminated == true)
		return;
	float pitch = GetPitch();
	pitch *= GetPitchModifier();
	(*this)->SetPitch(pitch);
}

unsigned int pragma::audio::CALSound::GetIndex() const { return m_index; }

void pragma::audio::CALSound::SetIndex(ALSound *snd, uint32_t idx)
{
	auto it = s_svIndexedSounds.find(idx);
	if(it != s_svIndexedSounds.end())
		s_svIndexedSounds.erase(it);

	snd->SetIndex(idx);
	if(idx == 0)
		return;
	s_svIndexedSounds.insert(decltype(s_svIndexedSounds)::value_type(idx, snd->shared_from_this()));
}

void pragma::audio::CALSound::Update()
{
	if(m_bTerminated == true)
		return;
	SoundSource::Update();
	auto old = GetState();
	UpdateState();
	if(IsStopped() == true) {
		CancelFade();
		CheckStateChange(old);
	}
	else if(m_fade != nullptr) {
		auto t = get_client_state()->RealTime() - m_fade->start;
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

void pragma::audio::CALSound::PostUpdate()
{
	if(m_bTerminated == true)
		return;
	ALSound::PostUpdate();
}

pragma::audio::ALState pragma::audio::CALSound::GetState() const
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

void pragma::audio::CALSound::FadeIn(float time)
{
	float gain = GetGain();
	SetGain(0);
	CancelFade();
	if(!IsPlaying())
		Play();
	m_fade = std::unique_ptr<SoundFade>(new SoundFade(true, get_client_state()->RealTime(), time, gain));
}

void pragma::audio::CALSound::FadeOut(float time)
{
	if(!IsPlaying())
		return;
	float gain = GetGain();
	CancelFade();
	m_fade = std::unique_ptr<SoundFade>(new SoundFade(false, get_client_state()->RealTime(), time, gain));
}

void pragma::audio::CALSound::UpdateState()
{
	if(m_bTerminated == true)
		return;
}

void pragma::audio::CALSound::Play()
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

void pragma::audio::CALSound::Stop()
{
	if(m_bTerminated == true)
		return;
	CancelFade();
	auto old = GetState();
	(*this)->Stop();
	UpdateState();
	CheckStateChange(old);
}

void pragma::audio::CALSound::Pause()
{
	if(m_bTerminated == true)
		return;
	CancelFade();
	auto old = GetState();
	(*this)->Pause();
	UpdateState();
	CheckStateChange(old);
}

void pragma::audio::CALSound::Rewind()
{
	if(m_bTerminated == true)
		return;
	CancelFade();
	auto old = GetState();
	SetOffset(0.f);
	UpdateState();
	CheckStateChange(old);
}

void pragma::audio::CALSound::SetOffset(float offset)
{
	if(m_bTerminated == true)
		return;
	(*this)->SetOffset(offset);
}

float pragma::audio::CALSound::GetOffset() const
{
	if(m_bTerminated == true)
		return 0.f;
	return (*this)->GetOffset();
}

void pragma::audio::CALSound::SetPitch(float pitch)
{
	m_pitch = pitch;
	UpdatePitch();
}

float pragma::audio::CALSound::GetPitch() const { return m_pitch; }

void pragma::audio::CALSound::SetLooping(bool loop)
{
	if(m_bTerminated == true)
		return;
	(*this)->SetLooping(loop);
}

bool pragma::audio::CALSound::IsIdle() const
{
	if(GetIndex() > 0)
		return false; // This is a server-side sound, keep it around until server-side representation is removed
	return SoundSource::IsIdle();
}
bool pragma::audio::CALSound::IsLooping() const
{
	if(m_bTerminated == true)
		return false;
	return (*this)->IsLooping();
}
bool pragma::audio::CALSound::IsPlaying() const
{
	if(m_bTerminated == true)
		return false;
	return (*this)->IsPlaying();
}
bool pragma::audio::CALSound::IsPaused() const
{
	if(m_bTerminated == true)
		return false;
	return (*this)->IsPaused();
}
bool pragma::audio::CALSound::IsStopped() const
{
	if(m_bTerminated == true)
		return false;
	return (*this)->IsStopped();
}
void pragma::audio::CALSound::SetGain(float gain)
{
	m_gain = gain;
	UpdateVolume();
}
float pragma::audio::CALSound::GetGain() const
{
	if(m_bTerminated == true)
		return 0.f;
	return glm::clamp((*this)->GetGain(), GetMinGain(), GetMaxGain());
}
void pragma::audio::CALSound::SetPosition(const Vector3 &pos)
{
	if(m_bTerminated == true)
		return;
	(*this)->SetPosition(pos);
}
Vector3 pragma::audio::CALSound::GetPosition() const
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
void pragma::audio::CALSound::SetVelocity(const Vector3 &vel)
{
	if(m_bTerminated == true)
		return;
	(*this)->SetVelocity(vel);
}
Vector3 pragma::audio::CALSound::GetVelocity() const
{
	if(m_bTerminated == true)
		return Vector3(0.f, 0.f, 0.f);
	return (*this)->GetVelocity();
}
void pragma::audio::CALSound::SetDirection(const Vector3 &dir)
{
	if(m_bTerminated == true)
		return;
	(*this)->SetDirection(dir);
}
Vector3 pragma::audio::CALSound::GetDirection() const
{
	if(m_bTerminated == true)
		return Vector3(0.f, 0.f, 0.f);
	return (*this)->GetDirection();
}
void pragma::audio::CALSound::SetRelative(bool b)
{
	if(m_bTerminated == true)
		return;
	(*this)->SetRelative(b);
}
bool pragma::audio::CALSound::IsRelative() const
{
	if(m_bTerminated == true)
		return false;
	return (*this)->IsRelative();
}
float pragma::audio::CALSound::GetDuration() const
{
	if(m_bTerminated == true)
		return 0.f;
	return (*this)->GetDuration();
}
float pragma::audio::CALSound::GetReferenceDistance() const
{
	if(m_bTerminated == true)
		return 0.f;
	return (*this)->GetReferenceDistance();
}
void pragma::audio::CALSound::SetReferenceDistance(float dist)
{
	if(m_bTerminated == true)
		return;
	(*this)->SetReferenceDistance(dist);
}
void pragma::audio::CALSound::SetRoomRolloffFactor(float roomFactor)
{
	if(m_bTerminated == true)
		return;
	(*this)->SetRoomRolloffFactor(roomFactor);
}
float pragma::audio::CALSound::GetRolloffFactor() const
{
	if(m_bTerminated == true)
		return 0.f;
	return (*this)->GetRolloffFactor();
}
float pragma::audio::CALSound::GetRoomRolloffFactor() const
{
	if(m_bTerminated == true)
		return 0.f;
	return (*this)->GetRoomRolloffFactor();
}
void pragma::audio::CALSound::SetRolloffFactor(float rolloff)
{
	if(m_bTerminated == true)
		return;
	(*this)->SetRolloffFactors(rolloff);
}
float pragma::audio::CALSound::GetMaxDistance() const
{
	if(m_bTerminated == true)
		return 0.f;
	return (*this)->GetMaxDistance();
}
void pragma::audio::CALSound::SetMaxDistance(float dist)
{
	if(m_bTerminated == true)
		return;
	(*this)->SetMaxDistance(dist);
}
float pragma::audio::CALSound::GetMinGain() const
{
	if(m_bTerminated == true)
		return 0.f;
	return (*this)->GetMinGain();
}
void pragma::audio::CALSound::SetMinGain(float gain)
{
	if(m_bTerminated == true)
		return;
	(*this)->SetMinGain(gain);
}
float pragma::audio::CALSound::GetMaxGain() const
{
	if(m_bTerminated == true)
		return 0.f;
	return (*this)->GetMaxGain();
}
void pragma::audio::CALSound::SetMaxGain(float gain)
{
	if(m_bTerminated == true)
		return;
	(*this)->SetMaxGain(gain);
}
float pragma::audio::CALSound::GetInnerConeAngle() const
{
	if(m_bTerminated == true)
		return 0.f;
	return (*this)->GetInnerConeAngle();
}
void pragma::audio::CALSound::SetInnerConeAngle(float ang)
{
	if(m_bTerminated == true)
		return;
	(*this)->SetInnerConeAngle(ang);
}
float pragma::audio::CALSound::GetOuterConeAngle() const
{
	if(m_bTerminated == true)
		return 0.f;
	return (*this)->GetOuterConeAngle();
}
void pragma::audio::CALSound::SetOuterConeAngle(float ang)
{
	if(m_bTerminated == true)
		return;
	(*this)->SetOuterConeAngle(ang);
}
float pragma::audio::CALSound::GetOuterConeGain() const
{
	if(m_bTerminated == true)
		return 0.f;
	return (*this)->GetOuterConeGain();
}
float pragma::audio::CALSound::GetOuterConeGainHF() const
{
	if(m_bTerminated == true)
		return 0.f;
	return (*this)->GetOuterConeGainHF();
}
void pragma::audio::CALSound::SetOuterConeGain(float gain)
{
	if(m_bTerminated == true)
		return;
	(*this)->SetOuterConeGain(gain);
}
void pragma::audio::CALSound::SetOuterConeGainHF(float gain)
{
	if(m_bTerminated == true)
		return;
	(*this)->SetOuterConeGainHF(gain);
}

uint32_t pragma::audio::CALSound::GetPriority()
{
	if(m_bTerminated == true)
		return 0;
	return (*this)->GetPriority();
}
void pragma::audio::CALSound::SetPriority(uint32_t priority)
{
	if(m_bTerminated == true)
		return;
	(*this)->SetPriority(priority);
}
void pragma::audio::CALSound::SetOrientation(const Vector3 &at, const Vector3 &up)
{
	if(m_bTerminated == true)
		return;
	(*this)->SetOrientation(at, up);
}
std::pair<Vector3, Vector3> pragma::audio::CALSound::GetOrientation() const
{
	if(m_bTerminated == true)
		return {};
	return (*this)->GetOrientation();
}
void pragma::audio::CALSound::SetDopplerFactor(float factor)
{
	if(m_bTerminated == true)
		return;
	(*this)->SetDopplerFactor(factor);
}
float pragma::audio::CALSound::GetDopplerFactor() const
{
	if(m_bTerminated == true)
		return 0.f;
	return (*this)->GetDopplerFactor();
}
void pragma::audio::CALSound::SetLeftStereoAngle(float ang)
{
	if(m_bTerminated == true)
		return;
	(*this)->SetLeftStereoAngle(ang);
}
float pragma::audio::CALSound::GetLeftStereoAngle() const
{
	if(m_bTerminated == true)
		return 0.f;
	return (*this)->GetLeftStereoAngle();
}
void pragma::audio::CALSound::SetRightStereoAngle(float ang)
{
	if(m_bTerminated == true)
		return;
	(*this)->SetRightStereoAngle(ang);
}
float pragma::audio::CALSound::GetRightStereoAngle() const
{
	if(m_bTerminated == true)
		return 0.f;
	return (*this)->GetRightStereoAngle();
}
void pragma::audio::CALSound::SetAirAbsorptionFactor(float factor)
{
	if(m_bTerminated == true)
		return;
	(*this)->SetAirAbsorptionFactor(factor);
}
float pragma::audio::CALSound::GetAirAbsorptionFactor() const
{
	if(m_bTerminated == true)
		return 0.f;
	return (*this)->GetAirAbsorptionFactor();
}
void pragma::audio::CALSound::SetGainAuto(bool directHF, bool send, bool sendHF)
{
	if(m_bTerminated == true)
		return;
	(*this)->SetGainAuto(directHF, send, sendHF);
}
std::tuple<bool, bool, bool> pragma::audio::CALSound::GetGainAuto() const
{
	if(m_bTerminated == true)
		return {false, false, false};
	return (*this)->GetGainAuto();
}
void pragma::audio::CALSound::SetDirectFilter(const SoundEffectParams &params)
{
	if(m_bTerminated == true)
		return;
	(*this)->SetDirectFilter(reinterpret_cast<const EffectParams &>(params));
}
const pragma::audio::SoundEffectParams &pragma::audio::CALSound::GetDirectFilter() const
{
	if(m_bTerminated == true) {
		static SoundEffectParams params {};
		return params;
	}
	return reinterpret_cast<const SoundEffectParams &>((*this)->GetDirectFilter());
}
bool pragma::audio::CALSound::AddEffect(const std::string &effectName, const SoundEffectParams &params)
{
	auto effect = get_cengine()->GetAuxEffect(effectName);
	if(effect == nullptr)
		return false;
	return (*this)->AddEffect(*effect, reinterpret_cast<const EffectParams &>(params));
}
void pragma::audio::CALSound::RemoveEffect(const std::string &effectName)
{
	auto effect = get_cengine()->GetAuxEffect(effectName);
	if(effect == nullptr)
		return;
	(*this)->RemoveEffect(*effect);
}
void pragma::audio::CALSound::SetEffectParameters(const std::string &effectName, const SoundEffectParams &params)
{
	auto effect = get_cengine()->GetAuxEffect(effectName);
	if(effect == nullptr)
		return;
	(*this)->SetEffectParameters(*effect, reinterpret_cast<const EffectParams &>(params));
}

void pragma::audio::CALSound::SetType(ALSoundType type)
{
	if(m_bTerminated == true)
		return;
	ALSound::SetType(type);
	UpdateVolume();
}
namespace {
	auto _ = pragma::console::client::register_variable_listener<bool>("cl_audio_always_play", +[](pragma::NetworkState *, const pragma::console::ConVar &, bool, bool) { pragma::get_client_state()->UpdateSoundVolume(); });
}
