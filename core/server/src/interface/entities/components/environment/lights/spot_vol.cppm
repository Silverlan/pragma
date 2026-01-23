// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.lights.spot_vol;

import :entities;
import :entities.components.entity;

export {
	namespace pragma {
		class DLLSERVER SLightSpotVolComponent final : public BaseEnvLightSpotVolComponent, public SBaseNetComponent {
		  public:
			SLightSpotVolComponent(ecs::BaseEntity &ent) : BaseEnvLightSpotVolComponent(ent) {}
			virtual void Initialize() override;
			virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
			virtual void InitializeLuaObject(lua::State *l) override;
		  protected:
			virtual void SetSpotlightTarget(ecs::BaseEntity &ent) override;
		};
	};

	class DLLSERVER EnvLightSpotVol : public SBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
