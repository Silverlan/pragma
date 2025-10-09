// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/clientdefinitions.h"
#include "pragma/lua/luaapi.h"



export module pragma.client:entities.components.prop_dynamic;

import :entities.base_entity;

export {
	namespace pragma {
		class DLLCLIENT CPropDynamicComponent final : public BasePropDynamicComponent {
		public:
			CPropDynamicComponent(BaseEntity &ent) : BasePropDynamicComponent(ent) {}
			virtual void Initialize() override;
			using BasePropDynamicComponent::BasePropDynamicComponent;
			virtual void InitializeLuaObject(lua_State *l) override;
		protected:
			virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		};
	};

	class DLLCLIENT CPropDynamic : public CBaseEntity {
	public:
		virtual void Initialize() override;
	};
};
