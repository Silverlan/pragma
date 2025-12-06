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
		pragma::physics::RayCastFlags GetFlags() const;
		const umath::Transform &GetSource() const;
		const umath::Transform &GetTarget() const;
		Vector3 GetSourceOrigin() const;
		Quat GetSourceRotation() const;
		Vector3 GetTargetOrigin() const;
		Quat GetTargetRotation() const;
		Vector3 GetDirection() const;
		float GetDistance() const;
		pragma::physics::CollisionMask GetCollisionFilterMask() const;
		pragma::physics::CollisionMask GetCollisionFilterGroup() const;
		void SetShape(const pragma::physics::IConvexShape &shape);
		const pragma::physics::IConvexShape *GetShape() const;
		void SetSource(const Vector3 &origin);
		void SetSourceRotation(const Quat &rot);
		void SetSource(const umath::Transform &t);
		void SetTarget(const Vector3 &target);
		void SetTargetRotation(const Quat &rot);
		void SetTarget(const umath::Transform &t);
		void SetRotation(const Quat &rot);
		void SetFlags(pragma::physics::RayCastFlags flags);
		void SetCollisionFilterMask(pragma::physics::CollisionMask mask);
		void SetCollisionFilterGroup(pragma::physics::CollisionMask group);
		void SetFilter(const std::shared_ptr<pragma::physics::IRayCastFilterCallback> &filter);
		void SetFilter(pragma::ecs::BaseEntity &ent);
		void SetFilter(std::vector<EntityHandle> &&ents);
		void SetFilter(pragma::physics::PhysObj &phys);
		void SetFilter(pragma::physics::ICollisionObject &colObj);
		void SetFilter(const std::function<pragma::physics::RayCastHitType(pragma::physics::IShape &, pragma::physics::IRigidBody &)> &preFilter, const std::function<pragma::physics::RayCastHitType(pragma::physics::IShape &, pragma::physics::IRigidBody &)> &postFilter = nullptr);
		const std::shared_ptr<pragma::physics::IRayCastFilterCallback> &GetFilter() const;
		bool HasFlag(pragma::physics::RayCastFlags flag) const;
	  protected:
		bool HasTarget() const;
	  private:
		umath::Transform m_tStart;
		umath::Transform m_tEnd;

		pragma::physics::RayCastFlags m_flags;
		bool m_bHasTarget = false;
		pragma::physics::CollisionMask m_filterMask;
		pragma::physics::CollisionMask m_filterGroup;
		std::shared_ptr<pragma::physics::IRayCastFilterCallback> m_filter = nullptr;
		util::WeakHandle<pragma::physics::IConvexShape> m_shape = {};
	};

	struct DLLNETWORK TraceResult {
		struct DLLNETWORK MeshInfo {
			std::vector<std::shared_ptr<ModelMesh>> meshes;
			ModelMesh *mesh = nullptr;
			pragma::ModelSubMesh *subMesh = nullptr;
		};
		TraceResult() {}
		TraceResult(const pragma::physics::TraceData &data);
		~TraceResult();
		pragma::physics::RayCastHitType hitType = pragma::physics::RayCastHitType::None;
		EntityHandle entity = {};
		util::TWeakSharedHandle<pragma::physics::ICollisionObject> collisionObj = {};
		std::weak_ptr<pragma::physics::IShape> shape = {};
		PhysObjHandle physObj = {};
		float fraction = 0.f;

		float distance = 0.f;
		Vector3 normal = {};
		Vector3 position = {};
		Vector3 startPosition = Vector3 {};

		std::shared_ptr<MeshInfo> meshInfo = nullptr;

		void GetMeshes(ModelMesh **mesh, pragma::ModelSubMesh **subMesh);
		msys::Material *GetMaterial();
		bool GetMaterial(std::string &mat);
	  private:
		void InitializeMeshes();
	};
};
