// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "memory"


#include "noise/noise.h"
#include "noiseutils.h"

module pragma.shared;

import :scripting.lua.libraries.noise;

NoiseBaseModule::NoiseBaseModule(noise::module::Module *mod) : std::shared_ptr<noise::module::Module>(mod) {}

int Lua::noise::perlin(lua_State *l)
{
	Lua::Push<NoisePerlin>(l, NoisePerlin(new ::noise::module::Perlin));
	return 1;
}

int Lua::noise::noise_const(lua_State *l)
{
	Lua::Push<NoiseConst>(l, NoiseConst(new ::noise::module::Const));
	return 1;
}

int Lua::noise::voronoi(lua_State *l)
{
	Lua::Push<NoiseVoronoi>(l, NoiseVoronoi(new ::noise::module::Voronoi));
	return 1;
}

int Lua::noise::ridged_multi(lua_State *l)
{
	Lua::Push<NoiseRidgedMulti>(l, NoiseRidgedMulti(new ::noise::module::RidgedMulti));
	return 1;
}

int Lua::noise::billow(lua_State *l)
{
	Lua::Push<NoiseBillow>(l, NoiseBillow(new ::noise::module::Billow));
	return 1;
}

int Lua::noise::generate_height_map(lua_State *l)
{
	auto &noiseModule = Lua::Check<::noise::module::Module>(l, 1);
	auto &destSize = Lua::Check<::Vector2>(l, 2);
	auto &bottomLeft = Lua::Check<::Vector2>(l, 3);
	auto &upperRight = Lua::Check<::Vector2>(l, 4);
	auto heightMap = std::make_shared<::noise::utils::NoiseMap>();
	::noise::utils::NoiseMapBuilderPlane heightMapBuilder;
	heightMapBuilder.SetSourceModule(noiseModule);
	heightMapBuilder.SetDestNoiseMap(*heightMap.get());
	heightMapBuilder.SetDestSize(CInt32(destSize.x), CInt32(destSize.y));
	heightMapBuilder.SetBounds(bottomLeft.x, upperRight.x, bottomLeft.y, upperRight.y);
	heightMapBuilder.Build();
	Lua::Push<std::shared_ptr<::noise::utils::NoiseMap>>(l, heightMap);
	return 1;
}

///////////////////////////////////////

// Noise Module
void Lua::noise::NoiseModule::GetValue(lua_State *l, NoiseBaseModule &mod, Vector3 &v) { Lua::PushNumber(l, mod->GetValue(v.x, v.y, v.z)); }
void Lua::noise::NoiseModule::SetScale(lua_State *, NoiseBaseModule &, float)
{
	::noise::module::ScaleBias s;
	//s.SetSourceModule(0,*mod.get()); // TODO
}
//

// Voronoi
void Lua::noise::VoronoiNoise::GetDisplacement(lua_State *l, NoiseVoronoi &voronoi) { Lua::PushNumber(l, static_cast<::noise::module::Voronoi *>(voronoi.get())->GetDisplacement()); }
void Lua::noise::VoronoiNoise::GetFrequency(lua_State *l, NoiseVoronoi &voronoi) { Lua::PushNumber(l, static_cast<::noise::module::Voronoi *>(voronoi.get())->GetFrequency()); }
void Lua::noise::VoronoiNoise::GetSeed(lua_State *l, NoiseVoronoi &voronoi) { Lua::PushNumber(l, static_cast<::noise::module::Voronoi *>(voronoi.get())->GetSeed()); }
void Lua::noise::VoronoiNoise::SetDisplacement(lua_State *, NoiseVoronoi &voronoi, double displacement) { static_cast<::noise::module::Voronoi *>(voronoi.get())->SetDisplacement(displacement); }
void Lua::noise::VoronoiNoise::SetFrequency(lua_State *, NoiseVoronoi &voronoi, double frequency) { static_cast<::noise::module::Voronoi *>(voronoi.get())->SetFrequency(frequency); }
void Lua::noise::VoronoiNoise::SetSeed(lua_State *, NoiseVoronoi &voronoi, int seed) { static_cast<::noise::module::Voronoi *>(voronoi.get())->SetSeed(seed); }
//

