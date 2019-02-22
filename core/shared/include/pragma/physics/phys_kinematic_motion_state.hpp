#ifndef __PHYS_KINEMATIC_MOTION_STATE_HPP__
#define __PHYS_KINEMATIC_MOTION_STATE_HPP__

#include "pragma/physics/phystransform.h"

class KinematicMotionState
	: public btMotionState
{
public:
	KinematicMotionState(const PhysTransform &initialTransform={});
    virtual ~KinematicMotionState() override;

	PhysTransform &GetWorldTransform();
	const PhysTransform &GetWorldTransform() const;
private:
    virtual void getWorldTransform(btTransform &worldTrans) const override;
    virtual void setWorldTransform(const btTransform &worldTrans) override;
	PhysTransform m_transform = {};
};

#endif
