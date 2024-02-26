/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan */

#ifndef __INTERSECTION_HANDLER_COMPONENT_HPP__
#define __INTERSECTION_HANDLER_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include <sharedutils/property/util_property_vector.h>

class ModelSubMesh;
namespace pragma {
	struct DLLNETWORK HitInfo {
		std::shared_ptr<ModelSubMesh> mesh;
		EntityHandle entity;
		size_t primitiveIndex;
		float distance;
		float t;
		float u;
		float v;
	};

	struct MeshIntersectionInfo;
	struct DLLNETWORK IntersectionInfo {
		virtual ~IntersectionInfo() = default;
	};

	struct DLLNETWORK MeshIntersectionInfo : public IntersectionInfo {
		struct DLLNETWORK MeshInfo {
			ModelSubMesh *mesh = nullptr;
			BaseEntity *entity = nullptr;
		};
		std::vector<MeshInfo> meshInfos;
	};

	struct MeshIntersectionRange;
	struct DLLNETWORK PrimitiveIntersectionInfo : public IntersectionInfo {
		PrimitiveIntersectionInfo() {}
		std::vector<size_t> primitives;
	};

	class DLLNETWORK IntersectionHandlerComponent final : public BaseEntityComponent {
	  public:
		struct DLLNETWORK IntersectionHandler {
			void *userData = nullptr;
			bool (*intersectionTest)(void *, const Vector3 &, const Vector3 &, float, float, HitInfo &) = [](void *, const Vector3 &, const Vector3 &, float, float, HitInfo &) -> bool { return false; };
			bool (*intersectionTestAabb)(void *, const Vector3 &, const Vector3 &, IntersectionInfo *) = [](void *, const Vector3 &, const Vector3 &, IntersectionInfo *) -> bool { return false; };
			bool (*intersectionTestKDop)(void *, const std::vector<umath::Plane> &, IntersectionInfo *) = [](void *, const std::vector<umath::Plane> &, IntersectionInfo *) -> bool { return false; };
		};
		IntersectionHandlerComponent(BaseEntity &ent);
		virtual void Initialize() override;

		void ClearIntersectionHandler();
		void SetIntersectionHandler(const IntersectionHandler &intersectionHandler);

		std::optional<HitInfo> IntersectionTest(const Vector3 &origin, const Vector3 &dir, umath::CoordinateSpace space, float minDist, float maxDist) const;
		bool IntersectionTest(const Vector3 &origin, const Vector3 &dir, umath::CoordinateSpace space, float minDist, float maxDist, HitInfo &outHitInfo) const;
		virtual bool IntersectionTest(const Vector3 &origin, const Vector3 &dir, float minDist, float maxDist, HitInfo &outHitInfo) const;
		bool IntersectionTestAabb(const Vector3 &min, const Vector3 &max) const;
		bool IntersectionTestAabb(const Vector3 &min, const Vector3 &max, IntersectionInfo &outIntersectionInfo) const;
		bool IntersectionTestKDop(const std::vector<umath::Plane> &planes) const;
		bool IntersectionTestKDop(const std::vector<umath::Plane> &planes, IntersectionInfo &outIntersectionInfo) const;

		virtual void InitializeLuaObject(lua_State *lua) override;
	  protected:
		mutable IntersectionHandler m_intersectionHandler {};
	};
};

#endif
