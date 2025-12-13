// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:audio.sound;

export import :audio.effect_params;
export import :audio.enums;
export import :entities.base_entity_handle;
export import :scripting.lua.callback_handler;

export namespace pragma {
	class NetworkState;
}
export namespace pragma::audio {
	struct DLLNETWORK SoundFade {
		SoundFade(bool fadein, double start, double duration, float gain)
		{
			this->fadein = fadein;
			this->start = start;
			this->duration = duration;
			this->gain = gain;
		}
		bool fadein;
		double start;
		double duration;
		float gain;
	};

#pragma warning(push)
#pragma warning(disable : 4251)
	class DLLNETWORK ALSound : virtual public util::inheritable_enable_shared_from_this<ALSound>, virtual public util::CallbackHandler, public LuaCallbackHandler {
	  public:
		enum class NetEvent : uint8_t {
			Play = 0,
			Stop,
			Pause,
			Rewind,
			SetOffset,
			SetPitch,
			SetLooping,
			SetGain,
			SetPos,
			SetVelocity,
			SetDirection,
			SetRelative,
			SetReferenceDistance,
			SetRolloffFactor,
			SetRoomRolloffFactor,
			SetMaxDistance,
			SetMinGain,
			SetMaxGain,
			SetConeInnerAngle,
			SetConeOuterAngle,
			SetConeOuterGain,
			SetConeOuterGainHF,
			SetFlags,
			SetType,
			SetSource,
			SetRange,
			ClearRange,
			SetFadeInDuration,
			SetFadeOutDuration,
			FadeIn,
			FadeOut,
			SetIndex,
			SetPriority,
			SetOrientation,
			SetDopplerFactor,
			SetLeftStereoAngle,
			SetRightStereoAngle,
			SetAirAbsorptionFactor,
			SetGainAuto,
			SetDirectFilter,
			AddEffect,
			RemoveEffect,
			SetEffectParameters,
			SetEntityMapIndex
		};
	  protected:
		unsigned int m_index = 0;
		void CheckStateChange(ALState old);
		float m_lastOffset = 0.f;
		float m_lastSecOffset = 0.f;
		float m_gain = 1.f;
		float m_tFadeOut = 0.f;
		float m_tFadeIn = 0.f;
		std::unique_ptr<std::pair<float, float>> m_range = nullptr;
		EntityHandle m_hSourceEntity;
		ALSoundType m_type;
		unsigned int m_flags = 0;
		mutable NetworkState *m_networkState = nullptr;
		std::unique_ptr<SoundFade> m_fade = nullptr;

