#ifndef __S_FUNC_WATER_H__
#define __S_FUNC_WATER_H__

#include "pragma/serverdefinitions.h"
#include "pragma/entities/trigger/s_trigger_touch.h"
#include "pragma/entities/func/basefuncwater.h"
#include "pragma/entities/components/s_entity_component.hpp"

namespace pragma
{
	class DLLSERVER SWaterComponent final
		: public BaseFuncWaterComponent,
		public SBaseNetComponent
	{
	public:
		SWaterComponent(BaseEntity &ent);
		virtual ~SWaterComponent() override;
		virtual void Initialize() override;
		virtual void OnEntitySpawn() override;
		virtual void SendData(NetPacket &packet,networking::ClientRecipientFilter &rp) override;
		virtual void CreateSplash(const Vector3 &origin,float radius,float force) override;
		void UpdateSurfaceSimulator();
		virtual const Vector3 &GetPosition() const override;
		virtual const Quat &GetOrientation() const override;
		virtual bool ShouldTransmitNetData() const override {return true;}
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	protected:
		virtual bool ShouldSimulateSurface() const override;
		bool m_bUsingClientsideSimulation = false;
		CallbackHandle m_cbClientSimulatorUpdate = {};
		CallbackHandle m_cbGameInitialized = {};
	};
};

class DLLSERVER FuncWater
	: public SBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif