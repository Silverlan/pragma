// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:math.intersection;

export import :physics.hitbox;
export import :types;
export import pragma.math;

export {
	namespace pragma::math::intersection {
		struct DLLNETWORK LineMeshResult {
			struct DLLNETWORK Precise {
				std::shared_ptr<asset::ModelMeshGroup> meshGroup;
				std::shared_ptr<pragma::geometry::ModelMesh> mesh;
				std::shared_ptr<pragma::geometry::ModelSubMesh> subMesh;

				uint32_t meshGroupIndex = std::numeric_limits<uint32_t>::max();
				uint32_t meshIdx = std::numeric_limits<uint32_t>::max();
				uint32_t subMeshIdx = std::numeric_limits<uint32_t>::max();

				uint64_t triIdx = std::numeric_limits<uint64_t>::max();
				double t = 0.0;
				double u = 0.0;
				double v = 0.0;
			};
			Result result = Result::NoIntersection;
			Vector3 hitPos = {};
			double hitValue = std::numeric_limits<double>::max(); // Range if hit: [0,1]

			// Only for imprecise results
			physics::Hitbox *hitbox = nullptr;
			uint32_t boneId;

			// Only for precise results
			std::shared_ptr<Precise> precise = nullptr;
		};
		DLLNETWORK bool line_with_mesh(const Vector3 &start, const Vector3 &dir, pragma::geometry::ModelMesh &mesh, LineMeshResult &outResult, bool precise = false, const Vector3 *origin = nullptr, const Quat *rot = nullptr);
		DLLNETWORK bool line_with_mesh(const Vector3 &start, const Vector3 &dir, pragma::geometry::ModelSubMesh &subMesh, LineMeshResult &outResult, bool precise = false, const Vector3 *origin = nullptr, const Quat *rot = nullptr);
		DLLNETWORK bool line_with_mesh(const Vector3 &start, const Vector3 &dir, asset::Model &mdl, LineMeshResult &outResult, bool precise, const std::vector<uint32_t> *bodyGroups, uint32_t lod, const Vector3 &origin, const Quat &rot);
		DLLNETWORK bool line_with_mesh(const Vector3 &start, const Vector3 &dir, asset::Model &mdl, LineMeshResult &outResult, bool precise, uint32_t lod, const Vector3 &origin, const Quat &rot);
		DLLNETWORK bool line_with_mesh(const Vector3 &start, const Vector3 &dir, asset::Model &mdl, LineMeshResult &outResult, bool precise, const std::vector<uint32_t> &bodyGroups, const Vector3 &origin, const Quat &rot);
		DLLNETWORK bool line_with_mesh(const Vector3 &start, const Vector3 &dir, asset::Model &mdl, LineMeshResult &outResult, bool precise, const Vector3 &origin, const Quat &rot);
	};
};
