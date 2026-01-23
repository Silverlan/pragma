// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.ownable;

import :entities.components.entity;

export namespace pragma {
	class DLLSERVER SOwnableComponent final : public BaseOwnableComponent, public SBaseNetComponent {
	  public:
		SOwnableComponent(ecs::BaseEntity &ent) : BaseOwnableComponent(ent) {}
		virtual void Initialize() override;
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void InitializeLuaObject(lua::State *l) override;
		using BaseOwnableComponent::SetOwner;
	  protected:
		virtual void SetOwner(ecs::BaseEntity *owner) override;
	};
};
