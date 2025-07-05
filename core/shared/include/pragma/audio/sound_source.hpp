// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __SOUND_SOURCE_HPP__
#define __SOUND_SOURCE_HPP__

#include "pragma/clientdefinitions.h"
#include <sharedutils/util_shared_handle.hpp>
#include <mathutil/uvec.h>

namespace pragma::audio {
	class ISoundSystem;
	class ISoundBuffer;
	class IDecoder;
	class IEffect;
	class DLLCLIENT ISoundSource {
	  public:
		virtual ~ISoundSource() = default;

		ISoundSource(ISoundSystem &system, ISoundBuffer &buffer);
		ISoundSource(ISoundSystem &system, IDecoder &decoder);

		const ISoundBuffer *GetBuffer() const;
		ISoundBuffer *GetBuffer();
		const IDecoder *GetDecoder() const;
		IDecoder *GetDecoder();
		util::TWeakSharedHandle<ISoundSource> GetHandle() const;
		ISoundSystem &GetSoundSystem() const;

		double GetDuration() const;
		bool IsMono() const;
		bool IsStereo() const;

		virtual void Play() = 0;
		virtual void Stop() = 0;
		virtual void Pause() = 0;
		virtual void Resume() = 0;
		virtual bool IsPlaying() const = 0;
		virtual bool IsPaused() const = 0;
		virtual bool IsStopped() const = 0;

		virtual void SetPriority(uint32_t priority) = 0;
		virtual uint32_t GetPriority() const = 0;

		virtual void SetOffset(double offset) = 0;
		virtual double GetOffset() const = 0;

		virtual void SetLooping(bool bLoop) = 0;
		virtual bool IsLooping() const = 0;

		virtual void SetPitch(float pitch) = 0;
		virtual float GetPitch() const = 0;

		virtual void SetGain(float gain) = 0;
		virtual float GetGain() const = 0;

		void SetGainRange(float minGain, float maxGain);
		std::pair<float, float> GetGainRange() const;
		virtual void SetMinGain(float gain) = 0;
		virtual float GetMinGain() const = 0;
		virtual void SetMaxGain(float gain) = 0;
		virtual float GetMaxGain() const = 0;

		virtual void SetDistanceRange(float refDist, float maxDist) = 0;
		virtual std::pair<float, float> GetDistanceRange() const = 0;
		virtual void SetReferenceDistance(float dist) = 0;
		virtual float GetReferenceDistance() const = 0;
		virtual void SetMaxDistance(float dist) = 0;
		virtual float GetMaxDistance() const = 0;

		virtual void SetPosition(const Vector3 &pos) = 0;
		virtual Vector3 GetPosition() const = 0;

		Vector3 GetWorldPosition() const;

		virtual void SetVelocity(const Vector3 &vel) = 0;
		virtual Vector3 GetVelocity() const = 0;

		virtual void SetDirection(const Vector3 &dir) = 0;
		virtual Vector3 GetDirection() const = 0;

		virtual void SetOrientation(const Vector3 &at, const Vector3 &up) = 0;
		virtual std::pair<Vector3, Vector3> GetOrientation() const = 0;

		virtual void SetConeAngles(float inner, float outer) = 0;
		virtual std::pair<float, float> GetConeAngles() const = 0;
		virtual void SetInnerConeAngle(float inner) = 0;
		virtual float GetInnerConeAngle() const = 0;
		virtual void SetOuterConeAngle(float outer) = 0;
		virtual float GetOuterConeAngle() const = 0;

		virtual void SetOuterConeGains(float gain, float gainHF = 1.f) = 0;
		virtual void SetOuterConeGain(float gain) = 0;
		virtual void SetOuterConeGainHF(float gain) = 0;
		virtual std::pair<float, float> GetOuterConeGains() const = 0;
		virtual float GetOuterConeGain() const = 0;
		virtual float GetOuterConeGainHF() const = 0;

