// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.radius;

import :entities.components.entity;

export namespace pragma {
	class DLLSERVER SRadiusComponent final : public BaseRadiusComponent, public SBaseNetComponent {
	  public:
		SRadiusComponent(ecs::BaseEntity &ent) : BaseRadiusComponent(ent) {}
		virtual void Initialize() override;
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void InitializeLuaObject(lua::State *l) override;
	};
};
