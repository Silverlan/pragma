// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include "pragma/entities/baseentity_handle.h"
#include "pragma/physics/physobj.h"
#include <sharedutils/util_weak_handle.hpp>
#include <cinttypes>

export module pragma.shared:physics.raycast_filter;

export {
	namespace pragma::physics {
		class DLLNETWORK IRayCastFilterCallback {
		public:
			virtual RayCastHitType PreFilter(pragma::physics::IShape &shape, pragma::physics::IRigidBody &rigidBody) const = 0;
			virtual RayCastHitType PostFilter(pragma::physics::IShape &shape, pragma::physics::IRigidBody &rigidBody) const = 0;
			virtual bool HasPreFilter() const = 0;
			virtual bool HasPostFilter() const = 0;
			virtual ~IRayCastFilterCallback() = default;
		};

		class DLLNETWORK EntityRayCastFilterCallback : public IRayCastFilterCallback {
		public:
			EntityRayCastFilterCallback(BaseEntity &ent);
			virtual RayCastHitType PreFilter(pragma::physics::IShape &shape, pragma::physics::IRigidBody &rigidBody) const override;
			virtual RayCastHitType PostFilter(pragma::physics::IShape &shape, pragma::physics::IRigidBody &rigidBody) const override;
			virtual bool HasPreFilter() const override;
			virtual bool HasPostFilter() const override;
		private:
			EntityHandle m_hEnt = {};
		};

		class DLLNETWORK MultiEntityRayCastFilterCallback : public IRayCastFilterCallback {
		public:
			MultiEntityRayCastFilterCallback(std::vector<EntityHandle> &&ents);
			virtual RayCastHitType PreFilter(pragma::physics::IShape &shape, pragma::physics::IRigidBody &rigidBody) const override;
			virtual RayCastHitType PostFilter(pragma::physics::IShape &shape, pragma::physics::IRigidBody &rigidBody) const override;
			virtual bool HasPreFilter() const override;
			virtual bool HasPostFilter() const override;
		private:
			std::vector<EntityHandle> m_ents = {};
		};

		class DLLNETWORK PhysObjRayCastFilterCallback : public IRayCastFilterCallback {
		public:
			PhysObjRayCastFilterCallback(PhysObj &physObj);
			virtual RayCastHitType PreFilter(pragma::physics::IShape &shape, pragma::physics::IRigidBody &rigidBody) const override;
			virtual RayCastHitType PostFilter(pragma::physics::IShape &shape, pragma::physics::IRigidBody &rigidBody) const override;
			virtual bool HasPreFilter() const override;
			virtual bool HasPostFilter() const override;
		private:
			PhysObjHandle m_hPhys = {};
		};

		class DLLNETWORK CollisionObjRayCastFilterCallback : public IRayCastFilterCallback {
		public:
			CollisionObjRayCastFilterCallback(ICollisionObject &physObj);
			virtual RayCastHitType PreFilter(pragma::physics::IShape &shape, pragma::physics::IRigidBody &rigidBody) const override;
			virtual RayCastHitType PostFilter(pragma::physics::IShape &shape, pragma::physics::IRigidBody &rigidBody) const override;
			virtual bool HasPreFilter() const override;
			virtual bool HasPostFilter() const override;
		private:
			util::TWeakSharedHandle<ICollisionObject> m_hColObj = {};
		};

		class DLLNETWORK CustomRayCastFilterCallback : public IRayCastFilterCallback {
		public:
			CustomRayCastFilterCallback(const std::function<RayCastHitType(pragma::physics::IShape &, pragma::physics::IRigidBody &)> &preFilter, const std::function<RayCastHitType(pragma::physics::IShape &, pragma::physics::IRigidBody &)> &postFilter = nullptr);
			virtual RayCastHitType PreFilter(pragma::physics::IShape &shape, pragma::physics::IRigidBody &rigidBody) const override;
			virtual RayCastHitType PostFilter(pragma::physics::IShape &shape, pragma::physics::IRigidBody &rigidBody) const override;
			virtual bool HasPreFilter() const override;
			virtual bool HasPostFilter() const override;
		private:
			std::function<RayCastHitType(pragma::physics::IShape &, pragma::physics::IRigidBody &)> m_preFilter = nullptr;
			std::function<RayCastHitType(pragma::physics::IShape &, pragma::physics::IRigidBody &)> m_postFilter = nullptr;
		};
	};
};
