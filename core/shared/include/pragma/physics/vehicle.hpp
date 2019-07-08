#ifndef __PHYS_VEHICLE_HPP__
#define __PHYS_VEHICLE_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/physics/base.hpp"

namespace pragma::physics
{
	class DLLNETWORK IVehicle
		: public IBase,public IWorldObject
	{
	public:
		virtual void OnRemove() override;
	protected:
		IVehicle(IEnvironment &env);
	};
	
	class DLLNETWORK IWheel
		: public IBase
	{
	public:
	protected:
		IWheel(IEnvironment &env);
	};
};

#endif
