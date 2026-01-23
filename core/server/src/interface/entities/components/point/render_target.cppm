// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.point.render_target;

import :entities;
import :entities.components.entity;

export {
	namespace pragma {
		class DLLSERVER SRenderTargetComponent final : public BasePointRenderTargetComponent, public SBaseNetComponent {
		  public:
			SRenderTargetComponent(ecs::BaseEntity &ent) : BasePointRenderTargetComponent(ent) {}
			virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
			virtual void InitializeLuaObject(lua::State *l) override;
		};
	};

	class DLLSERVER PointRenderTarget : public SBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
