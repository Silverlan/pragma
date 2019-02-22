#ifndef __S_LUA_VEHICLE_H__
#define __S_LUA_VEHICLE_H__

#if 0
#include "pragma/serverdefinitions.h"
#include "s_basevehicle.h"
#include "pragma/lua/classes/s_lua_entity.h"
#include "sh_lua_vehicle.h"
#include "vehiclehandle.h"
#include <baseluavehicle.h>

class SLuaVehicle;
DECLARE_LUA_ENTITY_HANDLE(DLLSERVER,SLuaVehicle,VehicleHandle,SLuaEntity);

DECLARE_LUA_ENTITY_BASE(DLLSERVER,SLuaVehicle,public SBaseVehicle COMMA public BaseLuaVehicle COMMA public SLuaBaseEntity,
public:
	virtual void ClearDriver() override;
	virtual void SetDriver(BaseEntity *ent) override;

	virtual void SendData(NetPacket &packet,nwm::RecipientFilter &rp) override;
	virtual Bool ReceiveNetEvent(Player *pl,UInt32 eventId,NetPacket &packet) override;
);

DECLARE_LUA_ENTITY_WRAPPER_DERIVED(DLLSERVER,SLuaVehicle,LuaVehicleWrapper COMMA public SLuaBaseEntityWrapper,
public:
	virtual void SendData(NetPacket&,nwm::RecipientFilter&) override {};
	virtual Bool ReceiveNetEvent(PlayerHandle&,UInt32,NetPacket&) override {return false;};
);
#endif

#endif
