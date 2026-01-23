// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:physics.raycast;

export import :entities.base_entity_handle;
export import :model.model_mesh;
export import :physics.collision_object;
export import :physics.object_handle;
export import :physics.raycast_filter;
export import :types;

export import pragma.materialsystem;

export namespace pragma::physics {
	class DLLNETWORK TraceData {
	  public:
		TraceData();
		TraceData(const TraceData &other);
		~TraceData();
		RayCastFlags GetFlags() const;
		const math::Transform &GetSource() const;
		const math::Transform &GetTarget() const;
		Vector3 GetSourceOrigin() const;
		Quat GetSourceRotation() const;
		Vector3 GetTargetOrigin() const;
		Quat GetTargetRotation() const;
		Vector3 GetDirection() const;
		float GetDistance() const;
		CollisionMask GetCollisionFilterMask() const;
		CollisionMask GetCollisionFilterGroup() const;
		void SetShape(const IConvexShape &shape);
		const IConvexShape *GetShape() const;
		void SetSource(const Vector3 &origin);
		void SetSourceRotation(const Quat &rot);
		void SetSource(const math::Transform &t);
		void SetTarget(const Vector3 &target);
		void SetTargetRotation(const Quat &rot);
		void SetTarget(const math::Transform &t);
		void SetRotation(const Quat &rot);
		void SetFlags(RayCastFlags flags);
		void SetCollisionFilterMask(CollisionMask mask);
		void SetCollisionFilterGroup(CollisionMask group);
		void SetFilter(const std::shared_ptr<IRayCastFilterCallback> &filter);
		void SetFilter(ecs::BaseEntity &ent);
		void SetFilter(std::vector<EntityHandle> &&ents);
		void SetFilter(PhysObj &phys);
		void SetFilter(ICollisionObject &colObj);
		void SetFilter(const std::function<RayCastHitType(IShape &, IRigidBody &)> &preFilter, const std::function<RayCastHitType(IShape &, IRigidBody &)> &postFilter = nullptr);
		const std::shared_ptr<IRayCastFilterCallback> &GetFilter() const;
		bool HasFlag(RayCastFlags flag) const;
	  protected:
		bool HasTarget() const;
	  private:
		math::Transform m_tStart;
		math::Transform m_tEnd;

		RayCastFlags m_flags;
		bool m_bHasTarget = false;
		CollisionMask m_filterMask;
		CollisionMask m_filterGroup;
		std::shared_ptr<IRayCastFilterCallback> m_filter = nullptr;
		util::WeakHandle<IConvexShape> m_shape = {};
	};

	struct DLLNETWORK TraceResult {
		struct DLLNETWORK MeshInfo {
			std::vector<std::shared_ptr<geometry::ModelMesh>> meshes;
			geometry::ModelMesh *mesh = nullptr;
			geometry::ModelSubMesh *subMesh = nullptr;
		};
		TraceResult() {}
		TraceResult(const TraceData &data);
		~TraceResult();
		RayCastHitType hitType = RayCastHitType::None;
		EntityHandle entity = {};
		util::TWeakSharedHandle<ICollisionObject> collisionObj = {};
		std::weak_ptr<IShape> shape = {};
		PhysObjHandle physObj = {};
		float fraction = 0.f;

		float distance = 0.f;
		Vector3 normal = {};
		Vector3 position = {};
		Vector3 startPosition = Vector3 {};

		std::shared_ptr<MeshInfo> meshInfo = nullptr;

		void GetMeshes(geometry::ModelMesh **mesh, geometry::ModelSubMesh **subMesh);
		material::Material *GetMaterial();
		bool GetMaterial(std::string &mat);
	  private:
		void InitializeMeshes();
	};
};
