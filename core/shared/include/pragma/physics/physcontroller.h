#ifndef __PHYSCONTROLLER_H__
#define __PHYSCONTROLLER_H__

#include "pragma/networkdefinitions.h"
#include <memory>
#include <sharedutils/def_handle.h>
#include <pragma/physics/physapi.h>
#include "pragma/physics/phystransform.h"
#include "pragma/physics/physbase.h"
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include "pragma/physics/physkinematiccharactercontroller.h"
#include <vector>

class PhysControllerHandle;
class PhysEnv;
class PhysConvexShape;
class PhysGhostObject;
class DLLNETWORK PhysController
	: public PhysBase
{
public:
	friend PhysEnv;
private:
	PhysController(PhysEnv *env);
protected:
	PhysController(PhysEnv *env,float halfWidth,float halfHeight,float stepHeight,const PhysTransform &startTransform=PhysTransform());
	PhysController(PhysEnv *env,const Vector3 &halfExtents,float stepHeight,const PhysTransform &startTransform=PhysTransform());

	std::unique_ptr<PhysKinematicCharacterController> m_controller;
	std::unique_ptr<PhysGhostObject> m_ghostObject;
	std::shared_ptr<PhysConvexShape> m_shape;
	std::unique_ptr<PhysControllerHandle> m_handle;
public:
	virtual ~PhysController() override;
	PhysKinematicCharacterController *GetCharacterController();
	PhysGhostObject *GetGhostObject();
	PhysControllerHandle *CreateHandle();
	PhysControllerHandle GetHandle();
	void SetWalkDirection(Vector3 &disp);
	Vector3 GetWalkDirection() const;
	std::shared_ptr<PhysConvexShape> GetShape();
	Vector3 GetDimensions() const;
	void SetDimensions(const Vector3 &dimensions);
	float GetMaxSlope() const;
	void SetMaxSlope(float slope);
};

DECLARE_BASE_HANDLE(DLLNETWORK,PhysController,PhysController);

#endif