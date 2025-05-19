/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/networkstate/networkstate.h"
#include "pragma/audio/alsoundscript.h"
#include <fsys/filesystem.h>
#include "datasystem.h"
#include "pragma/audio/soundscript.h"
#include "pragma/lua/ldefinitions.h"
#include <pragma/math/vector/wvvector3.h>
#include "luasystem.h"
#include "pragma/audio/alsound_type.h"

#pragma warning(disable : 4056)
ALSoundScript::ALSoundScript(NetworkState *nw, unsigned int idx, SoundScript *script, NetworkState *state, bool bStream) : ALSound(nw), m_script(script), m_networkState(state), m_bStream(bStream) { m_index = idx; }
#pragma warning(default : 4056)

ALSoundScript::~ALSoundScript()
{
	for(unsigned int i = 0; i < m_events.size(); i++)
		delete m_events[i];
	for(unsigned int i = 0; i < m_sounds.size(); i++)
		delete m_sounds[i];
}

void ALSoundScript::SetTargetPosition(unsigned int id, Vector3 pos)
{
	std::unordered_map<unsigned int, Vector3>::iterator it = m_positions.find(id);
	if(it != m_positions.end())
		it->second = pos;
	else
		m_positions.insert(std::unordered_map<unsigned int, Vector3>::value_type(id, pos));
	for(unsigned int i = 0; i < m_sounds.size(); i++) {
		SSESound *snd = m_sounds[i];
		SSEPlaySound *ev = static_cast<SSEPlaySound *>(snd->event);
		if(ev->position == CInt32(id)) {
			std::shared_ptr<ALSound> sound = snd->sound;
			sound->SetPosition(pos);
		}
	}
}

void ALSoundScript::FadeIn(float time)
{
	//ALSound::FadeIn(time);
	float gain = GetGain();
	SetGain(0);
	CancelFade();
	if(!IsPlaying())
		Play();
	m_fade = std::unique_ptr<SoundFade>(new SoundFade(true, m_networkState->RealTime(), time, gain));
}

void ALSoundScript::FadeOut(float time)
{
	//ALSound::FadeOut(time);
	if(!IsPlaying())
		return;
	float gain = GetGain();
	CancelFade();
	m_fade = std::unique_ptr<SoundFade>(new SoundFade(false, m_networkState->RealTime(), time, gain));
}

void ALSoundScript::Initialize()
{
	//ALSound::Initialize();
}

std::shared_ptr<ALSound> ALSoundScript::CreateSound(const std::string &name, ALChannel channel, ALCreateFlags createFlags)
{
	auto flags = ALCreateFlags::None;
	if(channel == ALChannel::Mono)
		flags |= ALCreateFlags::Mono;
	if(m_bStream == true)
		flags |= ALCreateFlags::Stream;
	flags |= createFlags;
	return m_networkState->CreateSound(name, GetType(), flags);
}

