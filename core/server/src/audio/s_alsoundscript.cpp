#include "stdafx_server.h"
#include "pragma/audio/s_alsoundscript.h"
#include <pragma/audio/soundscript_events.h>

extern DLLSERVER ServerState *server;

SALSoundScript::SALSoundScript(NetworkState *nw,unsigned int idx,SoundScript *script,NetworkState *state,bool bStream,bool bShared)
	: ALSoundScript(nw,idx,script,state,bStream),SALSound(nw,idx,0.f,bShared),ALSound(nw)
{}
std::shared_ptr<ALSound> SALSoundScript::CreateSound(const std::string &name,ALChannel channel,ALCreateFlags createFlags)
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
	return server->CreateSound(name,GetType(),flags);
}

ALState SALSoundScript::GetState() const {return SALSound::GetState();}
unsigned int SALSoundScript::GetIndex() const {return SALSound::GetIndex();}
void SALSoundScript::FadeIn(float time)
{
	ALSoundScript::FadeIn(time);
	SALSound::FadeIn(time);
}
void SALSoundScript::FadeOut(float time)
{
	//ALSoundScript::FadeOut(time);
	SALSound::FadeOut(time);
}
void SALSoundScript::Play()
{
	ALSoundScript::Play();
	SendEvent(NetEvent::Play);
	//SALSound::Play();
}
void SALSoundScript::Stop()
{
	ALSoundScript::Stop();
	SendEvent(NetEvent::Stop);
	//SALSound::Stop();
}
void SALSoundScript::Pause()
{
	ALSoundScript::Pause();
	SendEvent(NetEvent::Pause);
	//SALSound::Pause();
}
void SALSoundScript::Rewind()
{
	ALSoundScript::Rewind();
	SendEvent(NetEvent::Rewind);
	//SALSound::Rewind();
}
void SALSoundScript::SetOffset(float offset)
{
	ALSoundScript::SetOffset(offset);
	SALSound::SetOffset(offset);
}
float SALSoundScript::GetOffset() const {return SALSound::GetOffset();}
void SALSoundScript::SetPitch(float pitch)
{
	ALSoundScript::SetPitch(pitch);
	SALSound::SetPitch(pitch);
}
float SALSoundScript::GetPitch() const {return SALSound::GetPitch();}
void SALSoundScript::SetLooping(bool loop)
{
	ALSoundScript::SetLooping(loop);
	SALSound::SetLooping(loop);
}
bool SALSoundScript::IsLooping() const {return SALSound::IsLooping();}
bool SALSoundScript::IsPlaying() const {return SALSound::IsPlaying();}
bool SALSoundScript::IsPaused() const {return SALSound::IsPaused();}
bool SALSoundScript::IsStopped() const {return SALSound::IsStopped();}
void SALSoundScript::SetGain(float gain)
{
	ALSoundScript::SetGain(gain);
	SALSound::SetGain(gain);
}
float SALSoundScript::GetGain() const {return SALSound::GetGain();}
void SALSoundScript::SetPosition(const Vector3 &pos)
{
	ALSoundScript::SetPosition(pos);
	SALSound::SetPosition(pos);
}
Vector3 SALSoundScript::GetPosition() const {return SALSound::GetPosition();}
void SALSoundScript::SetVelocity(const Vector3 &vel)
{
	ALSoundScript::SetVelocity(vel);
	SALSound::SetVelocity(vel);
}
Vector3 SALSoundScript::GetVelocity() const {return SALSound::GetVelocity();}
void SALSoundScript::SetDirection(const Vector3 &dir)
{
	ALSoundScript::SetDirection(dir);
	SALSound::SetDirection(dir);
}
Vector3 SALSoundScript::GetDirection() const {return SALSound::GetDirection();}
void SALSoundScript::SetRelative(bool b)
{
	ALSoundScript::SetRelative(b);
	SALSound::SetRelative(b);
}
bool SALSoundScript::IsRelative() const {return SALSound::IsRelative();}
float SALSoundScript::GetDuration() const {return SALSound::GetDuration();}
float SALSoundScript::GetReferenceDistance() const {return SALSound::GetReferenceDistance();}
void SALSoundScript::SetReferenceDistance(float dist)
{
	ALSoundScript::SetReferenceDistance(dist);
	SALSound::SetReferenceDistance(dist);
}
void SALSoundScript::SetRoomRolloffFactor(float roomFactor)
{
	ALSoundScript::SetRoomRolloffFactor(roomFactor);
	SALSound::SetRoomRolloffFactor(roomFactor);
}
float SALSoundScript::GetRolloffFactor() const {return SALSound::GetRolloffFactor();}
void SALSoundScript::SetRolloffFactor(float factor)
{
	ALSoundScript::SetRolloffFactor(factor);
	SALSound::SetRolloffFactor(factor);
}
float SALSoundScript::GetRoomRolloffFactor() const {return SALSound::GetRoomRolloffFactor();}
float SALSoundScript::GetMaxDistance() const {return SALSound::GetMaxDistance();}
void SALSoundScript::SetMaxDistance(float dist)
{
	ALSoundScript::SetMaxDistance(dist);
	SALSound::SetMaxDistance(dist);
}
float SALSoundScript::GetMinGain() const {return SALSound::GetMinGain();}
void SALSoundScript::SetMinGain(float gain)
{
	ALSoundScript::SetMinGain(gain);
	SALSound::SetMinGain(gain);
}
float SALSoundScript::GetMaxGain() const {return SALSound::GetMaxGain();}
void SALSoundScript::SetMaxGain(float gain)
{
	ALSoundScript::SetMaxGain(gain);
	SALSound::SetMaxGain(gain);
}
float SALSoundScript::GetInnerConeAngle() const {return SALSound::GetInnerConeAngle();}
void SALSoundScript::SetInnerConeAngle(float ang)
{
	ALSoundScript::Update();
	SALSound::Update();
}
float SALSoundScript::GetOuterConeAngle() const {return SALSound::GetOuterConeAngle();}
void SALSoundScript::SetOuterConeAngle(float ang)
{
	ALSoundScript::SetOuterConeAngle(ang);
	SALSound::SetOuterConeAngle(ang);
}
float SALSoundScript::GetOuterConeGain() const {return SALSound::GetOuterConeGain();}
float SALSoundScript::GetOuterConeGainHF() const {return SALSound::GetOuterConeGainHF();}
void SALSoundScript::SetOuterConeGain(float gain)
{
	ALSoundScript::SetOuterConeGain(gain);
	SALSound::SetOuterConeGain(gain);
}
void SALSoundScript::SetOuterConeGainHF(float gain)
{
	ALSoundScript::SetOuterConeGainHF(gain);
	SALSound::SetOuterConeGainHF(gain);
}
uint32_t SALSoundScript::GetPriority() {return SALSound::GetPriority();}
void SALSoundScript::SetPriority(uint32_t priority)
{
	ALSoundScript::SetPriority(priority);
	SALSound::SetPriority(priority);
}
void SALSoundScript::SetOrientation(const Vector3 &at,const Vector3 &up)
{
	ALSoundScript::SetOrientation(at,up);
	SALSound::SetOrientation(at,up);
}
std::pair<Vector3,Vector3> SALSoundScript::GetOrientation() const {return SALSound::GetOrientation();}
void SALSoundScript::SetDopplerFactor(float factor)
{
	ALSoundScript::SetDopplerFactor(factor);
	SALSound::SetDopplerFactor(factor);
}
float SALSoundScript::GetDopplerFactor() const {return SALSound::GetDopplerFactor();}
void SALSoundScript::SetLeftStereoAngle(float ang)
{
	ALSoundScript::SetLeftStereoAngle(ang);
	SALSound::SetLeftStereoAngle(ang);
}
float SALSoundScript::GetLeftStereoAngle() const {return SALSound::GetLeftStereoAngle();}
void SALSoundScript::SetRightStereoAngle(float ang)
{
	ALSoundScript::SetRightStereoAngle(ang);
	SALSound::SetRightStereoAngle(ang);
}
float SALSoundScript::GetRightStereoAngle() const {return SALSound::GetRightStereoAngle();}
void SALSoundScript::SetAirAbsorptionFactor(float factor)
{
	ALSoundScript::SetAirAbsorptionFactor(factor);
	SALSound::SetAirAbsorptionFactor(factor);
}
float SALSoundScript::GetAirAbsorptionFactor() const {return SALSound::GetAirAbsorptionFactor();}
void SALSoundScript::SetGainAuto(bool directHF,bool send,bool sendHF)
{
	ALSoundScript::SetGainAuto(directHF,send,sendHF);
	SALSound::SetGainAuto(directHF,send,sendHF);
}
std::tuple<bool,bool,bool> SALSoundScript::GetGainAuto() const {return SALSound::GetGainAuto();}
void SALSoundScript::SetDirectFilter(const EffectParams &params) {SALSound::SetDirectFilter(params);}
const ALSound::EffectParams &SALSoundScript::GetDirectFilter() const {return SALSound::GetDirectFilter();}
bool SALSoundScript::AddEffect(const std::string &effectName,const EffectParams &params)
{
	ALSoundScript::AddEffect(effectName,params);
	return SALSound::AddEffect(effectName,params);
}
void SALSoundScript::RemoveEffect(const std::string &effectName)
{
	ALSoundScript::RemoveEffect(effectName);
	SALSound::RemoveEffect(effectName);
}
void SALSoundScript::SetEffectParameters(const std::string &effectName,const EffectParams &params)
{
	ALSoundScript::SetEffectParameters(effectName,params);
	SALSound::SetEffectParameters(effectName,params);
}

