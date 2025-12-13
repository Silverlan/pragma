// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:physics.raycast_filter;

export import :entities.base_entity_handle;
export import :physics.collision_object;
export import :physics.object_handle;
export import :physics.shape;

export {
	namespace pragma::ecs {
		class BaseEntity;
	}
	namespace pragma::physics {
		class DLLNETWORK IRayCastFilterCallback {
		  public:
			virtual RayCastHitType PreFilter(IShape &shape, IRigidBody &rigidBody) const = 0;
			virtual RayCastHitType PostFilter(IShape &shape, IRigidBody &rigidBody) const = 0;
			virtual bool HasPreFilter() const = 0;
			virtual bool HasPostFilter() const = 0;
			virtual ~IRayCastFilterCallback() = default;
		};

		class DLLNETWORK EntityRayCastFilterCallback : public IRayCastFilterCallback {
		  public:
			EntityRayCastFilterCallback(ecs::BaseEntity &ent);
			virtual RayCastHitType PreFilter(IShape &shape, IRigidBody &rigidBody) const override;
			virtual RayCastHitType PostFilter(IShape &shape, IRigidBody &rigidBody) const override;
			virtual bool HasPreFilter() const override;
			virtual bool HasPostFilter() const override;
		  private:
			EntityHandle m_hEnt = {};
		};

		class DLLNETWORK MultiEntityRayCastFilterCallback : public IRayCastFilterCallback {
		  public:
			MultiEntityRayCastFilterCallback(std::vector<EntityHandle> &&ents);
			virtual RayCastHitType PreFilter(IShape &shape, IRigidBody &rigidBody) const override;
			virtual RayCastHitType PostFilter(IShape &shape, IRigidBody &rigidBody) const override;
			virtual bool HasPreFilter() const override;
			virtual bool HasPostFilter() const override;
		  private:
			std::vector<EntityHandle> m_ents = {};
		};

		class DLLNETWORK PhysObjRayCastFilterCallback : public IRayCastFilterCallback {
		  public:
			PhysObjRayCastFilterCallback(PhysObj &physObj);
			virtual RayCastHitType PreFilter(IShape &shape, IRigidBody &rigidBody) const override;
			virtual RayCastHitType PostFilter(IShape &shape, IRigidBody &rigidBody) const override;
			virtual bool HasPreFilter() const override;
			virtual bool HasPostFilter() const override;
		  private:
			PhysObjHandle m_hPhys = {};
		};

		class DLLNETWORK CollisionObjRayCastFilterCallback : public IRayCastFilterCallback {
		  public:
			CollisionObjRayCastFilterCallback(ICollisionObject &physObj);
			virtual RayCastHitType PreFilter(IShape &shape, IRigidBody &rigidBody) const override;
			virtual RayCastHitType PostFilter(IShape &shape, IRigidBody &rigidBody) const override;
			virtual bool HasPreFilter() const override;
			virtual bool HasPostFilter() const override;
		  private:
			util::TWeakSharedHandle<ICollisionObject> m_hColObj = {};
		};

		class DLLNETWORK CustomRayCastFilterCallback : public IRayCastFilterCallback {
		  public:
			CustomRayCastFilterCallback(const std::function<RayCastHitType(IShape &, IRigidBody &)> &preFilter,
			  const std::function<RayCastHitType(IShape &, IRigidBody &)> &postFilter = nullptr);
			virtual RayCastHitType PreFilter(IShape &shape, IRigidBody &rigidBody) const override;
			virtual RayCastHitType PostFilter(IShape &shape, IRigidBody &rigidBody) const override;
			virtual bool HasPreFilter() const override;
			virtual bool HasPostFilter() const override;
		  private:
			std::function<RayCastHitType(IShape &, IRigidBody &)> m_preFilter = nullptr;
			std::function<RayCastHitType(IShape &, IRigidBody &)> m_postFilter = nullptr;
		};
	};
};
