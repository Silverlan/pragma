// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.child;

export import :entities.components.entity;

export namespace pragma {
	namespace ecs {
		class CParticleSystemComponent;
	}
	class DLLCLIENT CChildComponent final : public BaseChildComponent, public CBaseNetComponent {
	  public:
		CChildComponent(ecs::BaseEntity &ent) : BaseChildComponent(ent) {}
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual bool ShouldTransmitNetData() const override { return true; }

		virtual void ReceiveData(NetPacket &packet) override;
		virtual Bool ReceiveNetEvent(NetEventId eventId, NetPacket &packet) override;
		virtual void OnRemove() override;
	};
};
