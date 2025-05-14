/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __COLLISIONS_H__
#define __COLLISIONS_H__

#include "pragma/definitions.h"
#include <mathutil/uvec.h>
#include <mathutil/umath_geometry.hpp>
#include <optional>

struct AABB;
class ModelMeshGroup;
class ModelMesh;
class ModelSubMesh;
class Model;
struct Hitbox;
namespace Intersection {
	struct DLLNETWORK LineMeshResult {
		struct DLLNETWORK Precise {
			std::shared_ptr<ModelMeshGroup> meshGroup;
			std::shared_ptr<ModelMesh> mesh;
			std::shared_ptr<ModelSubMesh> subMesh;

			uint32_t meshGroupIndex = std::numeric_limits<uint32_t>::max();
			uint32_t meshIdx = std::numeric_limits<uint32_t>::max();
			uint32_t subMeshIdx = std::numeric_limits<uint32_t>::max();

			uint64_t triIdx = std::numeric_limits<uint64_t>::max();
			double t = 0.0;
			double u = 0.0;
			double v = 0.0;
		};
		umath::intersection::Result result = umath::intersection::Result::NoIntersection;
		Vector3 hitPos = {};
		double hitValue = std::numeric_limits<double>::max(); // Range if hit: [0,1]

		// Only for imprecise results
		Hitbox *hitbox = nullptr;
		uint32_t boneId;

		// Only for precise results
		std::shared_ptr<Precise> precise = nullptr;
	};
	DLLNETWORK bool LineMesh(const Vector3 &start, const Vector3 &dir, ModelMesh &mesh, LineMeshResult &outResult, bool precise = false, const Vector3 *origin = nullptr, const Quat *rot = nullptr);
	DLLNETWORK bool LineMesh(const Vector3 &start, const Vector3 &dir, ModelSubMesh &subMesh, LineMeshResult &outResult, bool precise = false, const Vector3 *origin = nullptr, const Quat *rot = nullptr);
	DLLNETWORK bool LineMesh(const Vector3 &start, const Vector3 &dir, Model &mdl, LineMeshResult &outResult, bool precise, const std::vector<uint32_t> *bodyGroups, uint32_t lod, const Vector3 &origin, const Quat &rot);
	DLLNETWORK bool LineMesh(const Vector3 &start, const Vector3 &dir, Model &mdl, LineMeshResult &outResult, bool precise, uint32_t lod, const Vector3 &origin, const Quat &rot);
	DLLNETWORK bool LineMesh(const Vector3 &start, const Vector3 &dir, Model &mdl, LineMeshResult &outResult, bool precise, const std::vector<uint32_t> &bodyGroups, const Vector3 &origin, const Quat &rot);
	DLLNETWORK bool LineMesh(const Vector3 &start, const Vector3 &dir, Model &mdl, LineMeshResult &outResult, bool precise, const Vector3 &origin, const Quat &rot);
};

#endif // __COLLISIONS_H__
