// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include <pragma/entities/components/base_generic_component.hpp>

export module pragma.client.entities.components.generic;

export namespace pragma {
	class DLLCLIENT CGenericComponent final : public BaseGenericComponent {
	  public:
		CGenericComponent(BaseEntity &ent) : BaseGenericComponent(ent) {}
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};
