#include "stdafx_shared.h"
#include "pragma/lua/classes/lsurfacematerial.h"
#include "luasystem.h"

void Lua::SurfaceMaterial::GetName(lua_State *l,::SurfaceMaterial &mat)
{
	Lua::PushString(l,mat.GetIdentifier());
}
void Lua::SurfaceMaterial::GetIndex(lua_State *l,::SurfaceMaterial &mat)
{
	Lua::PushInt(l,mat.GetIndex());
}
void Lua::SurfaceMaterial::GetFriction(lua_State *l,::SurfaceMaterial &mat)
{
	Lua::PushNumber(l,mat.GetFriction());
}
void Lua::SurfaceMaterial::SetFriction(lua_State*,::SurfaceMaterial &mat,Float friction)
{
	mat.SetFriction(friction);
}
void Lua::SurfaceMaterial::GetRestitution(lua_State *l,::SurfaceMaterial &mat)
{
	Lua::PushNumber(l,mat.GetRestitution());
}
void Lua::SurfaceMaterial::SetRestitution(lua_State*,::SurfaceMaterial &mat,Float restitution)
{
	mat.SetRestitution(restitution);
}
void Lua::SurfaceMaterial::GetFootstepType(lua_State *l,::SurfaceMaterial &mat)
{
	Lua::PushString(l,mat.GetFootstepType());
}
void Lua::SurfaceMaterial::SetFootstepType(lua_State*,::SurfaceMaterial &mat,const std::string &type)
{
	mat.SetFootstepType(type);
}
void Lua::SurfaceMaterial::SetImpactParticleEffect(lua_State*,::SurfaceMaterial &mat,const std::string &eff)
{
	mat.SetImpactParticleEffect(eff);
}
void Lua::SurfaceMaterial::GetImpactParticleEffect(lua_State *l,::SurfaceMaterial &mat)
{
	Lua::PushString(l,mat.GetImpactParticleEffect());
}
void Lua::SurfaceMaterial::GetBulletImpactSound(lua_State *l,::SurfaceMaterial &mat)
{
	Lua::PushString(l,mat.GetBulletImpactSound());
}
void Lua::SurfaceMaterial::SetBulletImpactSound(lua_State*,::SurfaceMaterial &mat,const std::string &snd)
{
	mat.SetBulletImpactSound(snd);
}
void Lua::SurfaceMaterial::SetHardImpactSound(lua_State*,::SurfaceMaterial &mat,const std::string &snd)
{
	mat.SetHardImpactSound(snd);
}
void Lua::SurfaceMaterial::GetHardImpactSound(lua_State *l,::SurfaceMaterial &mat)
{
	Lua::PushString(l,mat.GetHardImpactSound());
}
void Lua::SurfaceMaterial::SetSoftImpactSound(lua_State*,::SurfaceMaterial &mat,const std::string &snd)
{
	mat.SetSoftImpactSound(snd);
}
void Lua::SurfaceMaterial::GetSoftImpactSound(lua_State *l,::SurfaceMaterial &mat)
{
	Lua::PushString(l,mat.GetSoftImpactSound());
}
void Lua::SurfaceMaterial::SetAudioLowFrequencyAbsorption(lua_State*,::SurfaceMaterial &mat,float absp) {mat.SetAudioLowFrequencyAbsorption(absp);}
void Lua::SurfaceMaterial::GetAudioLowFrequencyAbsorption(lua_State *l,::SurfaceMaterial &mat) {Lua::PushNumber(l,mat.GetAudioLowFrequencyAbsorption());}
void Lua::SurfaceMaterial::SetAudioMidFrequencyAbsorption(lua_State*,::SurfaceMaterial &mat,float absp) {mat.SetAudioMidFrequencyAbsorption(absp);}
void Lua::SurfaceMaterial::GetAudioMidFrequencyAbsorption(lua_State *l,::SurfaceMaterial &mat) {Lua::PushNumber(l,mat.GetAudioMidFrequencyAbsorption());}
void Lua::SurfaceMaterial::SetAudioHighFrequencyAbsorption(lua_State*,::SurfaceMaterial &mat,float absp) {mat.SetAudioHighFrequencyAbsorption(absp);}
void Lua::SurfaceMaterial::GetAudioHighFrequencyAbsorption(lua_State *l,::SurfaceMaterial &mat) {Lua::PushNumber(l,mat.GetAudioHighFrequencyAbsorption());}
void Lua::SurfaceMaterial::SetAudioScattering(lua_State*,::SurfaceMaterial &mat,float scattering) {mat.SetAudioScattering(scattering);}
void Lua::SurfaceMaterial::GetAudioScattering(lua_State *l,::SurfaceMaterial &mat) {Lua::PushNumber(l,mat.GetAudioScattering());}
void Lua::SurfaceMaterial::SetAudioLowFrequencyTransmission(lua_State*,::SurfaceMaterial &mat,float transmission) {mat.SetAudioLowFrequencyTransmission(transmission);}
void Lua::SurfaceMaterial::GetAudioLowFrequencyTransmission(lua_State *l,::SurfaceMaterial &mat) {Lua::PushNumber(l,mat.GetAudioLowFrequencyTransmission());}
void Lua::SurfaceMaterial::SetAudioMidFrequencyTransmission(lua_State*,::SurfaceMaterial &mat,float transmission) {mat.SetAudioMidFrequencyTransmission(transmission);}
void Lua::SurfaceMaterial::GetAudioMidFrequencyTransmission(lua_State *l,::SurfaceMaterial &mat) {Lua::PushNumber(l,mat.GetAudioMidFrequencyTransmission());}
void Lua::SurfaceMaterial::SetAudioHighFrequencyTransmission(lua_State*,::SurfaceMaterial &mat,float transmission) {mat.SetAudioHighFrequencyTransmission(transmission);}
void Lua::SurfaceMaterial::GetAudioHighFrequencyTransmission(lua_State *l,::SurfaceMaterial &mat) {Lua::PushNumber(l,mat.GetAudioHighFrequencyTransmission());}
