// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:audio.sound_base;

export import :audio.effect_params;
export import :audio.enums;

export namespace pragma::audio {
#pragma warning(push)
#pragma warning(disable : 4251)
	class DLLNETWORK ALSoundBase {
	  public:
		virtual float GetDuration() const;

		virtual void SetPitch(float pitch);
		virtual float GetPitch() const;

		virtual void SetLooping(bool loop);
		virtual bool IsLooping() const;

		virtual void SetGain(float gain);
		virtual float GetGain() const;

		virtual void SetPosition(const Vector3 &pos);
		virtual Vector3 GetPosition() const;

		virtual void SetVelocity(const Vector3 &vel);
		virtual Vector3 GetVelocity() const;

		virtual void SetDirection(const Vector3 &dir);
		virtual Vector3 GetDirection() const;

		virtual void SetRelative(bool b);
		virtual bool IsRelative() const;

		virtual void SetReferenceDistance(float dist);
		virtual float GetReferenceDistance() const;

		virtual float GetRolloffFactor() const;
		virtual void SetRolloffFactor(float factor);

		virtual void SetRoomRolloffFactor(float roomFactor);
		virtual float GetRoomRolloffFactor() const;

		virtual void SetMaxDistance(float dist);
		virtual float GetMaxDistance() const;

		virtual void SetMinGain(float gain);
		virtual float GetMinGain() const;

		virtual void SetMaxGain(float gain);
		virtual float GetMaxGain() const;

		virtual void SetInnerConeAngle(float ang);
		virtual float GetInnerConeAngle() const;

		virtual void SetOuterConeAngle(float ang);
		virtual float GetOuterConeAngle() const;

		virtual void SetOuterConeGain(float gain);
		virtual float GetOuterConeGain() const;

		virtual void SetOuterConeGainHF(float gain);
		virtual float GetOuterConeGainHF() const;

		virtual void SetState(ALState state);
		virtual ALState GetState() const;

		virtual void SetOffset(float offset);
		virtual float GetOffset() const;

		virtual void SetPriority(uint32_t priority);
		virtual uint32_t GetPriority();

		virtual void SetOrientation(const Vector3 &at, const Vector3 &up);
		virtual std::pair<Vector3, Vector3> GetOrientation() const;

		virtual void SetDopplerFactor(float factor);
		virtual float GetDopplerFactor() const;

		virtual void SetLeftStereoAngle(float ang);
		virtual float GetLeftStereoAngle() const;

		virtual void SetRightStereoAngle(float ang);
		virtual float GetRightStereoAngle() const;

		virtual void SetAirAbsorptionFactor(float factor);
		virtual float GetAirAbsorptionFactor() const;

		virtual void SetGainAuto(bool directHF, bool send, bool sendHF);
		virtual std::tuple<bool, bool, bool> GetGainAuto() const;

		virtual void SetDirectFilter(const SoundEffectParams &params);
		const SoundEffectParams &GetDirectFilter() const;

		virtual bool IsPlaying() const;
		virtual bool IsPaused() const;
		virtual bool IsStopped() const;
	  protected:
		ALSoundBase() = default;
		float m_pitch = 1.f;
		float m_gain = 1.f;
		bool m_bLooping = false;
		ALState m_state = ALState::Initial;
		Vector3 m_pos;
		Vector3 m_vel;
		Vector3 m_dir;
		bool m_bRelative = ALSOUND_DEFAULT_RELATIVE;
		float m_refDist = ALSOUND_DEFAULT_REFERENCE_DISTANCE;
		std::pair<float, float> m_rolloff = {ALSOUND_DEFAULT_ROLLOFF_FACTOR, ALSOUND_DEFAULT_ROOM_ROLLOFF_FACTOR};
		double m_maxDist = ALSOUND_DEFAULT_MAX_DISTANCE;
		float m_minGain = ALSOUND_DEFAULT_MIN_GAIN;
		float m_maxGain = ALSOUND_DEFAULT_MAX_GAIN;
		float m_coneInnerAngle = ALSOUND_DEFAULT_CONE_INNER_ANGLE;
		float m_coneOuterAngle = ALSOUND_DEFAULT_CONE_OUTER_ANGLE;
		std::pair<float, float> m_coneOuterGain = {ALSOUND_DEFAULT_CONE_OUTER_GAIN, ALSOUND_DEFAULT_CONE_OUTER_GAIN_HF};
		uint32_t m_priority = ALSOUND_DEFAULT_PRIORITY;
		float m_dopplerFactor = ALSOUND_DEFAULT_DOPPLER_FACTOR;
		float m_airAbsorptionFactor = ALSOUND_DEFAULT_AIR_ABSORPTION_FACTOR;
		std::pair<Vector3, Vector3> m_orientation = {ALSOUND_DEFAULT_ORIENTATION_AT, ALSOUND_DEFAULT_ORIENTATION_UP};
		std::pair<float, float> m_stereoAngles = {ALSOUND_DEFAULT_STEREO_ANGLE_LEFT, ALSOUND_DEFAULT_STEREO_ANGLE_RIGHT};
		std::tuple<bool, bool, bool> m_gainAuto = std::tuple<bool, bool, bool> {ALSOUND_DEFAULT_DIRECT_GAIN_HF_AUTO, ALSOUND_DEFAULT_SEND_GAIN_AUTO, ALSOUND_DEFAULT_SEND_GAIN_HF_AUTO};
		SoundEffectParams m_directFilter = {ALSOUND_DEFAULT_DIRECT_FILTER_GAIN, ALSOUND_DEFAULT_DIRECT_FILTER_GAIN_HF, ALSOUND_DEFAULT_DIRECT_FILTER_GAIN_LF};
		float m_offset = 0.f;
		float m_duration = 0.f;
		double m_tLastUpdate = 0.0;
		double m_tPassed = 0.0;
	};
#pragma warning(pop)
};
