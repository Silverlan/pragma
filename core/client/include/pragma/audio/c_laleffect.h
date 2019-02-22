#ifndef __C_LALEFFECT_H__
#define __C_LALEFFECT_H__

#include "pragma/clientdefinitions.h"
#include "pragma/lua/c_ldefinitions.h"

namespace al
{
	class Effect;
	using PEffect = std::shared_ptr<Effect>;
	struct EfxEaxReverbProperties;
	struct EfxChorusProperties;
	struct EfxDistortionProperties;
	struct EfxEchoProperties;
	struct EfxFlangerProperties;
	struct EfxFrequencyShifterProperties;
	struct EfxVocalMorpherProperties;
	struct EfxPitchShifterProperties;
	struct EfxRingModulatorProperties;
	struct EfxAutoWahProperties;
	struct EfxCompressor;
	struct EfxEqualizer;
};
namespace Lua
{
	namespace ALEffect
	{
		DLLCLIENT void SetProperties(lua_State *l,al::PEffect &effect,const al::EfxEaxReverbProperties &props);
		DLLCLIENT void SetProperties(lua_State *l,al::PEffect &effect,const al::EfxChorusProperties &props);
		DLLCLIENT void SetProperties(lua_State *l,al::PEffect &effect,const al::EfxDistortionProperties &props);
		DLLCLIENT void SetProperties(lua_State *l,al::PEffect &effect,const al::EfxEchoProperties &props);
		DLLCLIENT void SetProperties(lua_State *l,al::PEffect &effect,const al::EfxFlangerProperties &props);
		DLLCLIENT void SetProperties(lua_State *l,al::PEffect &effect,const al::EfxFrequencyShifterProperties &props);
		DLLCLIENT void SetProperties(lua_State *l,al::PEffect &effect,const al::EfxVocalMorpherProperties &props);
		DLLCLIENT void SetProperties(lua_State *l,al::PEffect &effect,const al::EfxPitchShifterProperties &props);
		DLLCLIENT void SetProperties(lua_State *l,al::PEffect &effect,const al::EfxRingModulatorProperties &props);
		DLLCLIENT void SetProperties(lua_State *l,al::PEffect &effect,const al::EfxAutoWahProperties &props);
		DLLCLIENT void SetProperties(lua_State *l,al::PEffect &effect,const al::EfxCompressor &props);
		DLLCLIENT void SetProperties(lua_State *l,al::PEffect &effect,const al::EfxEqualizer &props);
	};
};

#endif
