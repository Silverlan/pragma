// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_IO_COMPONENT_HPP__
#define __C_IO_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/components/base_io_component.hpp>

namespace pragma {
	class DLLCLIENT CIOComponent final : public BaseIOComponent {
	  public:
		CIOComponent(BaseEntity &ent) : BaseIOComponent(ent) {}
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};

#endif
