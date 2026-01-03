// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.render;

import :entities.components.entity;

export namespace pragma {
	class DLLSERVER SRenderComponent final : public BaseRenderComponent, public SBaseNetComponent {
	  public:
		SRenderComponent(ecs::BaseEntity &ent) : BaseRenderComponent(ent) {}
		virtual void SetUnlit(bool b) override;
		virtual void SetCastShadows(bool b) override;
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void InitializeLuaObject(lua::State *l) override;
	};
};
