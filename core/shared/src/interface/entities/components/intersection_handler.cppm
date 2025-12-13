// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.intersection_handler;

export import :entities.components.base;

export {
	namespace pragma {
		namespace geometry {
			class ModelSubMesh;
		}
		struct DLLNETWORK HitInfo {
			std::shared_ptr<geometry::ModelSubMesh> mesh;
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
				geometry::ModelSubMesh *mesh = nullptr;
				ecs::BaseEntity *entity = nullptr;
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
				bool (*intersectionTestKDop)(void *, const std::vector<math::Plane> &, IntersectionInfo *) = [](void *, const std::vector<math::Plane> &, IntersectionInfo *) -> bool { return false; };
			};
			IntersectionHandlerComponent(ecs::BaseEntity &ent);
			virtual void Initialize() override;

			void ClearIntersectionHandler();
			void SetIntersectionHandler(const IntersectionHandler &intersectionHandler);

			std::optional<HitInfo> IntersectionTest(const Vector3 &origin, const Vector3 &dir, math::CoordinateSpace space, float minDist, float maxDist) const;
			bool IntersectionTest(const Vector3 &origin, const Vector3 &dir, math::CoordinateSpace space, float minDist, float maxDist, HitInfo &outHitInfo) const;
			virtual bool IntersectionTest(const Vector3 &origin, const Vector3 &dir, float minDist, float maxDist, HitInfo &outHitInfo) const;
			bool IntersectionTestAabb(const Vector3 &min, const Vector3 &max) const;
			bool IntersectionTestAabb(const Vector3 &min, const Vector3 &max, IntersectionInfo &outIntersectionInfo) const;
			bool IntersectionTestKDop(const std::vector<math::Plane> &planes) const;
			bool IntersectionTestKDop(const std::vector<math::Plane> &planes, IntersectionInfo &outIntersectionInfo) const;

			virtual void InitializeLuaObject(lua::State *lua) override;
		  protected:
			mutable IntersectionHandler m_intersectionHandler {};
		};
	};
}
