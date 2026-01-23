// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.name;

import :entities.components.entity;

export namespace pragma {
	class DLLSERVER SNameComponent final : public BaseNameComponent, public SBaseNetComponent {
	  public:
		SNameComponent(ecs::BaseEntity &ent) : BaseNameComponent(ent) {}
		virtual void SetName(std::string name) override;
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void InitializeLuaObject(lua::State *l) override;
	};
};
