// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

// Sound duration for mp3 files
//
#undef max

#ifdef WEAVE_MP3_SUPPORT_ENABLED
#ifdef _WIN32
#include <Windows.h>
#include <atlstr.h>
#include "mpafile.h"
#endif
#endif

module pragma.shared;

import :audio.sound;

pragma::audio::SoundEffectParams::SoundEffectParams(float pgain, float pgainHF, float pgainLF) : gain(pgain), gainHF(pgainHF), gainLF(pgainLF) {}

pragma::audio::ALSound::ALSound(NetworkState *nw) : inheritable_enable_shared_from_this<ALSound>(), CallbackHandler(), LuaCallbackHandler(), m_type(ALSoundType::Generic), m_networkState(nw)
{
	RegisterCallback<void>("OnDestroyed");
	RegisterCallback<void, ALState, ALState>("OnStateChanged");
}

pragma::audio::ALSound::~ALSound() {}

void pragma::audio::ALSound::OnRelease() { CallCallbacks<void>("OnDestroyed"); }

void pragma::audio::ALSound::SetRange(float start, float end) { m_range = std::make_unique<std::pair<float, float>>(std::pair<float, float> {start, end}); }
void pragma::audio::ALSound::ClearRange() { m_range = nullptr; }
bool pragma::audio::ALSound::HasRange() const { return (m_range != nullptr) ? true : false; }
void pragma::audio::ALSound::SetIndex(unsigned int idx) { m_index = idx; }
std::pair<float, float> pragma::audio::ALSound::GetRange() const
{
	if(HasRange() == true)
		return *m_range;
	return {0.f, GetDuration()};
}
std::pair<float, float> pragma::audio::ALSound::GetRangeOffsets() const
{
	if(HasRange() == false)
		return {0.f, 1.f};
	auto dur = GetDuration();
	auto range = GetRange();
	if(dur == 0.f)
		range = {0.f, 0.f};
	else {
		range.first /= dur;
		range.second /= dur;
	}
	return range;
}

bool pragma::audio::ALSound::IsSoundScript() const { return false; }
pragma::NetworkState *pragma::audio::ALSound::GetNetworkState() const { return m_networkState; }

void pragma::audio::ALSound::SetSource(ecs::BaseEntity *ent) { m_hSourceEntity = (ent != nullptr) ? ent->GetHandle() : EntityHandle {}; }
pragma::ecs::BaseEntity *pragma::audio::ALSound::GetSource() const { return const_cast<ecs::BaseEntity *>(m_hSourceEntity.get()); }

float pragma::audio::ALSound::GetSoundIntensity(const Vector3 &pos) const
{
	if(IsPlaying() == false)
		return 0.f;
	if(IsRelative() == true)
		return 1.f;
	auto rolloff = GetRolloffFactor();
	if(rolloff <= 0.f)
		return 1.f;
	auto d = uvec::distance(GetPosition(), pos);
	auto maxDist = GetMaxAudibleDistance();
	if(d >= maxDist)
		return 0.f;
	auto refDist = GetReferenceDistance();
	if(d <= refDist)
		return 1.f;
	d -= refDist;
	d /= maxDist - refDist;
	d *= 1.f / rolloff;
	return 1.f - d;
}

float pragma::audio::ALSound::GetLastOffset() const { return m_lastOffset; }
float pragma::audio::ALSound::GetDeltaOffset() const { return GetOffset() - m_lastOffset; }
float pragma::audio::ALSound::GetLastTimeOffset() const { return m_lastSecOffset; }
float pragma::audio::ALSound::GetDeltaTimeOffset() const { return GetTimeOffset() - m_lastSecOffset; }

float pragma::audio::ALSound::GetMaxAudibleDistance() const
{
	float rolloff = GetRolloffFactor();
	if(rolloff == 0.f)
		return std::numeric_limits<float>::max();
	return (GetMaxDistance() - GetReferenceDistance()) * (1.f / rolloff) + GetReferenceDistance();
}

unsigned int pragma::audio::ALSound::GetFlags() const { return m_flags; }
void pragma::audio::ALSound::SetFlags(unsigned int flags) { m_flags = flags; }
void pragma::audio::ALSound::AddFlags(unsigned int flags) { SetFlags(GetFlags() | flags); }
void pragma::audio::ALSound::RemoveFlags(unsigned int flags) { SetFlags(GetFlags() & ~flags); }

void pragma::audio::ALSound::PostUpdate()
{
	UpdateOffset();
	if(IsPlaying() == false || m_fade != nullptr || m_tFadeOut == 0.f)
		return;
	auto range = GetRangeOffsets();
	auto timeUntilEnd = range.second - GetOffset();
	auto tFadeOut = m_tFadeOut / GetDuration();
	if(timeUntilEnd <= 0.f || timeUntilEnd > tFadeOut)
		return;
	FadeOut(timeUntilEnd);
}

