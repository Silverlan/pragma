/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __ENV_QUAKE_H__
#define __ENV_QUAKE_H__

#include "pragma/entities/components/base_entity_component.hpp"

#define SF_QUAKE_GLOBAL_SHAKE 1
#define SF_QUAKE_IN_AIR 4
#define SF_QUAKE_AFFECT_PHYSICS 8
#define SF_QUAKE_DONT_SHAKE_VIEW 32
#define SF_QUAKE_REMOVE_ON_COMPLETE 4096

namespace pragma
{
	class DLLNETWORK BaseEnvQuakeComponent
		: public BaseEntityComponent
	{
	public:
		using BaseEntityComponent::BaseEntityComponent;
		virtual void Initialize() override;
		virtual util::EventReply HandleEvent(ComponentEventId eventId,ComponentEvent &evData) override;
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

#endif