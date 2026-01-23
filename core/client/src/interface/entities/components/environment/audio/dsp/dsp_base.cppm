// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.audio.dsp.base;

export import :entities.base_entity;
export import :entities.components.entity;
export import pragma.soundsystem;

export namespace pragma {
	class DLLCLIENT CBaseSoundDspComponent : public BaseEnvSoundDspComponent, public CBaseNetComponent {
	  public:
		virtual ~CBaseSoundDspComponent() override;
		virtual void Initialize() override;
		virtual void OnTick(double dt) override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual util::EventReply HandleEvent(ComponentEventId eventId, ComponentEvent &evData) override;
		virtual Bool ReceiveNetEvent(UInt32 eventId, NetPacket &p) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void OnEntitySpawn() override;
	  protected:
		CBaseSoundDspComponent(ecs::BaseEntity &ent) : BaseEnvSoundDspComponent(ent) {}

		bool m_bAffectRelative = false;
		bool m_bApplyGlobal = false;
		bool m_bAllWorldSounds = false;
		bool m_bAllSounds = false;
		std::shared_ptr<audio::IEffect> m_dsp = nullptr;
		std::vector<std::pair<audio::SoundSourceHandle, uint32_t>> m_affectedSounds;
		audio::ALSoundType m_types = audio::ALSoundType::All & ~audio::ALSoundType::GUI;

		audio::ALSoundType GetTargetSoundTypes() const;
		std::vector<std::pair<audio::SoundSourceHandle, uint32_t>>::iterator FindSoundSource(audio::SoundSource &src);
		void UpdateSoundSource(audio::SoundSource &src, float gain);
		void DetachSoundSource(audio::SoundSource &src);
		void DetachAllSoundSources();
	};

	class DLLCLIENT CSoundDspComponent final : public CBaseSoundDspComponent {
	  public:
		CSoundDspComponent(ecs::BaseEntity &ent) : CBaseSoundDspComponent(ent) {}
		virtual void InitializeLuaObject(lua::State *l) override;
	};
};

export class DLLCLIENT CEnvSoundDsp : public pragma::ecs::CBaseEntity {
  public:
	virtual void Initialize() override;
};
