// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include <ostream>

#include "pragma/lua/core.hpp"

#include <string>

module pragma.shared;

import :physics.constraint;

pragma::physics::IConstraint::IConstraint(IEnvironment &env) : IBase {env} {}

void pragma::physics::IConstraint::OnRemove()
{
	RemoveWorldObject();
	m_physEnv.RemoveConstraint(*this);
	IBase::OnRemove();
}

void pragma::physics::IConstraint::SetCollisionsEnabled(Bool b)
{
	if(b == m_bCollisionsEnabled)
		return;
	m_bCollisionsEnabled = b;
	DoSetCollisionsEnabled(b);
}
bool pragma::physics::IConstraint::GetCollisionsEnabled() const { return m_bCollisionsEnabled; }

void pragma::physics::IConstraint::EnableCollisions() { SetCollisionsEnabled(true); }
void pragma::physics::IConstraint::DisableCollisions() { SetCollisionsEnabled(false); }

umath::Transform &pragma::physics::IConstraint::GetSourceTransform() { return m_srcTransform; }
umath::Transform &pragma::physics::IConstraint::GetTargetTransform() { return m_tgtTransform; }
Vector3 pragma::physics::IConstraint::GetSourcePosition() { return m_srcTransform.GetOrigin(); }
Quat pragma::physics::IConstraint::GetSourceRotation() { return m_srcTransform.GetRotation(); }
Vector3 pragma::physics::IConstraint::GetTargetPosition() { return m_tgtTransform.GetOrigin(); }
Quat pragma::physics::IConstraint::GetTargetRotation() { return m_tgtTransform.GetRotation(); }
pragma::ecs::BaseEntity *pragma::physics::IConstraint::GetEntity() const { return const_cast<pragma::ecs::BaseEntity *>(m_hEntity.get()); }
void pragma::physics::IConstraint::SetEntity(pragma::ecs::BaseEntity &ent) { m_hEntity = ent.GetHandle(); }

void pragma::physics::IConstraint::OnBroken() { m_physEnv.OnConstraintBroken(*this); }
void pragma::physics::IConstraint::InitializeLuaObject(lua_State *lua) { IBase::InitializeLuaObject<IConstraint>(lua); }
void pragma::physics::IFixedConstraint::InitializeLuaObject(lua_State *lua) { IBase::InitializeLuaObject<IFixedConstraint>(lua); }
void pragma::physics::IBallSocketConstraint::InitializeLuaObject(lua_State *lua) { IBase::InitializeLuaObject<IBallSocketConstraint>(lua); }
void pragma::physics::IHingeConstraint::InitializeLuaObject(lua_State *lua) { IBase::InitializeLuaObject<IHingeConstraint>(lua); }
void pragma::physics::ISliderConstraint::InitializeLuaObject(lua_State *lua) { IBase::InitializeLuaObject<ISliderConstraint>(lua); }
void pragma::physics::IConeTwistConstraint::InitializeLuaObject(lua_State *lua) { IBase::InitializeLuaObject<IConeTwistConstraint>(lua); }
void pragma::physics::IDoFConstraint::InitializeLuaObject(lua_State *lua) { IBase::InitializeLuaObject<IDoFConstraint>(lua); }
void pragma::physics::IDoFSpringConstraint::InitializeLuaObject(lua_State *lua) { IBase::InitializeLuaObject<IDoFSpringConstraint>(lua); }
