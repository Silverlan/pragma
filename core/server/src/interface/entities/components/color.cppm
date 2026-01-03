// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.color;

import :entities.components.entity;

export namespace pragma {
	class DLLSERVER SColorComponent final : public BaseColorComponent, public SBaseNetComponent {
	  public:
		SColorComponent(ecs::BaseEntity &ent) : BaseColorComponent(ent) {}
		virtual void Initialize() override;
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;

		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void InitializeLuaObject(lua::State *l) override;
	};
};
