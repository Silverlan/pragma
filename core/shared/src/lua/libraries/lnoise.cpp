/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/lua/libraries/lnoise.h"
#include "pragma/lua/classes/ldef_vector.h"
#include "luasystem.h"

NoiseBaseModule::NoiseBaseModule(noise::module::Module *mod)
	: std::shared_ptr<noise::module::Module>(mod)
{}

int Lua_noise_perlin(lua_State *l)
{
	Lua::Push<NoisePerlin>(l,NoisePerlin(new noise::module::Perlin));
	return 1;
}

int Lua_noise_const(lua_State *l)
{
	Lua::Push<NoiseConst>(l,NoiseConst(new noise::module::Const));
	return 1;
}

int Lua_noise_voronoi(lua_State *l)
{
	Lua::Push<NoiseVoronoi>(l,NoiseVoronoi(new noise::module::Voronoi));
	return 1;
}

int Lua_noise_ridged_multi(lua_State *l)
{
	Lua::Push<NoiseRidgedMulti>(l,NoiseRidgedMulti(new noise::module::RidgedMulti));
	return 1;
}

int Lua_noise_billow(lua_State *l)
{
	Lua::Push<NoiseBillow>(l,NoiseBillow(new noise::module::Billow));
	return 1;
}

int Lua_noise_generate_height_map(lua_State *l)
{
	auto *noiseModule = Lua::CheckNoiseModule(l,1);
	auto *destSize = Lua::CheckVector2(l,2);
	auto *bottomLeft = Lua::CheckVector2(l,3);
	auto *upperRight = Lua::CheckVector2(l,4);
	auto heightMap = std::make_shared<noise::utils::NoiseMap>();
	noise::utils::NoiseMapBuilderPlane heightMapBuilder;
	heightMapBuilder.SetSourceModule(*noiseModule);
	heightMapBuilder.SetDestNoiseMap(*heightMap.get());
	heightMapBuilder.SetDestSize(CInt32(destSize->x),CInt32(destSize->y));
	heightMapBuilder.SetBounds(bottomLeft->x,upperRight->x,bottomLeft->y,upperRight->y);
	heightMapBuilder.Build();
	Lua::Push<std::shared_ptr<noise::utils::NoiseMap>>(l,heightMap);
	return 1;
}

///////////////////////////////////////

// Noise Module
void Lua_NoiseModule_GetValue(lua_State *l,NoiseBaseModule &mod,Vector3 &v)
{
	Lua::PushNumber(l,mod->GetValue(v.x,v.y,v.z));
}
void Lua_NoiseModule_SetScale(lua_State*,NoiseBaseModule&,float)
{
	noise::module::ScaleBias s;
	//s.SetSourceModule(0,*mod.get()); // TODO

}
//

// Voronoi
void Lua_VoronoiNoise_GetDisplacement(lua_State *l,NoiseVoronoi &voronoi)
{
	Lua::PushNumber(l,static_cast<noise::module::Voronoi*>(voronoi.get())->GetDisplacement());
}
void Lua_VoronoiNoise_GetFrequency(lua_State *l,NoiseVoronoi &voronoi)
{
	Lua::PushNumber(l,static_cast<noise::module::Voronoi*>(voronoi.get())->GetFrequency());
}
void Lua_VoronoiNoise_GetSeed(lua_State *l,NoiseVoronoi &voronoi)
{
	Lua::PushNumber(l,static_cast<noise::module::Voronoi*>(voronoi.get())->GetSeed());
}
void Lua_VoronoiNoise_SetDisplacement(lua_State*,NoiseVoronoi &voronoi,double displacement)
{
	static_cast<noise::module::Voronoi*>(voronoi.get())->SetDisplacement(displacement);
}
void Lua_VoronoiNoise_SetFrequency(lua_State*,NoiseVoronoi &voronoi,double frequency)
{
	static_cast<noise::module::Voronoi*>(voronoi.get())->SetFrequency(frequency);
}
void Lua_VoronoiNoise_SetSeed(lua_State*,NoiseVoronoi &voronoi,int seed)
{
	static_cast<noise::module::Voronoi*>(voronoi.get())->SetSeed(seed);
}
//

