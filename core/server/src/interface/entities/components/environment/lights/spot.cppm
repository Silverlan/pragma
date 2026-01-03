// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.lights.spot;

import :entities;
import :entities.components.entity;

export {
	namespace pragma {
		class DLLSERVER SLightSpotComponent final : public BaseEnvLightSpotComponent, public SBaseNetComponent {
		  public:
			SLightSpotComponent(ecs::BaseEntity &ent) : BaseEnvLightSpotComponent(ent) {}
			virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
			virtual void SetOuterConeAngle(float ang) override;
			virtual void SetBlendFraction(float fraction) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
			virtual void InitializeLuaObject(lua::State *l) override;
			virtual void SetConeStartOffset(float offset) override;
		  protected:
			virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		};
	};

	class DLLSERVER EnvLightSpot : public SBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
