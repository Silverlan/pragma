/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __CONSTRAINT_COPY_LOCATION_COMPONENT_HPP__
#define __CONSTRAINT_COPY_LOCATION_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"

namespace pragma {
	class ConstraintComponent;
	class ConstraintSpaceComponent;
	class DLLNETWORK ConstraintCopyLocationComponent final : public BaseEntityComponent {
	  public:
		ConstraintCopyLocationComponent(BaseEntity &ent);
		virtual void Initialize() override;

		virtual void InitializeLuaObject(lua_State *lua) override;
	  protected:
		void ApplyConstraint();
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		pragma::ComponentHandle<ConstraintComponent> m_constraintC;
		pragma::ComponentHandle<ConstraintSpaceComponent> m_constraintSpaceC;
	};
};

#endif
