// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "pragma/networkdefinitions.h"
#include <cinttypes>
#include <functional>

export module pragma.shared:physics.raycast_filter;

export import :entities.base_entity_handle;
export import :physics.collision_object;
export import :physics.object_handle;
export import :physics.shape;

export {
	namespace pragma::ecs {class BaseEntity;}
	namespace pragma::physics {
		class DLLNETWORK IRayCastFilterCallback {
		public:
			virtual pragma::physics::RayCastHitType PreFilter(pragma::physics::IShape &shape, pragma::physics::IRigidBody &rigidBody) const = 0;
			virtual pragma::physics::RayCastHitType PostFilter(pragma::physics::IShape &shape, pragma::physics::IRigidBody &rigidBody) const = 0;
			virtual bool HasPreFilter() const = 0;
			virtual bool HasPostFilter() const = 0;
			virtual ~IRayCastFilterCallback() = default;
		};

		class DLLNETWORK EntityRayCastFilterCallback : public IRayCastFilterCallback {
		public:
			EntityRayCastFilterCallback(pragma::ecs::BaseEntity &ent);
			virtual pragma::physics::RayCastHitType PreFilter(pragma::physics::IShape &shape, pragma::physics::IRigidBody &rigidBody) const override;
			virtual pragma::physics::RayCastHitType PostFilter(pragma::physics::IShape &shape, pragma::physics::IRigidBody &rigidBody) const override;
			virtual bool HasPreFilter() const override;
			virtual bool HasPostFilter() const override;
		private:
			EntityHandle m_hEnt = {};
		};

		class DLLNETWORK MultiEntityRayCastFilterCallback : public IRayCastFilterCallback {
		public:
			MultiEntityRayCastFilterCallback(std::vector<EntityHandle> &&ents);
			virtual pragma::physics::RayCastHitType PreFilter(pragma::physics::IShape &shape, pragma::physics::IRigidBody &rigidBody) const override;
			virtual pragma::physics::RayCastHitType PostFilter(pragma::physics::IShape &shape, pragma::physics::IRigidBody &rigidBody) const override;
			virtual bool HasPreFilter() const override;
			virtual bool HasPostFilter() const override;
		private:
			std::vector<EntityHandle> m_ents = {};
		};

		class DLLNETWORK PhysObjRayCastFilterCallback : public IRayCastFilterCallback {
		public:
			PhysObjRayCastFilterCallback(pragma::physics::PhysObj &physObj);
			virtual pragma::physics::RayCastHitType PreFilter(pragma::physics::IShape &shape, pragma::physics::IRigidBody &rigidBody) const override;
			virtual pragma::physics::RayCastHitType PostFilter(pragma::physics::IShape &shape, pragma::physics::IRigidBody &rigidBody) const override;
			virtual bool HasPreFilter() const override;
			virtual bool HasPostFilter() const override;
		private:
			PhysObjHandle m_hPhys = {};
		};

		class DLLNETWORK CollisionObjRayCastFilterCallback : public IRayCastFilterCallback {
		public:
			CollisionObjRayCastFilterCallback(ICollisionObject &physObj);
			virtual pragma::physics::RayCastHitType PreFilter(pragma::physics::IShape &shape, pragma::physics::IRigidBody &rigidBody) const override;
			virtual pragma::physics::RayCastHitType PostFilter(pragma::physics::IShape &shape, pragma::physics::IRigidBody &rigidBody) const override;
			virtual bool HasPreFilter() const override;
			virtual bool HasPostFilter() const override;
		private:
			util::TWeakSharedHandle<ICollisionObject> m_hColObj = {};
		};

		class DLLNETWORK CustomRayCastFilterCallback : public IRayCastFilterCallback {
		public:
			CustomRayCastFilterCallback(const std::function<pragma::physics::RayCastHitType(pragma::physics::IShape &, pragma::physics::IRigidBody &)> &preFilter, const std::function<pragma::physics::RayCastHitType(pragma::physics::IShape &, pragma::physics::IRigidBody &)> &postFilter = nullptr);
			virtual pragma::physics::RayCastHitType PreFilter(pragma::physics::IShape &shape, pragma::physics::IRigidBody &rigidBody) const override;
			virtual pragma::physics::RayCastHitType PostFilter(pragma::physics::IShape &shape, pragma::physics::IRigidBody &rigidBody) const override;
			virtual bool HasPreFilter() const override;
			virtual bool HasPostFilter() const override;
		private:
			std::function<pragma::physics::RayCastHitType(pragma::physics::IShape &, pragma::physics::IRigidBody &)> m_preFilter = nullptr;
			std::function<pragma::physics::RayCastHitType(pragma::physics::IShape &, pragma::physics::IRigidBody &)> m_postFilter = nullptr;
		};
	};
};
