// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/physics/phys_material.hpp"
#include "pragma/physics/environment.hpp"
#include "pragma/game/game.h"
#include "pragma/physics/base_t.hpp"

module pragma.shared;

import :physics.material;

void pragma::physics::IMaterial::SetFriction(Float friction)
{
	SetStaticFriction(friction);
	SetDynamicFriction(friction);
}
void pragma::physics::IMaterial::SetSurfaceMaterial(SurfaceMaterial &surfMat) { m_surfMatIdx = surfMat.GetIndex(); }
SurfaceMaterial *pragma::physics::IMaterial::GetSurfaceMaterial() const { return m_physEnv.GetNetworkState().GetGameState()->GetSurfaceMaterial(m_surfMatIdx); }
void pragma::physics::IMaterial::InitializeLuaObject(lua_State *lua) { IBase::InitializeLuaObject<IMaterial>(lua); }
