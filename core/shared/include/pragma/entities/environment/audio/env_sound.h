/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __ENV_SOUND_H__
#define __ENV_SOUND_H__

#include "pragma/entities/components/base_entity_component.hpp"
#include "pragma/audio/alsound_type.h"

enum class DLLNETWORK ALState : uint32_t;
namespace pragma {
	class DLLNETWORK BaseEnvSoundComponent : public BaseEntityComponent {
	  public:
		enum class DLLNETWORK SpawnFlags : uint32_t {
			None = 0,
			PlayEverywhere = 8,
			PlayOnSpawn = PlayEverywhere << 1,
			IsLooped = PlayOnSpawn << 1,
			Effect = IsLooped << 1,
			Music = Effect << 1,
			Voice = Music << 1,
			Weapon = Voice << 1,
			NPC = Weapon << 1,
			Player = NPC << 1,
			Vehicle = Player << 1,
			Physics = Vehicle << 1,
			Environment = Physics << 1,
			GUI = Environment << 1,

			AllTypes = Effect | Music | Voice | Weapon | NPC | Player | Vehicle | Physics | Environment | GUI
		};

		using BaseEntityComponent::BaseEntityComponent;
		static void RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
		virtual void Initialize() override;
		virtual void OnRemove() override;
		virtual void InjectStateChange(ALState oldState, ALState newState);
		virtual void OnEntitySpawn() override;

		const std::string &GetSoundSource() const;
		float GetPitch() const;
		float GetGain() const;
		float GetRolloffFactor() const;
		float GetMinGain() const;
		float GetMaxGain() const;
		float GetInnerConeAngle() const;
		float GetOuterConeAngle() const;
		float GetReferenceDistance() const;
		float GetMaxDistance() const;

		void SetSoundSource(const std::string &sndName);
		void SetPitch(float pitch);
		void SetGain(float gain);
		void SetRolloffFactor(float rolloffFactor);
		void SetMinGain(float minGain);
		void SetMaxGain(float maxGain);
		void SetInnerConeAngle(float angle);
		void SetOuterConeAngle(float angle);
		void SetOffset(float offset);
		void SetTimeOffset(float offsetInSeconds);
		void SetReferenceDistance(float referenceDist);
		void SetMaxDistance(float maxDist);

		float GetOffset() const;
		float GetTimeOffset() const;

		void SetRelativeToListener(bool bRelative);
		void SetPlayOnSpawn(bool bPlayOnSpawn);
		void SetLooping(bool bLoop);
		void SetSoundType(ALSoundType types);

		void Play();
		void Stop();
		void Pause();

		bool IsPaused() const;
		bool IsPlaying() const;
		const std::shared_ptr<ALSound> &GetSound() const;
	  protected:
		void InitializeSound();
		virtual void OnSoundCreated(ALSound &snd);
		std::string m_kvSoundName;
		float m_kvPitch = 1.f;
		float m_kvGain = 1.f;
		float m_kvRolloff = 1.f;
		float m_kvMinGain = 0.f;
		float m_kvMaxGain = 1.f;
		float m_kvInnerCone = 360.f;
		float m_kvOuterCone = 360.f;
		float m_kvOffset = 0.f;
		float m_kvReferenceDist = 1.f;
		float m_kvMaxDist = 1024.f;
		ALSoundType m_soundTypes = ALSoundType::Generic;
		std::shared_ptr<ALSound> m_sound;
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::BaseEnvSoundComponent::SpawnFlags);

#endif
