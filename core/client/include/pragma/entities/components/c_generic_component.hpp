// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_GENERIC_COMPONENT_HPP__
#define __C_GENERIC_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/components/base_generic_component.hpp>

namespace pragma {
	class DLLCLIENT CGenericComponent final : public BaseGenericComponent {
	  public:
		CGenericComponent(BaseEntity &ent) : BaseGenericComponent(ent) {}
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};

#endif