// Perlin
void Lua::noise::PerlinNoise::GetFrequency(lua_State *l, NoisePerlin &perlin) { Lua::PushNumber(l, static_cast<::noise::module::Perlin *>(perlin.get())->GetFrequency()); }
void Lua::noise::PerlinNoise::GetLacunarity(lua_State *l, NoisePerlin &perlin) { Lua::PushNumber(l, static_cast<::noise::module::Perlin *>(perlin.get())->GetLacunarity()); }
void Lua::noise::PerlinNoise::GetNoiseQuality(lua_State *l, NoisePerlin &perlin) { Lua::PushInt(l, static_cast<::noise::module::Perlin *>(perlin.get())->GetNoiseQuality()); }
void Lua::noise::PerlinNoise::GetOctaveCount(lua_State *l, NoisePerlin &perlin) { Lua::PushInt(l, static_cast<::noise::module::Perlin *>(perlin.get())->GetOctaveCount()); }
void Lua::noise::PerlinNoise::GetPersistence(lua_State *l, NoisePerlin &perlin) { Lua::PushNumber(l, static_cast<::noise::module::Perlin *>(perlin.get())->GetPersistence()); }
void Lua::noise::PerlinNoise::GetSeed(lua_State *l, NoisePerlin &perlin) { Lua::PushInt(l, static_cast<::noise::module::Perlin *>(perlin.get())->GetSeed()); }

void Lua::noise::PerlinNoise::SetFrequency(lua_State *, NoisePerlin &perlin, double frequency) { static_cast<::noise::module::Perlin *>(perlin.get())->SetFrequency(frequency); }
void Lua::noise::PerlinNoise::SetLacunarity(lua_State *, NoisePerlin &perlin, double lacunarity) { static_cast<::noise::module::Perlin *>(perlin.get())->SetLacunarity(lacunarity); }
void Lua::noise::PerlinNoise::SetNoiseQuality(lua_State *, NoisePerlin &perlin, int quality) { static_cast<::noise::module::Perlin *>(perlin.get())->SetNoiseQuality(::noise::NoiseQuality(quality)); }
void Lua::noise::PerlinNoise::SetOctaveCount(lua_State *, NoisePerlin &perlin, int octaveCount) { static_cast<::noise::module::Perlin *>(perlin.get())->SetOctaveCount(octaveCount); }
void Lua::noise::PerlinNoise::SetPersistence(lua_State *, NoisePerlin &perlin, double persistence) { static_cast<::noise::module::Perlin *>(perlin.get())->SetPersistence(persistence); }
void Lua::noise::PerlinNoise::SetSeed(lua_State *, NoisePerlin &perlin, int seed) { static_cast<::noise::module::Perlin *>(perlin.get())->SetSeed(seed); }
//

// Ridged Multi
void Lua::noise::RidgedMultiNoise::GetFrequency(lua_State *l, NoiseRidgedMulti &ridged) { Lua::PushNumber(l, static_cast<::noise::module::RidgedMulti *>(ridged.get())->GetFrequency()); }
void Lua::noise::RidgedMultiNoise::GetLacunarity(lua_State *l, NoiseRidgedMulti &ridged) { Lua::PushNumber(l, static_cast<::noise::module::RidgedMulti *>(ridged.get())->GetLacunarity()); }
void Lua::noise::RidgedMultiNoise::GetNoiseQuality(lua_State *l, NoiseRidgedMulti &ridged) { Lua::PushInt(l, static_cast<::noise::module::RidgedMulti *>(ridged.get())->GetNoiseQuality()); }
void Lua::noise::RidgedMultiNoise::GetOctaveCount(lua_State *l, NoiseRidgedMulti &ridged) { Lua::PushInt(l, static_cast<::noise::module::RidgedMulti *>(ridged.get())->GetOctaveCount()); }
void Lua::noise::RidgedMultiNoise::GetSeed(lua_State *l, NoiseRidgedMulti &ridged) { Lua::PushInt(l, static_cast<::noise::module::RidgedMulti *>(ridged.get())->GetSeed()); }

