#include "stdafx_server.h"
#include "pragma/lua/classes/s_lfaction.h"
#include "pragma/lua/classes/ldef_entity.h"
#include <luasystem.h>

void Lua::Faction::register_class(lua_State *l,luabind::module_ &mod)
{
	auto classDef = luabind::class_<::Faction>("Faction");
	classDef.def("AddClass",&AddClass);
	classDef.def("GetClasses",&GetClasses);
	classDef.def("SetDisposition",static_cast<void(*)(lua_State*,::Faction&,::Faction&,uint32_t,bool,int32_t)>(&SetDisposition));
	classDef.def("SetDisposition",static_cast<void(*)(lua_State*,::Faction&,::Faction&,uint32_t,bool)>(&SetDisposition));
	classDef.def("SetDisposition",static_cast<void(*)(lua_State*,::Faction&,::Faction&,uint32_t)>(&SetDisposition));
	classDef.def("SetEnemyFaction",static_cast<void(*)(lua_State*,::Faction&,::Faction&,bool,int32_t)>(&SetEnemyFaction));
	classDef.def("SetEnemyFaction",static_cast<void(*)(lua_State*,::Faction&,::Faction&,bool)>(&SetEnemyFaction));
	classDef.def("SetEnemyFaction",static_cast<void(*)(lua_State*,::Faction&,::Faction&)>(&SetEnemyFaction));
	classDef.def("SetAlliedFaction",static_cast<void(*)(lua_State*,::Faction&,::Faction&,bool,int32_t)>(&SetAlliedFaction));
	classDef.def("SetAlliedFaction",static_cast<void(*)(lua_State*,::Faction&,::Faction&,bool)>(&SetAlliedFaction));
	classDef.def("SetAlliedFaction",static_cast<void(*)(lua_State*,::Faction&,::Faction&)>(&SetAlliedFaction));
	classDef.def("SetNeutralFaction",static_cast<void(*)(lua_State*,::Faction&,::Faction&,bool,int32_t)>(&SetNeutralFaction));
	classDef.def("SetNeutralFaction",static_cast<void(*)(lua_State*,::Faction&,::Faction&,bool)>(&SetNeutralFaction));
	classDef.def("SetNeutralFaction",static_cast<void(*)(lua_State*,::Faction&,::Faction&)>(&SetNeutralFaction));
	classDef.def("SetFearsomeFaction",static_cast<void(*)(lua_State*,::Faction&,::Faction&,bool,int32_t)>(&SetFearsomeFaction));
	classDef.def("SetFearsomeFaction",static_cast<void(*)(lua_State*,::Faction&,::Faction&,bool)>(&SetFearsomeFaction));
	classDef.def("SetFearsomeFaction",static_cast<void(*)(lua_State*,::Faction&,::Faction&)>(&SetFearsomeFaction));
	classDef.def("GetDisposition",static_cast<void(*)(lua_State*,::Faction&,::Faction&)>(&GetDisposition));
	classDef.def("GetDisposition",static_cast<void(*)(lua_State*,::Faction&,const std::string&)>(&GetDisposition));
	classDef.def("GetDisposition",static_cast<void(*)(lua_State*,::Faction&,EntityHandle&)>(&GetDisposition));
	classDef.def("HasClass",&HasClass);
	classDef.def("SetDefaultDisposition",&SetDefaultDisposition);
	classDef.def("GetDefaultDisposition",&GetDefaultDisposition);
	classDef.def("GetName",&GetName);
	mod[classDef];
}

