#ifndef __PHYS_VEHICLE_HPP__
#define __PHYS_VEHICLE_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/physics/base.hpp"

namespace pragma::physics
{
	class ICollisionObject;
	class DLLNETWORK IVehicle
		: public IBase,public IWorldObject
	{
	public:
		virtual void OnRemove() override;

		ICollisionObject *GetCollisionObject();
		const ICollisionObject *GetCollisionObject() const;
		virtual void InitializeLuaObject(lua_State *lua) override;
	protected:
		IVehicle(IEnvironment &env,const util::TSharedHandle<ICollisionObject> &collisionObject);
		util::TSharedHandle<ICollisionObject> m_collisionObject = nullptr;
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
