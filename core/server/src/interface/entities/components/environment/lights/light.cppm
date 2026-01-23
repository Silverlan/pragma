// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.lights.base;

import :entities;
import :entities.components.entity;

export namespace pragma {
	class DLLSERVER SLightComponent final : public BaseEnvLightComponent, public SBaseNetComponent {
	  public:
		SLightComponent(ecs::BaseEntity &ent) : BaseEnvLightComponent(ent) {}
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void SetShadowType(ShadowType type) override;
		virtual void InitializeLuaObject(lua::State *l) override;

		virtual void SetFalloffExponent(float falloffExponent) override;
	};
};
