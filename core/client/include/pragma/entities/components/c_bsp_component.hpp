/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

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
