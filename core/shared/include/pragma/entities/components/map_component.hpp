// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

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
