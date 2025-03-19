/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2025 Silverlan
 */

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
