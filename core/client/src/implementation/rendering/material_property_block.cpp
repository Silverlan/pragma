/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2025 Silverlan
 */

module;

module pragma.client.rendering.material_property_block;

pragma::rendering::MaterialPropertyBlock::MaterialPropertyBlock() : m_propertyBlock {udm::Property::Create(udm::Type::Element)}
{
	(*m_propertyBlock)["properties"] = udm::Property::Create(udm::Type::Element);
	(*m_propertyBlock)["textures"] = udm::Property::Create(udm::Type::Element);
}

udm::PropertyWrapper pragma::rendering::MaterialPropertyBlock::GetPropertyBlock() const { return (*m_propertyBlock)["properties"]; }
udm::PropertyWrapper pragma::rendering::MaterialPropertyBlock::GetTextureBlock() const { return (*m_propertyBlock)["textures"]; }