void Lua::Faction::AddClass(lua_State*,::Faction &faction,const std::string &className) {faction.AddClass(className);}
void Lua::Faction::GetClasses(lua_State *l,::Faction &faction)
{
	auto &classes = faction.GetClasses();
	auto t = Lua::CreateTable(l);
	for(auto i=decltype(classes.size()){0};i<classes.size();++i)
	{
		Lua::PushInt(l,i +1);
		Lua::PushString(l,classes[i]);
		Lua::SetTableValue(l,t);
	}
}
void Lua::Faction::SetDisposition(lua_State*,::Faction &faction,::Faction &factionTgt,uint32_t disposition,bool revert,int32_t priority)
{
	faction.SetDisposition(factionTgt,static_cast<DISPOSITION>(disposition),revert,priority);
}
void Lua::Faction::SetDisposition(lua_State *l,::Faction &faction,::Faction &factionTgt,uint32_t disposition,bool revert) {SetDisposition(l,faction,factionTgt,disposition,revert,0);}
void Lua::Faction::SetDisposition(lua_State *l,::Faction &faction,::Faction &factionTgt,uint32_t disposition) {SetDisposition(l,faction,factionTgt,disposition,false,0);}
void Lua::Faction::SetEnemyFaction(lua_State*,::Faction &faction,::Faction &factionTgt,bool revert,int32_t priority)
{
	faction.SetEnemyFaction(factionTgt,revert,priority);
}
void Lua::Faction::SetEnemyFaction(lua_State *l,::Faction &faction,::Faction &factionTgt,bool revert) {SetEnemyFaction(l,faction,factionTgt,revert,0);}
void Lua::Faction::SetEnemyFaction(lua_State *l,::Faction &faction,::Faction &factionTgt) {SetEnemyFaction(l,faction,factionTgt,false,0);}
void Lua::Faction::SetAlliedFaction(lua_State*,::Faction &faction,::Faction &factionTgt,bool revert,int32_t priority)
{
	faction.SetAlliedFaction(factionTgt,revert,priority);
}
void Lua::Faction::SetAlliedFaction(lua_State *l,::Faction &faction,::Faction &factionTgt,bool revert) {SetAlliedFaction(l,faction,factionTgt,revert,0);}
void Lua::Faction::SetAlliedFaction(lua_State *l,::Faction &faction,::Faction &factionTgt) {SetAlliedFaction(l,faction,factionTgt,false,0);}
void Lua::Faction::SetNeutralFaction(lua_State*,::Faction &faction,::Faction &factionTgt,bool revert,int32_t priority)
{
	faction.SetNeutralFaction(factionTgt,revert,priority);
}
void Lua::Faction::SetNeutralFaction(lua_State *l,::Faction &faction,::Faction &factionTgt,bool revert) {SetNeutralFaction(l,faction,factionTgt,revert,0);}
void Lua::Faction::SetNeutralFaction(lua_State *l,::Faction &faction,::Faction &factionTgt) {SetNeutralFaction(l,faction,factionTgt,false,0);}
void Lua::Faction::SetFearsomeFaction(lua_State*,::Faction &faction,::Faction &factionTgt,bool revert,int32_t priority)
{
	faction.SetFearsomeFaction(factionTgt,revert,priority);
}
void Lua::Faction::SetFearsomeFaction(lua_State *l,::Faction &faction,::Faction &factionTgt,bool revert) {SetFearsomeFaction(l,faction,factionTgt,revert,0);}
void Lua::Faction::SetFearsomeFaction(lua_State *l,::Faction &faction,::Faction &factionTgt) {SetFearsomeFaction(l,faction,factionTgt,false,0);}
void Lua::Faction::GetDisposition(lua_State *l,::Faction &faction,::Faction &factionTgt)
{
	int32_t priority = 0;
	auto disp = faction.GetDisposition(factionTgt,&priority);
	Lua::PushInt(l,umath::to_integral(disp));
	Lua::PushInt(l,priority);
}
void Lua::Faction::GetDisposition(lua_State *l,::Faction &faction,const std::string &className)
{
	int32_t priority = 0;
	auto disp = faction.GetDisposition(className,&priority);
	Lua::PushInt(l,umath::to_integral(disp));
	Lua::PushInt(l,priority);
}
void Lua::Faction::GetDisposition(lua_State *l,::Faction &faction,EntityHandle &hEnt)
{
	LUA_CHECK_ENTITY(l,hEnt);
	int32_t priority = 0;
	auto disp = faction.GetDisposition(hEnt,&priority);
	Lua::PushInt(l,umath::to_integral(disp));
	Lua::PushInt(l,priority);
}
void Lua::Faction::HasClass(lua_State *l,::Faction &faction,const std::string &className)
{
	Lua::PushBool(l,faction.HasClass(className));
}
void Lua::Faction::SetDefaultDisposition(lua_State*,::Faction &faction,uint32_t disposition)
{
	faction.SetDefaultDisposition(static_cast<DISPOSITION>(disposition));
}
void Lua::Faction::GetDefaultDisposition(lua_State *l,::Faction &faction)
{
	Lua::PushInt(l,umath::to_integral(faction.GetDefaultDisposition()));
}
void Lua::Faction::GetName(lua_State *l,::Faction &faction)
{
	Lua::PushString(l,faction.GetName());
}
