// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.material_property_block;
pragma::rendering::MaterialPropertyBlock::MaterialPropertyBlock() : m_propertyBlock {udm::Property::Create(udm::Type::Element)}
{
	(*m_propertyBlock)["properties"] = udm::Property::Create(udm::Type::Element);
	(*m_propertyBlock)["textures"] = udm::Property::Create(udm::Type::Element);
}

udm::PropertyWrapper pragma::rendering::MaterialPropertyBlock::GetPropertyBlock() const { return (*m_propertyBlock)["properties"]; }
udm::PropertyWrapper pragma::rendering::MaterialPropertyBlock::GetTextureBlock() const { return (*m_propertyBlock)["textures"]; }
