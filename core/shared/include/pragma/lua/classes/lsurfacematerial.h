#ifndef __LSURFACEMATERIAL_H__
#define __LSURFACEMATERIAL_H__

#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>

namespace Lua
{
	namespace SurfaceMaterial
	{
		DLLNETWORK void GetName(lua_State *l,::SurfaceMaterial &mat);
		DLLNETWORK void GetIndex(lua_State *l,::SurfaceMaterial &mat);
		DLLNETWORK void GetFriction(lua_State *l,::SurfaceMaterial &mat);
		DLLNETWORK void SetFriction(lua_State *l,::SurfaceMaterial &mat,Float friction);
		DLLNETWORK void GetRestitution(lua_State *l,::SurfaceMaterial &mat);
		DLLNETWORK void SetRestitution(lua_State *l,::SurfaceMaterial &mat,Float restitution);
		DLLNETWORK void GetFootstepType(lua_State *l,::SurfaceMaterial &mat);
		DLLNETWORK void SetFootstepType(lua_State *l,::SurfaceMaterial &mat,const std::string &type);
		DLLNETWORK void SetImpactParticleEffect(lua_State*,::SurfaceMaterial &mat,const std::string &eff);
		DLLNETWORK void GetImpactParticleEffect(lua_State*,::SurfaceMaterial &mat);
		DLLNETWORK void GetBulletImpactSound(lua_State*,::SurfaceMaterial &mat);
		DLLNETWORK void SetBulletImpactSound(lua_State*,::SurfaceMaterial &mat,const std::string &snd);
		DLLNETWORK void SetHardImpactSound(lua_State*,::SurfaceMaterial &mat,const std::string &snd);
		DLLNETWORK void GetHardImpactSound(lua_State*,::SurfaceMaterial &mat);
		DLLNETWORK void SetSoftImpactSound(lua_State*,::SurfaceMaterial &mat,const std::string &snd);
		DLLNETWORK void GetSoftImpactSound(lua_State*,::SurfaceMaterial &mat);

		DLLNETWORK void SetAudioLowFrequencyAbsorption(lua_State*,::SurfaceMaterial &mat,float absp);
		DLLNETWORK void GetAudioLowFrequencyAbsorption(lua_State*,::SurfaceMaterial &mat);
		DLLNETWORK void SetAudioMidFrequencyAbsorption(lua_State*,::SurfaceMaterial &mat,float absp);
		DLLNETWORK void GetAudioMidFrequencyAbsorption(lua_State*,::SurfaceMaterial &mat);
		DLLNETWORK void SetAudioHighFrequencyAbsorption(lua_State*,::SurfaceMaterial &mat,float absp);
		DLLNETWORK void GetAudioHighFrequencyAbsorption(lua_State*,::SurfaceMaterial &mat);
		DLLNETWORK void SetAudioScattering(lua_State*,::SurfaceMaterial &mat,float scattering);
		DLLNETWORK void GetAudioScattering(lua_State*,::SurfaceMaterial &mat);
		DLLNETWORK void SetAudioLowFrequencyTransmission(lua_State*,::SurfaceMaterial &mat,float transmission);
		DLLNETWORK void GetAudioLowFrequencyTransmission(lua_State*,::SurfaceMaterial &mat);
		DLLNETWORK void SetAudioMidFrequencyTransmission(lua_State*,::SurfaceMaterial &mat,float transmission);
		DLLNETWORK void GetAudioMidFrequencyTransmission(lua_State*,::SurfaceMaterial &mat);
		DLLNETWORK void SetAudioHighFrequencyTransmission(lua_State*,::SurfaceMaterial &mat,float transmission);
		DLLNETWORK void GetAudioHighFrequencyTransmission(lua_State*,::SurfaceMaterial &mat);
	};
};

#endif
