// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:map.map_geometry;

export import :physics.surface_material;
export import pragma.materialsystem;

export {
	namespace pragma {
		class Game;
	}
	namespace pragma::ecs {
		class BaseEntity;
	}
	DLLNETWORK void BuildDisplacementTriangles(std::vector<Vector3> &sideVerts, unsigned int start, Vector3 &nu, Vector3 &nv, float sw, float sh, float ou, float ov, float su, float sv, unsigned char power, std::vector<std::vector<Vector3>> &normals,
	  std::vector<std::vector<Vector3>> &offsets, std::vector<std::vector<float>> &distances, unsigned char numAlpha, std::vector<std::vector<Vector2>> &alphas, std::vector<Vector3> &outVertices, std::vector<Vector3> &outNormals, std::vector<Vector2> &outUvs,
	  std::vector<unsigned int> &outTriangles, std::vector<Vector2> *outAlphas = nullptr);

	DLLNETWORK void ToTriangles(const std::vector<Vector3> &vertices, std::vector<uint16_t> &outTriangles);
	DLLNETWORK bool decimate_displacement_geometry(const std::vector<uint16_t> &indices, uint32_t power, std::vector<uint16_t> &outIndices, uint8_t decimateAmount = 2);

	namespace pragma {
		namespace level {
			DLLNETWORK void load_map_brushes(Game &game, uint32_t version,fs::VFilePtr f, ecs::BaseEntity *ent, std::vector<material::Material *> &materials, std::vector<physics::SurfaceMaterial> &surfaceMaterials, const Vector3 &origin);
			DLLNETWORK void load_optimized_map_geometry(Game &game, uint32_t version,fs::VFilePtr f, ecs::BaseEntity *ent, std::vector<material::Material *> &materials, std::vector<physics::SurfaceMaterial> &surfaceMaterials);
		};
	};
};
