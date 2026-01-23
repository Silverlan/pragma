// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "noise/noise.h"
#include "noiseutils.h"

module pragma.shared;

import :scripting.lua.libraries.noise;

NoiseBaseModule::NoiseBaseModule(module::Module *mod) : std::shared_ptr<module::Module>(mod) {}

int Lua::noise::perlin(lua::State *l)
{
	Lua::Push<NoisePerlin>(l, NoisePerlin(new module::Perlin));
	return 1;
}

int Lua::noise::noise_const(lua::State *l)
{
	Lua::Push<NoiseConst>(l, NoiseConst(new module::Const));
	return 1;
}

int Lua::noise::voronoi(lua::State *l)
{
	Lua::Push<NoiseVoronoi>(l, NoiseVoronoi(new module::Voronoi));
	return 1;
}

int Lua::noise::ridged_multi(lua::State *l)
{
	Lua::Push<NoiseRidgedMulti>(l, NoiseRidgedMulti(new module::RidgedMulti));
	return 1;
}

int Lua::noise::billow(lua::State *l)
{
	Lua::Push<NoiseBillow>(l, NoiseBillow(new module::Billow));
	return 1;
}

int Lua::noise::generate_height_map(lua::State *l)
{
	auto &noiseModule = Lua::Check<module::Module>(l, 1);
	auto &destSize = Lua::Check<::Vector2>(l, 2);
	auto &bottomLeft = Lua::Check<::Vector2>(l, 3);
	auto &upperRight = Lua::Check<::Vector2>(l, 4);
	auto heightMap = pragma::util::make_shared<utils::NoiseMap>();
	utils::NoiseMapBuilderPlane heightMapBuilder;
	heightMapBuilder.SetSourceModule(noiseModule);
	heightMapBuilder.SetDestNoiseMap(*heightMap.get());
	heightMapBuilder.SetDestSize(CInt32(destSize.x), CInt32(destSize.y));
	heightMapBuilder.SetBounds(bottomLeft.x, upperRight.x, bottomLeft.y, upperRight.y);
	heightMapBuilder.Build();
	Lua::Push<std::shared_ptr<utils::NoiseMap>>(l, heightMap);
	return 1;
}

///////////////////////////////////////

// Noise Module
void Lua::noise::NoiseModule::GetValue(lua::State *l, NoiseBaseModule &mod, Vector3 &v) { PushNumber(l, mod->GetValue(v.x, v.y, v.z)); }
void Lua::noise::NoiseModule::SetScale(lua::State *, NoiseBaseModule &, float)
{
	module::ScaleBias s;
	//s.SetSourceModule(0,*mod.get()); // TODO
}
//

// Voronoi
void Lua::noise::VoronoiNoise::GetDisplacement(lua::State *l, NoiseVoronoi &voronoi) { PushNumber(l, static_cast<module::Voronoi *>(voronoi.get())->GetDisplacement()); }
void Lua::noise::VoronoiNoise::GetFrequency(lua::State *l, NoiseVoronoi &voronoi) { PushNumber(l, static_cast<module::Voronoi *>(voronoi.get())->GetFrequency()); }
void Lua::noise::VoronoiNoise::GetSeed(lua::State *l, NoiseVoronoi &voronoi) { PushNumber(l, static_cast<module::Voronoi *>(voronoi.get())->GetSeed()); }
void Lua::noise::VoronoiNoise::SetDisplacement(lua::State *, NoiseVoronoi &voronoi, double displacement) { static_cast<module::Voronoi *>(voronoi.get())->SetDisplacement(displacement); }
void Lua::noise::VoronoiNoise::SetFrequency(lua::State *, NoiseVoronoi &voronoi, double frequency) { static_cast<module::Voronoi *>(voronoi.get())->SetFrequency(frequency); }
void Lua::noise::VoronoiNoise::SetSeed(lua::State *, NoiseVoronoi &voronoi, int seed) { static_cast<module::Voronoi *>(voronoi.get())->SetSeed(seed); }
//