void pragma::audio::ALSound::CheckStateChange(ALState old)
{
	auto state = GetState();
	if(state != old) {
		CallCallbacks<void, ALState, ALState>("OnStateChanged", old, state);
		CallLuaCallbacks<void, int32_t, int32_t>("OnStateChanged", math::to_integral(old), math::to_integral(state));
		auto *nw = GetNetworkState();
		if(nw != nullptr) {
			auto *game = nw->GetGameState();
			if(game != nullptr) {
				game->CallCallbacks<void, ALSound *, ALState, ALState>("OnSoundStateChanged", this, old, state);
				game->CallLuaCallbacks<void, std::shared_ptr<ALSound>, int32_t, int32_t>("OnSoundStateChanged", shared_from_this(), math::to_integral(old), math::to_integral(state));
			}
		}
	}
}

unsigned int pragma::audio::ALSound::GetIndex() const { return m_index; }

void pragma::audio::ALSound::SetFadeInDuration(float t) { m_tFadeIn = t; }
void pragma::audio::ALSound::SetFadeOutDuration(float t) { m_tFadeOut = t; }
float pragma::audio::ALSound::GetFadeInDuration() const { return m_tFadeIn; }
float pragma::audio::ALSound::GetFadeOutDuration() const { return m_tFadeOut; }

void pragma::audio::ALSound::InitRange()
{
	if(HasRange() == false)
		return;
	auto range = GetRangeOffsets();
	SetOffset(range.first);
}

void pragma::audio::ALSound::UpdateOffset()
{
	m_lastOffset = GetOffset();
	m_lastSecOffset = GetTimeOffset();
	if(HasRange() == false)
		return;
	auto range = GetRangeOffsets();
	if(m_lastOffset >= range.second) {
		if(IsLooping())
			SetOffset(range.first);
		else
			Stop();
	}
}

void pragma::audio::ALSound::CancelFade()
{
	if(m_fade == nullptr)
		return;
	bool fadein = m_fade->fadein;
	float gain = m_fade->gain;
	m_fade = nullptr;
	if(fadein)
		SetGain(gain);
	else
		Pause();
}

void pragma::audio::ALSound::SetType(ALSoundType type) { m_type = type; }
void pragma::audio::ALSound::AddType(ALSoundType type)
{
	type = static_cast<ALSoundType>(CUInt32(m_type) | CUInt32(type));
	SetType(type);
}
pragma::audio::ALSoundType pragma::audio::ALSound::GetType() const { return m_type; }
pragma::audio::ALState pragma::audio::ALSound::GetState() const { return ALState::Initial; }

void pragma::audio::ALSound::UpdateState() {}
void pragma::audio::ALSound::Initialize() {}

void pragma::audio::ALSound::SetGainRange(float minGain, float maxGain)
{
	SetMinGain(minGain);
	SetMaxGain(maxGain);
}
std::pair<float, float> pragma::audio::ALSound::GetGainRange() const { return {GetMinGain(), GetMaxGain()}; }
void pragma::audio::ALSound::SetDistanceRange(float refDist, float maxDist)
{
	SetReferenceDistance(refDist);
	SetMaxDistance(maxDist);
}
std::pair<float, float> pragma::audio::ALSound::GetDistanceRange() const { return {GetReferenceDistance(), GetMaxDistance()}; }
void pragma::audio::ALSound::SetConeAngles(float inner, float outer)
{
	SetInnerConeAngle(inner);
	SetOuterConeAngle(outer);
}
std::pair<float, float> pragma::audio::ALSound::GetConeAngles() const { return {GetInnerConeAngle(), GetOuterConeAngle()}; }
std::pair<float, float> pragma::audio::ALSound::GetOuterConeGains() const { return {GetOuterConeGain(), GetOuterConeGainHF()}; }
void pragma::audio::ALSound::SetStereoAngles(float leftAngle, float rightAngle)
{
	SetLeftStereoAngle(leftAngle);
	SetRightStereoAngle(rightAngle);
}
std::pair<float, float> pragma::audio::ALSound::GetStereoAngles() const { return {GetLeftStereoAngle(), GetRightStereoAngle()}; }
void pragma::audio::ALSound::SetOuterConeGains(float gain, float gainHF)
{
	SetOuterConeGain(gain);
	SetOuterConeGainHF(gainHF);
}
bool pragma::audio::ALSound::GetDirectGainHFAuto() const { return std::get<0>(GetGainAuto()); }
bool pragma::audio::ALSound::GetSendGainAuto() const { return std::get<1>(GetGainAuto()); }
bool pragma::audio::ALSound::GetSendGainHFAuto() const { return std::get<2>(GetGainAuto()); }
std::pair<float, float> pragma::audio::ALSound::GetRolloffFactors() const { return {GetRolloffFactor(), GetRoomRolloffFactor()}; }
void pragma::audio::ALSound::SetRolloffFactors(float factor, float roomFactor)
{
	SetRolloffFactor(factor);
	SetRoomRolloffFactor(roomFactor);
}
void pragma::audio::ALSound::SetEffectGain(const std::string &effectName, float gain) { SetEffectParameters(effectName, {gain}); }
void pragma::audio::ALSound::SetTimeOffset(float offset)
{
	auto dur = GetDuration();
	if(dur == 0.f) {
		SetOffset(0.f);
		return;
	}
	SetOffset(offset / dur);
}
float pragma::audio::ALSound::GetTimeOffset() const { return GetOffset() * GetDuration(); }
