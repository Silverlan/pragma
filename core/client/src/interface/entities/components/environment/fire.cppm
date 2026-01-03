// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.effects.fire;

export import :entities.base_entity;
export import :entities.components.entity;

export namespace pragma::ecs {
	class CParticleSystemComponent;
}
export namespace pragma {
	class DLLCLIENT CFireComponent final : public BaseEnvFireComponent, public CBaseNetComponent {
	  public:
		CFireComponent(ecs::BaseEntity &ent) : BaseEnvFireComponent(ent) {}
		virtual ~CFireComponent() override;
		virtual void Initialize() override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual util::EventReply HandleEvent(ComponentEventId eventId, ComponentEvent &evData) override;
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
	  protected:
		ComponentHandle<ecs::CParticleSystemComponent> m_hParticle;
		void InitializeParticle();
		void DestroyParticle();
	};
};

export class DLLCLIENT CEnvFire : public pragma::ecs::CBaseEntity {
  public:
	virtual void Initialize() override;
};
