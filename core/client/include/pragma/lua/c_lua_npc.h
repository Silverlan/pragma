#ifndef __C_LUA_NPC_H__
#define __C_LUA_NPC_H__

#include "pragma/clientdefinitions.h"
// COMPONENT TODO
#if 0
#include "c_basenpc.h"
#include "pragma/lua/classes/c_lua_entity.h"
#include "sh_lua_npc.h"
#include "npchandle.h"

class CLuaNPC;
DECLARE_LUA_ENTITY_HANDLE(DLLCLIENT,CLuaNPC,NPCHandle,CLuaEntity);

DECLARE_LUA_ENTITY_BASE(DLLCLIENT,CLuaNPC,public CBaseNPC COMMA public CLuaBaseEntity COMMA public BaseSharedLuaNPC,
protected:
	virtual Vector2 CalcMovementSpeed() const override;
	virtual void PlayFootStepSound(BaseCharacter::FootType foot,const SurfaceMaterial &surfMat,float scale) override;
public:
	virtual void ReceiveData(NetPacket &packet) override;
	virtual Bool ReceiveNetEvent(UInt32 eventId,NetPacket &packet) override;
	virtual void Render(RenderMode renderMode) override;
	virtual void PostRender(RenderMode renderMode) override;
);

DECLARE_LUA_ENTITY_WRAPPER_DERIVED(DLLCLIENT,CLuaNPC,LuaNPCWrapper COMMA public CLuaBaseEntityWrapper,
public:
	virtual void ReceiveData(NetPacket&) override {};
	virtual Bool ReceiveNetEvent(UInt32,NetPacket&) override {return false;};
	virtual void OnRender(uint32_t) override {};
	virtual void OnPostRender(uint32_t) override {};

	void PlayFootStepSound(uint32_t,SurfaceMaterial*,float) {}
	static bool default_PlayFootStepSound(lua_State*,NPCHandle&,uint32_t,SurfaceMaterial*,float) {return false;}

	void CalcMovementSpeed(Vector2 speed) {}
	static float default_CalcMovementSpeed(lua_State*,NPCHandle&,Vector2 speed) {return std::numeric_limits<float>::quiet_NaN();}
);
#endif

#endif