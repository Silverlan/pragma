#ifndef __C_LUA_VEHICLE_H__
#define __C_LUA_VEHICLE_H__

#include "pragma/clientdefinitions.h"
// COMPONENT TODO
#if 0
#include "c_basevehicle.h"
#include "pragma/lua/classes/c_lua_entity.h"
#include "sh_lua_vehicle.h"
#include "vehiclehandle.h"
#include <baseluavehicle.h>

class CLuaVehicle;
DECLARE_LUA_ENTITY_HANDLE(DLLCLIENT,CLuaVehicle,VehicleHandle,CLuaEntity);

DECLARE_LUA_ENTITY_BASE(DLLCLIENT,CLuaVehicle,public CBaseVehicle COMMA public BaseLuaVehicle COMMA public CLuaBaseEntity,
public:
	virtual void ClearDriver() override;
	virtual void SetDriver(BaseEntity *ent) override;

	virtual void ReceiveData(NetPacket &packet) override;
	virtual Bool ReceiveNetEvent(UInt32 eventId,NetPacket &packet) override;
	virtual void Render(RenderMode renderMode) override;
	virtual void PostRender(RenderMode renderMode) override;
	virtual void SetCamera(bool bFirstPerson) override;
);

DECLARE_LUA_ENTITY_WRAPPER_DERIVED(DLLCLIENT,CLuaVehicle,LuaVehicleWrapper COMMA public CLuaBaseEntityWrapper,
public:
	virtual void ReceiveData(NetPacket&) override {};
	virtual Bool ReceiveNetEvent(UInt32,NetPacket&) override {return false;};
	virtual void OnRender(uint32_t) override {};
	virtual void OnPostRender(uint32_t) override {};
);

#endif

#endif