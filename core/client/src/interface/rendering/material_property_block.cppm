// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include <udm.hpp>

export module pragma.client.rendering.material_property_block;

export namespace pragma::rendering {
	class DLLCLIENT MaterialPropertyBlock {
	  public:
		MaterialPropertyBlock();
		udm::PropertyWrapper GetPropertyBlock() const;
		udm::PropertyWrapper GetTextureBlock() const;
	  private:
		udm::PProperty m_propertyBlock = nullptr;
	};
};
