#ifndef __S_LUA_NPC_H__
#define __S_LUA_NPC_H__

#if 0

#include "pragma/serverdefinitions.h"
#include "pragma/lua/classes/s_lua_entity.h"
#include "sh_lua_npc.h"
#include "npchandle.h"

class SLuaNPC;
DECLARE_LUA_ENTITY_HANDLE(DLLSERVER,SLuaNPC,NPCHandle,SLuaEntity);

DECLARE_LUA_ENTITY_BASE(DLLSERVER,SLuaNPC,public SBaseNPC COMMA public SLuaBaseEntity COMMA public BaseSharedLuaNPC,
protected:
	virtual Vector2 CalcMovementSpeed() const override;
	virtual void SelectSchedule() override;
	virtual void SelectControllerSchedule() override;
	virtual void OnPrimaryTargetChanged(const ai::Memory::Fragment *memFragment) override;
	virtual void OnNPCStateChanged(NPCSTATE oldState,NPCSTATE newState) override;
	virtual void OnTargetVisibilityLost(const ai::Memory::Fragment &memFragment) override;
	virtual void OnTargetVisibilityReacquired(const ai::Memory::Fragment &memFragment) override;
	virtual void OnMemoryGained(const ai::Memory::Fragment &memFragment) override;
	virtual void OnMemoryLost(const ai::Memory::Fragment &memFragment) override;
	virtual void OnTargetAcquired(BaseEntity *ent,float dist,bool bFirst) override;
	virtual void OnScheduleComplete() override;
	virtual bool OnSuspiciousSoundHeared(std::shared_ptr<ALSound> &snd) override;
	virtual void OnControllerActionInput(Action action,bool b) override;
	virtual void OnStartControl(Player *pl) override;
	virtual void OnEndControl() override;
	virtual void OnDeath(DamageInfo *damageInfo) override;
	virtual void PlayFootStepSound(BaseCharacter::FootType foot,const SurfaceMaterial &surfMat,float scale) override;
public:
	virtual void SendData(NetPacket &packet,networking::ClientRecipientFilter &rp) override;
	virtual Bool ReceiveNetEvent(Player *pl,UInt32 eventId,NetPacket &packet) override;
);

// These have to be registered in "class.cpp" as well!
DECLARE_LUA_ENTITY_WRAPPER_DERIVED(DLLSERVER,SLuaNPC,LuaNPCWrapper COMMA public SLuaBaseEntityWrapper,
public:
	virtual void SendData(NetPacket&,nwm::RecipientFilter&) override {};
	virtual Bool ReceiveNetEvent(PlayerHandle&,UInt32,NetPacket&) override {return false;};

	void SelectSchedule() {}
	static void default_SelectSchedule(lua_State*,NPCHandle&) {}

	void SelectControllerSchedule() {}
	static void default_SelectControllerSchedule(lua_State*,NPCHandle&) {}

	void OnPrimaryTargetChanged(ai::Memory::Fragment*) {}
	static void default_OnPrimaryTargetChanged(lua_State*,NPCHandle&,ai::Memory::Fragment*) {}

	void OnNPCStateChanged(uint32_t,uint32_t) {}
	static void default_OnNPCStateChanged(lua_State*,NPCHandle&,uint32_t,uint32_t) {}

	void OnTargetVisibilityLost(ai::Memory::Fragment*) {}
	static void default_OnTargetVisibilityLost(lua_State*,NPCHandle&,ai::Memory::Fragment*) {}

	void OnTargetVisibilityReacquired(ai::Memory::Fragment*) {}
	static void default_OnTargetVisibilityReacquired(lua_State*,NPCHandle&,ai::Memory::Fragment*) {}

	void OnMemoryGained(ai::Memory::Fragment*) {}
	static void default_OnMemoryGained(lua_State*,NPCHandle&,ai::Memory::Fragment*) {}

	void OnMemoryLost(ai::Memory::Fragment*) {}
	static void default_OnMemoryLost(lua_State*,NPCHandle&,ai::Memory::Fragment*) {}

	void OnTargetAcquired(EntityHandle&,float,bool) {}
	static void default_OnTargetAcquired(lua_State*,NPCHandle&,EntityHandle&,float,bool) {}

	void OnScheduleComplete() {}
	static void default_OnScheduleComplete(lua_State*,NPCHandle&) {}

	bool OnSuspiciousSoundHeared(std::shared_ptr<ALSound>&) {return false;}
	static bool default_OnSuspiciousSoundHeared(lua_State*,NPCHandle&,std::shared_ptr<ALSound>&) {return false;}

	void OnControllerActionInput(std::underlying_type_t<Action>,bool) {}
	static void default_OnControllerActionInput(lua_State*,NPCHandle&,std::underlying_type_t<Action>,bool) {}

	void OnStartControl(PlayerHandle&) {}
	static void default_OnStartControl(lua_State*,NPCHandle&,PlayerHandle&) {}

	void OnEndControl() {}
	static void default_OnEndControl(lua_State*,NPCHandle&) {}

	void OnDeath(DamageInfo*) {}
	static bool default_OnDeath(lua_State*,NPCHandle&,DamageInfo*) {return false;}

	void PlayFootStepSound(uint32_t,SurfaceMaterial*,float) {}
	static bool default_PlayFootStepSound(lua_State*,NPCHandle&,uint32_t,SurfaceMaterial*,float) {return false;}

	void CalcMovementSpeed(Vector2 speed) {}
	static float default_CalcMovementSpeed(lua_State*,NPCHandle&,Vector2 speed) {return std::numeric_limits<float>::quiet_NaN();}
);

#endif

#endif
