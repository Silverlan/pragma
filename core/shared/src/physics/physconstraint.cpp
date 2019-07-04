#include "stdafx_shared.h"
#include "pragma/physics/environment.hpp"
#include "pragma/physics/constraint.hpp"

pragma::physics::IConstraint::IConstraint(IEnvironment &env)
	: IBase{env}
{}

void pragma::physics::IConstraint::Initialize() {}
void pragma::physics::IConstraint::OnRemove()
{
	IBase::OnRemove();
	m_physEnv.RemoveConstraint(*this);
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
