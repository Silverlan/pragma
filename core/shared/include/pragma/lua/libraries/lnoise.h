/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __LNOISE_H__
#define __LNOISE_H__
#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>
#include "noise/noise.h"
#include "noiseutils.h"

lua_registercheck(NoiseModule,noise::module::Module);
lua_registercheck(NoiseMap,noise::utils::NoiseMap);

class DLLNETWORK NoiseBaseModule
	: public std::shared_ptr<noise::module::Module>
{
public:
	NoiseBaseModule(noise::module::Module *mod);
};

#define DECLARE_NOISE_MODULE(name) \
	class DLLNETWORK Noise##name \
		: public NoiseBaseModule \
	{ \
	public: \
		using NoiseBaseModule::NoiseBaseModule; \
	}; \
	lua_registercheck(Noise##name,Noise##name);

DECLARE_NOISE_MODULE(Abs);
DECLARE_NOISE_MODULE(Billow);
DECLARE_NOISE_MODULE(Blend);
DECLARE_NOISE_MODULE(Cache);
DECLARE_NOISE_MODULE(Checkerboard);
DECLARE_NOISE_MODULE(Clamp);
DECLARE_NOISE_MODULE(Const);
DECLARE_NOISE_MODULE(Curve);
DECLARE_NOISE_MODULE(Cylinders);
DECLARE_NOISE_MODULE(Displace);
DECLARE_NOISE_MODULE(Exponent);
DECLARE_NOISE_MODULE(Invert);
DECLARE_NOISE_MODULE(Max);
DECLARE_NOISE_MODULE(Min);
DECLARE_NOISE_MODULE(Multiply);
DECLARE_NOISE_MODULE(Perlin);
DECLARE_NOISE_MODULE(Power);
DECLARE_NOISE_MODULE(RidgedMulti);
DECLARE_NOISE_MODULE(RotatePoint);
DECLARE_NOISE_MODULE(ScaleBias);
DECLARE_NOISE_MODULE(ScalePoint);
DECLARE_NOISE_MODULE(Select);
DECLARE_NOISE_MODULE(Spheres);
DECLARE_NOISE_MODULE(Terrace);
DECLARE_NOISE_MODULE(TranslatePoint);
DECLARE_NOISE_MODULE(Turbulance);
DECLARE_NOISE_MODULE(Voronoi);

namespace Lua::noise
{
	DLLNETWORK int perlin(lua_State *l);
	DLLNETWORK int noise_const(lua_State *l);
	DLLNETWORK int voronoi(lua_State *l);
	DLLNETWORK int ridged_multi(lua_State *l);
	DLLNETWORK int billow(lua_State *l);
	DLLNETWORK int generate_height_map(lua_State *l);
};

///////////////////////////////////////

namespace Lua::noise
{
	namespace NoiseModule
	{
		DLLNETWORK void GetValue(lua_State *l,NoiseBaseModule &mod,Vector3 &v);
		DLLNETWORK void SetScale(lua_State *l,NoiseBaseModule &mod,float scale);
	};
	
	namespace VoronoiNoise
	{
		DLLNETWORK void GetDisplacement(lua_State *l,NoiseVoronoi &voronoi);
		DLLNETWORK void GetFrequency(lua_State *l,NoiseVoronoi &voronoi);
		DLLNETWORK void GetSeed(lua_State *l,NoiseVoronoi &voronoi);
		DLLNETWORK void SetDisplacement(lua_State *l,NoiseVoronoi &voronoi,double displacement);
		DLLNETWORK void SetFrequency(lua_State *l,NoiseVoronoi &voronoi,double frequency);
		DLLNETWORK void SetSeed(lua_State *l,NoiseVoronoi &voronoi,int seed);
	};
	
	namespace PerlinNoise
	{
		DLLNETWORK void GetFrequency(lua_State *l,NoisePerlin &perlin);
		DLLNETWORK void GetLacunarity(lua_State *l,NoisePerlin &perlin);
		DLLNETWORK void GetNoiseQuality(lua_State *l,NoisePerlin &perlin);
		DLLNETWORK void GetOctaveCount(lua_State *l,NoisePerlin &perlin);
		DLLNETWORK void GetPersistence(lua_State *l,NoisePerlin &perlin);
		DLLNETWORK void GetSeed(lua_State *l,NoisePerlin &perlin);
		DLLNETWORK void SetFrequency(lua_State *l,NoisePerlin &perlin,double frequency);
		DLLNETWORK void SetLacunarity(lua_State *l,NoisePerlin &perlin,double lacunarity);
		DLLNETWORK void SetNoiseQuality(lua_State *l,NoisePerlin &perlin,int quality);
		DLLNETWORK void SetOctaveCount(lua_State *l,NoisePerlin &perlin,int octaveCount);
		DLLNETWORK void SetPersistence(lua_State *l,NoisePerlin &perlin,double persistence);
		DLLNETWORK void SetSeed(lua_State *l,NoisePerlin &perlin,int seed);
	};
	
	namespace RidgedMultiNoise
	{
		DLLNETWORK void GetFrequency(lua_State *l,NoiseRidgedMulti &ridged);
		DLLNETWORK void GetLacunarity(lua_State *l,NoiseRidgedMulti &ridged);
		DLLNETWORK void GetNoiseQuality(lua_State *l,NoiseRidgedMulti &ridged);
		DLLNETWORK void GetOctaveCount(lua_State *l,NoiseRidgedMulti &ridged);
		DLLNETWORK void GetSeed(lua_State *l,NoiseRidgedMulti &ridged);
		DLLNETWORK void SetFrequency(lua_State *l,NoiseRidgedMulti &ridged,double frequency);
		DLLNETWORK void SetLacunarity(lua_State *l,NoiseRidgedMulti &ridged,double lacunarity);
		DLLNETWORK void SetNoiseQuality(lua_State *l,NoiseRidgedMulti &ridged,int quality);
		DLLNETWORK void SetOctaveCount(lua_State *l,NoiseRidgedMulti &ridged,int octaveCount);
		DLLNETWORK void SetSeed(lua_State *l,NoiseRidgedMulti &ridged,int seed);
	};
	
	namespace BillowNoise
	{
		DLLNETWORK void GetFrequency(lua_State *l,NoiseBillow &billow);
		DLLNETWORK void GetLacunarity(lua_State *l,NoiseBillow &billow);
		DLLNETWORK void GetNoiseQuality(lua_State *l,NoiseBillow &billow);
		DLLNETWORK void GetOctaveCount(lua_State *l,NoiseBillow &billow);
		DLLNETWORK void GetPersistence(lua_State *l,NoiseBillow &billow);
		DLLNETWORK void GetSeed(lua_State *l,NoiseBillow &billow);
		DLLNETWORK void SetFrequency(lua_State *l,NoiseBillow &billow,double frequency);
		DLLNETWORK void SetLacunarity(lua_State *l,NoiseBillow &billow,double lacunarity);
		DLLNETWORK void SetNoiseQuality(lua_State *l,NoiseBillow &billow,int quality);
		DLLNETWORK void SetOctaveCount(lua_State *l,NoiseBillow &billow,int octaveCount);
		DLLNETWORK void SetPersistence(lua_State *l,NoiseBillow &billow,double persistence);
		DLLNETWORK void SetSeed(lua_State *l,NoiseBillow &billow,int seed);
	};
	
	namespace NoiseMap
	{
		DLLNETWORK void GetValue(lua_State *l,::noise::utils::NoiseMap &noiseMap,int x,int y);
		DLLNETWORK void GetHeight(lua_State *l,::noise::utils::NoiseMap &noiseMap);
		DLLNETWORK void GetWidth(lua_State *l,::noise::utils::NoiseMap &noiseMap);
	};
};

#endif