void ALSoundScript::InitializeEvent(SoundScriptEvent *ev)
{
	SSEPlaySound *ps = dynamic_cast<SSEPlaySound *>(ev);
	if(ps != NULL) {
		SSESound *snd = ps->CreateSound(m_tPassed, [this](const std::string &name, ALChannel channel, ALCreateFlags createFlags) { return CreateSound(name, channel, createFlags); });
		if(snd != NULL) {
			ALSound *als = snd->sound.get();
			auto bSetPos = false;
			if(ps->position != -1) {
				als->SetRelative(false);
				if(ps->position == -2) {
					Vector3 origin = GetPosition();
					Vector3 dir = uvec::create_random_unit_vector();
					dir.y = 0.f;
					uvec::normalize(&dir);
					dir *= 500.f;
					als->SetPosition(origin + dir);
					bSetPos = true;
				}
				else {
					std::unordered_map<unsigned int, Vector3>::iterator it = m_positions.find(ps->position);
					if(it != m_positions.end()) {
						bSetPos = true;
						als->SetPosition(it->second);
					}
				}
			}
			if(ps->position == 1)
				als->SetRelative(false);
			if(bSetPos == false) {
				auto pos = GetPosition();
				if(pos.x != 0.f || pos.y != 0.f || pos.z != 0.f)
					als->SetPosition(pos);
			}
			auto vel = GetVelocity();
			if(vel.x != 0.f || vel.y != 0.f || vel.z != 0.f)
				als->SetVelocity(vel);
			auto dir = GetDirection();
			if(dir.x != 0.f || dir.y != 0.f || dir.z != 0.f)
				als->SetDirection(dir);
			if(m_bRelative != ALSOUND_DEFAULT_RELATIVE)
				als->SetRelative(m_bRelative);
			if(m_refDist != ALSOUND_DEFAULT_REFERENCE_DISTANCE)
				als->SetReferenceDistance(m_refDist);
			if(m_rolloff.first != ALSOUND_DEFAULT_ROLLOFF_FACTOR)
				als->SetRolloffFactor(m_rolloff.first);
			if(m_rolloff.second != ALSOUND_DEFAULT_ROOM_ROLLOFF_FACTOR)
				als->SetRoomRolloffFactor(m_rolloff.second);
			if(m_maxDist != ALSOUND_DEFAULT_MAX_DISTANCE)
				als->SetMaxDistance(static_cast<float>(m_maxDist));
			if(m_minGain != ALSOUND_DEFAULT_MIN_GAIN)
				als->SetMinGain(ALSOUND_DEFAULT_MIN_GAIN);
			if(m_maxGain != ALSOUND_DEFAULT_MAX_GAIN)
				als->SetMaxGain(ALSOUND_DEFAULT_MAX_GAIN);
			if(m_coneInnerAngle != ALSOUND_DEFAULT_CONE_INNER_ANGLE)
				als->SetInnerConeAngle(m_coneInnerAngle);
			if(m_coneOuterAngle != ALSOUND_DEFAULT_CONE_OUTER_ANGLE)
				als->SetOuterConeAngle(m_coneOuterAngle);
			if(m_coneOuterGain.first != ALSOUND_DEFAULT_CONE_OUTER_GAIN)
				als->SetOuterConeGain(m_coneOuterGain.first);
			if(m_coneOuterGain.second != ALSOUND_DEFAULT_CONE_OUTER_GAIN_HF)
				als->SetOuterConeGainHF(m_coneOuterGain.second);
			if(m_priority != ALSOUND_DEFAULT_PRIORITY)
				als->SetPriority(m_priority);
			if(m_dopplerFactor != ALSOUND_DEFAULT_DOPPLER_FACTOR)
				als->SetDopplerFactor(m_dopplerFactor);
			if(m_airAbsorptionFactor != ALSOUND_DEFAULT_AIR_ABSORPTION_FACTOR)
				als->SetAirAbsorptionFactor(m_airAbsorptionFactor);
			if(m_orientation.first != ALSOUND_DEFAULT_ORIENTATION_AT || m_orientation.second != ALSOUND_DEFAULT_ORIENTATION_UP)
				als->SetOrientation(m_orientation.first, m_orientation.second);
			if(!umath::equals<float>(m_stereoAngles.first, ALSOUND_DEFAULT_STEREO_ANGLE_LEFT))
				als->SetLeftStereoAngle(m_stereoAngles.first);
			if(!umath::equals<float>(m_stereoAngles.second, ALSOUND_DEFAULT_STEREO_ANGLE_RIGHT))
				als->SetRightStereoAngle(m_stereoAngles.second);
			if(std::get<0>(m_gainAuto) != ALSOUND_DEFAULT_DIRECT_GAIN_HF_AUTO || std::get<1>(m_gainAuto) != ALSOUND_DEFAULT_SEND_GAIN_AUTO || std::get<2>(m_gainAuto) != ALSOUND_DEFAULT_SEND_GAIN_HF_AUTO)
				als->SetGainAuto(std::get<0>(m_gainAuto), std::get<1>(m_gainAuto), std::get<2>(m_gainAuto));
			if(m_directFilter.gain != ALSOUND_DEFAULT_DIRECT_FILTER_GAIN || m_directFilter.gainHF != ALSOUND_DEFAULT_DIRECT_FILTER_GAIN_HF || m_directFilter.gainLF != ALSOUND_DEFAULT_DIRECT_FILTER_GAIN_LF)
				als->SetDirectFilter(m_directFilter);
			als->SetSource(GetSource());
			float dur = als->GetDuration();
			if(dur > m_duration)
				m_duration = dur;
			m_sounds.push_back(snd);
			als->SetGain(als->GetGain() * GetGain());
			als->SetPitch(als->GetPitch() * GetPitch());
			CallCallbacks<void, ALSound *>("OnSoundCreated", als);
			als->Play();

			SetType(GetType() | als->GetType());
		}
	}
	else {
		//SSELua *lua = dynamic_cast<SSELua*>(ev); // Currently not implemented
		//if(lua != NULL)
		//	;
		//else
		m_events.push_back(ev->CreateEvent(m_tPassed));
	}
}

bool ALSoundScript::HandleEvents(SoundScriptEvent *ev, float eventOffset, float lastOffset, float newOffset)
{
	if(eventOffset <= lastOffset || eventOffset > newOffset)
		return false;
	auto &events = ev->GetEvents();
	for(unsigned int i = 0; i < events.size(); i++)
		InitializeEvent(events[i].get());
	return true;
}

void ALSoundScript::SetState(ALState state)
{
	auto old = GetState();
	if(state != old)
		CallCallbacks<void, ALState, ALState>("OnStateChanged", old, state);
	ALSoundBase::SetState(state);
}
ALState ALSoundScript::GetState() const { return ALSoundBase::GetState(); }

uint32_t ALSoundScript::GetSoundCount() const { return m_sounds.size(); }
ALSound *ALSoundScript::GetSound(uint32_t idx) { return (idx < m_sounds.size()) ? m_sounds.at(idx)->sound.get() : nullptr; }

bool ALSoundScript::IsSoundScript() const { return true; }

void ALSoundScript::Update()
{
	//ALSound::Update();
	auto old = GetState();
	UpdateState();
	if(GetState() == ALState::Stopped) {
		CancelFade();
		CheckStateChange(old);
	}
	else if(m_fade) {
		double t = m_networkState->RealTime() - m_fade->start;
		if(t >= m_fade->duration)
			CancelFade();
		else {
			float gain = CFloat(t / m_fade->duration) * m_fade->gain;
			if(m_fade->fadein)
				SetGain(gain);
			else
				SetGain(m_fade->gain - gain);
		}
	}
	if(GetState() == ALState::Playing) {
		double t = m_networkState->RealTime();
		double tDelta = t - m_tLastUpdate;
		tDelta *= GetPitch() / 1.f;
		float dur = GetDuration();
		m_offset += CFloat(tDelta) / dur;
		//float secOffset = GetSecOffset();
		if(m_offset > 1.0) {
			if(m_sounds.empty() && m_events.empty()) {
				m_offset = 1.0;
				m_tLastUpdate = t;
				SetState(ALState::Stopped);
			}
			else if(IsLooping())
				m_offset -= std::floor(m_offset);
			else
				m_offset = 1.0;
		}
	}
}

void ALSoundScript::PostUpdate()
{
	//double tLastUpdate = m_tLastUpdate;
	double tPassed = m_tPassed;
	if(GetState() == ALState::Playing) {
		double t = m_networkState->RealTime();
		double tDelta = t - m_tLastUpdate;
		tDelta *= GetPitch() / 1.f;
		m_tLastUpdate = t;
		m_tPassed += tDelta;
	}
	auto numSounds = m_sounds.size();
	//double t = m_networkState->RealTime();
	for(size_t i = 0; i < numSounds; i++) {
		SSESound *snd = m_sounds[i];
		if((*snd)->IsPlaying())
			HandleEvents(snd->event, snd->eventOffset, (*snd)->GetLastTimeOffset(), (*snd)->GetTimeOffset());
		else if(!(*snd)->IsPaused() && !(*snd)->IsLooping()) {
			SoundScriptEvent *ev = snd->event;
			bool bRepeat = ev->repeat;
			delete snd;
			m_sounds.erase(m_sounds.begin() + i);
			numSounds--;
			i--;
			if(bRepeat == true)
				InitializeEvent(ev);
		}
	}
	if(numSounds == 0 && GetState() == ALState::Playing)
		SetState(ALState::Stopped);
	auto numEvents = m_events.size();
	for(size_t i = 0; i < numEvents; i++) {
		SSEBase *sse = m_events[i];
		if(HandleEvents(sse->event, CFloat(sse->timeCreated + sse->eventOffset), CFloat(tPassed), CFloat(m_tPassed)) == true) {
			SoundScriptEvent *ev = sse->event;
			bool bRepeat = ev->repeat;
			delete sse;
			m_events.erase(m_events.begin() + i);
			numEvents--;
			i--;
			if(bRepeat == true)
				InitializeEvent(ev);
		}
	}
	ALSound::PostUpdate();
}

void ALSoundScript::Play()
{
	auto state = GetState();
	if(state == ALState::Initial) {
		auto &events = m_script->GetEvents();
		for(unsigned int i = 0; i < events.size(); i++)
			InitializeEvent(events[i].get());
	}
	if(state != ALState::Paused)
		ALSoundBase::SetOffset(0.f);
	SetState(ALState::Playing);
	m_tLastUpdate = m_networkState->RealTime();
	for(unsigned int i = 0; i < m_sounds.size(); i++)
		(*m_sounds[i])->Play();
}
void ALSoundScript::Stop()
{
	SetState(ALState::Stopped);
	for(unsigned int i = 0; i < m_sounds.size(); i++)
		(*m_sounds[i])->Stop();
}
void ALSoundScript::Pause()
{
	SetState(ALState::Paused);
	for(unsigned int i = 0; i < m_sounds.size(); i++)
		(*m_sounds[i])->Pause();
}
void ALSoundScript::Rewind()
{
	ALSoundBase::SetOffset(0.f);
	SetState(ALState::Initial);
	for(unsigned int i = 0; i < m_sounds.size(); i++)
		(*m_sounds[i])->Rewind();
}
void ALSoundScript::SetOffset(float offset)
{
	offset = std::min(offset, 1.f);
	ALSoundBase::SetOffset(offset);
	for(unsigned int i = 0; i < m_sounds.size(); i++)
		(*m_sounds[i])->SetOffset(offset);
}
float ALSoundScript::GetOffset() const { return ALSoundBase::GetOffset(); }
void ALSoundScript::SetPitch(float pitch)
{
	float pitchPrev = GetPitch();
	ALSoundBase::SetPitch(pitch);
	for(unsigned int i = 0; i < m_sounds.size(); i++) {
		ALSound *als = m_sounds[i]->sound.get();
		if(pitchPrev == 0.f)
			als->SetPitch(pitch);
		else
			als->SetPitch((als->GetPitch() / pitchPrev) * pitch);
	}
}
float ALSoundScript::GetPitch() const { return ALSoundBase::GetPitch(); }
void ALSoundScript::SetLooping(bool loop)
{
	ALSoundBase::SetLooping(loop);
	for(unsigned int i = 0; i < m_sounds.size(); i++)
		(*m_sounds[i])->SetLooping(loop);
}
bool ALSoundScript::IsLooping() const
{
	if(ALSoundBase::IsLooping() == true)
		return true;
	// Return true if one of our child-sounds is looping
	for(auto *snd : m_sounds) {
		if(snd->sound != nullptr && snd->sound->IsLooping() == true)
			return true;
	}
	return false;
}
bool ALSoundScript::IsPlaying() const { return ALSoundBase::IsPlaying(); }
bool ALSoundScript::IsPaused() const { return ALSoundBase::IsPaused(); }
bool ALSoundScript::IsStopped() const { return ALSoundBase::IsStopped(); }
void ALSoundScript::SetGain(float gain)
{
	float gainPrev = GetGain();
	ALSoundBase::SetGain(gain);
	for(unsigned int i = 0; i < m_sounds.size(); i++) {
		ALSound *als = m_sounds[i]->sound.get();
		if(gainPrev == 0.f)
			als->SetGain(gain);
		else
			als->SetGain((als->GetGain() / gainPrev) * gain);
	}
}
float ALSoundScript::GetGain() const { return ALSoundBase::GetGain(); }
void ALSoundScript::SetPosition(const Vector3 &pos)
{
	ALSoundBase::SetPosition(pos);
	for(unsigned int i = 0; i < m_sounds.size(); i++)
		(*m_sounds[i])->SetPosition(pos);
}
Vector3 ALSoundScript::GetPosition() const { return ALSoundBase::GetPosition(); }
void ALSoundScript::SetVelocity(const Vector3 &vel)
{
	ALSoundBase::SetVelocity(vel);
	for(unsigned int i = 0; i < m_sounds.size(); i++)
		(*m_sounds[i])->SetVelocity(vel);
}
Vector3 ALSoundScript::GetVelocity() const { return ALSoundBase::GetVelocity(); }
void ALSoundScript::SetDirection(const Vector3 &dir)
{
	ALSoundBase::SetDirection(dir);
	for(unsigned int i = 0; i < m_sounds.size(); i++)
		(*m_sounds[i])->SetDirection(dir);
}
Vector3 ALSoundScript::GetDirection() const { return ALSoundBase::GetDirection(); }
void ALSoundScript::SetRelative(bool b)
{
	ALSoundBase::SetRelative(b);
	for(unsigned int i = 0; i < m_sounds.size(); i++)
		(*m_sounds[i])->SetRelative(b);
}
bool ALSoundScript::IsRelative() const { return ALSoundBase::IsRelative(); }
void ALSoundScript::SetTimeOffset(float sec)
{
	for(unsigned int i = 0; i < m_sounds.size(); i++)
		(*m_sounds[i])->SetTimeOffset(sec);
}
float ALSoundScript::GetTimeOffset() const
{
	if(m_sounds.empty())
		return 0.f; //ALSound::GetTimeOffset();
	return (*m_sounds.front())->GetTimeOffset();
}
float ALSoundScript::GetDuration() const { return ALSoundBase::GetDuration(); }
float ALSoundScript::GetReferenceDistance() const { return ALSoundBase::GetReferenceDistance(); }
void ALSoundScript::SetReferenceDistance(float dist)
{
	ALSoundBase::SetReferenceDistance(dist);
	for(unsigned int i = 0; i < m_sounds.size(); i++)
		(*m_sounds[i])->SetReferenceDistance(dist);
}
void ALSoundScript::SetRoomRolloffFactor(float roomFactor)
{
	ALSoundBase::SetRoomRolloffFactor(roomFactor);
	for(auto *snd : m_sounds)
		(*snd)->SetRoomRolloffFactor(roomFactor);
}
float ALSoundScript::GetRolloffFactor() const { return ALSoundBase::GetRolloffFactor(); }
void ALSoundScript::SetRolloffFactor(float factor)
{
	ALSoundBase::SetRolloffFactor(factor);
	for(auto *snd : m_sounds)
		(*snd)->SetRolloffFactor(factor);
}
float ALSoundScript::GetRoomRolloffFactor() const { return ALSoundBase::GetRoomRolloffFactor(); }
float ALSoundScript::GetMaxDistance() const { return ALSoundBase::GetMaxDistance(); }
void ALSoundScript::SetMaxDistance(float dist)
{
	ALSoundBase::SetMaxDistance(dist);
	for(unsigned int i = 0; i < m_sounds.size(); i++)
		(*m_sounds[i])->SetMaxDistance(dist);
}
float ALSoundScript::GetMinGain() const { return ALSoundBase::GetMinGain(); }
void ALSoundScript::SetMinGain(float gain)
{
	ALSoundBase::SetMinGain(gain);
	for(unsigned int i = 0; i < m_sounds.size(); i++)
		(*m_sounds[i])->SetMinGain(gain);
}
float ALSoundScript::GetMaxGain() const { return ALSoundBase::GetMaxGain(); }
void ALSoundScript::SetMaxGain(float gain)
{
	ALSoundBase::SetMaxGain(gain);
	for(unsigned int i = 0; i < m_sounds.size(); i++)
		(*m_sounds[i])->SetMaxGain(gain);
}
float ALSoundScript::GetInnerConeAngle() const { return ALSoundBase::GetInnerConeAngle(); }
void ALSoundScript::SetInnerConeAngle(float ang)
{
	ALSoundBase::SetInnerConeAngle(ang);
	for(auto *snd : m_sounds)
		(*snd)->SetInnerConeAngle(ang);
}
float ALSoundScript::GetOuterConeAngle() const { return ALSoundBase::GetOuterConeAngle(); }
void ALSoundScript::SetOuterConeAngle(float ang)
{
	ALSoundBase::SetOuterConeAngle(ang);
	for(auto *snd : m_sounds)
		(*snd)->SetOuterConeAngle(ang);
}
float ALSoundScript::GetOuterConeGain() const { return ALSoundBase::GetOuterConeGain(); }
float ALSoundScript::GetOuterConeGainHF() const { return ALSoundBase::GetOuterConeGainHF(); }
void ALSoundScript::SetOuterConeGain(float gain)
{
	ALSoundBase::SetOuterConeGain(gain);
	for(auto *snd : m_sounds)
		(*snd)->SetOuterConeGain(gain);
}
void ALSoundScript::SetOuterConeGainHF(float gain)
{
	ALSoundBase::SetOuterConeGainHF(gain);
	for(auto *snd : m_sounds)
		(*snd)->SetOuterConeGainHF(gain);
}

