#ifndef __LNOISE_H__
#define __LNOISE_H__
#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>
#include "noise/noise.h"
#include "noise/noiseutils.h"

lua_registercheck(NoiseModule,std::shared_ptr<noise::module::Module>);
lua_registercheck(NoiseMap,std::shared_ptr<noise::utils::NoiseMap>);

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

DLLNETWORK int Lua_noise_perlin(lua_State *l);
DLLNETWORK int Lua_noise_const(lua_State *l);
DLLNETWORK int Lua_noise_voronoi(lua_State *l);
DLLNETWORK int Lua_noise_ridged_multi(lua_State *l);
DLLNETWORK int Lua_noise_billow(lua_State *l);
DLLNETWORK int Lua_noise_generate_height_map(lua_State *l);

///////////////////////////////////////

DLLNETWORK void Lua_NoiseModule_GetValue(lua_State *l,NoiseBaseModule &mod,Vector3 &v);
DLLNETWORK void Lua_NoiseModule_SetScale(lua_State *l,NoiseBaseModule &mod,float scale);

DLLNETWORK void Lua_VoronoiNoise_GetDisplacement(lua_State *l,NoiseVoronoi &voronoi);
DLLNETWORK void Lua_VoronoiNoise_GetFrequency(lua_State *l,NoiseVoronoi &voronoi);
DLLNETWORK void Lua_VoronoiNoise_GetSeed(lua_State *l,NoiseVoronoi &voronoi);
DLLNETWORK void Lua_VoronoiNoise_SetDisplacement(lua_State *l,NoiseVoronoi &voronoi,double displacement);
DLLNETWORK void Lua_VoronoiNoise_SetFrequency(lua_State *l,NoiseVoronoi &voronoi,double frequency);
DLLNETWORK void Lua_VoronoiNoise_SetSeed(lua_State *l,NoiseVoronoi &voronoi,int seed);

DLLNETWORK void Lua_PerlinNoise_GetFrequency(lua_State *l,NoisePerlin &perlin);
DLLNETWORK void Lua_PerlinNoise_GetLacunarity(lua_State *l,NoisePerlin &perlin);
DLLNETWORK void Lua_PerlinNoise_GetNoiseQuality(lua_State *l,NoisePerlin &perlin);
DLLNETWORK void Lua_PerlinNoise_GetOctaveCount(lua_State *l,NoisePerlin &perlin);
DLLNETWORK void Lua_PerlinNoise_GetPersistence(lua_State *l,NoisePerlin &perlin);
DLLNETWORK void Lua_PerlinNoise_GetSeed(lua_State *l,NoisePerlin &perlin);
DLLNETWORK void Lua_PerlinNoise_SetFrequency(lua_State *l,NoisePerlin &perlin,double frequency);
DLLNETWORK void Lua_PerlinNoise_SetLacunarity(lua_State *l,NoisePerlin &perlin,double lacunarity);
DLLNETWORK void Lua_PerlinNoise_SetNoiseQuality(lua_State *l,NoisePerlin &perlin,int quality);
DLLNETWORK void Lua_PerlinNoise_SetOctaveCount(lua_State *l,NoisePerlin &perlin,int octaveCount);
DLLNETWORK void Lua_PerlinNoise_SetPersistence(lua_State *l,NoisePerlin &perlin,double persistence);
DLLNETWORK void Lua_PerlinNoise_SetSeed(lua_State *l,NoisePerlin &perlin,int seed);

DLLNETWORK void Lua_RidgedMultiNoise_GetFrequency(lua_State *l,NoiseRidgedMulti &ridged);
DLLNETWORK void Lua_RidgedMultiNoise_GetLacunarity(lua_State *l,NoiseRidgedMulti &ridged);
DLLNETWORK void Lua_RidgedMultiNoise_GetNoiseQuality(lua_State *l,NoiseRidgedMulti &ridged);
DLLNETWORK void Lua_RidgedMultiNoise_GetOctaveCount(lua_State *l,NoiseRidgedMulti &ridged);
DLLNETWORK void Lua_RidgedMultiNoise_GetSeed(lua_State *l,NoiseRidgedMulti &ridged);
DLLNETWORK void Lua_RidgedMultiNoise_SetFrequency(lua_State *l,NoiseRidgedMulti &ridged,double frequency);
DLLNETWORK void Lua_RidgedMultiNoise_SetLacunarity(lua_State *l,NoiseRidgedMulti &ridged,double lacunarity);
DLLNETWORK void Lua_RidgedMultiNoise_SetNoiseQuality(lua_State *l,NoiseRidgedMulti &ridged,int quality);
DLLNETWORK void Lua_RidgedMultiNoise_SetOctaveCount(lua_State *l,NoiseRidgedMulti &ridged,int octaveCount);
DLLNETWORK void Lua_RidgedMultiNoise_SetSeed(lua_State *l,NoiseRidgedMulti &ridged,int seed);

DLLNETWORK void Lua_BillowNoise_GetFrequency(lua_State *l,NoiseBillow &billow);
DLLNETWORK void Lua_BillowNoise_GetLacunarity(lua_State *l,NoiseBillow &billow);
DLLNETWORK void Lua_BillowNoise_GetNoiseQuality(lua_State *l,NoiseBillow &billow);
DLLNETWORK void Lua_BillowNoise_GetOctaveCount(lua_State *l,NoiseBillow &billow);
DLLNETWORK void Lua_BillowNoise_GetPersistence(lua_State *l,NoiseBillow &billow);
DLLNETWORK void Lua_BillowNoise_GetSeed(lua_State *l,NoiseBillow &billow);
DLLNETWORK void Lua_BillowNoise_SetFrequency(lua_State *l,NoiseBillow &billow,double frequency);
DLLNETWORK void Lua_BillowNoise_SetLacunarity(lua_State *l,NoiseBillow &billow,double lacunarity);
DLLNETWORK void Lua_BillowNoise_SetNoiseQuality(lua_State *l,NoiseBillow &billow,int quality);
DLLNETWORK void Lua_BillowNoise_SetOctaveCount(lua_State *l,NoiseBillow &billow,int octaveCount);
DLLNETWORK void Lua_BillowNoise_SetPersistence(lua_State *l,NoiseBillow &billow,double persistence);
DLLNETWORK void Lua_BillowNoise_SetSeed(lua_State *l,NoiseBillow &billow,int seed);

DLLNETWORK void Lua_NoiseMap_GetValue(lua_State *l,std::shared_ptr<noise::utils::NoiseMap> &noiseMap,int x,int y);
DLLNETWORK void Lua_NoiseMap_GetHeight(lua_State *l,std::shared_ptr<noise::utils::NoiseMap> &noiseMap);
DLLNETWORK void Lua_NoiseMap_GetWidth(lua_State *l,std::shared_ptr<noise::utils::NoiseMap> &noiseMap);

#endif