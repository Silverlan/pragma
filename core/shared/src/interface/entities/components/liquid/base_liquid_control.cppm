// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.liquid.base_control;

export import :entities.components.base;
export import :game.bullet_info;
export import :physics.liquid;
export import :physics.raycast;
export import :physics.surface_material;

export {
	namespace pragma {
		namespace physics {
			namespace baseLiquidControlComponent {
				REGISTER_COMPONENT_EVENT(EVENT_ON_SPLASH)
				REGISTER_COMPONENT_EVENT(EVENT_ON_PROPERTIES_CHANGED)
			}
			class ICollisionObject;
		};
		namespace baseLiquidControlComponent {
			REGISTER_COMPONENT_EVENT(EVENT_ON_SPLASH)
			REGISTER_COMPONENT_EVENT(EVENT_ON_PROPERTIES_CHANGED)
		}
		class DLLNETWORK BaseLiquidControlComponent : public BaseEntityComponent {
		  public:
			struct DLLNETWORK SplashInfo {
				Vector3 origin = {};
				float radius = 0.f;
				float force = 0.f;
			};

			static void RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
			static void RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
			virtual void Initialize() override;
			virtual void OnEntitySpawn() override;

			const Vector3 &GetLiquidVelocity() const;
			void SetLiquidVelocity(const Vector3 &velocity);

			const physics::PhysLiquid &GetLiquidDescription() const;
			physics::PhysLiquid &GetLiquidDescription();

			double GetDensity() const;
			void SetDensity(double density);

			double GetLinearDragCoefficient() const;
			void SetLinearDragCoefficient(double coefficient);

			double GetTorqueDragCoefficient() const;
			void SetTorqueDragCoefficient(double coefficient);

			float GetStiffness() const;
			virtual void SetStiffness(float stiffness);
			float GetPropagation() const;
			virtual void SetPropagation(float propagation);

			virtual void CreateSplash(const Vector3 &origin, float radius, float force);
		  protected:
			BaseLiquidControlComponent(ecs::BaseEntity &ent);
			struct LocalRayResult {
				float fraction;
				float friction;
				Vector3 hitNormalLocal;
			};
			virtual bool OnBulletHit(const game::BulletInfo &bulletInfo, const physics::TraceData &data, physics::PhysObj *phys, physics::ICollisionObject *col, const LocalRayResult &result);
			void SetSurfaceMaterial(const std::string &mat);
			void SetSurfaceMaterial(const physics::SurfaceMaterial *mat);
			Vector3 m_liquidVelocity;
			physics::PhysLiquid m_liquid = {};
			std::string m_kvSurfaceMaterial;
			NetEventId m_netEvCreateSplash = INVALID_NET_EVENT;

			std::queue<SplashInfo> m_splashes;
		};
		struct DLLNETWORK CEOnSplash : public ComponentEvent {
			CEOnSplash(const BaseLiquidControlComponent::SplashInfo &splashInfo);
			virtual void PushArguments(lua::State *l) override;
			const BaseLiquidControlComponent::SplashInfo &splashInfo;
		};
	};
};
