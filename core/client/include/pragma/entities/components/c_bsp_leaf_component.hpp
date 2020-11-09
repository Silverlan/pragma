/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_BSP_LEAF_COMPONENT_HPP__
#define __C_BSP_LEAF_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include <pragma/entities/components/base_entity_component.hpp>
#include <unordered_set>

namespace pragma
{
	class DLLCLIENT CBSPLeafComponent final
		: public BaseEntityComponent
	{
	public:
		CBSPLeafComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
		virtual luabind::object InitializeLuaObject(lua_State *l) override;

		void SetLeaves(const std::vector<uint16_t> &leaves);
		bool GetLeafVisibility(uint16_t leafIdx) const;
	private:
		std::unordered_set<uint16_t> m_leaves;
	};
};

#endif
