#ifndef __C_VEHICLE_COMPONENT_HPP__
#define __C_VEHICLE_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/components/base_vehicle_component.hpp>
#include <pragma/util/util_handled.hpp>

class CBaseEntity;
namespace pragma
{
	class DLLCLIENT CVehicleComponent final
		: public BaseVehicleComponent,
		public CBaseNetComponent
	{
	public:
		CVehicleComponent(BaseEntity &ent);
		virtual ~CVehicleComponent() override;
		static unsigned int GetVehicleCount();
		static const std::vector<CVehicleComponent*> &GetAll();
		virtual void ClearDriver() override;
		virtual void SetDriver(BaseEntity *ent) override;
		virtual void ReceiveData(NetPacket &packet) override;
		void ToggleCamera();
		virtual void SetCamera(bool bFirstPerson);
		virtual void Initialize() override;
		virtual bool ShouldTransmitNetData() const override {return true;}
	protected:
		void ReadWheelInfo(NetPacket &packet);
		CallbackHandle m_camUpdate;
		CallbackHandle m_hCbSteeringWheel;
		CallbackHandle m_playerAction;
		bool m_bFirstPersonCamera;
		virtual Bool ReceiveNetEvent(pragma::NetEventId eventId,NetPacket &packet) override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	private:
		static std::vector<CVehicleComponent*> s_vehicles;
	};
};

#endif
