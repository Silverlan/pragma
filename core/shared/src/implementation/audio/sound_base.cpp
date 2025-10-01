// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "mathutil/uvec.h"

module pragma.shared;

import :audio.sound_base;

float ALSoundBase::GetDuration() const { return m_duration; }

void ALSoundBase::SetPitch(float pitch) { m_pitch = pitch; }
float ALSoundBase::GetPitch() const { return m_pitch; }

void ALSoundBase::SetLooping(bool loop) { m_bLooping = loop; }
bool ALSoundBase::IsLooping() const { return m_bLooping; }

void ALSoundBase::SetGain(float gain) { m_gain = gain; }
float ALSoundBase::GetGain() const { return m_gain; }

void ALSoundBase::SetPosition(const Vector3 &pos) { m_pos = pos; }
Vector3 ALSoundBase::GetPosition() const { return m_pos; }

void ALSoundBase::SetVelocity(const Vector3 &vel) { m_vel = vel; }
Vector3 ALSoundBase::GetVelocity() const { return m_vel; }

void ALSoundBase::SetDirection(const Vector3 &dir) { m_dir = dir; }
Vector3 ALSoundBase::GetDirection() const { return m_dir; }

void ALSoundBase::SetRelative(bool b) { m_bRelative = b; }
bool ALSoundBase::IsRelative() const { return m_bRelative; }

void ALSoundBase::SetReferenceDistance(float dist) { m_refDist = dist; }
float ALSoundBase::GetReferenceDistance() const { return m_refDist; }

float ALSoundBase::GetRolloffFactor() const { return m_rolloff.first; }
void ALSoundBase::SetRolloffFactor(float factor) { m_rolloff.first = factor; }

void ALSoundBase::SetRoomRolloffFactor(float roomFactor) { m_rolloff.second = roomFactor; }
float ALSoundBase::GetRoomRolloffFactor() const { return m_rolloff.second; }

void ALSoundBase::SetMaxDistance(float dist) { m_maxDist = dist; }
float ALSoundBase::GetMaxDistance() const { return m_maxDist; }

void ALSoundBase::SetMinGain(float gain) { m_minGain = gain; }
float ALSoundBase::GetMinGain() const { return m_minGain; }

void ALSoundBase::SetMaxGain(float gain) { m_maxGain = gain; }
float ALSoundBase::GetMaxGain() const { return m_maxGain; }

void ALSoundBase::SetInnerConeAngle(float ang) { m_coneInnerAngle = ang; }
float ALSoundBase::GetInnerConeAngle() const { return m_coneInnerAngle; }

void ALSoundBase::SetOuterConeAngle(float ang) { m_coneOuterAngle = ang; }
float ALSoundBase::GetOuterConeAngle() const { return m_coneOuterAngle; }

void ALSoundBase::SetOuterConeGain(float gain) { m_coneOuterGain.first = gain; }
float ALSoundBase::GetOuterConeGain() const { return m_coneOuterGain.first; }

void ALSoundBase::SetOuterConeGainHF(float gain) { m_coneOuterGain.second = gain; }
float ALSoundBase::GetOuterConeGainHF() const { return m_coneOuterGain.second; }

void ALSoundBase::SetState(ALState state) { m_state = state; }
ALState ALSoundBase::GetState() const { return m_state; }

void ALSoundBase::SetOffset(float offset) { m_offset = offset; }
float ALSoundBase::GetOffset() const { return m_offset; }

void ALSoundBase::SetPriority(uint32_t priority) { m_priority = priority; }
uint32_t ALSoundBase::GetPriority() { return m_priority; }

void ALSoundBase::SetOrientation(const Vector3 &at, const Vector3 &up) { m_orientation = {at, up}; }
std::pair<Vector3, Vector3> ALSoundBase::GetOrientation() const { return m_orientation; }

void ALSoundBase::SetDopplerFactor(float factor) { m_dopplerFactor = factor; }
float ALSoundBase::GetDopplerFactor() const { return m_dopplerFactor; }

void ALSoundBase::SetLeftStereoAngle(float ang) { m_stereoAngles.first = ang; }
float ALSoundBase::GetLeftStereoAngle() const { return m_stereoAngles.first; }

void ALSoundBase::SetRightStereoAngle(float ang) { m_stereoAngles.second = ang; }
float ALSoundBase::GetRightStereoAngle() const { return m_stereoAngles.second; }

void ALSoundBase::SetAirAbsorptionFactor(float factor) { m_airAbsorptionFactor = factor; }
float ALSoundBase::GetAirAbsorptionFactor() const { return m_airAbsorptionFactor; }

void ALSoundBase::SetGainAuto(bool directHF, bool send, bool sendHF) { m_gainAuto = std::tuple<bool, bool, bool> {directHF, send, sendHF}; }
std::tuple<bool, bool, bool> ALSoundBase::GetGainAuto() const { return m_gainAuto; }

void ALSoundBase::SetDirectFilter(const SoundEffectParams &params) { m_directFilter = params; }
const SoundEffectParams &ALSoundBase::GetDirectFilter() const { return m_directFilter; }

bool ALSoundBase::IsPlaying() const { return (GetState() == ALState::Playing) ? true : false; }
bool ALSoundBase::IsPaused() const { return (GetState() == ALState::Paused) ? true : false; }
bool ALSoundBase::IsStopped() const { return (GetState() == ALState::Stopped) ? true : false; }
