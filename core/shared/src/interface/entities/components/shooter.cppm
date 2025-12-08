// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.shooter;

export import :entities.components.base;
export import :game.bullet_info;
export import :physics.raycast;

export {
	namespace pragma {
		class BasePlayerComponent;
	};
	namespace pragma::ecs {
		namespace events {
			struct DLLNETWORK CEOnBulletsFired : public ComponentEvent {
				CEOnBulletsFired(const game::BulletInfo &bulletInfo, const std::vector<pragma::physics::TraceResult> &hitTargets);
				virtual void PushArguments(lua::State *l) override;
				const game::BulletInfo &bulletInfo;
				const std::vector<pragma::physics::TraceResult> &hitTargets;
			};
			struct DLLNETWORK CEOnFireBullets : public ComponentEvent {
				CEOnFireBullets(const game::BulletInfo &bulletInfo, Vector3 &bulletOrigin, Vector3 &bulletDir, Vector3 *effectsOrigin);
				virtual void PushArguments(lua::State *l) override;
				const game::BulletInfo &bulletInfo;
				Vector3 &bulletOrigin;
				Vector3 &bulletDir;
				Vector3 *effectsOrigin;

				virtual uint32_t GetReturnCount() override;
				virtual void HandleReturnValues(lua::State *l) override;
			};
		};

		namespace baseShooterComponent {
			CLASS_ENUM_COMPAT ComponentEventId EVENT_ON_FIRE_BULLETS;
			CLASS_ENUM_COMPAT ComponentEventId EVENT_ON_BULLETS_FIRED;
		};
		class DLLNETWORK BaseShooterComponent : public BaseEntityComponent {
		  public:
			static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);

			virtual void Initialize() override;

			void GetBulletTraceData(const game::BulletInfo &bulletInfo, pragma::physics::TraceData &data) const;

			virtual void OnFireBullets(const game::BulletInfo &bulletInfo, Vector3 &bulletOrigin, Vector3 &bulletDir, Vector3 *effectsOrigin = nullptr);
			virtual void FireBullets(const game::BulletInfo &bulletInfo, std::vector<pragma::physics::TraceResult> &outHitTargets, bool bMaster = true) = 0;
		  protected:
			BaseShooterComponent(pragma::ecs::BaseEntity &ent);
			struct DLLNETWORK NextBulletInfo {
				std::vector<Vector3> destinations;
				EntityHandle source;
			};
			mutable std::unique_ptr<NextBulletInfo> m_nextBullet = nullptr;
			void ReceiveBulletEvent(NetPacket &packet, pragma::BasePlayerComponent *pl = nullptr);
			std::vector<Vector3> GetBulletDestinations(const Vector3 &origin, const Vector3 &dir, const game::BulletInfo &bulletInfo);
			virtual pragma::physics::RayCastHitType OnBulletHit(const game::BulletInfo &bulletInfo, const pragma::physics::TraceData &data, pragma::physics::PhysObj &phys, physics::ICollisionObject &col);

			pragma::NetEventId m_netEvFireBullets = pragma::INVALID_NET_EVENT;
		};
	};
};
