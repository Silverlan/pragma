/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_ALSOUND_H__
#define __C_ALSOUND_H__

#include "pragma/clientdefinitions.h"
#include <pragma/audio/alsound.h>
#include <mathutil/glmutil.h>
#include <string>
#include <vector>
#include <alsound_source.hpp>

class ClientState;
#pragma warning(push)
#pragma warning(disable : 4251)
class DLLCLIENT CALSound : public al::SoundSource, public ALSound {
  public:
	static ALSound *FindByServerIndex(uint32_t idx);
	static void SetIndex(ALSound *snd, uint32_t idx);

	CALSound(NetworkState *nw, const al::PSoundChannel &channel);
	virtual ~CALSound() override;
	virtual void OnRelease() override;
	float GetMaxAudibleDistance() const;
	void SetPitchModifier(float mod);
	float GetPitchModifier() const;
	void SetVolumeModifier(float mod);
	float GetVolumeModifier() const;
	virtual unsigned int GetIndex() const override;
	virtual void FadeIn(float time) override;
	virtual void FadeOut(float time) override;
	virtual ALState GetState() const override;
	virtual void Play() override;
	virtual void Stop() override;
	virtual void Pause() override;
	virtual void Rewind() override;
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
	virtual void SetOffset(float sec) override;
	virtual float GetOffset() const override;
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
	virtual void SetOrientation(const Vector3 &at, const Vector3 &up) override;
	virtual std::pair<Vector3, Vector3> GetOrientation() const override;
	virtual void SetDopplerFactor(float factor) override;
	virtual float GetDopplerFactor() const override;
	virtual void SetLeftStereoAngle(float ang) override;
	virtual float GetLeftStereoAngle() const override;
	virtual void SetRightStereoAngle(float ang) override;
	virtual float GetRightStereoAngle() const override;
	virtual void SetAirAbsorptionFactor(float factor) override;
	virtual float GetAirAbsorptionFactor() const override;
	virtual void SetGainAuto(bool directHF, bool send, bool sendHF) override;
	virtual std::tuple<bool, bool, bool> GetGainAuto() const override;
	virtual void SetDirectFilter(const EffectParams &params) override;
	virtual const EffectParams &GetDirectFilter() const override;
	virtual bool AddEffect(const std::string &effectName, const EffectParams &params = {}) override;
	virtual void RemoveEffect(const std::string &effectName) override;
	virtual void SetEffectParameters(const std::string &effectName, const EffectParams &params = {}) override;

	virtual void Update() override;
	virtual void PostUpdate() override;
	virtual void SetType(ALSoundType type) override;
	virtual bool IsIdle() const override;
	void Terminate();

	bool AddEffect(al::IEffect &effect, const EffectParams &params = EffectParams());
	bool AddEffect(al::IEffect &effect, uint32_t &slotId, const EffectParams &params = EffectParams());
	bool AddEffect(al::IEffect &effect, float gain);
	bool AddEffect(al::IEffect &effect, uint32_t &slotId, float gain);
	void RemoveEffect(al::IEffect &effect);
	void RemoveEffect(uint32_t slotId);
  protected:
	friend ClientState;
  protected:
	static std::unordered_map<uint32_t, std::weak_ptr<ALSound>> s_svIndexedSounds; // Sounds created by the server
	float m_modPitch = 1.f;
	float m_modVol = 1.f;
	float m_pitch = 1.f;
	bool m_bTerminated = false;
	virtual void UpdateState() override;
	void UpdateVolume();
	void UpdatePitch();
};
#pragma warning(pop)

#endif