void Lua::noise::RidgedMultiNoise::SetFrequency(lua_State *, NoiseRidgedMulti &ridged, double frequency) { static_cast<::noise::module::RidgedMulti *>(ridged.get())->SetFrequency(frequency); }
void Lua::noise::RidgedMultiNoise::SetLacunarity(lua_State *, NoiseRidgedMulti &ridged, double lacunarity) { static_cast<::noise::module::RidgedMulti *>(ridged.get())->SetLacunarity(lacunarity); }
void Lua::noise::RidgedMultiNoise::SetNoiseQuality(lua_State *, NoiseRidgedMulti &ridged, int quality) { static_cast<::noise::module::RidgedMulti *>(ridged.get())->SetNoiseQuality(::noise::NoiseQuality(quality)); }
void Lua::noise::RidgedMultiNoise::SetOctaveCount(lua_State *, NoiseRidgedMulti &ridged, int octaveCount) { static_cast<::noise::module::RidgedMulti *>(ridged.get())->SetOctaveCount(octaveCount); }
void Lua::noise::RidgedMultiNoise::SetSeed(lua_State *, NoiseRidgedMulti &ridged, int seed) { static_cast<::noise::module::RidgedMulti *>(ridged.get())->SetSeed(seed); }
//

// Billow
void Lua::noise::BillowNoise::GetFrequency(lua_State *l, NoiseBillow &billow) { Lua::PushNumber(l, static_cast<::noise::module::Billow *>(billow.get())->GetFrequency()); }
void Lua::noise::BillowNoise::GetLacunarity(lua_State *l, NoiseBillow &billow) { Lua::PushNumber(l, static_cast<::noise::module::Billow *>(billow.get())->GetLacunarity()); }
void Lua::noise::BillowNoise::GetNoiseQuality(lua_State *l, NoiseBillow &billow) { Lua::PushInt(l, static_cast<::noise::module::Billow *>(billow.get())->GetNoiseQuality()); }
void Lua::noise::BillowNoise::GetOctaveCount(lua_State *l, NoiseBillow &billow) { Lua::PushInt(l, static_cast<::noise::module::Billow *>(billow.get())->GetOctaveCount()); }
void Lua::noise::BillowNoise::GetPersistence(lua_State *l, NoiseBillow &billow) { Lua::PushNumber(l, static_cast<::noise::module::Billow *>(billow.get())->GetPersistence()); }
void Lua::noise::BillowNoise::GetSeed(lua_State *l, NoiseBillow &billow) { Lua::PushInt(l, static_cast<::noise::module::Billow *>(billow.get())->GetSeed()); }
void Lua::noise::BillowNoise::SetFrequency(lua_State *, NoiseBillow &billow, double frequency) { static_cast<::noise::module::Billow *>(billow.get())->SetFrequency(frequency); }
void Lua::noise::BillowNoise::SetLacunarity(lua_State *, NoiseBillow &billow, double lacunarity) { static_cast<::noise::module::Billow *>(billow.get())->SetLacunarity(lacunarity); }
void Lua::noise::BillowNoise::SetNoiseQuality(lua_State *, NoiseBillow &billow, int quality) { static_cast<::noise::module::Billow *>(billow.get())->SetNoiseQuality(::noise::NoiseQuality(quality)); }
void Lua::noise::BillowNoise::SetOctaveCount(lua_State *, NoiseBillow &billow, int octaveCount) { static_cast<::noise::module::Billow *>(billow.get())->SetOctaveCount(octaveCount); }
void Lua::noise::BillowNoise::SetPersistence(lua_State *, NoiseBillow &billow, double persistence) { static_cast<::noise::module::Billow *>(billow.get())->SetPersistence(persistence); }
void Lua::noise::BillowNoise::SetSeed(lua_State *, NoiseBillow &billow, int seed) { static_cast<::noise::module::Billow *>(billow.get())->SetSeed(seed); }
//

// Noise Map
void Lua::noise::NoiseMap::GetValue(lua_State *l, ::noise::utils::NoiseMap &noiseMap, int x, int y) { Lua::PushNumber(l, noiseMap.GetValue(x, y)); }
void Lua::noise::NoiseMap::GetHeight(lua_State *l, ::noise::utils::NoiseMap &noiseMap) { Lua::PushNumber(l, noiseMap.GetHeight()); }
void Lua::noise::NoiseMap::GetWidth(lua_State *l, ::noise::utils::NoiseMap &noiseMap) { Lua::PushNumber(l, noiseMap.GetWidth()); }
//
