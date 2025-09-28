// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include <mathutil/glmutil.h>
#include <mathutil/transform.hpp>
#include <sharedutils/util_weak_handle.hpp>
#include <memory>
#include "material.h"

export module pragma.shared:physics.raycast;

export import :entities.base_entity_handle;
export import :model.model_mesh;
export import :physics.collision_object;
export import :physics.object_handle;
export import :physics.raycast_filter;

export {
	class BaseEntity;
	class DLLNETWORK TraceData {
	public:
		TraceData();
		TraceData(const TraceData &other);
		~TraceData();
		RayCastFlags GetFlags() const;
		const umath::Transform &GetSource() const;
		const umath::Transform &GetTarget() const;
		Vector3 GetSourceOrigin() const;
		Quat GetSourceRotation() const;
		Vector3 GetTargetOrigin() const;
		Quat GetTargetRotation() const;
		Vector3 GetDirection() const;
		float GetDistance() const;
		CollisionMask GetCollisionFilterMask() const;
		CollisionMask GetCollisionFilterGroup() const;
		void SetShape(const pragma::physics::IConvexShape &shape);
		const pragma::physics::IConvexShape *GetShape() const;
		void SetSource(const Vector3 &origin);
		void SetSourceRotation(const Quat &rot);
		void SetSource(const umath::Transform &t);
		void SetTarget(const Vector3 &target);
		void SetTargetRotation(const Quat &rot);
		void SetTarget(const umath::Transform &t);
		void SetRotation(const Quat &rot);
		void SetFlags(RayCastFlags flags);
		void SetCollisionFilterMask(CollisionMask mask);
		void SetCollisionFilterGroup(CollisionMask group);
		void SetFilter(const std::shared_ptr<pragma::physics::IRayCastFilterCallback> &filter);
		void SetFilter(BaseEntity &ent);
		void SetFilter(std::vector<EntityHandle> &&ents);
		void SetFilter(PhysObj &phys);
		void SetFilter(pragma::physics::ICollisionObject &colObj);
		void SetFilter(const std::function<RayCastHitType(pragma::physics::IShape &, pragma::physics::IRigidBody &)> &preFilter, const std::function<RayCastHitType(pragma::physics::IShape &, pragma::physics::IRigidBody &)> &postFilter = nullptr);
		const std::shared_ptr<pragma::physics::IRayCastFilterCallback> &GetFilter() const;
		bool HasFlag(RayCastFlags flag) const;
	protected:
		bool HasTarget() const;
	private:
		umath::Transform m_tStart;
		umath::Transform m_tEnd;

		RayCastFlags m_flags;
		bool m_bHasTarget = false;
		CollisionMask m_filterMask;
		CollisionMask m_filterGroup;
		std::shared_ptr<pragma::physics::IRayCastFilterCallback> m_filter = nullptr;
		util::WeakHandle<pragma::physics::IConvexShape> m_shape = {};
	};

	struct DLLNETWORK TraceResult {
		struct DLLNETWORK MeshInfo {
			std::vector<std::shared_ptr<ModelMesh>> meshes;
			ModelMesh *mesh = nullptr;
			ModelSubMesh *subMesh = nullptr;
		};
		TraceResult() {}
		TraceResult(const TraceData &data);
		~TraceResult();
		RayCastHitType hitType = RayCastHitType::None;
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

		void GetMeshes(ModelMesh **mesh, ModelSubMesh **subMesh);
		Material *GetMaterial();
		bool GetMaterial(std::string &mat);
	private:
		void InitializeMeshes();
	};
};
