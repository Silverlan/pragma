// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.effects.smoke_trail;

import :entities;
import :entities.components.entity;

export {
	namespace pragma {
		class DLLSERVER SSmokeTrailComponent final : public BaseEnvSmokeTrailComponent, public SBaseNetComponent {
		  public:
			SSmokeTrailComponent(ecs::BaseEntity &ent) : BaseEnvSmokeTrailComponent(ent) {}
			virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
			virtual void InitializeLuaObject(lua::State *l) override;
		};
	};

	class DLLSERVER EnvSmokeTrail : public SBaseEntity {
	  protected:
	  public:
		virtual void Initialize() override;
	};
};
