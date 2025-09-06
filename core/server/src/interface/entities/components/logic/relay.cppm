// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/serverdefinitions.h"
#include "pragma/entities/logic/logic_relay.h"

export module pragma.server.entities.components.logic.relay;

import pragma.server.entities;

export {
	namespace pragma {
		class DLLSERVER SLogicRelayComponent final : public BaseLogicRelayComponent {
		public:
			SLogicRelayComponent(BaseEntity &ent) : BaseLogicRelayComponent(ent) {}
			virtual void Initialize() override;
			virtual void InitializeLuaObject(lua_State *l) override;
		protected:
			void Trigger(BaseEntity *activator);
		};
	};

	class DLLSERVER LogicRelay : public SBaseEntity {
	public:
		virtual void Initialize() override;
	};
};
