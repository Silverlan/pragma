#include "stdafx_shared.h"
#include "pragma/physics/vehicle.hpp"

pragma::physics::IVehicle::IVehicle(IEnvironment &env)
	: IBase{env}
{}

pragma::physics::IWheel::IWheel(IEnvironment &env)
	: IBase{env}
{}
