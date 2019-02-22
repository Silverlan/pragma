#include "stdafx_shared.h"
#include "pragma/physics/phys_kinematic_motion_state.hpp"

KinematicMotionState::KinematicMotionState(const PhysTransform &initialTransform)
	: m_transform{initialTransform}
{}
KinematicMotionState::~KinematicMotionState() {}
void KinematicMotionState::getWorldTransform(btTransform &worldTrans) const {worldTrans = m_transform.GetTransform();}
void KinematicMotionState::setWorldTransform(const btTransform &worldTrans) {/*m_transform.SetTransform(worldTrans);*/}
PhysTransform &KinematicMotionState::GetWorldTransform() {return m_transform;}
const PhysTransform &KinematicMotionState::GetWorldTransform() const {return const_cast<KinematicMotionState*>(this)->GetWorldTransform();}
