// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :audio.sound_base;

float pragma::audio::ALSoundBase::GetDuration() const { return m_duration; }

void pragma::audio::ALSoundBase::SetPitch(float pitch) { m_pitch = pitch; }
float pragma::audio::ALSoundBase::GetPitch() const { return m_pitch; }

void pragma::audio::ALSoundBase::SetLooping(bool loop) { m_bLooping = loop; }
bool pragma::audio::ALSoundBase::IsLooping() const { return m_bLooping; }

void pragma::audio::ALSoundBase::SetGain(float gain) { m_gain = gain; }
float pragma::audio::ALSoundBase::GetGain() const { return m_gain; }

void pragma::audio::ALSoundBase::SetPosition(const Vector3 &pos) { m_pos = pos; }
Vector3 pragma::audio::ALSoundBase::GetPosition() const { return m_pos; }

void pragma::audio::ALSoundBase::SetVelocity(const Vector3 &vel) { m_vel = vel; }
Vector3 pragma::audio::ALSoundBase::GetVelocity() const { return m_vel; }

void pragma::audio::ALSoundBase::SetDirection(const Vector3 &dir) { m_dir = dir; }
Vector3 pragma::audio::ALSoundBase::GetDirection() const { return m_dir; }

void pragma::audio::ALSoundBase::SetRelative(bool b) { m_bRelative = b; }
bool pragma::audio::ALSoundBase::IsRelative() const { return m_bRelative; }

void pragma::audio::ALSoundBase::SetReferenceDistance(float dist) { m_refDist = dist; }
float pragma::audio::ALSoundBase::GetReferenceDistance() const { return m_refDist; }

float pragma::audio::ALSoundBase::GetRolloffFactor() const { return m_rolloff.first; }
void pragma::audio::ALSoundBase::SetRolloffFactor(float factor) { m_rolloff.first = factor; }

void pragma::audio::ALSoundBase::SetRoomRolloffFactor(float roomFactor) { m_rolloff.second = roomFactor; }
float pragma::audio::ALSoundBase::GetRoomRolloffFactor() const { return m_rolloff.second; }

void pragma::audio::ALSoundBase::SetMaxDistance(float dist) { m_maxDist = dist; }
float pragma::audio::ALSoundBase::GetMaxDistance() const { return m_maxDist; }

void pragma::audio::ALSoundBase::SetMinGain(float gain) { m_minGain = gain; }
float pragma::audio::ALSoundBase::GetMinGain() const { return m_minGain; }

void pragma::audio::ALSoundBase::SetMaxGain(float gain) { m_maxGain = gain; }
float pragma::audio::ALSoundBase::GetMaxGain() const { return m_maxGain; }

void pragma::audio::ALSoundBase::SetInnerConeAngle(float ang) { m_coneInnerAngle = ang; }
float pragma::audio::ALSoundBase::GetInnerConeAngle() const { return m_coneInnerAngle; }

void pragma::audio::ALSoundBase::SetOuterConeAngle(float ang) { m_coneOuterAngle = ang; }
float pragma::audio::ALSoundBase::GetOuterConeAngle() const { return m_coneOuterAngle; }

void pragma::audio::ALSoundBase::SetOuterConeGain(float gain) { m_coneOuterGain.first = gain; }
float pragma::audio::ALSoundBase::GetOuterConeGain() const { return m_coneOuterGain.first; }

void pragma::audio::ALSoundBase::SetOuterConeGainHF(float gain) { m_coneOuterGain.second = gain; }
float pragma::audio::ALSoundBase::GetOuterConeGainHF() const { return m_coneOuterGain.second; }

void pragma::audio::ALSoundBase::SetState(ALState state) { m_state = state; }
pragma::audio::ALState pragma::audio::ALSoundBase::GetState() const { return m_state; }

void pragma::audio::ALSoundBase::SetOffset(float offset) { m_offset = offset; }
float pragma::audio::ALSoundBase::GetOffset() const { return m_offset; }

void pragma::audio::ALSoundBase::SetPriority(uint32_t priority) { m_priority = priority; }
uint32_t pragma::audio::ALSoundBase::GetPriority() { return m_priority; }

void pragma::audio::ALSoundBase::SetOrientation(const Vector3 &at, const Vector3 &up) { m_orientation = {at, up}; }
std::pair<Vector3, Vector3> pragma::audio::ALSoundBase::GetOrientation() const { return m_orientation; }

void pragma::audio::ALSoundBase::SetDopplerFactor(float factor) { m_dopplerFactor = factor; }
float pragma::audio::ALSoundBase::GetDopplerFactor() const { return m_dopplerFactor; }

void pragma::audio::ALSoundBase::SetLeftStereoAngle(float ang) { m_stereoAngles.first = ang; }
float pragma::audio::ALSoundBase::GetLeftStereoAngle() const { return m_stereoAngles.first; }

void pragma::audio::ALSoundBase::SetRightStereoAngle(float ang) { m_stereoAngles.second = ang; }
float pragma::audio::ALSoundBase::GetRightStereoAngle() const { return m_stereoAngles.second; }

void pragma::audio::ALSoundBase::SetAirAbsorptionFactor(float factor) { m_airAbsorptionFactor = factor; }
float pragma::audio::ALSoundBase::GetAirAbsorptionFactor() const { return m_airAbsorptionFactor; }

void pragma::audio::ALSoundBase::SetGainAuto(bool directHF, bool send, bool sendHF) { m_gainAuto = std::tuple<bool, bool, bool> {directHF, send, sendHF}; }
std::tuple<bool, bool, bool> pragma::audio::ALSoundBase::GetGainAuto() const { return m_gainAuto; }

void pragma::audio::ALSoundBase::SetDirectFilter(const SoundEffectParams &params) { m_directFilter = params; }
const pragma::audio::SoundEffectParams &pragma::audio::ALSoundBase::GetDirectFilter() const { return m_directFilter; }

bool pragma::audio::ALSoundBase::IsPlaying() const { return (GetState() == ALState::Playing) ? true : false; }
bool pragma::audio::ALSoundBase::IsPaused() const { return (GetState() == ALState::Paused) ? true : false; }
bool pragma::audio::ALSoundBase::IsStopped() const { return (GetState() == ALState::Stopped) ? true : false; }
