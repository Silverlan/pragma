#include "stdafx_shared.h"
#include "pragma/physics/environment.hpp"
#include "pragma/physics/constraint.hpp"

pragma::physics::IConstraint::IConstraint(IEnvironment &env)
	: IBase{env}
{}

void pragma::physics::IConstraint::OnRemove()
{
	RemoveWorldObject();
	m_physEnv.RemoveConstraint(*this);
	IBase::OnRemove();
}

void pragma::physics::IConstraint::SetCollisionsEnabled(Bool b)
{
	if(m_bCollisionsEnabled == m_bCollisionsEnabled)
		return;
	m_bCollisionsEnabled = b;
	DoSetCollisionsEnabled(b);
}
bool pragma::physics::IConstraint::GetCollisionsEnabled() const {return m_bCollisionsEnabled;}

pragma::physics::Transform &pragma::physics::IConstraint::GetSourceTransform() {return m_srcTransform;}
pragma::physics::Transform &pragma::physics::IConstraint::GetTargetTransform() {return m_tgtTransform;}
Vector3 pragma::physics::IConstraint::GetSourcePosition() {return m_srcTransform.GetOrigin();}
Quat pragma::physics::IConstraint::GetSourceRotation() {return m_srcTransform.GetRotation();}
Vector3 pragma::physics::IConstraint::GetTargetPosition() {return m_tgtTransform.GetOrigin();}
Quat pragma::physics::IConstraint::GetTargetRotation() {return m_tgtTransform.GetRotation();}
void pragma::physics::IConstraint::InitializeLuaObject(lua_State *lua)
{
	IBase::InitializeLuaObject<IConstraint>(lua);
}
void pragma::physics::IFixedConstraint::InitializeLuaObject(lua_State *lua)
{
	IBase::InitializeLuaObject<IFixedConstraint>(lua);
}
void pragma::physics::IBallSocketConstraint::InitializeLuaObject(lua_State *lua)
{
	IBase::InitializeLuaObject<IBallSocketConstraint>(lua);
}
void pragma::physics::IHingeConstraint::InitializeLuaObject(lua_State *lua)
{
	IBase::InitializeLuaObject<IHingeConstraint>(lua);
}
void pragma::physics::ISliderConstraint::InitializeLuaObject(lua_State *lua)
{
	IBase::InitializeLuaObject<ISliderConstraint>(lua);
}
void pragma::physics::IConeTwistConstraint::InitializeLuaObject(lua_State *lua)
{
	IBase::InitializeLuaObject<IConeTwistConstraint>(lua);
}
void pragma::physics::IDoFConstraint::InitializeLuaObject(lua_State *lua)
{
	IBase::InitializeLuaObject<IDoFConstraint>(lua);
}
void pragma::physics::IDoFSpringConstraint::InitializeLuaObject(lua_State *lua)
{
	IBase::InitializeLuaObject<IDoFSpringConstraint>(lua);
}
