// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.func.base_surface_material;

export import :entities.components.base;

export namespace pragma {
	class DLLNETWORK BaseFuncSurfaceMaterialComponent : public BaseEntityComponent {
	  public:
		using BaseEntityComponent::BaseEntityComponent;
	  protected:
		std::string m_kvSurfaceMaterial;
		void UpdateSurfaceMaterial(Game *game);
	};
};
