/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __MAP_COMPONENT_HPP__
#define __MAP_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"

namespace pragma {
	class DLLNETWORK MapComponent final : public BaseEntityComponent {
	  public:
		MapComponent(BaseEntity &ent);
		virtual void Initialize() override;
		void SetMapIndex(unsigned int idx);
		unsigned int GetMapIndex() const;

		virtual void InitializeLuaObject(lua_State *l) override;
	  protected:
		uint32_t m_mapIndex = 0u;
	};
};

#endif
