#ifndef __ENV_SOUND_H__
#define __ENV_SOUND_H__

#include "pragma/entities/components/base_entity_component.hpp"
#include "pragma/audio/alsound_type.h"

namespace pragma
{
	class DLLNETWORK BaseEnvSoundComponent
		: public BaseEntityComponent
	{
	public:
		enum class DLLNETWORK SpawnFlags : uint32_t
		{
			None = 0,
			PlayEverywhere = 8,
			PlayOnSpawn = PlayEverywhere<<1,
			IsLooped = PlayOnSpawn<<1,
			Effect = IsLooped<<1,
			Music = Effect<<1,
			Voice = Music<<1,
			Weapon = Voice<<1,
			NPC = Weapon<<1,
			Player = NPC<<1,
			Vehicle = Player<<1,
			Physics = Vehicle<<1,
			Environment = Physics<<1,
			GUI = Environment<<1,

			AllTypes = Effect | Music | Voice | Weapon | NPC | Player | Vehicle | Physics | Environment | GUI
		};
		
		using BaseEntityComponent::BaseEntityComponent;
		virtual void Initialize() override;
		virtual void InjectStateChange(ALState oldState,ALState newState);
		virtual void OnEntitySpawn() override;

		void SetSoundSource(const std::string &sndName);
		void SetPitch(float pitch);
		void SetGain(float gain);
		void SetRolloffFactor(float rolloffFactor);
		void SetMinGain(float minGain);
		void SetMaxGain(float maxGain);
		void SetInnerConeAngle(float angle);
		void SetOuterConeAngle(float angle);
		void SetOffset(float offset);
		void SetReferenceDistance(float referenceDist);
		void SetMaxDistance(float maxDist);

		void SetRelativeToListener(bool bRelative);
		void SetPlayOnSpawn(bool bPlayOnSpawn);
		void SetLooping(bool bLoop);
		void SetSoundType(ALSoundType types);

		void Play();
		void Stop();

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
