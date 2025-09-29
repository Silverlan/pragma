// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include "mathutil/uvec.h"

export module pragma.shared:entities.components.liquid.base_control;

export import :entities.components.base;
export import :game.bullet_info;
export import :physics.liquid;
export import :physics.raycast;
export import :physics.surface_material;

export {
	class PhysObj;
	namespace pragma {
		namespace physics {class ICollisionObject;};
		class DLLNETWORK BaseLiquidControlComponent : public BaseEntityComponent {
		  public:
			struct DLLNETWORK SplashInfo {
				Vector3 origin = {};
				float radius = 0.f;
				float force = 0.f;
			};

			static ComponentEventId EVENT_ON_SPLASH;
			static ComponentEventId EVENT_ON_PROPERTIES_CHANGED;
			static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
			static void RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
			virtual void Initialize() override;
			virtual void OnEntitySpawn() override;

			const Vector3 &GetLiquidVelocity() const;
			void SetLiquidVelocity(const Vector3 &velocity);

			const PhysLiquid &GetLiquidDescription() const;
			PhysLiquid &GetLiquidDescription();

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
			BaseLiquidControlComponent(BaseEntity &ent);
			struct LocalRayResult {
				float fraction;
				float friction;
				Vector3 hitNormalLocal;
			};
			virtual bool OnBulletHit(const BulletInfo &bulletInfo, const TraceData &data, PhysObj *phys, pragma::physics::ICollisionObject *col, const LocalRayResult &result);
			void SetSurfaceMaterial(const std::string &mat);
			void SetSurfaceMaterial(const SurfaceMaterial *mat);
			Vector3 m_liquidVelocity;
			PhysLiquid m_liquid = {};
			std::string m_kvSurfaceMaterial;
			pragma::NetEventId m_netEvCreateSplash = pragma::INVALID_NET_EVENT;

			std::queue<SplashInfo> m_splashes;
		};
		struct DLLNETWORK CEOnSplash : public ComponentEvent {
			CEOnSplash(const BaseLiquidControlComponent::SplashInfo &splashInfo);
			virtual void PushArguments(lua_State *l) override;
			const BaseLiquidControlComponent::SplashInfo &splashInfo;
		};
	};
};
