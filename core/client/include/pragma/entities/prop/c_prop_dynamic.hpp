// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_PROP_DYNAMIC_HPP__
#define __C_PROP_DYNAMIC_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include <pragma/entities/prop/prop_dynamic.hpp>

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

#endif
