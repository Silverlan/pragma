// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include <string>

export module pragma.shared:entities.components.base_sound_emitter;

export import :entities.components.base;

export import :audio.enums;
export import :audio.sound;

export namespace pragma {
	class DLLNETWORK BaseSoundEmitterComponent : public BaseEntityComponent {
	  public:
		static ComponentEventId EVENT_ON_SOUND_CREATED;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);

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
		virtual std::shared_ptr<ALSound> CreateSound(std::string snd, ALSoundType type, const SoundInfo &sndInfo = {});
		virtual std::shared_ptr<ALSound> EmitSound(std::string snd, ALSoundType type, const SoundInfo &sndInfo = {});
		void StopSounds();
		void GetSounds(std::vector<std::shared_ptr<ALSound>> **sounds);
	  protected:
		BaseSoundEmitterComponent(BaseEntity &ent);
		void InitializeSound(const std::shared_ptr<ALSound> &snd);
		virtual void UpdateSoundTransform(ALSound &snd) const;
		virtual bool ShouldRemoveSound(ALSound &snd) const;
		virtual void MaintainSounds();

		std::vector<std::shared_ptr<ALSound>> m_sounds;
	};

	struct DLLNETWORK CEOnSoundCreated : public ComponentEvent {
		CEOnSoundCreated(const std::shared_ptr<ALSound> &sound);
		virtual void PushArguments(lua_State *l) override;
		std::shared_ptr<ALSound> sound;
	};
};
