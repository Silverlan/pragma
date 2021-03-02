/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "pragma/physics/phys_material.hpp"

void pragma::physics::IMaterial::SetFriction(Float friction)
{
	SetStaticFriction(friction);
	SetDynamicFriction(friction);
}
void pragma::physics::IMaterial::SetSurfaceMaterial(SurfaceMaterial &surfMat) {m_surfaceMaterial = &surfMat;}
SurfaceMaterial *pragma::physics::IMaterial::GetSurfaceMaterial() const {return m_surfaceMaterial;}
void pragma::physics::IMaterial::InitializeLuaObject(lua_State *lua)
{
	IBase::InitializeLuaObject<IMaterial>(lua);
}