// Perlin
void Lua::noise::PerlinNoise::GetFrequency(lua::State *l, NoisePerlin &perlin) { PushNumber(l, static_cast<module::Perlin *>(perlin.get())->GetFrequency()); }
void Lua::noise::PerlinNoise::GetLacunarity(lua::State *l, NoisePerlin &perlin) { PushNumber(l, static_cast<module::Perlin *>(perlin.get())->GetLacunarity()); }
void Lua::noise::PerlinNoise::GetNoiseQuality(lua::State *l, NoisePerlin &perlin) { PushInt(l, static_cast<module::Perlin *>(perlin.get())->GetNoiseQuality()); }
void Lua::noise::PerlinNoise::GetOctaveCount(lua::State *l, NoisePerlin &perlin) { PushInt(l, static_cast<module::Perlin *>(perlin.get())->GetOctaveCount()); }
void Lua::noise::PerlinNoise::GetPersistence(lua::State *l, NoisePerlin &perlin) { PushNumber(l, static_cast<module::Perlin *>(perlin.get())->GetPersistence()); }
void Lua::noise::PerlinNoise::GetSeed(lua::State *l, NoisePerlin &perlin) { PushInt(l, static_cast<module::Perlin *>(perlin.get())->GetSeed()); }

void Lua::noise::PerlinNoise::SetFrequency(lua::State *, NoisePerlin &perlin, double frequency) { static_cast<module::Perlin *>(perlin.get())->SetFrequency(frequency); }
void Lua::noise::PerlinNoise::SetLacunarity(lua::State *, NoisePerlin &perlin, double lacunarity) { static_cast<module::Perlin *>(perlin.get())->SetLacunarity(lacunarity); }
void Lua::noise::PerlinNoise::SetNoiseQuality(lua::State *, NoisePerlin &perlin, int quality) { static_cast<module::Perlin *>(perlin.get())->SetNoiseQuality(NoiseQuality(quality)); }
void Lua::noise::PerlinNoise::SetOctaveCount(lua::State *, NoisePerlin &perlin, int octaveCount) { static_cast<module::Perlin *>(perlin.get())->SetOctaveCount(octaveCount); }
void Lua::noise::PerlinNoise::SetPersistence(lua::State *, NoisePerlin &perlin, double persistence) { static_cast<module::Perlin *>(perlin.get())->SetPersistence(persistence); }
void Lua::noise::PerlinNoise::SetSeed(lua::State *, NoisePerlin &perlin, int seed) { static_cast<module::Perlin *>(perlin.get())->SetSeed(seed); }
//

// Ridged Multi
void Lua::noise::RidgedMultiNoise::GetFrequency(lua::State *l, NoiseRidgedMulti &ridged) { PushNumber(l, static_cast<module::RidgedMulti *>(ridged.get())->GetFrequency()); }
void Lua::noise::RidgedMultiNoise::GetLacunarity(lua::State *l, NoiseRidgedMulti &ridged) { PushNumber(l, static_cast<module::RidgedMulti *>(ridged.get())->GetLacunarity()); }
void Lua::noise::RidgedMultiNoise::GetNoiseQuality(lua::State *l, NoiseRidgedMulti &ridged) { PushInt(l, static_cast<module::RidgedMulti *>(ridged.get())->GetNoiseQuality()); }
void Lua::noise::RidgedMultiNoise::GetOctaveCount(lua::State *l, NoiseRidgedMulti &ridged) { PushInt(l, static_cast<module::RidgedMulti *>(ridged.get())->GetOctaveCount()); }
void Lua::noise::RidgedMultiNoise::GetSeed(lua::State *l, NoiseRidgedMulti &ridged) { PushInt(l, static_cast<module::RidgedMulti *>(ridged.get())->GetSeed()); }

