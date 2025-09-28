// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "sharedutils/netpacket.hpp"

export module pragma.server.entities.components.func.kinematic;

import pragma.server.entities;

import pragma.server.entities.components.entity;

export {
	namespace pragma {
		class DLLSERVER SKinematicComponent final : public BaseFuncKinematicComponent, public SBaseNetComponent {
		public:
			SKinematicComponent(BaseEntity &ent) : BaseFuncKinematicComponent(ent) {}
			virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
			virtual void StartForward() override;
			virtual void StartBackward() override;
			virtual bool ShouldTransmitNetData() const override { return true; }
			virtual void InitializeLuaObject(lua_State *l) override;
		};
	};

	class DLLSERVER FuncKinematic : public SBaseEntity {
	public:
		virtual void Initialize() override;
	};
};
