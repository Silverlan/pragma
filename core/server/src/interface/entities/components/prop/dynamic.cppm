// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include <pragma/entities/prop/prop_dynamic.hpp>

export module pragma.server.entities.components.prop.dynamic;

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
