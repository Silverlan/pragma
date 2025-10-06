// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/serverdefinitions.h"
#include "pragma/lua/luaapi.h"


export module pragma.server.entities.components.func.physics;

import pragma.server.entities;

import pragma.server.entities.components.entity;

export {
	namespace pragma {
		class DLLSERVER SFuncPhysicsComponent final : public BaseFuncPhysicsComponent, public SBaseNetComponent {
		public:
			SFuncPhysicsComponent(BaseEntity &ent) : BaseFuncPhysicsComponent(ent) {}
			virtual void Initialize() override;
			virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
			virtual void InitializeLuaObject(lua_State *l) override;
		protected:
			virtual PhysObj *InitializePhysics() override;
		};
	};

	class DLLSERVER FuncPhysics : public SBaseEntity {
	public:
		virtual void Initialize() override;
	};
};
