// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.timescale;

import :entities;
import :entities.components.entity;

export {
	namespace pragma {
		class DLLSERVER SEnvTimescaleComponent final : public BaseEnvTimescaleComponent, public SBaseNetComponent {
		  public:
			SEnvTimescaleComponent(ecs::BaseEntity &ent) : BaseEnvTimescaleComponent(ent) {}
			virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
			virtual void InitializeLuaObject(lua::State *l) override;
		};
	};

	class DLLSERVER EnvTimescale : public SBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
