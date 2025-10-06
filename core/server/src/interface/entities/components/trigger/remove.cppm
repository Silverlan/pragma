// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/serverdefinitions.h"
#include "pragma/lua/luaapi.h"



export module pragma.server.entities.components.triggers.remove;

import pragma.server.entities;

export {
	namespace pragma {
		class DLLSERVER STriggerRemoveComponent final : public BaseTriggerRemoveComponent {
		public:
			STriggerRemoveComponent(BaseEntity &ent) : BaseTriggerRemoveComponent(ent) {}
			virtual void InitializeLuaObject(lua_State *l) override;
		};
	};

	class DLLSERVER TriggerRemove : public SBaseEntity {
	protected:
	public:
		virtual void Initialize() override;
	};
};
