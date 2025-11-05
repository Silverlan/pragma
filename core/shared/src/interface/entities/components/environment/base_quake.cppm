// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.environment.base_quake;

export import :entities.components.base;

export {
	constexpr uint32_t SF_QUAKE_GLOBAL_SHAKE = 1;
	constexpr uint32_t SF_QUAKE_IN_AIR = 4;
	constexpr uint32_t SF_QUAKE_AFFECT_PHYSICS = 8;
	constexpr uint32_t SF_QUAKE_DONT_SHAKE_VIEW = 32;
	constexpr uint32_t SF_QUAKE_REMOVE_ON_COMPLETE = 4096;
	namespace pragma {
		class DLLNETWORK BaseEnvQuakeComponent : public BaseEntityComponent {
		  public:
			using BaseEntityComponent::BaseEntityComponent;
			virtual void Initialize() override;
			virtual util::EventReply HandleEvent(ComponentEventId eventId, ComponentEvent &evData) override;
			Float GetFrequency() const;
			Float GetAmplitude() const;
			Float GetRadius() const;
			Float GetDuration() const;
			Float GetFadeInDuration() const;
			Float GetFadeOutDuration() const;
			void SetFrequency(Float freq);
			void SetAmplitude(Float amplitude);
			void SetRadius(Float radius);
			void SetDuration(Float duration);
			void SetFadeInDuration(Float tFadeIn);
			void SetFadeOutDuration(Float tFadeOut);
		  protected:
			Float m_tStartShake = 0.f;
			Float m_frequency = 50.f;
			Float m_amplitude = 50.f;
			Float m_radius = 256.f;
			Float m_duration = 5.f;
			Float m_tFadeIn = 0.f;
			Float m_tFadeOut = 2.f;
			UInt32 m_quakeFlags = 0u;
			virtual void StartShake();
			virtual void StopShake();
			Bool IsShakeActive() const;
			void InitializeQuake(UInt32 flags);

			Bool IsGlobal() const;
			Bool InAir() const;
			Bool ShouldAffectPhyiscs() const;
			Bool ShouldShakeView() const;
			Bool ShouldRemoveOnComplete() const;
		};
	};
}
