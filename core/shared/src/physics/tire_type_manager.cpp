/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "pragma/physics/tire_type_manager.hpp"

pragma::physics::TireType::TireType(TypeId id,const std::string &name)
	: BaseNamedType{id,name}
{}
void pragma::physics::TireType::SetFrictionModifier(SurfaceType &surfType,float modifier)
{
	m_frictionModifiers[&surfType] = modifier;
}
std::optional<float> pragma::physics::TireType::GetFrictionModifier(SurfaceType &surfType)
{
	auto it = m_frictionModifiers.find(&surfType);
	return (it != m_frictionModifiers.end()) ? it->second : std::optional<float>{};
}
const std::unordered_map<pragma::physics::SurfaceType*,float> &pragma::physics::TireType::GetFrictionModifiers() const {return m_frictionModifiers;}
