// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.point_at_target;

export import :entities.components.entity;

export namespace pragma {
	class DLLCLIENT CPointAtTargetComponent final : public BasePointAtTargetComponent, public CBaseNetComponent {
	  public:
		CPointAtTargetComponent(ecs::BaseEntity &ent) : BasePointAtTargetComponent(ent) {}

		virtual void ReceiveData(NetPacket &packet) override;
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual bool ShouldTransmitNetData() const override { return true; }

		using BasePointAtTargetComponent::SetPointAtTarget;
	  protected:
		virtual bool ReceiveNetEvent(NetEventId eventId, NetPacket &packet) override;
	};
};
