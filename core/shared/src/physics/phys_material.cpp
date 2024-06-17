/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "pragma/physics/phys_material.hpp"
#include "pragma/physics/environment.hpp"
#include "pragma/game/game.h"
#include "pragma/physics/base_t.hpp"

void pragma::physics::IMaterial::SetFriction(Float friction)
{
	SetStaticFriction(friction);
	SetDynamicFriction(friction);
}
void pragma::physics::IMaterial::SetSurfaceMaterial(SurfaceMaterial &surfMat) { m_surfMatIdx = surfMat.GetIndex(); }
SurfaceMaterial *pragma::physics::IMaterial::GetSurfaceMaterial() const { return m_physEnv.GetNetworkState().GetGameState()->GetSurfaceMaterial(m_surfMatIdx); }
void pragma::physics::IMaterial::InitializeLuaObject(lua_State *lua) { IBase::InitializeLuaObject<IMaterial>(lua); }
