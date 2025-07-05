// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "pragma/physics/tire_type_manager.hpp"

pragma::physics::TireType::TireType(TypeId id, const std::string &name) : BaseNamedType {id, name} {}
void pragma::physics::TireType::SetFrictionModifier(SurfaceType &surfType, float modifier) { m_frictionModifiers[&surfType] = modifier; }
std::optional<float> pragma::physics::TireType::GetFrictionModifier(SurfaceType &surfType)
{
	auto it = m_frictionModifiers.find(&surfType);
	return (it != m_frictionModifiers.end()) ? it->second : std::optional<float> {};
}
const std::unordered_map<pragma::physics::SurfaceType *, float> &pragma::physics::TireType::GetFrictionModifiers() const { return m_frictionModifiers; }
