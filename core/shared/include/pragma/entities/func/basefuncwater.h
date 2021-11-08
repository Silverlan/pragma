/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __BASEFUNCWATER_H__
#define __BASEFUNCWATER_H__

#include "pragma/entities/components/base_entity_component.hpp"
#include "pragma/physics/phys_liquid.hpp"
#include "pragma/entities/trigger/base_trigger_touch.hpp"
#include "pragma/math/plane.h"
#include "pragma/basewaterobject.hpp"

class PhysWaterSurfaceSimulator;
struct BulletInfo;
namespace pragma
{
	class DLLNETWORK BaseFuncWaterComponent
		: public BaseEntityComponent,
		virtual public BaseWaterObject
	{
	public:
		static ComponentEventId EVENT_ON_WATER_SURFACE_SIMULATOR_CHANGED;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager,TRegisterComponentEvent registerEvent);
		using BaseEntityComponent::BaseEntityComponent;
		enum class DLLNETWORK SpawnFlags : uint32_t
		{
			SurfaceSimulation = 2048
		};
		struct LocalRayResult
		{
			float fraction;
			float friction;
			Vector3 hitNormalLocal;
		};

		virtual void Initialize() override;

		virtual bool OnBulletHit(const BulletInfo &bulletInfo,const TraceData &data,PhysObj *phys,pragma::physics::ICollisionObject *col,const LocalRayResult &result);

		const Vector3 &GetWaterVelocity() const;
		void SetWaterVelocity(const Vector3 &velocity);

		const PhysLiquid &GetLiquidDescription() const;
		PhysLiquid &GetLiquidDescription();

		double GetDensity() const;
		void SetDensity(double density);

		double GetLinearDragCoefficient() const;
		void SetLinearDragCoefficient(double coefficient);

		double GetTorqueDragCoefficient() const;
		void SetTorqueDragCoefficient(double coefficient);

		void SimulateBuoyancy() const;
		virtual void CreateSplash(const Vector3 &origin,float radius,float force);

		float GetStiffness() const;
		virtual void SetStiffness(float stiffness);
		float GetPropagation() const;
		virtual void SetPropagation(float propagation);
		uint32_t GetSpacing() const;
		void SetSpacing(uint32_t spacing);
		const PhysWaterSurfaceSimulator *GetSurfaceSimulator() const;
		PhysWaterSurfaceSimulator *GetSurfaceSimulator();
		virtual void ReloadSurfaceSimulator();
		virtual void OnEndTouch(BaseEntity *ent,PhysObj *phys);

		Vector3 ProjectToSurface(const Vector3 &pos) const;
		bool CalcLineSurfaceIntersection(const Vector3 &lineOrigin,const Vector3 &lineDir,double *outT=nullptr,double *outU=nullptr,double *outV=nullptr,bool bCull=false) const;
		virtual bool OnRayResultCallback(CollisionMask rayCollisionGroup,CollisionMask rayCollisionMask);
		virtual util::EventReply HandleEvent(ComponentEventId eventId,ComponentEvent &evData) override;
		virtual void OnEntitySpawn() override;
	protected:
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		virtual std::shared_ptr<PhysWaterSurfaceSimulator> InitializeSurfaceSimulator(const Vector2 &min,const Vector2 &max,float originY);
		virtual void ClearWaterSurface();
		virtual bool ShouldSimulateSurface() const;
		void OnPhysicsInitialized();
		void SetMaxWaveHeight(float height);
		void SetSurfaceMaterial(const std::string &mat);
		void SetSurfaceMaterial(const SurfaceMaterial *mat);
		void InitializeWaterSurface();

		struct SplashInfo
		{
			Vector3 origin = {};
			float radius = 0.f;
			float force = 0.f;
		};
		std::queue<SplashInfo> m_splashes;

		pragma::NetEventId m_netEvCreateSplash = pragma::INVALID_NET_EVENT;
		pragma::NetEventId m_netEvSetWaterPlane = pragma::INVALID_NET_EVENT;

		double m_originalWaterPlaneDistance = 0.0;
		Vector3 m_waterVelocity;
		uint32_t m_spacing = 10;
		std::string m_kvSurfaceMaterial;
		float m_kvMaxWaveHeight = 100.f;
		std::weak_ptr<ModelSubMesh> m_waterMesh = {};
		PhysLiquid m_liquid = {};
		std::shared_ptr<PhysWaterSurfaceSimulator> m_physSurfaceSim = nullptr;
	};
};

#endif
