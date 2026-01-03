// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.audio.sound_scape;

export import :entities.base_entity;
export import :entities.components.entity;

export namespace pragma {
	class DLLCLIENT CSoundScapeComponent final : public BaseEnvSoundScapeComponent, public CBaseNetComponent {
	  public:
		CSoundScapeComponent(ecs::BaseEntity &ent) : BaseEnvSoundScapeComponent(ent) {}
		virtual void Initialize() override;
		virtual void OnTick(double dt) override;
		virtual ~CSoundScapeComponent() override;
		virtual void ReceiveData(NetPacket &packet) override;
		void StartSoundScape();
		void StopSoundScape();
		bool IsPlayerInRange();
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void OnEntitySpawn() override;
	  protected:
		std::shared_ptr<audio::ALSound> m_sound;
		static CSoundScapeComponent *s_active;
		std::unordered_map<unsigned int, EntityHandle> m_targets;
		void UpdateTargetPositions();
	};
};

export class DLLCLIENT CEnvSoundScape : public pragma::ecs::CBaseEntity {
  public:
	virtual void Initialize() override;
};
