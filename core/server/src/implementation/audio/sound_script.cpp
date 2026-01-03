// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :audio.sound_script;

import :server_state;

pragma::audio::SALSoundScript::SALSoundScript(NetworkState *nw, unsigned int idx, SoundScript *script, NetworkState *state, const std::string &soundName, ALCreateFlags createFlags)
    : ALSoundScript(nw, idx, script, state, math::is_flag_set(createFlags, ALCreateFlags::Stream)), SALSound(nw, idx, 0.f, soundName, createFlags), ALSound(nw)
{
}
std::shared_ptr<pragma::audio::ALSound> pragma::audio::SALSoundScript::CreateSound(const std::string &name, ALChannel channel, ALCreateFlags createFlags)
{
	auto flags = ALCreateFlags::None;
	if(channel == ALChannel::Mono)
		flags |= ALCreateFlags::Mono;
	if(m_bStream == true)
		flags |= ALCreateFlags::Stream;

	//if(IsShared() == false)
	// Child sounds are never transmitted to clients as they will be handled by the client's sound-script object independently
	flags |= ALCreateFlags::DontTransmit;

	flags |= createFlags;
	return ServerState::Get()->CreateSound(name, GetType(), flags);
}

pragma::audio::ALState pragma::audio::SALSoundScript::GetState() const { return SALSound::GetState(); }
unsigned int pragma::audio::SALSoundScript::GetIndex() const { return SALSound::GetIndex(); }
void pragma::audio::SALSoundScript::FadeIn(float time)
{
	ALSoundScript::FadeIn(time);
	SALSound::FadeIn(time);
}
void pragma::audio::SALSoundScript::FadeOut(float time)
{
	//ALSoundScript::FadeOut(time);
	SALSound::FadeOut(time);
}
void pragma::audio::SALSoundScript::Play()
{
	ALSoundScript::Play();
	SendEvent(NetEvent::Play);
	//SALSound::Play();
}
void pragma::audio::SALSoundScript::Stop()
{
	ALSoundScript::Stop();
	SendEvent(NetEvent::Stop);
	//SALSound::Stop();
}
void pragma::audio::SALSoundScript::Pause()
{
	ALSoundScript::Pause();
	SendEvent(NetEvent::Pause);
	//SALSound::Pause();
}
void pragma::audio::SALSoundScript::Rewind()
{
	ALSoundScript::Rewind();
	SendEvent(NetEvent::Rewind);
	//SALSound::Rewind();
}
void pragma::audio::SALSoundScript::SetOffset(float offset)
{
	ALSoundScript::SetOffset(offset);
	SALSound::SetOffset(offset);
}
float pragma::audio::SALSoundScript::GetOffset() const { return SALSound::GetOffset(); }
void pragma::audio::SALSoundScript::SetPitch(float pitch)
{
	ALSoundScript::SetPitch(pitch);
	SALSound::SetPitch(pitch);
}
float pragma::audio::SALSoundScript::GetPitch() const { return SALSound::GetPitch(); }
void pragma::audio::SALSoundScript::SetLooping(bool loop)
{
	ALSoundScript::SetLooping(loop);
	SALSound::SetLooping(loop);
}
bool pragma::audio::SALSoundScript::IsLooping() const { return SALSound::IsLooping(); }
bool pragma::audio::SALSoundScript::IsPlaying() const { return SALSound::IsPlaying(); }
bool pragma::audio::SALSoundScript::IsPaused() const { return SALSound::IsPaused(); }
bool pragma::audio::SALSoundScript::IsStopped() const { return SALSound::IsStopped(); }
void pragma::audio::SALSoundScript::SetGain(float gain)
{
	ALSoundScript::SetGain(gain);
	SALSound::SetGain(gain);
}
float pragma::audio::SALSoundScript::GetGain() const { return SALSound::GetGain(); }
void pragma::audio::SALSoundScript::SetPosition(const Vector3 &pos)
{
	ALSoundScript::SetPosition(pos);
	SALSound::SetPosition(pos);
}
Vector3 pragma::audio::SALSoundScript::GetPosition() const { return SALSound::GetPosition(); }
void pragma::audio::SALSoundScript::SetVelocity(const Vector3 &vel)
{
	ALSoundScript::SetVelocity(vel);
	SALSound::SetVelocity(vel);
}
Vector3 pragma::audio::SALSoundScript::GetVelocity() const { return SALSound::GetVelocity(); }
void pragma::audio::SALSoundScript::SetDirection(const Vector3 &dir)
{
	ALSoundScript::SetDirection(dir);
	SALSound::SetDirection(dir);
}
Vector3 pragma::audio::SALSoundScript::GetDirection() const { return SALSound::GetDirection(); }
void pragma::audio::SALSoundScript::SetRelative(bool b)
{
	ALSoundScript::SetRelative(b);
	SALSound::SetRelative(b);
}
bool pragma::audio::SALSoundScript::IsRelative() const { return SALSound::IsRelative(); }
float pragma::audio::SALSoundScript::GetDuration() const { return SALSound::GetDuration(); }
float pragma::audio::SALSoundScript::GetReferenceDistance() const { return SALSound::GetReferenceDistance(); }
void pragma::audio::SALSoundScript::SetReferenceDistance(float dist)
{
	ALSoundScript::SetReferenceDistance(dist);
	SALSound::SetReferenceDistance(dist);
}
void pragma::audio::SALSoundScript::SetRoomRolloffFactor(float roomFactor)
{
	ALSoundScript::SetRoomRolloffFactor(roomFactor);
	SALSound::SetRoomRolloffFactor(roomFactor);
}
float pragma::audio::SALSoundScript::GetRolloffFactor() const { return SALSound::GetRolloffFactor(); }
void pragma::audio::SALSoundScript::SetRolloffFactor(float factor)
{
	ALSoundScript::SetRolloffFactor(factor);
	SALSound::SetRolloffFactor(factor);
}
float pragma::audio::SALSoundScript::GetRoomRolloffFactor() const { return SALSound::GetRoomRolloffFactor(); }
float pragma::audio::SALSoundScript::GetMaxDistance() const { return SALSound::GetMaxDistance(); }
void pragma::audio::SALSoundScript::SetMaxDistance(float dist)
{
	ALSoundScript::SetMaxDistance(dist);
	SALSound::SetMaxDistance(dist);
}
float pragma::audio::SALSoundScript::GetMinGain() const { return SALSound::GetMinGain(); }
void pragma::audio::SALSoundScript::SetMinGain(float gain)
{
	ALSoundScript::SetMinGain(gain);
	SALSound::SetMinGain(gain);
}
float pragma::audio::SALSoundScript::GetMaxGain() const { return SALSound::GetMaxGain(); }
void pragma::audio::SALSoundScript::SetMaxGain(float gain)
{
	ALSoundScript::SetMaxGain(gain);
	SALSound::SetMaxGain(gain);
}
float pragma::audio::SALSoundScript::GetInnerConeAngle() const { return SALSound::GetInnerConeAngle(); }
void pragma::audio::SALSoundScript::SetInnerConeAngle(float ang)
{
	ALSoundScript::Update();
	SALSound::Update();
}
float pragma::audio::SALSoundScript::GetOuterConeAngle() const { return SALSound::GetOuterConeAngle(); }
void pragma::audio::SALSoundScript::SetOuterConeAngle(float ang)
{
	ALSoundScript::SetOuterConeAngle(ang);
	SALSound::SetOuterConeAngle(ang);
}
float pragma::audio::SALSoundScript::GetOuterConeGain() const { return SALSound::GetOuterConeGain(); }
float pragma::audio::SALSoundScript::GetOuterConeGainHF() const { return SALSound::GetOuterConeGainHF(); }
void pragma::audio::SALSoundScript::SetOuterConeGain(float gain)
{
	ALSoundScript::SetOuterConeGain(gain);
	SALSound::SetOuterConeGain(gain);
}
void pragma::audio::SALSoundScript::SetOuterConeGainHF(float gain)
{
	ALSoundScript::SetOuterConeGainHF(gain);
	SALSound::SetOuterConeGainHF(gain);
}
uint32_t pragma::audio::SALSoundScript::GetPriority() { return SALSound::GetPriority(); }
void pragma::audio::SALSoundScript::SetPriority(uint32_t priority)
{
	ALSoundScript::SetPriority(priority);
	SALSound::SetPriority(priority);
}
void pragma::audio::SALSoundScript::SetOrientation(const Vector3 &at, const Vector3 &up)
{
	ALSoundScript::SetOrientation(at, up);
	SALSound::SetOrientation(at, up);
}
std::pair<Vector3, Vector3> pragma::audio::SALSoundScript::GetOrientation() const { return SALSound::GetOrientation(); }
void pragma::audio::SALSoundScript::SetDopplerFactor(float factor)
{
	ALSoundScript::SetDopplerFactor(factor);
	SALSound::SetDopplerFactor(factor);
}
float pragma::audio::SALSoundScript::GetDopplerFactor() const { return SALSound::GetDopplerFactor(); }
void pragma::audio::SALSoundScript::SetLeftStereoAngle(float ang)
{
	ALSoundScript::SetLeftStereoAngle(ang);
	SALSound::SetLeftStereoAngle(ang);
}
float pragma::audio::SALSoundScript::GetLeftStereoAngle() const { return SALSound::GetLeftStereoAngle(); }
void pragma::audio::SALSoundScript::SetRightStereoAngle(float ang)
{
	ALSoundScript::SetRightStereoAngle(ang);
	SALSound::SetRightStereoAngle(ang);
}
float pragma::audio::SALSoundScript::GetRightStereoAngle() const { return SALSound::GetRightStereoAngle(); }
void pragma::audio::SALSoundScript::SetAirAbsorptionFactor(float factor)
{
	ALSoundScript::SetAirAbsorptionFactor(factor);
	SALSound::SetAirAbsorptionFactor(factor);
}
float pragma::audio::SALSoundScript::GetAirAbsorptionFactor() const { return SALSound::GetAirAbsorptionFactor(); }
void pragma::audio::SALSoundScript::SetGainAuto(bool directHF, bool send, bool sendHF)
{
	ALSoundScript::SetGainAuto(directHF, send, sendHF);
	SALSound::SetGainAuto(directHF, send, sendHF);
}
std::tuple<bool, bool, bool> pragma::audio::SALSoundScript::GetGainAuto() const { return SALSound::GetGainAuto(); }
void pragma::audio::SALSoundScript::SetDirectFilter(const SoundEffectParams &params) { SALSound::SetDirectFilter(params); }
const pragma::audio::SoundEffectParams &pragma::audio::SALSoundScript::GetDirectFilter() const { return SALSound::GetDirectFilter(); }
bool pragma::audio::SALSoundScript::AddEffect(const std::string &effectName, const SoundEffectParams &params)
{
	ALSoundScript::AddEffect(effectName, params);
	return SALSound::AddEffect(effectName, params);
}
void pragma::audio::SALSoundScript::RemoveEffect(const std::string &effectName)
{
	ALSoundScript::RemoveEffect(effectName);
	SALSound::RemoveEffect(effectName);
}
void pragma::audio::SALSoundScript::SetEffectParameters(const std::string &effectName, const SoundEffectParams &params)
{
	ALSoundScript::SetEffectParameters(effectName, params);
	SALSound::SetEffectParameters(effectName, params);
}

