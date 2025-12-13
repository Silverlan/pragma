// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"
#include "noise/noise.h"
#include "noiseutils.h"

export module pragma.shared:scripting.lua.libraries.noise;

export import pragma.lua;

#define DECLARE_NOISE_MODULE(name)                                                                                                                                                                                                                                                               \
	class DLLNETWORK Noise##name : public NoiseBaseModule {                                                                                                                                                                                                                                      \
	  public:                                                                                                                                                                                                                                                                                    \
		using NoiseBaseModule::NoiseBaseModule;                                                                                                                                                                                                                                                  \
	};

export {
	class DLLNETWORK NoiseBaseModule : public std::shared_ptr<module::Module> {
	  public:
		NoiseBaseModule(module::Module *mod);
	};

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

	namespace Lua::noise {
		DLLNETWORK int perlin(lua::State *l);
		DLLNETWORK int noise_const(lua::State *l);
		DLLNETWORK int voronoi(lua::State *l);
		DLLNETWORK int ridged_multi(lua::State *l);
		DLLNETWORK int billow(lua::State *l);
		DLLNETWORK int generate_height_map(lua::State *l);
	};

	///////////////////////////////////////

	namespace Lua::noise {
		namespace NoiseModule {
			DLLNETWORK void GetValue(lua::State *l, NoiseBaseModule &mod, Vector3 &v);
			DLLNETWORK void SetScale(lua::State *l, NoiseBaseModule &mod, float scale);
		};

		namespace VoronoiNoise {
			DLLNETWORK void GetDisplacement(lua::State *l, NoiseVoronoi &voronoi);
			DLLNETWORK void GetFrequency(lua::State *l, NoiseVoronoi &voronoi);
			DLLNETWORK void GetSeed(lua::State *l, NoiseVoronoi &voronoi);
			DLLNETWORK void SetDisplacement(lua::State *l, NoiseVoronoi &voronoi, double displacement);
			DLLNETWORK void SetFrequency(lua::State *l, NoiseVoronoi &voronoi, double frequency);
			DLLNETWORK void SetSeed(lua::State *l, NoiseVoronoi &voronoi, int seed);
		};

		namespace PerlinNoise {
			DLLNETWORK void GetFrequency(lua::State *l, NoisePerlin &perlin);
			DLLNETWORK void GetLacunarity(lua::State *l, NoisePerlin &perlin);
			DLLNETWORK void GetNoiseQuality(lua::State *l, NoisePerlin &perlin);
			DLLNETWORK void GetOctaveCount(lua::State *l, NoisePerlin &perlin);
			DLLNETWORK void GetPersistence(lua::State *l, NoisePerlin &perlin);
			DLLNETWORK void GetSeed(lua::State *l, NoisePerlin &perlin);
			DLLNETWORK void SetFrequency(lua::State *l, NoisePerlin &perlin, double frequency);
			DLLNETWORK void SetLacunarity(lua::State *l, NoisePerlin &perlin, double lacunarity);
			DLLNETWORK void SetNoiseQuality(lua::State *l, NoisePerlin &perlin, int quality);
			DLLNETWORK void SetOctaveCount(lua::State *l, NoisePerlin &perlin, int octaveCount);
			DLLNETWORK void SetPersistence(lua::State *l, NoisePerlin &perlin, double persistence);
			DLLNETWORK void SetSeed(lua::State *l, NoisePerlin &perlin, int seed);
		};

		namespace RidgedMultiNoise {
			DLLNETWORK void GetFrequency(lua::State *l, NoiseRidgedMulti &ridged);
			DLLNETWORK void GetLacunarity(lua::State *l, NoiseRidgedMulti &ridged);
			DLLNETWORK void GetNoiseQuality(lua::State *l, NoiseRidgedMulti &ridged);
			DLLNETWORK void GetOctaveCount(lua::State *l, NoiseRidgedMulti &ridged);
			DLLNETWORK void GetSeed(lua::State *l, NoiseRidgedMulti &ridged);
			DLLNETWORK void SetFrequency(lua::State *l, NoiseRidgedMulti &ridged, double frequency);
			DLLNETWORK void SetLacunarity(lua::State *l, NoiseRidgedMulti &ridged, double lacunarity);
			DLLNETWORK void SetNoiseQuality(lua::State *l, NoiseRidgedMulti &ridged, int quality);
			DLLNETWORK void SetOctaveCount(lua::State *l, NoiseRidgedMulti &ridged, int octaveCount);
			DLLNETWORK void SetSeed(lua::State *l, NoiseRidgedMulti &ridged, int seed);
		};

		namespace BillowNoise {
			DLLNETWORK void GetFrequency(lua::State *l, NoiseBillow &billow);
			DLLNETWORK void GetLacunarity(lua::State *l, NoiseBillow &billow);
			DLLNETWORK void GetNoiseQuality(lua::State *l, NoiseBillow &billow);
			DLLNETWORK void GetOctaveCount(lua::State *l, NoiseBillow &billow);
			DLLNETWORK void GetPersistence(lua::State *l, NoiseBillow &billow);
			DLLNETWORK void GetSeed(lua::State *l, NoiseBillow &billow);
			DLLNETWORK void SetFrequency(lua::State *l, NoiseBillow &billow, double frequency);
			DLLNETWORK void SetLacunarity(lua::State *l, NoiseBillow &billow, double lacunarity);
			DLLNETWORK void SetNoiseQuality(lua::State *l, NoiseBillow &billow, int quality);
			DLLNETWORK void SetOctaveCount(lua::State *l, NoiseBillow &billow, int octaveCount);
			DLLNETWORK void SetPersistence(lua::State *l, NoiseBillow &billow, double persistence);
			DLLNETWORK void SetSeed(lua::State *l, NoiseBillow &billow, int seed);
		};

		namespace NoiseMap {
			DLLNETWORK void GetValue(lua::State *l, utils::NoiseMap &noiseMap, int x, int y);
			DLLNETWORK void GetHeight(lua::State *l, utils::NoiseMap &noiseMap);
			DLLNETWORK void GetWidth(lua::State *l, utils::NoiseMap &noiseMap);
		};
	};
};
