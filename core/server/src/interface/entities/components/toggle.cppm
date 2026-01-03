// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.toggle;

import :entities.components.entity;

export namespace pragma {
	class DLLSERVER SToggleComponent final : public BaseToggleComponent, public SBaseNetComponent {
	  public:
		SToggleComponent(ecs::BaseEntity &ent) : BaseToggleComponent(ent) {}
		virtual void TurnOn() override;
		virtual void TurnOff() override;

		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void InitializeLuaObject(lua::State *l) override;
	};
};
