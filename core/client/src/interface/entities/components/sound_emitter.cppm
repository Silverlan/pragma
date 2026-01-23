// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.sound_emitter;

export import :entities.components.entity;

export namespace pragma {
	class DLLCLIENT CSoundEmitterComponent final : public BaseSoundEmitterComponent, public CBaseNetComponent {
	  public:
		static void RegisterLuaBindings(lua::State *l, luabind::module_ &modEnts);

		CSoundEmitterComponent(ecs::BaseEntity &ent) : BaseSoundEmitterComponent(ent) {}
		// Sounds
		virtual std::shared_ptr<audio::ALSound> CreateSound(std::string sndname, audio::ALSoundType type, const SoundInfo &sndInfo = {}) override;
		virtual std::shared_ptr<audio::ALSound> EmitSound(std::string sndname, audio::ALSoundType type, const SoundInfo &sndInfo = {}) override;
		void AddSound(std::shared_ptr<audio::ALSound> snd); // Only to be used from netmessage

		virtual void PrecacheSounds() override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
	  protected:
		virtual void MaintainSounds() override;
		virtual bool ShouldRemoveSound(audio::ALSound &snd) const override;
	};
};
