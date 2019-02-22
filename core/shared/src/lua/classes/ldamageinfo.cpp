#include "stdafx_shared.h"
#include "pragma/lua/classes/ldamageinfo.h"
#include "pragma/game/damageinfo.h"
#include "luasystem.h"
#include "pragma/lua/classes/ldef_entity.h"
#include <mathutil/umath.h>

void Lua::DamageInfo::GetHitGroup(lua_State *l,::DamageInfo &info)
{
	Lua::PushInt(l,umath::to_integral(info.GetHitGroup()));
}
void Lua::DamageInfo::SetHitGroup(lua_State *l,::DamageInfo &info,uint32_t hitGroup)
{
	info.SetHitGroup(static_cast<HitGroup>(hitGroup));
}
void Lua::DamageInfo::SetDamage(lua_State*,::DamageInfo &info,unsigned int dmg)
{
	info.SetDamage(umath::limit<Int16>(dmg));
}
void Lua::DamageInfo::AddDamage(lua_State*,::DamageInfo &info,unsigned int dmg)
{
	info.AddDamage(umath::limit<Int16>(dmg));
}
void Lua::DamageInfo::ScaleDamage(lua_State*,::DamageInfo &info,float scale)
{
	info.ScaleDamage(scale);
}
void Lua::DamageInfo::GetDamage(lua_State *l,::DamageInfo &info)
{
	Lua::PushInt(l,info.GetDamage());
}
void Lua::DamageInfo::GetAttacker(lua_State *l,::DamageInfo &info)
{
	BaseEntity *ent = info.GetAttacker();
	if(ent == NULL)
		return;
	lua_pushentity(l,ent);
}
void Lua::DamageInfo::SetAttacker(lua_State*,::DamageInfo &info,EntityHandle &hAttacker)
{
	if(!hAttacker.IsValid())
		info.SetAttacker(NULL);
	else
		info.SetAttacker(hAttacker.get());
}
void Lua::DamageInfo::GetInflictor(lua_State *l,::DamageInfo &info)
{
	BaseEntity *ent = info.GetInflictor();
	if(ent == NULL)
		return;
	lua_pushentity(l,ent);
}
void Lua::DamageInfo::SetInflictor(lua_State*,::DamageInfo &info,EntityHandle &hInflictor)
{
	if(!hInflictor.IsValid())
		info.SetInflictor(NULL);
	else
		info.SetInflictor(hInflictor.get());
}
void Lua::DamageInfo::GetDamageTypes(lua_State *l,::DamageInfo &info)
{
	Lua::PushInt(l,info.GetDamageTypes());
}
void Lua::DamageInfo::SetDamageType(lua_State*,::DamageInfo &info,int type)
{
	info.SetDamageType(DAMAGETYPE(type));
}
void Lua::DamageInfo::AddDamageType(lua_State*,::DamageInfo &info,int type)
{
	info.AddDamageType(DAMAGETYPE(type));
}
void Lua::DamageInfo::RemoveDamageType(lua_State*,::DamageInfo &info,int type)
{
	info.RemoveDamageType(DAMAGETYPE(type));
}
void Lua::DamageInfo::IsDamageType(lua_State *l,::DamageInfo &info,int type)
{
	Lua::PushBool(l,info.IsDamageType(DAMAGETYPE(type)));
}
void Lua::DamageInfo::SetSource(lua_State*,::DamageInfo &info,Vector3 &source)
{
	info.SetSource(source);
}
void Lua::DamageInfo::GetSource(lua_State *l,::DamageInfo &info)
{
	Lua::Push<Vector3>(l,info.GetSource());
}
void Lua::DamageInfo::SetHitPosition(lua_State*,::DamageInfo &info,Vector3 &hitPos)
{
	info.SetHitPosition(hitPos);
}
void Lua::DamageInfo::GetHitPosition(lua_State *l,::DamageInfo &info)
{
	Lua::Push<Vector3>(l,info.GetHitPosition());
}
void Lua::DamageInfo::SetForce(lua_State*,::DamageInfo &info,Vector3 &force)
{
	info.SetForce(force);
}
void Lua::DamageInfo::GetForce(lua_State *l,::DamageInfo &info)
{
	Lua::Push<Vector3>(l,info.GetForce());
}