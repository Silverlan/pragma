// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.decal;

import :entities;
import :entities.components.entity;

export {
	namespace pragma {
		class DLLSERVER SDecalComponent final : public BaseEnvDecalComponent, public SBaseNetComponent {
		  public:
			SDecalComponent(ecs::BaseEntity &ent) : BaseEnvDecalComponent(ent) {}
			virtual void Initialize() override;
			virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
			virtual void InitializeLuaObject(lua::State *l) override;
		};
	};

	class DLLSERVER EnvDecal : public SBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
