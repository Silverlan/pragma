// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "memory"

#include "mathutil/umath.h"

#include "sharedutils/util.h"

// Sound duration for mp3 files
#ifdef WEAVE_MP3_SUPPORT_ENABLED
#ifdef _WIN32
#include <Windows.h>
#include <atlstr.h>
#include "mpafile.h"
#endif
#endif
//
#include <string>
#include <fsys/filesystem.h>
#include <cstring>
#include <mathutil/uvec.h>

#undef max

module pragma.shared;

import :audio.sound;

SoundEffectParams::SoundEffectParams(float pgain, float pgainHF, float pgainLF) : gain(pgain), gainHF(pgainHF), gainLF(pgainLF) {}

ALSound::ALSound(NetworkState *nw) : util::inheritable_enable_shared_from_this<ALSound>(), CallbackHandler(), LuaCallbackHandler(), m_type(ALSoundType::Generic), m_networkState(nw)
{
	RegisterCallback<void>("OnDestroyed");
	RegisterCallback<void, ALState, ALState>("OnStateChanged");
}

ALSound::~ALSound() {}

void ALSound::OnRelease() { CallCallbacks<void>("OnDestroyed"); }

void ALSound::SetRange(float start, float end) { m_range = std::make_unique<std::pair<float, float>>(std::pair<float, float> {start, end}); }
void ALSound::ClearRange() { m_range = nullptr; }
bool ALSound::HasRange() const { return (m_range != nullptr) ? true : false; }
void ALSound::SetIndex(unsigned int idx) { m_index = idx; }
std::pair<float, float> ALSound::GetRange() const
{
	if(HasRange() == true)
		return *m_range;
	return {0.f, GetDuration()};
}
std::pair<float, float> ALSound::GetRangeOffsets() const
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

bool ALSound::IsSoundScript() const { return false; }
NetworkState *ALSound::GetNetworkState() const { return m_networkState; }

void ALSound::SetSource(BaseEntity *ent) { m_hSourceEntity = (ent != nullptr) ? ent->GetHandle() : EntityHandle {}; }
BaseEntity *ALSound::GetSource() const { return const_cast<BaseEntity *>(m_hSourceEntity.get()); }

float ALSound::GetSoundIntensity(const Vector3 &pos) const
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

float ALSound::GetLastOffset() const { return m_lastOffset; }
float ALSound::GetDeltaOffset() const { return GetOffset() - m_lastOffset; }
float ALSound::GetLastTimeOffset() const { return m_lastSecOffset; }
float ALSound::GetDeltaTimeOffset() const { return GetTimeOffset() - m_lastSecOffset; }

float ALSound::GetMaxAudibleDistance() const
{
	float rolloff = GetRolloffFactor();
	if(rolloff == 0.f)
		return std::numeric_limits<float>::max();
	return (GetMaxDistance() - GetReferenceDistance()) * (1.f / rolloff) + GetReferenceDistance();
}

unsigned int ALSound::GetFlags() const { return m_flags; }
void ALSound::SetFlags(unsigned int flags) { m_flags = flags; }
void ALSound::AddFlags(unsigned int flags) { SetFlags(GetFlags() | flags); }
void ALSound::RemoveFlags(unsigned int flags) { SetFlags(GetFlags() & ~flags); }

void ALSound::PostUpdate()
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

void ALSound::CheckStateChange(ALState old)
{
	auto state = GetState();
	if(state != old) {
		CallCallbacks<void, ALState, ALState>("OnStateChanged", old, state);
		CallLuaCallbacks<void, int32_t, int32_t>("OnStateChanged", umath::to_integral(old), umath::to_integral(state));
		auto *nw = GetNetworkState();
		if(nw != nullptr) {
			auto *game = nw->GetGameState();
			if(game != nullptr) {
				game->CallCallbacks<void, ALSound *, ALState, ALState>("OnSoundStateChanged", this, old, state);
				game->CallLuaCallbacks<void, std::shared_ptr<ALSound>, int32_t, int32_t>("OnSoundStateChanged", shared_from_this(), umath::to_integral(old), umath::to_integral(state));
			}
		}
	}
}

unsigned int ALSound::GetIndex() const { return m_index; }

void ALSound::SetFadeInDuration(float t) { m_tFadeIn = t; }
void ALSound::SetFadeOutDuration(float t) { m_tFadeOut = t; }
float ALSound::GetFadeInDuration() const { return m_tFadeIn; }
float ALSound::GetFadeOutDuration() const { return m_tFadeOut; }

void ALSound::InitRange()
{
	if(HasRange() == false)
		return;
	auto range = GetRangeOffsets();
	SetOffset(range.first);
}

void ALSound::UpdateOffset()
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

void ALSound::CancelFade()
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

void ALSound::SetType(ALSoundType type) { m_type = type; }
void ALSound::AddType(ALSoundType type)
{
	type = static_cast<ALSoundType>(CUInt32(m_type) | CUInt32(type));
	SetType(type);
}
ALSoundType ALSound::GetType() const { return m_type; }
ALState ALSound::GetState() const { return ALState::Initial; }

void ALSound::UpdateState() {}
void ALSound::Initialize() {}

void ALSound::SetGainRange(float minGain, float maxGain)
{
	SetMinGain(minGain);
	SetMaxGain(maxGain);
}
std::pair<float, float> ALSound::GetGainRange() const { return {GetMinGain(), GetMaxGain()}; }
void ALSound::SetDistanceRange(float refDist, float maxDist)
{
	SetReferenceDistance(refDist);
	SetMaxDistance(maxDist);
}
std::pair<float, float> ALSound::GetDistanceRange() const { return {GetReferenceDistance(), GetMaxDistance()}; }
void ALSound::SetConeAngles(float inner, float outer)
{
	SetInnerConeAngle(inner);
	SetOuterConeAngle(outer);
}
std::pair<float, float> ALSound::GetConeAngles() const { return {GetInnerConeAngle(), GetOuterConeAngle()}; }
std::pair<float, float> ALSound::GetOuterConeGains() const { return {GetOuterConeGain(), GetOuterConeGainHF()}; }
void ALSound::SetStereoAngles(float leftAngle, float rightAngle)
{
	SetLeftStereoAngle(leftAngle);
	SetRightStereoAngle(rightAngle);
}
std::pair<float, float> ALSound::GetStereoAngles() const { return {GetLeftStereoAngle(), GetRightStereoAngle()}; }
void ALSound::SetOuterConeGains(float gain, float gainHF)
{
	SetOuterConeGain(gain);
	SetOuterConeGainHF(gainHF);
}
bool ALSound::GetDirectGainHFAuto() const { return std::get<0>(GetGainAuto()); }
bool ALSound::GetSendGainAuto() const { return std::get<1>(GetGainAuto()); }
bool ALSound::GetSendGainHFAuto() const { return std::get<2>(GetGainAuto()); }
std::pair<float, float> ALSound::GetRolloffFactors() const { return {GetRolloffFactor(), GetRoomRolloffFactor()}; }
void ALSound::SetRolloffFactors(float factor, float roomFactor)
{
	SetRolloffFactor(factor);
	SetRoomRolloffFactor(roomFactor);
}
void ALSound::SetEffectGain(const std::string &effectName, float gain) { SetEffectParameters(effectName, {gain}); }
void ALSound::SetTimeOffset(float offset)
{
	auto dur = GetDuration();
	if(dur == 0.f) {
		SetOffset(0.f);
		return;
	}
	SetOffset(offset / dur);
}
float ALSound::GetTimeOffset() const { return GetOffset() * GetDuration(); }
