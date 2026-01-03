// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.prop.base;

import :entities;
import :entities.components.entity;

export {
	namespace pragma {
		class DLLSERVER PropComponent final : public BasePropComponent, public SBaseNetComponent {
		  public:
			PropComponent(ecs::BaseEntity &ent) : BasePropComponent(ent) {}
			virtual void InitializeLuaObject(lua::State *l) override;
			virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
		};
	};

	class DLLSERVER SBaseProp : public SBaseEntity {
	  protected:
	};
};