void Lua::noise::RidgedMultiNoise::SetFrequency(lua::State *, NoiseRidgedMulti &ridged, double frequency) { static_cast<module::RidgedMulti *>(ridged.get())->SetFrequency(frequency); }
void Lua::noise::RidgedMultiNoise::SetLacunarity(lua::State *, NoiseRidgedMulti &ridged, double lacunarity) { static_cast<module::RidgedMulti *>(ridged.get())->SetLacunarity(lacunarity); }
void Lua::noise::RidgedMultiNoise::SetNoiseQuality(lua::State *, NoiseRidgedMulti &ridged, int quality) { static_cast<module::RidgedMulti *>(ridged.get())->SetNoiseQuality(NoiseQuality(quality)); }
void Lua::noise::RidgedMultiNoise::SetOctaveCount(lua::State *, NoiseRidgedMulti &ridged, int octaveCount) { static_cast<module::RidgedMulti *>(ridged.get())->SetOctaveCount(octaveCount); }
void Lua::noise::RidgedMultiNoise::SetSeed(lua::State *, NoiseRidgedMulti &ridged, int seed) { static_cast<module::RidgedMulti *>(ridged.get())->SetSeed(seed); }
//

// Billow
void Lua::noise::BillowNoise::GetFrequency(lua::State *l, NoiseBillow &billow) { PushNumber(l, static_cast<module::Billow *>(billow.get())->GetFrequency()); }
void Lua::noise::BillowNoise::GetLacunarity(lua::State *l, NoiseBillow &billow) { PushNumber(l, static_cast<module::Billow *>(billow.get())->GetLacunarity()); }
void Lua::noise::BillowNoise::GetNoiseQuality(lua::State *l, NoiseBillow &billow) { PushInt(l, static_cast<module::Billow *>(billow.get())->GetNoiseQuality()); }
void Lua::noise::BillowNoise::GetOctaveCount(lua::State *l, NoiseBillow &billow) { PushInt(l, static_cast<module::Billow *>(billow.get())->GetOctaveCount()); }
void Lua::noise::BillowNoise::GetPersistence(lua::State *l, NoiseBillow &billow) { PushNumber(l, static_cast<module::Billow *>(billow.get())->GetPersistence()); }
void Lua::noise::BillowNoise::GetSeed(lua::State *l, NoiseBillow &billow) { PushInt(l, static_cast<module::Billow *>(billow.get())->GetSeed()); }
void Lua::noise::BillowNoise::SetFrequency(lua::State *, NoiseBillow &billow, double frequency) { static_cast<module::Billow *>(billow.get())->SetFrequency(frequency); }
void Lua::noise::BillowNoise::SetLacunarity(lua::State *, NoiseBillow &billow, double lacunarity) { static_cast<module::Billow *>(billow.get())->SetLacunarity(lacunarity); }
void Lua::noise::BillowNoise::SetNoiseQuality(lua::State *, NoiseBillow &billow, int quality) { static_cast<module::Billow *>(billow.get())->SetNoiseQuality(NoiseQuality(quality)); }
void Lua::noise::BillowNoise::SetOctaveCount(lua::State *, NoiseBillow &billow, int octaveCount) { static_cast<module::Billow *>(billow.get())->SetOctaveCount(octaveCount); }
void Lua::noise::BillowNoise::SetPersistence(lua::State *, NoiseBillow &billow, double persistence) { static_cast<module::Billow *>(billow.get())->SetPersistence(persistence); }
void Lua::noise::BillowNoise::SetSeed(lua::State *, NoiseBillow &billow, int seed) { static_cast<module::Billow *>(billow.get())->SetSeed(seed); }
//

// Noise Map
void Lua::noise::NoiseMap::GetValue(lua::State *l, utils::NoiseMap &noiseMap, int x, int y) { PushNumber(l, noiseMap.GetValue(x, y)); }
void Lua::noise::NoiseMap::GetHeight(lua::State *l, utils::NoiseMap &noiseMap) { PushNumber(l, noiseMap.GetHeight()); }
void Lua::noise::NoiseMap::GetWidth(lua::State *l, utils::NoiseMap &noiseMap) { PushNumber(l, noiseMap.GetWidth()); }
//