void pragma::audio::SALSoundScript::SetType(ALSoundType type)
{
	ALSoundScript::SetType(type);
	SALSound::SetType(type);
}
void pragma::audio::SALSoundScript::SetFlags(unsigned int flags)
{
	ALSoundScript::SetFlags(flags);
	SALSound::SetFlags(flags);
}
void pragma::audio::SALSoundScript::SetSource(ecs::BaseEntity *ent)
{
	ALSoundScript::SetSource(ent);
	SALSound::SetSource(ent);
}
void pragma::audio::SALSoundScript::Update()
{
	ALSoundScript::Update();
	SALSound::Update();
}
void pragma::audio::SALSoundScript::PostUpdate()
{
	ALSoundScript::PostUpdate();
	SALSound::PostUpdate();
}
void pragma::audio::SALSoundScript::SetRange(float start, float end)
{
	ALSoundScript::SetRange(start, end);
	SALSound::SetRange(start, end);
}
void pragma::audio::SALSoundScript::ClearRange()
{
	ALSoundScript::ClearRange();
	SALSound::ClearRange();
}
void pragma::audio::SALSoundScript::SetFadeInDuration(float t)
{
	ALSoundScript::SetFadeInDuration(t);
	SALSound::SetFadeInDuration(t);
}
void pragma::audio::SALSoundScript::SetFadeOutDuration(float t)
{
	ALSoundScript::SetFadeOutDuration(t);
	SALSound::SetFadeOutDuration(t);
}
void pragma::audio::SALSoundScript::SetState(ALState state) { SALSound::SetState(state); }