// Perlin
void Lua_PerlinNoise_GetFrequency(lua_State *l,NoisePerlin &perlin)
{
	Lua::PushNumber(l,static_cast<noise::module::Perlin*>(perlin.get())->GetFrequency());
}
void Lua_PerlinNoise_GetLacunarity(lua_State *l,NoisePerlin &perlin)
{
	Lua::PushNumber(l,static_cast<noise::module::Perlin*>(perlin.get())->GetLacunarity());
}
void Lua_PerlinNoise_GetNoiseQuality(lua_State *l,NoisePerlin &perlin)
{
	Lua::PushInt(l,static_cast<noise::module::Perlin*>(perlin.get())->GetNoiseQuality());
}
void Lua_PerlinNoise_GetOctaveCount(lua_State *l,NoisePerlin &perlin)
{
	Lua::PushInt(l,static_cast<noise::module::Perlin*>(perlin.get())->GetOctaveCount());
}
void Lua_PerlinNoise_GetPersistence(lua_State *l,NoisePerlin &perlin)
{
	Lua::PushNumber(l,static_cast<noise::module::Perlin*>(perlin.get())->GetPersistence());
}
void Lua_PerlinNoise_GetSeed(lua_State *l,NoisePerlin &perlin)
{
	Lua::PushInt(l,static_cast<noise::module::Perlin*>(perlin.get())->GetSeed());
}

void Lua_PerlinNoise_SetFrequency(lua_State*,NoisePerlin &perlin,double frequency)
{
	static_cast<noise::module::Perlin*>(perlin.get())->SetFrequency(frequency);
}
void Lua_PerlinNoise_SetLacunarity(lua_State*,NoisePerlin &perlin,double lacunarity)
{
	static_cast<noise::module::Perlin*>(perlin.get())->SetLacunarity(lacunarity);
}
void Lua_PerlinNoise_SetNoiseQuality(lua_State*,NoisePerlin &perlin,int quality)
{
	static_cast<noise::module::Perlin*>(perlin.get())->SetNoiseQuality(noise::NoiseQuality(quality));
}
void Lua_PerlinNoise_SetOctaveCount(lua_State*,NoisePerlin &perlin,int octaveCount)
{
	static_cast<noise::module::Perlin*>(perlin.get())->SetOctaveCount(octaveCount);
}
void Lua_PerlinNoise_SetPersistence(lua_State*,NoisePerlin &perlin,double persistence)
{
	static_cast<noise::module::Perlin*>(perlin.get())->SetPersistence(persistence);
}
void Lua_PerlinNoise_SetSeed(lua_State*,NoisePerlin &perlin,int seed)
{
	static_cast<noise::module::Perlin*>(perlin.get())->SetSeed(seed);
}
//

// Ridged Multi
void Lua_RidgedMultiNoise_GetFrequency(lua_State *l,NoiseRidgedMulti &ridged)
{
	Lua::PushNumber(l,static_cast<noise::module::RidgedMulti*>(ridged.get())->GetFrequency());
}
void Lua_RidgedMultiNoise_GetLacunarity(lua_State *l,NoiseRidgedMulti &ridged)
{
	Lua::PushNumber(l,static_cast<noise::module::RidgedMulti*>(ridged.get())->GetLacunarity());
}
void Lua_RidgedMultiNoise_GetNoiseQuality(lua_State *l,NoiseRidgedMulti &ridged)
{
	Lua::PushInt(l,static_cast<noise::module::RidgedMulti*>(ridged.get())->GetNoiseQuality());
}
void Lua_RidgedMultiNoise_GetOctaveCount(lua_State *l,NoiseRidgedMulti &ridged)
{
	Lua::PushInt(l,static_cast<noise::module::RidgedMulti*>(ridged.get())->GetOctaveCount());
}
void Lua_RidgedMultiNoise_GetSeed(lua_State *l,NoiseRidgedMulti &ridged)
{
	Lua::PushInt(l,static_cast<noise::module::RidgedMulti*>(ridged.get())->GetSeed());
}

