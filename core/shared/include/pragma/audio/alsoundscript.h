/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __ALSOUNDSCRIPT_H__
#define __ALSOUNDSCRIPT_H__

#include "pragma/networkdefinitions.h"
#include "pragma/audio/alsound.h"
#include "pragma/audio/alsound_base.hpp"
#include <memory>

class NetworkState;
class SoundScript;
class SSEBase;
class SSESound;
class SoundScriptEvent;
class DLLNETWORK ALSoundScript
	: virtual public ALSound,
	virtual public ALSoundBase
{
protected:
	std::vector<SSEBase*> m_events;
	std::vector<SSESound*> m_sounds;
	NetworkState *m_networkState;
	SoundScript *m_script;
	virtual void SetState(ALState state) override;
	std::unordered_map<unsigned int,Vector3> m_positions;
	bool m_bStream = false;
	virtual std::shared_ptr<ALSound> CreateSound(const std::string &name,ALChannel channel,ALCreateFlags createFlags);
protected:
	bool HandleEvents(SoundScriptEvent *ev,float eventOffset,float lastOffset,float newOffset);
	void InitializeEvent(SoundScriptEvent *ev);
public:
	ALSoundScript(NetworkState *nw,unsigned int idx,SoundScript *script,NetworkState *state,bool bStream);
	virtual ~ALSoundScript() override;
	virtual void Initialize() override;
	virtual ALState GetState() const override;
	void SetTargetPosition(unsigned int id,Vector3 pos);
	virtual bool IsSoundScript() const override;

	uint32_t GetSoundCount() const;
	ALSound *GetSound(uint32_t idx);

	virtual void Play() override;
	virtual void Stop() override;
	virtual void Pause() override;
	virtual void Rewind() override;
	virtual void SetOffset(float offset) override;
	virtual float GetOffset() const override;
	virtual void SetPitch(float pitch) override;
	virtual float GetPitch() const override;
	virtual void SetLooping(bool loop) override;
	virtual bool IsLooping() const override;
	virtual bool IsPlaying() const override;
	virtual bool IsPaused() const override;
	virtual bool IsStopped() const override;
	virtual void SetGain(float gain) override;
	virtual float GetGain() const override;
	virtual void SetPosition(const Vector3 &pos) override;
	virtual Vector3 GetPosition() const override;
	virtual void SetVelocity(const Vector3 &vel) override;
	virtual Vector3 GetVelocity() const override;
	virtual void SetDirection(const Vector3 &dir) override;
	virtual Vector3 GetDirection() const override;
	virtual void SetRelative(bool b) override;
	virtual bool IsRelative() const override;
	virtual float GetDuration() const override;
	virtual float GetReferenceDistance() const override;
	virtual void SetReferenceDistance(float dist) override;
	virtual void SetRoomRolloffFactor(float roomFactor) override;
	virtual float GetRolloffFactor() const override;
	virtual void SetRolloffFactor(float factor) override;
	virtual float GetRoomRolloffFactor() const override;
	virtual float GetMaxDistance() const override;
	virtual void SetMaxDistance(float dist) override;
	virtual float GetMinGain() const override;
	virtual void SetMinGain(float gain) override;
	virtual float GetMaxGain() const override;
	virtual void SetMaxGain(float gain) override;
	virtual float GetInnerConeAngle() const override;
	virtual void SetInnerConeAngle(float ang) override;
	virtual float GetOuterConeAngle() const override;
	virtual void SetOuterConeAngle(float ang) override;
	virtual float GetOuterConeGain() const override;
	virtual float GetOuterConeGainHF() const override;
	virtual void SetOuterConeGain(float gain) override;
	virtual void SetOuterConeGainHF(float gain) override;
	virtual uint32_t GetPriority() override;
	virtual void SetPriority(uint32_t priority) override;
	virtual void SetOrientation(const Vector3 &at,const Vector3 &up) override;
	virtual void SetDopplerFactor(float factor) override;
	virtual float GetDopplerFactor() const override;
	virtual void SetLeftStereoAngle(float ang) override;
	virtual float GetLeftStereoAngle() const override;
	virtual void SetRightStereoAngle(float ang) override;
	virtual float GetRightStereoAngle() const override;
	virtual void SetAirAbsorptionFactor(float factor) override;
	virtual float GetAirAbsorptionFactor() const override;
	virtual void SetGainAuto(bool directHF,bool send,bool sendHF) override;
	virtual std::tuple<bool,bool,bool> GetGainAuto() const override;
	virtual void SetDirectFilter(const EffectParams &params) override;
	virtual const EffectParams &GetDirectFilter() const override;
	virtual bool AddEffect(const std::string &effectName,const EffectParams &params={}) override;
	virtual void RemoveEffect(const std::string &effectName) override;
	virtual void SetEffectParameters(const std::string &effectName,const EffectParams &params={}) override;
	virtual std::pair<Vector3,Vector3> GetOrientation() const override;

	virtual void SetTimeOffset(float sec) override;
	virtual float GetTimeOffset() const override;
	virtual void SetFlags(unsigned int flags) override;
	virtual void Update() override;
	virtual void PostUpdate() override;
	virtual void FadeIn(float time) override;
	virtual void FadeOut(float time) override;
};

#endif