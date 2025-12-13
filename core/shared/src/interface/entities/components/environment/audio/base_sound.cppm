// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"
#include "util_enum_flags.hpp"

export module pragma.shared:entities.components.environment.audio.base_sound;

export import :audio.enums;
export import :audio.sound;
export import :entities.components.base;

export {
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
			static void RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
			virtual void Initialize() override;
			virtual void OnRemove() override;
			virtual void InjectStateChange(audio::ALState oldState, audio::ALState newState);
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
			void SetSoundType(audio::ALSoundType types);

			void Play();
			void Stop();
			void Pause();

			bool IsPaused() const;
			bool IsPlaying() const;
			const std::shared_ptr<audio::ALSound> &GetSound() const;
		  protected:
			void InitializeSound();
			virtual void OnSoundCreated(audio::ALSound &snd);
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
			audio::ALSoundType m_soundTypes = audio::ALSoundType::Generic;
			std::shared_ptr<audio::ALSound> m_sound;
		};
		using namespace pragma::math::scoped_enum::bitwise;
	};
	REGISTER_ENUM_FLAGS(pragma::BaseEnvSoundComponent::SpawnFlags);
};
