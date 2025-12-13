// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:audio.sound_script;

export import :audio.sound;
export import :audio.sound_base;
export import :audio.sound_script_events;
export import :audio.sound_script_manager;

export namespace pragma {
	class NetworkState;
}
export namespace pragma::audio {
	class DLLNETWORK ALSoundScript : virtual public ALSound, virtual public ALSoundBase {
	protected:
		std::vector<SSEBase *> m_events;
		std::vector<SSESound *> m_sounds;
		NetworkState *m_networkState;
		SoundScript *m_script;
		virtual void SetState(ALState state) override;
		std::unordered_map<unsigned int, Vector3> m_positions;
		bool m_bStream = false;
		virtual std::shared_ptr<ALSound> CreateSound(const std::string &name, ALChannel channel, ALCreateFlags createFlags);
	protected:
		bool HandleEvents(SoundScriptEvent *ev, float eventOffset, float lastOffset, float newOffset);
		void InitializeEvent(SoundScriptEvent *ev);
	public:
		ALSoundScript(NetworkState *nw, unsigned int idx, SoundScript *script, NetworkState *state, bool bStream);
		virtual ~ALSoundScript() override;
		virtual void Initialize() override;
		virtual ALState GetState() const override;
		void SetTargetPosition(unsigned int id, Vector3 pos);
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
		virtual void SetOrientation(const Vector3 &at, const Vector3 &up) override;
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
		virtual void SetDirectFilter(const SoundEffectParams &params) override;
		virtual const SoundEffectParams &GetDirectFilter() const override;
		virtual bool AddEffect(const std::string &effectName, const SoundEffectParams &params = {}) override;
		virtual void RemoveEffect(const std::string &effectName) override;
		virtual void SetEffectParameters(const std::string &effectName, const SoundEffectParams &params = {}) override;
		virtual std::pair<Vector3, Vector3> GetOrientation() const override;

		virtual void SetTimeOffset(float sec) override;
		virtual float GetTimeOffset() const override;
		virtual void SetFlags(unsigned int flags) override;
		virtual void Update() override;
		virtual void PostUpdate() override;
		virtual void FadeIn(float time) override;
		virtual void FadeOut(float time) override;
	};
}
