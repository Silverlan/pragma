/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __BASEFUNCSURFACEMATERIAL_HPP__
#define __BASEFUNCSURFACEMATERIAL_HPP__

#include "pragma/entities/components/base_entity_component.hpp"

class Game;
namespace pragma {
	class DLLNETWORK BaseFuncSurfaceMaterialComponent : public BaseEntityComponent {
	  public:
		using BaseEntityComponent::BaseEntityComponent;
	  protected:
		std::string m_kvSurfaceMaterial;
		void UpdateSurfaceMaterial(Game *game);
	};
};

#endif
