/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __BASE_LIQUID_SURFACE_COMPONENT_HPP__
#define __BASE_LIQUID_SURFACE_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include <mathutil/plane.hpp>

namespace pragma {
	class DLLNETWORK BaseLiquidSurfaceComponent : public BaseEntityComponent {
	  public:
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		static void RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
		virtual void Initialize() override;
	  protected:
		BaseLiquidSurfaceComponent(BaseEntity &ent);
	};
};

#endif
