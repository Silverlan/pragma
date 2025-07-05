// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_BSP_COMPONENT_HPP__
#define __C_BSP_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include <pragma/entities/components/base_entity_component.hpp>

namespace source_engine::bsp {
	class File;
};
namespace pragma {
	class DLLCLIENT CBSPComponent final : public BaseEntityComponent {
	  public:
		CBSPComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua_State *l) override;
	  protected:
		void InitializeBSPTree(source_engine::bsp::File &bsp);
	};
};

#endif
