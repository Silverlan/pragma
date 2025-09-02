// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/serverdefinitions.h"
#include "pragma/entities/components/s_entity_component.hpp"
#include <pragma/entities/components/base_io_component.hpp>

export module pragma.server.entities.components.io;

export namespace pragma {
	class DLLSERVER SIOComponent final : public BaseIOComponent {
	  public:
		SIOComponent(BaseEntity &ent) : BaseIOComponent(ent) {}
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};
