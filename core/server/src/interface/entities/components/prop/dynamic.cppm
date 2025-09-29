// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/serverdefinitions.h"


export module pragma.server.entities.components.prop.dynamic;

import pragma.server.entities;

export {
	namespace pragma {
		class DLLSERVER SPropDynamicComponent final : public BasePropDynamicComponent {
		public:
			SPropDynamicComponent(BaseEntity &ent) : BasePropDynamicComponent(ent) {}
			virtual void InitializeLuaObject(lua_State *l) override;
		};
	};

	class DLLSERVER PropDynamic : public SBaseEntity {
	public:
		virtual void Initialize() override;
	};
};
