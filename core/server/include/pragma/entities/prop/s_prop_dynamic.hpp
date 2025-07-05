// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __S_PROP_DYNAMIC_HPP__
#define __S_PROP_DYNAMIC_HPP__

#include "pragma/serverdefinitions.h"
#include <pragma/entities/prop/prop_dynamic.hpp>

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

#endif