		void CancelFade();
		virtual void UpdateState();
		void UpdateOffset();
		void InitRange();
	  public:
		ALSound(NetworkState *nw);
		virtual ~ALSound();
		virtual void Initialize();
		virtual void OnRelease();
		virtual unsigned int GetIndex() const;
		void SetIndex(unsigned int idx);
		virtual void Update() = 0;
		virtual void PostUpdate();
		virtual ALState GetState() const;
		virtual bool IsSoundScript() const;
		NetworkState *GetNetworkState() const;
		virtual float GetMaxAudibleDistance() const;
		float GetSoundIntensity(const Vector3 &pos) const;
		virtual void SetSource(ecs::BaseEntity *ent);
		ecs::BaseEntity *GetSource() const;
		virtual void SetRange(float start, float end);
		virtual void ClearRange();
		bool HasRange() const;
		std::pair<float, float> GetRange() const;
		std::pair<float, float> GetRangeOffsets() const;
		virtual void SetFadeInDuration(float t);
		virtual void SetFadeOutDuration(float t);
		float GetFadeInDuration() const;
		float GetFadeOutDuration() const;
		virtual void FadeIn(float time) = 0;
		virtual void FadeOut(float time) = 0;
		virtual void Play() = 0;
		virtual void Stop() = 0;
		virtual void Pause() = 0;
		virtual void Rewind() = 0;
		virtual void SetOffset(float offset) = 0;
		virtual float GetOffset() const = 0;
		virtual void SetPitch(float pitch) = 0;
		virtual float GetPitch() const = 0;
		virtual void SetLooping(bool loop) = 0;
		virtual bool IsLooping() const = 0;
		virtual bool IsPlaying() const = 0;
		virtual bool IsPaused() const = 0;
		virtual bool IsStopped() const = 0;
		virtual void SetGain(float gain) = 0;
		virtual float GetGain() const = 0;
		virtual void SetPosition(const Vector3 &pos) = 0;
		virtual Vector3 GetPosition() const = 0;
		virtual void SetVelocity(const Vector3 &vel) = 0;
		virtual Vector3 GetVelocity() const = 0;
		virtual void SetDirection(const Vector3 &dir) = 0;
		virtual Vector3 GetDirection() const = 0;
		virtual void SetRelative(bool b) = 0;
		virtual bool IsRelative() const = 0;
		virtual float GetDuration() const = 0;
		virtual float GetReferenceDistance() const = 0;
		virtual void SetReferenceDistance(float dist) = 0;
		virtual void SetRoomRolloffFactor(float roomFactor) = 0;
		virtual float GetRolloffFactor() const = 0;
		virtual void SetRolloffFactor(float factor) = 0;
		virtual float GetRoomRolloffFactor() const = 0;
		virtual float GetMaxDistance() const = 0;
		virtual void SetMaxDistance(float dist) = 0;
		virtual float GetMinGain() const = 0;
		virtual void SetMinGain(float gain) = 0;
		virtual float GetMaxGain() const = 0;
		virtual void SetMaxGain(float gain) = 0;
		virtual float GetInnerConeAngle() const = 0;
		virtual void SetInnerConeAngle(float ang) = 0;
		virtual float GetOuterConeAngle() const = 0;
		virtual void SetOuterConeAngle(float ang) = 0;
		virtual float GetOuterConeGain() const = 0;
		virtual float GetOuterConeGainHF() const = 0;
		virtual void SetOuterConeGain(float gain) = 0;
		virtual void SetOuterConeGainHF(float gain) = 0;
		virtual uint32_t GetPriority() = 0;
		virtual void SetPriority(uint32_t priority) = 0;
		virtual void SetOrientation(const Vector3 &at, const Vector3 &up) = 0;
		virtual std::pair<Vector3, Vector3> GetOrientation() const = 0;
		virtual void SetDopplerFactor(float factor) = 0;
		virtual float GetDopplerFactor() const = 0;
		virtual void SetLeftStereoAngle(float ang) = 0;
		virtual float GetLeftStereoAngle() const = 0;
		virtual void SetRightStereoAngle(float ang) = 0;
		virtual float GetRightStereoAngle() const = 0;
		virtual void SetAirAbsorptionFactor(float factor) = 0;
		virtual float GetAirAbsorptionFactor() const = 0;
		virtual void SetGainAuto(bool directHF, bool send, bool sendHF) = 0;
		virtual std::tuple<bool, bool, bool> GetGainAuto() const = 0;
		virtual void SetDirectFilter(const SoundEffectParams &params) = 0;
		virtual const SoundEffectParams &GetDirectFilter() const = 0;
		virtual bool AddEffect(const std::string &effectName, const SoundEffectParams &params = {}) = 0;
		virtual void RemoveEffect(const std::string &effectName) = 0;
		virtual void SetEffectParameters(const std::string &effectName, const SoundEffectParams &params = {}) = 0;

		void SetGainRange(float minGain, float maxGain);
		std::pair<float, float> GetGainRange() const;
		void SetDistanceRange(float refDist, float maxDist);
		std::pair<float, float> GetDistanceRange() const;
		void SetConeAngles(float inner, float outer);
		std::pair<float, float> GetConeAngles() const;
		std::pair<float, float> GetOuterConeGains() const;
		void SetStereoAngles(float leftAngle, float rightAngle);
		std::pair<float, float> GetStereoAngles() const;
		void SetOuterConeGains(float gain, float gainHF = 1.f);
		bool GetDirectGainHFAuto() const;
		bool GetSendGainAuto() const;
		bool GetSendGainHFAuto() const;
		void SetEffectGain(const std::string &effectName, float gain);
		virtual void SetTimeOffset(float sec);
		virtual float GetTimeOffset() const;

		std::pair<float, float> GetRolloffFactors() const;
		void SetRolloffFactors(float factor, float roomFactor = 0.f);

		virtual ALSoundType GetType() const;
		virtual void SetType(ALSoundType type);
		void AddType(ALSoundType type);
		unsigned int GetFlags() const;
		virtual void SetFlags(unsigned int flags);
		void AddFlags(unsigned int flags);
		void RemoveFlags(unsigned int flags);
		float GetLastOffset() const;
		float GetDeltaOffset() const;
		float GetLastTimeOffset() const;
		float GetDeltaTimeOffset() const;
	};
#pragma warning(pop)
};