void Lua_RidgedMultiNoise_SetFrequency(lua_State*,NoiseRidgedMulti &ridged,double frequency)
{
	static_cast<noise::module::RidgedMulti*>(ridged.get())->SetFrequency(frequency);
}
void Lua_RidgedMultiNoise_SetLacunarity(lua_State*,NoiseRidgedMulti &ridged,double lacunarity)
{
	static_cast<noise::module::RidgedMulti*>(ridged.get())->SetLacunarity(lacunarity);
}
void Lua_RidgedMultiNoise_SetNoiseQuality(lua_State*,NoiseRidgedMulti &ridged,int quality)
{
	static_cast<noise::module::RidgedMulti*>(ridged.get())->SetNoiseQuality(noise::NoiseQuality(quality));
}
void Lua_RidgedMultiNoise_SetOctaveCount(lua_State*,NoiseRidgedMulti &ridged,int octaveCount)
{
	static_cast<noise::module::RidgedMulti*>(ridged.get())->SetOctaveCount(octaveCount);
}
void Lua_RidgedMultiNoise_SetSeed(lua_State*,NoiseRidgedMulti &ridged,int seed)
{
	static_cast<noise::module::RidgedMulti*>(ridged.get())->SetSeed(seed);
}
//

// Billow
void Lua_BillowNoise_GetFrequency(lua_State *l,NoiseBillow &billow)
{
	Lua::PushNumber(l,static_cast<noise::module::Billow*>(billow.get())->GetFrequency());
}
void Lua_BillowNoise_GetLacunarity(lua_State *l,NoiseBillow &billow)
{
	Lua::PushNumber(l,static_cast<noise::module::Billow*>(billow.get())->GetLacunarity());
}
void Lua_BillowNoise_GetNoiseQuality(lua_State *l,NoiseBillow &billow)
{
	Lua::PushInt(l,static_cast<noise::module::Billow*>(billow.get())->GetNoiseQuality());
}
void Lua_BillowNoise_GetOctaveCount(lua_State *l,NoiseBillow &billow)
{
	Lua::PushInt(l,static_cast<noise::module::Billow*>(billow.get())->GetOctaveCount());
}
void Lua_BillowNoise_GetPersistence(lua_State *l,NoiseBillow &billow)
{
	Lua::PushNumber(l,static_cast<noise::module::Billow*>(billow.get())->GetPersistence());
}
void Lua_BillowNoise_GetSeed(lua_State *l,NoiseBillow &billow)
{
	Lua::PushInt(l,static_cast<noise::module::Billow*>(billow.get())->GetSeed());
}
void Lua_BillowNoise_SetFrequency(lua_State*,NoiseBillow &billow,double frequency)
{
	static_cast<noise::module::Billow*>(billow.get())->SetFrequency(frequency);
}
void Lua_BillowNoise_SetLacunarity(lua_State*,NoiseBillow &billow,double lacunarity)
{
	static_cast<noise::module::Billow*>(billow.get())->SetLacunarity(lacunarity);
}
void Lua_BillowNoise_SetNoiseQuality(lua_State*,NoiseBillow &billow,int quality)
{
	static_cast<noise::module::Billow*>(billow.get())->SetNoiseQuality(noise::NoiseQuality(quality));
}
void Lua_BillowNoise_SetOctaveCount(lua_State*,NoiseBillow &billow,int octaveCount)
{
	static_cast<noise::module::Billow*>(billow.get())->SetOctaveCount(octaveCount);
}
void Lua_BillowNoise_SetPersistence(lua_State*,NoiseBillow &billow,double persistence)
{
	static_cast<noise::module::Billow*>(billow.get())->SetPersistence(persistence);
}
void Lua_BillowNoise_SetSeed(lua_State*,NoiseBillow &billow,int seed)
{
	static_cast<noise::module::Billow*>(billow.get())->SetSeed(seed);
}
//

// Noise Map
void Lua_NoiseMap_GetValue(lua_State *l,noise::utils::NoiseMap &noiseMap,int x,int y)
{
	Lua::PushNumber(l,noiseMap.GetValue(x,y));
}
void Lua_NoiseMap_GetHeight(lua_State *l,noise::utils::NoiseMap &noiseMap)
{
	Lua::PushNumber(l,noiseMap.GetHeight());
}
void Lua_NoiseMap_GetWidth(lua_State *l,noise::utils::NoiseMap &noiseMap)
{
	Lua::PushNumber(l,noiseMap.GetWidth());
}
//