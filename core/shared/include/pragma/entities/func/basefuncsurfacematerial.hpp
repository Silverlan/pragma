// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

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
