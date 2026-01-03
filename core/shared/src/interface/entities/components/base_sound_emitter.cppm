// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.base_sound_emitter;

export import :entities.components.base;

export import :audio.enums;
export import :audio.sound;

export namespace pragma {
	namespace baseSoundEmitterComponent {
		REGISTER_COMPONENT_EVENT(EVENT_ON_SOUND_CREATED)
	}
	class DLLNETWORK BaseSoundEmitterComponent : public BaseEntityComponent {
	  public:
		static void RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);

		struct DLLNETWORK SoundInfo {
			SoundInfo(float gain = 1.f, float pitch = 1.f) : gain {gain}, pitch {pitch} {}
			float gain = 1.f;
			float pitch = 1.f;
			bool transmit = true;
		};

		virtual ~BaseSoundEmitterComponent() override;
		virtual void Initialize() override;

		virtual void OnTick(double dt) override;
		virtual void PrecacheSounds();
		virtual std::shared_ptr<audio::ALSound> CreateSound(std::string snd, audio::ALSoundType type, const SoundInfo &sndInfo = {});
		virtual std::shared_ptr<audio::ALSound> EmitSound(std::string snd, audio::ALSoundType type, const SoundInfo &sndInfo = {});
		void StopSounds();
		void GetSounds(std::vector<std::shared_ptr<audio::ALSound>> **sounds);
	  protected:
		BaseSoundEmitterComponent(ecs::BaseEntity &ent);
		void InitializeSound(const std::shared_ptr<audio::ALSound> &snd);
		virtual void UpdateSoundTransform(audio::ALSound &snd) const;
		virtual bool ShouldRemoveSound(audio::ALSound &snd) const;
		virtual void MaintainSounds();

		std::vector<std::shared_ptr<audio::ALSound>> m_sounds;
	};

	struct DLLNETWORK CEOnSoundCreated : public ComponentEvent {
		CEOnSoundCreated(const std::shared_ptr<audio::ALSound> &sound);
		virtual void PushArguments(lua::State *l) override;
		std::shared_ptr<audio::ALSound> sound;
	};
};
