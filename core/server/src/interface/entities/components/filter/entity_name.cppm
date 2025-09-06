// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/serverdefinitions.h"
#include <pragma/entities/basefilterentity.h>

export module pragma.server.entities.components.filter.entity_name;

import pragma.server.entities;

export {
	namespace pragma {
		class DLLSERVER SFilterNameComponent final : public BaseFilterNameComponent {
		public:
			SFilterNameComponent(BaseEntity &ent) : BaseFilterNameComponent(ent) {}
			virtual void InitializeLuaObject(lua_State *l) override;
		};
	};

	class DLLSERVER FilterEntityName : public SBaseEntity {
	public:
		virtual void Initialize() override;
	};
};