void ALSoundScript::SetFlags(unsigned int flags)
{
	ALSound::SetFlags(flags);
	for(unsigned int i = 0; i < m_sounds.size(); i++)
		(*m_sounds[i])->SetFlags(flags);
}
uint32_t ALSoundScript::GetPriority() { return ALSoundBase::GetPriority(); }
void ALSoundScript::SetPriority(uint32_t priority)
{
	ALSoundBase::SetPriority(priority);
	for(auto *snd : m_sounds)
		(*snd)->SetPriority(priority);
}
void ALSoundScript::SetOrientation(const Vector3 &at, const Vector3 &up)
{
	ALSoundBase::SetOrientation(at, up);
	for(auto *snd : m_sounds)
		(*snd)->SetOrientation(at, up);
}
std::pair<Vector3, Vector3> ALSoundScript::GetOrientation() const
{
	return ALSoundBase::GetOrientation();
	;
}
void ALSoundScript::SetDopplerFactor(float factor)
{
	ALSoundBase::SetDopplerFactor(factor);
	for(auto *snd : m_sounds)
		(*snd)->SetDopplerFactor(factor);
}
float ALSoundScript::GetDopplerFactor() const { return ALSoundBase::GetDopplerFactor(); }
void ALSoundScript::SetLeftStereoAngle(float ang)
{
	ALSoundBase::SetLeftStereoAngle(ang);
	for(auto *snd : m_sounds)
		(*snd)->SetLeftStereoAngle(ang);
}
float ALSoundScript::GetLeftStereoAngle() const { return ALSoundBase::GetLeftStereoAngle(); }
void ALSoundScript::SetRightStereoAngle(float ang)
{
	ALSoundBase::SetRightStereoAngle(ang);
	for(auto *snd : m_sounds)
		(*snd)->SetRightStereoAngle(ang);
}
float ALSoundScript::GetRightStereoAngle() const { return ALSoundBase::GetRightStereoAngle(); }
void ALSoundScript::SetAirAbsorptionFactor(float factor)
{
	ALSoundBase::SetAirAbsorptionFactor(factor);
	for(auto *snd : m_sounds)
		(*snd)->SetAirAbsorptionFactor(factor);
}
float ALSoundScript::GetAirAbsorptionFactor() const { return ALSoundBase::GetAirAbsorptionFactor(); }
void ALSoundScript::SetGainAuto(bool directHF, bool send, bool sendHF)
{
	ALSoundBase::SetGainAuto(directHF, send, sendHF);
	for(auto *snd : m_sounds)
		(*snd)->SetGainAuto(directHF, send, sendHF);
}
std::tuple<bool, bool, bool> ALSoundScript::GetGainAuto() const { return ALSoundBase::GetGainAuto(); }
void ALSoundScript::SetDirectFilter(const EffectParams &params)
{
	ALSoundBase::SetDirectFilter(params);
	for(auto *snd : m_sounds)
		(*snd)->SetDirectFilter(params);
}
const ALSound::EffectParams &ALSoundScript::GetDirectFilter() const { return ALSoundBase::GetDirectFilter(); }

bool ALSoundScript::AddEffect(const std::string &effectName, const EffectParams &params)
{
	for(auto *snd : m_sounds)
		(*snd)->AddEffect(effectName, params);
	return true;
}
void ALSoundScript::RemoveEffect(const std::string &effectName)
{
	for(auto *snd : m_sounds)
		(*snd)->RemoveEffect(effectName);
}
void ALSoundScript::SetEffectParameters(const std::string &effectName, const EffectParams &params)
{
	for(auto *snd : m_sounds)
		(*snd)->SetEffectParameters(effectName, params);
}