		virtual void SetRolloffFactors(float factor, float roomFactor = 0.f) = 0;
		virtual void SetRoomRolloffFactor(float roomFactor) = 0;
		virtual std::pair<float, float> GetRolloffFactors() const = 0;
		virtual float GetRolloffFactor() const = 0;
		virtual float GetRoomRolloffFactor() const = 0;

		virtual void SetDopplerFactor(float factor) = 0;
		virtual float GetDopplerFactor() const = 0;

		virtual void SetRelative(bool bRelative) = 0;
		virtual bool IsRelative() const = 0;

		virtual void SetRadius(float radius) = 0;
		virtual float GetRadius() const = 0;

		virtual void SetStereoAngles(float leftAngle, float rightAngle) = 0;
		virtual std::pair<float, float> GetStereoAngles() const = 0;
		virtual void SetLeftStereoAngle(float ang) = 0;
		virtual float GetLeftStereoAngle() const = 0;
		virtual void SetRightStereoAngle(float ang) = 0;
		virtual float GetRightStereoAngle() const = 0;

		virtual void SetAirAbsorptionFactor(float factor) = 0;
		virtual float GetAirAbsorptionFactor() const = 0;

		virtual void SetGainAuto(bool directHF, bool send, bool sendHF) = 0;
		virtual std::tuple<bool, bool, bool> GetGainAuto() const = 0;
		virtual bool GetDirectGainHFAuto() const = 0;
		virtual bool GetSendGainAuto() const = 0;
		virtual bool GetSendGainHFAuto() const = 0;

		virtual float GetMaxAudibleDistance() const = 0;

		virtual void SetDirectFilter(const Effect::Params &params) = 0;
		virtual const Effect::Params &GetDirectFilter() const = 0;
		virtual bool AddEffect(IEffect &effect, const Effect::Params &params = Effect::Params()) = 0;
		virtual bool AddEffect(IEffect &effect, uint32_t &slotId, const Effect::Params &params = Effect::Params()) = 0;
		virtual bool AddEffect(IEffect &effect, float gain) = 0;
		virtual bool AddEffect(IEffect &effect, uint32_t &slotId, float gain) = 0;
		virtual void RemoveEffect(IEffect &effect) = 0;
		virtual void RemoveEffect(uint32_t slotId) = 0;
		virtual void RemoveEffects() = 0;
		virtual void SetEffectParameters(IEffect &effect, const Effect::Params &params) = 0;
		virtual void SetEffectParameters(uint32_t slotId, const Effect::Params &params) = 0;
		virtual void SetEffectGain(IEffect &effect, float gain) = 0;
		virtual void SetEffectGain(uint32_t slotId, float gain) = 0;
		virtual std::vector<IEffect *> GetEffects() = 0;

		virtual void Update() = 0;
		virtual bool IsIdle() const = 0; // If true is returned, and no references to the sound exist anymore, the sound will be released by the sound system

		void SetIdentifier(const std::string &identifier);
		const std::string &GetIdentifier() const;
	  private:
		ISoundSystem &m_system;
		std::weak_ptr<ISoundBuffer> m_buffer = {};
		std::string m_identifier = {};
		bool m_bReady = true;
		bool m_bSchedulePlay = false;
		std::vector<std::pair<std::shared_ptr<IEffect>, uint32_t>> m_effects;
		PDecoder m_decoder = nullptr;
		mutable util::TWeakSharedHandle<ISoundSource> m_handle = {};

		uint32_t m_nextAuxSlot = 0;
		std::queue<uint32_t> m_freeAuxEffectIds;

		virtual void OnReady();
		impl::BufferBase *GetBaseBuffer() const;
		void RemoveInternalEffect(decltype(m_effects)::iterator it);
	};
	using PSoundSource = util::TSharedHandle<ISoundSource>;
	using HSoundSource = std::TWeakSharedHandle<ISoundSource>;
};
#pragma warning(pop)

#endif
