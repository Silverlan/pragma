#include "stdafx_shared.h"
#include "pragma/physics/vehicle.hpp"
#include "pragma/physics/environment.hpp"

float pragma::physics::WheelCreateInfo::GetMomentOfInertia() const
{
	if(momentOfInertia.has_value())
		return *momentOfInertia;
	// MOI of a cylinder
	return 0.5f *mass *umath::pow2(radius);
}

pragma::physics::IVehicle::IVehicle(IEnvironment &env,const util::TSharedHandle<ICollisionObject> &collisionObject)
	: IBase{env},m_collisionObject{collisionObject}
{}

pragma::physics::ICollisionObject *pragma::physics::IVehicle::GetCollisionObject() {return m_collisionObject.Get();}
const pragma::physics::ICollisionObject *pragma::physics::IVehicle::GetCollisionObject() const {return const_cast<IVehicle*>(this)->GetCollisionObject();}

void pragma::physics::IVehicle::InitializeLuaObject(lua_State *lua)
{
	IBase::InitializeLuaObject<IVehicle>(lua);
}

void pragma::physics::IVehicle::OnRemove()
{
	RemoveWorldObject();
	m_physEnv.RemoveVehicle(*this);
	IBase::OnRemove();
}

pragma::physics::IWheel::IWheel(IEnvironment &env)
	: IBase{env}
{}