void SALSoundScript::SetType(ALSoundType type)
{
	ALSoundScript::SetType(type);
	SALSound::SetType(type);
}
void SALSoundScript::SetFlags(unsigned int flags)
{
	ALSoundScript::SetFlags(flags);
	SALSound::SetFlags(flags);
}
void SALSoundScript::SetSource(BaseEntity *ent)
{
	ALSoundScript::SetSource(ent);
	SALSound::SetSource(ent);
}
void SALSoundScript::Update()
{
	ALSoundScript::Update();
	SALSound::Update();
}
void SALSoundScript::PostUpdate()
{
	ALSoundScript::PostUpdate();
	SALSound::PostUpdate();
}
void SALSoundScript::SetRange(float start,float end)
{
	ALSoundScript::SetRange(start,end);
	SALSound::SetRange(start,end);
}
void SALSoundScript::ClearRange()
{
	ALSoundScript::ClearRange();
	SALSound::ClearRange();
}
void SALSoundScript::SetFadeInDuration(float t)
{
	ALSoundScript::SetFadeInDuration(t);
	SALSound::SetFadeInDuration(t);
}
void SALSoundScript::SetFadeOutDuration(float t)
{
	ALSoundScript::SetFadeOutDuration(t);
	SALSound::SetFadeOutDuration(t);
}
void SALSoundScript::SetState(ALState state) {SALSound::SetState(state);}
