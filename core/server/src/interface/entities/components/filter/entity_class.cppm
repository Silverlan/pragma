// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/serverdefinitions.h"


export module pragma.server.entities.components.filter.entity_class;

import pragma.server.entities;

export {
	namespace pragma {
		class DLLSERVER SFilterClassComponent final : public BaseFilterClassComponent {
		public:
			SFilterClassComponent(BaseEntity &ent) : BaseFilterClassComponent(ent) {}
			virtual void InitializeLuaObject(lua_State *l) override;
		};
	};

	class DLLSERVER FilterEntityClass : public SBaseEntity {
	public:
		virtual void Initialize() override;
	};
};
