// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.func.base_brush;

export import :entities.components.func.base_surface_material;

export namespace pragma {
	class DLLNETWORK BaseFuncBrushComponent : public BaseFuncSurfaceMaterialComponent {
	  public:
		using BaseFuncSurfaceMaterialComponent::BaseFuncSurfaceMaterialComponent;
		virtual void Initialize() override;
	  protected:
		bool m_kvSolid = true;
	};
};
