// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/components/base_vehicle_component.hpp>
#include <pragma/util/util_handled.hpp>

export module pragma.client.entities.components.vehicle;

export namespace pragma {
	class DLLCLIENT CVehicleComponent final : public BaseVehicleComponent, public CBaseSnapshotComponent {
	  public:
		static void RegisterLuaBindings(lua_State *l, luabind::module_ &modEnts);
		
		CVehicleComponent(BaseEntity &ent);
		virtual ~CVehicleComponent() override;
		static unsigned int GetVehicleCount();
		static const std::vector<CVehicleComponent *> &GetAll();
		virtual void ClearDriver() override;
		virtual void SetDriver(BaseEntity *ent) override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual void Initialize() override;
		virtual void OnEntitySpawn() override;
		virtual bool ShouldTransmitNetData() const override { return true; }

		virtual void ReceiveSnapshotData(NetPacket &packet) override;
	  protected:
		void ReadWheelInfo(NetPacket &packet);
		CallbackHandle m_hCbSteeringWheel;
		virtual Bool ReceiveNetEvent(pragma::NetEventId eventId, NetPacket &packet) override;
		virtual void InitializeLuaObject(lua_State *l) override;
	  private:
		static std::vector<CVehicleComponent *> s_vehicles;
	};
};
