/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_FUNC_WATER_H__
#define __C_FUNC_WATER_H__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/trigger/c_trigger_touch.h"
#include "pragma/entities/func/basefuncwater.h"
#include "pragma/c_water_object.hpp"
#include "pragma/entities/components/c_entity_component.hpp"

class CWaterSurface;
namespace pragma
{
	class DLLCLIENT CWaterComponent final
		: public BaseFuncWaterComponent,
		public CBaseNetComponent,
		public CWaterObject
	{
	public:
		CWaterComponent(BaseEntity &ent);
		virtual ~CWaterComponent() override;
		virtual void Initialize() override;
		virtual bool OnBulletHit(const BulletInfo &bulletInfo,const TraceData &data,PhysObj *phys,pragma::physics::ICollisionObject *col,const LocalRayResult &result) override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual Bool ReceiveNetEvent(pragma::NetEventId eventId,NetPacket &packet) override;
		virtual void CreateSplash(const Vector3 &origin,float radius,float force) override;
		void UpdateSurfaceSimulator();
		CWaterSurface *GetSurfaceEntity() const;

		virtual void SetStiffness(float stiffness) override;
		virtual void SetPropagation(float propagation) override;
		virtual void ReloadSurfaceSimulator() override;
		virtual CMaterial *GetWaterMaterial() const override;
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override {return true;}

		virtual const Vector3 &GetPosition() const override;
		virtual const Quat &GetOrientation() const override;
	protected:
		virtual void OnEntitySpawn() override;
		virtual std::shared_ptr<PhysWaterSurfaceSimulator> InitializeSurfaceSimulator(const Vector2 &min,const Vector2 &max,float originY) override;
		void SetupWater();
		bool m_bUsingClientsideSimulation = false;
		CallbackHandle m_cbClientSimulatorUpdate = {};
		CallbackHandle m_cbGameInitialized = {};
		mutable EntityHandle m_hWaterSurface = {};
	};
};

class CWaterSurface;
class DLLCLIENT CFuncWater
	: public CBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif
